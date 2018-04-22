local w2l
local wtg
local wts
local state1

local pack_eca

local type_map = {
    ['列表'] = -1,
    ['事件'] = 0,
    ['条件'] = 1,
    ['动作'] = 2,
    ['函数'] = 3,
}

local type_key = {
    ['事件'] = 'event',
    ['条件'] = 'condition',
    ['动作'] = 'action',
    ['函数'] = 'call',
}

local arg_type_map = {
    ['禁用'] = -1,
    ['预设'] = 0,
    ['变量'] = 1,
    ['函数'] = 2,
    ['常量'] = 3,
}

local function pack(fmt, ...)
    hex[#hex+1] = fmt:pack(...)
end

local function pack_head()
    pack('c4l', 'WTG!', 7)
end

local function pack_category()
    pack('l', #wtg.categories)
    for _, cate in ipairs(wtg.categories) do
        pack('lzl', cate.id, cate.name, cate.comment)
    end
end

local function pack_var(var)
    local name = var[1]
    local type = var[2]
    local unknow = 1
    local array = 0
    local size = 1
    local default = 0
    local value = ''
    for i = 3, #var do
        local k, v = var[i][1], var[i][2]
        if k == '数组' then
            array = 1
            size = v
        elseif k == '默认' then
            default = 1
            value = v
        end
    end
    pack('zzllllz', name, type, unknow, array, size, default, value)
end

local function pack_vars()
    pack('ll', 2, #wtg.vars-2)
    for i = 3, #wtg.vars do
        pack_var(wtg.vars[i])
    end
end

local function pack_arg(arg)
    local type = arg[1]
    local value = arg[2]
    local array = false
    if type_map[type] then
        type = '函数'
        if type_map[type] ~= '函数' then
            value = ''
        end
    elseif type == '数组' then
        array = true
        type = '变量'
    end
    if type == '常量' then
        value = w2l:load_wts(wts, value, 299, '触发器里的文本长度超过299字符', function(str)
            return str:gsub('\\', '\\\\'):gsub('"', '\\"')
        end)
    end
    pack('lz', arg_type_map[type], value)
    if type == '函数' then
        pack('l', 1)
        pack_eca(arg)
    else
        pack('l', 0)
    end
    if array then
        pack('l', 1)
        pack_arg(arg[3])
    else
        pack('l', 0)
    end
end

local arg_count = {}
local function get_ui_arg_count(ui)
    local name = ui.name
    if not arg_count[name] then
        local count = 0
        if ui.args then
            for _, arg in ipairs(ui.args) do
                if arg.type ~= 'nothing' then
                    count = count + 1
                end
            end
        end
        arg_count[name] = count
    end
    return arg_count[name]
end

local function pack_args(ui, eca)
    local eca_arg_count = 0
    for i = 3, #eca do
        if eca[i][2] then
            eca_arg_count = eca_arg_count + 1
            pack_arg(eca[i])
        end
    end

    if ui then
        if eca_arg_count ~= get_ui_arg_count(ui) then
            error(('[%s]的参数数量不正确：[%d] - [%d]'):format(ui.name, get_ui_arg_count(ui), eca_arg_count))
        end
    end
end

local function pack_list(lists, root)
    local child_count = 0
    for i = 3, #lists do
        if not lists[i][2] and type_map[lists[i][1]] and #lists[i] > 2 then
            child_count = child_count + #lists[i] - 2
        end
    end
    pack('l', child_count)
    local child_id = -1
    for i = 3, #lists do
        if not lists[i][2] and type_map[lists[i][1]] then
            child_id = child_id + 1
            local list = lists[i]
            local type = list[1]
            for i = 3, #list do
                if root then
                    pack_eca(list[i], nil, type)
                else
                    pack_eca(list[i], child_id, type)
                end
            end
        end
    end
end

function pack_eca(eca, child_id, eca_type)
    local name
    local type = eca_type or '函数'
    local enable = 1
    if eca[2] then
        name = eca[2]
        if eca[1] == '禁用' then
            enable = 0
        elseif type_map[eca[1]] then
            type = eca[1]
        else
            w2l.message('未知的动作属性', eca[1], #hex)
        end
    else
        name = eca[1]
    end
    local ui
    if state then
        ui = state.ui[type_key[type]][name]
        if not ui then
            error(('UI不存在：[%s]'):format(name))
        end
    end
    if child_id then
        pack('llzl', type_map[type], child_id, name, enable)
    else
        pack('lzl', type_map[type], name, enable)
    end
    pack_args(ui, eca)
    pack_list(eca)
end

local function pack_trigger(trg)
    pack('zzllllll',
        trg.name,
        trg.des,
        trg.type,
        trg.enable,
        trg.wct,
        trg.close,
        trg.run,
        trg.category
    )
    pack_list(trg.trg, true)
end

local function pack_triggers()
    pack('l', #wtg.triggers)
    for i = 1, #wtg.triggers do
        pack_trigger(wtg.triggers[i])
    end
end

return function (w2l_, wtg_, wts_)
    w2l = w2l_
    wtg = wtg_
    wts = wts_
    state, err = w2l:trigger_data()
    hex = {}

    pack_head()
    pack_category()
    pack_vars()
    pack_triggers()

    return table.concat(hex)
end
