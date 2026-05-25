--!strict
-- MERO Runtime - Decorator Support
-- Python decorator pattern implementation.
-- Developer: MERO:TG@QP4RM

local Decorator = {}

-- Apply a decorator to a function
function Decorator.apply(decorator: ((...any) -> any) -> (...any) -> any, func: (...any) -> any): (...any) -> any
    return decorator(func)
end

-- Apply multiple decorators (bottom-up, like Python)
function Decorator.apply_many(decorators: {((...any) -> any) -> (...any) -> any}, func: (...any) -> any): (...any) -> any
    local result = func
    for i = #decorators, 1, -1 do
        result = decorators[i](result)
    end
    return result
end

-- Common decorators

-- @cache / @lru_cache - memoization
function Decorator.cache(func: (...any) -> any): (...any) -> any
    local memo: {[string]: any} = {}
    return function(...: any): any
        local key = ""
        for i = 1, select("#", ...) do
            key ..= tostring(select(i, ...)) .. ","
        end
        if memo[key] ~= nil then return memo[key] end
        local result = func(...)
        memo[key] = result
        return result
    end
end

-- @timer - measure execution time
function Decorator.timer(func: (...any) -> any): (...any) -> any
    return function(...: any): any
        local start = os.clock()
        local result = func(...)
        local elapsed = os.clock() - start
        print(string.format("[timer] %.4fms", elapsed * 1000))
        return result
    end
end

-- @retry - retry on failure
function Decorator.retry(max_attempts: number, delay: number?): ((...any) -> any) -> (...any) -> any
    return function(func: (...any) -> any): (...any) -> any
        return function(...: any): any
            local last_err
            for attempt = 1, max_attempts do
                local ok, result = pcall(func, ...)
                if ok then return result end
                last_err = result
                if attempt < max_attempts and delay then
                    task.wait(delay)
                end
            end
            error(last_err)
        end
    end
end

-- @deprecated - mark function as deprecated
function Decorator.deprecated(message: string?): ((...any) -> any) -> (...any) -> any
    return function(func: (...any) -> any): (...any) -> any
        local warned = false
        return function(...: any): any
            if not warned then
                warn("[deprecated] " .. (message or "This function is deprecated"))
                warned = true
            end
            return func(...)
        end
    end
end

-- @throttle - rate limiting
function Decorator.throttle(interval: number): ((...any) -> any) -> (...any) -> any
    return function(func: (...any) -> any): (...any) -> any
        local last_call = 0
        return function(...: any): any
            local now = os.clock()
            if now - last_call >= interval then
                last_call = now
                return func(...)
            end
            return nil
        end
    end
end

-- @debounce - delay execution
function Decorator.debounce(delay_time: number): ((...any) -> any) -> (...any) -> any
    return function(func: (...any) -> any): (...any) -> any
        local scheduled = false
        local last_args: {any} = {}
        return function(...: any): any
            last_args = {...}
            if not scheduled then
                scheduled = true
                task.delay(delay_time, function()
                    scheduled = false
                    func(table.unpack(last_args))
                end)
            end
            return nil
        end
    end
end

return Decorator
