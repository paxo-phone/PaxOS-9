
function run()
    win=gui:window()

    local icon = gui:image(win, "low-battery.png", 110, 137, 100, 177)

    local alert = gui:label(win, 99, 362, 180, 20)
    alert:setText("Merci de recharger")
    alert:setFontSize(16)

    gui:setWindow(win)
end