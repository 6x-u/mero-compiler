--!strict
-- MERO Runtime - Core Runtime Module
-- Provides Python-compatible runtime support for compiled Luau code.
-- Developer: MERO:TG@QP4RM

local Runtime = {}

-- Utility: truthiness (Python semantics)
function Runtime.truthy(value: any): boolean
    if value == nil or value == false then return false end
    if type(value) == "number" then return value ~= 0 end
    if type(value) == "string" then return #value > 0 end
    if type(value) == "table" then return next(value) ~= nil end
    return true
end

-- print() with Python semantics
function Runtime.print(...: any)
    local args = {...}
    local parts = {}
    for i = 1, select("#", ...) do
        table.insert(parts, tostring(args[i]))
    end
    print(table.concat(parts, " "))
end

-- len()
function Runtime.len(obj: any): number
    if type(obj) == "string" then return #obj end
    if type(obj) == "table" then
        local count = 0
        for _ in obj do count += 1 end
        return count
    end
    error("object has no len()")
end

-- range()
function Runtime.range(start_or_stop: number, stop: number?, step: number?): () -> (number?, number?)
    local s, e, st
    if stop == nil then
        s, e, st = 0, start_or_stop, 1
    else
        s, e, st = start_or_stop, stop, step or 1
    end
    local i = s - st
    return function()
        i += st
        if st > 0 and i < e then return i, i
        elseif st < 0 and i > e then return i, i
        end
        return nil, nil
    end
end

-- Type conversions
function Runtime.to_int(value: any): number
    if type(value) == "number" then return math.floor(value) end
    if type(value) == "string" then
        local n = tonumber(value)
        if n then return math.floor(n) end
    end
    if type(value) == "boolean" then return if value then 1 else 0 end
    error("cannot convert to int")
end

function Runtime.to_float(value: any): number
    if type(value) == "number" then return value end
    if type(value) == "string" then
        local n = tonumber(value)
        if n then return n end
    end
    error("cannot convert to float")
end

function Runtime.to_str(value: any): string
    if value == nil then return "None" end
    if type(value) == "boolean" then return if value then "True" else "False" end
    return tostring(value)
end

function Runtime.to_bool(value: any): boolean
    return Runtime.truthy(value)
end

-- isinstance
function Runtime.isinstance(obj: any, cls: any): boolean
    if type(obj) == "table" and type(cls) == "table" then
        local mt = getmetatable(obj)
        while mt do
            if mt.__index == cls then return true end
            mt = getmetatable(mt.__index)
        end
    end
    if type(cls) == "string" then
        return typeof(obj) == cls
    end
    return false
end

-- type_of
function Runtime.type_of(obj: any): string
    local t = type(obj)
    if t == "table" then
        local mt = getmetatable(obj)
        if mt and mt.__type then return mt.__type end
    end
    return t
end

-- enumerate
function Runtime.enumerate(iterable: any, start: number?): () -> (number?, any?)
    local i = (start or 0) - 1
    local iter, state, initial = pairs(iterable)
    local key = initial
    return function()
        local val
        key, val = iter(state, key)
        if key == nil then return nil, nil end
        i += 1
        return i, val
    end
end

