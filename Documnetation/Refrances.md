# Engine.h

**Namespace:** `BSE`  
**Header file path:** `Engine.h`  

## Overview
The `Engine` class is the central core of the BSE engine. It provides basic system-level functionality, including frame and tick rate detection, as well as access to system hardware information.  

## Classes

### `Engine`
A lightweight core engine class that encapsulates essential runtime information and utility functions.

#### Public Methods

| Method | Description |
|--------|-------------|
| `Engine()` | Default constructor. Initializes the engine instance. |
| `void DetectFrameAndTickRates(BSE::Time& time)` | Detects and updates the current frame and tick rates. The `BSE::Time` object is used to store or compute time-related data. |
| `static unsigned int GetCPUThreadCount()` | Returns the number of CPU threads available on the system. |
| `static uint64_t GetTotalRAM()` | Returns the total RAM available on the system in bytes. |
| `static uint64_t GetAvailableRAM()` | Returns the currently available RAM in bytes. |

## Notes
- The class is lightweight and primarily designed for querying system information and time-related metrics.  
- All methods are non-blocking and thread-safe for querying hardware capabilities.

# Time.h

**Namespace:** `BSE`  
**Header file path:** `Time.h`  

## Overview
The `Time` class provides tick-based and frame-based timing for the engine. It supports fixed-step updates for logic (ticks) and interpolated updates for rendering (alpha blending).  

## Classes

### `Time`
Handles time measurement and tick management for the engine loop.

#### Public Methods

| Method | Description |
|--------|-------------|
| `Time(double tickRate = 60.0)` | Constructor. Initializes the time system with a given tick rate (ticks per second). Defaults to 60 ticks/sec. |
| `void Update()` | Updates the internal time counters and computes delta time since the last update. |
| `bool ShouldTick() const` | Returns `true` if enough time has passed to perform a logic tick. |
| `void ConsumeTick()` | Consumes one tick, decrementing the internal accumulator. |
| `double GetDeltaTime() const` | Returns the delta time (frame time) since the last update. |
| `double GetTimePerTick() const` | Returns the fixed time per tick based on the tick rate. |
| `double GetAlpha() const` | Returns the interpolation factor (alpha) for rendering between ticks. Useful for smooth graphics updates. |

## Notes
- `Time` uses a high-resolution clock from `std::chrono` for accurate timing.  
- Supports both logic updates at fixed ticks and smooth rendering using alpha interpolation.  
- Essential for synchronizing frame updates and game logic.  

# Window.h

**Namespace:** `BSE`  
**Header file path:** `Window.h`  

## Overview
The `Window` class provides a wrapper around SDL and OpenGL to create and manage an application window. It handles window creation, destruction, and OpenGL context management.

## Classes

### `Window`
Encapsulates a window with OpenGL context and basic properties.

#### Public Methods

| Method | Description |
|--------|-------------|
| `Window(const char* title, int width, int height, bool resizable = true, bool fullscreen = false)` | Constructor. Sets up window parameters such as title, dimensions, resizable flag, and fullscreen mode. |
| `~Window()` | Destructor. Cleans up window resources. |
| `void Create()` | Creates the SDL window and initializes the OpenGL context. |
| `void Destroy()` | Destroys the SDL window and deletes the OpenGL context. |
| `void SwapBuffers()` | Swaps the front and back buffers (double buffering) for rendering. |
| `bool IsOpen() const` | Returns `true` if the window is successfully created. Typically used in the main loop, e.g., `while (running && window.IsOpen())`. |
| `SDL_Window* GetSDLWindow() const` | Returns a pointer to the underlying SDL window. |

## Notes
- Internally uses `SDL_Window*` and `SDL_GLContext` to manage the OpenGL context.  
- Provides options for resizable or fullscreen windows.  
- `IsOpen()` is primarily used to control the main application loop.

# Logger.h

**Namespace:** `BSE`  
**Header file path:** `Logger.h`  

## Overview
The `Logger` class provides logging functionality for the engine. It supports logging messages, warnings, and errors, and maintains an internal cache of all log entries.

## Classes

### `Logger`
Static class for engine-wide logging.

#### Public Methods

| Method | Description |
|--------|-------------|
| `static void Initialize(const std::string& filename)` | Initializes the logger and opens a file to store log messages. |
| `static void LogMessage(const std::string& text)` | Logs a standard message. |
| `static void LogWarning(const std::string& text)` | Logs a warning message. |
| `static void LogError(const std::string& text)` | Logs an error message. |
| `static const std::vector<std::string>& GetCachedLogs()` | Retrieves a reference to the internal log cache. |

## Notes
- All methods are static; no instance of `Logger` is required.  
- Designed to be thread-safe for use across engine systems.  
- Useful for debugging, runtime consoles, or file-based log tracking.

# FileSystem.h

**Namespace:** `BSE`  
**Header file path:** `FileSystem.h`  

## Overview
The `FileStreamingSystem` class provides asynchronous and synchronous loading of core engine assets such as models, materials, sounds, and shaders. It also supports caching of loaded resources, and allows adding or retrieving `RenderMesh` data for rendering purposes.

