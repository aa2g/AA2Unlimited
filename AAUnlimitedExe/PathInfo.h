#pragma once

#include <vector>
#include <string>

extern std::string g_AAPlayPath;
extern std::string g_AAEditPath;
bool GetPathsFromRegistry();

std::vector<std::string> GetPossiblePlayExeList();
std::vector<std::string> GetPossibleEditExeList();
