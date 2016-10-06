#pragma once

#include "External\ExternalClasses\HClasses\HInfo.h"
#include "External\ExternalClasses\Bone.h"

namespace Facecam {

void PostTick(ExtClass::HInfo* hInfo, bool tickRetVal);
void AdjustCamera(ExtClass::Bone* bone);

}