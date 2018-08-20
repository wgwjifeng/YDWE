#include <base/util/console.h>
#include <windows.h>
#include <cstdio>
#include <base/thread/lock/spin.h>
#include <queue>
#include <mutex>

namespace base { namespace console {

	void enable()
	{
		HWND h = ::GetConsoleWindow();

		if (h)
		{
			::ShowWindow(h, SW_SHOW);
		}
		else
		{
			FILE* new_file;
			::AllocConsole();
			freopen_s(&new_file, "CONIN$", "r", stdin);
			freopen_s(&new_file, "CONOUT$", "w", stdout);
			freopen_s(&new_file, "CONOUT$", "w", stderr);
		}
	}
	
	void disable()
	{
		::ShowWindow(::GetConsoleWindow(), SW_HIDE);
	}

	bool disable_close_button()
	{
		HWND h = ::GetConsoleWindow();
		if (!h)
		{
			return false;
		}

		HMENU hmenu = ::GetSystemMenu(h, FALSE);   
		return !!::RemoveMenu(hmenu, SC_CLOSE, MF_BYCOMMAND);
	}

	void pause()
	{
		system("pause");
	}


	static std::queue<read_req_t*> queue;
	static lock::spin<> mtx_queue;
	static lock::spin<> mtx_reading;

	DWORD CALLBACK async_read_thread(void* data)
	{
		read_req_t* req = (read_req_t*)data;
		DWORD read_bytes = 0;
		if (::ReadConsoleW(req->handle, req->buffer, sizeof(req->buffer) / sizeof(req->buffer[0]) - 1, &read_bytes, NULL))
		{
			req->buffer[read_bytes] = L'\0';
			req->overlapped.Internal = 0;
			req->overlapped.InternalHigh = read_bytes;
		}
		else
		{
			req->overlapped.Internal = (ULONG_PTR)::GetLastError();
		}
		{
			std::unique_lock<lock::spin<>> lock(mtx_queue);
			queue.push(req);
		}
		mtx_reading.unlock();
		return 0;
	}

	bool read_post()
	{
		if (!mtx_reading.try_lock()) return false;
		console::read_req_t* req = new console::read_req_t;
		req->handle = ::GetStdHandle(STD_INPUT_HANDLE);
		bool suc = !!QueueUserWorkItem(async_read_thread, (void*)req, WT_EXECUTELONGFUNCTION);
		if (!suc)
		{
			delete req;
			mtx_reading.unlock();
		}
		return suc;
	}

	bool read_try(read_req_t*& req)
	{
		std::unique_lock<lock::spin<>> lock(mtx_queue);
		if (queue.empty()) {
			return false;
		}
		req = queue.front();
		queue.pop();
		return true;
	}

	void read_release(read_req_t* req)
	{
		delete req;
	}
}}
