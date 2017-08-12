#include "StdAfx.h"
#include "PersistentStorage.h"

picojson::value PersistentStorage::Storage::get(std::string key)
{
	return data.at(key);
}

void PersistentStorage::Storage::set(std::string key, picojson::value value)
{
	data[key] = value;
}

void PersistentStorage::Storage::save()
{
	std::ofstream out(this->file);
	out << picojson::value(data).serialize(true);
}
