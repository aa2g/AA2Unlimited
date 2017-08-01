#pragma once

#include <windows.h>
#include "ExceptionTypes.h"
#include <string>
#include "traits.h"
#include <type_traits>
#include <typeinfo>
#include "MetatableRegistry.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

/* The purpose of this header is to handle pushing and retrieving
 * primitives from the stack
 */

namespace sel {

namespace detail {

using TypeID = std::reference_wrapper<const std::type_info>;

template <typename T>
struct is_primitive {
    static constexpr bool value = false;
};
template <>
struct is_primitive<int> {
    static constexpr bool value = true;
};
template <>
struct is_primitive<DWORD> {
	static constexpr bool value = true;
};
template <>
struct is_primitive<BYTE> {
	static constexpr bool value = true;
};
template <>
struct is_primitive<unsigned int> {
    static constexpr bool value = true;
};
template <>
struct is_primitive<bool> {
    static constexpr bool value = true;
};
template <>
struct is_primitive<lua_Number> {
    static constexpr bool value = true;
};
template <>
struct is_primitive<lua_CFunction> {
	static constexpr bool value = true;
};
template <>
struct is_primitive<std::string> {
    static constexpr bool value = true;
};
template <>
struct is_primitive<const char *> {
	static constexpr bool value = true;
};

template<typename T>
using decay_primitive =
    typename std::conditional<
        is_primitive<typename std::decay<T>::type>::value,
        typename std::decay<T>::type,
        T
    >::type;

/* getters */
template <typename T>
inline T* _get(_id<T*>, lua_State *l, const int index) {
    if(MetatableRegistry::IsType(l, typeid(T), index)) {
        return (T*)lua_topointer(l, index);
    }
    return nullptr;
}

template <typename T>
inline T& _get(_id<T&>, lua_State *l, const int index) {
    if(!MetatableRegistry::IsType(l, typeid(T), index)) {
        throw TypeError{
            MetatableRegistry::GetTypeName(l, typeid(T)),
            MetatableRegistry::GetTypeName(l, index)
        };
    }

    T *ptr = (T*)lua_topointer(l, index);
    if(ptr == nullptr) {
        throw TypeError{MetatableRegistry::GetTypeName(l, typeid(T))};
    }
    return *ptr;
}

template <typename T>
inline typename std::enable_if<
    !is_primitive<typename std::decay<T>::type>::value, T
>::type
_get(_id<T>, lua_State *l, const int index) {
    return _get(_id<T&>{}, l, index);
}

inline bool _get(_id<bool>, lua_State *l, const int index) {
    return lua_toboolean(l, index) != 0;
}


#define _GET_INT(it) \
inline int _get(_id<it>, lua_State *l, const int index) { \
    return static_cast<it>(lua_tointeger(l, index)); \
}
_GET_INT(int)
_GET_INT(DWORD)
_GET_INT(WORD)
_GET_INT(BYTE)
#undef _GET_INT


inline unsigned int _get(_id<unsigned int>, lua_State *l, const int index) {
#if LUA_VERSION_NUM >= 502 && LUA_VERSION_NUM < 503
    return lua_tounsigned(l, index);
#else
    return static_cast<unsigned>(lua_tointeger(l, index));
#endif
}

inline lua_Number _get(_id<lua_Number>, lua_State *l, const int index) {
    return lua_tonumber(l, index);
}

inline std::string _get(_id<std::string>, lua_State *l, const int index) {
    size_t size;
    const char *buff = lua_tolstring(l, index, &size);
    return std::string{buff, size};
}

inline const char *_get(_id<const char *>, lua_State *l, const int index) {
    return lua_tostring(l, index);
}

using _lua_check_get = void (*)(lua_State *l, int index);
// Throw this on conversion errors to prevent long jumps caused in Lua from
// bypassing destructors. The outermost function can then call checkd_get(index)
// in a context where a long jump is safe.
// This way we let Lua generate the error message and use proper stack
// unwinding.
struct GetParameterFromLuaTypeError {
    _lua_check_get checked_get;
    int index;
};

template <typename T>
inline T* _check_get(_id<T*>, lua_State *l, const int index) {
    MetatableRegistry::CheckType(l, typeid(T), index);
    return (T *)lua_topointer(l, index);
}

template <typename T>
inline T& _check_get(_id<T&>, lua_State *l, const int index) {
    static_assert(!is_primitive<T>::value,
                  "Reference types must not be primitives.");

    T *ptr = _check_get(_id<T*>{}, l, index);

    if(ptr == nullptr) {
        throw GetUserdataParameterFromLuaTypeError{
            MetatableRegistry::GetTypeName(l, typeid(T)),
            index
        };
    }

    return *ptr;
}

template <typename T>
inline typename std::enable_if<
    !is_primitive<typename std::decay<T>::type>::value, T
>::type
_check_get(_id<T>, lua_State *l, const int index) {
    return _check_get(_id<T&>{}, l, index);
}

template <typename T>
inline T _check_get(_id<T&&>, lua_State *l, const int index) {
    return _check_get(_id<T>{}, l, index);
}


inline int _check_get(_id<int>, lua_State *l, const int index) {
#if LUA_VERSION_NUM >= 502
    int isNum = 0;
    auto res = static_cast<int>(lua_tointegerx(l, index, &isNum));
    if(!isNum){
        throw GetParameterFromLuaTypeError{
#if LUA_VERSION_NUM >= 503
            [](lua_State *l, int index){luaL_checkinteger(l, index);},
#else
            [](lua_State *l, int index){luaL_checkint(l, index);},
#endif
            index
        };
    }
    return res;
#else
#error "Not supported for Lua versions <5.2"
#endif
}

inline unsigned int _check_get(_id<unsigned int>, lua_State *l, const int index) {
    int isNum = 0;
#if LUA_VERSION_NUM >= 503
    auto res = static_cast<unsigned>(lua_tointegerx(l, index, &isNum));
    if(!isNum) {
        throw GetParameterFromLuaTypeError{
            [](lua_State *l, int index){luaL_checkinteger(l, index);},
            index
        };
    }
#elif LUA_VERSION_NUM >= 502
    auto res = static_cast<unsigned>(lua_tounsignedx(l, index, &isNum));
    if(!isNum) {
        throw GetParameterFromLuaTypeError{
            [](lua_State *l, int index){luaL_checkunsigned(l, index);},
            index
        };
    }
#else
#error "Not supported for Lua versions <5.2"
#endif
    return res;
}

inline BYTE _check_get(_id<BYTE>, lua_State *l, const int index) {
	int isNum = 0;

	auto res = static_cast<unsigned>(lua_tointegerx(l, index, &isNum));
	if (!isNum) {
		throw GetParameterFromLuaTypeError{
			[](lua_State *l, int index) {luaL_checkinteger(l, index); },
			index
		};
	}
	return res;
}

inline DWORD _check_get(_id<DWORD>, lua_State *l, const int index) {
	int isNum = 0;
	auto res = static_cast<unsigned>(lua_tointegerx(l, index, &isNum));
	if (!isNum) {
		throw GetParameterFromLuaTypeError{
			[](lua_State *l, int index) {luaL_checkinteger(l, index); },
			index
		};
	}
	return res;
}


inline lua_Number _check_get(_id<lua_Number>, lua_State *l, const int index) {
    int isNum = 0;
    auto res = lua_tonumberx(l, index, &isNum);
    if(!isNum){
        throw GetParameterFromLuaTypeError{
            [](lua_State *l, int index){luaL_checknumber(l, index);},
            index
        };
    }
    return res;
}

inline bool _check_get(_id<bool>, lua_State *l, const int index) {
    return lua_toboolean(l, index) != 0;
}

inline std::string _check_get(_id<std::string>, lua_State *l, const int index) {
    size_t size = 0;
    char const * buff = lua_tolstring(l, index, &size);
    if(buff == nullptr) {
        throw GetParameterFromLuaTypeError{
            [](lua_State *l, int index){luaL_checkstring(l, index);},
            index
        };
    }
    return std::string{buff, size};
}

inline const char *_check_get(_id<const char *>, lua_State *l, const int index) {
    char const * buff = lua_tostring(l, index);
    if(buff == nullptr) {
        throw GetParameterFromLuaTypeError{
            [](lua_State *l, int index){luaL_checkstring(l, index);},
            index
        };
    }
    return buff;
}


// Worker type-trait struct to _get_n
// Getting multiple elements returns a tuple
template <typename... Ts>
struct _get_n_impl {
    using type =  std::tuple<Ts...>;

