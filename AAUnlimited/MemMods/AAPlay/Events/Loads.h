#pragma once
#include <Windows.h>

namespace PlayInjections {
namespace Loads {

extern BYTE g_skirtOffOverride;
extern BYTE g_boobGravityOverride;
extern int g_eyeTracking;


void HiPolyLoadsInjection();
void SaveFileLoadInjection();
void TransferInInjection();
void TransferOutInjection();

}
}
