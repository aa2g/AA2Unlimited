#include "CharInstData.h"



CharInstData::CharInstData() : m_char(NULL) {
}


CharInstData::~CharInstData() {
}


void CharInstData::Reset() {
	m_char = NULL; //pointer pointing to the illusion data, now invalid
	m_cardData.Reset();
	for(int i = 0; i < 4; i++) m_hairs[i].clear(); //TODO: proper cleanup
}