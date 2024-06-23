function run()
    win = gui:window()

    -- Canvas tests
 
    print("COLOR_BLUE: " .. COLOR_BLUE)
    print("COLOR_DARK: " .. COLOR_DARK)
    

    testCanvas = gui:canvas(win, 65, 215, 25, 25)
    testCanvas:fillRect(5, 5, 15, 15, COLOR_BLUE)
    testCanvas:onClick (function() end)

    gui:setWindow(win)
end