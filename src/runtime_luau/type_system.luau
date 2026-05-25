--!strict
-- MERO Runtime - Type System
-- Runtime type checking and validation.
-- Developer: MERO:TG@QP4RM

local TypeSystem = {}

export type TypeInfo = {
    name: string,
    base: string?,
    check: (any) -> boolean,
}

local _registered_types: {[string]: TypeInfo} = {}

function TypeSystem.register(name: string, check: (any) -> boolean, base: string?)
    _registered_types[name] = {
        name = name,
        base = base,
        check = check,
    }
end

function TypeSystem.check_type(value: any, expected: string): boolean
    -- Built-in type checks
    if expected == "int" then
        return type(value) == "number" and math.floor(value) == value
    elseif expected == "float" then
        return type(value) == "number"
    elseif expected == "str" then
        return type(value) == "string"
    elseif expected == "bool" then
        return type(value) == "boolean"
    elseif expected == "list" or expected == "table" then
        return type(value) == "table"
    elseif expected == "None" or expected == "nil" then
        return value == nil
    elseif expected == "any" then
        return true
    end

    -- Custom registered types
    local type_info = _registered_types[expected]
    if type_info then
        return type_info.check(value)
    end

    -- Class instance check
    if type(value) == "table" then
        local mt = getmetatable(value)
        if mt and mt.__type then
            return mt.__type == expected
        end
    end

    return typeof(value) == expected
end

function TypeSystem.assert_type(value: any, expected: string, param_name: string?)
    if not TypeSystem.check_type(value, expected) then
        local name = param_name or "value"
        local actual = TypeSystem.get_type_name(value)
        error(string.format("TypeError: %s expected %s, got %s", name, expected, actual))
    end
end

function TypeSystem.get_type_name(value: any): string
    local t = type(value)
    if t == "nil" then return "None" end
    if t == "boolean" then return "bool" end
    if t == "number" then
        if math.floor(value) == value then return "int" end
        return "float"
    end
    if t == "string" then return "str" end
    if t == "table" then
        local mt = getmetatable(value)
        if mt and mt.__type then return mt.__type end
        -- Check if array-like
        if #value > 0 or next(value) == nil then return "list" end
        return "dict"
    end
    if t == "function" then return "function" end
    return typeof(value)
end

-- Type annotation runtime helpers
function TypeSystem.Optional(value: any, type_name: string): boolean
    return value == nil or TypeSystem.check_type(value, type_name)
end

function TypeSystem.Union(value: any, types: {string}): boolean
    for _, t in types do
        if TypeSystem.check_type(value, t) then return true end
    end
    return false
end

-- Register standard Python types
TypeSystem.register("int", function(v) return type(v) == "number" and math.floor(v) == v end)
TypeSystem.register("float", function(v) return type(v) == "number" end)
TypeSystem.register("str", function(v) return type(v) == "string" end)
TypeSystem.register("bool", function(v) return type(v) == "boolean" end)
TypeSystem.register("list", function(v) return type(v) == "table" end)
TypeSystem.register("dict", function(v) return type(v) == "table" end)
TypeSystem.register("NoneType", function(v) return v == nil end)
TypeSystem.register("callable", function(v) return type(v) == "function" end)

return TypeSystem
