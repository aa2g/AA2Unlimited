#include "Value.h"

#include "Files\Logger.h"

namespace Shared {
namespace Triggers {


//make sure to define in the same order as the type enum, so that
//g_Types[type] corresponds to the types Type class
Type g_Types[N_TYPES] = {
	{ TYPE_INVALID, TEXT("error type") },
	{ TYPE_INT, TEXT("int") },
	{ TYPE_BOOL, TEXT("bool") },
	{ TYPE_FLOAT, TEXT("float") },
	{ TYPE_STRING, TEXT("string") }
};



Value::Value() : type(TYPE_INVALID) {

}
Value::Value(Types type) {
	type = type;
	switch(type) {
	case TYPE_INT:
		iVal = 0;
		break;
	case TYPE_BOOL:
		bVal = false;
		break;
	case TYPE_FLOAT:
		fVal = 0;
		break;
	case TYPE_STRING:
		strVal = new std::wstring(TEXT("(default)"));
		break;
	default:
		break;
	}
}
Value::Value(int ival) {
	type = TYPE_INT;
	iVal = ival;
}
Value::Value(float fval) {
	type = TYPE_FLOAT;
	fVal = fval;
}
Value::Value(bool bval) {
	type = TYPE_BOOL;
	bVal = bval;
}
Value::Value(const std::wstring& wStr) {
	type = TYPE_STRING;
	strVal = new std::wstring(wStr);
}
Value::Value(const wchar_t* str) {
	type = TYPE_STRING;
	strVal = new std::wstring(str);
}
Value::Value(const char* str) {
	type = TYPE_STRING;
	strVal = new std::wstring(str, str+strlen(str));
}

Value::~Value() {
	switch (type) {
	case TYPE_STRING:
		delete strVal;
	case TYPE_INT:
	case TYPE_FLOAT:
	case TYPE_BOOL:
	case TYPE_INVALID:
		break;
	default:
		LOGPRIO(Logger::Priority::WARN) << "unrecognized Value type " << type << "\r\n";
		break;
	}
}

Value::Value(const Value & rhs) {
	type = rhs.type;
	switch (type) {
	case TYPE_INT:
		iVal = rhs.iVal;
		break;
	case TYPE_FLOAT:
		fVal = rhs.fVal;
		break;
	case TYPE_BOOL:
		bVal = rhs.bVal;
		break;
	case TYPE_STRING:
		strVal = new std::wstring(*rhs.strVal);
		break;
	default:
		break;
	}
}

Value::Value(Value && rhs) {
	type = rhs.type;
	switch (type) {
	case TYPE_INT:
		iVal = rhs.iVal;
		break;
	case TYPE_FLOAT:
		fVal = rhs.fVal;
		break;
	case TYPE_BOOL:
		bVal = rhs.bVal;
		break;
	case TYPE_STRING:
		strVal = rhs.strVal;
		rhs.strVal = NULL;
		break;
	default:
		break;
	}
}

Value & Value::operator=(const Value & rhs) {
	Clear();

	type = rhs.type;
	switch (type) {
	case TYPE_INT:
		iVal = rhs.iVal;
		break;
	case TYPE_FLOAT:
		fVal = rhs.fVal;
		break;
	case TYPE_BOOL:
		bVal = rhs.bVal;
		break;
	case TYPE_STRING:
		strVal = new std::wstring(*rhs.strVal);
		break;
	default:
		break;
	}
	return *this;
}

Value & Value::operator=(Value && rhs) {
	Clear();

	type = rhs.type;
	switch (type) {
	case TYPE_INT:
		iVal = rhs.iVal;
		break;
	case TYPE_FLOAT:
		fVal = rhs.fVal;
		break;
	case TYPE_BOOL:
		bVal = rhs.bVal;
		break;
	case TYPE_STRING:
		strVal = rhs.strVal;
		rhs.strVal = NULL;
		break;
	default:
		break;
	}
	return *this;
}


void Value::Clear() {
	switch(type) {
	case TYPE_STRING:
		delete strVal;
		strVal = NULL;
		break;
	default:
		break;
	}
	
}

}
}