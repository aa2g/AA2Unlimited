#pragma once
#include <fstream>
#include <Windows.h>

#define LOGPRIONC(prio) if(g_Logger.FilterPriority(prio)) g_Logger << prio << 
#define LOGPRIOC(prio) if(g_Logger.FilterPriority(prio)) g_Logger << prio << __FUNCSIC __ ": " <<
#define LOGPRIO(prio) if(g_Logger.FilterPriority(prio)) g_Logger << prio << \
	(g_Logger.FilterPriority(Logger::Priority::SPAM) ? ("[" __FUNCSIG__ "]:\r\n\t") : "")


/*
 * Logger class.
 * Can have a current priority state that is set by outputting a priority (logger << SPAM),
 * as well as a filter priority. If the output priority is not covered by the filter, no output
 * will be performed.
 */

class Logger
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
		}
		outfile.flush();
		return *this;
	}

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

	void SetPriority(Priority prio);
	//returns true if the logger would print messages for the given priority
	bool FilterPriority(Priority prio);
private:
	std::ofstream outfile;
	Priority filter;
	Priority currPrio;
};

extern Logger g_Logger;