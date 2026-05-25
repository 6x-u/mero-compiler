--!strict
-- MERO Runtime - Event System
-- Python-style event handling for Roblox signals.
-- Developer: MERO:TG@QP4RM

local EventSystem = {}

export type EventConnection = {
    disconnect: () -> (),
    connected: boolean,
}

export type Event = {
    _handlers: {(...any) -> ()},
    connect: (self: Event, handler: (...any) -> ()) -> EventConnection,
    fire: (self: Event, ...any) -> (),
    wait: (self: Event) -> ...any,
    once: (self: Event, handler: (...any) -> ()) -> EventConnection,
}

function EventSystem.create(): Event
    local event = {
        _handlers = {},
    } :: any

    function event:connect(handler: (...any) -> ()): EventConnection
        table.insert(self._handlers, handler)
        local idx = #self._handlers
        local conn: EventConnection = {
            connected = true,
            disconnect = function()
                if conn.connected then
                    table.remove(self._handlers, idx)
                    conn.connected = false
                end
            end,
        }
        return conn
    end

    function event:fire(...: any)
        for _, handler in self._handlers do
            task.spawn(handler, ...)
        end
    end

    function event:wait(): ...any
        local co = coroutine.running()
        local conn: EventConnection
        conn = self:connect(function(...: any)
            conn.disconnect()
            task.spawn(co, ...)
        end)
        return coroutine.yield()
    end

    function event:once(handler: (...any) -> ()): EventConnection
        local conn: EventConnection
        conn = self:connect(function(...: any)
            conn.disconnect()
            handler(...)
        end)
        return conn
    end

    return event :: Event
end

-- Decorator-style event binding
function EventSystem.on(roblox_event: RBXScriptSignal, handler: (...any) -> ()): RBXScriptConnection
    return roblox_event:Connect(handler)
end

function EventSystem.once_roblox(roblox_event: RBXScriptSignal, handler: (...any) -> ()): RBXScriptConnection
    local conn: RBXScriptConnection
    conn = roblox_event:Connect(function(...: any)
        conn:Disconnect()
        handler(...)
    end)
    return conn
end

return EventSystem
