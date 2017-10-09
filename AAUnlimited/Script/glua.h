#pragma once

#include <string.h>
#include <assert.h>

#include "lua.hpp"

namespace GLua {

static const int ACCESOR = 0;
static const int METHOD = 1;


static const int RIDX_DELTA = 1<<16;
static const int METHOD_FLAG = 1<<24;
struct Value;
struct State;

typedef int (*Function)(State &);

template<typename T>
struct Index {
	T key;
	lua_State *L;
	int tab;
	inline int get_tab() {
		int t = tab;
		if (t < 0) {
			lua_pushglobaltable(L);
			t = lua_gettop(L);
		}
		return t;
	}

	// assingment to a table
	template<typename V>
	inline V const& operator=(V const &v);

	// get value represented by this table index
	inline Value get();
       
	// read from a table
	template<typename V>
	inline operator V() {
		return get();
	}

	// calls invoke the cast, and forwards arguments
	template <typename... Ts>
	inline Value operator()(const Ts&... values);
      
	template<typename K>
	Index<K> operator[](K key);

};

struct Value {
	lua_State *L;
	int idx;
	inline operator const char *() const { return lua_tostring(L, idx); }
	inline operator long() const { return lua_tointeger(L, idx); }
	inline operator int() const { return lua_tointeger(L, idx); }
	inline operator short() const { return lua_tointeger(L, idx); }
	inline operator char() const { return lua_tointeger(L, idx); }
	inline operator unsigned long() const { return lua_tointeger(L, idx); }
	inline operator unsigned long long() const { return lua_tointeger(L, idx); }
	inline operator unsigned int() const { return lua_tointeger(L, idx); }
	inline operator unsigned short() const { return lua_tointeger(L, idx); }
	inline operator unsigned char() const { return lua_tointeger(L, idx); }
	inline operator double() const { return lua_tonumber(L, idx); }
	inline operator float() const { return lua_tonumber(L, idx); }
	inline operator bool() const { return lua_toboolean(L, idx); }

	template <class T>
	inline operator T*() const { if (!idx) return 0; void *v = lua_touserdata(L, idx); if (!v) v = (void*)lua_tointeger(L, idx); return (T*)v; }

	inline bool isnil() { return lua_isnil(L, idx); }
	inline int type() { return lua_type(L, idx); }

	template <typename T>
	inline Index<T> operator [](T key) {
		return {key,L,idx};
	}

	template <typename... Ts>
	inline Value operator()(const Ts&... values) const;

};

struct State {
	// We are mere shadow over lua's L. A bit awkward, but saves us
	// the trouble of remembering our own `*this` in callbacks.

	State() = delete;
	~State() = delete;
	static State* make(lua_State *L) {
		return (State*)L;
	}
	static State* make() {
		lua_State *L = luaL_newstate();
		luaL_openlibs(L);
		return (State*)L;
	}

	inline lua_State *L() {
		return (lua_State*)this;
	}

	// Integers
	inline State& pushi(lua_Integer i) {
		lua_pushinteger(L(), i); return *this;
	}
	inline State& pushu(unsigned long u) {
		lua_pushinteger(L(), u); return *this;
	}

	inline auto& push(char c) { return pushi(c); }
	inline auto& push(short c) { return pushi(c); }
	inline auto& push(int c) { return pushi(c); }
	inline auto& push(long c) { return pushi(c); }

	inline auto& push(unsigned char c) { return pushu(c); }
	inline auto& push(unsigned short c) { return pushu(c); }
	inline auto& push(unsigned int c) { return pushu(c); }
	inline auto& push(unsigned long c) { return pushu(c); }
	inline State& push(unsigned long long u) {
		lua_pushinteger(L(), u); return *this;
	}

	inline auto& push(float f) { lua_pushnumber(L(), f);  return *this; }
	inline auto& push(double f) { lua_pushnumber(L(), f);  return *this; }
	inline auto& push(bool b) {
		lua_pushboolean(L(), b);  return *this;
	}

	inline auto& nil() {
		lua_pushnil(L());
		return *this;
	}

	// Strings
	inline auto& push(const char *s) {
		//printf("pushing string\n");
		if (s == 0) return nil();
		lua_pushstring(L(), s);
		return *this;
	}
	inline auto& pushlstring(const char *s, size_t n) {
		if (s == 0) return nil();
		lua_pushlstring(L(), s, n);
		return *this;
	}
	inline auto& push(const std::string& s) {
		lua_pushlstring(L(), s.c_str(), s.length());
		return *this;
	}

	// Closure

