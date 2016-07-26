#include "Buffer.h"

#include <Windows.h>

namespace General {


bool BufferAppend(char** buffer,int* size,int at,const char* value,int dsize, bool resize) {
	int currSize = *size;
	int leftSize = currSize - at;
	if(leftSize < dsize) {
		if (!resize) return false;
		//resize buffer to fit
		int oldsize = currSize;
		if (currSize == 0) currSize = 16; //in case size is 0, this loop wouldnt finish
		do {
			currSize *= 2;
			leftSize = currSize - at;
		} while (leftSize < dsize);
		char* newBuffer = new char[currSize];
		if(*buffer != NULL) {
			memcpy(newBuffer,*buffer,oldsize);
			delete[] *buffer;
		}
		*buffer = newBuffer;
		*size = currSize;
	}

	memcpy(*buffer+at,value,dsize);
	return true;
}


}