-- zip
function Runtime.zip(a: {any}, b: {any}): () -> (any?, any?)
    local i = 0
    local len = math.min(#a, #b)
    return function()
        i += 1
        if i <= len then return a[i], b[i] end
        return nil, nil
    end
end

-- map
function Runtime.map(func: (any) -> any, iterable: {any}): {any}
    local result = {}
    for _, v in iterable do
        table.insert(result, func(v))
    end
    return result
end

-- filter
function Runtime.filter(func: ((any) -> boolean)?, iterable: {any}): {any}
    local result = {}
    for _, v in iterable do
        if func then
            if func(v) then table.insert(result, v) end
        else
            if Runtime.truthy(v) then table.insert(result, v) end
        end
    end
    return result
end

-- sorted
function Runtime.sorted(iterable: {any}, reverse: boolean?): {any}
    local copy = table.clone(iterable)
    table.sort(copy)
    if reverse then
        local n = #copy
        for i = 1, math.floor(n / 2) do
            copy[i], copy[n - i + 1] = copy[n - i + 1], copy[i]
        end
    end
    return copy
end

-- reversed
function Runtime.reversed(iterable: {any}): {any}
    local result = {}
    for i = #iterable, 1, -1 do
        table.insert(result, iterable[i])
    end
    return result
end

-- sum
function Runtime.sum(iterable: {number}, start: number?): number
    local total = start or 0
    for _, v in iterable do
        total += v
    end
    return total
end

-- any / all
function Runtime.any(iterable: {any}): boolean
    for _, v in iterable do
        if Runtime.truthy(v) then return true end
    end
    return false
end

function Runtime.all(iterable: {any}): boolean
    for _, v in iterable do
        if not Runtime.truthy(v) then return false end
    end
    return true
end

-- hasattr / getattr / setattr
function Runtime.hasattr(obj: any, name: string): boolean
    if type(obj) ~= "table" then return false end
    return obj[name] ~= nil
end

function Runtime.getattr(obj: any, name: string, default: any?): any
    if type(obj) == "table" then
        local val = obj[name]
        if val ~= nil then return val end
    end
    if default ~= nil then return default end
    error("AttributeError: object has no attribute '" .. name .. "'")
end

function Runtime.setattr(obj: any, name: string, value: any)
    if type(obj) ~= "table" then
        error("cannot set attribute on non-table")
    end
    obj[name] = value
end

-- slice
function Runtime.slice(obj: any, start: number?, stop: number?, step: number?): any
    if type(obj) == "string" then
        local s = start or 1
        local e = stop or #obj
        return string.sub(obj, s, e)
    end
    if type(obj) == "table" then
        local result = {}
        local s = start or 1
        local e = stop or #obj
        local st = step or 1
        for i = s, e, st do
            table.insert(result, obj[i])
        end
        return result
    end
    return nil
end

-- Conversion helpers
function Runtime.to_list(obj: any): {any}
    if type(obj) == "table" then return table.clone(obj) end
    if type(obj) == "string" then
        local result = {}
        for i = 1, #obj do
            table.insert(result, string.sub(obj, i, i))
        end
        return result
    end
    return {}
end

function Runtime.to_dict(obj: any): {[any]: any}
    if type(obj) == "table" then return table.clone(obj) end
    return {}
end

function Runtime.to_set(obj: any): {[any]: boolean}
    local result = {}
    if type(obj) == "table" then
        for _, v in obj do result[v] = true end
    end
    return result
end

function Runtime.to_tuple(obj: any): {any}
    return Runtime.to_list(obj)
end

-- String helpers
function Runtime.hex(n: number): string
    return string.format("0x%x", n)
end

function Runtime.oct(n: number): string
    local result = ""
    local num = math.abs(math.floor(n))
    if num == 0 then return "0o0" end
    while num > 0 do
        result = tostring(num % 8) .. result
        num = math.floor(num / 8)
    end
    return (if n < 0 then "-0o" else "0o") .. result
end

function Runtime.bin(n: number): string
    local result = ""
    local num = math.abs(math.floor(n))
    if num == 0 then return "0b0" end
    while num > 0 do
        result = tostring(num % 2) .. result
        num = math.floor(num / 2)
    end
    return (if n < 0 then "-0b" else "0b") .. result
end

function Runtime.id(obj: any): number
    return 0 -- Roblox doesn't expose memory addresses
end

function Runtime.hash(obj: any): number
    if type(obj) == "number" then return obj end
    if type(obj) == "string" then
        local h = 0
        for i = 1, #obj do
            h = h * 31 + string.byte(obj, i)
        end
        return h
    end
    return 0
end

function Runtime.repr(obj: any): string
    if type(obj) == "string" then return "'" .. obj .. "'" end
    return tostring(obj)
end

function Runtime.format(value: any, spec: string?): string
    return tostring(value)
end

-- input (limited in Roblox)
function Runtime.input(prompt: string?): string
    if prompt then print(prompt) end
    return "" -- No stdin in Roblox
end

-- Iterator protocol
function Runtime.iter(obj: any): () -> any?
    if type(obj) == "table" then
        local i = 0
        return function()
            i += 1
            return obj[i]
        end
    end
    error("object is not iterable")
end

function Runtime.next(iterator: () -> any?, default: any?): any
    local val = iterator()
    if val == nil then
        if default ~= nil then return default end
        error("StopIteration")
    end
    return val
end

-- super
function Runtime.super(cls: any, self_ref: any): any
    local mt = getmetatable(cls)
    if mt and mt.__index then
        return setmetatable({}, {__index = mt.__index})
    end
    return {}
end

return Runtime
