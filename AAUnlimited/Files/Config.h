#pragma once
#include <string>

/*
* Parses a config file and holds the data defined in it
*/
class Config
{
public:
	enum Members {
		USE_TAN_SLOTS, USE_MESH_TEXTURE_OVERRIDES, USE_H_AI,

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
	Config(const char* path);
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
		BOOL, INT, FLOAT, STRING
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

