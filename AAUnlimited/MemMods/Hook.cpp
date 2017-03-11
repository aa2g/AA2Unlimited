#include <Windows.h>
#include <initializer_list>

#include "Files/Logger.h"
#include "Files/Config.h"
#include "MemRightsLock.h"

/*
 * Can be used as newData DWORD parameters of the hook function; if used, the parameter
 * will not be interpreted as a byte, but have special meanings
 */
namespace HookControl {
	/*
	 * The following DWORD will be a relative offset of given size
	 */
	const DWORD RELATIVE_DWORD = (DWORD)-1;
	const DWORD RELATIVE_WORD  = (DWORD)-2;
	const DWORD ABSOLUTE_DWORD = (DWORD)-3;
	const DWORD ABSOLUTE_WORD = (DWORD)-4;
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

	int expectedSize = expected.size();
	//find out how many bytes newData fills
	int newDataSize = 0;
	{DWORD lastControl = 0;
	for(DWORD d : newData) {
		if(lastControl != 0) {
			if (lastControl == RELATIVE_DWORD || lastControl == ABSOLUTE_DWORD) newDataSize += 4;
			else if (lastControl == RELATIVE_WORD || lastControl == ABSOLUTE_WORD) newDataSize += 2;
			else {
				LOGPRIO(Logger::Priority::ERR) << "Unrecognized Hook Control: " << lastControl << "\n";
			}
			lastControl = 0;
		}
		else if (IsHookControl(d)) lastControl = d;
		else newDataSize++;
	}
	}
	if(expectedSize != newDataSize) {
		LOGPRIO(Logger::Priority::ERR) << "Mismatch between expected size and newData size\n";
	}
	int nBytes = max(expectedSize, newDataSize);

	Memrights writeRights(location,nBytes);
	if (writeRights.good) {
		//first, check if the expected bytes are found
		BYTE* it = location;
		bool err = false;
		for(DWORD d : expected) {
			if(*it++ != d) {
				err = true;
			}
		}
		if(err) {
			LOGPRIO(Logger::Priority::WARN) << "Hook mismatch between expected and found data: expected {";
			g_Logger << std::hex;
			for (DWORD d : expected) g_Logger << d << " ";
			g_Logger << "}, but found {";
			for (unsigned int i = 0; i < expected.size(); i++)  {
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
					LOGPRIO(Logger::Priority::WARN) << "non-control hook parameter was bigger than a BYTE; additional bits will be discarded\r\n";
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

void InsertRedirectCall(void* redirectFunction, void* toCall, int offset = -1) {
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

#include "General/ModuleInfo.h"

#include "External/ExternalClasses.h"
#include "External/ExternalVariables.h"
#include "Functions/Shared/Globals.h"
#include "Functions/Shared/Overrides.h"

#include "MemMods/Shared/Events/MeshTexture.h"
#include "MemMods/Shared/Misc/EyeTexture.h"
#include "MemMods/Shared/Events/ArchiveFileOpen.h"
#include "MemMods/Shared/Events/FileDump.h"
#include "MemMods/Shared/Events/HairMeshes.h"

#include "MemMods/AAPlay/Events/HInjections.h"
#include "MemMods/AAPlay/Events/PcConversation.h"
#include "MemMods/AAPlay/Events/Loads.h"
#include "MemMods/AAPlay/Misc/TanSlotUnlimit.h"
#include "MemMods/AAPlay/Events/ClothingDialog.h"
#include "MemMods/AAPlay/Events/NpcActions.h"

#include "MemMods/AAEdit/TanSlotUnlimited.h"
#include "MemMods/AAEdit/SaveCard.h"
#include "MemMods/AAEdit/OpenCard.h"
#include "MemMods/AAEdit/Dialog.h"

#include "Functions/AAPlay/GameState.h"

void InitializeHooks() {
	ExtVars::InitializeExtVars();
	Shared::Init();

	//shared
	{
		using namespace SharedInjections;

		ArchiveFile::OpenFileInject();
		if (g_Config.GetKeyValue(Config::USE_MESH_TEXTURE_OVERRIDES).bVal) {
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
	}

	if (General::IsAAPlay) {
		using namespace ExtClass;
		HGUIButton::InitializeHooks();

		using namespace PlayInjections;
		HPlayInjections::TickInjection();
		HPlayInjections::FocusCameraInjection();

		PcConversation::StartInjection();
		PcConversation::EndInjection();
		PcConversation::TickInjection();	
		PcConversation::NpcAnswerInjection();
		PcConversation::PcAnswerInjection();

		Loads::HiPolyLoadsInjection();
		Loads::SaveFileLoadInjection();
		Loads::TransferInInjection();
		Loads::TransferOutInjection();
		if (g_Config.GetKeyValue(Config::USE_TAN_SLOTS).bVal) {
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
	}
	else if (General::IsAAEdit) {
		using namespace EditInjections;
		if (g_Config.GetKeyValue(Config::USE_TAN_SLOTS).bVal) {
			TanSlotUnlimit::LoadLoopStartInject();
			TanSlotUnlimit::LoadLoopPaPointerInject();
			TanSlotUnlimit::LoadLoopEndInject();
			TanSlotUnlimit::InsertLoopCall();
			TanSlotUnlimit::InsertLoopEnd();
		}

		SaveCard::AddUnlimitDataInject();
		OpenCard::ReadUnlimitDataInject();
		OpenCard::PreviewCardInject();

		

		Dialog::DialogProcInject();
	}
	
}