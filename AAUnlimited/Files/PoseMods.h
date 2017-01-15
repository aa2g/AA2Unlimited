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
	enum FrameCategory {
		Torso,
		Arms,
		LeftHand,
		RightHand,
		Legs,
		Skirt,
		Room,
		Other
	};
	PoseMods(std::wstring path);
	~PoseMods();

	inline std::vector<std::tuple<FrameCategory,std::string,std::string>> GetInput() { return m_data; }
private:
	std::vector<std::tuple<FrameCategory,std::string,std::string>> m_data;
};

