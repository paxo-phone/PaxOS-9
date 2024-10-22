function run(arg)
    win = gui:window()

    title = gui:label(win, 0, 160, 320, 40)
    title:setText("Paxo")
    title:setTextColor(58, 186, 153)
    title:setHorizontalAlignment(CENTER_ALIGNMENT)
    title:setFontSize(42)

    title = gui:label(win, 0, 210, 320, 40)
    title:setText("Running on PaxOS 9")
    title:setTextColor(100, 100, 100)
    title:setHorizontalAlignment(CENTER_ALIGNMENT)
    title:setFontSize(20)

    button = gui:button(win, 100, 380, 120, 40)
    button:setIcon("sys_apps/.oobe/resources/images/arrow_forward_ios_64px.png")
    button:setText("Welcome")
    button:onClick(function ()
        paxo.system.config.set("oobe", true)
        paxo.system.config.write()

        paxo.app.quit()
    end)

    gui:setWindow(win)
end
