#pragma once

#include <Windows.h>
#include <string>

namespace Shared {
namespace Triggers {

enum Types {
	TYPE_INVALID,
	TYPE_INT,TYPE_BOOL,TYPE_FLOAT,TYPE_STRING,

	N_TYPES
};

/*
* A Type, such as ints, floats, strings etc
*/
class Type {
public:
	Types id;
	std::wstring name;
};

class Value {
public:
	Types type;
	union {
		std::wstring* strVal;   //for STRING
		int iVal;				//for INT
		float fVal;				//for FLOAT
		bool bVal;				//for BOOL
	};

	Value();
	Value(Types type); //default value
	Value(int ival);
	Value(float fval);
	Value(bool bVal);
	Value(const std::wstring& wStr);
	Value(const wchar_t* str);
	Value(const char* str);
	~Value();
	Value(const Value& rhs);
	Value(Value&& rhs);
	Value& operator= (const Value& rhs);
	Value& operator= (Value&& rhs);

	void Clear();
};




extern Type g_Types[N_TYPES];




}
}