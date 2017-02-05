#pragma once

#include "External\ExternalVariables\AAPlay\GameGlobals.h"

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

		inline ExtClass::CharacterStruct ** getPlayerCharacter()
		{
			return ExtVars::AAPlay::PlayerCharacterPtr();
		}

		inline ExtClass::PcConversationStruct * getPlayerConversation()
		{
			return ExtVars::AAPlay::PlayerConversationPtr();
		}
	}
}