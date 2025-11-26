#include "Engine/Window.h"
#include "Renderer/Model.h"
#include "Renderer/Material.h"
#include "Renderer/Shader.h"
#include "Renderer/OpenGL.h"
#include "Renderer/Lighting.h"
#include "NodeGraph/Node.h"
#include "NodeGraph/Components.h"

#include <fstream>
#include <iostream>
#include <string>

using namespace BSE;

static std::string ReadFileToString(const std::string& path)
{
    std::ifstream in(path);
    if (!in.is_open()) return {};
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

int main(int argc, char** argv)
{
    std::string modelPath;
    std::string matPath;
    int width = 1280, height = 720;

    for (int i = 1; i < argc; ++i)
    {
        std::string a(argv[i]);
        if (a.rfind("-ModFile:", 0) == 0) modelPath = a.substr(9);
        else if (a.rfind("-MatFile:", 0) == 0) matPath = a.substr(9);
        else if (a.rfind("-width:", 0) == 0) width = std::stoi(a.substr(7));
        else if (a.rfind("-height:", 0) == 0) height = std::stoi(a.substr(8));
    }

    try
    {
        Window window("BSE Model Viewer", width, height, true, false);
        window.Create();

        std::string vertPath = "Extras/Shaders/PBRBasic.vert";
        std::string fragPath = "Extras/Shaders/PBRBasic.frag";

        std::string vertSrc = ReadFileToString(vertPath);
        std::string fragSrc = ReadFileToString(fragPath);

        if (vertSrc.empty() || fragSrc.empty())
        {
            std::cerr << "Failed to load PBRBasic shaders from '" << vertPath << "' or '" << fragPath << "'.\n";
            return 1;
        }

        Shader vert(vertSrc, ShaderType::Vertex);
        Shader frag(fragSrc, ShaderType::Fragment);
        auto program = std::make_shared<ShaderProgram>(vert, frag);

        auto material = std::make_shared<Material>();
        if (!matPath.empty())
        {
            if (!material->LoadFromFile(matPath))
            {
                std::cerr << "Failed to load material: " << matPath << " - using defaults\n";
            }
        }

        auto modelPtr = std::make_shared<Model>();
        if (!modelPath.empty())
        {
            if (!modelPtr->LoadFromFile(modelPath))
            {
                std::cerr << "Failed to load model: " << modelPath << "\n";
                return 1;
            }
        }
        else
        {
            std::cerr << "No model file provided. Use -ModFile:<path>\n";
            return 1;
        }

        ModelRenderer renderer;

        bool running = true;
        bool dragging = false;
        int lastX = 0, lastY = 0;

        float yaw = 0.0f;
        float pitch = 0.15f;
        float distance = 3.0f;
        glm::vec3 center(0.0f);

        const auto& meshes = modelPtr->GetMeshes();
        if (!meshes.empty())
        {
            glm::vec3 minp(FLT_MAX), maxp(-FLT_MAX);
            for (const auto& p : meshes[0].positions)
            {
                minp = glm::min(minp, p);
                maxp = glm::max(maxp, p);
            }
            center = (minp + maxp) * 0.5f;
            float radius = glm::length(maxp - center);
            if (radius > 0.001f) distance = radius * 2.0f;
        }

        Lighting::SetMode(Lighting::Mode::Unlit);
        Lighting::Clear();
        Lighting::SetAmbient(glm::vec3(1.0f), 1.0f);

        auto rootNode = std::make_shared<Node>("Root");
        auto camNode = std::make_shared<Node>("CameraNode");
        auto modelNode = std::make_shared<Node>("ModelNode");

        auto camCompUP = std::make_unique<Camera3DComponent>();
        Camera3DComponent* camComp = camCompUP.get();
        camComp->AspectRatio = (float)width / (float)height;
        camComp->FOV = 45.0f;
        camComp->NearPlane = 0.01f;
        camComp->FarPlane = 1000.0f;
        camNode->AddComponent(std::move(camCompUP), "Camera3D");

        struct ViewerModelComponent : public Component
        {
            std::shared_ptr<Model> model;
            std::shared_ptr<Material> mat;
            std::shared_ptr<ShaderProgram> prog;
            ModelRenderer& renderer;
            glm::mat4 viewProj = glm::mat4(1.0f);
            glm::vec3 cameraPos = glm::vec3(0.0f);

            ViewerModelComponent(ModelRenderer& r) : renderer(r) {}
            virtual void Update(double Tick) override
            {
                if (model) model->UpdateRenderTransforms();
            }
            virtual void Render(double Alpha) override
            {
                if (!prog || !mat || !model) return;
                prog->Bind();
                GLint locCam = glGetUniformLocation(prog->GetID(), "uCameraPos");
                if (locCam >= 0) glUniform3fv(locCam, 1, &cameraPos[0]);
                mat->Bind(prog->GetID());
                if (Lighting::ShaderUsesLighting(prog->GetID()))
                    Lighting::Apply(prog->GetID());
                model->Render(renderer, viewProj, prog->GetID());
                prog->Unbind();
            }
        };

        auto vmcUP = std::make_unique<ViewerModelComponent>(renderer);
        ViewerModelComponent* vmc = vmcUP.get();
        vmc->model = modelPtr;
        vmc->mat = material;
        vmc->prog = program;
        modelNode->AddComponent(std::move(vmcUP), "ViewerModel");

        rootNode->AddChild(camNode);
        rootNode->AddChild(modelNode);

        while (running && window.IsOpen())
        {
            // Was faster at the time to use SDL events directly than the Input System while making this
            SDL_Event ev;
            while (SDL_PollEvent(&ev))
            {
                if (ev.type == SDL_EVENT_QUIT) { running = false; }
                else if (ev.type == SDL_EVENT_MOUSE_WHEEL)
                {
                    float wheel = (float)ev.wheel.y;
                    if (wheel > 0) distance *= 0.85f;
                    else if (wheel < 0) distance *= 1.15f;
                    distance = glm::clamp(distance, 0.01f, 1000.0f);
                }
                else if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
                {
                    if (ev.button.button == SDL_BUTTON_LEFT)
                    {
                        dragging = true;
                        lastX = ev.button.x;
                        lastY = ev.button.y;
                    }
                }
                else if (ev.type == SDL_EVENT_MOUSE_BUTTON_UP)
                {
                    if (ev.button.button == SDL_BUTTON_LEFT) dragging = false;
                }
                else if (ev.type == SDL_EVENT_MOUSE_MOTION)
                {
                    if (dragging)
                    {
                        int dx = ev.motion.x - lastX;
                        int dy = ev.motion.y - lastY;
                        lastX = ev.motion.x;
                        lastY = ev.motion.y;

                        float sens = 0.005f;
                        yaw -= dx * sens;
                        pitch += dy * sens;
                        pitch = glm::clamp(pitch, -1.49f, 1.49f);
                    }
                }
            }


            glm::vec3 camDir;
            camDir.x = cosf(pitch) * sinf(yaw);
            camDir.y = sinf(pitch);
            camDir.z = cosf(pitch) * cosf(yaw);
            glm::vec3 camPos = center + camDir * distance;

            camComp->Position = camPos;
            glm::vec3 forward = glm::normalize(center - camPos);
            camComp->Pitch = glm::degrees(asin(glm::clamp(forward.y, -1.0f, 1.0f)));
            camComp->Yaw = glm::degrees(atan2(forward.z, forward.x));
            camComp->AspectRatio = (float)width / (float)height;
            camComp->Update(0.0);

            vmc->viewProj = camComp->GetViewProjMatrix();
            vmc->cameraPos = camComp->Position;

            GL::ClearBuffers();

            rootNode->UpdateNode(0.0);
            rootNode->RenderNode(0.0);

            window.SwapBuffers();
            SDL_Delay(1);
        }

        window.Destroy();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Exception: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