## Type Aliases
| Alias | Type | Description |
|-------|------|-------------|
| `ModelPtr` | `std::shared_ptr<ModelLoader>` | Shared pointer to a loaded model. |
| `MaterialPtr` | `std::shared_ptr<Material>` | Shared pointer to a loaded material. |
| `SoundPtr` | `std::shared_ptr<SoundBuffer>` | Shared pointer to a loaded sound buffer. |

## Classes

### `FileStreamingSystem`

#### Constructor / Destructor
| Method | Description |
|--------|-------------|
| `explicit FileStreamingSystem(ThreadingSystem& threadSystem)` | Creates a new file streaming system, using the provided `ThreadingSystem` for asynchronous tasks. |
| `~FileStreamingSystem()` | Destructor. Cleans up any remaining resources. |

#### Asynchronous Loading
| Method | Description |
|--------|-------------|
| `void LoadModelAsync(const std::string& id, const std::string& filepath, const std::function<void(ModelPtr)>& callback)` | Loads a model asynchronously and calls the callback with the loaded `ModelPtr`. |
| `void LoadMaterialAsync(const std::string& id, const std::string& filepath, const std::function<void(MaterialPtr)>& callback)` | Loads a material asynchronously and calls the callback with the loaded `MaterialPtr`. |
| `void LoadSoundAsync(const std::string& id, const std::string& filepath, const std::function<void(SoundPtr)>& callback)` | Loads a sound asynchronously and calls the callback with the loaded `SoundPtr`. |
| `void LoadShaderProgramAsync(const std::string& id, const std::string& vertexPath, const std::string& fragmentPath, const std::function<void(GLuint)>& callback)` | Loads a shader program asynchronously and calls the callback with the compiled OpenGL program ID. |

#### Synchronous Loading / Retrieval
| Method | Description |
|--------|-------------|
| `ModelPtr GetOrLoadModelSync(const std::string& id, const std::string& filepath)` | Retrieves a cached model by ID or loads it synchronously if not cached. |
| `MaterialPtr GetOrLoadMaterialSync(const std::string& id, const std::string& filepath)` | Retrieves a cached material by ID or loads it synchronously if not cached. |
| `SoundPtr GetOrLoadSoundSync(const std::string& id, const std::string& filepath)` | Retrieves a cached sound by ID or loads it synchronously if not cached. |
| `GLuint GetOrLoadShaderProgramSync(const std::string& id, const std::string& vertexPath, const std::string& fragmentPath)` | Retrieves a cached shader program by ID or compiles it synchronously if not cached. |

#### Render Mesh Management
| Method | Description |
|--------|-------------|
| `void AddRenderMesh(const std::string& id, const RenderMesh& mesh)` | Adds a `RenderMesh` to the internal cache under the given ID. |
| `std::vector<RenderMesh> GetRenderMeshes(const std::string& id) const` | Retrieves all `RenderMesh` objects associated with the given ID. |
| `void RemoveRenderMeshes(const std::string& id)` | Removes all `RenderMesh` objects for the given ID. |

#### Resource Unloading
| Method | Description |
|--------|-------------|
| `void UnloadModel(const std::string& id)` | Removes the cached model for the given ID. |
| `void UnloadMaterial(const std::string& id)` | Removes the cached material for the given ID. |
| `void UnloadSound(const std::string& id)` | Removes the cached sound for the given ID. |
| `void UnloadShaderProgram(const std::string& id)` | Removes the cached shader program for the given ID. |

#### Resource Checks
| Method | Description |
|--------|-------------|
| `bool HasModel(const std::string& id) const` | Checks if a model with the given ID is cached. |
| `bool HasMaterial(const std::string& id) const` | Checks if a material with the given ID is cached. |
| `bool HasSound(const std::string& id) const` | Checks if a sound with the given ID is cached. |
| `bool HasShaderProgram(const std::string& id) const` | Checks if a shader program with the given ID is cached. |

#### Update
| Method | Description |
|--------|-------------|
| `void Update()` | Processes completed asynchronous tasks and executes callbacks on the main thread. |

## Notes
- Designed to be thread-safe: multiple mutexes protect access to individual resource caches.  
- Supports both immediate (synchronous) loading and background (asynchronous) loading using the `ThreadingSystem`.  
- Can store multiple `RenderMesh` objects under the same ID, useful for pre-processed meshes or LODs.  
- Shader programs are stored as OpenGL program IDs, while models, materials, and sounds are stored as shared pointers.

# ThreadingSystem.h

**Namespace:** `BSE`  
**Header file path:** `Threading/ThreadingSystem.h`  

## Overview
The `ThreadingSystem` class provides a lightweight multithreading utility for submitting tasks asynchronously and managing completed tasks. It is built on Intel TBB's `task_group` for efficient parallel execution.

## Classes

### `ThreadingSystem`

