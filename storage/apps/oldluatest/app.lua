local lib = require("lib")

local window

function run()
    lib.helloWorld()

    window = gui:window()

    local titleLabel = gui:label(window, 0, 0, 64, 64)
    titleLabel:setText("Hello World ! (Old Lua)")

    table.dump(gui)

    gui:setWindow(window)
end
