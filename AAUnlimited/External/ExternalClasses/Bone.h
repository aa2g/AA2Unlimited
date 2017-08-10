#pragma once

#include <Windows.h>
#include <d3d9.h>

namespace ExtClass {

	class Frame;

	class Bone
	{
	public:
		DWORD m_nameBufferSize;
		char* m_name;
		void* m_unknownPointer;

		D3DMATRIX m_matrix;

		D3DMATRIX* m_attachmentMatrix; //actually a pointer into a frame struct, offset 0x54 (the second matrix).
									   //seems to be the corresponding frame to this bone
	public:
#define LUA_CLASS ExtClass::Bone
		static inline void bindLua() {
			LUA_BINDSTRP(m_name);
			LUA_BINDARRE(m_matrix, .m[0], 16);
			LUA_BINDARRE(m_attachmentMatrix, ->m[0], 16 || !_self->m_attachmentMatrix);
			LUA_METHOD(GetFrame, { return _gl.push(_self->GetFrame()).one; });
			LUA_METHOD(SetFrame, { _self->SetFrame(_gl.get(1)); })
		}
#undef LUA_CLASS
		inline Frame* GetFrame() {
			if (m_attachmentMatrix == NULL) return NULL;
			return (Frame*)((BYTE*)(m_attachmentMatrix)-0x54);
		}

		inline int SetFrame(Frame* newFrame) {
			if (newFrame == NULL) m_attachmentMatrix = NULL;
			else m_attachmentMatrix = (D3DMATRIX*)((BYTE*)(newFrame)+0x54);
			return 0;
		}
	};

	static_assert(sizeof(Bone) == 0x50,"size mismatch"); //full size known, lea eax, [eax+eax*4], shl eax, 4

}