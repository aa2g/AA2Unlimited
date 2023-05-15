#pragma once
#include "OverrideFile.h"

class XXObjectFile : public OverrideFile{
public:
	XXObjectFile();
	XXObjectFile(const TCHAR* path,PathStart tryPathStarts);
};