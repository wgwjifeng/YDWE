require 'utiliy'
require 'config'
require 'ffi.loadlibrary'
local ffi = require 'ffi'
local event = require 'ev'
local storm = require 'ffi.storm'
local hook = require 'ffi.hook'

local _, patch = global_config_war3_version()

hook.iat('void*(__stdcall*)(const char*)', 'War3.exe', 'kernel32.dll', 'LoadLibraryA', function (rLoadLibraryA, dllname)
    dllname = ffi.string(dllname)
    log.info('LoadLibraryA', dllname)
    if dllname:lower() == 'game.dll' then
        local lib
        if patch then
            lib = sys.load_library(patch / 'game.dll')
            storm.open(patch / 'patch.mpq', 9)
        end
        if not lib then
            lib = rLoadLibraryA(dllname)
        end
        event.emit('GameDll加载')
        return lib
    end
    return rLoadLibraryA(dllname)
end)

event.on('GameDll加载', function ()
    sys.load_library(fs.ydwe_path() / 'plugin' / 'warcraft3' / 'yd_loader.dll')
end)

event.on('GameDll加载', function ()
    local plugin = fs.ydwe_path() / "plugin" / "warcraft3"
    local cfg = sys.ini_load(plugin / 'config.cfg')
    local libs = {}
    for file in plugin:list_directory() do	
        if not fs.is_directory(file) 
        and file:filename():string():lower() ~= 'yd_loader.dll'
        and file:extension():string():lower() == '.dll'
        and cfg.Enable[file:filename():string()] ~= '0'
        then
            libs[#libs+1] = sys.load_library(file)
        end
    end
    for _, lib in ipairs(libs) do
        local init = sys.get_proc_address(lib, 'Initialize', 'void(__stdcall*)()')
        if init then
            init()
        end
    end
end)

if '0' ~= global_config.MapTest.LaunchDisableSecurityAccess then
    event.on('GameDll加载', function()
        hook.iat('void*(__stdcall*)(const char*)', 'Game.dll', 'kernel32.dll', 'LoadLibraryA', function (rLoadLibraryA, dllname)
            dllname = ffi.string(dllname)
            if dllname:lower() == 'advapi32.dll' then
                return nil
            end
            return rLoadLibraryA(dllname)
        end)
    end)
end

if 'Direct3D 9' == global_config.MapTest.LaunchRenderingEngine then
    event.on('GameDll加载', function()
        local d3d8proxy = sys.load_library 'd3d8proxy.dll'
        if not d3d8proxy then
            return
        end
        local d3d8create = sys.get_proc_address(d3d8proxy, 'Direct3DCreate8', 'void*(__stdcall*)(unsigned int)')
        if not d3d8create then
            return
        end
        hook.dyn_iat('void*(__stdcall*)(unsigned int)', 'game.dll', "d3d8.dll", "Direct3DCreate8", function(rDirect3DCreate8, ver)
            return d3d8create(ver)
        end)
    end)
end
