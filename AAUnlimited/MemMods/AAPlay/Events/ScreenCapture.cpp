#include "StdAfx.h"
#include <gdiplus.h>


#pragma comment(lib, "Gdiplus.lib")

namespace PlayInjections {
	namespace ScreenCapture {

		CLSID JPGencoderClsid;
		CLSID PNGencoderClsid;
		Gdiplus::EncoderParameters jpegParameters;

		ULONG jpegQuality;
		bool gdiInit;

		int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
		{
			UINT  num = 0;          // number of image encoders
			UINT  size = 0;         // size of the image encoder array in bytes

			Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;
			Gdiplus::GetImageEncodersSize(&num, &size);
			if (size == 0)
				return -1;  // Failure

			pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
			if (pImageCodecInfo == NULL)
				return -1;  // Failure

			GetImageEncoders(num, size, pImageCodecInfo);

			for (UINT j = 0; j < num; ++j)
			{
				if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
				{
					*pClsid = pImageCodecInfo[j].Clsid;
					free(pImageCodecInfo);
					return j;  // Success
				}
			}

			free(pImageCodecInfo);
			return -1;  // Failure
		}

		void InitGDI() {
			ULONG_PTR gdiToken;
			Gdiplus::GdiplusStartupInput gdiStartupInput;
			gdiStartupInput.GdiplusVersion = 1;
			gdiStartupInput.DebugEventCallback = NULL;
			gdiStartupInput.SuppressBackgroundThread = FALSE;
			gdiStartupInput.SuppressExternalCodecs = FALSE;

			Gdiplus::GdiplusStartup(&gdiToken, &gdiStartupInput, NULL);

			GetEncoderClsid(L"image/jpeg", &JPGencoderClsid);
			GetEncoderClsid(L"image/png", &PNGencoderClsid);

			gdiInit = true;
		}

		void __stdcall SaveAs(DWORD gdiBitmapInfo, DWORD gdiBitmapData, WCHAR* path) {
			if (!gdiInit) {
				InitGDI();
				jpegParameters.Count = 1;
				jpegParameters.Parameter[0].Guid = Gdiplus::EncoderQuality;
				jpegParameters.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
				jpegParameters.Parameter[0].NumberOfValues = 1;
				jpegParameters.Parameter[0].Value = &jpegQuality;
				jpegQuality = 100; // make it look shit, so that people use png instead // F U
			}
			Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromBITMAPINFO((BITMAPINFO*)gdiBitmapInfo, (void*)gdiBitmapData);
			size_t pathLength = wcslen(path);
			const wchar_t *ext = L"jpg";
			auto cls = &JPGencoderClsid;
			auto *params = &jpegParameters;

			if (g_Config.screenshotFormat == 2) {
				cls = &PNGencoderClsid;
				ext = L"png";
				params = NULL;
			}

			wcsncpy_s(path + pathLength - 3, 4, ext, 3);
			bitmap->Save(path, cls, params);
			delete bitmap;
		}

		void __declspec(naked) SaveRedirect() {
			__asm {
				mov edx, [esp + 0xb8]
				mov eax, [edx] // name

				lea ecx, [esp + 0x6C] //bmap info
				mov edx, [esp + 0x3C] //bmap data

				push eax
				push edx
				push ecx
				call SaveAs
				ret
			}
		}

		void InitInjection() {
			gdiInit = false;
			if (!g_Config.screenshotFormat) return;
			// Screenshot formats:
			// 0 - BMP (don't redirect)
			// 1 - JPG
			// 2 - PNG
			
			/*
			00E85EEC   > 8B9424 B4000000MOV EDX,DWORD PTR SS:[ESP+B4]            ;  Full path pointer
			00E85EF3   . 8B02           MOV EAX,DWORD PTR DS:[EDX]               ;  Full path pointer value
			00E85EF5   . E8 66AB0000    CALL AA2Play.00E90A60                    ;  Create file
			*/
			DWORD address = General::GameBase + 0x1C5EEC;
			if (General::IsAAEdit)
				address = General::GameBase + 0x1A868C;
			DWORD redirectAddress = (DWORD)(&SaveRedirect);
			Hook((BYTE*)address,
			{ 0x8B, 0x94, 0x24, 0xB4, 0x00, 0x00, 0x00,				//expected values
			  0x8B, 0x02,
			  0xE8 },
			{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress,	//redirect to our function
			  0xE9, 0x0B, 0x01, 0x00, 0x00 },						//jump to cleanup
			  NULL);

			if (General::IsAAPlay) {
				GameTick::RegisterMsgFilter(GameTick::MsgFilterFunc([](MSG *m) {
					if (m->message == WM_KEYDOWN) {
						DWORD addr = 0;
						if (m->wParam == VK_F9)
							addr = General::GameBase + 0x38F6CA;
						else if (m->wParam == VK_F11 || m->wParam == VK_SNAPSHOT)
							addr = General::GameBase + 0x38F6C9;
						if (addr) {
							*reinterpret_cast<BYTE*>(addr) = 1;
							return true;
						}
					}
					return false;
				}));
			}
		}

	}
}
