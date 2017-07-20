#pragma once
#include <fstream>
#include <sstream>
#include <Windows.h>
#include "Script/ScriptLua.h"

#define LOGPRIONC(prio) if(g_Logger.FilterPriority(prio)) g_Logger << prio << 
#define LOGPRIOC(prio) if(g_Logger.FilterPriority(prio)) g_Logger << prio << __FUNCSIG__ ": " <<
#define LOGPRIO(prio) if(g_Logger.FilterPriority(prio)) g_Logger << prio << \
	(g_Logger.FilterPriority(Logger::Priority::SPAM) ? ("[" __FUNCSIG__ "]:\r\n\t") : "")


/*
 * Logger class.
 * Can have a current priority state that is set by outputting a priority (logger << SPAM),
 * as well as a filter priority. If the output priority is not covered by the filter, no output
 * will be performed.
 */

extern class Logger
{
public:
	enum class Priority {
		SPAM,
		INFO,
		WARN,
		ERR,
		CRIT_ERR
	};
public:
	Logger();
	Logger(const TCHAR* file,Priority prio);
	~Logger();
	void Initialize(const TCHAR * file, Priority prio);

	template<typename T>
	Logger& operator<<(const T& p) {
		if (outfile.good() && currPrio >= filter) {
			outfile << p;
			outbuf << p;
		}
		flush();
		outfile.flush();
		return *this;
	}
	void flush();
	template<>
	Logger& operator<<(const Priority& prio) {
		currPrio = prio;
		switch (prio) {
		case Priority::SPAM:
			*this << "[SPAM] ";
			break;
		case Priority::INFO:
			*this << "[INFO] ";
			break;
		case Priority::WARN:
			*this << "[WARNING] ";
			break;
		case Priority::ERR:
			*this << "[ERROR] ";
			break;
		case Priority::CRIT_ERR:
			*this << "[CRITICAL ERROR] ";
			break;
		}
		return *this;
	}

	Logger& operator<<(const std::wstring& str) {
		std::string cstr(str.begin(),str.end());
		outfile << cstr.c_str();
		outfile.flush();
		return *this;
	}

	Logger& operator<<(const BYTE b) {
		*this << (int)b;
		return *this;
	}

	static inline void bindLua() {
		// The functions we want to bind are too snow-flakeish, so we have to do that
		// by hand.
		auto b = g_Lua[LUA_BINDING_TABLE];
		b["setlogprio"] = ([](int n) {
			g_Logger.SetPriority(Logger::Priority(n));
		});

		b["logger"] = lua_CFunction([](lua_State *L) {
			Logger::Priority prio = (Logger::Priority)luaL_checkinteger(L, 1);
			int top = lua_gettop(L);
			for (int i = 2; i <= top; i++) {
				g_Logger << prio << luaL_checkstring(L, i) << "\r\n";
			}
			return 0;
		});
	}

	void SetPriority(Priority prio);
	//returns true if the logger would print messages for the given priority
	bool FilterPriority(Priority prio);
private:
	std::ofstream outfile;
	std::ostringstream outbuf;
	Priority filter;
	Priority currPrio;
} g_Logger;


