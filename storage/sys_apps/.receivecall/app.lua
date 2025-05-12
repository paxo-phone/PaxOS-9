-- recevoir un appel

function checkHangOn()
    if(gsm.getCallState() == IDLE) then
        gui:setWindow(nil)
    end
end

function quit()
    gsm:endCall()
end

function callPage()
    print("call page")

    win2=gui:window()

    num=gui:label(win2, 16, 83, 288, 41)
    num:setHorizontalAlignment(CENTER_ALIGNMENT)
    num:setFontSize(36)
    num:setText(gsm.getNumber())

    hangOnB=gui:label(win2, 35, 394, 250, 38)
    hangOnB:setHorizontalAlignment(CENTER_ALIGNMENT)
    hangOnB:setText("Quitter pour raccrocher")

    gui:setWindow(win2)
end

function run(arg)
    win=gui:window()

    num=gui:label(win, 16, 175, 288, 41)
    num:setHorizontalAlignment(CENTER_ALIGNMENT)
    num:setFontSize(36)

    local c = gsm.getContactByNumber(gsm.getNumber())
    if(c.name ~= "") then
        num:setText(c.name)
    else
        num:setText(gsm.getNumber())
    end

    state=gui:label(win, 16, 219, 288, 28)
    state:setHorizontalAlignment(CENTER_ALIGNMENT)
    state:setFontSize(24)
    state:setTextColor(COLOR_GREY)
    state:setText("Appel...")

    accept=gui:button(win, 35, 344, 250, 38)
    accept:setText("Accepter")
    accept:onClick(function() gsm:acceptCall() callPage() end)

    refuse=gui:button(win, 35, 394, 250, 38)
    refuse:setText("Refuser")
    refuse:onClick(function() gsm:rejectCall() end)
    
    hangonevent = time:setInterval(checkHangOn, 200)

    gui:setWindow(win)
end