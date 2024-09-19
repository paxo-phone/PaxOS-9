local lib = require("lib")

local window

function paxo.load()
    lib.helloWorld()

    window = paxo.gui.window.new()

    local titleLabel = paxo.gui.label.new(window, 0, 0, 64, 64)
    titleLabel:setText("Hello World !")

    local canvas = paxo.gui.canvas.new(window, 0, 64, 64, 64)
    canvas:drawLine(0, 0, 64, 64, 65535)

    table.dump(paxo)

    paxo.gui.setWindow(window)
end

function paxo.update()

end
