#include "StdAfx.h"
#include "PersistentStorage.h"

std::map<std::wstring, PersistentStorage::ClassStorage> PersistentStorage::ClassStorage::allStorages;

picojson::value PersistentStorage::ClassStorage::get(std::string key)
{
	return data[key];
}

PersistentStorage::ClassStorage PersistentStorage::current() {
	return ClassStorage::getStorage(Shared::GameState::getCurrentClassSaveName());
}

PersistentStorage::ClassStorage PersistentStorage::ClassStorage::set(std::string key, picojson::value value)
{
	data[key] = value;
	allStorages[this->className] = *this;
	return *this;
}

//storeClassData

PersistentStorage::ClassStorage PersistentStorage::ClassStorage::storeClassBool(std::wstring key, bool value)
{
	return this->set(General::CastToString(key), picojson::value(value));
}

PersistentStorage::ClassStorage PersistentStorage::ClassStorage::storeClassInt(std::wstring key, int value)
{
	return this->set(General::CastToString(key), picojson::value(1.0 * value));
}

PersistentStorage::ClassStorage PersistentStorage::ClassStorage::storeClassFloat(std::wstring key, float value)
{
	return this->set(General::CastToString(key), picojson::value(1.0 * value));
}

PersistentStorage::ClassStorage PersistentStorage::ClassStorage::storeClassString(std::wstring key, std::string value)
{
	return this->set(General::CastToString(key), picojson::value(value));
}

//storeCardData

PersistentStorage::ClassStorage PersistentStorage::ClassStorage::storeCardBool(CharInstData* character, std::wstring key, bool value)
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

PersistentStorage::ClassStorage PersistentStorage::ClassStorage::storeCardInt(CharInstData* character, std::wstring key, int value)
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

PersistentStorage::ClassStorage PersistentStorage::ClassStorage::storeCardFloat(CharInstData* character, std::wstring key, float value)
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

PersistentStorage::ClassStorage PersistentStorage::ClassStorage::storeCardString(CharInstData* character, std::wstring key, std::string value)
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

PersistentStorage::ClassStorage PersistentStorage::ClassStorage::storeCardObject(CharInstData* character, std::wstring key, picojson::object value)
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

//storeCardAAUData

PersistentStorage::ClassStorage PersistentStorage::ClassStorage::storeCardAAUDataBool(CharInstData* character, std::wstring key, bool value)
{
	std::string name = std::to_string(character->m_char->m_seat) + " " + std::string(character->m_char->m_charData->m_forename) + " " + std::string(character->m_char->m_charData->m_surname);

	auto cardStorageValue = this->get(name);
	picojson::object record;
	if (!cardStorageValue.is<picojson::object>()) {
		cardStorageValue = picojson::value(picojson::object());
	}
	auto aaudStorageValue = cardStorageValue.get<picojson::object>()["AAUData"];
	if (!aaudStorageValue.is<picojson::object>()) {
		aaudStorageValue = picojson::value(picojson::object());
	}
	picojson::object aaudStorage = aaudStorageValue.get<picojson::object>();
	aaudStorage[General::CastToString(key)] = picojson::value(value);
	record["AAUData"] = picojson::value(aaudStorage);

	this->set(name, picojson::value(record));
	return *this;
}

PersistentStorage::ClassStorage PersistentStorage::ClassStorage::storeCardAAUDataInt(CharInstData* character, std::wstring key, int value)
{
	std::string name = std::to_string(character->m_char->m_seat) + " " + std::string(character->m_char->m_charData->m_forename) + " " + std::string(character->m_char->m_charData->m_surname);

	auto cardStorageValue = this->get(name);
	picojson::object record;
	if (!cardStorageValue.is<picojson::object>()) {
		cardStorageValue = picojson::value(picojson::object());
	}
	auto aaudStorageValue = cardStorageValue.get<picojson::object>()["AAUData"];
	if (!aaudStorageValue.is<picojson::object>()) {
		aaudStorageValue = picojson::value(picojson::object());
	}
	picojson::object aaudStorage = aaudStorageValue.get<picojson::object>();
	aaudStorage[General::CastToString(key)] = picojson::value(value * 1.0);
	record["AAUData"] = picojson::value(aaudStorage);

	this->set(name, picojson::value(record));
	return *this;
}

PersistentStorage::ClassStorage PersistentStorage::ClassStorage::storeCardAAUDataFloat(CharInstData* character, std::wstring key, float value)
{
	std::string name = std::to_string(character->m_char->m_seat) + " " + std::string(character->m_char->m_charData->m_forename) + " " + std::string(character->m_char->m_charData->m_surname);

	auto cardStorageValue = this->get(name);
	picojson::object record;
	if (!cardStorageValue.is<picojson::object>()) {
		cardStorageValue = picojson::value(picojson::object());
	}
	auto aaudStorageValue = cardStorageValue.get<picojson::object>()["AAUData"];
	if (!aaudStorageValue.is<picojson::object>()) {
		aaudStorageValue = picojson::value(picojson::object());
	}
	picojson::object aaudStorage = aaudStorageValue.get<picojson::object>();
	aaudStorage[General::CastToString(key)] = picojson::value(value);
	record["AAUData"] = picojson::value(aaudStorage);

	this->set(name, picojson::value(record));
	return *this;
}

