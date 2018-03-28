local w2l
local wtg
local wct
local loader

local function load_custom()
    wct.custom = {
        comment = loader '代码.txt' or '',
        code    = loader '代码.j' or '',
    }
end

local function load_vars()
    wtg.vars = w2l:parse_lml(loader '变量.lml' or '')
end

local function load_trigger(trg, id, filename)
    local trigger = {
        category = id,
        type = 0,
        enable = 1,
        close = 0,
        run = 0,
        wct = 0,
    }
    local name = trg[1]
    trigger.name = name
    for i = 3, #trg do
        local line = trg[i]
        local k, v = line[1], line[2]
        if k == '名称' then
            trigger.name = v
        elseif k == '注释' then
            trigger.type = 1
        elseif k == '禁用' then
            trigger.enable = 0
        elseif k == '关闭' then
            trigger.close = 1
        elseif k == '运行' then
            trigger.run = 1
        end
    end

    local path = filename .. '\\' .. name

    trigger.trg = w2l:parse_lml(loader(path..'.lml') or '')
    trigger.des = loader(path..'.txt') or ''
    
    local buf = loader(path..'.j')
    if buf then
        trigger.wct = 1
        wct.triggers[#wct.triggers+1] = buf
    else
        wct.triggers[#wct.triggers+1] = ''
    end

    wtg.triggers[#wtg.triggers+1] = trigger
end

local function load_category(dir)
    local category = {
        comment = 0,
    }
    local filename = dir[1]
    local id = tonumber(dir[2])
    category.name = filename
    category.id = id

    for i = 3, #dir do
        local line = dir[i]
        local k, v = line[1], line[2]
        if v then
            if k == '名称' then
                category.name = v
            elseif k == '注释' and v == '1' then
                category.comment = 1
            end
        else
            load_trigger(line, id, filename)
        end
    end
    
    wtg.categories[#wtg.categories+1] = category
end

local function load_triggers()
    wtg.categories = {}
    wtg.triggers = {}
    wct.triggers = {}
    local buf = loader '目录.lml'
    if not buf then
        return
    end
    local list_file = w2l:parse_lml(buf)
    for i = 3, #list_file do
        local dir = list_file[i]
        load_category(dir)
    end
end

return function (w2l_, loader_)
    w2l = w2l_
    wtg = {}
    wct = {}
    loader = loader_

    load_custom()
    load_vars()
    load_triggers()

    return wtg, wct
end