	inline auto& push(lua_CFunction c) {
		if (c == 0) return nil();
		lua_pushcfunction(L(), c);
		return *this;
	}

	inline auto& push(Function c) {
		if (c == 0) return nil();
		lua_pushcfunction(L(), lua_CFunction(c));
		return *this;
	}


#if 0	
	// remove const
	template <class T>
	inline State& push(const T v) {
		return push((T)v);
	}
#endif

	// remove reference and turn to pointer
/*	template <class T>
	inline State& push(const T &p) {
		printf("pushing pod\n");
		return push((T*)(&p));
	}*/

	// push pointers to class instances and record their typeid
	template <typename T>
	inline State& push(T *p) {
		if (p == 0) return nil();
		int tid = push_type(*this, type_id<T>());
		pop();
		lua_pushlightuserdatax(L(), p, tid);
		return *this;
	}

/*	inline auto& push(Value &v) {
		//printf("pushvalue ref\n");
		lua_pushvalue(L(), v.idx); return *this;
	}*/
	inline auto& push(Value v) {
		//printf("pushvalue copy\n");
		lua_pushvalue(L(), v.idx); return *this;
	}

	inline int pushmulti(int n) { return n; };
	template <typename T, typename... Ts>
	inline int pushmulti(int n, const T &value, const Ts&... values) {
		//printf("pushmulti %d\n", n);
		push(value);
		return pushmulti(n + 1, values...);
	}

	inline auto& pop(int n = 1) {
		lua_pop(L(), n);
		return *this;
	}

	template <class T>
	inline auto &setname(const char *n) {
		push(type_id<T>());
		lua_setfield(L(), LUA_REGISTRYINDEX, n);
		push_type(*this, type_id<T>());
		field("__name", -1, n);
		return pop();
	}

	inline State& cast(const char *name, void *p) {
		if (name) {
			lua_getfield(L(), LUA_REGISTRYINDEX, name);
			int tid = get();
			pop();
			lua_pushlightuserdatax(L(), p, tid);
		}
		else {
			lua_pushlightuserdata(L(), p);
		}
		return *this;
	}

	// Bind a class<T> method
/*	template <class T, class F>
	inline auto& bind(int kind, const char *name, F lam) {
		if (0) { int ret = lam(*this); (void)ret; }
		return bind<T>(kind, name, (lua_CFunction)(lam));
	}*/
	template <class T>
	inline auto& bind(int kind, const char *name, int (*handler)(State &s)) {
		return bind<T>(kind, name, (lua_CFunction)(handler));
	}
	template <class T>
	inline auto& bind(int kind, const char *name, lua_CFunction handler) {
		push_type(*this, type_id<T>());

		push(name);

		int nup = kind == ACCESOR;
		if (nup) nil();
		lua_pushcclosure(L(), handler, nup);

		lua_rawset(L(), -3);
		pop();

		return *this;
	}

	// Free-standing functions
	template <class F>
	inline Value bind(F lam) {
		if (0) { int ret = lam(*this); (void)ret; }
		push((lua_CFunction)(lam));
		return get();
	}

	template<typename T>
	inline int type_id() {
		static int id;
		if (!id) id = lua_allocatetypex(L());
		return id;
	};

	static int push_type(State &s, int t) {
		lua_State *L = s.L();
		if (lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_LAST + RIDX_DELTA + t) != LUA_TNIL)
			return t;
		lua_pop(L, 1);

		lua_newtable(L);

/*		lua_pushinteger(L, t);
		lua_setfield(L, -2, "__type");*/

		lua_pushvalue(L, -1);
		lua_rawseti(L, LUA_REGISTRYINDEX, LUA_RIDX_LAST + RIDX_DELTA + t);

		s.push(__index);
		lua_setfield(L, -2, "__index");
		s.push(__newindex);
		lua_setfield(L, -2, "__newindex");

		lua_pushvalue(L, -1);
		lua_setmetatable(L, -2);

		lua_pushvalue(L, -1);
		lua_setmetatablex(L, t);
		return t;
	}

	static int __index(lua_State *L) {
		if (!lua_getmetatable(L, 1))
			return 0;
		lua_pushvalue(L, 2);
		if (lua_rawget(L, -2) != LUA_TFUNCTION)
			return 0;


		// They're looking for a method fn value, not getter result
		if (!lua_getupvalue(L, -1, 1))
			return 1;
		lua_pop(L, 1);

		lua_pushvalue(L, 1); // obj
		lua_call(L, 1, 1);
		return 1;
	}

