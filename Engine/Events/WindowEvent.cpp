#include "WindowEvent.h"
#include <cstddef>

namespace BSE
{
    size_t SDLEventManager::Subscribe(WindowEventType type, Callback cb)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        size_t token = m_nextToken.fetch_add(1, std::memory_order_relaxed);
        m_listeners[type].emplace_back(token, std::move(cb));
        m_tokenToType[token] = type;
        return token;
    }

    void SDLEventManager::Unsubscribe(size_t token)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_tokenToType.find(token);
        if (it == m_tokenToType.end()) return;
        WindowEventType type = it->second;
        auto lit = m_listeners.find(type);
        if (lit == m_listeners.end()) { m_tokenToType.erase(it); return; }

        auto &vec = lit->second;
        vec.erase(std::remove_if(vec.begin(), vec.end(),
            [token](const std::pair<size_t, Callback>& p) { return p.first == token; }),
            vec.end());

        m_tokenToType.erase(it);
    }

    void SDLEventManager::ClearAll()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_listeners.clear();
        m_tokenToType.clear();
    }

    void SDLEventManager::Dispatch(const WindowEvent& ev)
    {
        std::vector<Callback> callbacks;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_listeners.find(ev.type);
            if (it != m_listeners.end())
            {
                callbacks.reserve(it->second.size());
                for (auto &p : it->second) callbacks.push_back(p.second);
            }
        }

        for (auto &cb : callbacks)
        {
            try { cb(ev); }
            catch (const std::exception& e)
            {
                std::cerr << "[SDLEventManager] callback exception: " << e.what() << std::endl;
            }
            catch (...)
            {
                std::cerr << "[SDLEventManager] callback exception (unknown)" << std::endl;
            }
        }
    }

    void SDLEventManager::PollEvents(SDL_Window* window, bool &running)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_EVENT_QUIT)
            {
                running = false;
            }

    #if defined(SDL_WINDOWEVENT)
            if (e.type == SDL_WINDOWEVENT)
            {
                WindowEvent ev;
                ev.type = WindowEventType::None;

                switch (e.window.event)
                {
    #ifdef SDL_WINDOWEVENT_CLOSE
                    case SDL_WINDOWEVENT_CLOSE:
                        ev.type = WindowEventType::Close;
                        break;
    #endif
    #ifdef SDL_WINDOWEVENT_RESIZED
                    case SDL_WINDOWEVENT_RESIZED:
                        ev.type = WindowEventType::Resize;
                        ev.width = e.window.data1;
                        ev.height = e.window.data2;
                        break;
    #endif
    #ifdef SDL_WINDOWEVENT_MOVED
                    case SDL_WINDOWEVENT_MOVED:
                        ev.type = WindowEventType::Moved;
                        ev.x = e.window.data1;
                        ev.y = e.window.data2;
                        break;
    #endif
    #ifdef SDL_WINDOWEVENT_FOCUS_GAINED
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                        ev.type = WindowEventType::FocusGained;
                        break;
    #endif
    #ifdef SDL_WINDOWEVENT_FOCUS_LOST
                    case SDL_WINDOWEVENT_FOCUS_LOST:
                        ev.type = WindowEventType::FocusLost;
                        break;
    #endif
    #ifdef SDL_WINDOWEVENT_MINIMIZED
                    case SDL_WINDOWEVENT_MINIMIZED:
                        ev.type = WindowEventType::Minimized;
                        break;
    #endif
    #ifdef SDL_WINDOWEVENT_MAXIMIZED
                    case SDL_WINDOWEVENT_MAXIMIZED:
                        ev.type = WindowEventType::Maximized;
                        break;
    #endif
    #ifdef SDL_WINDOWEVENT_RESTORED
                    case SDL_WINDOWEVENT_RESTORED:
                        ev.type = WindowEventType::Restored;
                        break;
    #endif
                    default:
                        break;
                }

                if (ev.type != WindowEventType::None)
                    Dispatch(ev);

                continue;
            }
    #endif
        }
    }
}
