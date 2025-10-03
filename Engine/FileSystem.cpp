#include "FileSystem.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace BSE
{
    bool FileStreamingSystem::ReadFileToString(const std::string& filepath, std::string& out)
    {
        std::ifstream ifs(filepath, std::ios::in | std::ios::binary);
        if (!ifs.is_open()) return false;
        std::ostringstream ss;
        ss << ifs.rdbuf();
        out = ss.str();
        return true;
    }

    FileStreamingSystem::FileStreamingSystem(ThreadingSystem& threadSystem)
        : m_threadSystem(threadSystem)
    {
    }

    FileStreamingSystem::~FileStreamingSystem()
    {
        m_threadSystem.WaitAll();

        {
            std::lock_guard<std::mutex> lock(m_shaderMutex);
            for (auto& kv : m_shaderPrograms)
            {
                if (kv.second != 0) glDeleteProgram(kv.second);
            }
            m_shaderPrograms.clear();
        }

        {
            std::lock_guard<std::mutex> lock(m_materialMutex);
            m_materials.clear();
        }

        {
            std::lock_guard<std::mutex> lock(m_modelMutex);
            m_models.clear();
        }

        {
            std::lock_guard<std::mutex> lock(m_renderMeshMutex);
            for (auto& kv : m_renderMeshes)
            {
                for (const RenderMesh& rm : kv.second)
                {
                    if (rm.VAO) glDeleteVertexArrays(1, &rm.VAO);
                    if (rm.VBO) glDeleteBuffers(1, &rm.VBO);
                    if (rm.EBO) glDeleteBuffers(1, &rm.EBO);
                }
            }
            m_renderMeshes.clear();
        }

        {
            std::lock_guard<std::mutex> lock(m_soundMutex);
            m_sounds.clear();
        }
    }

    void FileStreamingSystem::LoadModelAsync(const std::string& id, const std::string& filepath, const std::function<void(ModelPtr)>& callback)
    {
        {
            std::lock_guard<std::mutex> lock(m_modelMutex);
            if (m_models.find(id) != m_models.end())
            {
                std::lock_guard<std::mutex> qlock(m_queueMutex);
                m_mainThreadTasks.push_back({[cb = callback, model = m_models[id]]() {
                    if (cb) cb(model);
                }});
                return;
            }
        }

        m_threadSystem.SubmitTask([this, id, filepath, callback]()
        {
            auto loader = std::make_shared<ModelLoader>();
            if (!loader->Load(filepath))
            {
                std::cerr << "[FileStreamingSystem] Model parse failed: " << filepath << std::endl;
                // schedule callback(nullptr)
                std::lock_guard<std::mutex> qlock(m_queueMutex);
                m_mainThreadTasks.push_back({[cb = callback]() {
                    if (cb) cb(nullptr);
                }});
                return;
            }

            std::lock_guard<std::mutex> qlock(m_queueMutex);
            m_mainThreadTasks.push_back({[this, id, loader, callback]() {
                {
                    std::lock_guard<std::mutex> lock(m_modelMutex);
                    m_models[id] = loader;
                }
                if (callback) callback(loader);
            }});
        });
    }

    ModelPtr FileStreamingSystem::GetOrLoadModelSync(const std::string& id, const std::string& filepath)
    {
        {
            std::lock_guard<std::mutex> lock(m_modelMutex);
            auto it = m_models.find(id);
            if (it != m_models.end()) return it->second;
        }

        auto loader = std::make_shared<ModelLoader>();
        if (!loader->Load(filepath))
        {
            std::cerr << "[FileStreamingSystem] GetOrLoadModelSync failed: " << filepath << std::endl;
            return nullptr;
        }

        {
            std::lock_guard<std::mutex> lock(m_modelMutex);
            m_models[id] = loader;
        }
        return loader;
    }

    void FileStreamingSystem::UnloadModel(const std::string& id)
    {
        std::lock_guard<std::mutex> lock(m_modelMutex);
        m_models.erase(id);
    }

    void FileStreamingSystem::LoadMaterialAsync(const std::string& id, const std::string& filepath, const std::function<void(MaterialPtr)>& callback)
    {
        {
            std::lock_guard<std::mutex> lock(m_materialMutex);
            if (m_materials.find(id) != m_materials.end())
            {
                std::lock_guard<std::mutex> qlock(m_queueMutex);
                m_mainThreadTasks.push_back({[cb = callback, mat = m_materials[id]]() {
                    if (cb) cb(mat);
                }});
                return;
            }
        }

        m_threadSystem.SubmitTask([this, id, filepath, callback]()
        {
            auto mat = std::make_shared<Material>();
            if (!mat->ParseMaterialFile(filepath))
            {
                std::cerr << "[FileStreamingSystem] Material parse failed: " << filepath << std::endl;
                std::lock_guard<std::mutex> qlock(m_queueMutex);
                m_mainThreadTasks.push_back({[cb = callback]() { if (cb) cb(nullptr); }});
                return;
            }

            std::vector<std::string> texPaths;
            if (!mat->diffusePath.empty())  texPaths.push_back(mat->diffusePath);
            if (!mat->normalPath.empty())   texPaths.push_back(mat->normalPath);
            if (!mat->roughnessPath.empty())texPaths.push_back(mat->roughnessPath);
            if (!mat->metallicPath.empty()) texPaths.push_back(mat->metallicPath);
            if (!mat->aoPath.empty())       texPaths.push_back(mat->aoPath);
            if (!mat->emissivePath.empty()) texPaths.push_back(mat->emissivePath);

            std::unordered_map<std::string, ImageData> images;
            for (const auto& p : texPaths)
            {
                ImageData img;
                if (Texture2D::LoadImageToMemory(p, img, true))
                {
                    images.emplace(p, std::move(img));
                }
                else
                {
                    std::cerr << "[FileStreamingSystem] Image decode failed: " << p << std::endl;
                }
            }

            std::lock_guard<std::mutex> qlock(m_queueMutex);
            m_mainThreadTasks.push_back({[this, id, mat, images = std::move(images), callback]() mutable {
                mat->FinalizeTexturesFromImageData(images);
                {
                    std::lock_guard<std::mutex> lock(m_materialMutex);
                    m_materials[id] = mat;
                }
                if (callback) callback(mat);
            }});
        });
    }

    MaterialPtr FileStreamingSystem::GetOrLoadMaterialSync(const std::string& id, const std::string& filepath)
    {
        {
            std::lock_guard<std::mutex> lock(m_materialMutex);
            auto it = m_materials.find(id);
            if (it != m_materials.end()) return it->second;
        }

        auto mat = std::make_shared<Material>();
        if (!mat->LoadFromFile(filepath))
        {
            std::cerr << "[FileStreamingSystem] GetOrLoadMaterialSync failed: " << filepath << std::endl;
            return nullptr;
        }

        {
            std::lock_guard<std::mutex> lock(m_materialMutex);
            m_materials[id] = mat;
        }
        return mat;
    }

    void FileStreamingSystem::UnloadMaterial(const std::string& id)
    {
        std::lock_guard<std::mutex> lock(m_materialMutex);
        m_materials.erase(id);
    }

    void FileStreamingSystem::LoadSoundAsync(const std::string& id, const std::string& filepath, const std::function<void(SoundPtr)>& callback)
    {
        {
            std::lock_guard<std::mutex> lock(m_soundMutex);
            if (m_sounds.find(id) != m_sounds.end())
            {
                std::lock_guard<std::mutex> qlock(m_queueMutex);
                m_mainThreadTasks.push_back({[cb = callback, snd = m_sounds[id]]() {
                    if (cb) cb(snd);
                }});
                return;
            }
        }

        std::lock_guard<std::mutex> qlock(m_queueMutex);
        m_mainThreadTasks.push_back({[this, id, filepath, callback]() {
            auto snd = std::make_shared<SoundBuffer>();
            if (!snd->LoadFromFile(filepath))
            {
                std::cerr << "[FileStreamingSystem] Sound load failed: " << filepath << std::endl;
                if (callback) callback(nullptr);
                return;
            }
            {
                std::lock_guard<std::mutex> lock(m_soundMutex);
                m_sounds[id] = snd;
            }
            if (callback) callback(snd);
        }});
    }

    SoundPtr FileStreamingSystem::GetOrLoadSoundSync(const std::string& id, const std::string& filepath)
    {
        {
            std::lock_guard<std::mutex> lock(m_soundMutex);
            auto it = m_sounds.find(id);
            if (it != m_sounds.end()) return it->second;
        }

        auto snd = std::make_shared<SoundBuffer>();
        if (!snd->LoadFromFile(filepath))
        {
            std::cerr << "[FileStreamingSystem] GetOrLoadSoundSync failed: " << filepath << std::endl;
            return nullptr;
        }

        {
            std::lock_guard<std::mutex> lock(m_soundMutex);
            m_sounds[id] = snd;
        }
        return snd;
    }

    void FileStreamingSystem::UnloadSound(const std::string& id)
    {
        std::lock_guard<std::mutex> lock(m_soundMutex);
        m_sounds.erase(id);
    }

    void FileStreamingSystem::LoadShaderProgramAsync(const std::string& id, const std::string& vertexPath, const std::string& fragmentPath, const std::function<void(GLuint)>& callback)
    {
        {
            std::lock_guard<std::mutex> lock(m_shaderMutex);
            if (m_shaderPrograms.find(id) != m_shaderPrograms.end())
            {
                GLuint prog = m_shaderPrograms[id];
                std::lock_guard<std::mutex> qlock(m_queueMutex);
                m_mainThreadTasks.push_back({[cb = callback, prog]() { if (cb) cb(prog); }});
                return;
            }
        }

        m_threadSystem.SubmitTask([this, id, vertexPath, fragmentPath, callback]()
        {
            std::string vertSrc, fragSrc;
            bool okV = vertexPath.empty() || ReadFileToString(vertexPath, vertSrc);
            bool okF = fragmentPath.empty() || ReadFileToString(fragmentPath, fragSrc);

            if (!okV || !okF)
            {
                std::cerr << "[FileStreamingSystem] Shader file read failed: " << vertexPath << " / " << fragmentPath << std::endl;
                std::lock_guard<std::mutex> qlock(m_queueMutex);
                m_mainThreadTasks.push_back({[cb = callback]() { if (cb) cb(0); }});
                return;
            }

            std::lock_guard<std::mutex> qlock(m_queueMutex);
            m_mainThreadTasks.push_back({[this, id, vertSrc = std::move(vertSrc), fragSrc = std::move(fragSrc), callback]() mutable {
                try
                {
                    Shader vertex(vertSrc, ShaderType::Vertex);
                    Shader fragment(fragSrc, ShaderType::Fragment);
                    ShaderProgram program(vertex, fragment);

                    GLuint programID = program.Release();

                    {
                        std::lock_guard<std::mutex> lock(m_shaderMutex);
                        m_shaderPrograms[id] = programID;
                    }

                    if (callback) callback(programID);
                }
                catch (const std::exception& e)
                {
                    std::cerr << "[FileStreamingSystem] Shader compile/link failed: " << e.what() << std::endl;
                    if (callback) callback(0);
                }
            }});
        });
    }

    GLuint FileStreamingSystem::GetOrLoadShaderProgramSync(const std::string& id, const std::string& vertexPath, const std::string& fragmentPath)
    {
        {
            std::lock_guard<std::mutex> lock(m_shaderMutex);
            auto it = m_shaderPrograms.find(id);
            if (it != m_shaderPrograms.end()) return it->second;
        }

        std::string vertSrc, fragSrc;
        if (!vertexPath.empty() && !ReadFileToString(vertexPath, vertSrc))
        {
            std::cerr << "[FileStreamingSystem] GetOrLoadShaderProgramSync failed to read vertex: " << vertexPath << std::endl;
            return 0;
        }
        if (!fragmentPath.empty() && !ReadFileToString(fragmentPath, fragSrc))
        {
            std::cerr << "[FileStreamingSystem] GetOrLoadShaderProgramSync failed to read fragment: " << fragmentPath << std::endl;
            return 0;
        }

        try
        {
            Shader vertex(vertSrc, ShaderType::Vertex);
            Shader fragment(fragSrc, ShaderType::Fragment);
            ShaderProgram program(vertex, fragment);
            GLuint programID = program.Release();

            {
                std::lock_guard<std::mutex> lock(m_shaderMutex);
                m_shaderPrograms[id] = programID;
            }
            return programID;
        }
        catch (const std::exception& e)
        {
            std::cerr << "[FileStreamingSystem] GetOrLoadShaderProgramSync compile failed: " << e.what() << std::endl;
            return 0;
        }
    }

    void FileStreamingSystem::UnloadShaderProgram(const std::string& id)
    {
        std::lock_guard<std::mutex> lock(m_shaderMutex);
        auto it = m_shaderPrograms.find(id);
        if (it != m_shaderPrograms.end())
        {
            if (it->second != 0) glDeleteProgram(it->second);
            m_shaderPrograms.erase(it);
        }
    }

    void FileStreamingSystem::AddRenderMesh(const std::string& id, const RenderMesh& mesh)
    {
        std::lock_guard<std::mutex> lock(m_renderMeshMutex);
        m_renderMeshes[id].push_back(mesh);
    }

    std::vector<RenderMesh> FileStreamingSystem::GetRenderMeshes(const std::string& id) const
    {
        std::lock_guard<std::mutex> lock(m_renderMeshMutex);
        auto it = m_renderMeshes.find(id);
        if (it == m_renderMeshes.end()) return {};
        return it->second;
    }

    void FileStreamingSystem::RemoveRenderMeshes(const std::string& id)
    {
        std::lock_guard<std::mutex> lock(m_renderMeshMutex);
        auto it = m_renderMeshes.find(id);
        if (it != m_renderMeshes.end())
        {
            // clean up VAO/VBO/EBO
            for (const RenderMesh& rm : it->second)
            {
                if (rm.VAO) glDeleteVertexArrays(1, &rm.VAO);
                if (rm.VBO) glDeleteBuffers(1, &rm.VBO);
                if (rm.EBO) glDeleteBuffers(1, &rm.EBO);
            }
            m_renderMeshes.erase(it);
        }
    }

    bool FileStreamingSystem::HasModel(const std::string& id) const
    {
        std::lock_guard<std::mutex> lock(m_modelMutex);
        return m_models.find(id) != m_models.end();
    }

    bool FileStreamingSystem::HasMaterial(const std::string& id) const
    {
        std::lock_guard<std::mutex> lock(m_materialMutex);
        return m_materials.find(id) != m_materials.end();
    }

    bool FileStreamingSystem::HasSound(const std::string& id) const
    {
        std::lock_guard<std::mutex> lock(m_soundMutex);
        return m_sounds.find(id) != m_sounds.end();
    }

    bool FileStreamingSystem::HasShaderProgram(const std::string& id) const
    {
        std::lock_guard<std::mutex> lock(m_shaderMutex);
        return m_shaderPrograms.find(id) != m_shaderPrograms.end();
    }

    void FileStreamingSystem::Update()
    {
        std::vector<MainThreadTask> tasks;
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            tasks.swap(m_mainThreadTasks);
        }

        for (auto& t : tasks)
        {
            try { t.fn(); }
            catch (const std::exception& e) { std::cerr << "[FileStreamingSystem] main task exception: " << e.what() << std::endl; }
        }
    }
}
