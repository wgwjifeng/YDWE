#include <lua.hpp>
#include <base/warcraft3/virtual_mpq.h>	  
#include <base/lua/object.h>
#include <base/lua/guard.h>
#include <LuaEngine/logging.h>

static bool VirtualMpqWatchCB(const base::lua::object& func, const std::string& filename, const void** pbuf, uint32_t* plen, uint32_t reserve_size)
{
	lua_State* L = func.l();
	base::lua::guard guard(L);

	try {
		func.push();
		lua_pushlstring(L, filename.data(), filename.size());
		if (LUA_OK != lua_pcall(L, 1, 1, 0)) {
			throw std::exception(lua_tostring(L, -1));
		}
	} catch (const std::exception& e) {
		LOGGING_ERROR(logging::get_logger("lua")) << e.what();
		return false;
	}

	if (LUA_TSTRING != lua_type(L, -1))
	{
		return false;
	}

	size_t len = 0;
	const char* buf = lua_tolstring(L, -1, &len);
	void* tmpbuf = base::warcraft3::virtual_mpq::storm_alloc(len + reserve_size);
	if (!tmpbuf)
	{
		return false;
	}
	memcpy(tmpbuf, buf, len);
	*pbuf = tmpbuf;
	if (reserve_size) memset((unsigned char*)tmpbuf + len, 0, reserve_size);
	if (plen) *plen = len;
	lua_pop(L, 1);
	return true;
}

static int VirtualMpqOpenPath(lua_State* L)
{
	base::warcraft3::virtual_mpq::open_path(*(fs::path*)luaL_checkudata(L, 1, "filesystem"), (uint32_t)luaL_checkinteger(L, 2));
	return 0;
}

static int VirtualMpqWatch(lua_State* L)
{
	size_t len = 0;
	const char* str = luaL_checklstring(L, 1, &len);
	base::warcraft3::virtual_mpq::watch(std::string(str, len), false, std::bind(VirtualMpqWatchCB, base::lua::object(L, 2), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	return 0;
}

static int VirtualMpqForceWatch(lua_State* L)
{
	size_t len = 0;
	const char* str = luaL_checklstring(L, 1, &len);
	base::warcraft3::virtual_mpq::watch(std::string(str, len), true, std::bind(VirtualMpqWatchCB, base::lua::object(L, 2), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	return 0;
}

static void VirtualMpqEventCB(const base::lua::object& func, const std::string& name, const std::string& data)
{
	lua_State* L = func.l();
	base::lua::guard guard(L);

	try {
		func.push();
		lua_pushlstring(L, name.data(), name.size());
		lua_pushlstring(L, data.data(), data.size());
		if (LUA_OK != lua_pcall(L, 2, 0, 0)) {
			throw std::exception(lua_tostring(L, -1));
		}
	}
	catch (const std::exception& e) {
		LOGGING_ERROR(logging::get_logger("lua")) << e.what();
	}
}

static int VirtualMpqEvent(lua_State* L)
{
	base::warcraft3::virtual_mpq::event(std::bind(VirtualMpqEventCB, base::lua::object(L, 1), std::placeholders::_1, std::placeholders::_2));
	return 0;
}

int luaopen_virtual_mpq(lua_State* L)
{
	base::warcraft3::virtual_mpq::initialize(::GetModuleHandleW(NULL));

	luaL_Reg l[] = {
		{ "open_path", VirtualMpqOpenPath },
		{ "watch", VirtualMpqWatch },
		{ "force_watch", VirtualMpqForceWatch },
		{ "event", VirtualMpqEvent },
		{ NULL, NULL },
	};
	luaL_newlib(L, l);
	lua_setglobal(L, "virtual_mpq");
	return 0;
}
