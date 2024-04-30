function quit()
    gsm.saveContacts()
end

function openContact(contact)
    win2=gui:window()

    local icon = gui:image(win2, "back.png", 30, 30, 18, 18)

    local name = gui:label(win2, 53, 110, 210, 28)
    name:setFontSize(24)
    name::setHorizontalAlignment(CENTER_ALIGNMENT)
    name:setText(contact.name)

    gui:setWindow(win2)
end

function run()
    if win ~= nil then
        gui:setWindow(win)
        return
    end

    contactList = gsm:listContacts()

    win=gui:window()

    local title=gui:label(win, 35, 35, 144, 28)
    title:setFontSize(24)
    title:setText("Contacts")

    add = gui:box(win, 250, 410, 40, 40)
    add:setMainColor(COLOR_DARK)
    add:setRadius(20)
    
    local icon_plus = gui:image(add, "plus.png", 14, 14, 12, 12, COLOR_DARK)

    listO = gui:vlist(win, 35, 90, 250, 280)

    for i, value in pairs(contactList) do
        local case = gui:box(listO, 0, 0, 250, 36)

        local name = gui:label(case, 0, 0, 230, 18)
        name:setText(value.name)
        name:setFontSize(16)

        local num = gui:label(case, 0, 18, 230, 18)
        num:setText(value.phone)
        num:setTextColor(COLOR_GREY)
        num:setFontSize(16)

        local icon = gui:image(case, "arrowr.png", 230, 8, 20, 20)

        case:onClick(function() openContact(value) end)
    end

    gui:setWindow(win)
end