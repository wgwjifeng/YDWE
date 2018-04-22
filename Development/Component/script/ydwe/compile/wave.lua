require "filesystem"
local process = require "process"

local root = fs.ydwe_devpath()

wave = {}
wave.path                = fs.ydwe_path() / "plugin" / "wave"
wave.exe_path            = wave.path / "Wave.exe"
wave.sys_include_path    = wave.path / "include"
wave.plugin_include_path = fs.ydwe_path() / "plugin"
wave.force_file_path     = wave.sys_include_path / "WaveForce.i"

local function pathstring(path)
	local str = path:string()
	if str:sub(-1) == '\\' then
		return '"' .. str .. ' "'
	else
		return '"' .. str .. '"'
	end
end

-- 预处理代码
-- op.input - 输入文件路径
-- op.option - 预处理选项，table，支持的值有
-- 	runtime_version - 表示魔兽版本
-- 	enable_jasshelper_debug - 布尔值，是否是调试模式
--	enable_yd_trigger - 布尔值，是否启用YD触发器
-- 返回：number, info, path - 子进程返回值；预处理输出信息；输出文件路径
function wave:do_compile(op)
	local cmd = ''
	cmd = cmd .. string.format('--output=%s ', pathstring(op.output))
	cmd = cmd .. string.format('--sysinclude=%s ', pathstring(self.sys_include_path))
	cmd = cmd .. string.format('--sysinclude=%s ', pathstring(self.plugin_include_path))
	cmd = cmd .. string.format('--include=%s ',    pathstring(op.map_path:parent_path()))
    for _, path in ipairs(require 'ui') do
        if fs.exists(path / 'jass') then
            cmd = cmd .. string.format('--include=%s ',    pathstring(path / 'jass'))
        end
    end
	cmd = cmd .. string.format('--define=WARCRAFT_VERSION=%d ', 100 * op.option.runtime_version.major + op.option.runtime_version.minor)
	cmd = cmd .. string.format('--define=YDWE_VERSION_STRING=\\"%s\\" ', tostring(ydwe_version))
	if op.option.enable_jasshelper_debug then
		cmd = cmd .. '--define=DEBUG=1 '
	end
	if tonumber(global_config["ScriptInjection"]["Option"]) == 0 then
		cmd = cmd .. "--define=SCRIPT_INJECTION=1 "
	end
	if not op.option.enable_yd_trigger then
		cmd = cmd .. '--define=DISABLE_YDTRIGGER=1 '
	end
	if fs.exists(self.force_file_path) then
		cmd = cmd .. string.format('--forceinclude=%s ', self.force_file_path:filename():string())
	end
	cmd = cmd .. "--extended --c99 --preserve=2 --line=0 "

	local command_line = string.format('%s %s %s', pathstring(self.exe_path), cmd, pathstring(op.input))
	-- 启动进程
	local p = process()
	p:hide_window()
	local stdout, stderr = p:std_output(), p:std_error()
	if not p:create(nil, command_line, nil) then
		log.error(string.format("Executed %s failed", command_line))
		return -1, nil, nil
	end
	log.trace(string.format("Executed %s.", command_line))
    local out = stdout:read 'a'
    local err = stderr:read 'a'
    local exit_code = p:wait()
    p:close()
    p = nil
    return exit_code, out, err
end

function wave:compile(op)
	log.trace("Wave compilation start.")		
	
	local f = io.open(op.input, "a+b")
	if f then
		f:write("/**/\r\n")
		f:close()
	end
	local exit_code, out, err = self:do_compile(op)
	
	-- 退出码0代表成功
	if exit_code ~= 0 then
		if out and err then
			gui.error_message(nil, LNG.PREPEOCESSOR_FAILED, out, err)
		else
			gui.error_message(nil, LNG.PREPEOCESSOR_LAUNCH_FAILED)
		end
		return false
	end

	return true
end
