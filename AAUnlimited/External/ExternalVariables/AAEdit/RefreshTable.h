#pragma once

#include <Windows.h>

namespace ExtVars {
namespace AAEdit {


enum Category {
	BAR = 0, //the bar with the tabs
	SYSTEM = 1,FIGURE = 2,CHEST = 3,BODY_COLOR = 4,FACE = 5,EYES = 6, EYE_COLOR = 7,
	EYEBROWS = 8, FACE_DETAILS = 9, HAIR = 10, HAIR_COLOR = 11, CHARACTER = 12, PERSONALITY = 13, TRAITS = 14
};

enum RedrawId {
	FIGURE_HEIGHT = 4, //reloads entire model
	HAIR_FRONT = 0x16, HAIR_EXTENSION = 0x17, HAIR_SIDE = 0x18, HAIR_BACK = 0x19,
	EYES_ALL = 0x13, //used multiple times
	FACEDETAILS_LIPCOLOR = 0x13,FACEDETAILS_GLASSES = 0x14, BODYCOLOR_SKINTONE = 0xa,
	BODYCOLOR_TAN = 0x10, BODYCOLOR_MOSAIC = 0x11, BODYCOLOR_PUBHAIR = 0xB, BODYCOLOR_NIPS = 0x8

};

void RedrawBodyPart(Category cat, RedrawId redraw);


}
}
