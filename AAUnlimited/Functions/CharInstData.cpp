#include "StdAfx.h"
#include "Files\PersistentStorage.h"



CharInstData::CharInstData() : m_char(NULL) {
}


CharInstData::~CharInstData() {
}

void CharInstData::SetCurrentStyle(int index)
{
	if (index > GetStyleCount()) return;
	m_cardData.SwitchActiveCardStyle(index, this->m_char->m_charData);

	auto storage = PersistentStorage::ClassStorage::getStorage(Shared::GameState::getCurrentClassSaveName());
	storage.storeCardInt(&AAPlay::g_characters[this->m_char->m_seat], L"m_currCardStyle", index);
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
	return General::CastToString(m_cardData.m_styles[index].m_name).c_str();
}

void CharInstData::Reset() {
	m_char = NULL; //pointer pointing to the illusion data, now invalid
	m_cardData.Reset();
	for(int i = 0; i < 4; i++) m_hairs[i].clear(); //TODO: proper cleanup
}

