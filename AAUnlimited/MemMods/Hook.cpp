#include "StdAfx.h"
#include "Files/PNGData.h"

/*
 * Can be used as newData DWORD parameters of the hook function; if used, the parameter
 * will not be interpreted as a byte, but have special meanings
 */
namespace HookControl {
	/*
	 * The following DWORD will be a relative offset of given size
	 */
/*	const DWORD RELATIVE_DWORD = (DWORD)-1;
	const DWORD RELATIVE_WORD  = (DWORD)-2;
	const DWORD ABSOLUTE_DWORD = (DWORD)-3;
	const DWORD ABSOLUTE_WORD = (DWORD)-4;
	const DWORD ANY_DWORD = (DWORD)-5;*/

	bool IsHookControl(DWORD d) {
		return d <= RELATIVE_DWORD && d >= ABSOLUTE_WORD;
	}
};

DWORD DWAbsoluteToRelative(BYTE* currLoc, DWORD targetLoc) {
	return targetLoc - (DWORD)(currLoc + 4);
}

/*
 * Same as DWRelativeToAbsolute, but for WORD offsets
 */
WORD WAbsoluteToRelative(BYTE* currLoc,DWORD targetLoc) {
	DWORD diff = targetLoc - (DWORD)(currLoc + 2);
	return (WORD)diff;
}

DWORD DWRelativeToAbsolute(BYTE* currLoc, DWORD offset) {
	return (DWORD)(currLoc + 4) + offset;
}

DWORD WRelativeToAbsolute(BYTE* currLoc,WORD offset) {
	//zero-extend the word so negative numbers actually substract
	DWORD dOffset = offset; if (offset & 0x80) offset |= 0xFFFF0000;
	return (DWORD)(currLoc + 2) + dOffset;
}

bool Hook(BYTE* location,std::initializer_list<DWORD> expected, std::initializer_list<DWORD> newData, DWORD* hookedValue) {
	using namespace HookControl;

	int expectedSize = 0;
	//find out how many bytes newData fills
	int newDataSize = 0;
	{DWORD lastControl = 0;
	for(DWORD d : newData) {
		if(lastControl != 0) {
			if (lastControl == RELATIVE_DWORD || lastControl == ABSOLUTE_DWORD) newDataSize += 4;
			else if (lastControl == RELATIVE_WORD || lastControl == ABSOLUTE_WORD) newDataSize += 2;
			else {
				__debugbreak();
				LOGPRIO(Logger::Priority::ERR) << "Unrecognized Hook Control: " << lastControl << "\n";
			}
			lastControl = 0;
		}
		else if (IsHookControl(d)) lastControl = d;
		else newDataSize++;
	}
	}
	int nBytes = newDataSize;

	Memrights writeRights(location,nBytes);
	if (writeRights.good) {
		//first, check if the expected bytes are found
		BYTE* it = location;
		bool err = false;

		for (auto d = expected.begin(); d != expected.end(); d++) {
			if (*d == ANY_DWORD) {
				it += 4;
				expectedSize += 4;
				continue;
			} else if (*it != *d) {
				err = true;
			}
			it++;
			expectedSize++;
		}
		if (expectedSize != newDataSize) {
			LOGPRIONC(Logger::Priority::ERR) std::dec << "Mismatch between expected size and newData size, " << expectedSize << "!=" << newDataSize << "\n";
			__debugbreak();
		}


		if (err) {
			LOGPRIONC(Logger::Priority::WARN) std::dec << "Hook mismatch between expected and found data: expected {";
			g_Logger << std::hex;
			for (auto d = expected.begin(); d != expected.end(); d++) {
				if (*d == ANY_DWORD) {
					g_Logger << "<any dword> ";
					continue;
				}
				g_Logger << *d << " ";
			}
			g_Logger << "}, but found {";
			for (unsigned int i = 0; i < expectedSize; i++)  {
				g_Logger << location[i] << " ";
			}
			g_Logger << "}\r\n";
		}
		//replace bytes/words/dwords
		it = location-1;
		DWORD lastControl = 0;
		for(DWORD d : newData) {
			//if this is a hook control, register that
			if(IsHookControl(d)) {
				lastControl = d;
				continue;
			}
			it++; //pretend it was part of the for loop as well
			DWORD control = lastControl;
			lastControl = 0;
			if(!control) {
				//no control, interpret as byte and apply
				if((d & ~0xFF) != 0) {
					LOGPRIONC(Logger::Priority::WARN) std::dec << "non-control hook parameter was bigger than a BYTE; additional bits will be discarded\r\n";
					__debugbreak();
				}
				*it = (BYTE)d;
			}
			else if(control == ABSOLUTE_DWORD) {
				//write full dword
				if (hookedValue != NULL) *hookedValue  = *(DWORD*)it;
				*(DWORD*)it = d;
				it += 3; //skip 3 additional bytes
			}
			else if(control == ABSOLUTE_WORD) {
				//write full word
				if (hookedValue != NULL) *hookedValue = *(WORD*)it;
				*(WORD*)it = (WORD)d;
				it += 1; //skip 1 additional byte
			}
			else if(control == RELATIVE_DWORD) {
				if (hookedValue != NULL) *hookedValue = DWRelativeToAbsolute(it, *(DWORD*)it);
				*(DWORD*)it = DWAbsoluteToRelative(it,d);
				it += 3;
			}
			else if(control == RELATIVE_WORD) {
				if (hookedValue != NULL) *hookedValue = WRelativeToAbsolute(it,*(WORD*)it);
				*(WORD*)it = WAbsoluteToRelative(it,d);
				it += 1;
			}
		}
		return true;
	}
	else {
		LOGPRIO(Logger::Priority::ERR) << "Failed to hook location " << location << "\n";
		return false;
	}
}

