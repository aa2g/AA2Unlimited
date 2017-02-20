#include <Windows.h>
#include "Config.h"
#include "Logger.h"

const Config::MemberInfo Config::knownMembers[num_elements] = {
	MemberInfo(Config::BOOL,	"bUseAdditionalTanSlots", false),	//USE_TAN_SLOTS
	MemberInfo(Config::BOOL,	"bUseMeshTextureOverrides", false),	//USE_MESH_TEXTURE_OVERRIDES
	MemberInfo(Config::BOOL,	"bUseHAi", false),	//USE_H_AI
	MemberInfo(Config::BOOL,	"bUseShadowing", false),	//USE_SHADOWING
	MemberInfo(Config::BOOL,	"bEnableFacecam", false), //USE_H_FACECAM
	MemberInfo(Config::BOOL,	"bEnableHPosButtonReorder", false), //USE_H_POS_BUTTON_MOVE
	MemberInfo(Config::BOOL,	"bUseClothesPoser", false), //USE_POSER_CLOTHES
	MemberInfo(Config::BOOL,	"bUseDialoguePoser", false), //USE_POSER_DIALOGUE
	MemberInfo(Config::BOOL,	"bNoPromptHIsRape", false),	//NO_PROMPT_IS_FORCE
	MemberInfo(Config::BOOL,	"bHAiOnNoPromptH", false),	//HAI_ON_NO_PROMPT
	MemberInfo(Config::INT,		"savedEyeTextureUsage", 1),	//SAVED_EYE_TEXTURE_USAGE,	
	MemberInfo(Config::INT,		"savedFileUsage", 1),	////SAVED_FILE_USAGE,
	MemberInfo(Config::BOOL,	"bSaveFileAutoRemove", false),	//SAVED_FILE_REMOVE,	
	MemberInfo(Config::BOOL,	"bSaveFileBackup",true), //SAVED_FILE_BACKUP
	MemberInfo(Config::INT,		"facelessMaleSlot", 90),	//FACELESS_SLOT_MALE,	
	MemberInfo(Config::INT,		"facelessFemaleSlot", 7),	//FACELESS_SLOT_FEMALE
	MemberInfo(Config::FLOAT,	"fPOVOffsetX", 0.0f),	//POV_OFFSET_X
	MemberInfo(Config::FLOAT,	"fPOVOffsetY", 0.0f),	//POV_OFFSET_Y
	MemberInfo(Config::FLOAT,	"fPOVOffsetZ", 0.0f),	//POV_OFFSET_Z
	MemberInfo(Config::INT,		"legacyMode", 0),	//LEGACY_MODE
	MemberInfo(Config::BOOL,		"bPoserHotkeys", false),	//USE_POSER_HOTKEYS
	MemberInfo(Config::INT,		"hkPoserTranslate", 0x57),	//HKEY_POSER_TRANSLATE
	MemberInfo(Config::INT,		"hkPoserRotate", 0x45),	//HKEY_POSER_ROTATE
	MemberInfo(Config::INT,		"hkPoserScale", 0x52)	//HKEY_POSER_SCALE
};

Config g_Config;

Config::Config() {
	//initialize with default values
	for (int i = 0; i < num_elements; i++) {
		switch (knownMembers[i].type) {
		case BOOL:
			m_members[i].bVal = knownMembers[i].data.bVal;
			break;
		case INT:
			m_members[i].iVal = knownMembers[i].data.iVal;
			break;
		case FLOAT:
			m_members[i].fVal = knownMembers[i].data.fVal;
			break;
		case STRING:
			m_members[i].sVal = knownMembers[i].data.sVal;
			break;
		}

	}
}

