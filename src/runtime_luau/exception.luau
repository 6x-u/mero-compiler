--!strict
-- MERO Runtime - Exception System
-- Python-style exception handling for Luau.
-- Developer: MERO:TG@QP4RM

local Exception = {}

export type ExceptionObject = {
    type: string,
    message: string,
    traceback: string?,
    cause: ExceptionObject?,
}

function Exception.new(exc_type: string, message: string): ExceptionObject
    return {
        type = exc_type,
        message = message,
        traceback = debug.traceback(nil, 2),
        cause = nil,
    }
end

function Exception.ValueError(message: string): ExceptionObject
    return Exception.new("ValueError", message)
end

function Exception.TypeError(message: string): ExceptionObject
    return Exception.new("TypeError", message)
end

function Exception.KeyError(key: any): ExceptionObject
    return Exception.new("KeyError", tostring(key))
end

function Exception.IndexError(message: string): ExceptionObject
    return Exception.new("IndexError", message)
end

function Exception.AttributeError(message: string): ExceptionObject
    return Exception.new("AttributeError", message)
end

function Exception.RuntimeError(message: string): ExceptionObject
    return Exception.new("RuntimeError", message)
end

function Exception.StopIteration(): ExceptionObject
    return Exception.new("StopIteration", "")
end

function Exception.NotImplementedError(message: string?): ExceptionObject
    return Exception.new("NotImplementedError", message or "")
end

function Exception.ZeroDivisionError(): ExceptionObject
    return Exception.new("ZeroDivisionError", "division by zero")
end

function Exception.is_instance(exc: any, exc_type: string): boolean
    if type(exc) == "table" and exc.type then
        return exc.type == exc_type
    end
    if type(exc) == "string" then
        return string.find(exc, exc_type, 1, true) ~= nil
    end
    return false
end

function Exception.format(exc: ExceptionObject): string
    local result = exc.type .. ": " .. exc.message
    if exc.traceback then
        result ..= "\n" .. exc.traceback
    end
    return result
end

-- try/except implementation helper
function Exception.try_except(
    try_fn: () -> (),
    handlers: {{type: string?, name: string?, handler: (any) -> ()}}?,
    else_fn: (() -> ())?,
    finally_fn: (() -> ())?
)
    local ok, err = pcall(try_fn)

    if not ok and handlers then
        local handled = false
        for _, h in handlers do
            if h.type == nil or Exception.is_instance(err, h.type) then
                h.handler(err)
                handled = true
                break
            end
        end
        if not handled then
            if finally_fn then finally_fn() end
            error(err)
        end
    elseif ok and else_fn then
        else_fn()
    end

    if finally_fn then
        finally_fn()
    end
end

return Exception
