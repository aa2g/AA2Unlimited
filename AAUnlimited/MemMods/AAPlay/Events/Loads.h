#pragma once
#include <Windows.h>

namespace PlayInjections {
namespace Loads {

extern BYTE g_skirtOffOverride;
extern BYTE g_boobGravityOverride;
extern int g_eyeTracking;
extern BYTE g_invisibraOverride[256];


void HiPolyLoadsInjection();
void SaveFileLoadInjection();
void TransferInInjection();
void hairUpdateInject();
void TransferOutInjection();

}
}
