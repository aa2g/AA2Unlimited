#pragma once

#include <Windows.h>

namespace ExtClass {

#pragma pack(push, 1)
/*
 * Represents a texture.
 */
class TextureStruct
{
public:
	DWORD m_nameLength;
	char* m_name;		//file name without paths n stuff
	BYTE m_unknown1[42];
	BYTE* m_texture;	//binary texture file (tga or bmp)
public:
	TextureStruct() = delete;
	~TextureStruct() = delete;


};
#pragma pack(pop)


}