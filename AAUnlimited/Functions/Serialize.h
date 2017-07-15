#pragma once

#include <vector>
#include <string>
#include <map>
#include <exception>

#include "Shared\Triggers\Triggers.h"
#include "Shared\Triggers\Module.h"
#include "General\Buffer.h"

namespace Serialize {

class InsufficientBufferException : public std::exception {
public:
	inline InsufficientBufferException(int exp, int left) : sizeExpected(exp), sizeLeft(left) {}
	inline int ExpectedSize() { return sizeExpected; }
	inline int AvailableSize() { return sizeLeft; }
private:
	int sizeExpected;
	int sizeLeft;

};

inline std::wstring WStringFromBuffer(char* buffer, int size) {
	std::wstring retVal;
	retVal.reserve(size*2);
	for(int i = 0; i < size; i++) {
		static const wchar_t map[16] {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
		BYTE val = buffer[i];
		wchar_t b[2];
		b[0] = map[(val & 0xF0) >> 4];
		b[1] = map[(val & 0x0F)];
		retVal.push_back(b[0]);
		retVal.push_back(b[1]);
	}
	return retVal;
}

//must delete[] resulting buffer
inline char* BufferFromWString(wchar_t* buffer, int length) {
	if (length % 2 == 1) return NULL;
	char* retBuffer = new char[length/2];
	for(int i = 0; i < length; i+= 2) {
		BYTE b = 0;
		wchar_t c = buffer[i];
		wchar_t c2 = buffer[i+1];
		if(c >= L'0' && c <= L'9') {
			b |= c - L'0';
		}
		else if(c >= L'A' && c <= L'F') {
			b |= c - L'A';
		}
		b = b << 4;
		if (c2 >= L'0' && c2 <= L'9') {
			b |= c2 - L'0';
		}
		else if (c2 >= L'A' && c2 <= L'F') {
			b |= c2 - L'A' + 10;
		}
		retBuffer[i/2] = b;
	}
	return retBuffer;
}




//call with a pointer to a buffer and its size; it will modify those values to point at the remaining part of the buffer after execution
template<typename T>
T ReadData(char*& buffer,int& size);
	template<typename T>
	T ReadData_sub(char*& buffer,int& size,  T*);
	template<typename T>
	T* ReadData_sub(char*& buffer,int& size, T**);
	std::vector<BYTE> ReadData_sub(char*& buffer,int& size,std::vector<BYTE>*);
	template<typename T>
	std::vector<T> ReadData_sub(char*& buffer,int& size,std::vector<T>*);
	template<typename T,typename U>
	std::pair<T,U> ReadData_sub(char*& buffer,int& size,std::pair<T,U>*);
	std::wstring ReadData_sub(char*& buffer,int& size,std::wstring*);
	template<typename T,typename U>
	std::map<T,U> ReadData_sub(char*& buffer,int& size,std::map<T,U>*);
	Shared::Triggers::Trigger ReadData_sub(char*& buffer,int& size,Shared::Triggers::Trigger*);
	Shared::Triggers::ParameterisedEvent ReadData_sub(char*& buffer,int& size,Shared::Triggers::ParameterisedEvent*);
	Shared::Triggers::ParameterisedAction ReadData_sub(char*& buffer,int& size,Shared::Triggers::ParameterisedAction*);
	Shared::Triggers::Trigger::GUIAction ReadData_sub(char*& buffer,int& size,Shared::Triggers::Trigger::GUIAction*);
	Shared::Triggers::ParameterisedExpression ReadData_sub(char*& buffer,int& size,Shared::Triggers::ParameterisedExpression*);
	Shared::Triggers::Variable ReadData_sub(char*& buffer,int& size,Shared::Triggers::Variable*);
	Shared::Triggers::GlobalVariable ReadData_sub(char*& buffer,int& size,Shared::Triggers::GlobalVariable*);
	Shared::Triggers::Value ReadData_sub(char*& buffer,int& size,Shared::Triggers::Value*);
	Shared::Triggers::Module ReadData_sub(char*& buffer,int& size,Shared::Triggers::Module*);

//write help functions
template<typename T>
bool WriteData(char** buffer,int* size,int& at,const T& data,bool resize);
	template<typename T>
	bool WriteData_sub(char** buffer,int* size,int& at,const T& data,bool resize,T*);
	template<typename T>
	bool WriteData_sub(char** buffer,int* size,int& at,const T*data,bool resize,T**);
	template<typename T>
	bool WriteData_sub(char** buffer,int* size,int& at,T* data,bool resize,T**);
	bool WriteData_sub(char** buffer,int* size,int& at,const std::wstring& data,bool resize,std::wstring*);
	template<typename T>
	bool WriteData_sub(char** buffer,int* size,int& at,const std::vector<T>& data,bool resize,std::vector<T>*);
	template<typename T,typename U>
	bool WriteData_sub(char** buffer,int* size,int& at,const std::pair<T,U>& data,bool resize,std::pair<T,U>*);
	template<typename T,typename U>
	bool WriteData_sub(char** buffer,int* size,int& at,const std::map<T,U>& data,bool resize,std::map<T,U>*);
	bool WriteData_sub(char** buffer,int* size,int& at,const Shared::Triggers::Trigger& data,bool resize,Shared::Triggers::Trigger*);
	bool WriteData_sub(char** buffer,int* size,int& at,const Shared::Triggers::ParameterisedEvent& data,bool resize,Shared::Triggers::ParameterisedEvent*);
	bool WriteData_sub(char** buffer,int* size,int& at,const Shared::Triggers::ParameterisedAction& data,bool resize,Shared::Triggers::ParameterisedAction*);
	bool WriteData_sub(char** buffer,int* size,int& at,const Shared::Triggers::Trigger::GUIAction& data,bool resize,Shared::Triggers::Trigger::GUIAction*);
	bool WriteData_sub(char** buffer,int* size,int& at,const Shared::Triggers::ParameterisedExpression& data,bool resize,Shared::Triggers::ParameterisedExpression*);
	bool WriteData_sub(char** buffer,int* size,int& at,const Shared::Triggers::Variable& data,bool resize,Shared::Triggers::Variable*);
	bool WriteData_sub(char** buffer,int* size,int& at,const Shared::Triggers::GlobalVariable& data,bool resize,Shared::Triggers::GlobalVariable*);
	bool WriteData_sub(char** buffer,int* size,int& at,const Shared::Triggers::Value& data,bool resize,Shared::Triggers::Value*);
	bool WriteData_sub(char** buffer,int* size,int& at,const Shared::Triggers::Module& data,bool resize,Shared::Triggers::Module*);


template<typename T>
T ReadData(char*& buffer,int& size) {
	return ReadData_sub(buffer,size,(T*)0);
}

template<typename T>
T ReadData_sub(char*& buffer,int& size,T*) {
	if (size < sizeof(T)) {
		throw InsufficientBufferException(sizeof(T),size);
	}

	T retVal = *(T*)(buffer);
	buffer += sizeof(T),size -= sizeof(T);
	
	return retVal;
}

template<typename T>
T* ReadData_sub(char*& buffer,int& size,T**) {
	return new T(ReadData<T>(buffer,size));
}

//read for string
inline std::wstring ReadData_sub(char*& buffer,int& size,std::wstring*) {
	DWORD length = ReadData<DWORD>(buffer,size);
	if (size < 0 || (DWORD)size < length*sizeof(TCHAR)) {
		throw InsufficientBufferException(length*sizeof(TCHAR), size);
	}
	std::wstring retVal((TCHAR*)buffer,length);
	buffer += length*sizeof(TCHAR),size -= length*sizeof(TCHAR);
	return retVal;
}

//special vector read for byte vectors, doesnt push every element
inline std::vector<BYTE> ReadData_sub(char*& buffer,int& size,std::vector<BYTE>*) {
	DWORD length = ReadData<DWORD>(buffer,size);
	if (size < 0 || (DWORD)size < length) {
		throw InsufficientBufferException(length,size);
	}
	std::vector<BYTE> retVal;
	retVal.reserve(length);
	retVal.assign(buffer,buffer+length);
	buffer += length,size -= length;
	return retVal;
}

//read for vectors
template<typename T>
std::vector<T> ReadData_sub(char*&buffer,int& size,std::vector<T>*) {
	DWORD length = ReadData<DWORD>(buffer,size);
	std::vector<T> retVal;
	retVal.reserve(length);
	for (unsigned int i = 0; i < length; i++) {
		T val = ReadData<T>(buffer,size);
		retVal.push_back(std::move(val));
	}
	return retVal;
}
//for pairs
template<typename T,typename U>
std::pair<T,U> ReadData_sub(char*&buffer,int& size,std::pair<T,U>*) {
	T val1 = ReadData<T>(buffer,size);
	U val2 = ReadData<U>(buffer,size);
	return std::make_pair(std::move(val1),std::move(val2));
}

template<typename T,typename U>
std::map<T,U> ReadData_sub(char *& buffer,int & size,std::map<T,U>*)
{
	DWORD length = ReadData<DWORD>(buffer,size);
	std::map<T,U> retVal;
	for (int i = 0; i < length; i++) {
		T tval = ReadData<T>(buffer,size);
		U uval = ReadData<U>(buffer,size);
		retVal.emplace(std::move(tval),std::move(uval));
	}
	return retVal;
}

inline Shared::Triggers::Trigger ReadData_sub(char *& buffer,int & size,Shared::Triggers::Trigger *)
{
	using namespace Shared::Triggers;
	Trigger retVal;
	retVal.name = ReadData<std::wstring>(buffer,size);
	retVal.events = ReadData<decltype(retVal.events)>(buffer,size);
	retVal.vars = ReadData<decltype(retVal.vars)>(buffer,size);
	retVal.guiActions = ReadData<decltype(retVal.guiActions)>(buffer,size);
	return retVal;
}

inline Shared::Triggers::ParameterisedEvent ReadData_sub(char*& buffer,int& size,Shared::Triggers::ParameterisedEvent*)
{
	using namespace Shared::Triggers;
	ParameterisedEvent retVal;
	int id = ReadData<int>(buffer,size);
	retVal.event = Event::FromId(id);
	retVal.actualParameters = ReadData<decltype(retVal.actualParameters)>(buffer,size);
	return retVal;
}

inline Shared::Triggers::ParameterisedAction ReadData_sub(char*& buffer,int& size,Shared::Triggers::ParameterisedAction*)
{
	using namespace Shared::Triggers;
	ParameterisedAction retVal;
	int id = ReadData<int>(buffer,size);
	retVal.action = Action::FromId(id);
	retVal.actualParameters = ReadData<decltype(retVal.actualParameters)>(buffer,size);
	return retVal;
}
inline Shared::Triggers::Trigger::GUIAction ReadData_sub(char *& buffer,int & size,Shared::Triggers::Trigger::GUIAction *)
{
	using namespace Shared::Triggers;
	Trigger::GUIAction retVal;
	retVal.parent = NULL;
	retVal.action = ReadData<decltype(retVal.action)>(buffer,size);

	retVal.subactions = ReadData<decltype(retVal.subactions)>(buffer,size);
	for (auto* elem : retVal.subactions) {
		elem->parent = &retVal;
	}
	return retVal;
}
inline Shared::Triggers::ParameterisedExpression ReadData_sub(char*& buffer,int& size,Shared::Triggers::ParameterisedExpression*)
{
	using namespace Shared::Triggers;
	ParameterisedExpression retVal;
	Types type = ReadData<Types>(buffer,size);
	int id = ReadData<int>(buffer,size);
	retVal.expression = Expression::FromId(type,id);
	if (id == EXPR_CONSTANT) {
		//constant
		retVal.constant = ReadData<decltype(retVal.constant)>(buffer,size);
	}
	else if (id == EXPR_VAR) {
		//variable
		retVal.varName = ReadData<decltype(retVal.varName)>(buffer,size);
	}
	else if (id == EXPR_NAMEDCONSTANT) {
		int id = ReadData<int>(buffer,size);
		retVal.namedConstant = NamedConstant::FromId(retVal.expression->returnType,id);
	}
	else {
		//function
		retVal.actualParameters = ReadData<decltype(retVal.actualParameters)>(buffer,size);
	}


	return retVal;
}
inline Shared::Triggers::Variable ReadData_sub(char*& buffer,int& size,Shared::Triggers::Variable*)
{
	using namespace Shared::Triggers;
	Variable retVal;
	retVal.type = ReadData<Types>(buffer,size);
	retVal.name = ReadData<std::wstring>(buffer,size);
	retVal.defaultValue = ReadData<decltype(retVal.defaultValue)>(buffer,size);
	return retVal;
}

inline Shared::Triggers::GlobalVariable ReadData_sub(char*& buffer,int& size,Shared::Triggers::GlobalVariable*)
{
	using namespace Shared::Triggers;
	GlobalVariable retVal;
	retVal.type = ReadData<Types>(buffer,size);
	retVal.name = ReadData<std::wstring>(buffer,size);
	retVal.defaultValue = ReadData<decltype(retVal.defaultValue)>(buffer,size);
	retVal.currentValue = ReadData<decltype(retVal.currentValue)>(buffer,size);
	retVal.initialized = ReadData<bool>(buffer,size);
	return retVal;
}

inline Shared::Triggers::Value ReadData_sub(char*& buffer,int& size,Shared::Triggers::Value*)
{
	using namespace Shared::Triggers;
	Value retVal;
	retVal.type = ReadData<Types>(buffer,size);
	switch (retVal.type) {
	case TYPE_INT:
		retVal.iVal = ReadData<int>(buffer,size);
		break;
	case TYPE_BOOL:
		retVal.bVal = ReadData<bool>(buffer,size);
		break;
	case TYPE_FLOAT:
		retVal.fVal = ReadData<float>(buffer,size);
		break;
	case TYPE_STRING:
		retVal.strVal = new std::wstring(ReadData<std::wstring>(buffer,size));
		break;
	default:
		break;
	}
	return retVal;
}

inline Shared::Triggers::Module ReadData_sub(char*& buffer,int& size,Shared::Triggers::Module*)
{
	using namespace Shared::Triggers;
	Module mod;
	mod.name = ReadData<std::wstring>(buffer,size);
	mod.description = ReadData<std::wstring>(buffer,size);
	mod.triggers = ReadData<decltype(mod.triggers)>(buffer,size);
	mod.globals = ReadData<decltype(mod.globals)>(buffer,size);
	mod.dependencies = ReadData<decltype(mod.dependencies)>(buffer,size);
	return mod;
}

/***************************/
/* Generic Write functions */
/***************************/

template<typename T>
bool WriteData(char** buffer,int* size,int& at,const T& data,bool resize) {
	return WriteData_sub(buffer,size,at,data,resize,(T*)0);
}

//general
template<typename T>
bool WriteData_sub(char** buffer,int* size,int& at,const T& data,bool resize,T*) {
	bool ret = General::BufferAppend(buffer,size,at,(char*)(&data),sizeof(data),resize);
	at += sizeof(data);
	return ret;
}

template<typename T>
bool WriteData_sub(char** buffer,int* size,int& at,const T* data,bool resize,T**) {
	bool ret = true;
	if (data == NULL) return true;
	ret &= WriteData(buffer,size,at,*data,resize);
	return ret;
}

template<typename T>
bool WriteData_sub(char** buffer,int* size,int& at,T* data,bool resize,T**) {
	bool ret = true;
	if (data == NULL) return true;
	ret &= WriteData(buffer,size,at,*data,resize);
	return ret;
}

//for string
inline bool WriteData_sub(char** buffer,int* size,int& at,const std::wstring& data,bool resize,std::wstring*) {
	bool ret = true;
	//write size first, then buffer
	DWORD ssize = data.size();
	ret &= General::BufferAppend(buffer,size,at,(const char*)(&ssize),4,resize);
	at += 4;
	ret &= General::BufferAppend(buffer,size,at,(const char*)data.c_str(),data.size()*sizeof(TCHAR),resize);
	at += data.size() * sizeof(TCHAR);
	return ret;
}
//for vector
template<typename T>
bool WriteData_sub(char** buffer,int* size,int& at,const std::vector<T>& data,bool resize,std::vector<T>*) {
	DWORD length = data.size();
	bool ret = true;
	ret &= WriteData(buffer,size,at,length,resize);
	for (DWORD i = 0; i < length; i++) {
		ret &= WriteData(buffer,size,at,data[i],resize);
	}
	return ret;
}
//for pairs
template<typename T,typename U>
bool WriteData_sub(char** buffer,int* size,int& at,const std::pair<T,U>& data,bool resize,std::pair<T,U>*) {
	bool ret = true;
	ret &= WriteData(buffer,size,at,data.first,resize);
	ret &= WriteData(buffer,size,at,data.second,resize);
	return ret;
}

template<typename T,typename U>
bool WriteData_sub(char ** buffer,int * size,int & at,const std::map<T,U>& data,bool resize,std::map<T,U>*) {
	bool ret = true;
	ret &= WriteData(buffer,size,at,data.size(),resize);
	for (const auto& it : data) {
		ret &= WriteData(buffer,size,at,it,resize);
	}
	return ret;
}

inline bool WriteData_sub(char** buffer,int* size,int& at,const Shared::Triggers::Trigger& data,bool resize,Shared::Triggers::Trigger*)
{
	bool ret = true;
	ret &= WriteData(buffer,size,at,data.name,resize);
	ret &= WriteData(buffer,size,at,data.events,resize);
	ret &= WriteData(buffer,size,at,data.vars,resize);
	ret &= WriteData(buffer,size,at,data.guiActions,resize);
	return ret;
}
inline bool WriteData_sub(char** buffer,int* size,int& at,const Shared::Triggers::ParameterisedEvent& data,bool resize,Shared::Triggers::ParameterisedEvent*)
{
	bool ret = true;
	ret &= WriteData(buffer,size,at,data.event->id,resize);
	ret &= WriteData(buffer,size,at,data.actualParameters,resize);
	return ret;
}
inline bool WriteData_sub(char** buffer,int* size,int& at,const Shared::Triggers::ParameterisedAction& data,bool resize,Shared::Triggers::ParameterisedAction*)
{
	bool ret = true;
	ret &= WriteData(buffer,size,at,data.action->id,resize);
	ret &= WriteData(buffer,size,at,data.actualParameters,resize);
	return ret;
}
inline bool WriteData_sub(char ** buffer,int * size,int & at,const Shared::Triggers::Trigger::GUIAction & data,bool resize,Shared::Triggers::Trigger::GUIAction *)
{
	bool ret = true;
	ret &= WriteData(buffer,size,at,data.action,resize);
	ret &= WriteData(buffer,size,at,data.subactions,resize);
	return ret;
}
inline bool WriteData_sub(char** buffer,int* size,int& at,const Shared::Triggers::ParameterisedExpression& data,bool resize,Shared::Triggers::ParameterisedExpression*)
{
	bool ret = true;
	ret &= WriteData(buffer,size,at,data.expression->returnType,resize);
	ret &= WriteData(buffer,size,at,data.expression->id,resize);
	if (data.expression->id == Shared::Triggers::EXPR_CONSTANT) {
		ret &= WriteData(buffer,size,at,data.constant,resize);
	}
	else if (data.expression->id == Shared::Triggers::EXPR_VAR) {
		ret &= WriteData(buffer,size,at,data.varName,resize);
	}
	else if (data.expression->id == Shared::Triggers::EXPR_NAMEDCONSTANT) {
		ret &= WriteData(buffer,size,at,data.namedConstant->id,resize);
	}
	else {
		ret &= WriteData(buffer,size,at,data.actualParameters,resize);
	}
	return ret;
}
inline bool WriteData_sub(char** buffer,int* size,int& at,const Shared::Triggers::Variable& data,bool resize,Shared::Triggers::Variable*)
{
	bool ret = true;
	ret &= WriteData(buffer,size,at,data.type,resize);
	ret &= WriteData(buffer,size,at,data.name,resize);
	ret &= WriteData(buffer,size,at,data.defaultValue,resize);
	return ret;
}
inline bool WriteData_sub(char** buffer,int* size,int& at,const Shared::Triggers::GlobalVariable& data,bool resize,Shared::Triggers::GlobalVariable*) 
{
	bool ret = true;
	ret &= WriteData(buffer,size,at,data.type,resize);
	ret &= WriteData(buffer,size,at,data.name,resize);
	ret &= WriteData(buffer,size,at,data.defaultValue,resize);
	ret &= WriteData(buffer,size,at,data.currentValue,resize);
	ret &= WriteData(buffer,size,at,data.initialized,resize);
	return ret;
}
inline bool WriteData_sub(char** buffer,int* size,int& at,const Shared::Triggers::Value& data,bool resize,Shared::Triggers::Value*)
{
	using namespace Shared::Triggers;
	bool ret = true;
	ret &= WriteData(buffer,size,at,data.type,resize);

	switch (data.type) {
	case TYPE_INT:
		ret &= WriteData(buffer,size,at,data.iVal,resize);
		break;
	case TYPE_BOOL:
		ret &= WriteData(buffer,size,at,data.bVal,resize);
		break;
	case TYPE_FLOAT:
		ret &= WriteData(buffer,size,at,data.fVal,resize);
		break;
	case TYPE_STRING:
		ret &= WriteData(buffer,size,at,data.strVal,resize);
		break;
	default:
		break;
	}
	return ret;
}

inline bool WriteData_sub(char** buffer,int* size,int& at,const Shared::Triggers::Module& data,bool resize,Shared::Triggers::Module*) 
{
	using namespace Shared::Triggers;
	bool ret = true;
	WriteData(buffer,size,at,data.name,resize);
	WriteData(buffer,size,at,data.description,resize);
	WriteData(buffer,size,at,data.triggers,resize);
	WriteData(buffer,size,at,data.globals,resize);
	WriteData(buffer,size,at,data.dependencies,resize);
	return ret;
}

}