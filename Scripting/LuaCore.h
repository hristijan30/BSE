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

        bool RegisterCFunction(const std::string& name, lua_CFunction func);

        template<typename R, typename... Args>
        bool RegisterFunction(const std::string& name, R(*func)(Args...))
        {
            return RegisterFunction(name, std::function<R(Args...)>(func));
        }

        template<typename R, typename... Args>
        bool RegisterFunction(const std::string& name, std::function<R(Args...)> callable)
        {
            if (!m_LuaCore) return false;
            lua_State* L = m_LuaCore->GetLuaState();
            if (!L) return false;

            auto wrapper = std::make_unique<std::function<int(lua_State*)>>(
                [callable = std::move(callable)](lua_State* L) -> int
                {
                    constexpr std::size_t ARGC = sizeof...(Args);
                    int top = lua_gettop(L);
                    if (top < static_cast<int>(ARGC))
                    {
                        return luaL_error(L, "Lua -> C++: insufficient arguments (need %zu, got %d)", ARGC, top);
                    }

                    std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> argsTuple;

                    bool ok = GetArgsFromLua<0, Args...>(L, argsTuple);
                    if (!ok)
                    {
                        return luaL_error(L, "Lua -> C++: argument type mismatch");
                    }

                    if constexpr (std::is_void<R>::value)
                    {
                        std::apply(callable, argsTuple);
                        return 0;
                    }
                    else
                    {
                        R result = std::apply(callable, argsTuple);
                        PushSingle(L, result);
                        return 1;
                    }
                }
            );

            std::function<int(lua_State*)>* rawPtr = wrapper.get();
            m_cppFunctions.push_back(std::move(wrapper));

            void* ud = lua_newuserdata(m_LuaCore->GetLuaState(), sizeof(std::function<int(lua_State*)>*));
            *static_cast<std::function<int(lua_State*)>**>(ud) = rawPtr;

            lua_pushcclosure(L, &LuaEngine::FunctionTrampoline, 1);
            lua_setglobal(L, name.c_str());

            return true;
        }

    private:
        std::vector<std::unique_ptr<std::function<int(lua_State*)>>> m_cppFunctions;

        std::unique_ptr<LuaCore> m_LuaCore = nullptr;

        static void PushSingle(lua_State* L, int value) { lua_pushinteger(L, static_cast<lua_Integer>(value)); }
        static void PushSingle(lua_State* L, long long value) { lua_pushinteger(L, static_cast<lua_Integer>(value)); }
        static void PushSingle(lua_State* L, double value) { lua_pushnumber(L, static_cast<lua_Number>(value)); }
        static void PushSingle(lua_State* L, float value) { lua_pushnumber(L, static_cast<lua_Number>(value)); }
        static void PushSingle(lua_State* L, bool value) { lua_pushboolean(L, value); }
        static void PushSingle(lua_State* L, const char* value) { lua_pushstring(L, value); }
        static void PushSingle(lua_State* L, const std::string& value) { lua_pushlstring(L, value.c_str(), value.size()); }

        template<typename T>
        static std::optional<T> GetValue(lua_State* L, int index);

        template<typename T>
        static bool GetLuaArg(lua_State* L, int index, T& out)
        {
            auto v = GetValue<T>(L, index);
            if (!v) return false;
            out = *v;
            return true;
        }

        template<std::size_t I>
        static bool GetArgsFromLua(lua_State* /*L*/, std::tuple<> &)
        {
            return true;
        }

        template<std::size_t Index, typename First, typename... Rest, typename Tuple>
        static bool GetArgsFromLua(lua_State* L, Tuple& tup)
        {
            First val;
            if (!GetLuaArg<First>(L, static_cast<int>(Index + 1), val))
                return false;
            std::get<Index>(tup) = std::move(val);
            if constexpr (sizeof...(Rest) == 0)
            {
                return true;
            }
            else
            {
                return GetArgsFromLua<Index + 1, Rest...>(L, tup);
            }
        }

        static int FunctionTrampoline(lua_State* L);
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
