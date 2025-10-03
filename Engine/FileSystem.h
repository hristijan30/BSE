#pragma once

#include "Define.h"
#include "StandardInclude.h"

#include "Renderer/Material.h"
#include "Renderer/Model.h"
#include "Sound/Sound.h"
#include "Renderer/Shader.h"
#include "Renderer/Texture2D.h"

#include "Threading/ThreadingSystem.h"

namespace BSE
{
    using ModelPtr    = std::shared_ptr<ModelLoader>;
    using MaterialPtr = std::shared_ptr<Material>;
    using SoundPtr    = std::shared_ptr<SoundBuffer>;

    class DLL_EXPORT FileStreamingSystem
    {
    public:
        explicit FileStreamingSystem(ThreadingSystem& threadSystem);
        ~FileStreamingSystem();

        void LoadModelAsync(const std::string& id, const std::string& filepath, const std::function<void(ModelPtr)>& callback);
        void LoadMaterialAsync(const std::string& id, const std::string& filepath, const std::function<void(MaterialPtr)>& callback);
        void LoadSoundAsync(const std::string& id, const std::string& filepath, const std::function<void(SoundPtr)>& callback);
        void LoadShaderProgramAsync(const std::string& id, const std::string& vertexPath, const std::string& fragmentPath, const std::function<void(GLuint)>& callback);

        ModelPtr    GetOrLoadModelSync(const std::string& id, const std::string& filepath);
        MaterialPtr GetOrLoadMaterialSync(const std::string& id, const std::string& filepath);
        SoundPtr    GetOrLoadSoundSync(const std::string& id, const std::string& filepath);
        GLuint      GetOrLoadShaderProgramSync(const std::string& id, const std::string& vertexPath, const std::string& fragmentPath);

        void AddRenderMesh(const std::string& id, const RenderMesh& mesh);
        std::vector<RenderMesh> GetRenderMeshes(const std::string& id) const;
        void RemoveRenderMeshes(const std::string& id);

        void UnloadModel(const std::string& id);
        void UnloadMaterial(const std::string& id);
        void UnloadSound(const std::string& id);
        void UnloadShaderProgram(const std::string& id);

        bool HasModel(const std::string& id) const;
        bool HasMaterial(const std::string& id) const;
        bool HasSound(const std::string& id) const;
        bool HasShaderProgram(const std::string& id) const;

        void Update();

    private:
        ThreadingSystem& m_threadSystem;

        mutable std::mutex m_queueMutex;
        struct MainThreadTask { std::function<void()> fn; };
        std::vector<MainThreadTask> m_mainThreadTasks;

        mutable std::mutex m_modelMutex;
        std::unordered_map<std::string, ModelPtr> m_models;

        mutable std::mutex m_renderMeshMutex;
        std::unordered_map<std::string, std::vector<RenderMesh>> m_renderMeshes;

        mutable std::mutex m_materialMutex;
        std::unordered_map<std::string, MaterialPtr> m_materials;

        mutable std::mutex m_soundMutex;
        std::unordered_map<std::string, SoundPtr> m_sounds;

        mutable std::mutex m_shaderMutex;
        std::unordered_map<std::string, GLuint> m_shaderPrograms;

        static bool ReadFileToString(const std::string& filepath, std::string& out);
    };
}