#### Constructor / Destructor
| Method | Description |
|--------|-------------|
| `ThreadingSystem()` | Default constructor. Initializes the threading system. |
| `~ThreadingSystem()` | Destructor. Ensures that all tasks are completed before destruction. |

#### Task Submission
| Method | Description |
|--------|-------------|
| `template<typename Func> void SubmitTask(Func&& task)` | Submits a callable task to be executed asynchronously. Uses TBB's `task_group` internally. |

#### Task Synchronization
| Method | Description |
|--------|-------------|
| `void WaitAll()` | Waits for all currently submitted tasks to complete. |

#### Main Thread Task Management
| Method | Description |
|--------|-------------|
| `void AddCompletedTask(const std::function<void()>& task)` | Adds a task to be executed later on the main thread, typically for tasks that require main-thread context (e.g., OpenGL resource updates). |
| `std::vector<std::function<void()>> RetrieveCompletedTasks()` | Retrieves and clears all tasks queued for main-thread execution. |

## Notes
- `SubmitTask` is thread-safe and allows for easy parallelization of workloads.  
- `AddCompletedTask` + `RetrieveCompletedTasks` provides a way to safely hand off tasks from worker threads to the main thread.  
- Internally uses a mutex `m_queueMutex` to protect access to `m_completedTasks`.  
- Designed for integration with asset streaming, physics updates, or any CPU-intensive tasks that benefit from multithreading.

# Sound.h

**Namespace:** `BSE`  
**Header file path:** `Sound/Sound.h`  

## Overview
This header provides classes for managing audio using OpenAL. It includes `SoundBuffer` for storing audio data and `SoundSource` for playing sounds in 3D space with configurable properties like gain, pitch, and position.

## Classes

### `SoundBuffer`

Represents an audio buffer storing sound data.

#### Methods
| Method | Description |
|--------|-------------|
| `SoundBuffer()` | Initializes a new audio buffer. |
| `~SoundBuffer()` | Releases the OpenAL buffer resources. |
| `bool LoadFromFile(const std::string& filepath)` | Loads audio data from a file into the buffer. |
| `void SetData(const void* data, ALsizei size, ALsizei freq, ALenum format)` | Directly sets audio data into the buffer. |
| `ALuint GetID() const` | Returns the OpenAL buffer ID. |

---

### `SoundSource`

Represents an audio source that can play `SoundBuffer` data with configurable properties.

#### Methods
| Method | Description |
|--------|-------------|
| `SoundSource()` | Initializes a new sound source. |
| `~SoundSource()` | Releases the OpenAL source resources. |
| `void AttachBuffer(const SoundBuffer& buffer)` | Attaches a `SoundBuffer` to the source. |
| `void Play()` | Starts playback of the attached buffer. |
| `void Pause()` | Pauses playback. |
| `void Stop()` | Stops playback. |
| `void SetLooping(bool loop)` | Sets whether the sound should loop. |
| `void SetGain(float gain)` | Sets the volume of the source. |
| `void SetPitch(float pitch)` | Sets the pitch multiplier for playback. |
| `void SetPosition(const glm::vec3& pos)` | Sets the 3D position of the source. |
| `void SetVelocity(const glm::vec3& vel)` | Sets the velocity for Doppler effects. |
| `bool IsPlaying() const` | Returns `true` if the source is currently playing. |
| `ALuint GetID() const` | Returns the OpenAL source ID. |

## Notes
- `SoundBuffer` and `SoundSource` are designed to work together: buffers store the audio, sources play it.  
- Supports 3D positional audio via `SetPosition` and `SetVelocity`.  
- Gain, pitch, and looping are configurable per source for flexible audio playback.

# SoundManager.h

**Namespace:** `BSE`  
**Header file path:** `Sound/SoundManager.h`  

## Overview
This header provides the `SoundManager` class, which manages the OpenAL audio context and listener properties for 3D audio in the engine.

## Classes

### `SoundManager`

Manages the OpenAL device, context, and listener attributes.

#### Methods
| Method | Description |
|--------|-------------|
| `SoundManager()` | Initializes the manager object (does not initialize OpenAL). |
| `~SoundManager()` | Cleans up any remaining OpenAL resources. |
| `bool Initialize()` | Initializes the OpenAL device and context. Must be called before playing any sounds. |
| `void Shutdown()` | Shuts down the OpenAL device and context, freeing resources. |
| `void SetListenerPosition(const glm::vec3& pos)` | Sets the 3D position of the listener. |
| `void SetListenerOrientation(const glm::vec3& forward, const glm::vec3& up)` | Sets the orientation of the listener in 3D space. |
| `void SetListenerVelocity(const glm::vec3& vel)` | Sets the velocity of the listener for Doppler effects. |

## Notes
- This class handles the global listener for the audio scene.  
- The listener defines the point of view for 3D sound positioning, affecting how all `SoundSource` objects are heard.  
- Must initialize before creating or playing `SoundSource` instances.

# Camera.h

**Namespace:** `BSE`  
**Header file path:** `Renderer/Camera.h`  

