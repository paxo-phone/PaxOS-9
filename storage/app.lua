function run()
    print("Running lua script")
    win=gui:window()

    i=gui:input(win, 35, 35, 0, 0)
    i:setText("this is text")

    b=gui:switch(win,35, 100)
    b:setState(true)

    b:onClick(function ()
        if (b:getState() == true) then
            i:enable()
        else
            i:disable()
        end end)

        time:setInterval(function () b:setState(not b:getState()) end, 2000)

    gui:setWindow(win)
end