global_list = {
    { time = "08:30", enabled = 1 },
    { time = "09:30", enabled = 0 }
}

function reloadProcess()
    system.app.stopApp("alarme..task")
    launch("alarme..task")
end

function showAlarm(index)
    win3 = gui:window()

    local label = gui:label(win3, 67, 19, 186, 37)
    label:setText("Alarmes")
    label:setHorizontalAlignment(CENTER_ALIGNMENT)
    label:setFontSize(32)

    timel = gui:label(win3, 68, 197, 183, 45)
    timel:setText(global_list[index].time)
    timel:setHorizontalAlignment(CENTER_ALIGNMENT)
    timel:setFontSize(48)

    local button = gui:button(win3, 35, 401, 250, 38)
    button:setText("Arrêter")
    button:onClick(function()
        system.app.quit()
    end)

    time:setInterval(function()
        hardware.vibrate({true, true, false, false, true, true})
    end, 1000)

    gui:setWindow(win3)
end

function openAlarm(index)
    if(index == -1) then
        create = true
        index = #global_list + 1
        table.insert(global_list, {time = "00:00", enabled = 0})
    else
        create = false
    end

    if(win2 ~= nil) then
        gui:del(win2)
    end
    win2 = gui:window()

    local label = gui:label(win2, 67, 19, 186, 37)
    label:setText("Alarmes")
    label:setHorizontalAlignment(CENTER_ALIGNMENT)
    label:setFontSize(32)

    timel = gui:label(win2, 68, 197, 183, 45)
    timel:setText(global_list[index].time)
    timel:setHorizontalAlignment(CENTER_ALIGNMENT)
    timel:setFontSize(48)

    timef = {tonumber(global_list[index].time:sub(1, 1)), tonumber(global_list[index].time:sub(2, 2)), tonumber(global_list[index].time:sub(4, 4)), tonumber(global_list[index].time:sub(5, 5))}

    for i = 1, 4 do
        imgup = gui:image(win2, "up.png", 64 + (i - 1) * 48, 149, 48, 48)
        imgup:onClick(function()
            if(i == 1) then
                timef[1] = (timef[1] + 1)%3
            elseif(i == 2) then
                timef[2] = (timef[2] + 1)%10
            elseif(i == 3) then
                timef[3] = (timef[3] + 1)%6
            elseif(i == 4) then
                timef[4] = (timef[4] + 1)%10
            end

            if(timef[1]*10 + timef[2] > 23) then
                timef[1] = 0
                timef[2] = 0
            end

            timel:setText(string.format("%02d:%02d", timef[1] * 10 + timef[2], timef[3] * 10 + timef[4]))
        end)
    end

    for i = 1, 4 do
        imgdown = gui:image(win2, "down.png", 64 + (i - 1) * 48, 242, 48, 48)
        imgdown:onClick(function()
            if(i == 1) then
                timef[1] = (timef[1] - 1) % 3
            elseif(i == 2) then
                timef[2] = (timef[2] - 1) % 10
            elseif(i == 3) then
                timef[3] = (timef[3] - 1) % 6
            elseif(i == 4) then
                timef[4] = (timef[4] - 1) % 10
            end

            if(timef[1]*10 + timef[2] > 23) then
                timef[1] = 2
                timef[2] = 3
            end

            timel:setText(string.format("%02d:%02d", timef[1] * 10 + timef[2], timef[3] * 10 + timef[4]))
        end)
    end

    local button = gui:button(win2, 35, 401, 250, 38)
    button:setText("Enregistrer")
    button:onClick(function()
        global_list[index].time = timel:getText()
        saveTable("alarms.tab", global_list)
        gui:del(win2)
        gui:del(win)
        win = nil
        win2 = nil

        reloadProcess()

        run({})
    end)

    gui:setWindow(win2)
end

function run(arg)
    if storage:isFile("alarms.tab") then
        global_list = loadTable("alarms.tab")
    else
        saveTable("alarms.tab", global_list)
    end

    if(#arg == 1) then
        showAlarm(tonumber(arg[1]))
        return
    end

    win = gui:window()

    local label = gui:label(win, 67, 19, 186, 37)
    label:setText("Alarmes")
    label:setHorizontalAlignment(CENTER_ALIGNMENT)
    label:setFontSize(32)

    local list = gui:vlist(win, 86, 81, 192, 320)
    for i, v in ipairs(global_list) do
        local case = gui:box(list, 0, 0, 192, 50)
            local label = gui:label(case, 0, 7, 97, 37)
                label:setText(v.time)
                label:setFontSize(32)
                label:setVerticalAlignment(CENTER_ALIGNMENT)
            local switch = gui:switch(case, 95, 15)
                switch:setState(v.enabled == 1)
                switch:onClick(function()
                    global_list[i].enabled = switch:getState() and 1 or 0
                    saveTable("alarms.tab", global_list)
                    reloadProcess()
                end)
            local del = gui:image(case, "delete.png", 160, 9, 32, 32)
                del:onClick(function()
                    table.remove(global_list, i)
                    saveTable("alarms.tab", global_list)

                    gui:del(win)
                    reloadProcess()
                    run({})
                end)
            case:onClick(function()
                openAlarm(i)
            end)
    end

    local add = gui:box(win, 250, 410, 40, 40)
    add:setBackgroundColor(COLOR_DARK)
    add:setRadius(20)
    
        local icon_plus = gui:image(add, "plus.png", 14, 14, 12, 12, COLOR_DARK)

    add:onClick(function()
        openAlarm(-1)
    end)

    gui:setWindow(win)
end