    template <std::size_t... N>
    static type worker(lua_State *l,
                       _indices<N...>) {
        return std::make_tuple(_get(_id<Ts>{}, l, N + 1)...);
    }

    static type apply(lua_State *l) {
        return worker(l, typename _indices_builder<sizeof...(Ts)>::type());
    }
};

// Getting nothing returns void
template <>
struct _get_n_impl<> {
    using type = void;
    static type apply(lua_State *) {}
};

// Getting one element returns an unboxed value
template <typename T>
struct _get_n_impl<T> {
    using type = T;
    static type apply(lua_State *l) {
        return _get(_id<T>{}, l, -1);
    }
};

template <typename... T>
typename _get_n_impl<T...>::type _get_n(lua_State *l) {
    return _get_n_impl<T...>::apply(l);
}

template <typename T>
T _pop(_id<T> t, lua_State *l) {
    T ret =  _get(t, l, -1);
    lua_pop(l, 1);
    return ret;
}

/* Setters */

inline void _push(lua_State *) {}

template<typename T>
inline void push_ud_typeid2(lua_State *l, T& ref, TypeID tid) {
	push_ud_typeid(l, (void*)&ref, tid);
}
/*inline void push_ud_typeid2(lua_State *l, void *p, TypeID tid) {
	return push_ud_typeid(l, p, tid);
}*/
inline void push_ud_typeid(lua_State *l, void *p, TypeID tid) {
	MetatableRegistry::detail::_push_meta_table(l);
	MetatableRegistry::detail::_push_typeinfo(l, tid);
    lua_gettable(l, -2);
    int typex = lua_tointeger(l, -1);
    lua_pop(l, 2);
    if (!typex) {
        // there is no typex, so must be legit userdata
	if (!MetatableRegistry::IsRegisteredType(l, tid)) {
        	throw CopyUnregisteredType(tid);
	}
	lua_pushuserdata(l, p);
    } else {
	lua_pushlightuserdatax(l, p, typex);
    }
}

template <typename T>
inline void _push(lua_State *l, T* t) {
  if(t == nullptr) {
    lua_pushnil(l);
  }
  else {
    push_ud_typeid(l, t, typeid(T));
  }
}

template <typename T>
inline typename std::enable_if<
    !is_primitive<typename std::decay<T>::type>::value
>::type
_push(lua_State *l, T& t) {
    push_ud_typeid(l, (void*)&t, typeid(T*));
}

/*
template <typename T>
inline typename std::enable_if<
    !is_primitive<typename std::decay<T>::type>::value
    && std::is_rvalue_reference<T&&>::value
>::type
_push(lua_State *l, T&& t) {
    if(!MetatableRegistry::IsRegisteredType(l, typeid(t)))
    {
        throw CopyUnregisteredType(typeid(t));
    }

    void *addr = lua_newuserdata(l, sizeof(T));
    new(addr) T(std::forward<T>(t));
    MetatableRegistry::SetMetatable(l, typeid(T));
}*/

inline void _push(lua_State *l, bool b) {
    lua_pushboolean(l, b);
}

#define _PUSH_INT(it) \
inline void _push(lua_State *l, it i) { \
    lua_pushinteger(l, int(i)); \
}

_PUSH_INT(int)
_PUSH_INT(DWORD)
_PUSH_INT(WORD)
_PUSH_INT(BYTE)
#undef _PUSH_INT

inline void _push(lua_State *l, unsigned int u) {
#if LUA_VERSION_NUM >= 503
  lua_pushinteger(l, (lua_Integer)u);
#elif LUA_VERSION_NUM >= 502
    lua_pushunsigned(l, u);
#else
    lua_pushinteger(l, static_cast<int>(u));
#endif
}

inline void _push(lua_State *l, lua_CFunction f) {
	lua_pushcfunction(l, f);
}

inline void _push(lua_State *l, lua_Number f) {
    lua_pushnumber(l, f);
}

inline void _push(lua_State *l, const std::string &s) {
    lua_pushlstring(l, s.c_str(), s.size());
}

inline void _push(lua_State *l, const char *s) {
    lua_pushstring(l, s);
}

template <typename T>
inline void _set(lua_State *l, T &&value, const int index) {
    _push(l, std::forward<T>(value));
    lua_replace(l, index);
}

inline void _push_n(lua_State *) {}

template <typename T, typename... Rest>
inline void _push_n(lua_State *l, T &&value, Rest&&... rest) {
    _push(l, std::forward<T>(value));
    _push_n(l, std::forward<Rest>(rest)...);
}

template <typename... T, std::size_t... N>
inline void _push_dispatcher(lua_State *l,
                             const std::tuple<T...> &values,
                             _indices<N...>) {
    _push_n(l, std::get<N>(values)...);
}

inline void _push(lua_State *, std::tuple<>) {}

template <typename... T>
inline void _push(lua_State *l, const std::tuple<T...> &values) {
    constexpr int num_values = sizeof...(T);
    _push_dispatcher(l, values,
                     typename _indices_builder<num_values>::type());
}

template <typename... T>
inline void _push(lua_State *l, std::tuple<T...> &&values) {
    _push(l, const_cast<const std::tuple<T...> &>(values));
}

}
}
