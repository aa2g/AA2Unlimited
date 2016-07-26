#include <Windows.h>

/*
 * A RAIR class that manages memory access rights
 */

class Memrights {
private:
	DWORD oldRights;
	LPVOID addr;
	DWORD size;
public:
	bool good;

	inline Memrights::Memrights(LPVOID addr,DWORD size) : addr(addr),size(size) {
		good = VirtualProtect(addr,size,PAGE_EXECUTE_READWRITE,&(this->oldRights)) == TRUE;
	}
	inline Memrights::~Memrights() {
		DWORD tmp;
		if (good) VirtualProtect(addr,size,oldRights,&tmp);
	}
};

