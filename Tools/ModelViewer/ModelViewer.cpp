#include "Engine/Window.h"
#include "Renderer/Model.h"
#include "Renderer/Material.h"
#include "Renderer/Shader.h"
#include "Renderer/OpenGL.h"

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
        ShaderProgram program(vert, frag);

        Material material;
        if (!matPath.empty())
        {
            if (!material.LoadFromFile(matPath))
            {
                std::cerr << "Failed to load material: " << matPath << " - using defaults\n";
            }
        }

        Model model;
        if (!modelPath.empty())
        {
            if (!model.LoadFromFile(modelPath))
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

        const auto& meshes = model.GetMeshes();
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

        while (running && window.IsOpen())
        {
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

            glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.01f, 1000.0f);
            glm::mat4 view = glm::lookAt(camPos, center, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 viewProj = proj * view;

            GL::ClearBuffers();

            program.Bind();

            GLint locCam = glGetUniformLocation(program.GetID(), "uCameraPos");
            if (locCam >= 0) glUniform3fv(locCam, 1, &camPos[0]);

            material.Bind(program.GetID());

            model.Render(renderer, viewProj, program.GetID());

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
