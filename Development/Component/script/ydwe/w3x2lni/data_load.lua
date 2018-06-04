require 'filesystem'
local root = fs.ydwe_devpath()
local w2l_path = root / 'plugin' / 'w3x2lni'

return function (w2l, filename)
    if filename:sub(1, 3) == 'ui/' then
        return io.load(root / filename)
    end
    if filename:sub(1, 5) == 'data/' then
        return io.load(w2l_path / 'data' / w2l.setting.data / filename:sub(6))
    end
    return io.load(w2l_path / filename)
end
