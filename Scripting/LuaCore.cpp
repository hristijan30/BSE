#include "LuaCore.h"

namespace BSE
{
    LuaCore::LuaCore()
    {
        Initialize();
    }

    LuaCore::~LuaCore()
    {
        Shutdown();
    }

    bool LuaCore::Initialize()
    {
        m_LuaState = luaL_newstate();
        if (!m_LuaState)
        {
            return false;
        }

        luaL_openlibs(m_LuaState);
        return true;
    }

    void LuaCore::Shutdown()
    {
        if (m_LuaState)
        {
            lua_close(m_LuaState);
            m_LuaState = nullptr;
        }
    }

    bool LuaEngine::SetLuaCore(std::unique_ptr<LuaCore> luaCore)
    {
        if (!luaCore)
        {
            return false;
        }

        m_LuaCore = std::move(luaCore);
        return true;
    }

    bool LuaEngine::ExecuteScriptFile(const std::string& path)
    {
        if (!m_LuaCore) return false;
        lua_State* L = m_LuaCore->GetLuaState();
        if (!L) return false;

        int loadStatus = luaL_loadfile(L, path.c_str());
        if (loadStatus != LUA_OK)
        {
            const char* msg = lua_tostring(L, -1);
            printf("Failed to load Lua file '%s': %s\n", path.c_str(), msg ? msg : "unknown");
            lua_pop(L, 1);
            return false;
        }

        if (lua_pcall(L, 0, 0, 0) != LUA_OK)
        {
            const char* msg = lua_tostring(L, -1);
            printf("Error running Lua file '%s': %s\n", path.c_str(), msg ? msg : "unknown");
            lua_pop(L, 1);
            return false;
        }

        return true;
    }

    bool LuaEngine::ExecuteScriptString(const std::string& chunk)
    {
        if (!m_LuaCore) return false;
        lua_State* L = m_LuaCore->GetLuaState();
        if (!L) return false;

        int loadStatus = luaL_loadstring(L, chunk.c_str());
        if (loadStatus != LUA_OK)
        {
            const char* msg = lua_tostring(L, -1);
            printf("Failed to load Lua chunk: %s\n", msg ? msg : "unknown");
            lua_pop(L, 1);
            return false;
        }

        if (lua_pcall(L, 0, 0, 0) != LUA_OK)
        {
            const char* msg = lua_tostring(L, -1);
            printf("Error running Lua chunk: %s\n", msg ? msg : "unknown");
            lua_pop(L, 1);
            return false;
        }

        return true;
    }
}

