--!strict
-- MERO Runtime - Signal System
-- Lightweight pub/sub signal system.
-- Developer: MERO:TG@QP4RM

local Signal = {}

export type SignalConnection = {
    disconnect: () -> (),
    connected: boolean,
}

export type SignalObject = {
    _handlers: {((...any) -> ())},
    connect: (self: SignalObject, handler: (...any) -> ()) -> SignalConnection,
    emit: (self: SignalObject, ...any) -> (),
    wait: (self: SignalObject) -> ...any,
    once: (self: SignalObject, handler: (...any) -> ()) -> SignalConnection,
    clear: (self: SignalObject) -> (),
}

function Signal.new(): SignalObject
    local signal = {
        _handlers = {},
    } :: any

    function signal:connect(handler: (...any) -> ()): SignalConnection
        table.insert(self._handlers, handler)
        local idx = #self._handlers
        local conn: SignalConnection = {
            connected = true,
            disconnect = function()
                if conn.connected then
                    for i, h in self._handlers do
                        if h == handler then
                            table.remove(self._handlers, i)
                            break
                        end
                    end
                    conn.connected = false
                end
            end,
        }
        return conn
    end

    function signal:emit(...: any)
        for _, handler in self._handlers do
            task.spawn(handler, ...)
        end
    end

    function signal:wait(): ...any
        local co = coroutine.running()
        local conn: SignalConnection
        conn = self:connect(function(...: any)
            conn.disconnect()
            task.spawn(co, ...)
        end)
        return coroutine.yield()
    end

    function signal:once(handler: (...any) -> ()): SignalConnection
        local conn: SignalConnection
        conn = self:connect(function(...: any)
            conn.disconnect()
            handler(...)
        end)
        return conn
    end

    function signal:clear()
        table.clear(self._handlers)
    end

    return signal :: SignalObject
end

return Signal
