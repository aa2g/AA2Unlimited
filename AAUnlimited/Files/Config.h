#pragma once
#include <string>

/*
* Parses a config file and holds the data defined in it
*/
class Config
{
public:
	enum Members {
		USE_TAN_SLOTS, USE_MESH_TEXTURE_OVERRIDES, USE_H_AI, USE_SHADOWING, USE_H_FACECAM,
		USE_H_POS_BUTTON_MOVE, USE_POSER_CLOTHES, USE_POSER_DIALOGUE,
		
		NO_PROMPT_IS_FORCE, HAI_ON_NO_PROMPT,

		SAVED_EYE_TEXTURE_USAGE,
		SAVED_FILE_USAGE, SAVED_FILE_REMOVE, SAVED_FILE_BACKUP,

		FACELESS_SLOT_MALE, FACELESS_SLOT_FEMALE,

		POV_OFFSET_X, POV_OFFSET_Y, POV_OFFSET_Z,

		LEGACY_MODE,

		USE_POSER_HOTKEYS,
		HKEY_POSER_TRANSLATE, HKEY_POSER_ROTATE, HKEY_POSER_SCALE,

		num_elements
	};
	union MemberData {
		bool bVal;
		int iVal;
		float fVal;
		char* sVal;
	};
public:
	Config();
	Config(const TCHAR* path);
	Config(Config& rhs);
	Config(Config&& rhs);
	Config& operator=(Config& rhs);
	Config& operator=(Config&& rhs);
	~Config();

	inline const MemberData& GetKeyValue(Members key) {
		return m_members[key];
	}
private:
	//map Member -> datatype, name, default value
	enum MemberType {
		BOOL, INT, FLOAT, STRING, UINT
	};
	struct MemberInfo {
		MemberType type;
		const char* name;
		MemberData data;
		MemberInfo(MemberType type, const char* name, bool b) : type(type), name(name) {
			data.bVal = b;
		}
		MemberInfo(MemberType type, const char* name, float f) : type(type), name(name) {
			data.fVal = f;
		}
		MemberInfo(MemberType type, const char* name, int i) : type(type), name(name) {
			data.iVal = i;
		}
		MemberInfo(MemberType type, const char* name, char* str) : type(type), name(name) {
			data.sVal = str;
		}
	};

	static const MemberInfo knownMembers[num_elements];
	MemberData m_members[num_elements];

	bool StartsWith(const char* str, const char* word);
	void GetLine(char* str, char** nextLine);
	char* GetToken(char* str, char** nextPart);
};

extern Config g_Config;

