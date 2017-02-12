#pragma once

namespace ExtClass {

#pragma pack(push, 1)

/*
 * An array, as used pretty much everywhere in illusions code base
 */
template<typename T>
class IllusionArray {
public:
	T* m_start;
	T* m_end;

	inline int GetSize() {
		return m_end - m_start;
	}
};

#pragma pack(pop)

}