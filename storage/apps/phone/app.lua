garg = nil
local win = nil
win2 = nil

function hangOn()
    print("hang on")
    -- Si garg est défini et vide, on revient à la fenêtre principale
    if garg ~= nil and #garg == 0 then
        gui:setWindow(win)
    end
end

function checkHangOn()
    local callState = gsm.getCallState()
    print("hang on check " .. tostring(callState))
    if callState == 0 then
        hangOn()
    else
        time:setTimeout(checkHangOn, 500)
    end
end

function quit()
    gsm:endCall()
    if win2 then
        gui:setWindow(win)
        win2 = nil
    end
end

function callPage()
    print("call page")
    if win2 ~= nil then
        gui:setWindow(win2)
        time:setTimeout(checkHangOn, 500)
        return
    end

    win2 = gui:window()

    local numLabel = gui:label(win2, 16, 83, 288, 41)
    numLabel:setHorizontalAlignment(CENTER_ALIGNMENT)
    numLabel:setFontSize(36)
    numLabel:setText(gsm.getNumber())

    local hangOnButton = gui:button(win2, 35, 394, 250, 38)
    hangOnButton:setText("Raccrocher")
    hangOnButton:onClick(function()
        quit()
    end)

    time:setTimeout(checkHangOn, 0)

    gui:setWindow(win2)
    print("end callPage")
end

function run(arg)
    garg = arg
    print("start call " .. tostring(#garg))

    -- Si un seul numéro est fourni, lancer l'appel directement
    if garg ~= nil and #garg == 1 then
        gsm.newCall(garg[1])
        print("Success")
        callPage()
        return
    end

    -- Si la fenêtre principale existe, l'afficher
    if win ~= nil then
        gui:setWindow(win)
        return
    end

    -- Créer la fenêtre principale
    win = gui:window()

    local inputLabel = gui:label(win, 16, 83, 288, 41)
    inputLabel:setHorizontalAlignment(CENTER_ALIGNMENT)
    inputLabel:setFontSize(36)
    inputLabel:setText("+33")

    local buttons = {}

    -- Création des boutons 1 à 9
    for row = 1, 3 do
        for col = 1, 3 do
            local num = (row - 1) * 3 + col
            buttons[num] = gui:label(win, 51 + 72 * (col - 1), 174 + 68 * (row - 1), 72, 68)
            buttons[num]:setVerticalAlignment(CENTER_ALIGNMENT)
            buttons[num]:setHorizontalAlignment(CENTER_ALIGNMENT)
            buttons[num]:setText(tostring(num))
            buttons[num]:setFontSize(24)

            buttons[num]:onClick(function()
                inputLabel:setText(inputLabel:getText() .. tostring(num))
            end)
        end
    end

    -- Bouton 0
    buttons[0] = gui:label(win, 123, 378, 72, 68)
    buttons[0]:setText("0")
    buttons[0]:setVerticalAlignment(CENTER_ALIGNMENT)
    buttons[0]:setHorizontalAlignment(CENTER_ALIGNMENT)
    buttons[0]:setFontSize(24)

    buttons[0]:onClick(function()
        inputLabel:setText(inputLabel:getText() .. "0")
    end)

    -- Bouton Effacer
    buttons["clear"] = gui:label(win, 51, 378, 72, 68)
    buttons["clear"]:setText("Effacer")
    buttons["clear"]:setVerticalAlignment(CENTER_ALIGNMENT)
    buttons["clear"]:setHorizontalAlignment(CENTER_ALIGNMENT)
    buttons["clear"]:setFontSize(16)

    buttons["clear"]:onClick(function()
        inputLabel:setText("+33")
    end)

    -- Bouton Appeler
    local callButton = gui:label(win, 195, 378, 72, 68)
    callButton:setText("Appeler")
    callButton:setVerticalAlignment(CENTER_ALIGNMENT)
    callButton:setHorizontalAlignment(CENTER_ALIGNMENT)
    callButton:setFontSize(16)

    callButton:onClick(function()
        local numberToCall = inputLabel:getText()
        if numberToCall and numberToCall ~= "" and numberToCall ~= "+33" then
            gsm.newCall(numberToCall)
            print("Success")
            callPage()
        else
            print("Numéro invalide")
        end
    end)

    gui:setWindow(win)
end
