#include "StdAfx.h"
#include "Dictionary.h"

std::map<std::wstring, Storage::Dictionary> Storage::Dictionary::allStorages;

Storage::Dictionary Storage::Dictionary::getStorage(std::wstring file, std::wstring language)
{
	if (Storage::Dictionary::allStorages.find(file) == Storage::Dictionary::allStorages.end())
	{
		Storage::Dictionary::allStorages[file] = Storage::Dictionary(file, language);
	}
	return Storage::Dictionary::allStorages[file];
}

Storage::Option<std::string> Storage::Dictionary::getDictTypeString(std::wstring key)
{
	Option<std::string> result;
	auto i = this->data.find(General::CastToString(key));
	result.isValid = i != this->data.end();
	if (result.isValid) {
		if (i->second.is<std::string>()) result.value = i->second.get<std::string>();
		else result.isValid = false;
	}
	return result;
}

