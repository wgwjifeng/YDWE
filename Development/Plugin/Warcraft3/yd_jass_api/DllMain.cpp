#include <windows.h>

namespace base { namespace warcraft3 { namespace japi {
	void initialize();
}}}

void Initialize()
{
	base::warcraft3::japi::initialize();
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID /*pReserved*/)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(module);
	}

	return TRUE;
}

// 6F2978FC
