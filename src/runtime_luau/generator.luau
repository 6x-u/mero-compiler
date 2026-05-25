--!strict
-- MERO Runtime - Generator/Coroutine Support
-- Python-style generators using Luau coroutines.
-- Developer: MERO:TG@QP4RM

local Generator = {}

export type GeneratorObject = {
    _co: thread,
    _done: boolean,
    _value: any,
    send: (self: GeneratorObject, value: any?) -> (boolean, any),
    close: (self: GeneratorObject) -> (),
    iter: (self: GeneratorObject) -> () -> any?,
}

function Generator.create(func: (...any) -> ()): (...any) -> GeneratorObject
    return function(...: any): GeneratorObject
        local args = {...}
        local gen = {} :: any

        gen._co = coroutine.create(function()
            func(table.unpack(args))
        end)
        gen._done = false
        gen._value = nil

        function gen:send(value: any?): (boolean, any)
            if self._done then return false, nil end

            local ok, result = coroutine.resume(self._co, value)
            if not ok or coroutine.status(self._co) == "dead" then
                self._done = true
                return false, result
            end
            self._value = result
            return true, result
        end

        function gen:close()
            self._done = true
        end

        function gen:iter(): () -> any?
            return function()
                local has_value, value = self:send(nil)
                if has_value then return value end
                return nil
            end
        end

        -- Start the coroutine
        local ok, first = coroutine.resume(gen._co)
        if not ok or coroutine.status(gen._co) == "dead" then
            gen._done = true
        else
            gen._value = first
        end

        return gen :: GeneratorObject
    end
end

-- yield equivalent (must be called inside a generator function)
function Generator.yield(value: any)
    coroutine.yield(value)
end

-- yield from equivalent
function Generator.yield_from(iterable: {any} | GeneratorObject)
    if type(iterable) == "table" then
        if iterable._co then
            -- It's a generator
            local gen = iterable :: GeneratorObject
            for val in gen:iter() do
                coroutine.yield(val)
            end
        else
            -- It's a list
            for _, val in iterable :: {any} do
                coroutine.yield(val)
            end
        end
    end
end

-- Infinite generator helpers
function Generator.count(start: number?, step: number?): GeneratorObject
    return Generator.create(function()
        local current = start or 0
        local s = step or 1
        while true do
            Generator.yield(current)
            current += s
        end
    end)()
end

function Generator.cycle(iterable: {any}): GeneratorObject
    return Generator.create(function()
        while true do
            for _, val in iterable do
                Generator.yield(val)
            end
        end
    end)()
end

return Generator
