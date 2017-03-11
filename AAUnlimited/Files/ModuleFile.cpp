#include "ModuleFile.h"

#include "Logger.h"
#include "Functions\Serialize.h"
#include "General\Util.h"
using namespace Serialize;

ModuleFile::ModuleFile(const Shared::Triggers::Module& mod)
{
	good = false;
	this->mod = mod;
	good = true;
}

void ModuleFile::FromFile(const TCHAR* path) {
	good = false;
	HANDLE file = CreateFile(path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
	if(file == NULL || file == INVALID_HANDLE_VALUE) {
		return;
	}
	DWORD lo,hi;
	lo = GetFileSize(file,&hi);
	if(lo == 0) {
		CloseHandle(file);
		return;
	}
	char* buffer = new char[lo];
	ReadFile(file,buffer,lo,&hi,NULL);

	char* bufCpy = buffer;
	int sizeCpy = lo;
	try {
		mod = ReadData<decltype(mod)>(bufCpy,sizeCpy);
		good = true;
	}
	catch(InsufficientBufferException e) {
		good = false;
	}
	catch(std::exception e) {
		good = false;
	}

	delete[] buffer;
	CloseHandle(file);
}

void ModuleFile::WriteToFile(const TCHAR* path) {
	if (!good) return;
	int size,at;
	char* buffer = ToBuffer(size,at);
	if (!buffer) return;

	General::CreatePathForFile(path);
	HANDLE file = CreateFile(path,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,0,NULL);
	if(file == NULL || file == INVALID_HANDLE_VALUE) {
		int error = GetLastError();
		delete[] buffer;
		LOGPRIO(Logger::Priority::WARN) << "Tried to write to file " << path << ", but could not open file; error " << error << "\r\n";
		return;
	}
	DWORD written;
	WriteFile(file,buffer,at,&written,NULL);
	CloseHandle(file);

	delete[] buffer;

}

char * ModuleFile::ToBuffer(int& size,int& at) {
	if (!good) return NULL;
	char* buffer = NULL;
	size = 0;
	at = 0;

	WriteData(&buffer,&size,at,mod,true);
	return buffer;
}

