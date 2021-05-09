#include "StdAfx.h"

#define NEW_HOOK 1
#define REGRESSIONS 1

namespace PlayInjections {
	namespace ClothesLimit {
		using namespace ExtClass;
		DWORD* clothingSlotReturnAddress = 0;
		int fourByteClothes = 0;




		void trimRemover() {
			//find all places where the game accesses the byte and feed it the 4 byte address instead
			//every time it trims the value to a byte, make it not do that

			DWORD address = General::GameBase + 0xAB5C9;
			Hook((BYTE*)address,
			{ 0x0F, 0xB6, 0x55, 0x00 },
			{ 0x8B, 0x55, 0x00, 0x90 },
				NULL);

			address = General::GameBase + 0xAA791;
			Hook((BYTE*)address,
			{ 0x0F, 0xB6, 0xBC, 0x24, 0x14, 0x01, 0x00, 0x00 },
			{ 0x8B, 0xBC, 0x24, 0x14, 0x01, 0x00, 0x00, 0x90 },
				NULL);

			address = General::GameBase + 0x11371E;
			Hook((BYTE*)address,
			{ 0x8A, 0x06 },
			{ 0x8B, 0x06 },
				NULL);

			address = General::GameBase + 0x1139F9;
			Hook((BYTE*)address,
			{ 0x0F, 0xB6, 0x11 },
			{ 0x8B, 0x11, 0x90 },
				NULL);

			address = General::GameBase + 0x1264C4;
			Hook((BYTE*)address,
			{ 0x8A, 0x9C, 0x24, 0x34, 0x5C, 0x00, 0x00 },
			{ 0x8B, 0x9C, 0x24, 0x34, 0x5C, 0x00, 0x00 },
				NULL);

			address = General::GameBase + 0x1264EE;
			Hook((BYTE*)address,
			{ 0x0F, 0xB6, 0xC3 },
			{ 0x8B, 0xC3, 0x90 },
				NULL);

			address = General::GameBase + 0x113A95;
			Hook((BYTE*)address,
			{ 0x8A, 0x06 },
			{ 0x8B, 0x06 },
				NULL);

			address = General::GameBase + 0x113A9E;
			Hook((BYTE*)address,
			{ 0x88, 0x44, 0x24, 0x13 },
			{ 0x89, 0x44, 0x24, 0x13},
				NULL);


			address = General::GameBase + 0x113EAE;
			Hook((BYTE*)address,
			{ 0x8A, 0x0E },
			{ 0x8B, 0x0E },
				NULL);
			
			address = General::GameBase + 0x113EBA;
			Hook((BYTE*)address,
			{ 0x88, 0x4C, 0x24, 0x1B },
			{ 0x89, 0x4C, 0x24, 0x1B},
				NULL);

			address = General::GameBase + 0x11451A;
			Hook((BYTE*)address,
			{ 0x8A, 0x17 },
			{ 0x8B, 0x17 },
				NULL);

			address = General::GameBase + 0x111C53;
			Hook((BYTE*)address,
			{ 0x8A, 0x10 },
			{ 0x8B, 0x10 },
				NULL);

			address = General::GameBase + 0x11237E;
			Hook((BYTE*)address,
			{ 0x0F, 0xB6, 0x11 },
			{ 0x8B, 0x11, 0x90 },
				NULL);

			address = General::GameBase + 0x112620;
			Hook((BYTE*)address,
			{ 0x8A, 0x0E },
			{ 0x8B, 0x0E },
				NULL);

			address = General::GameBase + 0x112629;
			Hook((BYTE*)address,
			{ 0x88, 0x4C, 0x24, 0x17},
			{ 0x89, 0x4C, 0x24, 0x17},
				NULL);

			address = General::GameBase + 0x112912;
			Hook((BYTE*)address,
			{ 0x8A, 0x06 },
			{ 0x8B, 0x06 },
				NULL);

			address = General::GameBase + 0x11291E;
			Hook((BYTE*)address,
			{ 0x88, 0x44, 0x24, 0x1F},
			{ 0x89, 0x44, 0x24, 0x1F},
				NULL);

			address = General::GameBase + 0x1130FD;
			Hook((BYTE*)address,
			{ 0x8A, 0x17 },
			{ 0x8B, 0x17 },
				NULL);


			address = General::GameBase + 0xA6A19;
			Hook((BYTE*)address,
			{ 0x0F, 0xB6, 0x82, 0x80, 0x0A, 0x00, 0x00 },
			{ 0x8B, 0x82, 0x80, 0x0A, 0x00, 0x00, 0x90 },
				NULL);

			address = General::GameBase + 0x140BB9;
			Hook((BYTE*)address,
			{ 0x0F, 0xB6, 0x84, 0x24, 0xE0, 0x00, 0x00, 0x00 },
			{ 0x8B, 0x84, 0x24, 0xE0, 0x00, 0x00, 0x00, 0x90 },
				NULL);

			address = General::GameBase + 0x140C75;
			Hook((BYTE*)address,
			{ 0x0F, 0xB6, 0x94, 0x24, 0xE0, 0x00, 0x00, 0x00 },
			{ 0x8B, 0x94, 0x24, 0xE0, 0x00, 0x00, 0x00, 0x90 },
				NULL);

			address = General::GameBase + 0x140D84;
			Hook((BYTE*)address,
			{ 0x0F, 0xB6, 0x94, 0x24, 0xE0, 0x00, 0x00, 0x00 },
			{ 0x8B, 0x94, 0x24, 0xE0, 0x00, 0x00, 0x00, 0x90 },
				NULL);

			address = General::GameBase + 0x114508;
			Hook((BYTE*)address,
			{ 0x75, 0x09},
			{ 0x90, 0x90 },
				NULL);

		}

