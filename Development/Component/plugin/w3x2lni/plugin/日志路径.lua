local mt = {}

mt.info = {
    name = '日志路径',
    version = 1.0,
    author = '最萌小汐',
}

function mt:on_convert(w2l)
    w2l.log_path = fs.ydwe(false) / 'logs' / 'w3x2lni'
end

function mt:on_pack(w2l)
    w2l.log_path = fs.ydwe(false) / 'logs' / 'w3x2lni'
end

return mt
