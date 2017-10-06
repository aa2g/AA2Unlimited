#pragma once

#include <Windows.h>

namespace SharedInjections {
namespace ArchiveFile {

BYTE *ReadBuf(const wchar_t *path, DWORD *readBytes);
void OpenFileInject();
void bindLua();


}
}