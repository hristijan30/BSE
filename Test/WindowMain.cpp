#include "../Engine/Window.h"
#include "../Engine/Renderer/Shader.h"
#include "../Engine/Time.h"
#include "../Engine/Renderer/Camera.h"

#if defined(_WIN32)
#include <windows.h>
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int main(int argc, char* argv[])
#endif
{
    try
    {
        BSE::Window window("OpenGL 4.6 Triangle Camera Test", 800, 600);
        window.Create();

        glEnable(GL_DEPTH_TEST);

        // Triangle vertices (position + color)
        float vertices[] = {
            0.0f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
           -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
            0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f
        };

        GLuint VAO, VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // Shaders
        const std::string vertexSrc = R"(
            #version 460 core
            layout(location = 0) in vec3 aPos;
            layout(location = 1) in vec3 aColor;
            uniform mat4 uMVP;
            out vec3 vColor;
            void main() {
                vColor = aColor;
                gl_Position = uMVP * vec4(aPos, 1.0);
            }
        )";

        const std::string fragmentSrc = R"(
            #version 460 core
            in vec3 vColor;
            out vec4 FragColor;
            void main() {
                FragColor = vec4(vColor, 1.0);
            }
        )";

        BSE::Shader vertexShader(vertexSrc, BSE::ShaderType::Vertex);
        BSE::Shader fragmentShader(fragmentSrc, BSE::ShaderType::Fragment);
        BSE::ShaderProgram shader(vertexShader, fragmentShader);

        // Time and camera
        BSE::Time time(60.0);
        BSE::Camera camera(time, glm::vec3(0.0f, 0.0f, 3.0f));

        bool running = true;
        SDL_Event event;

        while (running && window.IsOpen())
        {
            time.Update();
            float deltaTime = static_cast<float>(time.GetDeltaTime());

            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_EVENT_QUIT)
                    running = false;
            }

            // Keyboard input
            const bool* state = SDL_GetKeyboardState(NULL);

            float moveSpeed = 5.0f;    // units per second
            float rotateSpeed = 90.0f; // degrees per second

            if (state[SDL_SCANCODE_W]) camera.MoveForward(moveSpeed * deltaTime);
            if (state[SDL_SCANCODE_S]) camera.MoveForward(-moveSpeed * deltaTime);
            if (state[SDL_SCANCODE_A]) camera.MoveRight(-moveSpeed * deltaTime);
            if (state[SDL_SCANCODE_D]) camera.MoveRight(moveSpeed * deltaTime);
            if (state[SDL_SCANCODE_Q]) camera.Rotate(-rotateSpeed * deltaTime, 0.0f);
            if (state[SDL_SCANCODE_E]) camera.Rotate(rotateSpeed * deltaTime, 0.0f);

            // Render
            glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shader.Bind();

            glm::mat4 model = glm::mat4(1.0f);
            glm::mat4 view = camera.GetViewMatrix();
            glm::mat4 proj = camera.GetProjectionMatrix(800.0f / 600.0f);
            glm::mat4 mvp = proj * view * model;

            GLuint loc = glGetUniformLocation(shader.GetID(), "uMVP");
            glUniformMatrix4fv(loc, 1, GL_FALSE, &mvp[0][0]);

            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);
            shader.Unbind();

            window.SwapBuffers();
        }

        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        window.Destroy();
    }
    catch (const std::exception& e)
    {
#if defined(_WIN32)
        MessageBoxA(NULL, e.what(), "Fatal Error", MB_ICONERROR | MB_OK);
#else
        std::cerr << "Error: " << e.what() << std::endl;
#endif
        return -1;
    }

    return 0;
}
