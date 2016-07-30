#include "FindProcess.h"

#include <Psapi.h>
#include <string>
#include <vector>

#include "Error.h"
#include "PathInfo.h"
#include "PickExe.h"

#pragma comment( lib, "psapi.lib" )


DWORD FindProcess() {
	int waitCount = 0;

	std::vector<std::string> playExeList = GetPossiblePlayExeList();
	std::vector<std::string> editExeList = GetPossibleEditExeList();
	std::vector<std::pair<std::string, DWORD>>  matches;
	if (playExeList.size() == 0) {
		MessageBox(NULL, "No exe files where found in the AAPlay folder", "Error", MB_ICONERROR);
		return (DWORD)-1;
	}
	if (editExeList.size() == 0) {
		MessageBox(NULL, "No exe files where found in the AAEdit folder", "Error", MB_ICONERROR);
		return (DWORD)-1;
	}

	DWORD procBuffer[2048];
	DWORD nProcs;
	DWORD procId = (DWORD)-1;
	while (true) {
		EnumProcesses(procBuffer, 2048 * sizeof(DWORD), &nProcs);
		char buffer[MAX_PATH];
		for (int i = 0; i < nProcs / sizeof(DWORD); i++) {
			HANDLE proc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
				FALSE, procBuffer[i]);
			if (proc != NULL) {
				if (GetModuleFileNameEx(proc, 0, buffer, MAX_PATH)) {
					//truncate path to file
					char* buffit = buffer;
					while (*buffit) buffit++;
					while (buffit != buffer && *buffit != '\\') buffit--;
					buffit++;
					bool match = false; //make sure we dont count exes twice in single folder installations
						for (const auto& exe : playExeList) {
						if (exe == buffit) {
							match = true;
							procId = procBuffer[i];
							matches.push_back(std::make_pair(buffit, procId));
						}
					}
					if (!match) for (const auto& exe : editExeList) {
						if (exe == buffit) {
							procId = procBuffer[i];
							matches.push_back(std::make_pair(buffit, procId));
						}
					}
				}
			}
			CloseHandle(proc);
		}
		if (procId == (DWORD)-1) {
			if (waitCount < 500) {
				waitCount++;
			}
			else {
				int res = MessageBox(NULL, "Could not find a running AAEdit or AAPlay. Retry?", "Info", MB_YESNO);
				if (res == IDNO) {
					return (DWORD)-1;
				}
			}
		}
		else {
			break;
		}
	}
	if (matches.size() > 1) {
		//more than one exe match
		int sel = ChooseIndex(matches);
		if (sel == -1) return (DWORD)-1;
		procId = matches.at(sel).second;
	}
	return procId;
}