## Overview
This header provides classes for different types of cameras used in the engine, including perspective, orthographic, and core camera handling. These classes manage view and projection matrices, camera movement, and orientation.

## Classes

### `CoreCamera`

Basic camera class storing position and orientation vectors and generating view and projection matrices.

#### Constructor
| Method | Description |
|--------|-------------|
| `CoreCamera(const glm::vec3& position = glm::vec3(0.0f), const glm::vec3& forward = glm::vec3(0.0f, 0.0f, -1.0f), const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f)))` | Initializes the camera with position, forward, and up vectors. |

#### Methods
| Method | Description |
|--------|-------------|
| `glm::mat4 GetViewMatrix() const` | Returns the camera's view matrix based on position and orientation. |
| `glm::mat4 GetProjectionMatrix(float aspectRatio, float fovY = 45.0f, float nearPlane = 0.1f, float farPlane = 100.0f) const` | Returns a perspective projection matrix using the given parameters. |

#### Attributes
| Attribute | Description |
|-----------|-------------|
| `glm::vec3 Position` | Camera position in world space. |
| `glm::vec3 Forward` | Forward direction vector. |
| `glm::vec3 Up` | Up direction vector. |
| `glm::vec3 Right` | Right direction vector (computed from forward and up). |
| `float FOV` | Field of view in degrees. |

---

### `Camera`

A more advanced perspective camera that supports movement and rotation, tracking time for smooth updates.

#### Constructor
| Method | Description |
|--------|-------------|
| `Camera(BSE::Time& time, const glm::vec3& position = glm::vec3(0.0f, 0.0f, 3.0f), const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f, float pitch = 0.0f)` | Initializes the camera with position, up vector, yaw, pitch, and a reference to the engine time system. |

#### Methods
| Method | Description |
|--------|-------------|
| `glm::mat4 GetViewMatrix() const` | Returns the view matrix for the camera. |
| `glm::mat4 GetProjectionMatrix(float aspectRatio, float nearPlane = 0.1f, float farPlane = 100.0f) const` | Returns the perspective projection matrix. |
| `void MoveForward(float delta)` | Moves the camera forward/backward. |
| `void MoveRight(float delta)` | Moves the camera right/left. |
| `void MoveUp(float delta)` | Moves the camera up/down. |
| `void Rotate(float yawOffset, float pitchOffset, bool constrainPitch = true)` | Rotates the camera by yaw and pitch offsets. |
| `void SetFOV(float fov)` | Sets the camera's field of view. |

#### Attributes
| Attribute | Description |
|-----------|-------------|
| `glm::vec3 Position` | Camera position in world space. |
| `glm::vec3 Forward` | Forward direction vector. |
| `glm::vec3 Up` | Up direction vector. |
| `glm::vec3 Right` | Right direction vector. |
| `float Yaw` | Yaw angle in degrees. |
| `float Pitch` | Pitch angle in degrees. |
| `float FOV` | Field of view in degrees. |

---

### `OrthographicCamera`

Camera class for orthographic projection.

#### Constructor
| Method | Description |
|--------|-------------|
| `OrthographicCamera(float left, float right, float bottom, float top, float nearPlane = -1.0f, float farPlane = 1.0f, const glm::vec3& position = glm::vec3(0.0f))` | Initializes the camera with orthographic bounds and optional position. |

#### Methods
| Method | Description |
|--------|-------------|
| `glm::mat4 GetViewMatrix() const` | Returns the orthographic view matrix. |
| `glm::mat4 GetProjectionMatrix() const` | Returns the orthographic projection matrix. |
| `void SetProjection(float left, float right, float bottom, float top, float nearPlane = -1.0f, float farPlane = 1.0f)` | Updates the orthographic projection parameters. |

#### Attributes
| Attribute | Description |
|-----------|-------------|
| `glm::vec3 Position` | Camera position in world space. |
| `glm::vec3 Forward` | Forward direction vector. |
| `glm::vec3 Up` | Up direction vector. |

## Notes
- `CoreCamera` is a minimal base camera.  
- `Camera` handles free movement in 3D space with rotation and field of view adjustments.  
- `OrthographicCamera` is used for 2D or UI rendering with a fixed projection volume.

# Model.h

**Namespace:** `BSE`  
**Header file path:** `Renderer/Model.h`  

## Overview
This header provides classes for handling 3D models, including mesh data, model loading, processing, and rendering. It allows per-mesh transformations, overall model transformations, and integration with OpenGL rendering.

## Classes

### `MeshData`

Stores the data for a single mesh, including vertices, normals, UVs, indices, and transform information.

#### Methods
| Method | Description |
|--------|-------------|
| `glm::mat4 GetLocalTRS() const` | Returns the local transformation matrix based on the mesh's position, rotation, and scale. |
| `glm::mat4 GetFinalTransform(const glm::mat4& modelTRS) const` | Returns the final transformation matrix combining the model transform, mesh transform, and local TRS. |

