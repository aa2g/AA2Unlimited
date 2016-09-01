#include "PathInfo.h"

#include <Windows.h>

std::string g_AAPlayPath;
std::string g_AAEditPath;

static std::string getInstallPath( LPCSTR key_name, LPCSTR ErrorNoFound, LPCSTR ErrorNoDir ) {
	//Get the key containing the game settings
	HKEY key;
	auto res = RegOpenKeyEx( HKEY_CURRENT_USER, key_name, 0, KEY_READ, &key );
	if( res != ERROR_SUCCESS ) {
		MessageBox( NULL, ErrorNoFound, TEXT( "Critical Error" ), 0 );
		return {};
	}

	//Get the string containing the directory path to the game
	DWORD keyType;
	BYTE buffer[512];
	DWORD outSize = sizeof( buffer );
	RegQueryValueEx( key, TEXT( "INSTALLDIR" ), NULL, &keyType, buffer, &outSize );
	if( keyType != REG_SZ || outSize == 0 ) {
		MessageBox( NULL, ErrorNoDir, TEXT( "Critical Error" ), 0 );
		return {};
	}

	//Convert it to a "\\" terminated string
	if( buffer[outSize - 1] == '\0' ) outSize--;
	std::string out{ reinterpret_cast<char*>(buffer), outSize };
	return (out.back()=='\\') ? out : out+'\\';
}

bool GetPathsFromRegistry() {
	g_AAPlayPath = getInstallPath( TEXT( "SOFTWARE\\illusion\\AA2Play" ), TEXT( "Could not find AAPlay path in registry" ), TEXT( "Could not find AAPlay INSTALLDIR key in registry" ) );
	if( g_AAPlayPath.empty() ) return false;

	g_AAEditPath = getInstallPath( TEXT( "SOFTWARE\\illusion\\AA2Edit" ), TEXT( "Could not find AAEdit path in registry" ), TEXT( "Could not find AAEdit INSTALLDIR key in registry" ) );
	if( g_AAEditPath.empty() ) return false;

	return true;
}

static std::vector<std::string> getDirectoryContents( std::string filter ) {
	WIN32_FIND_DATA data;
	HANDLE hSearch = FindFirstFile( filter.c_str(), &data );

	if( hSearch == INVALID_HANDLE_VALUE ) return {};

	std::vector<std::string> list;
	do {
		list.push_back( data.cFileName );
	} while( FindNextFile( hSearch, &data ) != FALSE );
	FindClose( hSearch );
	return list;
}

std::vector<std::string> GetPossiblePlayExeList() {
	return getDirectoryContents( g_AAPlayPath + "*.exe" );
}

std::vector<std::string> GetPossibleEditExeList() {
	return getDirectoryContents( g_AAEditPath + "*.exe" );
}