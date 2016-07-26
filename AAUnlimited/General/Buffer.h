#pragma once

namespace General {

/*
* appens dsize bytes from value to buffer[at].
* if resize == true, *buffer might be replaced by a bigger buffer if the data didnt fit
*/
bool BufferAppend(char** buffer,int* size,int at,const char* value,int dsize, bool resize);

}