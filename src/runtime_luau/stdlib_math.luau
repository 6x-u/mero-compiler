--!strict
-- MERO Runtime - Math Standard Library
-- Python math module equivalent for Luau.
-- Developer: MERO:TG@QP4RM

local MathLib = {}

MathLib.pi = math.pi
MathLib.e = 2.718281828459045
MathLib.inf = math.huge
MathLib.nan = 0/0
MathLib.tau = math.pi * 2

function MathLib.floor(x: number): number return math.floor(x) end
function MathLib.ceil(x: number): number return math.ceil(x) end
function MathLib.sqrt(x: number): number return math.sqrt(x) end
function MathLib.log(x: number, base: number?): number
    if base then return math.log(x) / math.log(base) end
    return math.log(x)
end
function MathLib.log2(x: number): number return math.log(x) / math.log(2) end
function MathLib.log10(x: number): number return math.log(x) / math.log(10) end
function MathLib.exp(x: number): number return math.exp(x) end
function MathLib.pow(x: number, y: number): number return x ^ y end

function MathLib.sin(x: number): number return math.sin(x) end
function MathLib.cos(x: number): number return math.cos(x) end
function MathLib.tan(x: number): number return math.tan(x) end
function MathLib.asin(x: number): number return math.asin(x) end
function MathLib.acos(x: number): number return math.acos(x) end
function MathLib.atan(x: number): number return math.atan(x) end
function MathLib.atan2(y: number, x: number): number return math.atan2(y, x) end
function MathLib.sinh(x: number): number return (math.exp(x) - math.exp(-x)) / 2 end
function MathLib.cosh(x: number): number return (math.exp(x) + math.exp(-x)) / 2 end
function MathLib.tanh(x: number): number
    local e2x = math.exp(2 * x)
    return (e2x - 1) / (e2x + 1)
end

function MathLib.fabs(x: number): number return math.abs(x) end
function MathLib.factorial(n: number): number
    if n < 0 then error("factorial() not defined for negative values") end
    if n <= 1 then return 1 end
    local result = 1
    for i = 2, n do result *= i end
    return result
end

function MathLib.gcd(a: number, b: number): number
    a, b = math.abs(a), math.abs(b)
    while b ~= 0 do a, b = b, a % b end
    return a
end

function MathLib.lcm(a: number, b: number): number
    if a == 0 or b == 0 then return 0 end
    return math.abs(a * b) / MathLib.gcd(a, b)
end

function MathLib.isnan(x: number): boolean return x ~= x end
function MathLib.isinf(x: number): boolean return x == math.huge or x == -math.huge end
function MathLib.isfinite(x: number): boolean return not MathLib.isnan(x) and not MathLib.isinf(x) end

function MathLib.copysign(x: number, y: number): number
    if y >= 0 then return math.abs(x) else return -math.abs(x) end
end

function MathLib.fmod(x: number, y: number): number return x % y end
function MathLib.trunc(x: number): number
    if x >= 0 then return math.floor(x) else return math.ceil(x) end
end

function MathLib.radians(deg: number): number return deg * math.pi / 180 end
function MathLib.degrees(rad: number): number return rad * 180 / math.pi end

function MathLib.comb(n: number, k: number): number
    if k < 0 or k > n then return 0 end
    if k == 0 or k == n then return 1 end
    k = math.min(k, n - k)
    local result = 1
    for i = 0, k - 1 do
        result = result * (n - i) / (i + 1)
    end
    return math.floor(result)
end

function MathLib.perm(n: number, k: number?): number
    local r = k or n
    if r < 0 or r > n then return 0 end
    local result = 1
    for i = 0, r - 1 do result *= (n - i) end
    return result
end

return MathLib
