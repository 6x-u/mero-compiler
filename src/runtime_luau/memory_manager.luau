--!strict
-- MERO Runtime - Memory Manager
-- Helps manage memory usage and provides GC optimization hints.
-- Developer: MERO:TG@QP4RM

local MemoryManager = {}

local _pools: {[string]: {any}} = {}
local _stats = {
    allocations = 0,
    deallocations = 0,
    pool_hits = 0,
    pool_misses = 0,
}

-- Object pooling for frequently created/destroyed objects
function MemoryManager.get_from_pool(pool_name: string): any?
    local pool = _pools[pool_name]
    if pool and #pool > 0 then
        _stats.pool_hits += 1
        return table.remove(pool)
    end
    _stats.pool_misses += 1
    return nil
end

function MemoryManager.return_to_pool(pool_name: string, obj: any, max_size: number?)
    if not _pools[pool_name] then
        _pools[pool_name] = {}
    end
    local pool = _pools[pool_name]
    local limit = max_size or 50
    if #pool < limit then
        table.insert(pool, obj)
        _stats.deallocations += 1
    end
end

function MemoryManager.clear_pool(pool_name: string)
    _pools[pool_name] = {}
end

function MemoryManager.clear_all_pools()
    table.clear(_pools)
end

-- Memory tracking
function MemoryManager.track_alloc()
    _stats.allocations += 1
end

function MemoryManager.get_stats(): typeof(_stats)
    return table.clone(_stats)
end

function MemoryManager.get_memory_usage(): number
    return gcinfo()  -- Returns KB used
end

-- Weak references for cache-like patterns
function MemoryManager.create_weak_table(mode: string?): {[any]: any}
    local t = {}
    setmetatable(t, {__mode = mode or "v"})
    return t
end

-- Periodic GC hint
function MemoryManager.schedule_gc(interval: number)
    task.spawn(function()
        while true do
            task.wait(interval)
            -- In Roblox, GC is automatic but we can help by clearing unused pools
            for name, pool in _pools do
                if #pool > 20 then
                    -- Trim pool to half
                    local new_pool = {}
                    for i = 1, math.floor(#pool / 2) do
                        table.insert(new_pool, pool[i])
                    end
                    _pools[name] = new_pool
                end
            end
        end
    end)
end

return MemoryManager
