#pragma once
#include "../General/Util.h"
#include "../3rdparty/picojson/picojson.h"
#include <vector>
#include <fstream>

namespace Storage {

	template<typename T>
	class Option {
	public:
		bool isValid;
		T value;

		Option() : isValid(false) {}
		Option(T val) : isValid(true), value(val) {}
		Option(Option &that) : isValid(that.isValid), value(that.value) {}
	};

	class Dictionary {
	private:
		picojson::object data;	//all the json data
		std::wstring file;	//file with its path starting at AA2_PLAY\ 
		static std::map<std::wstring, Dictionary> allStorages;	//collection of all the loaded storages
	public:
		std::wstring dictType;
		static Dictionary getStorage(std::wstring file, std::wstring language);
		Option<std::string> getDictTypeString(std::wstring key);
		Dictionary() {}
		Dictionary(Dictionary &that)
		{
			this->data = that.data;
			this->file = that.file;
			this->dictType = that.dictType;
		}
		Dictionary(std::wstring dictType, std::wstring language) : dictType(dictType) {
			picojson::value json;
			this->file = General::AAUPath + L"resources\\localization\\modules\\" +dictType + language+ L".json";
			std::ifstream in(this->file);

			in >> json;

			if (picojson::get_last_error().empty() && json.is<picojson::object>())	//storage file loaded correctly			
			{
				//load the data in memory
				data = json.get<picojson::object>();
			}
			Dictionary::allStorages[dictType] = *this;
		}
	};
}