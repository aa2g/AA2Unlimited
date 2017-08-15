#pragma once
#include "StdAfx.h"
#include "../AAUnlimited/Functions/Shared/Globals.h"
#include "../3rdparty/picojson/picojson.h"
#include <vector>
#include <fstream>

namespace PersistentStorage {

	class Storage {
	protected:
		picojson::object data;
		std::wstring file;
	public:

		virtual picojson::value get(std::string key);
		virtual void set(std::string key, picojson::value value);

		void save();
		std::wstring report() {
			return General::CastToWString(picojson::value(data).to_str());
		}
	};

	class ClassStorage : public Storage {
	private:
	public:
		std::wstring className;
		
		ClassStorage(std::wstring filename) : className(filename) {
			picojson::value json;

			this->file = CLASS_SAVES_PATH + filename + L".json";
			std::ifstream in(file);

			in >> json;

			if (picojson::get_last_error().empty() && json.is<picojson::object>())	//storage file loaded correctly			
			{
				//load the data in memory
				data = json.get<picojson::object>();				
			}
			else	//file not found or invalid
			{
				this->set("class_name", picojson::value(General::CastToString(className)));
			}
		}
	};


}