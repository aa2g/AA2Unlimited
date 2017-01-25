#pragma once

#include "External\ExternalClasses\HClasses\HInfo.h"
#include "External\ExternalClasses\Frame.h"

namespace Facecam {

void PostTick(ExtClass::HInfo* hInfo, bool tickRetVal);
void AdjustCamera(ExtClass::Frame* bone);
void Cleanup();

}