// glua is a simple, no-frills C++ template glue for lua. it's designed for
// performance, not necessarily safety of use
//
// values lifted from tables, return values of function calls and everything
// else are cached on Lua stack, and casted to C++ value on-demand. This means
// the stack gets constantly polluted and one needs to manage scopes, especially
// in loops.
//
// void print_passwd() {
//   GLua::Scope scope(lua);
//   auto io = lua["io"];
//   auto file = io["open"]("/etc/passwd", "rb");
//   auto read = file["read"];
//   while (1) {
//      // without a private scope there, stack would overflow with previous
//      // contents of 'ln'
//   	GLua::Scope scope(lua);
//      auto ln = read(file, "*l")
//      if (!ln) break;
//      std::cout << got << "\n"
//   }; // next iteration, 'ln' contents are invalid because it's scoped.
//   // scope gets destroyed
// }
//
//   

#define S printf("line %d top %d\n", __LINE__, top());

#include <type_traits>
#include <locale>
#include <codecvt>
#include <typeinfo>
#include <functional>
#include <tuple>

namespace GLua_detail {
	template <typename T> struct string { static constexpr bool value = false; };
	template <>struct string<const char *> { static constexpr bool value = true; };
	template <>struct string<const wchar_t *> { static constexpr bool value = true; };
	template <>struct string<std::string> { static constexpr bool value = true; };
	template <>struct string<std::wstring> { static constexpr bool value = true; };

	template <typename T>
	class callable
	{
		typedef char one;
		typedef long two;
		template <typename C> static one test( decltype(&C::operator()));
		template <typename C> static two test(...);    
		public:
		enum { value = sizeof(test<T>(0)) == sizeof(char) };
	};

	template <typename T>
	struct lambda : public lambda<decltype(&T::operator())> {};

	template <typename T, typename Ret, typename... Args>
	struct lambda<Ret(T::*)(Args...) const> {
		using f = Ret(*)(Args...);
	};


}

struct GLua {
	using type_id = std::reference_wrapper<const std::type_info>;
	static std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8;
	lua_State *L;

	// Representions of Lua values with two-way autocasts. These simply
	// assume the lua type is convertible with the C++ counterpart, otherwise
	// lua_to* behavior is to be expected. The only exception is casts to
	// std::string, which result "" strings for lua non-strings.

	// Narrow for integers
	struct Integer {
		lua_Integer i;
		template <class T,
			 typename std::enable_if<std::is_integral<T>::value,int>::type=0>
		inline Integer(T v) : i(v) {}
		template <typename T>
		inline operator T() const { return (T)i; }
	};

	// Otherwise it becomes float
	struct Number {
		lua_Number n;
		template <class T,
			 typename std::enable_if<std::is_floating_point<T>::value, int>::type=0>
		inline Number(T v) : n(v) {}
		template <typename T>
		inline operator T() const { return (T)n; }
	};

	// All strings become utf8
	class String {
	public:;
		std::string ss;
		const char *s;

		inline String(const String &src) : ss(src.ss), s(src.s) {
			// if the char str points to where it came from,
			// make it point to our copy
			if (src.ss.c_str() == s)
				s = ss.c_str();
		};

		template <class T,
			 typename std::enable_if<std::is_convertible<T,std::string>::value, int>::type=0>
		inline String(T v) : ss(v) {
			s = ss.c_str();
		}

		template <class T,
			 typename std::enable_if<std::is_convertible<T,std::wstring>::value, int>::type=0>
		inline String(T v) { ss = utf8.to_bytes(v); s = ss.c_str(); }

		inline String(lua_State *L, int idx = -1) {
			size_t sz = 0;
			s = lua_tolstring(L, idx, &sz);
			ss = std::string(s?s:"", sz);
		}




		inline operator const char*() const { 
			// ?Return POD string ONLY if it came to us from Lua.
			//return (s != (ss.c_str()))?s:NULL;
			if (!s) return NULL;
			return ss.c_str();
		}
		inline operator std::string() const { return ss; }
		inline operator std::wstring() const { return std::wstring(utf8.from_bytes(ss)); }
		inline void pushto(lua_State *L) {
			lua_pushlstring(L, ss.c_str(), ss.size());
		}
	};


