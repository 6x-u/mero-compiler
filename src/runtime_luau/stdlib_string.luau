--!strict
-- MERO Runtime - String Standard Library
-- Python string method implementations for Luau.
-- Developer: MERO:TG@QP4RM

local StringLib = {}

function StringLib.strip(s: string, chars: string?): string
    if chars then
        local pattern = "[" .. string.gsub(chars, "([%^%$%(%)%%%.%[%]%*%+%-%?])", "%%%1") .. "]"
        s = string.gsub(s, "^" .. pattern .. "+", "")
        s = string.gsub(s, pattern .. "+$", "")
        return s
    end
    return string.gsub(string.gsub(s, "^%s+", ""), "%s+$", "")
end

function StringLib.lstrip(s: string, chars: string?): string
    if chars then
        local pattern = "[" .. string.gsub(chars, "([%^%$%(%)%%%.%[%]%*%+%-%?])", "%%%1") .. "]"
        return string.gsub(s, "^" .. pattern .. "+", "")
    end
    return string.gsub(s, "^%s+", "")
end

function StringLib.rstrip(s: string, chars: string?): string
    if chars then
        local pattern = "[" .. string.gsub(chars, "([%^%$%(%)%%%.%[%]%*%+%-%?])", "%%%1") .. "]"
        return string.gsub(s, pattern .. "+$", "")
    end
    return string.gsub(s, "%s+$", "")
end

function StringLib.startswith(s: string, prefix: string): boolean
    return string.sub(s, 1, #prefix) == prefix
end

function StringLib.endswith(s: string, suffix: string): boolean
    return string.sub(s, -#suffix) == suffix
end

function StringLib.count(s: string, sub: string): number
    local count = 0
    local start = 1
    while true do
        local pos = string.find(s, sub, start, true)
        if not pos then break end
        count += 1
        start = pos + 1
    end
    return count
end

function StringLib.capitalize(s: string): string
    if #s == 0 then return s end
    return string.upper(string.sub(s, 1, 1)) .. string.lower(string.sub(s, 2))
end

function StringLib.title(s: string): string
    local result = ""
    local prev_space = true
    for i = 1, #s do
        local c = string.sub(s, i, i)
        if c == " " or c == "\t" or c == "\n" then
            prev_space = true
            result ..= c
        elseif prev_space then
            result ..= string.upper(c)
            prev_space = false
        else
            result ..= string.lower(c)
        end
    end
    return result
end

function StringLib.isdigit(s: string): boolean
    if #s == 0 then return false end
    return string.match(s, "^%d+$") ~= nil
end

function StringLib.isalpha(s: string): boolean
    if #s == 0 then return false end
    return string.match(s, "^%a+$") ~= nil
end

function StringLib.isalnum(s: string): boolean
    if #s == 0 then return false end
    return string.match(s, "^%w+$") ~= nil
end

function StringLib.isspace(s: string): boolean
    if #s == 0 then return false end
    return string.match(s, "^%s+$") ~= nil
end

function StringLib.center(s: string, width: number, fillchar: string?): string
    local fill = fillchar or " "
    local pad = width - #s
    if pad <= 0 then return s end
    local left = math.floor(pad / 2)
    local right = pad - left
    return string.rep(fill, left) .. s .. string.rep(fill, right)
end

function StringLib.ljust(s: string, width: number, fillchar: string?): string
    local fill = fillchar or " "
    local pad = width - #s
    if pad <= 0 then return s end
    return s .. string.rep(fill, pad)
end

function StringLib.rjust(s: string, width: number, fillchar: string?): string
    local fill = fillchar or " "
    local pad = width - #s
    if pad <= 0 then return s end
    return string.rep(fill, pad) .. s
end

function StringLib.zfill(s: string, width: number): string
    local pad = width - #s
    if pad <= 0 then return s end
    if string.sub(s, 1, 1) == "-" then
        return "-" .. string.rep("0", pad) .. string.sub(s, 2)
    end
    return string.rep("0", pad) .. s
end

function StringLib.replace(s: string, old: string, new: string, count: number?): string
    if count then
        local result = s
        for _ = 1, count do
            local pos = string.find(result, old, 1, true)
            if not pos then break end
            result = string.sub(result, 1, pos - 1) .. new .. string.sub(result, pos + #old)
        end
        return result
    end
    return string.gsub(s, string.gsub(old, "([%^%$%(%)%%%.%[%]%*%+%-%?])", "%%%1"), new)
end

function StringLib.join(sep: string, items: {string}): string
    return table.concat(items, sep)
end

return StringLib
