--!strict
-- MERO Runtime - Iterator Protocol
-- Python-style iterators and iteration utilities.
-- Developer: MERO:TG@QP4RM

local Iterator = {}

export type IteratorObject = {
    next: () -> (any?, any?),
}

function Iterator.from_table(t: {any}): () -> (number?, any?)
    local i = 0
    return function()
        i += 1
        if i <= #t then return i - 1, t[i] end
        return nil, nil
    end
end

function Iterator.from_pairs(t: {[any]: any}): () -> (any?, any?)
    local key = nil
    return function()
        local k, v = next(t, key)
        key = k
        return k, v
    end
end

function Iterator.enumerate(iterable: {any}, start: number?): () -> (number?, any?)
    local i = (start or 0) - 1
    local idx = 0
    return function()
        idx += 1
        if idx <= #iterable then
            i += 1
            return i, iterable[idx]
        end
        return nil, nil
    end
end

function Iterator.zip(...: {any}): () -> ...any?
    local arrays = {...}
    local min_len = math.huge
    for _, arr in arrays do
        min_len = math.min(min_len, #arr)
    end
    local i = 0
    return function(): ...any?
        i += 1
        if i > min_len then return nil end
        local values = {}
        for _, arr in arrays do
            table.insert(values, arr[i])
        end
        return table.unpack(values)
    end
end

function Iterator.chain(...: {any}): () -> any?
    local arrays = {...}
    local arr_idx = 1
    local elem_idx = 0
    return function()
        while arr_idx <= #arrays do
            elem_idx += 1
            if elem_idx <= #arrays[arr_idx] then
                return arrays[arr_idx][elem_idx]
            end
            arr_idx += 1
            elem_idx = 0
        end
        return nil
    end
end

function Iterator.repeat_value(value: any, times: number?): () -> any?
    local count = 0
    local max = times or math.huge
    return function()
        count += 1
        if count <= max then return value end
        return nil
    end
end

function Iterator.count(start: number?, step: number?): () -> number?
    local current = (start or 0) - (step or 1)
    local s = step or 1
    return function()
        current += s
        return current
    end
end

function Iterator.take(iter: () -> any?, n: number): {any}
    local result = {}
    for _ = 1, n do
        local val = iter()
        if val == nil then break end
        table.insert(result, val)
    end
    return result
end

function Iterator.collect(iter: () -> any?): {any}
    local result = {}
    while true do
        local val = iter()
        if val == nil then break end
        table.insert(result, val)
    end
    return result
end

function Iterator.map(func: (any) -> any, iter: () -> any?): () -> any?
    return function()
        local val = iter()
        if val == nil then return nil end
        return func(val)
    end
end

function Iterator.filter(pred: (any) -> boolean, iter: () -> any?): () -> any?
    return function()
        while true do
            local val = iter()
            if val == nil then return nil end
            if pred(val) then return val end
        end
    end
end

return Iterator
