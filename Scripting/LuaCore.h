#pragma once

#include "../Engine/Define.h"
#include "../Engine/StandardInclude.h"

#include "Lua.h"

namespace BSE
{
    class LuaCore
    {
    public:
        LuaCore();
        ~LuaCore();

        bool Initialize();
        void Shutdown();

        lua_State* GetLuaState() const { return m_LuaState; }

    private:
        lua_State* m_LuaState = nullptr;
    };

    class LuaEngine
    {
    public:
        LuaEngine() = default;
        ~LuaEngine() = default;
        
        bool SetLuaCore(std::unique_ptr<LuaCore> luaCore);

        bool ExecuteScriptFile(const std::string& path);

        bool ExecuteScriptString(const std::string& chunk);

        template<typename... Args>
        bool CallFunction(const std::string& funcName, Args&&... args)
        {
            if (!m_LuaCore) return false;
            lua_State* L = m_LuaCore->GetLuaState();
            if (!L) return false;

            lua_getglobal(L, funcName.c_str());
            if (!lua_isfunction(L, -1))
            {
                lua_pop(L, 1);
                return false;
            }

            int numArgs = PushAll(L, std::forward<Args>(args)...);

            if (lua_pcall(L, numArgs, 0, 0) != LUA_OK)
            {
                const char* msg = lua_tostring(L, -1);
                printf("Lua error calling %s: %s\n", funcName.c_str(), msg ? msg : "unknown");
                lua_pop(L, 1);
                return false;
            }

            return true;
        }

        template<typename R, typename... Args>
        std::optional<R> CallFunctionWithReturn(const std::string& funcName, Args&&... args)
        {
            if (!m_LuaCore) return std::nullopt;
            lua_State* L = m_LuaCore->GetLuaState();
            if (!L) return std::nullopt;

            lua_getglobal(L, funcName.c_str());
            if (!lua_isfunction(L, -1))
            {
                lua_pop(L, 1);
                return std::nullopt;
            }

            int numArgs = PushAll(L, std::forward<Args>(args)...);

            if (lua_pcall(L, numArgs, 1, 0) != LUA_OK)
            {
                const char* msg = lua_tostring(L, -1);
                printf("Lua error calling %s: %s\n", funcName.c_str(), msg ? msg : "unknown");
                lua_pop(L, 1);
                return std::nullopt;
            }

            std::optional<R> result = GetValue<R>(L, -1);
            lua_pop(L, 1);
            return result;
        }

    private:
        std::unique_ptr<LuaCore> m_LuaCore = nullptr;

        static void PushSingle(lua_State* L, int value) { lua_pushinteger(L, static_cast<lua_Integer>(value)); }
        static void PushSingle(lua_State* L, long long value) { lua_pushinteger(L, static_cast<lua_Integer>(value)); }
        static void PushSingle(lua_State* L, double value) { lua_pushnumber(L, static_cast<lua_Number>(value)); }
        static void PushSingle(lua_State* L, float value) { lua_pushnumber(L, static_cast<lua_Number>(value)); }
        static void PushSingle(lua_State* L, bool value) { lua_pushboolean(L, value); }
        static void PushSingle(lua_State* L, const char* value) { lua_pushstring(L, value); }
        static void PushSingle(lua_State* L, const std::string& value) { lua_pushlstring(L, value.c_str(), value.size()); }

        static int PushAll(lua_State* L) { return 0; }

        template<typename T, typename... Rest>
        static int PushAll(lua_State* L, T&& first, Rest&&... rest)
        {
            PushSingle(L, std::forward<T>(first));
            return 1 + PushAll(L, std::forward<Rest>(rest)...);
        }

        template<typename T>
        static std::optional<T> GetValue(lua_State* L, int index);
    };

    template<>
    inline std::optional<int> LuaEngine::GetValue<int>(lua_State* L, int index)
    {
        if (!lua_isinteger(L, index) && !lua_isnumber(L, index)) return std::nullopt;
        return static_cast<int>(lua_tointeger(L, index));
    }

    template<>
    inline std::optional<long long> LuaEngine::GetValue<long long>(lua_State* L, int index)
    {
        if (!lua_isinteger(L, index) && !lua_isnumber(L, index)) return std::nullopt;
        return static_cast<long long>(lua_tointeger(L, index));
    }

    template<>
    inline std::optional<double> LuaEngine::GetValue<double>(lua_State* L, int index)
    {
        if (!lua_isnumber(L, index)) return std::nullopt;
        return static_cast<double>(lua_tonumber(L, index));
    }

    template<>
    inline std::optional<float> LuaEngine::GetValue<float>(lua_State* L, int index)
    {
        if (!lua_isnumber(L, index)) return std::nullopt;
        return static_cast<float>(lua_tonumber(L, index));
    }

    template<>
    inline std::optional<bool> LuaEngine::GetValue<bool>(lua_State* L, int index)
    {
        if (!lua_isboolean(L, index)) return std::nullopt;
        return (lua_toboolean(L, index) != 0);
    }

    template<>
    inline std::optional<std::string> LuaEngine::GetValue<std::string>(lua_State* L, int index)
    {
        if (!lua_isstring(L, index)) return std::nullopt;
        size_t len = 0;
        const char* s = lua_tolstring(L, index, &len);
        return std::string(s, len);
    }
}
