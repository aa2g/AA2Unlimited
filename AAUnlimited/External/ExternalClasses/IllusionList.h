#pragma once
#include <Windows.h>


namespace ExtClass {

#pragma pack(push, 1)
/*
 * An awkward list structure that can be found several times in the code
 * of both the game and the maker.
 * Its a strictly recursive list implimentation, whose last elements next
 * is not NULLED, but instead points back to its beginning.
 * similarily, the prev of the first element is the last element.
 * In a way, its not really a list, but rather a circle.
 */
class IllusionList
{
public:
	IllusionList* next;
	IllusionList* prev;
	void* data;

public:
	IllusionList() = delete;
	~IllusionList() = delete;
};
#pragma pack(pop)

}