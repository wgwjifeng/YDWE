
local w2l

local function search_staticfile(map, callback)
    for _, name in ipairs {'(listfile)', '(signature)', '(attributes)'} do
        callback(name)
    end
end

local function search_listfile(map, callback)
    local buf = map:get '(listfile)'
    if buf then
        for name in buf:gmatch '[^\r\n]+' do
            callback(name)
        end
    end
end

local function search_imp(map, callback)
    local buf = map:get 'war3map.imp'
    if buf then
        local _, count, index = ('ll'):unpack(buf)
        local name
        for i = 1, count do
            _, name, index = ('c1z'):unpack(buf, index)
            callback(name)
            if not map:has(name) then
                local name = 'war3mapimported\\' .. name
                callback(name)
            end
        end
    end
end

local searchers = {
    search_listfile,
    search_staticfile,
    search_imp,
}

local function search_mpq(map, progress)
    local total = map:number_of_files()
    local count = 0
    local clock = os.clock()
    local mark = {}
    for i, searcher in ipairs(searchers) do
        pcall(searcher, map, function (name)
            name = name:lower():gsub('/', '\\')
            if mark[name] then
                return
            end
            mark[name] = true
            if not map:has(name) then
                return
            end
            map:get(name)
            count = count + 1
            if os.clock() - clock > 0.1 then
                clock = os.clock()
                print(('正在读取文件... (%d/%d)'):format(count, total))
                progress(count / total)
            end
        end)
    end

    if count ~= total then
        print('-report|1严重错误', ('还有%d个文件没有读取'):format(total - count))
        print('-tip', '这些文件被丢弃了,请包含完整(listfile)')
        print('-report|1严重错误', ('读取(%d/%d)个文件'):format(count, total))
    end
end

local ignore = {}
for _, name in ipairs {'.git', '.svn', '.vscode', '.gitignore'} do
    ignore[name] = true
end

local function scan_dir(dir, callback)
    for path in dir:list_directory() do
        if not ignore[path:filename():string()] then
            if fs.is_directory(path) then
                scan_dir(path, callback)
            else
                callback(path)
            end
        end
    end
end

local function search_dir(map, progress)
    local total = map:number_of_files()
    local clock = os.clock()
    local count = 0
    local len = #map.path:string()
    for _, dir_name in ipairs {'map', 'resource', 'scripts', 'sound', 'trigger', 'plugin'} do
        scan_dir(map.path / dir_name, function(path)
            local name = path:string():sub(len+2):lower()
            map:get(name)
            count = count + 1
            if os.clock() - clock > 0.1 then
                clock = os.clock()
                print(('正在读取文件... (%d/%d)'):format(count, total))
                progress(count / total)
            end
        end)
    end
end

return function (map, progress)
    progress = progress or function () end
    if map:get_type() == 'mpq' then
        search_mpq(map, progress)
    else
        search_dir(map, progress)
    end
end