#### Attributes
| Attribute | Description |
|-----------|-------------|
| `std::string name` | Name of the mesh. |
| `glm::mat4 transform` | Local transform of the mesh relative to the model. |
| `glm::vec3 Position` | Position of the mesh. |
| `glm::quat Rotation` | Rotation of the mesh. |
| `glm::vec3 Scale` | Scale of the mesh. |
| `std::vector<glm::vec3> positions` | Vertex positions. |
| `std::vector<glm::vec3> normals` | Vertex normals. |
| `std::vector<glm::vec2> uvs` | Vertex texture coordinates. |
| `std::vector<uint32_t> indices` | Triangle indices. |

---

### `RenderMesh`

Stores GPU data for rendering a mesh.

#### Attributes
| Attribute | Description |
|-----------|-------------|
| `GLuint VAO` | Vertex Array Object ID. |
| `GLuint VBO` | Vertex Buffer Object ID. |
| `GLuint EBO` | Element Buffer Object ID. |
| `uint32_t indexCount` | Number of indices. |
| `glm::mat4 transform` | Mesh transform for rendering. |

---

### `ModelLoader`

Loads 3D model files and manages the collection of `MeshData`.

#### Methods
| Method | Description |
|--------|-------------|
| `ModelLoader()` | Default constructor. |
| `~ModelLoader()` | Destructor that calls `Unload()`. |
| `bool Load(const std::string& filepath)` | Loads a model from a file. |
| `void Unload()` | Frees all loaded meshes. |
| `const std::vector<MeshData>& GetMeshes() const` | Returns a const reference to the loaded meshes. |
| `std::vector<MeshData>& GetMeshesMutable()` | Returns a mutable reference to the loaded meshes. |

---

### `ModelProcessor`

Processes mesh data into GPU-ready render meshes.

#### Methods
| Method | Description |
|--------|-------------|
| `ModelProcessor()` | Default constructor. |
| `~ModelProcessor()` | Destructor that calls `Release()`. |
| `void Process(const std::vector<MeshData>& meshes)` | Converts `MeshData` into `RenderMesh` objects. |
| `void Release()` | Frees processed render meshes. |
| `const std::vector<RenderMesh>& GetRenderMeshes() const` | Returns a const reference to render meshes. |
| `std::vector<RenderMesh>& GetRenderMeshesMutable()` | Returns a mutable reference to render meshes. |

---

### `ModelRenderer`

Handles rendering of processed meshes using OpenGL.

#### Methods
| Method | Description |
|--------|-------------|
| `ModelRenderer()` | Default constructor. |
| `void Render(const std::vector<RenderMesh>& meshes, const glm::mat4& viewProjMatrix, GLuint shaderProgram)` | Renders the given meshes using the specified view-projection matrix and shader program. |

---

### `Model`

High-level model class combining loading, processing, per-mesh transformations, and rendering.

#### Methods
| Method | Description |
|--------|-------------|
| `Model()` | Default constructor. |
| `~Model()` | Destructor that calls `Unload()`. |
| `bool LoadFromFile(const std::string& filepath)` | Loads a model from a file. |
| `void Unload()` | Frees all resources associated with the model. |
| `void SetPosition(const glm::vec3& pos)` | Sets the model position. |
| `void SetRotation(const glm::quat& rot)` | Sets the model rotation. |
| `void SetScale(const glm::vec3& scale)` | Sets the model scale. |
| `void Translate(const glm::vec3& delta)` | Translates the model by a delta vector. |
| `void Rotate(const glm::quat& delta)` | Rotates the model by a quaternion delta. |
| `void Rescale(const glm::vec3& factor)` | Scales the model by a factor vector. |
| `glm::vec3 GetPosition() const` | Returns the model position. |
| `glm::quat GetRotation() const` | Returns the model rotation. |
| `glm::vec3 GetScale() const` | Returns the model scale. |
| `void SetMeshPosition(size_t meshIndex, const glm::vec3& pos)` | Sets position of a specific mesh. |
| `void TranslateMesh(size_t meshIndex, const glm::vec3& delta)` | Translates a specific mesh. |
| `void SetMeshRotation(size_t meshIndex, const glm::quat& rot)` | Sets rotation of a specific mesh. |
| `void RotateMesh(size_t meshIndex, const glm::quat& delta)` | Rotates a specific mesh. |
| `void SetMeshScale(size_t meshIndex, const glm::vec3& scale)` | Sets scale of a specific mesh. |
| `void RescaleMesh(size_t meshIndex, const glm::vec3& factor)` | Scales a specific mesh by a factor. |
| `void UpdateRenderTransforms()` | Updates the transforms of all render meshes based on model and mesh transforms. |
| `void Render(ModelRenderer& renderer, const glm::mat4& viewProjMatrix, GLuint shaderProgram)` | Renders the model using the given renderer, view-projection matrix, and shader. |
| `const std::vector<MeshData>& GetMeshes() const` | Returns the model’s meshes. |
| `const std::vector<RenderMesh>& GetRenderMeshes() const` | Returns the processed render meshes. |

