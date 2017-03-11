#include "IllusionUtil.h"

namespace General {

namespace {
	struct Name {
		bool valid;
		char prefix;
		char extraPrefix;
		unsigned int num1;
		unsigned int num2;
		unsigned int num3;
		unsigned int num4;
		unsigned int glassnum;
		char suffix;

		Name(const char* name) : prefix(0),suffix(0),num1(0),num2(0),num3(0),num4(0) {
			valid = false;
			//two prefixes first
			prefix = *name;
			name++; if (!*name) return;
			if (!isdigit(*name)) {
				extraPrefix = *name;
				name++; if (!*name) return;
			}
			else {
				extraPrefix = '\0';
			}
			//now the 4 numbers
			const char* it = NULL;
			num1 = strtoul(name,(char**)&it,10);
			if (*it != '_') { return; }
			it++; if (!*it) return;
			if (*it == '3' && prefix == 'A' && extraPrefix == 'S' && num1 == 0) {
				it++; if (!*it) return;
				num2 = 3;
				glassnum = strtoul(it, (char**)&it, 10);
			}
			else {
				num2 = strtoul(it, (char**)&it, 10);
			}
			if (*it != '_') { return; }
			it++; if (!*it) return;
			num3 = strtoul(it,(char**)&it,10);
			if (*it != '_') { return; }
			it++; if (!*it) return;
			num4 = strtoul(it,(char**)&it,10);
			if (*it) suffix = *it;
			valid = true;
		}
	};
}

ExtClass::CharacterStruct::Models GetModelFromName(const char* name) {
	//names we know:
	//FACE			A00_10_xx_00
	//SKELETON		A00_00_0x_00h | S00_00_0x_00h
	//BODY			(A|S)00_zz_0x_yy	zz = cloth slot, x = body size (small,normal,tall), yy = clothing state | note that (yy == 0) => (zz == 0)
	//HAIR_FRONT	AS00_20_xx_yy (yy is flip)
	//HAIR_SIDE		AS00_21_xx_yy (yy is flip)
	//HAIR_BACK		AS00_22_xx_yy (yy is flip)
	//HAIR_EXT		AS00_23_xx_yy (yy is flip)
	//FACE_SLIDERS	<empty>
	//SKIRT			A02_xx_00_yy (xx = slot, yy = body type)
	if (name == NULL) return ExtClass::CharacterStruct::INVALID;
	Name props{ name };
	if (!props.valid) {
		if (strstr(name, "MP_"))
			return ExtClass::CharacterStruct::H3DROOM;
		else return ExtClass::CharacterStruct::INVALID;
	}
	//if (props.num1 != 0) return ExtClass::CharacterStruct::INVALID;
	if (props.extraPrefix == 'S') {
		//hairs
		switch (props.num2) {
		case 20:
			return ExtClass::CharacterStruct::HAIR_FRONT;
		case 21:
			return ExtClass::CharacterStruct::HAIR_SIDE;
		case 22:
			return ExtClass::CharacterStruct::HAIR_BACK;
		case 23:
			return ExtClass::CharacterStruct::HAIR_EXT;
		case 3:
			return ExtClass::CharacterStruct::GLASSES;
		default:
			return ExtClass::CharacterStruct::INVALID;
		}
	}
	else if (props.extraPrefix == '\0') {
		//other stuff
		if(props.num1 == 0) {
			//face and body: note the implication above, use to differentiate face from body
			if (props.num2 == 10 && props.num4 == 0) return ExtClass::CharacterStruct::FACE;
			if (props.num2 == 11 && props.num4 == 0) return ExtClass::CharacterStruct::TONGUE;
			if (props.num2 == 0 && props.num4 == 0 && props.suffix == 'h') return ExtClass::CharacterStruct::SKELETON;
			if (props.num4 == 0 && props.num2 == 0 || props.num4 != 0) {
				if (props.num3 >= 0 && props.num3 <= 9) {
					if (props.suffix == '\0') return ExtClass::CharacterStruct::BODY;
				}
			}
		}
		else if(props.num1 == 2 && props.num3 == 0) {
			return ExtClass::CharacterStruct::SKIRT;
		}

	}
	return ExtClass::CharacterStruct::INVALID;

}



}