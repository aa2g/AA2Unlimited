#pragma once

#include <string>
#include <vector>
#include <tuple>

/*
 * Holds the frames that the pose window can modify
 */
class PoseMods
{
public:
	PoseMods(std::wstring path);
	~PoseMods();

	inline std::vector<std::tuple<std::string,std::string>> GetInput() { return m_data; }
private:
	std::vector<std::tuple<std::string,std::string>> m_data;
};

