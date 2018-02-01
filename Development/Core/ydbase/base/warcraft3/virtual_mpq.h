#pragma once

#include <base/config.h>			  		
#include <base/filesystem.h>
#include <functional>
#include <Windows.h>

namespace base { namespace warcraft3 { namespace virtual_mpq {

	typedef std::function<bool(const std::string&, const void**, uint32_t*, uint32_t)> watch_cb;
	typedef std::function<void(const std::string&, const std::string&)> event_cb;

	_BASE_API bool  initialize(HMODULE module_handle);
	_BASE_API bool  open_path(const fs::path& p, uint32_t priority);
	_BASE_API bool  close_path(const fs::path& p, uint32_t priority);
	_BASE_API void* storm_alloc(size_t n);
	_BASE_API void  watch(const std::string& filename, bool force, watch_cb callback);
	_BASE_API void  event(event_cb callback);
}}}
