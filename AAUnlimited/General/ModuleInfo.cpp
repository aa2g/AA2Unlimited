#include "ModuleInfo.h"
#include "Files/Logger.h"

namespace General {


HINSTANCE DllInst = 0;
DWORD GameBase = 0;
bool IsAAPlay = false;
bool IsAAEdit = false;

std::string AAEditPath;
std::string AAPlayPath;

namespace {
	static const WORD AAPlayResLength = 0x264;
	static const WORD AAEditResLength = 0x284;
	static const DWORD AANameOffset = 0xF4;
	//name and file version in here
	static const unsigned char AAEditName[] = {
		0xB8, 0x30, 0xF3, 0x30, 0xB3, 0x30, 0xA6, 0x30, 0xAC, 0x30, 0xAF, 0x30, 0xA8, 0x30, 0xF3, 0x30, 0x12,
		0xFF, 0x20, 0x00, 0x4D, 0x30, 0x83, 0x30, 0x89, 0x30, 0x81, 0x30, 0x44, 0x30, 0x4F, 0x30, 0x00,
		0x00, 0x00, 0x00, 0x2C, 0x00, 0x06, 0x00, 0x01, 0x00, 0x46, 0x00, 0x69, 0x00, 0x6C, 0x00, 0x65,
		0x00, 0x56, 0x00, 0x65, 0x00, 0x72, 0x00, 0x73, 0x00, 0x69, 0x00, 0x6F, 0x00, 0x6E, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x31, 0x00, 0x2E, 0x00, 0x30, 0x00, 0x2E, 0x00, 0x31, 0x00, 0x00
	};
	static const unsigned char AAPlayName[] = {
		0xB8, 0x30, 0xF3, 0x30, 0xB3, 0x30, 0xA6, 0x30, 0xAC, 0x30, 0xAF, 0x30, 0xA8, 0x30, 0xF3, 0x30,
		0x12, 0xFF, 0x00, 0x00, 0x2C, 0x00, 0x06, 0x00, 0x01, 0x00, 0x46, 0x00, 0x69, 0x00, 0x6C, 0x00,
		0x65, 0x00, 0x56, 0x00, 0x65, 0x00, 0x72, 0x00, 0x73, 0x00, 0x69, 0x00, 0x6F, 0x00, 0x6E, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x32, 0x00, 0x2E, 0x00, 0x30, 0x00, 0x2E, 0x00, 0x31, 0x00, 0x00
	};
}

bool Initialize() {
	GameBase = (DWORD)GetModuleHandle(NULL);

	//try to find out what program we were attached to. im not going to do this the correct way.
	//im just gonna load the version resource and compare predefined bytes with hardcoded references.
	{

		HRSRC version = FindResource((HMODULE)GameBase, MAKEINTRESOURCE(VS_VERSION_INFO), MAKEINTRESOURCE(RT_VERSION));
		if (version == NULL) {
			MessageBox(NULL, TEXT("Failed to get version resource of executable that we were injected in. ")
				TEXT("We probably were not injected into AAPlay or AAEdit.\r\n"), TEXT("Critical Error"), 0);
			return false;
		}
		HGLOBAL resource = LoadResource((HMODULE)GameBase, version);
		if (resource == NULL) {
			int lastError = GetLastError();
			MessageBox(NULL, TEXT("Failed to load version resource."), TEXT("Critical Error"), 0);
			return false;
		}

		DWORD size = SizeofResource((HMODULE)GameBase, version);
		LPVOID buffer = LockResource(resource);
		if (size == 0 || buffer == NULL) {
			MessageBox(NULL, TEXT("Failed to copy resource data.\r\n"), TEXT("Critical Error"), 0);
			return false;
		}

		WORD length = *(WORD*)(buffer);
		if (length == AAEditResLength && memcmp(((BYTE*)buffer) + AANameOffset, AAEditName, sizeof(AAEditName)) == 0) {
			IsAAEdit = true;
		}
		else if (length == AAPlayResLength && memcmp(((BYTE*)buffer) + AANameOffset, AAPlayName, sizeof(AAPlayName)) == 0) {
			IsAAPlay = true;
		}
		else {
			MessageBox(NULL, TEXT("Resource Data does not match AAEdit or AAPlay.\r\n"), TEXT("Critical Error"), 0);
			return false;
		}
	}

	// find paths in registry
	{
		BYTE buffer[512];
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
		RegQueryValueEx(playKey, TEXT("INSTALLDIR"), NULL, &keyType, buffer, &outSize);
		if (keyType != REG_SZ || outSize < 2) {
			MessageBox(NULL,TEXT("Could not find AAPlay INSTALLDIR key in registry") , TEXT("Critical Error"), 0);
			return false;
		}
		if (buffer[outSize - 1] != '\0') { buffer[outSize - 1] = '\0'; outSize++; }
		AAPlayPath = std::string((char*)buffer);
		if (buffer[outSize - 2] != '\\') AAPlayPath.push_back('\\');

		//aaedit path
		HKEY editKey;

		res = RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\illusion\\AA2Edit"), 0, KEY_READ, &editKey);
		if (res != ERROR_SUCCESS) {
			MessageBox(NULL, TEXT("Could not find AAEdit path in registry"), TEXT("Critical Error"), 0);
			return false;
		}
		outSize = sizeof(buffer);
		RegQueryValueEx(editKey, TEXT("INSTALLDIR"), NULL, &keyType, buffer, &outSize);
		if (keyType != REG_SZ || outSize < 2) {
			MessageBox(NULL, TEXT("Could not find AAEdit INSTALLDIR key in registry"), TEXT("Critical Error"), 0);
			return false;
		}
		if (buffer[outSize - 1] != '\0') { buffer[outSize - 1] = '\0'; outSize++; }
		AAEditPath = std::string((char*)buffer);
		if (buffer[outSize - 2] != '\\') AAEditPath.push_back('\\');

	}

	return true;
}


}