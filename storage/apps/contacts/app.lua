function quit()
    gsm.saveContacts()
end

function update()
    time:setTimeout(function ()
        gui:del(win2)
        gui:del(win)
        print("update finished")
        run()
    end,0)
end

function leaveContact(contact)
    time:setTimeout(function ()
        gui:del(win2)
        gui:del(win3)
        print("update finished")
        openContact(contact)
    end,0)
end

function warning(c)
    print("warning")
    win4=gui:window()

    local back = gui:image(win4, "back.png", 30, 30, 18, 18)
    back:onClick(function() 
        time:setTimeout(function () gui:del(win4) gui:setWindow(win2) end, 0) end)

    local icon = gui:image(win4, "warning.png", 136, 149, 48, 48)

    local msg = gui:label(win4, 23, 212, 273, 21)
    msg:setText("Suppression de \"" .. c.name .. "\"")
    msg:setHorizontalAlignment(CENTER_ALIGNMENT)

    local war = gui:label(win4, 23, 239, 273, 21)
    war:setText("Cette action est irréversible")
    war:setTextColor(COLOR_GREY)
    war:setHorizontalAlignment(CENTER_ALIGNMENT)

    local delete = gui:button(win4, 35, 394, 250, 38);
    delete:setText("Supprimer")
    delete:onClick(function()
        time:setTimeout(function ()
            gsm.deleteContact(c.name)
            gui:del(win4)
            update()
        end,0)
    end)

    gui:setWindow(win4)
end

function editContact(contact)
    win3=gui:window()

    local icon = gui:image(win3, "back.png", 30, 30, 18, 18)
    icon:onClick(function() leaveContact(contact) end)

    local name = gui:input(win3, 35, 121, 250, 40)
    name:setTitle("Nom")
    name:setText(contact.name)
    name:onClick(function () name:setText(gui:keyboard("Nom", contact.name)) end)

    local num = gui:input(win3, 35, 216, 250, 40)
    num:setTitle("Numéro")
    num:setText(contact.phone)
    num:onClick(function () num:setText(gui:keyboard("Numéro", contact.phone)) end)

    edit = gui:button(win3, 35, 394, 250, 38);
    edit:setText("Modifier")
    edit:onClick(function()
        local c = Contact:new()
        c.name=name:getText()
        c.phone=num:getText()
        gsm.editContact(contact.name, c)
        print(c.name .. " " .. c.phone)
        leaveContact(c)
    end)

    gui:setWindow(win3)
end

function newContact()
    win2=gui:window()

    local icon = gui:image(win2, "back.png", 30, 30, 18, 18)
    icon:onClick(update)

    local name = gui:input(win2, 35, 121, 250, 40)
    name:setTitle("Nom")
    name:onClick(function () name:setText(gui:keyboard("Nom", "")) end)

    local num = gui:input(win2, 35, 216, 250, 40)
    num:setTitle("Numéro")
    num:onClick(function () num:setText(gui:keyboard("Numéro", "")) end)

    edit = gui:button(win2, 35, 394, 250, 38);
    edit:setText("Créer")
    edit:onClick(function()
        local c = Contact:new()
        c.name=name:getText()
        c.phone=num:getText()
        gsm.addContact(c)
        print(c.name .. " " .. c.phone)
        update()
    end)

    gui:setWindow(win2)
end

function openContact(contact)
    win2=gui:window()

    local icon = gui:image(win2, "back.png", 30, 30, 18, 18)
    icon:onClick(update)

    local name = gui:label(win2, 53, 110, 210, 28)
    name:setFontSize(24)
    name:setHorizontalAlignment(CENTER_ALIGNMENT)
    name:setText(contact.name)

    local num = gui:label(win2, 53, 144, 210, 28)
    num:setFontSize(24)
    num:setHorizontalAlignment(CENTER_ALIGNMENT)
    num:setTextColor(COLOR_GREY)
    num:setText(contact.phone)

    local messages = gui:button(win2, 35, 244, 250, 38)
    messages:setText("Messages")
    messages:onClick(function () launch("messages", {contact.phone}) end)

    local call = gui:button(win2, 35, 294, 250, 38);
    call:setText("Appeler")
    call:onClick(function () launch("phone", {contact.phone}) end)

    local edit = gui:button(win2, 35, 344, 250, 38);
    edit:setText("Éditer")
    edit:onClick(function()
        editContact(contact)
    end)

    local delete = gui:button(win2, 35, 394, 250, 38);
    delete:setText("Supprimer")
    delete:onClick(function() warning(contact) end)

    gui:setWindow(win2)
end

function quit()
    gsm.saveContacts()
end

function run()
    contactList = gsm:listContacts()

    win=gui:window()

    local title=gui:label(win, 35, 35, 144, 28)
    title:setFontSize(24)
    title:setText("Contacts")

    local add = gui:box(win, 250, 410, 40, 40)
    add:setBackgroundColor(COLOR_DARK)
    add:setRadius(20)
    
        local icon_plus = gui:image(add, "plus.png", 14, 14, 12, 12, COLOR_DARK)

    add:onClick(newContact)

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