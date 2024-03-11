function run()
    print("Running lua script")
    win=gui:window()

    i=gui:input(win, 35, 35, 0, 0)
    i:setText("this is text")

    gui:setWindow(win)
end