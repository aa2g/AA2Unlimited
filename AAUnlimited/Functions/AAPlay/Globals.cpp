#include "Globals.h"

#include "External\ExternalVariables\AAPlay\GameGlobals.h"
#include "Functions\Shared\TriggerEventDistributor.h"

using namespace Shared::Triggers;

namespace AAPlay {



CharInstData g_characters[25];
CharInstData g_previewChar;

void InitOnLoad() {
	for(int i = 0; i < 25; i++) {
		g_characters[i].Reset();
	}

	//initialize characters
	ExtClass::CharacterStruct** start = ExtVars::AAPlay::ClassMembersArray();
	ExtClass::CharacterStruct** end = ExtVars::AAPlay::ClassMembersArrayEnd();
	for(start; start != end; start++) {
		ExtClass::CharacterStruct* it = *start;

		int seat = it->m_seat;
		g_characters[seat].m_char = it;
		g_characters[seat].m_cardData.FromFileBuffer((char*)it->m_charData->m_pngBuffer,it->m_charData->m_pngBufferSize);
		//throw init event
		CardInitializeData data;
		data.card = seat;
		ThrowEvent(&data);
	}
}

void InitTransferedCharacter(ExtClass::CharacterStruct* character) {
	int seat = character->m_seat;
	g_characters[seat].m_char = character;
	g_characters[seat].m_cardData.FromFileBuffer((char*)character->m_charData->m_pngBuffer,character->m_charData->m_pngBufferSize);
	//throw init event
	CardInitializeData data;
	data.card = seat;
	ThrowEvent(&data);
}
void RemoveTransferedCharacter(ExtClass::CharacterStruct* character) {
	int seat = character->m_seat;
	//throw destroy event
	CardDestroyData data;
	data.card = seat;
	ThrowEvent(&data);
	//destroy
	g_characters[seat].Reset();
}

void SetPreviewChar(ExtClass::CharacterStruct* previewChar) {
	g_previewChar.Reset();
	g_previewChar.m_char = previewChar;
	g_previewChar.m_cardData.FromFileBuffer((char*)previewChar->m_charData->m_pngBuffer,previewChar->m_charData->m_pngBufferSize);
}


}