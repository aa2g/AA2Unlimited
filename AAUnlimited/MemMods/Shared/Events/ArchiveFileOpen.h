#pragma once

#include <Windows.h>

namespace SharedInjections {
namespace ArchiveFile {


void OpenFileInject();
void DirScanInject();
void CreateFileInject();
void RegisterPP(const wchar_t*);


}
}