## Notes
- `MeshData` stores individual mesh information, while `RenderMesh` stores GPU-ready data.  
- `ModelLoader` handles file input, `ModelProcessor` converts meshes for rendering, and `ModelRenderer` draws them.  
- `Model` provides a high-level interface for transformations and rendering of complete models.

# Shader.h

**Namespace:** `BSE`  
**Header file path:** `Renderer/Shader.h`  

## Overview
This header provides classes for handling OpenGL shaders, including individual shader objects, shader programs, and compute shader programs. It includes utilities for setting uniforms and managing GPU resources.

## Enums

### `ShaderType`

Specifies the type of shader.

| Enum Value | Description |
|------------|-------------|
| `Vertex` | Vertex shader. |
| `Fragment` | Fragment (pixel) shader. |
| `Geometry` | Geometry shader. |
| `Compute` | Compute shader. |
| `TessControl` | Tessellation control shader. |
| `TessEval` | Tessellation evaluation shader. |

#### Utility
| Function | Description |
|----------|-------------|
| `GLenum ShaderTypeToGLenum(ShaderType type)` | Converts a `ShaderType` enum to the corresponding OpenGL shader type. |

---

### `Shader`

Represents a single compiled GPU shader.

#### Methods
| Method | Description |
|--------|-------------|
| `Shader(const std::string& source, ShaderType type)` | Compiles a shader from source code with the specified type. |
| `~Shader()` | Deletes the shader from GPU memory. |
| `GLuint GetID() const` | Returns the OpenGL ID of the shader. |

---

### `ShaderProgram`

Manages an OpenGL shader program composed of multiple shaders.

#### Methods
| Method | Description |
|--------|-------------|
| `ShaderProgram(const Shader& vertex, const Shader& fragment, const Shader* geometry = nullptr, const Shader* tessControl = nullptr, const Shader* tessEval = nullptr)` | Links shaders into a program. Optional geometry and tessellation shaders can be included. |
| `~ShaderProgram()` | Deletes the program from GPU memory. |
| `void Bind() const` | Binds the shader program for rendering. |
| `void Unbind() const` | Unbinds the shader program. |
| `void SetUniform(const std::string& name, int value) const` | Sets an integer uniform variable. |
| `void SetUniform(const std::string& name, float value) const` | Sets a float uniform variable. |
| `void SetUniform(const std::string& name, const glm::mat4& matrix) const` | Sets a `mat4` uniform variable. |
| `GLuint GetID() const` | Returns the OpenGL ID of the shader program. |
| `GLuint Release()` | Releases ownership of the shader program and returns its ID. |

---

### `ComputeShaderProgram`

Represents a compute shader program in OpenGL.

#### Methods
| Method | Description |
|--------|-------------|
| `ComputeShaderProgram(const Shader& compute)` | Creates a compute shader program from a compiled compute shader. |
| `~ComputeShaderProgram()` | Deletes the compute program from GPU memory. |
| `void Bind() const` | Binds the compute shader program for execution. |
| `void Unbind() const` | Unbinds the compute shader program. |
| `void Dispatch(GLuint x, GLuint y, GLuint z) const` | Dispatches compute work groups of the specified size. |
| `GLuint GetID() const` | Returns the OpenGL ID of the compute shader program. |

## Notes
- `Shader` represents individual GPU shaders; `ShaderProgram` links them for rendering.  
- `ComputeShaderProgram` handles GPU compute operations separately.  
- Uniform setters allow sending data from CPU to GPU shaders efficiently.

# Texture2D.h

**Namespace:** `BSE`  
**Header file path:** `Renderer/Texture2D.h`  

## Overview
This header provides classes for handling 2D textures in OpenGL, including loading from files or memory, creating GPU textures, and binding/unbinding for rendering.

## Classes

### `ImageData`

Stores raw image data for texture creation.

#### Attributes
| Attribute | Description |
|-----------|-------------|
| `int width` | Width of the image in pixels. |
| `int height` | Height of the image in pixels. |
| `int channels` | Number of color channels (e.g., 3 for RGB, 4 for RGBA). |
| `std::vector<unsigned char> pixels` | Raw pixel data. |

---

### `Texture2D`

Represents a 2D GPU texture.

#### Methods
| Method | Description |
|--------|-------------|
| `Texture2D()` | Default constructor. |
| `explicit Texture2D(const std::string& path, bool srgb = true)` | Loads a texture from a file path. |
| `~Texture2D()` | Deletes the texture from GPU memory. |
| `static bool LoadImageToMemory(const std::string& path, ImageData& out, bool flipVertically = true)` | Loads an image file into CPU memory. |
| `bool CreateFromImageData(const ImageData& data, bool srgb = true)` | Creates a GPU texture from CPU image data. |
| `bool LoadFromFile(const std::string& path, bool srgb = true)` | Loads a texture from a file and uploads it to the GPU. |
| `void Bind(GLuint slot = 0) const` | Binds the texture to a specified texture slot. |
| `void Unbind() const` | Unbinds the texture. |
| `GLuint GetID() const` | Returns the OpenGL ID of the texture. |
| `int GetWidth() const` | Returns the width of the texture in pixels. |
| `int GetHeight() const` | Returns the height of the texture in pixels. |
| `bool IsLoaded() const` | Returns true if the texture is successfully loaded on the GPU. |

