#include <Windows.h>
#include "Logger.h"

#include <string>

Logger g_Logger;

Logger::Logger() : currPrio(Priority::ERR), filter(Priority::SPAM) {

}

Logger::Logger(const TCHAR * file,Priority prio) : currPrio(Priority::ERR),filter(prio)
{
	Initialize(file, prio);
}

void Logger::Initialize(const TCHAR * file, Priority prio) {
	outfile.open(file);
	if (!outfile.good()) {
		MessageBox(0, (std::wstring(TEXT("Could not open Logfile ")) + file).c_str(), TEXT("Error"), 0);
	}
}

Logger::~Logger()
{
	outfile.close();
}

void Logger::SetPriority(Priority prio) {
	filter = prio;
	*this << "------- applied priority " << prio << " -------\r\n";
}

/* Returns true if the given priority is covered by the current priority, or else false */
bool Logger::FilterPriority(Priority prio)
{
	return prio >= filter;
}
