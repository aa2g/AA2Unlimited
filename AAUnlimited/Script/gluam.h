#pragma once

// Basic member binding, accessor for both reads and writes.
#define LUA_BIND(var) \
GLUA_BIND(LUA_GLOBAL, ACCESOR, LUA_CLASS, var, { \
	if (_gl.top() == 1) { \
		_gl.push(_self->var); \
		return 1; \
	} \
	_self->var = _gl.get(2); \
});

// Bind as a pointer to POD. Getter only.
#define LUA_BINDP(var) \
GLUA_BIND(LUA_GLOBAL, ACCESOR, LUA_CLASS, var, { \
	_gl.push(&_self->var); \
	return 1; \
});

// Custom method
#define LUA_METHOD(var, fn) \
GLUA_BIND(LUA_GLOBAL, METHOD, LUA_CLASS, var, fn);

// Call a C++ getter method, 0 args, 1 result
#define LUA_MGETTER0(var) \
GLUA_BIND(LUA_GLOBAL, METHOD, LUA_CLASS, var, { \
	return _gl.push(_self->var()).one; \
});

// 1 arg
#define LUA_MGETTER1(var) \
GLUA_BIND(LUA_GLOBAL, METHOD, LUA_CLASS, var, { \
	return _gl.push(_self->var(_gl.get(2))).one; \
});

// 2 arg
#define LUA_MGETTER2(var) \
GLUA_BIND(LUA_GLOBAL, METHOD, LUA_CLASS, var, { \
	return _gl.push(_self->var(_gl.get(2), _gl.get(3))).one; \
});


// get/set struct embedded byte string (ie fix buffer, not a pointer)
#define LUA_BINDSTR(var) \
GLUA_BIND(LUA_GLOBAL, ACCESOR, LUA_CLASS, var, { \
        if (_gl.top() == 1) { \
                _gl.push((const char*)_self->var); \
                return 1; \
        } \
	_gl.gets(2, (char*)_self->var, sizeof(_self->var)); \
});

// read-only asciiz string (can be both pointer or buffer)
#define LUA_BINDSTRP(var) \
GLUA_BIND(LUA_GLOBAL, ACCESOR, LUA_CLASS, var, { \
        _gl.push((const char*)_self->var); \
        return 1; \
});

// extended array reference. sub can be used for partial keying / cherrypicking into sub-structs.
// limit is to customize array bounds (can refer to other members via _self->)
#define LUA_BINDARRE(var,sub,limit) \
GLUA_BIND(LUA_GLOBAL, METHOD, LUA_CLASS, var, { \
        unsigned _idx = _gl.get(2); \
        if (_idx >= limit) return 0; \
        if (_gl.top() == 2) { \
                _gl.push(_self->var sub[_idx]); \
                return 1; \
        } \
        _self->var sub[_idx] = _gl.get(3); \
});

#define LUA_ARRAYLEN(n) (sizeof((n))/sizeof((n)[0]))

// bind a struct embedded array. BINDARR won't work, because its
// setter would have to return unwrapped arrays difficult to deal with
#define LUA_BINDARREP(var,sub,limit) \
GLUA_BIND(LUA_GLOBAL, METHOD, LUA_CLASS, var, { \
        unsigned _idx = _gl.get(2); \
        if (_idx >= limit) return 0; \
        _gl.push(&_self->var[_idx]); \
        return 1; \
});

// the simple array binding used most of the time
#define LUA_BINDARR(var) \
        LUA_BINDARRE(var,,LUA_ARRAYLEN(_self->var))

#define LUA_BINDARRP(var) \
        LUA_BINDARREP(var,,LUA_ARRAYLEN(_self->var))


#define LUA_XSTR(a) LUA_STR(a)
#define LUA_STR(a) #a
#define LUA_NAME \
	LUA_GLOBAL.setname<LUA_CLASS>(LUA_XSTR(LUA_CLASS)); \
	LUA_METHOD(dump, { \
		_gl.pushlstring((const char*)_self, sizeof(*_self)); \
		return 1; \
	});
