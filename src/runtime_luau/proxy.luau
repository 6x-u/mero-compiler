--!strict
-- MERO Runtime - Proxy/Metatables
-- Dynamic attribute access and operator overloading support.
-- Developer: MERO:TG@QP4RM

local Proxy = {}

-- Create a proxy object with custom __index and __newindex
function Proxy.create(target: {[any]: any}, handler: {
    get: ((target: any, key: any) -> any)?,
    set: ((target: any, key: any, value: any) -> ())?,
    call: ((target: any, ...any) -> any)?,
}): any
    local mt = {}

    if handler.get then
        mt.__index = function(_, key)
            return handler.get(target, key)
        end
    else
        mt.__index = target
    end

    if handler.set then
        mt.__newindex = function(_, key, value)
            handler.set(target, key, value)
        end
    else
        mt.__newindex = target
    end

    if handler.call then
        mt.__call = function(_, ...)
            return handler.call(target, ...)
        end
    end

    mt.__tostring = function()
        if target.__str then return target.__str(target) end
        return tostring(target)
    end

    mt.__len = function()
        if target.__len then return target.__len(target) end
        return #target
    end

    mt.__eq = function(a, b)
        if target.__eq then return target.__eq(a, b) end
        return rawequal(a, b)
    end

    mt.__add = function(a, b)
        if target.__add then return target.__add(a, b) end
        return nil
    end

    mt.__sub = function(a, b)
        if target.__sub then return target.__sub(a, b) end
        return nil
    end

    mt.__mul = function(a, b)
        if target.__mul then return target.__mul(a, b) end
        return nil
    end

    return setmetatable({}, mt)
end

-- Python __getattr__ / __setattr__ simulation
function Proxy.with_attr_access(obj: any, getattr: (string) -> any, setattr: (string, any) -> ()): any
    return setmetatable({}, {
        __index = function(_, key)
            local direct = rawget(obj, key)
            if direct ~= nil then return direct end
            return getattr(key)
        end,
        __newindex = function(_, key, value)
            setattr(key, value)
        end,
    })
end

-- @property support
function Proxy.make_property(obj: any, name: string, getter: () -> any, setter: ((any) -> ())?)
    local mt = getmetatable(obj) or {}
    local properties = mt._properties or {}
    properties[name] = {get = getter, set = setter}
    mt._properties = properties

    if not mt._has_property_index then
        local original_index = mt.__index
        mt.__index = function(self, key)
            local props = getmetatable(self)._properties
            if props and props[key] then
                return props[key].get()
            end
            if type(original_index) == "table" then
                return original_index[key]
            elseif type(original_index) == "function" then
                return original_index(self, key)
            end
            return nil
        end

        local original_newindex = mt.__newindex
        mt.__newindex = function(self, key, value)
            local props = getmetatable(self)._properties
            if props and props[key] then
                if props[key].set then
                    props[key].set(value)
                else
                    error("AttributeError: can't set attribute '" .. key .. "'")
                end
                return
            end
            if original_newindex then
                original_newindex(self, key, value)
            else
                rawset(self, key, value)
            end
        end
        mt._has_property_index = true
    end

    setmetatable(obj, mt)
end

return Proxy
