#pragma once

#include <Windows.h>

namespace Shared {


bool OpenShadowedFile(wchar_t* archive, wchar_t* file, DWORD* readBytes, BYTE** outBuffer);


}
