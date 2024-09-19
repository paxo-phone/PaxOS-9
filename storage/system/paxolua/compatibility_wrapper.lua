--[[

    PaxoLua compatibility wrapper.

    This wrapper is used to run old lua application with the new Paxo Lua api.
    It is automatically included by "OldLibrary". (Manifest: {"compatibility_mode": "old_lua"}).

    It allows developers to make their old lua applications run on Paxo Lua (to gain access to the new APIs),
    but without changing any line of code (except in the manifest).

    >>> !IMPORTANT! <<<

    DO NOT TOUCH, THIS IS A CRITICAL SYSTEM FILE.

]]--

function paxo.load()
    run()
end

function paxo.update()

end

local function bind(values)
    local obj = {}
    local i = 1
    while i <= #values do
        local f = values[i + 1]
        obj[values[i]] = function(_, ...)
            return f(...)
        end
        i = i + 2
    end
    return obj
end

_ENV["gui"] = bind({
    "setWindow", paxo.gui.setWindow,
    "window", paxo.gui.window.new,
    "label", paxo.gui.label.new
})

table.dump(gui)