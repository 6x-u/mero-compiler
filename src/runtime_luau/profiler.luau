--!strict
-- MERO Runtime - Runtime Profiler
-- Performance measurement tools for compiled code.
-- Developer: MERO:TG@QP4RM

local Profiler = {}

type TimerEntry = {
    name: string,
    start: number,
    total: number,
    calls: number,
}

local _timers: {[string]: TimerEntry} = {}
local _active: {string} = {}
local _enabled = false

function Profiler.enable()
    _enabled = true
end

function Profiler.disable()
    _enabled = false
end

function Profiler.start(name: string)
    if not _enabled then return end
    if not _timers[name] then
        _timers[name] = {name = name, start = 0, total = 0, calls = 0}
    end
    _timers[name].start = os.clock()
    _timers[name].calls += 1
    table.insert(_active, name)
end

function Profiler.stop(name: string)
    if not _enabled then return end
    local timer = _timers[name]
    if timer and timer.start > 0 then
        timer.total += os.clock() - timer.start
        timer.start = 0
    end
    for i = #_active, 1, -1 do
        if _active[i] == name then
            table.remove(_active, i)
            break
        end
    end
end

function Profiler.measure(name: string, func: () -> any): any
    Profiler.start(name)
    local ok, result = pcall(func)
    Profiler.stop(name)
    if not ok then error(result) end
    return result
end

function Profiler.report(): string
    local entries = {}
    for _, timer in _timers do
        table.insert(entries, timer)
    end
    table.sort(entries, function(a, b) return a.total > b.total end)

    local lines = {"=== MERO Profiler Report ==="}
    local total_time = 0
    for _, e in entries do total_time += e.total end

    table.insert(lines, string.format("Total tracked: %.4fms", total_time * 1000))
    table.insert(lines, "")

    local max_name = 0
    for _, e in entries do
        max_name = math.max(max_name, #e.name)
    end

    for _, e in entries do
        local pct = if total_time > 0 then (e.total / total_time) * 100 else 0
        local avg = if e.calls > 0 then e.total / e.calls else 0
        table.insert(lines, string.format(
            "  %-" .. max_name .. "s  %7.3fms (%5.1f%%)  %d calls  avg %.3fms",
            e.name, e.total * 1000, pct, e.calls, avg * 1000
        ))
    end

    return table.concat(lines, "\n")
end

function Profiler.reset()
    table.clear(_timers)
    table.clear(_active)
end

-- Decorator-style profiling
function Profiler.wrap(name: string, func: (...any) -> any): (...any) -> any
    return function(...: any): any
        Profiler.start(name)
        local results = {pcall(func, ...)}
        Profiler.stop(name)
        if not results[1] then error(results[2]) end
        return table.unpack(results, 2)
    end
end

return Profiler
