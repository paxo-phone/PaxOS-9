require("perso.lua")

-- define heights for the different sections

boxHeight_base = 55
boxHeight_base_text = 78
boxHeight_base_button = 94

function createBox(parent, y, h)
    local box = gui:box(parent, 30, 118 + y, 260, h)
    box:setBackgroundColor(0xf79e)
    box:setRadius(12)

    return box
end

function pinSettings()
    win3 = gui:window()

    local backbox = gui:box(win3, 19, 19, 166, 27)
    local icon = gui:image(backbox, "back.png", 0, 3, 18, 18)
    local text = gui:label(backbox, 25, 0, 166, 27)
    text:setFontSize(20)
    text:setVerticalAlignment(CENTER_ALIGNMENT)
    text:setText("Réseau")
    backbox:onClick(function()
        gui:setWindow(win2)
        gui:del(win3)
    end)

    local title = gui:label(win3, 88, 55, 143, 42)
    title:setFontSize(36)
    title:setHorizontalAlignment(CENTER_ALIGNMENT)
    title:setVerticalAlignment(CENTER_ALIGNMENT)
    title:setText("Code PIN")

    local pinLabel = gui:label(win3, 88, 120, 143, 42)
    pinLabel:setFontSize(36)
    pinLabel:setHorizontalAlignment(CENTER_ALIGNMENT)
    pinLabel:setVerticalAlignment(CENTER_ALIGNMENT)
    pinLabel:setText("")

    local pin = ""

    local function addDigit(digit)
        if #pin < 8 then
            pin = pin .. digit
            pinLabel:setText(string.rep("*", #pin))
        end
    end

    local function deleteDigit()
        if #pin > 0 then
            pin = string.sub(pin, 1, #pin - 1)
            pinLabel:setText(string.rep("*", #pin))
        end
    end

    local function submitPin()
        settings.network.setSimPin(pin, function(success)
            if success then
                gui:setWindow(win2)
                gui:del(win3)
            else
                pinLabel:setText("PIN incorrect")
                pin = ""
            end
        end)
    end

    -- Create a table for the buttons
    buttons = {}

    -- Create a loop to generate the number buttons
    for row = 1, 3 do
        for col = 1, 3 do
            local num = (row - 1) * 3 + col
            buttons[num] = gui:label(win3, 51 + 72 * (col - 1), 174 + 68 * (row - 1), 72, 68)
            buttons[num]:setVerticalAlignment(CENTER_ALIGNMENT)
            buttons[num]:setHorizontalAlignment(CENTER_ALIGNMENT)
            buttons[num]:setText(tostring(num))
            buttons[num]:setFontSize(24)

            -- Add onClick event to each button
            buttons[num]:onClick(function()
                addDigit(tostring(num))
            end)
        end
    end

    -- Create the 0 button
    buttons[0] = gui:label(win3, 123, 378, 72, 68)
    buttons[0]:setText("0")
    buttons[0]:setVerticalAlignment(CENTER_ALIGNMENT)
    buttons[0]:setHorizontalAlignment(CENTER_ALIGNMENT)
    buttons[0]:setFontSize(24)

    -- Add onClick event to the 0 button
    buttons[0]:onClick(function()
        addDigit("0")
    end)

    buttons[11] = gui:label(win3, 51, 378, 72, 68)
    buttons[11]:setText("Effacer")
    buttons[11]:setVerticalAlignment(CENTER_ALIGNMENT)
    buttons[11]:setHorizontalAlignment(CENTER_ALIGNMENT)
    buttons[11]:setFontSize(16)

    buttons[11]:onClick(function()
        deleteDigit()
    end)

    call = gui:label(win3, 195, 378, 72, 68)
    call:setText("Valider")
    call:setVerticalAlignment(CENTER_ALIGNMENT)
    call:setHorizontalAlignment(CENTER_ALIGNMENT)
    call:setFontSize(16)

    call:onClick(function()
        submitPin()
    end)

    gui:setWindow(win3)
end

function networkSettings()
    win2 = gui:window()

    local backbox = gui:box(win2, 19, 19, 166, 27)
        local icon = gui:image(backbox, "back.png", 0, 3, 18, 18)
        local text = gui:label(backbox, 25, 0, 166, 27)
            text:setFontSize(20)
            text:setVerticalAlignment(CENTER_ALIGNMENT)
            text:setText("Paramètres")
        backbox:onClick(function()
            gui:setWindow(win)
            gui:del(win2)
        end)


    local title = gui:label(win2, 88, 55, 143, 42)
    title:setFontSize(36)
    title:setHorizontalAlignment(CENTER_ALIGNMENT)
    title:setVerticalAlignment(CENTER_ALIGNMENT)
    title:setText("Réseau")

    -- create the boxes
    local box4G = createBox(win2, 0, boxHeight_base)
        local text4G = gui:label(box4G, 16, 0, 177, 55)
            text4G:setFontSize(24)
            text4G:setVerticalAlignment(CENTER_ALIGNMENT)
            text4G:setText("4G")
        local buttonPin4G = gui:button(box4G, 5, 49, 250, 38)
            buttonPin4G:setText("Code Pin de la carte SIM")
            buttonPin4G:disable()
            buttonPin4G:onClick(pinSettings)

        local switch4G = gui:switch(box4G, 201, 17)
            switch4G:setState(settings.network.isCellularEnabled())
            switch4G:onClick(function ()
                settings.network.setCellularEnabled(switch4G:getState())
                if(settings.network.isSimPinRequired()) then
                    if(switch4G:getState()) then
                        box4G:setHeight(boxHeight_base_button)
                        buttonPin4G:enable()
                    else
                        box4G:setHeight(boxHeight_base)
                        buttonPin4G:disable()
                    end
                end
            end)
    
    local boxWifi = createBox(win2, 100, boxHeight_base)
        local textWifi = gui:label(boxWifi, 16, 0, 177, 55)
            textWifi:setFontSize(24)
            textWifi:setVerticalAlignment(CENTER_ALIGNMENT)
            textWifi:setText("Wifi")

        local switchWifi = gui:switch(boxWifi, 201, 17)
            switchWifi:setState(settings.network.isWifiEnabled())
            switchWifi:onClick(function ()
                if(switchWifi:getState()) then
                    settings.network.enableWifi()
                else
                    settings.network.disableWifi()
                end

                if(not switchWifi:getState()) then
                    if(wifiList) then
                        gui:del(wifiList)
                        wifiList = nil
                    end
                    boxWifi:setHeight(boxHeight_base)
                end
            end)
        
        boxWifi:onClick(function()
            if(not switchWifi:getState()) then
                return
            end

            if(wifiList ~= nil) then
                gui:del(wifiList)
                wifiList = nil
            end
            
            -- show the list of available networks
            local wifi_list = settings.network.getAvailableWifiSSID()
            local nbWifi = #wifi_list
            local connectedWifi = settings.network.getConnectedWifi()

            wifiList = gui:vlist(boxWifi, 24, 60, 211, nbWifi*30)
            wifiList:setSpaceLine(0)
                for i, value in pairs(wifi_list) do
                    local case = gui:box(wifiList, 0, 0, 211, 30)
                        local text = gui:label(case, 0, 0, 211, 30)
                            text:setText(value)
                            text:setFontSize(20)
                        
                        case:onClick(function()
                            gui:keyboard("Mot de passe pour " .. value, "", function(password)
                                if password ~= "" then
                                    settings.network.connectWifi(value, password)
                                end
                            end)
                        end)

                        if(value == connectedWifi) then
                            local img = gui:image(case, "yes.png", 181, 0, 30, 30)
                            img:setTransparentColor(0xFFFF)
                        end
                end
            boxWifi:setHeight(boxHeight_base_text + nbWifi*30)
        end)
        

    gui:setWindow(win2)
end

function systemSettings()
    win2 = gui:window()

    local backbox = gui:box(win2, 19, 19, 166, 27)
        local icon = gui:image(backbox, "back.png", 0, 3, 18, 18)
        local text = gui:label(backbox, 25, 0, 166, 27)
            text:setFontSize(20)
            text:setVerticalAlignment(CENTER_ALIGNMENT)
            text:setText("Paramètres")
        backbox:onClick(function()
            gui:setWindow(win)
            gui:del(win2)
        end)

    local title = gui:label(win2, 88, 55, 143, 42)
        title:setFontSize(36)
        title:setHorizontalAlignment(CENTER_ALIGNMENT)
        title:setVerticalAlignment(CENTER_ALIGNMENT)
        title:setText("Système")

    -- create the boxes
    local brightnessBox = createBox(win2, 0, 73)
        local textBrightness = gui:label(brightnessBox, 16, 0, 177, 55)
            textBrightness:setFontSize(24)
            textBrightness:setVerticalAlignment(CENTER_ALIGNMENT)
            textBrightness:setText("Luminosité")
        
        local brightnessSlider = gui:slider(brightnessBox, 15, 53, 230, 10, 5, 255, settings.getBrightness())
            brightnessSlider:onChange(function() 
                settings.setBrightness(brightnessSlider:getValue(), false)
            end)

    local sleepTimeBox = createBox(win2, 100, 73)
        local textsleepTime = gui:label(sleepTimeBox, 16, 0, 177, 50)
            textsleepTime:setFontSize(24)
            textsleepTime:setVerticalAlignment(CENTER_ALIGNMENT)
            textsleepTime:setText("Veille: " .. int(settings.getStandBySleepTime()/1000) .. "s")
        print("sleep time: " .. settings.getStandBySleepTime())
        local sleepTimeSlider = gui:slider(sleepTimeBox, 15, 53, 230, 10, 20, 120, int(settings.getStandBySleepTime()/1000))
            sleepTimeSlider:onChange(function() 
                settings.setStandBySleepTime(sleepTimeSlider:getValue()*1000, true)
                textsleepTime:setText("Veille: " .. int(sleepTimeSlider:getValue()) .. "s")
            end)

    gui:setWindow(win2)
end

function PaxOSSettings()

    win2 = gui:window()

    local OS_VERSION_MAJOR = 9
    local OS_VERSION_MINOR = 0
    local OS_VERSION_PATCH = 0
    local OS_VERSION_BUILD = 0

    local OS_VERSION_CODENAME = "Red Panic"

    local Licence = "AGPL-3.0"

    local OS_VERSION = OS_VERSION_MAJOR .. "." .. OS_VERSION_MINOR .. "." .. OS_VERSION_PATCH .. "-" .. OS_VERSION_BUILD

    local backbox = gui:box(win2, 19, 19, 166, 27)
        local icon = gui:image(backbox, "back.png", 0, 3, 18, 18)
        local text = gui:label(backbox, 25, 0, 166, 27)
            text:setFontSize(20)
            text:setVerticalAlignment(CENTER_ALIGNMENT)
            text:setText("Paramètres")
        backbox:onClick(function()
            gui:setWindow(win)
            gui:del(win2)
        end)

    local title = gui:label(win2, 88, 85, 143, 42)
    title:setFontSize(36)
    title:setHorizontalAlignment(CENTER_ALIGNMENT)
    title:setVerticalAlignment(CENTER_ALIGNMENT)
    title:setText("PaxOS-9")

    local versionLabel = gui:label(win2, 88, 130, 143, 25)
    versionLabel:setFontSize(20)
    versionLabel:setHorizontalAlignment(CENTER_ALIGNMENT)
    versionLabel:setVerticalAlignment(CENTER_ALIGNMENT)
    versionLabel:setTextColor(COLOR_GREY)
    versionLabel:setText(OS_VERSION)

    local Debounce = true
    
    versionLabel:onClick(function()
        if Debounce == true then
            Debounce = false
            versionLabel:setText(OS_VERSION_CODENAME)

        elseif Debounce == false then
            Debounce = true
            versionLabel:setText(OS_VERSION)
        end
    end)

    local licenceLabel = gui:label(win2, 86, 155, 150, 25)
    licenceLabel:setFontSize(20)
    licenceLabel:setHorizontalAlignment(CENTER_ALIGNMENT)
    licenceLabel:setVerticalAlignment(CENTER_ALIGNMENT)
    licenceLabel:setTextColor(COLOR_GREY)
    licenceLabel:setText("Licence : " .. Licence)

    local updateLabel = gui:label(win2, 20, 240, 280, 100)
    updateLabel:setFontSize(18)
    updateLabel:setHorizontalAlignment(CENTER_ALIGNMENT)
    updateLabel:setVerticalAlignment(CENTER_ALIGNMENT)
    updateLabel:setTextColor(COLOR_GREY)
    updateLabel:setText("Merci de vous rendre sur paxo.fr/flash (avec Chrome) pour rechercher des mises à jour disponible.")

    local infoLabel = gui:label(win2, 20, 370, 280, 80)
    infoLabel:setFontSize(22)
    infoLabel:setHorizontalAlignment(CENTER_ALIGNMENT)
    infoLabel:setVerticalAlignment(CENTER_ALIGNMENT)
    infoLabel:setText("Pour toute autre information rendez-vous sur Paxo.fr ")

    gui:setWindow(win2)
end

function run() -- create the menu
    win = gui:window()

    local title = gui:label(win, 65, 28, 189, 40)
    title:setFontSize(36)
    title:setHorizontalAlignment(CENTER_ALIGNMENT)
    title:setText("Paramètres")

    -- create the boxes
    local box_network = gui:box(win, 30, 118, 260, 55, boxHeight_base)
        local text_network = gui:label(box_network, 16, 0, 190, 55)
            text_network:setFontSize(30)
            text_network:setVerticalAlignment(CENTER_ALIGNMENT)
            text_network:setText("Réseau")
        local img = gui:image(box_network, "r_arrow.png", 219, 15, 25, 25)
        box_network:onClick(networkSettings)

    local box_system = gui:box(win, 30, 173, 260, 55, boxHeight_base)
        local text_system = gui:label(box_system, 16, 0, 190, 55)
            text_system:setFontSize(30)
            text_system:setVerticalAlignment(CENTER_ALIGNMENT)
            text_system:setText("Système")
        local img = gui:image(box_system, "r_arrow.png", 219, 15, 25, 25)
        box_system:onClick(systemSettings)

    local box_security = gui:box(win, 30, 228, 260, 55, boxHeight_base)
        local text_security = gui:label(box_security, 16, 0, 190, 55)
            text_security:setFontSize(30)
            text_security:setVerticalAlignment(CENTER_ALIGNMENT)
            text_security:setText("Sécurité")
        local img = gui:image(box_security, "r_arrow.png", 219, 15, 25, 25)
        box_security:onClick(securitySettings)

    local box_personalization = gui:box(win, 30, 283, 260, 55, boxHeight_base)
        local text_personalization = gui:label(box_personalization, 16, 0, 190, 55)
            text_personalization:setFontSize(30)
            text_personalization:setVerticalAlignment(CENTER_ALIGNMENT)
            text_personalization:setText("Personnalisation")
        local img = gui:image(box_personalization, "r_arrow.png", 219, 15, 25, 25)
        box_personalization:onClick(initColorScreen)
    
    local box_PaxOS = gui:box(win, 30, 338, 260, 55, boxHeight_base)
        local text_PaxOS = gui:label(box_PaxOS, 16, 0, 190, 55)
            text_PaxOS:setFontSize(30)
            text_PaxOS:setVerticalAlignment(CENTER_ALIGNMENT)
            text_PaxOS:setText("PaxOS")
        local img = gui:image(box_PaxOS, "r_arrow.png", 219, 15, 25, 25)
        box_PaxOS:onClick(PaxOSSettings)
    
    gui:setWindow(win)
end



local oldWin
function manageWindow()

    local win

    win = gui:window()
    gui:setWindow(win)

    if oldWin then 
        gui:del(oldWin) 
        oldWin  =nil 
    end
    
    oldWin = win
    return win
end
