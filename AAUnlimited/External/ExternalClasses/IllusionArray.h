#pragma once

namespace ExtClass {

#pragma pack(push, 1)

/*
 * An array, as used pretty much everywhere in illusions code base.
 * Im pretty sure that this is just a std::vector, evidenced by the size, storage method, alignment of members,
 * and most importantly, the text in some appending methods referencing this struct as "vector<T>", 
 * but i cant make any sense of the first member, so that might or might not be true
 */
template<typename T>
class IllusionArray {
public:
	DWORD m_unknown;
	T* m_start;		//first valid member
	T* m_end;
	T* m_allocEnd;

	inline int GetSize() {
		return m_end - m_start;
	}
	inline T& operator[](int index) {
		return *(m_start + index);
	}
};

#pragma pack(pop)

}