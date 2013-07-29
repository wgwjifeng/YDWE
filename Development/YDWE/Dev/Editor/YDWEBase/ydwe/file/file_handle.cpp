#include <ydwe/file/file_handle.h>
#include <ydwe/exception/windows_exception.h>

namespace ydwe { namespace file {

	file_handle::file_handle(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
		: _Mybase(::CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile))
	{
		if (!_Mybase::operator bool())
		{
			throw windows_exception(L"failed to open file");
		}
	}

	uint64_t file_handle::get_size()
	{
		DWORD   size_high;
		DWORD   size_low   =   ::GetFileSize(_Mybase::get(), &size_high);
		DWORD   error_code =   ::GetLastError();

		if (INVALID_FILE_SIZE == size_low && ERROR_SUCCESS != error_code)
		{
			throw windows_exception(L"failed to determine file size", error_code);
		}

		return (static_cast<uint64_t>(size_high) << 32) | size_low;
	}
}}
