garg = nil



local win = nil   -- Fenêtre principale
local win2 = nil  -- Fenêtre d'appel en cours (pour appels


function hangOn()
    if(garg ~= nil) then
        gui:setWindow(win)
    end
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
    if win2 ~= nil then
        gui:setWindow(win2)
        time:setTimeout(checkHangOn, 500)
        return
    end

    win2=gui:window()

    num=gui:label(win2, 16, 83, 288, 41)
    num:setHorizontalAlignment(CENTER_ALIGNMENT)
    num:setFontSize(36)
    num:setText(gsm.getNumber())

    hangOnB=gui:button(win2, 35, 394, 250, 38)
    hangOnB:setText("Raccrocher")
    --hangOnB:onClick(function() gsm:endCall() if(#garg == 1) then gui:setWindow(nil) end end)
    
    time:setTimeout(checkHangOn, 0)

    gui:setWindow(win2)
    print("end callpage")
end

function findContactInListByNumber(phoneNumber)
    local contactList = gsm:listContacts()
    for i, contact in ipairs(contactList) do
        if contact.phone == phoneNumber then
            return contact
        end
    end
    return nil
end

function addContactSecurely(contactToAdd)

    if not contactToAdd or not contactToAdd.phone or not contactToAdd.name then
        print("Erreur : Les données du contact sont incomplètes")
        return false
    end

    print(string.format("Tentative d'ajout du contact : %s (%s)", contactToAdd.name, contactToAdd.phone))

    local c = Contact:new()
    c.name=contactToAdd.name
    c.phone=contactToAdd.phone


    gsm.addContact(c)
    gsm.saveContacts()
    print(string.format("Contact '%s' (%s) ajouté avec succès.", contactToAdd.name, contactToAdd.phone))
    
    
    return true
end

function getDate()

    local today = time:get("y,mo,d,h,mi,s")
    local year = today[1]
    local month = string.format("%02d", today[2])
    local day = string.format("%02d", today[3])
    local heure = string.format("%02d", today[4])
    local minute = string.format("%02d", today[5])

    return day.."/"..month.."/"..year.." "..heure..":"..minute
end

function saveNumberToHistorical(phoneNumber)
    if phoneNumber == "+33" or phoneNumber == nil or phoneNumber == "" then
        print("Aucun numéro valide à enregistrer.")
        return
    end

    print("Tentative d'enregistrement du numéro : " .. phoneNumber)

    local filename = "historical.json"
    local records = {}

    local loadedData = loadTable(filename)

    if loadedData and type(loadedData) == "table" and loadedData.records then
        if type(loadedData.records) == "table" then
            records = loadedData.records
        else
            print("Le fichier JSON existe mais 'records' n'est pas une table. Réinitialisation.")
        end
    end

    local currentDateTime = getDate()
    local foundExistingRecord = false

    for i, record in ipairs(records) do
        if record.number == phoneNumber then
            record.date = currentDateTime
            record.count = (record.count or 1) + 1
            foundExistingRecord = true
            print(string.format("Numéro '%s' mis à jour (appel #%d) à la date '%s'.", phoneNumber, record.count, currentDateTime))
            break
        end
    end

    if not foundExistingRecord then
        local newRecord = {
            number = phoneNumber,
            date = currentDateTime,
            count = 1
        }
        table.insert(records, newRecord)
        print("Nouveau numéro '" .. phoneNumber .. "' enregistré avec sa première date '" .. currentDateTime .. "'.")
    end

    local dataToSave = {
        records = records
    }

    saveTable(filename, dataToSave)
    print("Historique sauvegardé avec les mises à jour.")
end

function clearCallHistory()
    local filename = "historical.json"
    local dataToSave = {
        records = {}
    }
    saveTable(filename, dataToSave)

    print("Tout l'historique d'appels a été effacé de " .. filename .. ".")
end

function openNumber(record)
    win4 = gui:window()

    local backbox = gui:box(win4, 19, 19, 166, 27)
        local icon = gui:image(backbox, "back.png", 0, 3, 18, 18)
        local text = gui:label(backbox, 25, 0, 166, 27)
            text:setFontSize(20)
            text:setVerticalAlignment(CENTER_ALIGNMENT)
            text:setText("Historique")
        backbox:onClick(function()
            gui:setWindow(win3)
        end)

    local title = gui:label(win4, 0, 78, 320, 42)
    title:setFontSize(26)
    title:setHorizontalAlignment(CENTER_ALIGNMENT)
    title:setVerticalAlignment(CENTER_ALIGNMENT)
    title:setText(record.number)
    gui:setWindow(win4)

    local Datelabel = gui:label(win4, 53, 115, 210, 20)
    Datelabel:setHorizontalAlignment(CENTER_ALIGNMENT)
    Datelabel:setFontSize(16)
    Datelabel:setText(record.date)

    local contactInfoLabel = gui:label(win4, 53, 135, 210, 28)
    contactInfoLabel:setHorizontalAlignment(CENTER_ALIGNMENT)
    contactInfoLabel:setFontSize(16)

    local nameInput = gui:input(win4, 35, 180, 250, 40)
    nameInput:setTitle("Nom du nouveau contact")
    nameInput:setText("")
    nameInput:onClick(function () 
        gui:keyboard("contact", nameInput:getText(),
            function (n)
                nameInput:setText(n)
            end)
    end)

    -- Bouton "Ajouter aux contacts"
    local addContactButton = gui:button(win4, 35, 294, 250, 38)
    addContactButton:setText("Ajouter aux contacts")
    addContactButton:disable() -- Désactivé par défaut

    -- Fonction qui vérifie le contact et met à jour l'affichage
    local function findContactAndDisplay()
        local contact = findContactInListByNumber(record.number)
        print("Recherche du contact pour le numéro : " .. record.number)

        if contact then
            print("Contact trouvé : " .. contact.name)
            contactInfoLabel:setText("Contact enregistré : " .. contact.name)
            addContactButton:disable() 
            nameInput:disable() 
        else
            print("Aucun contact trouvé pour le numéro : " .. record.number)
            contactInfoLabel:setText("Aucun contact enregistré.")
            addContactButton:enable()
            nameInput:enable() 
        end
    end

    addContactButton:onClick(function()
        local contactNameInput = nameInput:getText()
        
        if contactNameInput == "" then
            gui:showInfoMessage("Veuillez entrer un nom pour le contact.")
            return
        end

        local contactToAdd = { name = contactNameInput, phone = record.number }

         -- Utilisation de la fonction sécurisée
        if addContactSecurely(contactToAdd) then
            gui:showInfoMessage("Contact ajouté : " .. contactToAdd.name .. " " .. contactToAdd.phone)
            findContactAndDisplay() -- Rafraîchir l'affichage après l'ajout
        else
            gui:showInfoMessage("Impossible d'ajouter le contact : il existe déjà.")
        end
    end)

    local messages = gui:button(win4, 35, 344, 250, 38)
    messages:setText("Messages")
    messages:onClick(function () launch("messages", {record.number}) end)

    local call = gui:button(win4, 35, 394, 250, 38)
    call:setText("Appeler")
    call:onClick(function ()
        gsm.newCall(record.number) 
        callPage() 
        saveNumberToHistorical(record.number)
    end)

    findContactAndDisplay() 
    gui:setWindow(win4)
end

function HistoricalPage()
    print("historical page")

    if win3 ~= nil then
        gui:setWindow(win3)
        refreshHistorical()
        return
    end

    win3 = gui:window()
    
    local backbox = gui:box(win3, 19, 19, 166, 27)
        local icon = gui:image(backbox, "back.png", 0, 3, 18, 18)
        local text = gui:label(backbox, 25, 0, 166, 27)
            text:setFontSize(20)
            text:setVerticalAlignment(CENTER_ALIGNMENT)
            text:setText("Numéroteur")
        backbox:onClick(function()
            gui:setWindow(win) 
        end)

    local title = gui:label(win3, 19, 55, 143, 42)
    title:setFontSize(36)
    title:setHorizontalAlignment(CENTER_ALIGNMENT)
    title:setVerticalAlignment(CENTER_ALIGNMENT)
    title:setText("Historique")
    
    local ClearButton = gui:button(win3, 215, 60, 85, 35)
    ClearButton:setText("Effacer")
    ClearButton:onClick(function()
        clearCallHistory() 
        refreshHistorical() 
        print("Historique d'appels effacé et mis à jour.")
    end)

    local listO = gui:vlist(win3, 19, 110, 282, 340)
    listO:setSpaceLine(2)

    function refreshHistorical()

        print("Rafraîchissement de l'historique des appels...")

        listO:clear()

        local filename = "historical.json"
        local storedData = loadTable(filename)
        local records = {}
        

        if storedData and type(storedData) == "table" and storedData.records then
            if type(storedData.records) == "table" and #storedData.records > 0 then
                records = storedData.records
            else
                local emptyMessage = gui:label(listO, 0, 0, 282, 36)
                emptyMessage:setText("Aucun appel dans l'historique.")
                emptyMessage:setFontSize(16)
                emptyMessage:setHorizontalAlignment(CENTER_ALIGNMENT)
                emptyMessage:setVerticalAlignment(CENTER_ALIGNMENT)
            return
                print("L'historique est vide ou le fichier historical.json est mal formé.")
            end
        else
            print("L'historique est vide ou le fichier historical.json est manquant ou mal formé.")
            return
        end

        print("\nChargement du contenu de historical.json dans l'interface :")
        for i, record in ipairs(records) do
            if record.number and record.date and record.count then
                print(string.format("Numéro : %s (x%d), Date : %s", record.number, record.count, record.date))
                
                local case = gui:box(listO, 0, 0, 282, 50) 
                
                local display_number_text = record.number
                if record.count > 1 then
                    display_number_text = string.format("%s (%d)", record.number, record.count)
                end

                local numberLabel = gui:label(case, 10, 5, 251, 18) 
                numberLabel:setText(display_number_text)
                numberLabel:setFontSize(18)
                numberLabel:setVerticalAlignment(TOP_ALIGNMENT)
                
                local dateLabel = gui:label(case, 10, 23, 251, 16) 
                dateLabel:setText(record.date)
                dateLabel:setFontSize(14) 
                dateLabel:setVerticalAlignment(TOP_ALIGNMENT)
                dateLabel:setTextColor(COLOR_GREY) 
                                                
                case:onClick(function()
                    openNumber(record)
                end)
                
                local icon = gui:image(case, "arrowr.png", 250, 12, 20, 20) 
            else
                print("Enregistrement malformé trouvé dans historical.json.")
            end
        end
    end
    
    refreshHistorical()

    gui:setWindow(win3) 
end

function run(arg)
    garg = arg
    print("start call " .. #garg)
    if(#garg == 1) then
        gsm.newCall(garg[1])
        print("Success")
        callPage()
        return
    end

    if win ~= nil then
        gui:setWindow(win)
        if i:getText() == "" then
             i:setText("+33") -- Commence vide
        end
        return
    end

    win=gui:window()

    i=gui:label(win, 16, 83, 288, 41)
    i:setHorizontalAlignment(CENTER_ALIGNMENT)
    i:setFontSize(36)
    i:setText("+33") 

    -- Create a table for the buttons
    buttons = {}

    -- Create a loop to generate the number buttons (1-9)
    for row = 1, 3 do
        for col = 1, 3 do
            local num = (row - 1) * 3 + col
            buttons[num] = gui:label(win, 51 + 72 * (col - 1), 174 + 68 * (row - 1), 72, 68)
            buttons[num]:setVerticalAlignment(CENTER_ALIGNMENT)
            buttons[num]:setHorizontalAlignment(CENTER_ALIGNMENT)
            buttons[num]:setText(tostring(num))
            buttons[num]:setFontSize(24)

            -- Add onClick event to each button
            buttons[num]:onClick(function()
                i:setText(i:getText() .. num)
            end)
        end
    end

    -- Create the 0 button
    buttons[0] = gui:label(win, 123, 378, 72, 68)
    buttons[0]:setText("0")
    buttons[0]:setVerticalAlignment(CENTER_ALIGNMENT)
    buttons[0]:setHorizontalAlignment(CENTER_ALIGNMENT)
    buttons[0]:setFontSize(24)

    -- Add onClick event to the 0 button
    buttons[0]:onClick(function()
        i:setText(i:getText() .. "0")
    end)

    buttons[11] = gui:label(win, 51, 378, 72, 68) 
    buttons[11]:setText("-") 
    buttons[11]:setVerticalAlignment(CENTER_ALIGNMENT)
    buttons[11]:setHorizontalAlignment(CENTER_ALIGNMENT)
    buttons[11]:setFontSize(24)

    buttons[11]:onClick(function()
        i:setText(i:getText() .. "-") 
    end)

    
    
    local clearAllButton = gui:label(win, 115, 145, 90, 27)
    clearAllButton:setText("Effacer")
    clearAllButton:setFontSize(20)
    clearAllButton:setHorizontalAlignment(CENTER_ALIGNMENT)
    clearAllButton:setVerticalAlignment(CENTER_ALIGNMENT)
    clearAllButton:onClick(function()
        i:setText("+") -- Efface tout le texte
    end)


    local Historicalbox = gui:box(win, 19, 19, 166, 27)
        local icon = gui:image(Historicalbox, "menu.png", 0, 3, 18, 18)
        local text = gui:label(Historicalbox, 25, 0, 166, 27)
            text:setFontSize(20)
            text:setVerticalAlignment(CENTER_ALIGNMENT)
            text:setText("Historique")
        Historicalbox:onClick(function()
            HistoricalPage()
        end)

    call = gui:label(win, 195, 378, 72, 68)
    call:setText("Appeler")
    call:setVerticalAlignment(CENTER_ALIGNMENT)
    call:setHorizontalAlignment(CENTER_ALIGNMENT)
    call:setFontSize(16)

    call:onClick(function()
        local fullNumber = i:getText()
        
        -- Vérification si le numéro est vide avant d'appeler
        if fullNumber == "+" then
            gui:showInfoMessage("Veuillez entrer un numéro à appeler.")
            return
        end

        gsm.newCall(fullNumber)
        saveNumberToHistorical(fullNumber) -- Enregistre le numéro complet dans l'historique
        print("Appel réussi")
        callPage()
    end)

    gui:setWindow(win)
end