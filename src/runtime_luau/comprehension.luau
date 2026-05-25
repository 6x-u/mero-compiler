--!strict
-- MERO Runtime - List Comprehension Helpers
-- Runtime support for compiled list/dict/set comprehensions.
-- Developer: MERO:TG@QP4RM

local Comprehension = {}

-- [expr for x in iterable]
function Comprehension.list(iterable: {any}, transform: (any) -> any, filter: ((any) -> boolean)?): {any}
    local result = {}
    for _, item in iterable do
        if filter == nil or filter(item) then
            table.insert(result, transform(item))
        end
    end
    return result
end

-- [expr for x in iterable for y in iterable2]
function Comprehension.list_nested(
    outer: {any},
    inner_fn: (any) -> {any},
    transform: (any, any) -> any,
    filter: ((any, any) -> boolean)?
): {any}
    local result = {}
    for _, x in outer do
        for _, y in inner_fn(x) do
            if filter == nil or filter(x, y) then
                table.insert(result, transform(x, y))
            end
        end
    end
    return result
end

-- {key: value for x in iterable}
function Comprehension.dict(
    iterable: {any},
    key_fn: (any) -> any,
    value_fn: (any) -> any,
    filter: ((any) -> boolean)?
): {[any]: any}
    local result = {}
    for _, item in iterable do
        if filter == nil or filter(item) then
            result[key_fn(item)] = value_fn(item)
        end
    end
    return result
end

-- {expr for x in iterable}
function Comprehension.set(iterable: {any}, transform: (any) -> any, filter: ((any) -> boolean)?): {[any]: boolean}
    local result = {}
    for _, item in iterable do
        if filter == nil or filter(item) then
            result[transform(item)] = true
        end
    end
    return result
end

-- Generator expression (lazy)
function Comprehension.generator(iterable: {any}, transform: (any) -> any, filter: ((any) -> boolean)?): () -> any?
    local i = 0
    return function()
        while true do
            i += 1
            if i > #iterable then return nil end
            local item = iterable[i]
            if filter == nil or filter(item) then
                return transform(item)
            end
        end
    end
end

return Comprehension
