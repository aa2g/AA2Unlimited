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

ExtClass::Light* CharInstData::GetLightArray()
{
	if (this->IsValid() && this->m_char->m_xxSkeleton != NULL) {
		auto result = this->m_char->m_xxSkeleton->m_lightsArray;
		return result;
	}
	return nullptr;
}

int CharInstData::GetLightIndex(const char* name)
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
	if (this->IsValid() && this->m_char->m_xxSkeleton != NULL) {
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
			this->m_char->m_xxSkeleton->m_lightsArray[light].m_lightMatrix[2][0] = x;
			this->m_char->m_xxSkeleton->m_lightsArray[light].m_lightMatrix[2][1] = y;
			this->m_char->m_xxSkeleton->m_lightsArray[light].m_lightMatrix[2][2] = z;
			this->m_char->m_xxSkeleton->m_lightsArray[light].m_lightMatrix[2][3] = w;
		}
	}
}

void CharInstData::SetAmbientColor(BYTE red, BYTE green, BYTE blue)
{
	if (this->IsValid()) {
		ExtClass::XXFile* xxlist[] = {
			this->m_char->m_xxFace, this->m_char->m_xxGlasses, this->m_char->m_xxFrontHair, this->m_char->m_xxSideHair,
			this->m_char->m_xxBackHair, this->m_char->m_xxHairExtension, this->m_char->m_xxTounge, this->m_char->m_xxSkeleton,
			this->m_char->m_xxBody, this->m_char->m_xxLegs, this->m_char->m_xxSkirt
		};
		if (this->m_char->m_charData->m_gender == 0) {
			xxlist[10] = NULL;
		}

		for (ExtClass::XXFile* file : xxlist) {
			if (file == NULL) continue;
			file->m_ambientLightBlue = blue;
			file->m_ambientLightGreen = green;
			file->m_ambientLightRed = red;
		}
	}
}

std::vector<float> CharInstData::GetLightMaterialColor(int light, int materialSlot)
{
	if (this->IsValid() && this->m_char->m_xxSkeleton != NULL) {
		if (light < this->m_char->m_xxSkeleton->m_lightsCount) {
			if (materialSlot < this->m_char->m_xxSkeleton->m_lightsArray[light].m_materialCount) {
				auto r = this->m_char->m_xxSkeleton->m_lightsArray[light].m_material[materialSlot].m_materialRed;
				auto g = this->m_char->m_xxSkeleton->m_lightsArray[light].m_material[materialSlot].m_materialGreen;
				auto b = this->m_char->m_xxSkeleton->m_lightsArray[light].m_material[materialSlot].m_materialBlue;
				auto a = this->m_char->m_xxSkeleton->m_lightsArray[light].m_material[materialSlot].m_materialAlpha;
				return std::vector<float>({ r, g, b, a });
			}
		}
	}
	return std::vector<float>();
}

std::vector<float> CharInstData::GetLightDirection(int light)
{
	if (this->IsValid() && this->m_char->m_xxSkeleton != NULL) {
		if (light < this->m_char->m_xxSkeleton->m_lightsCount) {
			auto x = this->m_char->m_xxSkeleton->m_lightsArray[light].m_lightMatrix[2][0];
			auto y = this->m_char->m_xxSkeleton->m_lightsArray[light].m_lightMatrix[2][1];
			auto z = this->m_char->m_xxSkeleton->m_lightsArray[light].m_lightMatrix[2][2];
			auto w = this->m_char->m_xxSkeleton->m_lightsArray[light].m_lightMatrix[2][3];
			return std::vector<float>({ x, y, z, w });
		}
	}
	return std::vector<float>();
}

std::vector<BYTE> CharInstData::GetAmbientColor()
{
	if (this->IsValid() && this->m_char->m_xxSkeleton != NULL) {
		auto r = this->m_char->m_xxSkeleton->m_ambientLightRed;
		auto g = this->m_char->m_xxSkeleton->m_ambientLightGreen;
		auto b = this->m_char->m_xxSkeleton->m_ambientLightBlue;
		return std::vector<BYTE>({ r, g, b });
	}
	return std::vector<BYTE>();
}

int CharInstData::GetLoveTowards(CharInstData* towards)
{
	auto* relations = this->m_char->GetRelations();
	auto* it = relations->m_start;
	for (it; it != relations->m_end; it++) {
		if (it->m_targetSeat == towards->m_char->m_seat) break;
	}
	if (it == relations->m_end) return 0;

	return it->m_lovePoints + it->m_loveCount * 30;
}

int CharInstData::GetLikeTowards(CharInstData* towards)
{
	auto* relations = this->m_char->GetRelations();
	auto* it = relations->m_start;
	for (it; it != relations->m_end; it++) {
		if (it->m_targetSeat == towards->m_char->m_seat) break;
	}
	if (it == relations->m_end) return 0;

	return it->m_likePoints + it->m_likeCount * 30;
}

int CharInstData::GetDislikeTowards(CharInstData* towards)
{
	auto* relations = this->m_char->GetRelations();
	auto* it = relations->m_start;
	for (it; it != relations->m_end; it++) {
		if (it->m_targetSeat == towards->m_char->m_seat) break;
	}
	if (it == relations->m_end) return 0;

	return it->m_dislikePoints + it->m_dislikeCount * 30;
}

int CharInstData::GetHateTowards(CharInstData* towards)
{
	auto* relations = this->m_char->GetRelations();
	auto* it = relations->m_start;
	for (it; it != relations->m_end; it++) {
		if (it->m_targetSeat == towards->m_char->m_seat) break;
	}
	if (it == relations->m_end) return 0;

	return it->m_hatePoints + it->m_hateCount * 30;
}

int CharInstData::GetCurrentRoom()
{
	if (this->m_char->m_npcData != nullptr) {
		if (this->m_char->m_npcData->roomPtr != nullptr) {
			if ((((int*)this->m_char->m_npcData->roomPtr) + 5) != nullptr) {
				auto roomno = *(((int*)this->m_char->m_npcData->roomPtr) + 5);
				return roomno;
			}
			else return -1;
		}
		else return -1;
	}
	else return -1;
}

bool CharInstData::IsValid() {
	if (General::IsAAEdit) return Editable();
	ExtClass::CharacterStruct** start = ExtVars::AAPlay::ClassMembersArray();
	ExtClass::CharacterStruct** end = ExtVars::AAPlay::ClassMembersArrayEnd();
	for (start; start != end; start++) {
		ExtClass::CharacterStruct* it = *start;
		if (it == m_char) {
			return m_char->m_seat + 1;
		}
	}
	return false;
}

bool CharInstData::IsPC() {
	if (!this->IsValid()) return false;
	return (Shared::GameState::getPlayerCharacter())->m_char->m_seat == this->m_char->m_seat;
}

void CharInstData::Reset() {
	m_char = NULL; //pointer pointing to the illusion data, now invalid
	m_cardData.Reset();
	for (int i = 0; i < 4; i++) m_hairs[i].clear(); //TODO: proper cleanup
}

