#pragma once

#include "../Define.h"
#include "../StandardInclude.h"
#include "../SDL_Include.h"

namespace BSE
{
    enum class WindowEventType
    {
        None,
        Close,
        Resize,
        FocusGained,
        FocusLost,
        Minimized,
        Maximized,
        Restored,
        Moved
    };

    struct WindowEvent
    {
        WindowEventType type = WindowEventType::None;
        int width = 0;
        int height = 0;
        int x = 0;
        int y = 0;
    };

    class DLL_EXPORT SDLEventManager
    {
    public:
        using Callback = std::function<void(const WindowEvent&)>;

        SDLEventManager() = default;
        ~SDLEventManager() = default;

        size_t Subscribe(WindowEventType type, Callback cb);
        void Unsubscribe(size_t token);

        void PollEvents(SDL_Window* window, bool &running);
        void ClearAll();

    private:
        void Dispatch(const WindowEvent& ev);

        mutable std::mutex m_mutex;
        std::unordered_map<WindowEventType, std::vector<std::pair<size_t, Callback>>> m_listeners;
        std::unordered_map<size_t, WindowEventType> m_tokenToType;
        std::atomic_size_t m_nextToken{1};
    };
}
