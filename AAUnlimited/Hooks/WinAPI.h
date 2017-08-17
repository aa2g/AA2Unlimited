#pragma once

namespace SharedInjections {
	namespace WinAPI {
		
		void Inject();
		void *patch_iat(char *func, void *to);

	}
}