PersistentStorage::ClassStorage PersistentStorage::ClassStorage::storeCardAAUDataString(CharInstData* character, std::wstring key, std::string value)
{
	std::string name = std::to_string(character->m_char->m_seat) + " " + std::string(character->m_char->m_charData->m_forename) + " " + std::string(character->m_char->m_charData->m_surname);

	auto cardStorageValue = this->get(name);
	picojson::object record;
	if (!cardStorageValue.is<picojson::object>()) {
		cardStorageValue = picojson::value(picojson::object());
	}
	auto aaudStorageValue = cardStorageValue.get<picojson::object>()["AAUData"];
	if (!aaudStorageValue.is<picojson::object>()) {
		aaudStorageValue = picojson::value(picojson::object());
	}
	picojson::object aaudStorage = aaudStorageValue.get<picojson::object>();
	aaudStorage[General::CastToString(key)] = picojson::value(value);
	record["AAUData"] = picojson::value(aaudStorage);

	this->set(name, picojson::value(record));
	return *this;
}

PersistentStorage::ClassStorage PersistentStorage::ClassStorage::storeCardAAUDataObject(CharInstData* character, std::wstring key, picojson::object value)
{
	std::string name = std::to_string(character->m_char->m_seat) + " " + std::string(character->m_char->m_charData->m_forename) + " " + std::string(character->m_char->m_charData->m_surname);

	auto cardStorageValue = this->get(name);
	picojson::object record;
	if (!cardStorageValue.is<picojson::object>()) {
		cardStorageValue = picojson::value(picojson::object());
	}
	auto aaudStorageValue = cardStorageValue.get<picojson::object>()["AAUData"];
	if (!aaudStorageValue.is<picojson::object>()) {
		aaudStorageValue = picojson::value(picojson::object());
	}
	picojson::object aaudStorage = aaudStorageValue.get<picojson::object>();
	aaudStorage[General::CastToString(key)] = picojson::value(value);
	record["AAUData"] = picojson::value(aaudStorage);

	this->set(name, picojson::value(record));
	return *this;
}

//getClassData
PersistentStorage::Option<int> PersistentStorage::ClassStorage::getClassInt(std::wstring key)
{
	Option<int> result;
	auto i = this->data.find(General::CastToString(key));
	result.isValid = i != this->data.end();
	if (result.isValid)
		result.value = i->second.get<double>();
	return result;
}
PersistentStorage::Option<float> PersistentStorage::ClassStorage::getClassFloat(std::wstring key)
{
	Option<float> result;
	auto i = this->data.find(General::CastToString(key));
	result.isValid = i != this->data.end();
	if (result.isValid)
		result.value = i->second.get<double>();
	return result;
}
PersistentStorage::Option<bool> PersistentStorage::ClassStorage::getClassBool(std::wstring key)
{
	Option<bool> result;
	auto i = this->data.find(General::CastToString(key));
	result.isValid = i != this->data.end();
	if (result.isValid)
		result.value = i->second.evaluate_as_boolean();
	return result;
}
PersistentStorage::Option<std::string> PersistentStorage::ClassStorage::getClassString(std::wstring key)
{
	Option<std::string> result;
	auto i = this->data.find(General::CastToString(key));
	result.isValid = i != this->data.end();
	if (result.isValid)
		result.value = i->second.get<std::string>();
	return result;
}

//getCardData
PersistentStorage::Option<picojson::object> PersistentStorage::ClassStorage::getCardRecord(CharInstData* character, std::wstring key)
{
	std::string name = std::to_string(character->m_char->m_seat) + " " + std::string(character->m_char->m_charData->m_forename) + " " + std::string(character->m_char->m_charData->m_surname);

	Option<picojson::object> result;
	auto i = this->data.find(name);
	result.isValid = i != this->data.end();
	if (result.isValid)
		result.value = i->second.get<picojson::object>();

	return result;
}

PersistentStorage::Option<bool> PersistentStorage::ClassStorage::getCardBool(CharInstData* character, std::wstring key)
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

PersistentStorage::Option<int> PersistentStorage::ClassStorage::getCardInt(CharInstData* character, std::wstring key)
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

PersistentStorage::Option<float> PersistentStorage::ClassStorage::getCardFloat(CharInstData* character, std::wstring key)
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

