-- recevoir un appel

function hangOn()
    gui:setWindow(nil)
    print("hang on")
end

function checkHangOn()
    print("hang on check" .. tostring(gsm.getCallState()))
    if (gsm.getCallState() == 0) then
        hangOn()
    else
        time:setTimeout(checkHangOn, 500)
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

    hangOnB=gui:button(win2, 35, 394, 250, 38)
    hangOnB:setText("Raccrocher")
    --hangOnB:onClick(function() gsm:endCall() end)

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
    
    time:setTimeout(checkHangOn, 5000)

    gui:setWindow(win)
end