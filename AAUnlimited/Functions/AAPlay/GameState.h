#pragma once

#include "External\ExternalVariables\AAPlay\GameGlobals.h"
#include "Functions\AAPlay\Globals.h"

namespace Shared {
	namespace GameState {

		void reset();

		void setIsClothesScreen(bool value);
		bool getIsClothesScreen();

		void setIsPcConversation(bool value);
		bool getIsPcConversation();

		void setIsOverriding(bool value);
		bool getIsOverriding();
		void updateIsOverriding();

		void setIsHighPolyLoaded(bool value);
		bool getIsHighPolyLoaded();

		void setIsPeeping(bool value);
		bool getIsPeeping();

		void setH_AI(bool value);
		bool getH_AI();

		void setVoyeur(ExtClass::CharacterStruct * voyeur);
		ExtClass::CharacterStruct * getVoyeur();

		void setVoyeurTarget(ExtClass::NpcData * target);
		ExtClass::NpcData * getVoyeurTarget();

		void setPCConversationState(DWORD value);
		DWORD getPCConversationState();

		void setNPCLineState(DWORD value);
		DWORD getNPCLineState();

		void setInterrupt(DWORD value);
		DWORD getInterrupt();

		void SetRoomNumber(int seat, int room);
		int GetRoomNumber(int seat);

		void setInterrupt(int value);
		DWORD getInterrupt();

		void addConversationCharacter(ExtClass::CharacterStruct * chara);
		ExtClass::CharacterStruct * getConversationCharacter(int idx);
		void setConversationCharacter(ExtClass::CharacterStruct * chara, int idx);
		void clearConversationCharacter(int idx);

		inline ExtClass::CharacterStruct * getPlayerCharacter()
		{
			return *(ExtVars::AAPlay::PlayerCharacterPtr());
		}
		void setPlayerCharacter(int seat);

		inline ExtClass::PcConversationStruct * getPlayerConversation()
		{
			return ExtVars::AAPlay::PlayerConversationPtr();
		}

		std::wstring getCurrentClassSaveName();
	}
}