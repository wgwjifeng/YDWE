require "filesystem"
local stormlib = require 'ffi.stormlib'

local jasshelper = {}

jasshelper.path     = fs.ydwe_path() / "plugin" / "jasshelper"
jasshelper.exe_path = jasshelper.path / "jasshelper.exe"

local config = [[
[jasscompiler]
%q
"%s$COMMONJ $BLIZZARDJ $WAR3MAPJ"
]]


-- 根据版本获取YDWE自带的Jass库函数（bj和cj）路径
-- version - 魔兽版本，数
-- 返回：cj路径，bj路径，都是fs.path
function jasshelper:default_jass_libs(version)
	if version == 24 then
		return (fs.ydwe_path() / "share" / "jass" / "ht" / "common.j"),
			(fs.ydwe_path() / "share" / "jass" / "ht" / "blizzard.j")
	else
		return (fs.ydwe_path() / "share" / "jass" / "rb" / "common.j"),
			(fs.ydwe_path() / "share" / "jass" / "rb" / "blizzard.j")
	end
end

-- 准备魔兽争霸3的Jass库函数（common.j和blizzard.j）供语法检查用
-- 如果地图中有，则优先使用地图的，否则使用自带的
-- map_path - 地图路径，fs.path对象
-- 返回2个值：cj路径，bj路径，都是fs.path。
function jasshelper:prepare_jass_libs(map_path, version)
	local common_j_path = self.path / "common.j"
	local blizzard_j_path = self.path / "blizzard.j"
	local map_has_cj = false
	local map_has_bj = false
	
	-- 从地图中解压缩两个重要文件到jasshelper目录（供语法检查用）
	local mpq = stormlib.open(map_path, true)
	if mpq then
		-- 如果地图中导入了，优先使用地图的
		if mpq:has_file("common.j") then
			mpq:extract("common.j", common_j_path)
			map_has_cj = true
		elseif mpq:has_file("scripts\\common.j") then
			mpq:extract("scripts\\common.j", common_j_path)
			map_has_cj = true
		end

		if mpq:has_file("blizzard.j") then
			mpq:extract("blizzard.j", blizzard_j_path)
			map_has_bj = true
		elseif mpq:has_file("scripts\\blizzard.j") then
			mpq:extract("scripts\\blizzard.j", blizzard_j_path)
			map_has_bj = true
		end
		mpq:close()
	else
		log.warn("Cannot open map archive, using default bj and cj instead.")
	end

	local default_common_j_path, default_blizzard_j_path = self:default_jass_libs(version)
	if not map_has_cj then
		common_j_path = default_common_j_path
	end
	if not map_has_bj then
		blizzard_j_path = default_blizzard_j_path
	end
	
	return common_j_path, blizzard_j_path
end


-- 使用JassHelper编译地图
-- map_path - 地图路径，fs.path对象
-- common_j_path - common.j路径，fs.path对象
-- blizzard_j_path - blizzard.j路径，fs.path对象
-- option - 编译选项, table，目前支持参数：
-- 	enable_jasshelper - 启用JassHelper，true/false
--	enable_jasshelper_debug - 启用JassHelper的Debug，true/false
--	enable_jasshelper_optimization - 启用优化，true/false
-- 返回：true编译成功，false编译失败
function jasshelper:do_compile(map_path, common_j_path, blizzard_j_path, option)
	local parameter = ""
	
	-- 需要做vJass编译？
	if option.enable_jasshelper then
		-- debug选项（--debug）
		if option.enable_jasshelper_debug then
			parameter = parameter .. " --debug"
		end
		-- （关闭）优化选项（--nooptimize）
		if not option.enable_jasshelper_optimization then
			parameter = parameter .. " --nooptimize"
		end
	else
		-- 不编译vJass选项（--nopreprocessor）
		parameter = parameter .. " --nopreprocessor"
	end

	if option.enable_jasshelper_scriptonly then
		local compiler = require 'compile.compiler'
		return compiler:update_script(map_path, "5_vjass.j",
			function (map_handle, in_script_path)
				local out_script_path = fs.ydwe_path() / "logs" / "6_vjass.j"
				
				-- 生成命令行
				local command_line = string.format('"%s"%s --scriptonly "%s" "%s" "%s" "%s"',
					self.exe_path:string(),
					parameter,
					common_j_path:string(),
					blizzard_j_path:string(),
					in_script_path:string(),
					out_script_path:string()
				)
				-- 执行并获取结果
				if not sys.spawn(command_line, fs.ydwe_path(), true) then
					return nil
				end

				return out_script_path
			end
		)
	else
		-- 生成命令行
		local command_line = string.format('"%s"%s "%s" "%s" "%s"',
			self.exe_path:string(),
			parameter,
			common_j_path:string(),
			blizzard_j_path:string(),
			map_path:string()
		)

		-- 执行并获取结果
		return sys.spawn(command_line, fs.ydwe_path(), true)
	end
	
end

function jasshelper:compile(map_path, option)	
	log.trace("JassHelper compilation start.")	
	local common_j_path, blizzard_j_path = self:prepare_jass_libs(map_path, option.runtime_version)
	if option.pjass == '1' then
		io.save(fs.ydwe_path() / 'jasshelper.conf', config:format('../pjass/pjass-classic.exe', ''))
	else
		if option.runtime_version == 24 then
			io.save(fs.ydwe_path() / 'jasshelper.conf', config:format('../pjass/pjass-latest.exe', ''))
		else
			io.save(fs.ydwe_path() / 'jasshelper.conf', config:format('../pjass/pjass-latest.exe', '+rb '))
		end
	end
	local res = self:do_compile(map_path, common_j_path, blizzard_j_path, option)
	fs.remove(fs.ydwe_path() / 'jasshelper.conf')
	return res
end

return jasshelper
