#pragma once

#include <string>
#include <vector>

class PoseFile {
public:
	PoseFile();
	PoseFile(std::wstring path);

	void DumpToFile(std::wstring path);

	struct FrameMod {
		std::string frameName;
		float matrix[9];
	};

	inline int GetPose() { return pose; }
	inline float GetFrame() { return frame; }
	inline const std::vector<FrameMod>& GetMods() { return mods; }

	inline void SetPoseInfo(int pose,float frame) { this->pose = pose; this->frame = frame; }
	inline void AddFrameMod(FrameMod& mod) {
		mods.push_back(mod);
	}

private:
	int pose;
	float frame;
	std::vector<FrameMod> mods;

	
};