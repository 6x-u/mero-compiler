--!strict
-- MERO Runtime - Async/Await Runtime
-- Coroutine-based async support for Roblox task scheduler.
-- Developer: MERO:TG@QP4RM

local AsyncRuntime = {}

export type Future = {
    _coroutine: thread?,
    _result: any,
    _error: any,
    _done: boolean,
    _callbacks: {(any) -> ()},
}

function AsyncRuntime.create_future(): Future
    return {
        _coroutine = nil,
        _result = nil,
        _error = nil,
        _done = false,
        _callbacks = {},
    }
end

function AsyncRuntime.async(func: (...any) -> any): (...any) -> Future
    return function(...: any): Future
        local future = AsyncRuntime.create_future()
        local args = {...}

        task.spawn(function()
            local ok, result = pcall(func, table.unpack(args))
            if ok then
                future._result = result
            else
                future._error = result
            end
            future._done = true

            for _, cb in future._callbacks do
                task.spawn(cb, future._result)
            end
        end)

        return future
    end
end

function AsyncRuntime.await(future: Future): any
    if future._done then
        if future._error then error(future._error) end
        return future._result
    end

    local co = coroutine.running()
    if not co then
        -- Not in a coroutine, busy wait
        while not future._done do
            task.wait()
        end
        if future._error then error(future._error) end
        return future._result
    end

    table.insert(future._callbacks, function(result)
        task.spawn(co)
    end)
    coroutine.yield()

    if future._error then error(future._error) end
    return future._result
end

function AsyncRuntime.gather(...: Future): {any}
    local futures = {...}
    local results = {}

    for i, future in futures do
        results[i] = AsyncRuntime.await(future)
    end

    return results
end

function AsyncRuntime.sleep(seconds: number): Future
    local future = AsyncRuntime.create_future()
    task.delay(seconds, function()
        future._result = nil
        future._done = true
        for _, cb in future._callbacks do
            task.spawn(cb, nil)
        end
    end)
    return future
end

function AsyncRuntime.wait_for(future: Future, timeout: number): any
    local deadline = os.clock() + timeout
    while not future._done and os.clock() < deadline do
        task.wait()
    end
    if not future._done then
        error("TimeoutError: operation timed out")
    end
    if future._error then error(future._error) end
    return future._result
end

-- Generator support
export type Generator = {
    _co: thread,
    _done: boolean,
}

function AsyncRuntime.generator(func: (...any) -> ()): (...any) -> Generator
    return function(...: any): Generator
        local args = {...}
        local co = coroutine.create(function()
            func(table.unpack(args))
        end)
        return {
            _co = co,
            _done = false,
        }
    end
end

function AsyncRuntime.yield(value: any)
    coroutine.yield(value)
end

function AsyncRuntime.next_gen(gen: Generator): (boolean, any)
    if gen._done then return false, nil end
    local ok, value = coroutine.resume(gen._co)
    if not ok or coroutine.status(gen._co) == "dead" then
        gen._done = true
        return false, value
    end
    return true, value
end

function AsyncRuntime.iter_gen(gen: Generator): () -> any?
    return function()
        local has_value, value = AsyncRuntime.next_gen(gen)
        if has_value then return value end
        return nil
    end
end

return AsyncRuntime
