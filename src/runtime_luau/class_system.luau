--!strict
-- MERO Runtime - Class System
-- Python-style OOP support with inheritance and metaclasses.
-- Developer: MERO:TG@QP4RM

local ClassSystem = {}

export type ClassMeta = {
    __name: string,
    __bases: {any},
    __init: ((self: any, ...any) -> ())?,
    __str: ((self: any) -> string)?,
    __repr: ((self: any) -> string)?,
    __eq: ((self: any, other: any) -> boolean)?,
    __lt: ((self: any, other: any) -> boolean)?,
    __add: ((self: any, other: any) -> any)?,
    __sub: ((self: any, other: any) -> any)?,
    __mul: ((self: any, other: any) -> any)?,
    __len: ((self: any) -> number)?,
    __getitem: ((self: any, key: any) -> any)?,
    __setitem: ((self: any, key: any, value: any) -> ())?,
    __contains: ((self: any, item: any) -> boolean)?,
    __iter: ((self: any) -> () -> any?)?,
}

function ClassSystem.create(name: string, bases: {any}?): any
    local cls = {}
    cls.__name = name
    cls.__bases = bases or {}
    cls.__index = cls

    -- Set up inheritance chain
    if #cls.__bases > 0 then
        setmetatable(cls, {__index = cls.__bases[1]})
    end

    -- Constructor
    function cls.new(...: any): any
        local instance = setmetatable({}, cls)
        if cls.__init then
            cls.__init(instance, ...)
        end
        return instance
    end

    -- Operator overloading support
    local mt = getmetatable(cls) or {}
    mt.__call = function(_, ...: any)
        return cls.new(...)
    end
    mt.__tostring = function(self)
        if cls.__str then return cls.__str(self) end
        return string.format("<%s instance>", name)
    end
    setmetatable(cls, mt)

    return cls
end

function ClassSystem.inherit(child: any, parent: any)
    setmetatable(child, {__index = parent})
    child.__bases = {parent}
end

function ClassSystem.is_instance(obj: any, cls: any): boolean
    if type(obj) ~= "table" then return false end
    local mt = getmetatable(obj)
    while mt do
        if mt.__index == cls then return true end
        local parent_mt = getmetatable(mt.__index)
        if parent_mt then
            mt = parent_mt
        else
            break
        end
    end
    return false
end

function ClassSystem.is_subclass(child: any, parent: any): boolean
    if child == parent then return true end
    if type(child) ~= "table" or not child.__bases then return false end
    for _, base in child.__bases do
        if base == parent or ClassSystem.is_subclass(base, parent) then
            return true
        end
    end
    return false
end

-- Property descriptor
function ClassSystem.property(fget: ((any) -> any)?, fset: ((any, any) -> ())?): any
    return {
        __type = "property",
        get = fget,
        set = fset,
    }
end

-- Static method marker
function ClassSystem.staticmethod(func: (...any) -> any): any
    return {__type = "staticmethod", func = func}
end

-- Class method marker
function ClassSystem.classmethod(func: (any, ...any) -> any): any
    return {__type = "classmethod", func = func}
end

return ClassSystem