void InsertRedirectCall(void* redirectFunction, void* toCall, int offset) {
	BYTE* funcIt = (BYTE*)redirectFunction;
	BYTE firstOp = *funcIt;
	if (firstOp == 0xE9) {
		//relative jump. probably 32 bit
		funcIt++;
		DWORD offset = *(DWORD*)(funcIt);
		DWORD target = DWRelativeToAbsolute(funcIt, offset);
		funcIt = (BYTE*)target;
	}
	else if (firstOp == 0xFF) {
		//absolute memory jump
		//TODO: Implement
		LOGPRIO(Logger::Priority::CRIT_ERR) << "Absoulte memory jumps are not implemented yet\r\n";
	}
	if (offset == -1) {
		//search nops
		static const BYTE nop5[] = { 0x90,0x90,0x90,0x90,0x90 };
		while (memcmp(funcIt, nop5, 5) != 0) {
			funcIt++;
		}
	}
	else {
		funcIt += offset;
	}
	Memrights rights(funcIt, 5);
	*funcIt = 0xE8;
	funcIt++;
	*(DWORD*)(funcIt) = DWAbsoluteToRelative(funcIt, (DWORD)toCall);
}

DWORD PatchIAT(void *piat, void *newp)
{
	DWORD *iat = (DWORD*)piat;
	Memrights r(iat, 4);
	DWORD orig = *iat;
	*iat = (DWORD)newp;
	return orig;
}

#include "General/ModuleInfo.h"

#include "External/ExternalClasses.h"
#include "External/ExternalVariables.h"
#include "Functions/Shared/Globals.h"
#include "Functions/Shared/Overrides.h"

#include "MemMods/Shared/Events/MeshTexture.h"
#include "MemMods/Shared/Misc/EyeTexture.h"
#include "MemMods/Shared/Misc/FixLocale.h"
#include "MemMods/Shared/Events/ArchiveFileOpen.h"
#include "MemMods/Shared/Events/FileDump.h"
#include "MemMods/Shared/Events/HairMeshes.h"
#include "MemMods/Shared/Events/MemAlloc.h"
#include "MemMods/Shared/Events/GameTick.h"
#include "Hooks\WinAPI.h"


#include "MemMods/AAPlay/Events/HInjections.h"
#include "MemMods/AAPlay/Events/PcConversation.h"
#include "MemMods/AAPlay/Events/Time.h"
#include "MemMods/AAPlay/Events/Loads.h"
#include "MemMods/AAPlay/Misc/TanSlotUnlimit.h"
#include "MemMods/AAPlay/Events/ClothingDialog.h"
#include "MemMods/AAPlay/Events/NpcActions.h"
#include "MemMods/AAPlay/Events/ScreenCapture.h"
#include "MemMods/AAPlay/Events/UiEvent.h"
#include "MemMods/AAPlay/Events/CharacterRender.h"


