#include "StdAfx.h"
#include "CharacterRender.h"
#include "External\ExternalClasses\CharacterStruct.h"


namespace PlayInjections {
	namespace CharacterRender {

		DWORD UpdateFrameAttachmentOrig;
		ExtClass::CharacterStruct *character;
		void __declspec(naked) UpdateFrameAttachmentRedirect() {
			__asm mov character, ebx
			if (ExtClass::g_anim_data[character->m_seat][0]) {	// this is set if character is poser spawned
				__asm mov [esp + 0x8], 0						// Change second function parameter to 0
			}
			__asm jmp UpdateFrameAttachmentOrig
		}

		void InitInjection() {
			/*
			115BE3 - 6A 01                 - push 01
			115BE5 - 51                    - push ecx
			115BE6 - E8 956E0100           - call AA2Play.exe+12CA80
			115BEB - 8D 83 20080000        - lea eax,[ebx+00000820]
			115BF1 - E8 9A17FFFF           - call AA2Play.exe+107390
			*/
			DWORD address = General::GameBase + 0x115BE6;
			DWORD redirectAddress = (DWORD)(&UpdateFrameAttachmentRedirect);
			Hook((BYTE*)address,
				{ 0xE8, 0x95, 0x6E, 0x01, 0x00 },
				{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },
				&UpdateFrameAttachmentOrig);
		}
	}
}
