#pragma once

#include <base/config.h>		 	  		
#include <base/filesystem.h>

namespace base { namespace path {
	_BASE_API bool equal(fs::path const& lhs, fs::path const& rhs);
	_BASE_API fs::path normalize(const fs::path& p);
	_BASE_API fs::path ydwe(bool support_dev);
}}
