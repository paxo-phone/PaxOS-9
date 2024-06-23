function run()
    win=gui:window()

    list = gui:vlist(win, 35, 90, 250, 280)

    
    -- read messages
    messages = gsm.getMessages("+33612345678")

    print(messages[1].message)
    -- iterate over messages
    for i, message in pairs(messages) do
        -- create a new label
        label = gui:label(list, 0, 0, 250-35, 36)
        -- set the message text
        print(message)
        label:setText(message.message)
    end

    gui:setWindow(win)
end