		int __stdcall fourByte(DWORD* param) {
			//Calculates the pointer to the 4byte clothes address struct from within m_charPos struct. 
			ExtClass:CharacterStruct* card;
			card = *(CharacterStruct**)((char*)(param)+0x24);
			int curr = card->m_currClothes;
			return card->m_charData->m_clothes[curr].slot;
		}

		void __stdcall clothingSlotFirstEvent(DWORD* param) {

			fourByteClothes = fourByte(param);

			const DWORD offset[]{ 0x130411 };
			clothingSlotReturnAddress = (DWORD*)ExtVars::ApplyRule(offset);
		}

		void __declspec(naked) clothingSlotFirstRedirect() {
			__asm {
				pushad
				push ebp
				call clothingSlotFirstEvent
				popad
				//original code
				mov ecx, fourByteClothes
				push ecx
				jmp clothingSlotReturnAddress
			}
		}


		void clothingSlotFirstInject() {
			//EBP is m_charPos
			//AA2Play.exe+13040C - 0FB6 4D 30            - movzx ecx,byte ptr [ebp+30]
			//AA2Play.exe+130410 - 51                    - push ecx
			// AA2Play.exe+130411 - 52                    - push edx (return to here)
			
			DWORD address = General::GameBase + 0x13040C;
			DWORD redirectAddress = (DWORD)(&clothingSlotFirstRedirect);
			Hook((BYTE*)address,
			{ 0x0F, 0xB6, 0x4D, 0x30, 0x51 },						//expected values
			{ 0xE9, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
				NULL);
		}


		void __stdcall clothingSlotSecondEvent(DWORD* param) {

			fourByteClothes = fourByte(param);

			const DWORD offset[]{ 0x1306D1 };
			clothingSlotReturnAddress = (DWORD*)ExtVars::ApplyRule(offset);
		}


		void __declspec(naked) clothingSlotSecondRedirect() {
			__asm {
				pushad
				push ebx
				call clothingSlotSecondEvent
				popad
				//original code
				mov ecx, fourByteClothes
				push ecx
				jmp clothingSlotReturnAddress
			}
		}


		void clothingSlotSecondInject() {
				//EBX is m_charPos
				//AA2Play.exe + 1306CC - 0FB6 4B 30 - movzx ecx, byte ptr[ebx + 30]
				//AA2Play.exe + 1306D0 - 51 - push ecx
				//AA2Play.exe + 1306D1 - 52 - push edx

				DWORD address = General::GameBase + 0x1306CC;
				DWORD redirectAddress = (DWORD)(&clothingSlotSecondRedirect);
				Hook((BYTE*)address,
				{ 0x0F, 0xB6, 0x4B, 0x30, 0x51 },						//expected values
				{ 0xE9, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
					NULL);
			}



		void __stdcall clothingSlotThirdEvent(DWORD* param) {

			fourByteClothes = fourByte(param);

			const DWORD offset[]{ 0x130979 };
			clothingSlotReturnAddress = (DWORD*)ExtVars::ApplyRule(offset);
		}


		void __declspec(naked) clothingSlotThirdRedirect() {
			__asm {
				pushad
				push ebx
				call clothingSlotThirdEvent
				popad
				//original code
				mov ecx, fourByteClothes
				push ecx
				jmp clothingSlotReturnAddress
			}
		}


		void clothingSlotThirdInject() {
			//EBX is m_charPos
			//AA2Play.exe + 130974 - 0FB6 4B 30 - movzx ecx, byte ptr[ebx + 30]
			//AA2Play.exe + 130978 - 51 - push ecx
			//AA2Play.exe + 130979 - 52 - push edx

			DWORD address = General::GameBase + 0x130974;
			DWORD redirectAddress = (DWORD)(&clothingSlotThirdRedirect);
			Hook((BYTE*)address,
			{ 0x0F, 0xB6, 0x4B, 0x30, 0x51 },						//expected values
			{ 0xE9, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
				NULL);
		}


		void __stdcall clothingSlotFourthEvent(ExtClass::CharacterStruct* character) {
			//unused, there in case we decide to complete the clothing UI functions
			int curr = character->m_currClothes;
			fourByteClothes = character->m_charData->m_clothes[curr].slot;

			const DWORD offset[]{ 0x1136F3 };
			clothingSlotReturnAddress = (DWORD*)ExtVars::ApplyRule(offset);
		}


		void __declspec(naked) clothingSlotFourthRedirect() {
			__asm {
				pushad
				push ebx
				call clothingSlotThirdEvent
				popad
				//original code
				mov eax, fourByteClothes
				mov edi, [ebx + 28]
				jmp clothingSlotReturnAddress
			}
		}

		void clothingSlotFourthInject() {
			//We dont use this one. It's a part of many functions related to button presses to change textures. It's there if we get bullied into fixing that for all cases.
			//EBX is m_char
			//AA2Play.exe + 1136ED - 8A 43 44 - mov al, [ebx + 44]
			//AA2Play.exe + 1136F0 - 8B 7B 28 - mov edi, [ebx + 28]
			//AA2Play.exe + 1136F3 - 89 7C 24 24 - mov[esp + 24], edi

			DWORD address = General::GameBase + 0x1136ED;
			DWORD redirectAddress = (DWORD)(&clothingSlotFourthRedirect);
			Hook((BYTE*)address,
			{ 0x8A, 0x43, 0x44, 0x8B, 0x7B, 0x28 },						//expected values
			{ 0xE9, HookControl::RELATIVE_DWORD, redirectAddress, 0x90 },	//redirect to our function
				NULL);
		}


	}
}
