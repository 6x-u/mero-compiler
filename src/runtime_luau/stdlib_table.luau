--!strict
-- MERO Runtime - Table/List/Dict Standard Library
-- Python list and dict method implementations.
-- Developer: MERO:TG@QP4RM

local TableLib = {}

-- List operations
function TableLib.list_remove(list: {any}, value: any)
    for i, v in list do
        if v == value then
            table.remove(list, i)
            return
        end
    end
    error("ValueError: list.remove(x): x not in list")
end

function TableLib.list_reverse(list: {any})
    local n = #list
    for i = 1, math.floor(n / 2) do
        list[i], list[n - i + 1] = list[n - i + 1], list[i]
    end
end

function TableLib.list_extend(list: {any}, other: {any})
    for _, v in other do
        table.insert(list, v)
    end
end

function TableLib.list_index(list: {any}, value: any, start: number?, stop: number?): number
    local s = start or 1
    local e = stop or #list
    for i = s, e do
        if list[i] == value then return i - 1 end  -- 0-based for Python compat
    end
    error("ValueError: value not in list")
end

function TableLib.list_count(list: {any}, value: any): number
    local count = 0
    for _, v in list do
        if v == value then count += 1 end
    end
    return count
end

-- Dict operations
function TableLib.dict_keys(dict: {[any]: any}): {any}
    local keys = {}
    for k in dict do
        table.insert(keys, k)
    end
    return keys
end

function TableLib.dict_values(dict: {[any]: any}): {any}
    local values = {}
    for _, v in dict do
        table.insert(values, v)
    end
    return values
end

function TableLib.dict_items(dict: {[any]: any}): {{any}}
    local items = {}
    for k, v in dict do
        table.insert(items, {k, v})
    end
    return items
end

function TableLib.dict_get(dict: {[any]: any}, key: any, default: any?): any
    local val = dict[key]
    if val ~= nil then return val end
    return default
end

function TableLib.dict_update(dict: {[any]: any}, other: {[any]: any})
    for k, v in other do
        dict[k] = v
    end
end

function TableLib.dict_pop(dict: {[any]: any}, key: any, default: any?): any
    local val = dict[key]
    if val ~= nil then
        dict[key] = nil
        return val
    end
    if default ~= nil then return default end
    error("KeyError: " .. tostring(key))
end

function TableLib.dict_setdefault(dict: {[any]: any}, key: any, default: any?): any
    local val = dict[key]
    if val ~= nil then return val end
    local d = default or nil
    dict[key] = d
    return d
end

-- Set operations
function TableLib.set_add(s: {[any]: boolean}, value: any)
    s[value] = true
end

function TableLib.set_remove(s: {[any]: boolean}, value: any)
    if not s[value] then error("KeyError: " .. tostring(value)) end
    s[value] = nil
end

function TableLib.set_discard(s: {[any]: boolean}, value: any)
    s[value] = nil
end

function TableLib.set_union(a: {[any]: boolean}, b: {[any]: boolean}): {[any]: boolean}
    local result = table.clone(a)
    for k in b do result[k] = true end
    return result
end

function TableLib.set_intersection(a: {[any]: boolean}, b: {[any]: boolean}): {[any]: boolean}
    local result = {}
    for k in a do
        if b[k] then result[k] = true end
    end
    return result
end

function TableLib.set_difference(a: {[any]: boolean}, b: {[any]: boolean}): {[any]: boolean}
    local result = {}
    for k in a do
        if not b[k] then result[k] = true end
    end
    return result
end

function TableLib.contains(collection: any, value: any): boolean
    if type(collection) == "table" then
        for _, v in collection do
            if v == value then return true end
        end
        -- Check as dict key
        if collection[value] ~= nil then return true end
    elseif type(collection) == "string" and type(value) == "string" then
        return string.find(collection, value, 1, true) ~= nil
    end
    return false
end

return TableLib
