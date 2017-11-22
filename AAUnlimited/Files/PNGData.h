// v2 modcards:
// PNG
// IHDR
// IDAT [original png]
// AAUD [aaud_data]
// IEND
// [illusion_data]
// DWORD [sizeof(illusion_data)]
//
// v3 modcards (partially compatible with v1 and v2)
// PNG
// IHDR
// IDAT [original_png]
// AAUD [aaud_data]
// IEND
// [illusion data]
// [aaud_blob_data]
// "MODCARD3"
// DWORD [sizeof(aaud_blob_data)]
// DWORD [offsetof(aaud_data)]
// DWORD [sizeof(illusion_data)+sizeof(aaud_blob_data)+16]



namespace Shared {
namespace PNG {
void InstallHooks();
static const char vermagic[] = "MODCARD3";
void Reset();
struct Footer {
	char vermagic[8];		// keep version in some visible place now
	DWORD aaublob_delta;	// filesize-aaublob_delta must point to blob (and also a blob truncation point)
	DWORD aaud_delta;		// filesize-aaud_delta must point to data part of AAUD chunk
	DWORD illusion_delta;	// filesize-illusion_delta points to illusion data
};

// this is similiar to CharacterDataTypes::Clothes, yet not quite
// move this somewhere relevant when needed
#pragma pack(push, 1)
struct CardClothes {
	char buf[91];
/*
	DWORD slot;
	BYTE skirtLength;
	BYTE socks;
	BYTE indoorShoes;
	BYTE outdoorShoes;

	DWORD colorTop1;
	DWORD colorTop2;
	DWORD colorTop3;
	DWORD colorTop4;
	DWORD colorBottom1;
	DWORD colorBottom2;
	DWORD colorUnderwear;
	DWORD colorSocks;
	DWORD colorIndoorShoes;
	DWORD colorOutdoorShoes;
	DWORD textureBottom1;
	DWORD textureUnderwear;
	DWORD textureBottom1Hue;
	DWORD textureBottom1Lightness;
	DWORD shadowBottom1Hue;
	DWORD shadowBottom1Lightness;

	BYTE isOnePiece;
	BYTE hasUnderwear;
	BYTE hasSkirt;

	DWORD textureUnderwearHue;
	DWORD textureUnderwearLightness;
	DWORD shadowUnderwearHue;
	DWORD shadowUnderwearLightness;*/
};
extern CardClothes saved_clothes[4];
extern bool have_clothes;
#pragma pack(pop)


}
}