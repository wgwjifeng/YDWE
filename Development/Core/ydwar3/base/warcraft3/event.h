#pragma once

#include <base/warcraft3/config.h>
#include <functional>

namespace base { namespace warcraft3 {
	_WAR3_API void event_game_reset(std::function<void()> func);
}}
