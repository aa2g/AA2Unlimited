#include "ModuleFile.h"

#include "Logger.h"
#include "Functions\Serialize.h"
#include "General\Util.h"
using namespace Serialize;

ModuleFile::ModuleFile(const std::wstring & name,const std::wstring description,const std::vector<Shared::Triggers::Trigger*>& triggers)
	: name(name), description(description), good(false)
{
	this->triggers.reserve(triggers.size());
	for(Shared::Triggers::Trigger* t : triggers) {
		this->triggers.push_back(*t);
	}
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
		this->name = ReadData<std::wstring>(bufCpy,sizeCpy);
		this->description = ReadData<std::wstring>(bufCpy,sizeCpy);
		this->triggers = ReadData<decltype(triggers)>(bufCpy,sizeCpy);
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
	char* buffer = NULL;
	int size = 0;
	int at = 0;

	WriteData(&buffer,&size,at,name,true);
	WriteData(&buffer,&size,at,description,true);
	WriteData(&buffer,&size,at,triggers,true);

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

