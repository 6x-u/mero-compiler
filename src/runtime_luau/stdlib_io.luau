--!strict
-- MERO Runtime - I/O Standard Library
-- File and I/O operations adapted for Roblox environment.
-- Developer: MERO:TG@QP4RM

local IOLib = {}

-- Since Roblox doesn't have file system access, we use DataStores and StringValues

export type FileObject = {
    name: string,
    mode: string,
    _buffer: {string},
    _position: number,
    _closed: boolean,
    read: (self: FileObject, size: number?) -> string?,
    write: (self: FileObject, data: string) -> (),
    readline: (self: FileObject) -> string?,
    readlines: (self: FileObject) -> {string},
    close: (self: FileObject) -> (),
    seek: (self: FileObject, offset: number) -> (),
}

function IOLib.open(name: string, mode: string?): FileObject
    local m = mode or "r"
    local file = {
        name = name,
        mode = m,
        _buffer = {},
        _position = 1,
        _closed = false,
    } :: any

    function file:read(size: number?): string?
        if self._closed then error("ValueError: I/O operation on closed file") end
        if #self._buffer == 0 then return nil end
        local content = table.concat(self._buffer, "\n")
        if size then
            local chunk = string.sub(content, self._position, self._position + size - 1)
            self._position += size
            return chunk
        end
        return content
    end

    function file:write(data: string)
        if self._closed then error("ValueError: I/O operation on closed file") end
        table.insert(self._buffer, data)
    end

    function file:readline(): string?
        if self._closed then error("ValueError: I/O operation on closed file") end
        local content = table.concat(self._buffer, "\n")
        local line_end = string.find(content, "\n", self._position)
        if not line_end then
            if self._position <= #content then
                local line = string.sub(content, self._position)
                self._position = #content + 1
                return line
            end
            return nil
        end
        local line = string.sub(content, self._position, line_end - 1)
        self._position = line_end + 1
        return line
    end

    function file:readlines(): {string}
        if self._closed then error("ValueError: I/O operation on closed file") end
        local content = table.concat(self._buffer, "\n")
        local lines = string.split(content, "\n")
        return lines
    end

    function file:close()
        self._closed = true
    end

    function file:seek(offset: number)
        self._position = offset + 1
    end

    return file :: FileObject
end

-- print function with end and sep parameters
function IOLib.print_advanced(args: {any}, sep: string?, ending: string?)
    local separator = sep or " "
    local line_end = ending or "\n"
    local parts = {}
    for _, v in args do
        table.insert(parts, tostring(v))
    end
    local output = table.concat(parts, separator)
    if line_end ~= "\n" then
        output ..= line_end
    end
    print(output)
end

-- String-based file operations using ModuleScripts/StringValues
function IOLib.read_module(parent: Instance, name: string): string?
    local child = parent:FindFirstChild(name)
    if child then
        if child:IsA("StringValue") then
            return child.Value
        elseif child:IsA("ModuleScript") then
            return child.Source
        end
    end
    return nil
end

function IOLib.write_module(parent: Instance, name: string, content: string)
    local child = parent:FindFirstChild(name)
    if child and child:IsA("StringValue") then
        child.Value = content
    else
        local sv = Instance.new("StringValue")
        sv.Name = name
        sv.Value = content
        sv.Parent = parent
    end
end

return IOLib
