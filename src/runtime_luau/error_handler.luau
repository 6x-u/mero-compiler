--!strict
-- MERO Runtime - Error Handler
-- Formats and reports runtime errors with Python-style tracebacks.
-- Developer: MERO:TG@QP4RM

local ErrorHandler = {}

export type ErrorInfo = {
    error_type: string,
    message: string,
    traceback: string,
    source_file: string?,
    source_line: number?,
}

function ErrorHandler.wrap(func: (...any) -> ...any): (...any) -> (boolean, ...any)
    return function(...: any)
        return pcall(func, ...)
    end
end

function ErrorHandler.handle(err: any): ErrorInfo
    local info: ErrorInfo = {
        error_type = "RuntimeError",
        message = "",
        traceback = "",
        source_file = nil,
        source_line = nil,
    }

    if type(err) == "string" then
        -- Parse Luau error format: "file:line: message"
        local file, line, msg = string.match(err, "^(.+):(%d+): (.+)$")
        if file and line and msg then
            info.source_file = file
            info.source_line = tonumber(line)
            info.message = msg
        else
            info.message = err
        end
    elseif type(err) == "table" then
        info.error_type = err.type or "RuntimeError"
        info.message = err.message or tostring(err)
        info.traceback = err.traceback or ""
    else
        info.message = tostring(err)
    end

    if info.traceback == "" then
        info.traceback = debug.traceback(nil, 2)
    end

    return info
end

function ErrorHandler.format_error(info: ErrorInfo): string
    local lines = {}
    table.insert(lines, "Traceback (most recent call last):")

    if info.traceback and #info.traceback > 0 then
        for line in string.gmatch(info.traceback, "[^\n]+") do
            if not string.find(line, "ErrorHandler") then
                table.insert(lines, "  " .. line)
            end
        end
    end

    if info.source_file and info.source_line then
        table.insert(lines, string.format('  File "%s", line %d', info.source_file, info.source_line))
    end

    table.insert(lines, info.error_type .. ": " .. info.message)
    return table.concat(lines, "\n")
end

function ErrorHandler.safe_call(func: (...any) -> ...any, ...: any): (boolean, any)
    local args = {...}
    local ok, result = xpcall(function()
        return func(table.unpack(args))
    end, function(err)
        return ErrorHandler.handle(err)
    end)

    if ok then
        return true, result
    else
        warn(ErrorHandler.format_error(result))
        return false, result
    end
end

return ErrorHandler
