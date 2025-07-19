function loadNotifs()
    local unreadfile = storage:file("unread.txt", READ)

    unreadfile:open()
    notifs = unreadfile:readAll()
    -- print ("unread file: " .. notifs)
    unreadfile:close()
end

listUpdated = false

function eraseNotif(number)
    local unreadfile = storage:file("unread.txt", WRITE)

    unreadfile:open()
    unreadfile:write(string.gsub(notifs, number.."\n", ""))
    unreadfile:close()

    listUpdated = true
end

function openImage(path)
    win3=gui:window()

    local image = gui:image(win3, "images/" .. path, 0, 0, 320, 480)
    gui:setWindow(win3)
end

function appendMessage(msg, list)
    local bull2 = gui:box(list, 0, 0, 184, 30)
    
    label_sent = gui:label(bull2, 0, 0, 184, 0)
    label_sent:setHorizontalAlignment(CENTER_ALIGNMENT)
    label_sent:setText(msg)
    label_sent:setFontSize(18)

    local labelHeight = label_sent:getTextHeight() + 8

    label_sent:setHeight(labelHeight)

    local canva = gui:canvas(bull2, 0, labelHeight, 68, 1)
    canva:fillRect(0, 0, 68, 1, COLOR_DARK)
    canva:setX(57)

    bull2:setX(96)
    bull2:setHeight(labelHeight + 9)
end

function converation(number)
    events.onmessage(function ()
        if (win2~= nil) then
            gui:del(win2)
            converation(number)
        end
    end)

    win2=gui:window()

    local c = gsm.getContactByNumber(number)

    if(notifs == nil) then
        loadNotifs()
    end

    if(notifs:find(number) ~= nil) then
        eraseNotif(number)
    end

    local title=gui:label(win2, 90, 30, 141, 22)
    title:setHorizontalAlignment(CENTER_ALIGNMENT)
    title:setFontSize(20)
    if(c.name ~= "") then
        title:setText(c.name)
    else
        title:setText(number)
    end

    list = gui:vlist(win2, 20, 76, 280, 320)
    list:setSelectionFocus(SELECTION_DOWN)
    
    messages = gsm.getMessages(number)
    --print("getMessages returned " .. tostring(messages))

    for i, message in pairs(messages) do
        local numberPart = message.message:match("/(%d+)%.jpg")  -- Extract the number part

        if(numberPart) then -- image
            -- print("image " .. numberPart)
            local image = gui:image(list, "images/" .. numberPart .. "p.jpg" , 0, 0, 60, 60)

            image:onClick(function ()
                openImage(string.sub(message.message, 2))
            end)
        else
            -- print("message " .. message.message)
            local bull = gui:box(list, 0, 0, 184, 30)
            
            local label = gui:label(bull, 0, 0, 184, 0)
            label:setHorizontalAlignment(CENTER_ALIGNMENT)
            label:setText(message.message)
            label:setFontSize(18)

            local labelHeight = label:getTextHeight() + 8

            label:setHeight(labelHeight)

            local canva = gui:canvas(bull, 0, labelHeight, 68, 1)
            canva:fillRect(0, 0, 68, 1, COLOR_DARK)
            canva:setX(57)

            if(message.who == false) then
                bull:setX(96)
            end

            bull:setHeight(labelHeight + 9)
        end
    end

    list:setIndex(#messages -1)

    local add = gui:box(win2, 250, 410, 40, 40)
    add:setBackgroundColor(COLOR_DARK)
    add:setRadius(20)
        local icon_plus = gui:image(add, "plus.png", 14, 14, 12, 12, COLOR_DARK)
        

    add:onClick(function ()
        gui:keyboard("Message au " .. number, "",
            function (msg) 
                if(msg ~= "") then
                    gsm.newMessage(number, msg)
                    appendMessage(msg, list)
                end
            end)
    end)

    local back = gui:image(win2, "back.png", 30, 30, 18, 18)
    back:onClick(function() 
        time:setTimeout(
            function ()
                gui:del(win2)
                
            end, 0)
            
        if(listUpdated == true) then
            gui:del(win)
            run({})
        else
            gui:setWindow(win)
        end
    end)

    events.onmessageerror(function ()
        if (label_sent~= nil and win2~= nil) then
            label_sent:setTextColor(COLOR_ERROR)
        end
    end)

    gui:setWindow(win2)
end

function run(arg)
    if(#arg == 1) then
        converation(arg[1])
        return
    end

    contactList = gsm:listContacts()
    
    win=gui:window()
    
    local title=gui:label(win, 35, 35, 144, 28)
    title:setFontSize(24)
    title:setText("Message")
    
    listO = gui:vlist(win, 35, 90, 250+21, 280)
    
    local files = storage:listDir("data")

    loadNotifs()

    table.sort(files, function (a, b)
        numbera = a:match("(.+)%.json")
        numberb = b:match("(.+)%.json")
        if notifs:find(numbera) ~= nil and notifs:find(numberb) == nil then
            return true
        elseif notifs:find(numberb) ~= nil and notifs:find(numbera) == nil then
            return false
        end

        return numbera < numberb
    end)
    
    for i, file in ipairs(files) do
        local case = gui:box(listO, 0, 0, 250+21, 36)
        
        local number = file:match("(.+)%.json")
        
        if notifs:find(number) then
            local unread = gui:box(case, 0, 12, 13, 13)
            unread:setBackgroundColor(COLOR_WARNING)
            unread:setRadius(6)
        end
        
        local c = gsm.getContactByNumber(number)
        
        local name = gui:label(case, 21, 0, 230, 18)
        if c.name ~= "" then
            name:setText(c.name)
            name:setFontSize(16)
        else
            name:setText(number)
            name:setFontSize(16)
        end
        
        local num = gui:label(case, 21, 18, 230, 18)
        if c.namgsme ~= "" then
            num:setText(c.phone)
        else
            num:setText("-")
        end
        
        num:setTextColor(COLOR_GREY)
        num:setFontSize(16)
        
        case:onClick(function() converation(number) end)
    end
    
    gui:setWindow(win)
end
