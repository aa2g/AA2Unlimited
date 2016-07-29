#include "Error.h"
#include <Windows.h>
#include <stdio.h>

namespace {
	int loc_lastError = 0;
	const char* loc_context = NULL;
};

namespace Error {

void SetLastError(int error,const char* context) {
	loc_lastError = error;
	loc_context = context;
}

/*
 * Prints the following error based on the value of SetLastError:
 * [
 *		<prefix>
 *
 *		"<context>" failed with code <code>: "<error-msg>".\r\n
 *
 *		<postfix>
 * ]
 */
void PrintLastError(const char* prefix, const char* postfix) {
	if (prefix == NULL) prefix = "";
	if (postfix == NULL) postfix = "";
	if (loc_context == NULL) loc_context = "";
	int prefixlen = strlen(prefix);
	int postfixlen = strlen(postfix);
	int contextlen = strlen(loc_context);

	//get error-msg
	char* errbuffer = new char[1024];
	DWORD errsize;

	errsize = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,loc_lastError,MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),errbuffer,1024,NULL);
	if(errsize == 0) {
		int error = GetLastError();
		sprintf_s(errbuffer,1024,"<formaterror:%d>",error);
	}

	const char formatstr[] = "%s\r\n\r\n\"%s\" failed with code %d: \r\n%s\r\n%s";
	//9.223.372.036.854.775.807 = max 64 bit number, thats 19 numbers (its a 32 bit int. just for safety tho.).
	DWORD buffersize = sizeof(formatstr) + prefixlen + contextlen + 19 + errsize + postfixlen;
	char* buffer = new char[buffersize];
	sprintf_s(buffer,buffersize, formatstr,prefix,loc_context,loc_lastError,errbuffer,postfix);

	MessageBoxA(NULL,buffer,"Error",MB_ICONERROR);

	delete[] buffer;
	delete[] errbuffer;
}

}