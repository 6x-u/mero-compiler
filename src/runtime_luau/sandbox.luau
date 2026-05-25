--!strict
-- MERO Runtime - Sandbox System
-- Isolates compiled code execution for security.
-- Developer: MERO:TG@QP4RM

local Sandbox = {}

local ALLOWED_GLOBALS = {
    "print", "warn", "error", "assert", "pcall", "xpcall",
    "type", "typeof", "tostring", "tonumber", "tostring",
    "select", "unpack", "rawget", "rawset", "rawequal",
    "setmetatable", "getmetatable",
    "pairs", "ipairs", "next",
    "string", "table", "math", "bit32", "coroutine",
    "task", "os", "debug",
    "Instance", "Enum", "Vector3", "Vector2",
    "CFrame", "Color3", "UDim2", "UDim",
    "BrickColor", "TweenInfo", "NumberRange",
    "Ray", "Region3", "Rect",
    "game", "workspace", "script",
    "require", "wait", "delay", "spawn",
    "tick", "time", "utf8",
}

function Sandbox.create_environment(base_env: {[string]: any}?): {[string]: any}
    local env = {}

    -- Copy allowed globals
    local source = base_env or getfenv(0)
    for _, name in ALLOWED_GLOBALS do
        if source[name] ~= nil then
            env[name] = source[name]
        end
    end

    -- Prevent access to restricted APIs
    env["loadstring"] = nil
    env["dofile"] = nil
    env["load"] = nil
    env["getfenv"] = nil
    env["setfenv"] = nil

    return env
end

function Sandbox.execute(code_func: () -> any, env: {[string]: any}?): (boolean, any)
    local sandbox_env = env or Sandbox.create_environment()

    -- Set environment for the function
    if setfenv then
        setfenv(code_func, sandbox_env)
    end

    return pcall(code_func)
end

function Sandbox.execute_with_timeout(code_func: () -> any, timeout: number, env: {[string]: any}?): (boolean, any)
    local result_ok = false
    local result_value: any = nil
    local finished = false

    local sandbox_env = env or Sandbox.create_environment()
    if setfenv then
        setfenv(code_func, sandbox_env)
    end

    task.spawn(function()
        result_ok, result_value = pcall(code_func)
        finished = true
    end)

    local start = os.clock()
    while not finished and (os.clock() - start) < timeout do
        task.wait()
    end

    if not finished then
        return false, "ExecutionTimeout: exceeded " .. tostring(timeout) .. "s"
    end

    return result_ok, result_value
end

function Sandbox.restrict_memory(env: {[string]: any}, max_tables: number)
    local table_count = 0
    local original_table = env.table or table

    env.table = setmetatable({}, {
        __index = function(_, key)
            if key == "create" then
                return function(size, value)
                    table_count += 1
                    if table_count > max_tables then
                        error("MemoryError: table allocation limit exceeded")
                    end
                    return original_table.create(size, value)
                end
            end
            return original_table[key]
        end
    })
end

return Sandbox
