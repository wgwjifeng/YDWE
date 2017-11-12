#pragma once

#include <windows.h>

#include <base/filesystem.h>
#include <base/thread/thread.h>
#include <atomic>
#include <map>	 

class DllModule
{
public:
	DllModule();

	void Attach();
	void Detach();
	void ThreadStart();
	void ThreadStop();
	void ThreadFunc();
	void SetWindow(HWND hwnd);

	HMODULE  hGameDll;
	HWND     hWar3Wnd;
	bool     IsWindowMode;
	bool     IsAuto;
	bool     IsFullWindowedMode;
	bool     IsLockingMouse;
	bool     IsFixedRatioWindowed;
	bool     IsWideScreenSupport;
	fs::path ydwe_path;

private:
	std::shared_ptr<base::thread>   daemon_thread_;
	std::atomic<bool>               daemon_thread_exit_;
	std::map<std::string, HMODULE>  plugin_mapping;
};

extern DllModule g_DllMod;
