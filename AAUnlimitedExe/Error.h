#pragma once
#include <Windows.h>

namespace Error {
	void PrintLastError(const char* prefix = NULL, const char* postfix = NULL);
	void SetLastError(int error,const char* context);
}