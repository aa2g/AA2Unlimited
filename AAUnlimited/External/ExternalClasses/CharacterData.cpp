#include "StdAfx.h"

namespace ExtClass {
	void CardStyleData::CopyCharacterData(CharacterData * data)
	{
		this->m_gender = data->m_gender;
		this->m_figure = data->m_figure;
		this->m_chest = data->m_chest;
		this->m_bodyColor = data->m_bodyColor;
		this->m_faceSlot = data->m_faceSlot;
		this->m_eyes = data->m_eyes;
		this->m_eyebrows = data->m_eyebrows;
		this->m_faceDetails = data->m_faceDetails;
		this->m_hair = data->m_hair;	//should be deep copy
		for (int i = 0; i< 4; i++) this->m_clothes[i] = data->m_clothes[i];
	}

	void CardStyleData::CopyCharacterFigureData(CharacterData* data)
	{
		this->m_gender = data->m_gender;
		this->m_figure = data->m_figure;
		this->m_chest = data->m_chest;
		this->m_bodyColor = data->m_bodyColor;
		for (int i = 0; i < 4; i++) this->m_clothes[i] = data->m_clothes[i];
	}

	void CardStyleData::CopyCharacterFaceData(CharacterData* data)
	{
		this->m_faceSlot = data->m_faceSlot;
		this->m_faceDetails = data->m_faceDetails;
		this->m_eyebrows = data->m_eyebrows;
	}

	void CardStyleData::CopyCharacterEyesData(CharacterData* data)
	{
		this->m_eyes = data->m_eyes;
	}

	void CardStyleData::CopyCharacterHairData(CharacterData* data)
	{
		this->m_hair = data->m_hair;
	}

	void CharacterData::CopyCharacterSetData(CardStyleData * data)
	{
		this->m_gender = data->m_gender;
		this->m_figure = data->m_figure;
		this->m_chest = data->m_chest;
		this->m_bodyColor = data->m_bodyColor;
		this->m_faceSlot = data->m_faceSlot;
		this->m_eyes = data->m_eyes;
		this->m_eyebrows = data->m_eyebrows;
		this->m_faceDetails = data->m_faceDetails;
		this->m_hair = data->m_hair;	//should be deep copy
		for (int i = 0; i< 4; i++) this->m_clothes[i] = data->m_clothes[i];
	}
}