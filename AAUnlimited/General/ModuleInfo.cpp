#include "ModuleInfo.h"
#include "Files/Logger.h"

#pragma comment( lib, "version.lib" )

#include <string>
#include <sstream>

namespace General {


HINSTANCE DllInst = 0;
DWORD GameBase = 0;
bool IsAAPlay = false;
bool IsAAEdit = false;

std::wstring AAEditPath;
std::wstring AAPlayPath;
std::wstring GameExeName;

namespace {
	wchar_t AAPlayVersion[] = L"2.0.1";
	wchar_t AAEditVersion[] = L"1.0.1";
	wchar_t AAPlayProductName[] = L"ジンコウガクエン２";
	wchar_t AAPlayASUProductName[] = L"Artificial Academy 2";
	wchar_t AAEditProductName[] = L"ジンコウガクエン２ きゃらめいく";
}

bool Initialize() {
	GameBase = (DWORD)GetModuleHandle(NULL);

	// find paths in registry
	{
		wchar_t buffer[512];
		DWORD keyType;
		DWORD outSize;
		LONG res;

		//aaplay path
		HKEY playKey;

		res = RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\illusion\\AA2Play"), 0, KEY_READ, &playKey);
		if (res != ERROR_SUCCESS) {
			MessageBox(NULL, TEXT("Could not find AAPlay path in registry"), TEXT("Critical Error"), 0);
			return false;
		}
		outSize = sizeof(buffer);
		RegQueryValueEx(playKey, TEXT("INSTALLDIR"), NULL, &keyType, (BYTE*)buffer, &outSize);
		outSize /= 2;
		if (keyType != REG_SZ || outSize < 2) {
			MessageBox(NULL, TEXT("Could not find AAPlay INSTALLDIR key in registry"), TEXT("Critical Error"), 0);
			return false;
		}
		if (buffer[outSize - 1] != '\0') { buffer[outSize] = '\0'; outSize++; }
		AAPlayPath = std::wstring((TCHAR*)buffer);
		if (buffer[outSize - 2] != L'\\') AAPlayPath.push_back(L'\\');

		//aaedit path
		HKEY editKey;

		res = RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\illusion\\AA2Edit"), 0, KEY_READ, &editKey);
		if (res != ERROR_SUCCESS) {
			MessageBox(NULL, TEXT("Could not find AAEdit path in registry"), TEXT("Critical Error"), 0);
			return false;
		}
		outSize = sizeof(buffer);
		RegQueryValueEx(editKey, TEXT("INSTALLDIR"), NULL, &keyType, (BYTE*)buffer, &outSize);
		outSize /= 2;
		if (keyType != REG_SZ || outSize < 2) {
			MessageBox(NULL, TEXT("Could not find AAEdit INSTALLDIR key in registry"), TEXT("Critical Error"), 0);
			return false;
		}
		if (buffer[outSize - 1] != '\0') { buffer[outSize] = '\0'; outSize++; }
		AAEditPath = std::wstring((TCHAR*)buffer);
		if (buffer[outSize - 2] != L'\\') AAEditPath.push_back(L'\\');

	}

	//get name of exe we are in
	{
		TCHAR buffer[512];
		GetModuleFileName(GetModuleHandle(NULL), buffer, 512);
		GameExeName = buffer;
	}

	//try to check the resource file to figure out where we are
	
	{
		std::wstringstream resourceInfo(L""); //keps additional information in case of fail
		DWORD legacy;
		DWORD size = GetFileVersionInfoSize(GameExeName.c_str(), &legacy);
		if (size == 0) {
			int error = GetLastError();
			resourceInfo << L"Failed to load version resource (error " + error << L")";
		}
		else {
			BYTE* buffer = new BYTE[size];
			GetFileVersionInfo(GameExeName.c_str(), NULL, size, buffer);
			VS_FIXEDFILEINFO* info = NULL;
			UINT infoSize;
			VerQueryValue(buffer, TEXT("\\"), (void**)&info, &infoSize);
			wchar_t* strFileVersion = NULL;
			UINT strFileVersionLength;
			wchar_t* strProductName = NULL;
			UINT strProductNameLength;
			//note: version is japanese (0411), codepage unicode (04B0)
			BOOL suc = TRUE;
			suc = suc && VerQueryValue(buffer, TEXT("\\StringFileInfo\\041104b0\\FileVersion"), (void**)&strFileVersion, &strFileVersionLength);
			suc = suc && VerQueryValue(buffer, TEXT("\\StringFileInfo\\041104b0\\ProductName"), (void**)&strProductName, &strProductNameLength);
			if (suc) {
				bool editVersion = false, editName = false,
					playVersion = false, playName = false;
				if (wcscmp(AAPlayVersion, strFileVersion) == 0) playVersion = true;
				if (wcscmp(AAEditVersion, strFileVersion) == 0) editVersion = true;
				if (wcscmp(AAPlayProductName, strProductName) == 0) playName = true;
				if (wcscmp(AAPlayASUProductName, strProductName) == 0) playName = true;
				if (wcscmp(AAEditProductName, strProductName) == 0) editName = true;
				if (editVersion && editName) IsAAEdit = true;
				else if (playVersion && playName) IsAAPlay = true;
				else {
					resourceInfo << L"Resource data unconclusive:\r\n\tVersion: " << strFileVersion;
					if		(editVersion) resourceInfo << L" (Version of AAEdit)";
					else if (playVersion) resourceInfo << L" (Version of AAPlay)";
					else				  resourceInfo << L" (unknown version number; expected " << AAPlayVersion << L" for play or " << AAEditVersion << L" for edit)";
					resourceInfo << L"\r\n\tProduct Name: " << strProductName;
					if		(editName) resourceInfo << L" (Name of AAEdit)";
					else if (playName) resourceInfo << L" (Name of AAPlay)";
					else			   resourceInfo << L" (unknown Name; expected " << AAPlayProductName << L" for play or " << AAEditProductName << L" for edit)";
				}
			}
			delete[] buffer;
		}

		if (!IsAAEdit && !IsAAPlay) {
			std::wstringstream msg(L"Resource Analysis was inconclusive. Information:\r\n");
			msg << resourceInfo.str() << L"\r\n\r\n";
			msg << L"With that, we do not know which program we were injected into. If YOU know, you may now choose manually where we are.\r\n"
				L"Keep in mind that a wrong choice will crash the Program. To choose are we in AAPlay?:\r\n"
				L"Press YES if we are in AAPLAY (the Game)\r\n"
				L"Press NO if we are in AAEDIT (the Maker)\r\n"
				L"Press CANCEL to abort\r\n";
			int ret = MessageBoxW(NULL, msg.str().c_str(), L"Resource Analysis failed", MB_YESNOCANCEL);
			if (ret == IDYES) {
				IsAAPlay = true;
			}
			else if (ret == IDNO) {
				IsAAEdit = true;
			}
			else {
				//failed
				return false;
			}
		}
	}

	

	return true;
}


}