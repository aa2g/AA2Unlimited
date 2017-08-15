#include "StdAfx.h"
#include "PersistentStorage.h"

std::map<std::wstring, PersistentStorage::ClassStorage> PersistentStorage::ClassStorage::allStorages;

picojson::value PersistentStorage::ClassStorage::get(std::string key)
{
	return data[key];
}

PersistentStorage::ClassStorage PersistentStorage::ClassStorage::set(std::string key, picojson::value value)
{
	data[key] = value;
	allStorages[this->className] = *this;
	return *this;
}

//storeCardData

PersistentStorage::ClassStorage PersistentStorage::ClassStorage::storeCardBool(CharInstData * character, std::wstring key, bool value)
{
	/*
	{
		"seat LastName FirstName": {
			"key1": value1,
			"key2": value2,
			...
			"key": value
		},
	}
	*/

	std::string name = std::to_string(character->m_char->m_seat) + " " + std::string(character->m_char->m_charData->m_forename) + " " + std::string(character->m_char->m_charData->m_surname);

	auto val = this->get(name);
	picojson::object record;
	if (val.is<picojson::object>())
	{
		record = val.get<picojson::object>();
	}
	record[General::CastToString(key)] = picojson::value(value);

	this->set(name, picojson::value(record));
	return *this;
}

PersistentStorage::ClassStorage PersistentStorage::ClassStorage::storeCardInt(CharInstData * character, std::wstring key, int value)
{
	std::string name = std::to_string(character->m_char->m_seat) + " " + std::string(character->m_char->m_charData->m_forename) + " " + std::string(character->m_char->m_charData->m_surname);

	auto val = this->get(name);
	picojson::object record;
	if (val.is<picojson::object>())
	{
		record = val.get<picojson::object>();
	}
	record[General::CastToString(key)] = picojson::value(1.0 * value);

	this->set(name, picojson::value(record));
	return *this;
}

PersistentStorage::ClassStorage PersistentStorage::ClassStorage::storeCardFloat(CharInstData * character, std::wstring key, float value)
{
	std::string name = std::to_string(character->m_char->m_seat) + " " + std::string(character->m_char->m_charData->m_forename) + " " + std::string(character->m_char->m_charData->m_surname);

	auto val = this->get(name);
	picojson::object record;
	if (val.is<picojson::object>())
	{
		record = val.get<picojson::object>();
	}
	record[General::CastToString(key)] = picojson::value(value);

	this->set(name, picojson::value(record));
	return *this;
}

PersistentStorage::ClassStorage PersistentStorage::ClassStorage::storeCardString(CharInstData * character, std::wstring key, std::string value)
{
	std::string name = std::to_string(character->m_char->m_seat) + " " + std::string(character->m_char->m_charData->m_forename) + " " + std::string(character->m_char->m_charData->m_surname);

	auto val = this->get(name);
	picojson::object record;
	if (val.is<picojson::object>())
	{
		record = val.get<picojson::object>();
	}
	record[General::CastToString(key)] = picojson::value(value);

	this->set(name, picojson::value(record));
	return *this;
}

PersistentStorage::ClassStorage PersistentStorage::ClassStorage::storeCardObject(CharInstData * character, std::wstring key, picojson::object value)
{
	std::string name = std::to_string(character->m_char->m_seat) + " " + std::string(character->m_char->m_charData->m_forename) + " " + std::string(character->m_char->m_charData->m_surname);

	auto val = this->get(name);
	picojson::object record;
	if (val.is<picojson::object>())
	{
		record = val.get<picojson::object>();
	}
	record[General::CastToString(key)] = picojson::value(value);

	this->set(name, picojson::value(record));
	return *this;
}

//getCardData
PersistentStorage::Option<picojson::object> PersistentStorage::ClassStorage::getCardRecord(CharInstData* character, std::wstring key)
{
	std::string name = std::to_string(character->m_char->m_seat) + " " + std::string(character->m_char->m_charData->m_forename) + " " + std::string(character->m_char->m_charData->m_surname);

	Option<picojson::object> result;
	auto i = this->data.find(name);
	result.isValid = i != this->data.end();
	if(result.isValid)
		result.value = i->second.get<picojson::object>();

	return result;
}

PersistentStorage::Option<bool> PersistentStorage::ClassStorage::getCardBool(CharInstData * character, std::wstring key)
{
	auto val = getCardRecord(character, key);
	Option<bool> result;
	if (val.isValid)
	{
		auto i = val.value.find(General::CastToString(key));
		result.isValid = i != val.value.end();
		if (result.isValid)
			result.value = i->second.evaluate_as_boolean();
	}
	return result;
}

PersistentStorage::Option<int> PersistentStorage::ClassStorage::getCardInt(CharInstData * character, std::wstring key)
{
	auto val = getCardRecord(character, key);
	Option<int> result;
	if (val.isValid)
	{
		auto i = val.value.find(General::CastToString(key));
		result.isValid = i != val.value.end();
		if (result.isValid)
			result.value = i->second.get<double>();
	}
	return result;
}

PersistentStorage::Option<float> PersistentStorage::ClassStorage::getCardFloat(CharInstData * character, std::wstring key)
{
	auto val = getCardRecord(character, key);
	Option<float> result;
	if (val.isValid)
	{
		auto i = val.value.find(General::CastToString(key));
		result.isValid = i != val.value.end();
		if (result.isValid)
			result.value = i->second.get<double>();
	}
	return result;
}

PersistentStorage::Option<std::string> PersistentStorage::ClassStorage::getCardString(CharInstData * character, std::wstring key)
{
	auto val = getCardRecord(character, key);
	Option<std::string> result;
	if (val.isValid)
	{
		auto i = val.value.find(General::CastToString(key));
		result.isValid = i != val.value.end();
		if (result.isValid) result.value = i->second.get<std::string>();
	}
	return result;
}

PersistentStorage::Option<picojson::object> PersistentStorage::ClassStorage::getCardObject(CharInstData * character, std::wstring key)
{
	auto val = getCardRecord(character, key);
	Option<picojson::object> result;
	if (val.isValid)
	{
		auto i = val.value.find(General::CastToString(key));
		result.isValid = i != val.value.end();
		if (result.isValid) result.value = i->second.get<picojson::object>();
	}
	return result;
}



PersistentStorage::ClassStorage PersistentStorage::ClassStorage::save()
{
	std::ofstream out(this->file);
	if (!out.bad()) {
		out << picojson::value(data).serialize(true);
	}
	return *this;
}
PersistentStorage::ClassStorage PersistentStorage::ClassStorage::getStorage(std::wstring file)
{
	if (PersistentStorage::ClassStorage::allStorages.find(file) == PersistentStorage::ClassStorage::allStorages.end())
	{
		PersistentStorage::ClassStorage::allStorages[file] = PersistentStorage::ClassStorage(file);
	}
	return PersistentStorage::ClassStorage::allStorages[file];
}