Config::Config(const TCHAR* path) : Config()
{
	HANDLE hFile = CreateFile(path,FILE_READ_ACCESS,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
	if (hFile == NULL ||hFile == INVALID_HANDLE_VALUE) {
		LOGPRIO(Logger::Priority::ERR) << "could not config open file " << path << "\r\n";
		return;
	}
	DWORD size,hi;
	size = GetFileSize(hFile,&hi);
	if (size == 0) {
		//empty config file
		LOGPRIO(Logger::Priority::WARN) << "config file appears to be empty\r\n";
		CloseHandle(hFile);
		return;
	}
	char* buffer = new char[size+1];
	ReadFile(hFile,buffer,size,&hi,NULL);
	CloseHandle(hFile);
	buffer[size] = '\0';

	//read line by line for value changes
	char* it = NULL,*nextLine = buffer;
	do {
		it = nextLine;
		GetLine(it,&nextLine);
		if (*it == '\0') continue;
		//analyse line
		char* varName;
		char* value;
		//skip whitespaces till name
		while (isspace(*it) && *it) it++;
		if (*it == '\0') continue; //was just line of whitespaces
		varName = it;
		//skip till equals or whitespaces (after name)
		while (!isspace(*it) && *it && *it != '=') it++;
		if (*it == '\0') {
			LOGPRIO(Logger::Priority::WARN) << "varaible without assigned value in config file: " << varName << "\r\n";
			continue;
		}
		//nullterminate; now we have varName
		char old = *it; *it = '\0';
		it++;
		//skip over equals sign
		if (old != '=') {
			while (*it != '=' && *it) it++;
			it++;
		}
		//skip whitespaces till value
		while (isspace(*it) && *it) it++;
		if (*it == '\0') {
			LOGPRIO(Logger::Priority::WARN) << "empty assignment statement in config file for variable: " << varName << "\r\n";
			continue;
		}
		//rest is value
		value = it;

		//find name in list of known variables
		int index = -1;
		for (int i = 0; i < num_elements; i++) {
			if (strcmp(knownMembers[i].name,varName) == 0) {
				index = i;
				break;
			}
		}
		if (index == -1) {
			LOGPRIO(Logger::Priority::WARN) << "unkown variable in config file: " << varName << "\r\n";
			continue;
		}

		//interpret value depending on type
		switch (knownMembers[index].type) {
		case BOOL:
			if (StartsWith(value,"true")) {
				m_members[index].bVal = true;
			}
			else if (StartsWith(value,"false")) {
				m_members[index].bVal = false;
			}
			else {
				LOGPRIO(Logger::Priority::WARN) << "invalid boolean parameter " << value << "\r\n";
			}
			break;
		case INT:
			m_members[index].iVal = strtol(value,NULL,0);
			break;
		case FLOAT:
			m_members[index].fVal = strtof(value,NULL);
			break;
		case STRING: {
			//find starting and closing "s
			char* start,*end;
			while (*it != '"' && it != '\0') it++;
			if (it == '\0') {
				LOGPRIO(Logger::Priority::WARN) << "invalid string parameter; no opening '\"' in " << value << "\r\n";
				break;
			}
			it++;
			start = it;
			while (*it != '"' && it != '\0') it++;
			if (it == '\0') {
				LOGPRIO(Logger::Priority::WARN) << "invalid string parameter; no closing '\"' in " << value << "\r\n";
				break;
			}
			end = it-1;
			*it = '\0';
			m_members[index].sVal = _strdup(start);
			break; }
		}
	} while (nextLine != NULL);
	delete[] buffer;
}

Config::Config(Config & rhs)
{
	for (int i = 0; i < num_elements; i++) {
		if (knownMembers[i].type == STRING && rhs.m_members[i].sVal != knownMembers[i].data.sVal) {
			//make copy of string if its not the default one
			m_members[i].sVal = rhs.m_members[i].sVal;
		}
		else {
			m_members[i] = rhs.m_members[i];
		}
	}
}

Config::Config(Config && rhs)
{
	for (int i = 0; i < num_elements; i++) {
		m_members[i] = rhs.m_members[i];
		rhs.m_members[i].sVal = NULL;
	}
}

Config & Config::operator=(Config & rhs)
{
	for (int i = 0; i < num_elements; i++) {
		if (knownMembers[i].type == STRING && rhs.m_members[i].sVal != knownMembers[i].data.sVal) {
			//make copy of string if its not the default one
			m_members[i].sVal = rhs.m_members[i].sVal;
		}
		else {
			m_members[i] = rhs.m_members[i];
		}
	}
	return *this;
}

Config & Config::operator=(Config && rhs)
{
	for (int i = 0; i < num_elements; i++) {
		m_members[i] = rhs.m_members[i];
		rhs.m_members[i].sVal = NULL;
	}
	return *this;
}


Config::~Config()
{
	for (int i = 0; i < num_elements; i++) {
		if (knownMembers[i].type == STRING && m_members[i].sVal != knownMembers[i].data.sVal) {
			delete m_members[i].sVal;
			m_members[i].sVal = NULL;
		}
	}
}


/*
* Checks if the string at str starts with word
*/
bool Config::StartsWith(const char * str,const char * word)
{
	while (*word && *str && (*str++ == *word++));
	if (*word == '\0') return true;
	return false;
}

/*
* Modifies str so that it points to a single line, comments (;) or newlines filtered out.
* if nextLine != NULL, *nextLine becomes the start of the next line, or NULL if end was reached
*/
void Config::GetLine(char* str,char** nextLine) {
	char* it = str;
	//iterate to either ; (comment), \r\n (newline), \n (newline - unix style), \r (newline - mac style),
	//so probably just \r or \n, or \0
	while (true) {
		if (*it == ';') {
			*it++ = '\0';
			//skip rest of line
			while (*it && *it != '\r' && *it != '\n') it++;
			while (*it && *it == '\r' || *it == '\n') it++;
			if (nextLine != NULL) {
				if (*it)
					*nextLine = it;
				else
					*nextLine = NULL;
			}
			break;
		}
		else if (*it == '\r' || *it == '\n') {
			*it++ = '\0';
			while (*it && *it == '\r' || *it == '\n') it++;
			if (nextLine != NULL) {
				if (*it)
					*nextLine = it;
				else
					*nextLine = NULL;
			}
			break;
		}
		else if (*it == '\0') {
			if (nextLine != NULL) *nextLine = NULL;
			break;
		}
		it++;
	}
}

/*
* Gets next whitespace seperated token from str, nullterminates it,
* sets nextPart (if != NULL) to point to after the \0 and then
* returns a pointer to the beginning of the token (skipping whitespaces),
* or NULL if no token was found (only whitespaces and \0)
*/
char* Config::GetToken(char* str,char** nextPart) {
	while (isspace(*str) && *str != '\0') str++;
	if (*str == '\0') {
		if (nextPart != NULL) *nextPart = NULL;
		return NULL;
	}
	char* ret = str;
	while (!isspace(*str) && *str != '\0') str++;
	if (*str == '\0') {
		if (nextPart != NULL) *nextPart = NULL;
		return ret;
	}
	*str = '\0';
	if (nextPart != NULL) {
		if (*(str+1) == '\0') {
			*nextPart = NULL;
		}
		else {
			*nextPart = str+1;
		}
	}
	return ret;
}