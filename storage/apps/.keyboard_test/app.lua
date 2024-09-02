function run()
    win=gui:window()

    local label = gui:label(win, 0, 0, 200, 32)

    local keyboard = gui:keyboard("Placeholder", "Default text")

    label:setText(keyboard)

    gui:setWindow(win)
end