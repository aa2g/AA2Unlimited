#include "StdAfx.h"
#include "Files\PersistentStorage.h"



CharInstData::CharInstData() : m_char(NULL) {
}


CharInstData::~CharInstData() {
}

static std::string styleName;

void CharInstData::SetCurrentStyle(int index)
{
	if (index > GetStyleCount()) return;
	m_cardData.SwitchActiveCardStyle(index, this->m_char->m_charData);

	auto storage = PersistentStorage::ClassStorage::getStorage(Shared::GameState::getCurrentClassSaveName());
	storage.storeCardInt(&AAPlay::g_characters[this->m_char->m_seat], L"m_currCardStyle", index);
}

void CharInstData::ApplyDecals(int bodyPart, int decalStrength)
{
	DWORD position = (DWORD)bodyPart;
	DWORD strength = (DWORD)decalStrength;
	const DWORD offset[]{ 0x151900 };
	DWORD* address = (DWORD*)ExtVars::ApplyRule(offset);
	auto somepointer = *(DWORD*)((char*)(this->m_char->m_somePointer) + 0x13c);
	__asm
	{
		mov eax, position
		mov edi, somepointer
		mov ecx, strength
		push ebp
		push ecx
		call [address]
		pop ebp
	}
}

int CharInstData::GetStyleCount()
{
	return m_cardData.m_styles.size();
}

int CharInstData::GetCurrentStyle()
{
	return m_cardData.m_currCardStyle;
}

const char* CharInstData::GetStyleName(int index)
{
	if (index > GetStyleCount()) return "";
	styleName = General::CastToString(m_cardData.m_styles[index].m_name);
	return styleName.c_str();
}

void CharInstData::Reset() {
	m_char = NULL; //pointer pointing to the illusion data, now invalid
	m_cardData.Reset();
	for(int i = 0; i < 4; i++) m_hairs[i].clear(); //TODO: proper cleanup
}