## Notes
- `ImageData` is used for intermediate storage of pixel data before uploading to the GPU.  
- `Texture2D` manages the OpenGL texture resource lifecycle, including creation, binding, and deletion.  
- Supports sRGB textures and optional vertical flipping when loading from files.

# Material.h

**Namespace:** `BSE`  
**Header file path:** `Renderer/Material.h`  

## Overview
This header provides the `Material` class, which manages material properties and associated textures for rendering. It supports loading from files, parsing material definitions, and binding textures to shaders.

## Classes

### `Material`

Represents a material with textures and shading properties.

#### Methods
| Method | Description |
|--------|-------------|
| `Material()` | Default constructor. |
| `~Material()` | Default destructor. |
| `bool LoadFromFile(const std::string& filepath)` | Loads a material definition from a file. |
| `bool ParseMaterialFile(const std::string& filepath)` | Parses a material file and sets texture paths and properties. |
| `void FinalizeTexturesFromImageData(const std::unordered_map<std::string, ImageData>& images)` | Creates `Texture2D` objects from preloaded `ImageData`. |
| `void Bind(GLuint shaderProgram) const` | Binds the material's textures to the specified shader program. |
| `const Texture2D* GetDiffuseMap() const` | Returns the diffuse texture. |
| `const Texture2D* GetNormalMap() const` | Returns the normal map texture. |
| `const Texture2D* GetRoughnessMap() const` | Returns the roughness map texture. |
| `const Texture2D* GetMetallicMap() const` | Returns the metallic map texture. |
| `const Texture2D* GetAOMap() const` | Returns the ambient occlusion map texture. |
| `const Texture2D* GetEmissiveMap() const` | Returns the emissive map texture. |

#### Attributes
| Attribute | Description |
|-----------|-------------|
| `glm::vec3 BaseColor` | Base albedo color of the material. |
| `glm::vec3 EmissionColor` | Emission color of the material. |
| `float Metallic` | Metallic property factor. |
| `float Roughness` | Roughness factor. |
| `float Transparency` | Transparency factor. |
| `float EmissionStrength` | Intensity of emission. |
| `float SpecularStrength` | Intensity of specular reflections. |
| `std::string diffusePath` | File path for the diffuse texture. |
| `std::string normalPath` | File path for the normal map. |
| `std::string roughnessPath` | File path for the roughness map. |
| `std::string metallicPath` | File path for the metallic map. |
| `std::string aoPath` | File path for the ambient occlusion map. |
| `std::string emissivePath` | File path for the emissive map. |

## Notes
- The `Material` class handles all textures required for physically-based rendering (PBR).  
- Texture maps include diffuse, normal, roughness, metallic, ambient occlusion (AO), and emissive.  
- Material properties like metallic, roughness, and specular strength are applied when rendering.  

# Node.h

**Namespace:** `BSE`  
**Header file path:** `NodeSystem/Node.h`  

## Overview
This header provides the `Node` class, which represents a hierarchical scene graph node. It supports parent-child relationships, updates, and rendering, and can be extended for custom behavior.

## Classes

### `Node`

Represents a single node in a scene graph hierarchy.

#### Methods
| Method | Description |
|--------|-------------|
| `Node(const std::string& name = "Node")` | Creates a node with an name. |
| `virtual ~Node()` | Virtual destructor to allow proper cleanup in derived classes. |
| `void SetParent(Node* parent)` | Sets the parent of the node. |
| `void AddChild(std::shared_ptr<Node> child)` | Adds a child node. |
| `void RemoveChild(std::shared_ptr<Node> child)` | Removes a child node. |
| `const std::vector<std::shared_ptr<Node>>& GetChildren() const` | Returns a list of the node's children. |
| `Node* GetParent() const` | Returns the parent node. |
| `const std::string& GetName() const` | Returns the node's name. |
| `virtual void Update(float tick)` | Called every tick to update the node; can be overridden. |
| `virtual void Render(float alpha)` | Called during rendering; can be overridden. |

## Notes
- `Node` is the base class for hierarchical scene objects.  
- Supports tree-like parent-child relationships, making it suitable for scene graphs.  
- Can be extended to implement custom update and render logic in derived classes.

# Scene.h

**Namespace:** `BSE`  
**Header file path:** `NodeSystem/Scene.h`  

## Overview
This header provides the `Scene` class, which represents a root node for a scene graph. It inherits from `Node` and allows updating and rendering the entire hierarchy.

## Classes

### `Scene`

Represents a scene containing a hierarchy of nodes.

#### Methods
| Method | Description |
|--------|-------------|
| `Scene(const std::string& name = "Scene")` | Creates a scene with an name. |
| `~Scene()` | Destructor. |
| `void UpdateAll(float tick)` | Updates this scene and all child nodes recursively. |
| `void RenderAll(float alpha)` | Renders this scene and all child nodes recursively. |

