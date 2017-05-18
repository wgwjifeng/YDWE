#include <windows.h>
#include <shlwapi.h>
#include <string>

void InstallEnv(HINSTANCE module)
{
	/* ��ü���Ҫ�ӵ�Path���ļ���·�� */
	wchar_t binaryPath[MAX_PATH];
	wchar_t pathEnvironmentVariable[32767 /* http://msdn.microsoft.com/en-us/library/windows/desktop/ms683188.aspx */];

	// ��ǰexe·��
	GetModuleFileNameW(module, binaryPath, sizeof(binaryPath) / sizeof(binaryPath[0]));
	PathRemoveBlanksW(binaryPath);
	PathUnquoteSpacesW(binaryPath);
	PathRemoveBackslashW(binaryPath);

	// bin�ļ���·��
	PathRemoveFileSpecW(binaryPath);
	PathAppendW(binaryPath, L"bin");

	// ������������·���ַ���
	std::wstring newPathEnvironmentVariable = binaryPath;

	/* ����PATH�������� */
	// ��ȡ��������
	pathEnvironmentVariable[0] = L'\0';
	DWORD length = GetEnvironmentVariableW(L"PATH", pathEnvironmentVariable, sizeof(pathEnvironmentVariable) / sizeof(pathEnvironmentVariable[0]));

	// ����»�������
	if (length > 0)
		newPathEnvironmentVariable.append(L";");

	newPathEnvironmentVariable.append(pathEnvironmentVariable);

	// ������ȥ
	wcscpy_s(pathEnvironmentVariable, newPathEnvironmentVariable.c_str());

	// ���û�������
	SetEnvironmentVariableW(L"PATH", pathEnvironmentVariable);
}

typedef INT (WINAPI *TPFStartup)(HINSTANCE currentInstance, HINSTANCE previousInstance, LPSTR pCommandLine, INT showType);

INT WINAPI WinMain(HINSTANCE currentInstance, HINSTANCE previousInstance, LPSTR pCommandLine, INT showType)
{
	InstallEnv(currentInstance);

	INT res = -1;
	HMODULE m = LoadLibraryW(L"YDWEStartup.dll");
	if (m)
	{
		TPFStartup pfStartup = (TPFStartup)GetProcAddress(m, "YDWEStartup");
		if (pfStartup) {
			res = pfStartup(currentInstance, previousInstance, pCommandLine, showType);
		}
		FreeLibrary(m);
	}
	return res;
}
