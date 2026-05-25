--!strict
-- MERO Runtime - OS Standard Library
-- Python os module equivalent adapted for Roblox.
-- Developer: MERO:TG@QP4RM

local OSLib = {}

function OSLib.time(): number
    return os.time()
end

function OSLib.clock(): number
    return os.clock()
end

function OSLib.date(format: string?, t: number?): string
    return os.date(format or "%Y-%m-%d %H:%M:%S", t) :: string
end

function OSLib.difftime(t2: number, t1: number): number
    return os.difftime(t2, t1)
end

-- time.sleep equivalent using task.wait
function OSLib.sleep(seconds: number)
    task.wait(seconds)
end

-- time.time() equivalent (high precision)
function OSLib.perf_counter(): number
    return os.clock()
end

-- Platform info (always Roblox)
function OSLib.platform(): string
    return "Roblox"
end

function OSLib.system(): string
    return "MERO/Luau"
end

-- Environment variables (using attributes on a config module)
local _env: {[string]: string} = {}

function OSLib.getenv(key: string, default: string?): string?
    return _env[key] or default
end

function OSLib.setenv(key: string, value: string)
    _env[key] = value
end

-- Process-like operations
function OSLib.exit(code: number?)
    error("SystemExit:" .. tostring(code or 0))
end

-- Random (using Random object for better distribution)
local _rng = Random.new()

function OSLib.random(): number
    return _rng:NextNumber()
end

function OSLib.randint(a: number, b: number): number
    return _rng:NextInteger(a, b)
end

function OSLib.choice(seq: {any}): any
    if #seq == 0 then error("IndexError: Cannot choose from empty sequence") end
    return seq[_rng:NextInteger(1, #seq)]
end

function OSLib.shuffle(list: {any})
    for i = #list, 2, -1 do
        local j = _rng:NextInteger(1, i)
        list[i], list[j] = list[j], list[i]
    end
end

function OSLib.sample(population: {any}, k: number): {any}
    if k > #population then error("ValueError: Sample larger than population") end
    local copy = table.clone(population)
    local result = {}
    for _ = 1, k do
        local idx = _rng:NextInteger(1, #copy)
        table.insert(result, copy[idx])
        table.remove(copy, idx)
    end
    return result
end

return OSLib
