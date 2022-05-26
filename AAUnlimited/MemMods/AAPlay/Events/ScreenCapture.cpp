#include "StdAfx.h"

namespace PlayInjections {
	namespace ScreenCapture {

		CLSID JPGencoderClsid;
		CLSID PNGencoderClsid;
		Gdiplus::EncoderParameters jpegParameters;

		ULONG jpegQuality;
		bool gdiInit;

		bool AltScreenshotFormat = false;
		bool AltScreenshotFormatInstant = false;

		struct {
			const wchar_t *extension = nullptr;
			CLSID *encoderId = nullptr;
			Gdiplus::EncoderParameters *parameters = nullptr;
		} encoders[2];

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

		void __stdcall SaveAs(DWORD gdiBitmapInfo, DWORD gdiBitmapData, WCHAR* path) {
			if (!gdiInit) {
				gdiInit = true;
				GetEncoderClsid(L"image/jpeg", &JPGencoderClsid);
				GetEncoderClsid(L"image/png", &PNGencoderClsid);

				jpegParameters.Count = 1;
				jpegParameters.Parameter[0].Guid = Gdiplus::EncoderQuality;
				jpegParameters.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
				jpegParameters.Parameter[0].NumberOfValues = 1;
				jpegParameters.Parameter[0].Value = &jpegQuality;
				jpegQuality = 100; // make it look shit, so that people use png instead // F U

				encoders[0].extension = L"jpg";
				encoders[0].encoderId = &JPGencoderClsid;
				encoders[0].parameters = &jpegParameters;

				encoders[1].extension = L"png";
				encoders[1].encoderId = &PNGencoderClsid;
			}

			// Update this function when we handle more formats. This shouldn't be called when screenshot format is BMP.
			assert(g_Config.screenshotFormat > 0 && g_Config.screenshotFormat <= 2);

			int screenshotFormat = g_Config.screenshotFormat - 1;
			screenshotFormat = AltScreenshotFormat || AltScreenshotFormatInstant ? !screenshotFormat : screenshotFormat;

			Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromBITMAPINFO((BITMAPINFO*)gdiBitmapInfo, (void*)gdiBitmapData);
			size_t pathLength = wcslen(path);

#define POSER_THUMBNAIL_WIDTH 1024
#define POSER_THUMBNAIL_HEIGHT 576
			auto poserMessage = General::IsAAPlay ? *reinterpret_cast<BYTE*>(General::GameBase + 0x38F6C9) : *reinterpret_cast<BYTE*>(General::GameBase + 0x36C6C1);
			if ((poserMessage & 0xF) == 0xF) {
				Gdiplus::Bitmap* poserThumb = new Gdiplus::Bitmap(POSER_THUMBNAIL_WIDTH, POSER_THUMBNAIL_HEIGHT, bitmap->GetPixelFormat());
				poserThumb->SetResolution(bitmap->GetHorizontalResolution(), bitmap->GetVerticalResolution());
				{
					auto scaleRatio = (Gdiplus::REAL)POSER_THUMBNAIL_HEIGHT / (Gdiplus::REAL)bitmap->GetHeight();
					auto horizontalOffset = ((Gdiplus::REAL)POSER_THUMBNAIL_WIDTH - ((Gdiplus::REAL)bitmap->GetWidth()) * scaleRatio) / 2.0f;
					horizontalOffset = max(horizontalOffset, 0) / scaleRatio;
					Gdiplus::Graphics g(poserThumb);
					g.ScaleTransform(scaleRatio, scaleRatio);
					g.DrawImage(bitmap, static_cast<int>(horizontalOffset), 0);
					auto thumbnailOverlay = poserMessage == 0x0F ? /*0x0F*/ General::BuildAAUPath(L"PoseOverlay.png") : /*0x1F*/ General::BuildAAUPath(L"SceneOverlay.png");
					auto overlay = new Gdiplus::Bitmap(thumbnailOverlay.c_str());
					overlay->SetResolution(bitmap->GetHorizontalResolution(), bitmap->GetVerticalResolution());
					g.ResetTransform();
					g.DrawImage(overlay, 0, 0);
					delete overlay;
				}
				auto screenshotFilePath = General::BuildPlayPath(L"poser-screenshot.png");
				poserThumb->Save(screenshotFilePath.c_str(), encoders[1].encoderId, encoders[1].parameters);
				delete poserThumb;
				LUA_EVENT_NORET("poser_saved_thumbnail");
			}
#undef POSER_THUMBNAIL_WIDTH
#undef POSER_THUMBNAIL_HEIGHT
			else {
				wcsncpy_s(path + pathLength - 3, 4, encoders[screenshotFormat].extension, 3);
				bitmap->Save(path, encoders[screenshotFormat].encoderId, encoders[screenshotFormat].parameters);
			}
			delete bitmap;

			//Notifications::AddNotification(L"Screenshot saved (" + std::wstring(path) + L")", RegularNotification);

			AltScreenshotFormatInstant = false;
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
						else if (m->wParam == VK_PAUSE) { // For some reason I can't use the F10 key?
							addr = General::GameBase + 0x38F6C9;
							AltScreenshotFormatInstant = true;
						}

						if (m->wParam == VK_SHIFT)
							AltScreenshotFormat = true;

						if (addr) {
							*reinterpret_cast<BYTE*>(addr) = 1;
							return true;
						}
					}
					else if (m->message == WM_KEYUP) {
						if (m->wParam == VK_SHIFT)
							AltScreenshotFormat = false;
					}
					return false;
				}));
			}
		}

	}
}
