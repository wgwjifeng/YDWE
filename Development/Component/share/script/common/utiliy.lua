require "log"
require "filesystem"
local process = require "process"

sys = {}

local uni = require 'ffi.unicode'

fs.__ydwe_path = fs.ydwe(false)
fs.__ydwe_devpath = fs.ydwe(true)
log.debug('ydwe path ' .. fs.__ydwe_path:string())
if fs.__ydwe_path ~= fs.__ydwe_devpath then
    log.debug('ydwe dev path ' .. fs.__ydwe_devpath:string())
end

function fs.ydwe_path()
	return fs.__ydwe_path
end

function fs.ydwe_devpath()
	return fs.__ydwe_devpath
end

function fs.war3_path()
	return fs.__war3_path
end

require "localization"

io.__open = io.open
function io.open(file_path, mode)
	local f, e = io.__open(file_path:string(), mode)
	if f then
		if not mode or (not mode:match('b') and mode:match('r'))  then
			if f:read(3) ~= '\xEF\xBB\xBF' then
				f:seek('set', 0)
			end
		end
	end
	return f, e
end

io.__lines = io.lines
function io.lines(file_path)
	return io.__lines(file_path:string())
end

-- 载入一个文件的内容
-- file_path - 文件路径, 必须是fs.path类型
-- 返回文件内容, nil表示出错
function io.load(file_path)
	local f, e = io.open(file_path, "rb")

	if f then
		local content = f:read("*a")
		f:close()
		return content
	else
		return nil, e
	end
end

-- 保存一个文件的内容
-- file_path - 文件路径, 必须是fs.path类型
-- content - 文件内容,必须是字符串
-- 返回true表示成功,false表示失败
function io.save(file_path, content)
	local f, e = io.open(file_path, "wb")

	if f then
		f:write(content)
		f:close()
		return true
	else
		return false, e
	end
end

local function trim(str) 
	return str:gsub("^%s*(.-)%s*$", "%1")
end

function sys.spawn (command_line, current_dir, wait)
	local p = process()
	if not p:create(nil, command_line, current_dir) then
		log.error(string.format("Executed %s failed", command_line))
		return false
	end

	if wait then
		local exit_code = p:wait()
		p:close()
		p = nil
		log.trace(string.format("Executed %s, returned %d", command_line, exit_code))
		return exit_code == 0
	end
	
	p:close()
	p = nil	
	log.trace(string.format("Executed %s.", command_line))
	return false
end

function sys.ini_load (path)
	local f, e = io.open(path, "r")
	if not f then
		return nil, e
	end
	local tbl = {}
	local section = nil
	for line in f:lines() do
		line = trim(line)
		if string.sub(line,1,1) == "[" then
			section = trim(string.sub(line, 2, string.len(line) - 1 ))
			tbl[section] = {}
		elseif string.sub(line,1,2) == "//" then
		elseif line ~= "" then
			local key = trim(string.sub(line, 1, string.find(line, "=") - 1))
			local value = trim(string.sub(line, string.find(line, "=") + 1))
			tbl[section][key] = value or ""
		end
	end
	f:close()
	return tbl
end

local ffi = require 'ffi'
ffi.cdef[[
    const wchar_t* __stdcall GetCommandLineW();
    bool __stdcall TerminateProcess(int hProcess, unsigned int uExitCode);
    int  __stdcall GetCurrentProcess();
]]

function sys.reboot(map)
    local p = process()
    local ydwe = fs.get(fs.DIR_EXE)
    local cmd = '"'.. ydwe:string() .. '"'
    if map then
        cmd = cmd .. ' -loadfile "' .. map .. '"'
    end
	if not p:create(ydwe, cmd, fs.current_path()) then
		return
	end
    p:close()
    ffi.C.TerminateProcess(ffi.C.GetCurrentProcess(), 0)
	return
end