	static int __newindex(lua_State *L) {
		if (!lua_getmetatable(L, 1))
			return 0;
		lua_pushvalue(L, 2);
		if (lua_rawget(L, -2) != LUA_TFUNCTION)
			luaL_argerror(L, 2, "unexpected field write");

		lua_pushvalue(L, 1);
		lua_pushvalue(L, 3);
		lua_call(L, 2, 1);
		return 1;
	}


	inline int top() { return lua_gettop(L()); }
	inline State& top(int n) { lua_settop(L(), n); return *this; }

	inline auto& global(const char *s) {
		lua_getglobal(L(), s);
		return *this;
	}
	inline State& field(const char *s, int idx = -1) {
		lua_getfield(L(), idx, s);
		return *this;
	}

	template <typename T>
	inline State& field(const char *s, int idx, T val) {
		lua_pushvalue(L(), idx);
		lua_pushstring(L(), s);
		push(val);
		lua_rawset(L(), -3);
		return pop();
	}


	template <typename T>
	inline auto& global(const char *s, T v) {
		push(v);
		lua_setglobal(L(), s);
		return *this;
	}

	static int traceback(lua_State *L) {
		const char *msg = lua_tostring(L, -1);
		luaL_traceback(L, L, msg, 1);
		return 1;
	}

	bool isnil(int idx = -1) {
		if (idx > 0 && idx > top())
			return true;
		return lua_isnil(L(), idx);
	}

	template <typename... Ts>
	auto& call(const Ts&... values) {
		lua_State *L = this->L();
		assert(!lua_isnil(L, -1));
		assert(lua_isfunction(L, -1));
		lua_pushcfunction(L, traceback);
		lua_insert(L, -2);
		int nargs = pushmulti(0,values...);
		if (lua_pcall(L, nargs, 1, top() - nargs - 1) != LUA_OK) {
			const char *msg = lua_tostring(L, -1);
			pop(2);
			throw msg;
		}
		lua_remove(L, -2);
		return *this;
	}

	inline auto& eval(const char *code) {
		if (luaL_loadstring(L(), code) == LUA_ERRSYNTAX) {
			traceback(L());
			const char *msg = lua_tostring(L(), -1);
			pop();
			throw msg;
		}
		call();
		return *this;
	}

	inline Value newtable() {
		lua_newtable(L());
		return get();
	}

	inline Value get(int idx = -1) {
		return Value {L(), idx < 0 ? (top() + 1 + idx) : idx};
	}

	inline const char *gets(size_t *sz, int idx = -1) {
		return lua_tolstring(L(), idx, sz);
	}

	inline int gets(int idx, char *buf, size_t limit) {
		size_t sz;
		const char *s = lua_tolstring(L(), idx, &sz);
		if (!s) return -1;
		sz++;
		if (sz > limit) sz = limit;
		::memcpy(buf, s, limit);
		return sz;
	}


	static const int one = 1;
	static const int zero = 0;

	template <typename T>
	inline Index<T> operator [](T key) {
		return {key,L(),-1};
	}
};

static inline State* newstate() {
	return State::make();
}

struct Scope {
	struct State *parent;
	int stack;
	inline Scope(State *p) : parent(p) {
		stack = parent->top();
	}
	inline ~Scope() {
		parent->top(stack);
	}
};

template<typename T>
template<typename V>
inline V const& Index<T>::operator=(V const &v)
{
	auto s = State::make(L);
	int t = get_tab();
	s->push(key);
	s->push(v);
	lua_settable(L, t);
	if (tab < 0) lua_pop(L, 1);
	return v;
}


template<typename T>
inline Value Index<T>::get() {
	int t = get_tab();
	auto *s = State::make(L);
	s->push(key);
	lua_gettable(L, t);
	if (tab < 0) lua_remove(L, -2); // remove globals
	return {L, lua_gettop(L)};
}

template<typename T>
template<typename K>
inline Index<K> Index<T>::operator[](K key) {
	return get()[key];
}

template<typename T>
template <typename... Ts>
inline Value Index<T>::operator()(const Ts&... values) {
	return get()(values...);
}

template <typename... Ts>
inline Value Value::operator()(const Ts&... values) const {
	auto *s = State::make(L);
	s->push(*this);
	s->call(values...);
	return s->get();
}


}
// Basic macro used to generate class method/member bindings
#define GLUA_BIND(G, typ, kls, var, fun) { \
	G.bind<kls>(GLua::typ, #var, [](auto &_gl) { \
		lua_State *_L = _gl.L(); \
		(void)_L; \
		(void)_gl; \
		kls *_self = _gl.get(1); \
		(void)_self; \
		fun; \
		return 0; \
	}); \
}


