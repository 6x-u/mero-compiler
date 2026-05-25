--!strict
-- MERO Runtime - Module Loader
-- Implements Python-style module importing within Roblox.
-- Developer: MERO:TG@QP4RM

local ModuleLoader = {}

local _modules: {[string]: any} = {}
local _loading: {[string]: boolean} = {}

function ModuleLoader.register(name: string, module: any)
    _modules[name] = module
end

function ModuleLoader.require(name: string): any
    -- Already loaded
    if _modules[name] then return _modules[name] end

    -- Circular import detection
    if _loading[name] then
        error("Circular import detected: " .. name)
    end

    _loading[name] = true

    -- Try to find the module script
    local module_script = ModuleLoader._resolve(name)
    if module_script then
        local result = require(module_script)
        _modules[name] = result
        _loading[name] = nil
        return result
    end

    _loading[name] = nil
    error("ModuleNotFoundError: No module named '" .. name .. "'")
end

function ModuleLoader.import_from(module_name: string, names: {string}): {[string]: any}
    local module = ModuleLoader.require(module_name)
    local result = {}

    for _, name in names do
        if name == "*" then
            if type(module) == "table" then
                for k, v in module do
                    if type(k) == "string" and not string.sub(k, 1, 1) == "_" then
                        result[k] = v
                    end
                end
            end
        else
            if type(module) == "table" and module[name] ~= nil then
                result[name] = module[name]
            else
                error("ImportError: cannot import name '" .. name .. "' from '" .. module_name .. "'")
            end
        end
    end

    return result
end

function ModuleLoader.is_loaded(name: string): boolean
    return _modules[name] ~= nil
end

function ModuleLoader.unload(name: string)
    _modules[name] = nil
end

function ModuleLoader.list_modules(): {string}
    local names = {}
    for name in _modules do
        table.insert(names, name)
    end
    table.sort(names)
    return names
end

function ModuleLoader._resolve(name: string): ModuleScript?
    local parts = string.split(name, ".")
    local current: Instance? = script.Parent

    if not current then return nil end

    for _, part in parts do
        current = current:FindFirstChild(part)
        if not current then return nil end
    end

    if current:IsA("ModuleScript") then
        return current :: ModuleScript
    end

    -- Try with "init" submodule
    local init = current:FindFirstChild("init")
    if init and init:IsA("ModuleScript") then
        return init :: ModuleScript
    end

    return nil
end

return ModuleLoader
