#pragma once

namespace SharedInjections {
	namespace WinAPI {
		typedef struct {
			const char *name;
			void *fn;
			void **old;
		} HookImport;
		void HookImports(HookImport *patches);
		void Inject();
		void *patch_iat(char *func, void *to);

	}
}
