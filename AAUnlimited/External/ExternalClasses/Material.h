#pragma once

#include <Windows.h>
#include <stdint.h>

namespace ExtClass {
	struct Material {
		uint32_t m_nameLength;
		const char* m_name;
		float m_lightingAttributes[16];
		float m_shininess;

		uint32_t m_texture1NameLength;
		const char* m_texture1Name;
		void* m_unknownPointer1;
		BYTE m_texture1Flags[16];

		uint32_t m_texture2NameLength;
		const char* m_texture2Name;
		void* m_unknownPointer2;
		BYTE m_texture2Flags[16];

		uint32_t m_texture3NameLength;
		const char* m_texture3Name;
		void* m_unknownPointer3;
		BYTE m_texture3Flags[16];

		uint32_t m_texture4NameLength;
		const char* m_texture4Name;
		void* m_unknownPointer4;
		BYTE m_texture4Flags[16];

		BYTE m_hexFlags[88];
		BYTE m_unknown[0x20];
	};

static_assert(sizeof(Material) == 0x134, "Material size mismatch; must be 0x134 bytes");

}