PersistentStorage::Option<std::string> PersistentStorage::ClassStorage::getCardString(CharInstData* character, std::wstring key)
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

PersistentStorage::Option<picojson::object> PersistentStorage::ClassStorage::getCardObject(CharInstData* character, std::wstring key)
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

//getCardAAUData
PersistentStorage::Option<bool> PersistentStorage::ClassStorage::getCardAAUDataBool(CharInstData* character, std::wstring key)
{
	std::string name = std::to_string(character->m_char->m_seat) + " " + std::string(character->m_char->m_charData->m_forename) + " " + std::string(character->m_char->m_charData->m_surname);
	auto cardStorage = this->get(name);
	Option<bool> result;
	result.isValid = false;
	if (cardStorage.is<picojson::object>())
	{
		auto aaudStorageValue = cardStorage.get<picojson::object>()["AAUData"];
		if (aaudStorageValue.is<picojson::object>()) {
			auto i = aaudStorageValue.get<picojson::object>()[General::CastToString(key)];
			result.isValid = i.is<bool>();
			if (result.isValid)
				result.value = i.get<bool>();
		}
	}
	return result;
}

PersistentStorage::Option<int> PersistentStorage::ClassStorage::getCardAAUDataInt(CharInstData* character, std::wstring key)
{
	std::string name = std::to_string(character->m_char->m_seat) + " " + std::string(character->m_char->m_charData->m_forename) + " " + std::string(character->m_char->m_charData->m_surname);
	auto cardStorage = this->get(name);
	Option<int> result;
	result.isValid = false;
	if (cardStorage.is<picojson::object>())
	{
		auto aaudStorageValue = cardStorage.get<picojson::object>()["AAUData"];
		if (aaudStorageValue.is<picojson::object>()) {
			auto i = aaudStorageValue.get<picojson::object>()[General::CastToString(key)];
			result.isValid = i.is<double>();
			if (result.isValid)
				result.value = i.get<double>();
		}
	}
	return result;
}

PersistentStorage::Option<float> PersistentStorage::ClassStorage::getCardAAUDataFloat(CharInstData* character, std::wstring key)
{
	std::string name = std::to_string(character->m_char->m_seat) + " " + std::string(character->m_char->m_charData->m_forename) + " " + std::string(character->m_char->m_charData->m_surname);
	auto cardStorage = this->get(name);
	Option<float> result;
	result.isValid = false;
	if (cardStorage.is<picojson::object>())
	{
		auto aaudStorageValue = cardStorage.get<picojson::object>()["AAUData"];
		if (aaudStorageValue.is<picojson::object>()) {
			auto i = aaudStorageValue.get<picojson::object>()[General::CastToString(key)];
			result.isValid = i.is<double>();
			if (result.isValid)
				result.value = i.get<double>();
		}
	}
	return result;
}

PersistentStorage::Option<std::string> PersistentStorage::ClassStorage::getCardAAUDataString(CharInstData* character, std::wstring key)
{
	std::string name = std::to_string(character->m_char->m_seat) + " " + std::string(character->m_char->m_charData->m_forename) + " " + std::string(character->m_char->m_charData->m_surname);
	auto cardStorage = this->get(name);
	Option<std::string> result;
	result.isValid = false;
	if (cardStorage.is<picojson::object>())
	{
		auto aaudStorageValue = cardStorage.get<picojson::object>()["AAUData"];
		if (aaudStorageValue.is<picojson::object>()) {
			auto i = aaudStorageValue.get<picojson::object>()[General::CastToString(key)];
			result.isValid = i.is<std::string>();
			if (result.isValid)
				result.value = i.get<std::string>();
		}
	}
	return result;
}

PersistentStorage::Option<picojson::object> PersistentStorage::ClassStorage::getCardAAUDataObject(CharInstData* character, std::wstring key)
{
	std::string name = std::to_string(character->m_char->m_seat) + " " + std::string(character->m_char->m_charData->m_forename) + " " + std::string(character->m_char->m_charData->m_surname);
	auto cardStorage = this->get(name);
	Option<picojson::object> result;
	result.isValid = false;
	if (cardStorage.is<picojson::object>())
	{
		auto aaudStorageValue = cardStorage.get<picojson::object>()["AAUData"];
		if (aaudStorageValue.is<picojson::object>()) {
			auto i = aaudStorageValue.get<picojson::object>()[General::CastToString(key)];
			result.isValid = i.is<picojson::object>();
			if (result.isValid)
				result.value = i.get<picojson::object>();
		}
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
PersistentStorage::ClassStorage PersistentStorage::ClassStorage::getCurrentClassStorage()
{
	return PersistentStorage::ClassStorage::getStorage(Shared::GameState::getCurrentClassSaveName());
}
void PersistentStorage::ClassStorage::reset(std::wstring file)
{
	PersistentStorage::ClassStorage::allStorages.erase(file);
}