	// Index is cross-dependent with Value, so we need it declare forward,
	// and implement outside
	struct Value;

	template<typename T>
	struct Index {
		T key;
		GLua &g;
		int tab;

		inline int gtab()
		{
			int t = tab;
			if (t < 0) {
				lua_pushglobaltable(g.L);
				t = g.top();
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
		Value operator[](K key);
	};


	struct Value {
		public:;
		GLua &G;
		int idx;
		inline Value(GLua &g) : G(g), idx(0) {};
		inline Value(GLua &g, int i) : G(g), idx(i) {};

		template <class T,
			 typename std::enable_if<std::is_integral<T>::value,int>::type=0>
		inline operator T() const {
			return Integer(lua_tointeger(G.L, idx));
		}

		template <class T,
			 typename std::enable_if<std::is_floating_point<T>::value, int>::type=0>
		inline operator T() const { return Number(lua_tonumber(G.L, idx)); }

		template <class T,
			 typename std::enable_if<GLua_detail::string<typename std::decay<T>::type>::value, int>::type=0>
		inline operator T() const { return String(G.L, idx); }

		inline operator bool() const { if (!idx) return false; return lua_toboolean(G.L, idx); }

		template <class T,
			 typename std::enable_if<std::is_pointer<T>::value,int>::type=0>
		inline operator T() const { if (!idx) return 0; return (T)lua_topointer(G.L, idx); }

		template <typename... Ts>
		inline Value operator()(const Ts&... values) const {
			int top = G.top();
			lua_pushcfunction(G.L, G.errh);
			G.push(*this);
			G.push(values...);
			int nargs = G.top() - top - 2;
			if (lua_pcall(G.L, nargs, 1, top+1) != LUA_OK)
				return G.pop(2);
			lua_remove(G.L, top+1);
			return G.get();
		}

		template <typename T>
		inline Index<T> operator [](T key) {
			return {key,G,idx};
		}

		Value pop(int n = 1) { return G.pop(n); }
	};

	// finds or creates type, non-specialized.
	int find_type_by_id(size_t hash, const char *name = 0) {
		int typex;
		if (lua_rawgeti(L, LUA_REGISTRYINDEX, hash) == LUA_TNIL && name) {
			pop();
			typex = lua_allocatetypex(L);
			luaL_newmetatable(L, name);
			// index handler
			pushv([](lua_State *L) {
				lua_getmetatable(L, 1);
				lua_pushvalue(L, 2);
				int typ = lua_rawget(L, -2); 
				// invoke getter
				if (typ != LUA_TFUNCTION)
					return 0;
				if (!lua_getupvalue(L, -1, 1))
					return 0;

				// its a function to call, just return it
				if (!lua_toboolean(L, -1)) {
					lua_pop(L, 1);
					return 1;
				}
				lua_pop(L, 1); // pop upvalue, leaving function
				lua_pushvalue(L, 1); // object
				// otherwise a getter
				lua_call(L, 1, 1);
				return 1;
			});
			lua_setfield(L, -2, "__index");

			// newindex handler
			pushv([](lua_State *L) {
				lua_getmetatable(L, 1);
				lua_pushvalue(L, 2);
				// invoke getter
				lua_rawget(L, -2); // fn
				lua_pushvalue(L, 1); // object
				lua_pushvalue(L, 3); // new value
				// otherwise a getter
				lua_call(L, 2, 0);
				return 0;
			});
			lua_setfield(L, -2, "__newindex");

			lua_pushinteger(L, typex);
			lua_setfield(L, -2, "__typex");

			// map hash_code -> typex in registry, too
			lua_pushinteger(L, typex);
			lua_rawseti(L, LUA_REGISTRYINDEX, hash);

			lua_setmetatablex(L, typex);
		} else {
			typex = lua_tointeger(L, -1);
			pop();
		}
		return typex;
	}

	// leaves metatable on stack
	void bind_type(size_t hash, const char *name = 0) {
		lua_getmetatablex(L, find_type_by_id(hash, name));
	}

	inline void pushuv(void **p, const std::type_info& tid) {
		if (!*p) {
			lua_pushnil(L);
			return;
		}
		lua_pushlightuserdatax(L, *p, find_type_by_id(tid.hash_code(), tid.name()));
	}

	// remove all qualifiers, and turn non-pointers into ones
	template <class T,
	typename std::enable_if<
		(!GLua_detail::string<typename std::decay<T>::type>::value) &&
		(!std::is_arithmetic<T>::value) &&
		(!GLua_detail::callable<T>::value)
	, int>::type=0>
	inline void pushv(T &v) {
		using TT = typename std::remove_cv<T>::type;
		if (std::is_pointer<T>::value) {
			pushuv((void**)&v,typeid(TT));
		} else {
			auto p = (void*)&v;
			pushuv(&p,typeid(TT*));
		}
	}

	template <class T,
	typename std::enable_if<
		GLua_detail::callable<T>::value
	, int>::type=0>
	inline void pushv(T lambda) {
		pushv((typename GLua_detail::lambda<T>::f)(lambda));
	}
	template <int idx, typename Tup> void get_arg(int delta, Tup &args) { }
	template <int idx, typename Tup, typename Current, typename... Args>
	void get_arg(int delta, Tup &args) {
		std::get<idx>(args) = get(idx+delta);
		get_arg<idx+1, Tup, Args...>(delta, args);
	}

	// freestanding and lambdas
	template <typename Ret, typename... Args>
	inline void pushv(Ret (*fun)(Args...)) {
		lua_pushboolean(L, 0);
		lua_pushlightuserdata(L, this);
		lua_pushlightuserdata(L, (void*)(fun));

		lua_pushcclosure(L, lua_CFunction([](lua_State *L) {
			using Tup = std::tuple<typename std::remove_reference<Args>::type...>;
			GLua *self = (GLua*)lua_touserdata(L, lua_upvalueindex(2));
			Tup args;
			self->get_arg<0, Tup, Args...>(1, args);
			auto tfn = (decltype(fun))lua_touserdata(L, lua_upvalueindex(3));
			auto temp = std::experimental::apply(tfn, args);
			self->pushv(temp);
			return int(1);
		}), 3);
	}

	// brutally cast one type to another
	template <typename T, typename F>
	inline T anycast(F f) {
		union {
			F ff;
			T tt;
		} u;
		u.ff = f;
		return u.tt;
	}
	
	// member functions
	template <class C, typename Ret, typename... Args>
	inline void pushv(Ret(C::*fun)(Args...)) {
		lua_pushboolean(L, 0);
		lua_pushlightuserdata(L, this);
		lua_pushlightuserdata(L, anycast<void*>(fun));
		lua_pushcclosure(L, lua_CFunction([](lua_State *L) {
			using Tup = std::tuple<typename std::remove_reference<Args>::type...>;
			GLua *self = (GLua*)lua_touserdata(L, lua_upvalueindex(2));
			C *obj = (C*)lua_touserdata(self->L, 1);
			auto tfun = self->anycast<Ret(C::*)(Args...)>(lua_touserdata(L, lua_upvalueindex(3)));
			Tup args;
			self->get_arg<0, Tup, Args...>(2, args);
			auto temp = std::experimental::apply([=](const Args&... args){
				return (obj->*tfun)(args...);
			}, args);
			self->pushv(temp);
			return int(1);
		}), 3);
	}

	template <class T, class C>
	C *getclass(T C::*);

	// getter: fn(obj)
	// setter: fn(obj,nval)
	template <class T, class C>
	inline void pushv(T C::*mem) {
		lua_pushboolean(L, 1);
		lua_pushlightuserdata(L, this);
		lua_pushlightuserdata(L, anycast<void*>(mem));
		lua_pushcclosure(L, lua_CFunction([](lua_State *L) {
			GLua *self = (GLua*)lua_touserdata(L, lua_upvalueindex(2));
			C *obj = (C*)lua_touserdata(L, 1);
			auto tmem = self->anycast<T C::*>(lua_touserdata(L, lua_upvalueindex(3)));

			// getter (obj)
			if (self->top() == 1) {
				// getter
				self->pushv(obj->*tmem);
				return 1;
			}
			// setter (obj,val)
			obj->*tmem = self->get();
			return 0;
		}), 3);
	}

	inline void pushv(lua_CFunction cf) {
		lua_pushcfunction(L, cf);
	}

	inline void pushv() { lua_pushnil(L); }
	inline void pushv(Value v) { assert(v.idx != 0); lua_pushvalue(L, v.idx); }
	inline void pushv(const Number n) { lua_pushnumber(L, n); }
	inline void pushv(const Integer n) {
		lua_pushinteger(L, n); }
	inline void pushv(String s) { s.pushto(L); }

	// bool would eat other types via its implicit casts otherwise, so
	// we have to constrain it to being canonical type
	template <class T, typename std::enable_if<std::is_same<T,bool>::value,int>::type=0>
	inline void pushv(bool b) { lua_pushboolean(L, b); }

	// ellipsis for push
	inline void push() {};

	template <typename T, typename... Ts>
	inline void push(const T &value, const Ts&... values) {
		pushv(value);
		push(values...);
	}

	inline Value pop(int n = 1) { lua_pop(L, n); return Value(*this); }
	inline int top() { return lua_gettop(L); }
	inline void top(int n) { return lua_settop(L, n); }


	inline Value get(int idx = -1) {
		Value v(*this);
		v.idx = idx;
		if (idx < 0)
			v.idx = top() + 1 + idx;
		return v;
	}

	inline void close() {
		lua_close(L);
		L = 0;
	}

	inline ~GLua() {
		close();
	}

	struct Scope {
		struct GLua &parent;
		int stack;
		inline Scope(GLua &p) : parent(p) {
			stack = parent.top();
		}
		inline ~Scope() {
			parent.top(stack);
		}
	};


	// Operators
	//
	inline Value eval(String code) {
		if (luaL_loadstring(L, code) == LUA_ERRSYNTAX) {
			{
				Scope s(*this);
				errh(L);
			}
			return pop();
		}
		return get()();
	}

	template <typename T>
	inline Index<T> operator [](T key) {
		return {key,*this,-1};
	}

	// When a tabindex is placed as function call argument somewhere.
	template <typename T>
	inline void pushv(Index<T> v) {
		auto dummy = Value(v);
	}



	lua_CFunction errh = [](lua_State *L) {
		fprintf(stderr, "Lua error: %s\n", lua_tostring(L, -1));
		return 1;
	};

	std::string errmsg;

/*	template <class T, class C>
	C *getclass(T C::*);*/

	// bind member functions, typeid by containing class typeid
	template <class T, class C>
	inline void bind(const char *clsn, const char *n, T C::*mem) {
		const std::type_info& tid = typeid(C*);
		bind_type(tid.hash_code(), clsn);
		pushv(mem);
		lua_setfield(L, -2, n);
		lua_pop(L, 1);
	}

	// free-standing functions, we have to trust name on this
	template <class T>
	inline void bind(const char *clsn, const char *n, T fun) {
		luaL_newmetatable(L, clsn);
		pushv(fun);
		lua_setfield(L, -2, n);
		lua_pop(L, 1);
		S
	}

};

template<typename T>
template<typename V>
inline V const& GLua::Index<T>::operator=(V const &v)
{
	int t = gtab();
	g.pushv(key);
	g.pushv(v);
	lua_settable(g.L, t);
	if (tab < 0) g.pop();
	return v;
}

template<typename T>
inline GLua::Value GLua::Index<T>::get()
{
	int t = gtab();
	g.pushv(key);
	//printf("getting %s from %d\n", key, t);
	lua_gettable(g.L, t);
	if (tab < 0) lua_remove(g.L, -2); // remove globals
	return g.get();
}

template<typename T>
template <typename... Ts>
inline GLua::Value GLua::Index<T>::operator()(const Ts&... values) {
	return (GLua::Value(*this))(values...);
}

template<typename T>
template<typename K>
GLua::Value GLua::Index<T>::operator[](K key) {
	return (GLua::Value(*this))[key];
}