#include "MemMods/AAEdit/TanSlotUnlimited.h"
#include "MemMods/AAEdit/Dialog.h"

#include "Functions/AAPlay/GameState.h"

void InitializeHooks() {
	ExtVars::InitializeExtVars();

	// These pigeonholes mean jack shit now
	using namespace SharedInjections;
	using namespace PlayInjections;
	using namespace ExtClass;
	using namespace EditInjections;

	//shared
	{

		GameTick::Initialize();
		WinAPI::Inject();

		ArchiveFile::OpenFileInject();
		if (g_Config.getb("bUseMeshTextureOverrides")) {
			LOGPRIO(Logger::Priority::SPAM) << "Mesh texture override init" << "\n";
			MeshTexture::OverrideTextureListSizeInject();
			MeshTexture::OverrideTextureListNameInject();
			MeshTexture::OverrideStartInject();
			MeshTexture::OverrideNameInject();
			MeshTexture::OverrideFileSizeInject();
			MeshTexture::OverrideFileInject();
			MeshTexture::OverrideOutlineColorInject();
			//MeshTexture::OverrideBoneInject();
			//MeshTexture::OverrideBoneInjectV2();
			MeshTexture::OverrideFrameInject();
			MeshTexture::OverrideBoneManipulationInject();

			MeshTexture::OverrideObjectInject();

			MeshTexture::OverrideTanColorInject();

			HairMeshes::HairLoadInject();
			HairMeshes::XXCleanupInjection();

			Shared::GameState::setIsOverriding(General::IsAAEdit); //always override in aaedit
		}
		EyeTexture::EyeTextureInject();

		FileDump::FileDumpStartInject();
		Loads::HiPolyLoadsInjection();
		Shared::PNG::InstallHooks();
		ScreenCapture::InitInjection();
	}

	if (General::IsAAPlay) {
		HGUIButton::InitializeHooks();

		UIEvent::Inject();

		HPlayInjections::TickInjection();
		PcConversation::StartInjection();
		PcConversation::EndInjection();
		PcConversation::TickInjection();	
		PcConversation::NpcAnswerInjection();
		PcConversation::PcAnswerInjection();

		Loads::SaveFileLoadInjection();
		Loads::TransferInInjection();
		Loads::TransferOutInjection();
		if (g_Config.getb("bUseAdditionalTanSlots")) {
			TanSlotUnlimit::LoadLoopStartInject();
			TanSlotUnlimit::LoadLoopPaPointerInject();
			TanSlotUnlimit::LoadLoopEndInject();
			TanSlotUnlimit::InsertLoopCall();
			TanSlotUnlimit::InsertLoopEnd();
			TanSlotUnlimit::LoadLowPolyInject();
		}
		ClothingDialog::InitInjection();
		ClothingDialog::ExitInjection();

		NpcActions::NpcAnswerInjection();
		NpcActions::NpcMovingActionInjection();
		NpcActions::NpcMovingActionPlanInjection();
		NpcActions::RoomChangeInjection();
		NpcActions::hPositionChangeInjection();
		NpcActions::LowPolyUpdateEndInject();
		NpcActions::LowPolyUpdateStartInjectForGirls();
		NpcActions::LowPolyUpdateStartInjectForBoys();
		Time::PeriodChangeInjection();	//most likely PeriodChangeRedirect() needs fixing
//		if (int(g_Config["FixLocale"]) > FixLocale::IsEmulated())
//			FixLocale::PatchAA2Play();
		CharacterRender::InitInjection();
	}
	else if (General::IsAAEdit) {
		if (g_Config.getb("bUseAdditionalTanSlots")) {
			TanSlotUnlimited::LoadLoopStartInject();
			TanSlotUnlimited::LoadLoopPaPointerInject();
			TanSlotUnlimited::LoadLoopEndInject();
			TanSlotUnlimited::InsertLoopCall();
			TanSlotUnlimited::InsertLoopEnd();
		}
		Loads::hairUpdateInject();

#if 0
		SaveCard::AddUnlimitDataInject();
		OpenCard::ReadUnlimitDataInject();
		OpenCard::PreviewCardInject();
#endif
		

		Dialog::DialogProcInject();
	}
	
}
