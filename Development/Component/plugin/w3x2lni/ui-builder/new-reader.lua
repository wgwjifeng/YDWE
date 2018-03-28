local lni = require 'lni'

local mt = {}
mt.__index = mt

local function switch(value)
    return function (mapping)
        if mapping[value] then
            mapping[value]()
        end
    end
end

function mt:read_ui(type, buf)
    if not buf then
        return
    end
    local lastkey
    local last
    local function savelast()
        if not last then
            return
        end
        last.name = lastkey
        if type == 'call' then
            if not last.use_in_event then
                last.use_in_event = '1'
            end
        end
        if not self.categories[type][last.category] then
            self.categories[type][last.category] = {}
            table.insert(self.categories[type], last.category)
        end
        table.insert(self.categories[type][last.category], last)
    end
    local t = setmetatable({}, {
        __newindex = function(_, key, value)
            self.ui[type][key] = value
            savelast()
            lastkey = key
            last = value
        end,
    })
    lni(buf, type, {t})
    savelast()
end

function mt:reset()
    self.ui = {
        define = {},
        event = {},
        condition = {},
        action = {},
        call = {},
    }
    self.categories = {
        event = {},
        condition = {},
        action = {},
        call = {},
    }
end

function mt:parse_define(section, key, value)
    if not self.ui.define[section] then
        self.ui.define[section] = {}
    end
    table.insert(self.ui.define[section], {key, value})
end

function mt:parse(section, key, value)
    switch(section) {
        TriggerCategories = function ()
            self:parse_define(section, key, value)
        end,
        TriggerTypes = function ()
            self:parse_define(section, key, value)
        end,
        TriggerTypeDefaults = function ()
            self:parse_define(section, key, value)
        end,
        TriggerParams = function ()
            self:parse_define(section, key, value)
        end,
        DefaultTriggerCategories = function ()
            self:parse_define(section, key, value)
        end,
        DefaultTriggers = function ()
            self:parse_define(section, key, value)
        end,
        AIFunctionStrings = function ()
            self:parse_define(section, key, value)
        end,
    }
end

function mt:read_define(buf)
    if not buf then
        return
    end
	local section = nil
	for line in buf:gmatch '[^\r\n]+' do
		if line:sub(1,1) == "[" then
			section = line:sub(2, #line - 1)
		elseif line:sub(1,2) == "//" then
		elseif line ~= "" then
			local first = line:find("=")
			if first then
				local key = line:sub(1, first - 1)
				local value = line:sub(first + 1) or ""
				self:parse(section, key, value)
			end
		end
	end
end

function mt:read(loader)
    self:reset()
    self:read_define(loader [[define.txt]])
    self:read_ui('event', loader [[event.txt]])
    self:read_ui('condition', loader [[condition.txt]])
    self:read_ui('action', loader [[action.txt]])
    self:read_ui('call', loader [[call.txt]])
end

return function(loader)
    local obj = setmetatable({}, mt)
    obj:read(loader)
    return obj
end
