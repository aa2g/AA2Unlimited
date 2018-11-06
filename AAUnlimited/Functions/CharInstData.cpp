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
		call[address]
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

ExtClass::Light * CharInstData::GetLightArray()
{
	if (this->IsValid() && this->m_char->m_xxSkeleton != NULL) {
		auto result = this->m_char->m_xxSkeleton->m_lightsArray;
		return result;
	}
	return nullptr;
}

int CharInstData::GetLightIndex(const char * name)
{
	if (this->IsValid() && this->m_char->m_xxSkeleton != NULL) {
		for (int i = 0; i < this->m_char->m_xxSkeleton->m_lightsCount; i++) {
			//if (strlen(name) != strlen(this->m_char->m_xxSkeleton->m_lightsArray[i].m_name)) continue;
			if (!strcmp(this->m_char->m_xxSkeleton->m_lightsArray[i].m_name, name)) {
				return i;
			}
		}
	}
	return -1;
}

void CharInstData::SetLightMaterialColor(int light, int materialSlot, float red, float green, float blue, float alpha)
{
	if (this->IsValid() && this->m_char->m_xxSkeleton != NULL){
		if (light < this->m_char->m_xxSkeleton->m_lightsCount) {
			if (materialSlot < this->m_char->m_xxSkeleton->m_lightsArray[light].m_materialCount) {
				this->m_char->m_xxSkeleton->m_lightsArray[light].m_material[materialSlot].m_materialRed = red;
				this->m_char->m_xxSkeleton->m_lightsArray[light].m_material[materialSlot].m_materialGreen = green;
				this->m_char->m_xxSkeleton->m_lightsArray[light].m_material[materialSlot].m_materialBlue = blue;
				this->m_char->m_xxSkeleton->m_lightsArray[light].m_material[materialSlot].m_materialAlpha = alpha;
			}
		}
	}
}

void CharInstData::SetLightDirection(int light, float x, float y, float z, float w)
{
	if (this->IsValid() && this->m_char->m_xxSkeleton != NULL) {
		if (light < this->m_char->m_xxSkeleton->m_lightsCount) {
			this->m_char->m_xxSkeleton->m_lightsArray[light].m_matrix2[2][0] = x;
			this->m_char->m_xxSkeleton->m_lightsArray[light].m_matrix2[2][1] = y;
			this->m_char->m_xxSkeleton->m_lightsArray[light].m_matrix2[2][2] = z;
			this->m_char->m_xxSkeleton->m_lightsArray[light].m_matrix2[2][3] = w;
		}
	}
}

std::vector<float> CharInstData::GetLightDirection(int light)
{
	if (this->IsValid() && this->m_char->m_xxSkeleton != NULL) {
		if (light < this->m_char->m_xxSkeleton->m_lightsCount) {
			auto x = this->m_char->m_xxSkeleton->m_lightsArray[light].m_matrix2[2][0];
			auto y = this->m_char->m_xxSkeleton->m_lightsArray[light].m_matrix2[2][1];
			auto z = this->m_char->m_xxSkeleton->m_lightsArray[light].m_matrix2[2][2];
			auto w = this->m_char->m_xxSkeleton->m_lightsArray[light].m_matrix2[2][3];
			return std::vector<float>({ x, y, z, w });
		}
	}
	return std::vector<float>();
}

void CharInstData::Reset() {
	m_char = NULL; //pointer pointing to the illusion data, now invalid
	m_cardData.Reset();
	for (int i = 0; i < 4; i++) m_hairs[i].clear(); //TODO: proper cleanup
}