## Notes
- `Scene` is the root node for a scene graph and manages all child nodes.  
- `UpdateAll` and `RenderAll` provide a convenient way to process the entire node hierarchy.  
- Inherits all functionality from `Node`.

# RootNode.h

**Namespace:** `BSE`  
**Header file path:** `NodeSystem/RootNode.h`  

## Overview
This header provides the `RootNode` class, a minimal node that can be used as a base for custom nodes. It does not implement any scene graph management by itself and serves as a placeholder for user-defined behavior, such as cameras, lights, or meshes.

## Classes

### `RootNode`

A minimal node for custom extensions.

#### Methods
| Method | Description |
|--------|-------------|
| `RootNode(const std::string& name = "RootNode")` | Creates a root node with an optional name. |
| `~RootNode()` | Destructor. |

## Notes
- `RootNode` inherits from `Node` but does not add functionality by default.  
- Intended to serve as a starting point for custom nodes in a scene graph.  
- Users can extend this class to implement cameras, lights, meshes, or other scene objects.

# NodeManager.h

**Namespace:** `BSE`  
**Header file path:** `NodeSystem/NodeManager.h`  

## Overview
This header provides the `NodeManager` class, which manages multiple scenes and keeps track of the currently active scene. It allows creation, retrieval, and switching of scenes in a centralized way.

## Classes

### `NodeManager`

Manages a collection of scenes and the active scene.

#### Methods
| Method | Description |
|--------|-------------|
| `NodeManager()` | Default constructor. |
| `~NodeManager()` | Default destructor. |
| `std::shared_ptr<Scene> CreateScene(const std::string& name)` | Creates a new `Scene` with the specified name and adds it to the manager. |
| `void SetActiveScene(std::shared_ptr<Scene> scene)` | Sets the specified scene as the active scene. |
| `std::shared_ptr<Scene> GetActiveScene() const` | Returns the currently active scene. |
| `const std::vector<std::shared_ptr<Scene>>& GetScenes() const` | Returns a list of all managed scenes. |

## Notes
- `NodeManager` centralizes scene management in the engine.  
- Supports multiple scenes but tracks one active scene for updating and rendering.  
- Simplifies scene creation, switching, and access for higher-level systems.

# Key.h

**Namespace:** `BSE`  
**Header file path:** `Input/Key.h`  

## Overview
This header provides the `KeyCode` enum, which maps keyboard keys to SDL scancodes. It standardizes key input codes for use across the engine.

## Enums

### `KeyCode`

Represents keyboard keys using SDL scancodes.

| Enum Value | Description |
|------------|-------------|
| `UNKNOWN` | Unknown key. |
| `A` – `Z` | Alphabet keys A to Z. |
| `NUM_0` – `NUM_9` | Number keys 0 to 9. |
| `SPACE` | Spacebar. |
| `LSHIFT`, `RSHIFT` | Left and right Shift keys. |
| `LCTRL`, `RCTRL` | Left and right Control keys. |
| `LALT`, `RALT` | Left and right Alt keys. |
| `ESCAPE` | Escape key. |
| `ENTER` | Enter / Return key. |
| `TAB` | Tab key. |
| `BACKSPACE` | Backspace key. |
| `UP`, `DOWN`, `LEFT`, `RIGHT` | Arrow keys. |

## Notes
- `KeyCode` maps engine-independent key names to SDL scancodes.  
- Can be used to handle keyboard input consistently across the engine.  

# InputManager.h

**Namespace:** `BSE`  
**Header file path:** `Input/InputManager.h`  

## Overview
This header provides the `InputManager` class, which handles keyboard and mouse input. It tracks key states, mouse buttons, mouse position, and movement deltas.

## Classes

### `InputManager`

Manages input state for keyboard and mouse.

#### Methods
| Method | Description |
|--------|-------------|
| `InputManager()` | Initializes the input manager and internal state. |
| `~InputManager()` | Default destructor. |
| `void Update()` | Updates the current input state, must be called each frame. |
| `bool IsKeyDown(KeyCode key) const` | Returns true if the key is currently pressed. |
| `bool IsKeyPressed(KeyCode key) const` | Returns true if the key was pressed this frame. |
| `bool IsKeyReleased(KeyCode key) const` | Returns true if the key was released this frame. |
| `bool IsMouseButtonDown(int button) const` | Returns true if the mouse button is currently pressed. |
| `bool IsMouseButtonPressed(int button) const` | Returns true if the mouse button was pressed this frame. |
| `bool IsMouseButtonReleased(int button) const` | Returns true if the mouse button was released this frame. |
| `glm::ivec2 GetMousePosition() const` | Returns the current mouse position. |
| `glm::ivec2 GetMouseDelta() const` | Returns the change in mouse position since the last update. |

## Notes
- Tracks current and previous states for keys and mouse buttons to detect presses and releases.  
- `Update()` must be called each frame to refresh input states.  
- Provides both absolute mouse position and delta movement for camera or UI control.
