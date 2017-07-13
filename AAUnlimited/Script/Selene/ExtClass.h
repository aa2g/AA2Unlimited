#pragma once

#include "ClassFun.h"
#include "Ctor.h"
#include "Dtor.h"
#include "MetatableRegistry.h"
#include <map>
#include <memory>
#include "util.h"
#include <vector>
#include <stack>

namespace sel {

template <typename T,
          typename... Members>
class ExtClass : public BaseClass {
private:
    std::string _name;
    std::string _metatable_name;
    using Funs = std::vector<std::unique_ptr<BaseFun>>;
    Funs _funs;

    template <typename M>
    void _register_member(lua_State *state,
                          const char *member_name,
                          M T::*member) {
        _register_member(state, member_name, member,
                         typename std::is_const<M>::type{});
    }

    template <typename M>
    void _register_member(lua_State *state,
                          const char *member_name,
                          M T::*member,
                          std::false_type) {
        std::function<M(T*)> lambda_get = [member](T *t) {
            return t->*member;
        };
        _funs.emplace_back(
            sel::make_unique<ClassFun<1, T, M>>(
                state, std::string("get_") + member_name,
                _metatable_name.c_str(), lambda_get));

        std::function<void(T*, M)> lambda_set = [member](T *t, M value) {
            (t->*member) = value;
        };
        _funs.emplace_back(
            sel::make_unique<ClassFun<0, T, void, M>>(
                state, std::string("set_") + member_name,
                _metatable_name.c_str(), lambda_set));
    }

    template <typename M>
    void _register_member(lua_State *state,
                          const char *member_name,
                          M T::*member,
                          std::true_type) {
        std::function<M(T*)> lambda_get = [member](T *t) {
            return t->*member;
        };
        _funs.emplace_back(
            sel::make_unique<ClassFun<1, T, M>>(
                state, std::string{member_name},
                _metatable_name.c_str(), lambda_get));
    }

    template <typename Ret, typename... Args>
    void _register_member(lua_State *state,
                          const char *fun_name,
                          Ret(T::*fun)(Args&&...)) {
        std::function<Ret(T*, Args&&...)> lambda = [fun](T *t, Args&&... args) -> Ret {
            return (t->*fun)(std::forward<Args>(args)...);
        };
        constexpr int arity = detail::_arity<Ret>::value;
        _funs.emplace_back(
            sel::make_unique<ClassFun<arity, T, Ret, Args...>>(
                state, std::string(fun_name),
                _metatable_name.c_str(), lambda));
    }

    template <typename Ret, typename... Args>
    void _register_member(lua_State *state,
                          const char *fun_name,
                          Ret(T::*fun)(Args...)) {
        std::function<Ret(T*, Args...)> lambda = [fun](T *t, Args... args) {
            return (t->*fun)(args...);
        };
        constexpr int arity = detail::_arity<Ret>::value;
        _funs.emplace_back(
            sel::make_unique<ClassFun<arity, T, Ret, Args...>>(
                state, std::string(fun_name),
                _metatable_name.c_str(), lambda));
    }

    template <typename Ret, typename... Args>
    void _register_member(lua_State *state,
                          const char *fun_name,
                          Ret(T::*fun)(Args...) const) {
        std::function<Ret(const T*, Args...)> lambda =
            [fun](const T *t, Args... args) {
                return (t->*fun)(args...);
            };
        constexpr int arity = detail::_arity<Ret>::value;
        _funs.emplace_back(
            sel::make_unique<ClassFun<arity, const T, Ret, Args...>>(
                state, std::string(fun_name),
                _metatable_name.c_str(), lambda));
    }

    void _register_members(lua_State *state) {}

    template <typename M, typename... Ms>
    void _register_members(lua_State *state,
                           const char *name,
                           M member,
                           Ms... members) {
        _register_member(state, name, member);
        _register_members(state, members...);
    }

public:
    ExtClass(lua_State *state,
          const std::string &name,
          Members... members) : _name(name) {
		lua_State *L = state;

		// top level meta
		_metatable_name = "ExtClass::" + _name;
		MetatableRegistry::PushNewMetatable(state, typeid(T), _metatable_name, true);
		_register_members(L, members...);

		// getter
		lua_pushcfunction(state, lua_CFunction([](lua_State *L) {
			lua_getmetatable(L, 1);
			lua_pushvalue(L, 2);
			lua_rawget(L, -2);
			if (lua_isnil(L, -1)) {
				lua_pushstring(L, (std::string("get_") + lua_tostring(L, 2)).c_str());
				lua_rawget(L, -3);
				lua_pushvalue(L, 1);
				lua_call(L, 1, 1);
			}
			return 1;
		}));
		lua_setfield(L, -2, "__index");

		// setter
		lua_pushcfunction(L, lua_CFunction([](lua_State *L) {
			lua_getmetatable(L, 1);
			lua_pushstring(L, (std::string("set_") + lua_tostring(L, 2)).c_str());
			lua_rawget(L, -2);
			lua_pushvalue(L, 1);
			lua_pushvalue(L, 3);
			lua_call(L, 2, 0);
			return 0;
		}));
		lua_setfield(L, -2, "__newindex");
		lua_pop(L, 1);
    }
    ~ExtClass() = default;
    ExtClass(const ExtClass &) = delete;
    ExtClass& operator=(const ExtClass &) = delete;
    ExtClass(ExtClass &&other) = default;
    ExtClass& operator=(ExtClass &&other) = default;
};
}
