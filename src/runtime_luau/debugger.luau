--!strict
-- MERO Runtime - Debug Tools
-- Runtime inspection and debugging utilities.
-- Developer: MERO:TG@QP4RM

local Debugger = {}

local _breakpoints: {[string]: {[number]: boolean}} = {}
local _watches: {[string]: () -> any} = {}
local _log_enabled = false

function Debugger.enable_logging()
    _log_enabled = true
end

function Debugger.disable_logging()
    _log_enabled = false
end

function Debugger.log(message: string, level: string?)
    if not _log_enabled then return end
    local prefix = level or "DEBUG"
    local trace = debug.traceback(nil, 2)
    local first_line = string.match(trace, "[^\n]+\n([^\n]+)") or ""
    warn(string.format("[%s] %s | %s", prefix, message, first_line))
end

function Debugger.inspect(value: any, depth: number?): string
    return Debugger._serialize(value, depth or 3, 0, {})
end

function Debugger._serialize(value: any, max_depth: number, current: number, seen: {[any]: boolean}): string
    if current >= max_depth then return "..." end

    local t = type(value)
    if t == "nil" then return "None"
    elseif t == "boolean" then return if value then "True" else "False"
    elseif t == "number" then return tostring(value)
    elseif t == "string" then return string.format("'%s'", value)
    elseif t == "table" then
        if seen[value] then return "<circular>" end
        seen[value] = true

        local mt = getmetatable(value)
        local type_name = (mt and mt.__type) or "table"

        local parts = {}
        local is_array = #value > 0

        if is_array then
            for i, v in ipairs(value) do
                if i > 10 then
                    table.insert(parts, "...")
                    break
                end
                table.insert(parts, Debugger._serialize(v, max_depth, current + 1, seen))
            end
            return "[" .. table.concat(parts, ", ") .. "]"
        else
            local count = 0
            for k, v in value do
                count += 1
                if count > 10 then
                    table.insert(parts, "...")
                    break
                end
                local key_str = Debugger._serialize(k, max_depth, current + 1, seen)
                local val_str = Debugger._serialize(v, max_depth, current + 1, seen)
                table.insert(parts, key_str .. ": " .. val_str)
            end
            if type_name ~= "table" then
                return type_name .. "{" .. table.concat(parts, ", ") .. "}"
            end
            return "{" .. table.concat(parts, ", ") .. "}"
        end
    elseif t == "function" then
        return "<function>"
    end
    return tostring(value)
end

function Debugger.assert_eq(actual: any, expected: any, message: string?)
    if actual ~= expected then
        local msg = message or ""
        error(string.format(
            "AssertionError: %s\n  Expected: %s\n  Actual:   %s",
            msg, tostring(expected), tostring(actual)
        ))
    end
end

function Debugger.assert_type(value: any, expected: string, message: string?)
    local actual = type(value)
    if actual ~= expected then
        error(string.format(
            "AssertionError: %s expected type '%s', got '%s'",
            message or "value", expected, actual
        ))
    end
end

function Debugger.add_watch(name: string, getter: () -> any)
    _watches[name] = getter
end

function Debugger.check_watches(): {[string]: any}
    local results = {}
    for name, getter in _watches do
        local ok, val = pcall(getter)
        results[name] = if ok then val else "<error>"
    end
    return results
end

function Debugger.stacktrace(): string
    return debug.traceback(nil, 2)
end

return Debugger
