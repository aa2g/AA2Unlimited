#include "Overrides.h"

#include <Windows.h>

#include "Files\Logger.h"
#include "Files\Config.h"
#include "General\Util.h"
#include "General\ModuleInfo.h"
#include "Functions\CharInstData.h"
#include "Functions\AAEdit\Globals.h"
#include "Functions\AAPlay\Globals.h"
#include "Functions\TextureImage.h"
#include "Functions\AAUCardData.h"
#include "Functions\Shared\Globals.h"
#include "External\ExternalClasses\CharacterStruct.h"

namespace Shared {


	CharInstData* g_currentChar = &AAEdit::g_currChar;
	bool g_isOverriding = false;

	/********************/
	/* Poly Load Events */
	/********************/
	//note that these events are only fireing in AAPlay, so the current card always stays the global
	//AAEdit::g_cardData if we are in AAEdit

	void MeshTextureCharLoadStart(ExtClass::CharacterStruct* loadCharacter) {
		g_currentChar = &AAPlay::g_characters[loadCharacter->m_seat];
		g_isOverriding = true;
	}

	void MeshTextureCharLoadEnd() {
		g_isOverriding = false;
	}

	/*********************************/
	/* General Archive File Redirect */
	/*********************************/

	namespace {
		TCHAR loc_archiveBuffer[256];
		TCHAR loc_fileBuffer[256];
	}

	bool ArchiveReplaceRules(wchar_t** archive, wchar_t** file, DWORD* readBytes, BYTE** outBuffer) {
		TCHAR* strArchive = General::FindFileInPath(*archive);
		auto match = g_currentChar->m_cardData.GetArchiveRedirectFile(strArchive, *file);
		if (match != NULL) {
			size_t size = (strArchive - *archive); //note that the /2 is done automatically
			wcsncpy_s(loc_archiveBuffer, *archive, size);
			wcscpy_s(loc_archiveBuffer + size, 256 - size, match->first.c_str());
			wcscpy_s(loc_fileBuffer, match->second.c_str());
			*archive = loc_archiveBuffer;
			*file = loc_fileBuffer;
		}
		return false;
	}

	/*********************************/
	/* General Archive File Override */
	/*********************************/

	bool ArchiveOverrideRules(wchar_t* archive, wchar_t* file, DWORD* readBytes, BYTE** outBuffer) {
		TCHAR* strArchive = General::FindFileInPath(archive);

		const OverrideFile* match = g_currentChar->m_cardData.GetArchiveOverrideFile(strArchive, file);
		if (match == NULL) return false;

		void* fileBuffer = Shared::IllusionMemAlloc(match->GetFileSize());
		match->WriteToBuffer((BYTE*)fileBuffer);
		*outBuffer = (BYTE*)fileBuffer;
		*readBytes = match->GetFileSize();
		return true;
	}

	/*************************/
	/* Eye Texture Overrides */
	/*************************/

	namespace {
		TCHAR* loc_savedPointer = NULL;
		BYTE loc_replaceBuffer[1024];
		bool loc_dumpTexture = false;
		const std::vector<BYTE>* loc_textureBuffer;
	};

	void __stdcall EyeTextureStart(int leftRight, TCHAR** texture) {
		const std::wstring& eyeTexture = g_currentChar->m_cardData.GetEyeTexture(leftRight);
		if (eyeTexture.size() > 0) {
			//if usage is 2, texture should be dumped from the buffer directly
			const std::vector<BYTE>& fileSave = g_currentChar->m_cardData.GetEyeTextureBuffer(leftRight);
			if (fileSave.size() > 0 && g_Config.GetKeyValue(Config::SAVED_EYE_TEXTURE_USAGE).iVal == 2) {
				loc_dumpTexture = true;
				loc_textureBuffer = &fileSave;
			}
			else {
				loc_savedPointer = *texture;
				//*texture = (TCHAR*)eyeTexture.c_str();
				memcpy_s((void*)loc_replaceBuffer, 1024, (BYTE*)(*texture) - 16, 16);
				std::wstring fullPath = General::BuildEditPath(TEXT("data\\texture\\eye\\"), eyeTexture.c_str());
				wcscpy_s((TCHAR*)(loc_replaceBuffer + 16), 512 - 16 / 2, fullPath.c_str());
				*texture = (TCHAR*)(loc_replaceBuffer + 16);
			}
			
		}
	}

	BYTE* EyeTextureDump(wchar_t* fileName, DWORD* readBytes) {
		if (loc_dumpTexture) {
			loc_dumpTexture = false;
			void* mem = IllusionMemAlloc(loc_textureBuffer->size());
			memcpy_s(mem, loc_textureBuffer->size(), loc_textureBuffer->data(), loc_textureBuffer->size());
			*readBytes = loc_textureBuffer->size();
			return (BYTE*)mem;
		}
		return FALSE;
	}

	void __stdcall EyeTextureEnd(int leftRight, TCHAR** texture) {
		if (loc_savedPointer != NULL) {
			*texture = loc_savedPointer;
			loc_savedPointer = NULL;
		}
		loc_dumpTexture = false;
	}

	/**************************/
	/* Mesh-Texture overrides */
	/**************************/
	//for more information, look at the MemMods/AAEdit/MeshTexture.cpp

	const TextureImage* MeshTextureOverrideRules(wchar_t* fileName) {
		return g_currentChar->m_cardData.GetMeshOverrideTexture(fileName);
	}

	void XXFileModification(ExtClass::XXFile* xxFile) {
		size_t written;
		TCHAR name[256];
		mbstowcs_s(&written,name,xxFile->m_name,256);
		const auto* match = Shared::g_currentChar->m_cardData.GetBoneRule(name);
		if (match == NULL) return;

		xxFile->EnumBonesPreOrder([&](ExtClass::Bone* bone) {
			mbstowcs_s(&written,name,bone->m_name,256);
			auto it = match->find(name);
			if (it != match->end()) {
				//applies to this rule
				//TODO: all transformations, not only scale
				D3DMATRIX mat = {
					it->second.scales[0],0,0,0,
					0,it->second.scales[1],0,0,
					0,0,it->second.scales[2],0,
					0,0,0,1.0f
				};
				(*Shared::D3DXMatrixMultiply)(&bone->m_matrix1,&mat,&bone->m_matrix1);
				(*Shared::D3DXMatrixMultiply)(&bone->m_matrix5,&mat,&bone->m_matrix5);
			}
		});
		//also do animations
		for (int i = 0; i < xxFile->m_animArraySize; i++) {
			mbstowcs_s(&written,name,xxFile->m_animArray[i].m_name,256);
			auto it = match->find(name);
			if (it != match->end()) {
				for (int j = 0; j < xxFile->m_animArray[i].m_nFrames; i++) {

					xxFile->m_animArray[i].m_frameArray[j].m_scaleX *= it->second.scales[0];
					xxFile->m_animArray[i].m_frameArray[j].m_scaleY *= it->second.scales[1];
					xxFile->m_animArray[i].m_frameArray[j].m_scaleZ *= it->second.scales[2];

				}
			}
		}
	}



}