require "gestionDate.lua"
require "gestionDebug.lua"

local boxImgDay, boxImgWeek, boxImgMonth, boxImgParametre, boxImgAdd
local menu
local currentMode
local winDay, winWeek, winMonth, winNewEvent, winConfig
local oldWin


local affichageTop = 60

-- Parametrage du calendrier
local config = {}

-- liste d'event du calendrier
local PATH_DATA = "/data"
local data = {}



-- ---------------------------------------
-- FONCTIONS DU CALENDRIER
-- ---------------------------------------

-- Lancement de l'appication
function run()

    -- récupération de la date courante
    local today = getToday()
    year  = today[1]
    month = today[2]
    day   = today[3]

    -- chargement de la config de l'application
    loadConfig()
    
    -- lancement de la vue par défaut de l'application
    if (config.defaultView == "day") then
        displayDay (year, month, day)
    elseif config.defaultView == "week" then
        displayWeek(year, month, day)
    else 
        displayMonth(year, month)
    end
end -- run

-- chargement du fichier de configuration de l'application
function loadConfig()

    -- A remplacer par une lecture du fichier JSON
    -- Gererles erreurs Json et les valeurs par défaut
    local strFilename ="config.json"
    local fileConfig = storage:file (strFilename, READ)
    local configStr

    if fileConfig and storage:isFile(strFilename) then
        fileConfig:open()
        configStr = fileConfig:readAll()
        fileConfig:close()
        fileConfig = nil
    else
            configStr = '{"defaultView":"day","displayBusinessWeek":false,"displayWeekNum":false,"day":{"heureDebut":8, "heureFin":17}}'
        end
    local json_obj = Json:new(configStr)

    if json_obj:has_key("defaultView") then
        config.defaultView = json_obj:get_string("defaultView")
    else
        config.defaultView = "day" 
    end

    if json_obj:has_key("displayBusinessWeek") then
        config.displayBusinessWeek = json_obj:get_bool("displayBusinessWeek")
    else
        config.displayBusinessWeek = false 
    end

    if json_obj:has_key("displayWeekNum") then
        config.displayWeekNum = json_obj:get_bool("displayWeekNum")
    else 
        config.displayWeekNum = true 
    end

    config.day = {}
    if (json_obj:has_key("day") and json_obj["day"]:has_key("heureDebut")) then
        config.day.heureDebut = json_obj["day"]:get_int("heureDebut")
    else
        config.day.heureDebut = 8
    end

    if (json_obj:has_key("day") and json_obj["day"]:has_key("heureFin")) then
            config.day.heureFin = json_obj["day"]:get_int("heureFin")
    else
        config.day.heureFin = 17
    end

    --debugPrint(config)

end

-- ---------------------------------------
-- AFFICHAGE ECRAN CONFIG
-- ----------------------

function displayConfig()

--    winConfig = gui:window()
  --  gui:setWindow (winConfig)
  print("displayConfig")

  winConfig = manageWindow()

    print("on a notre wonConfig")
    print(winConfig)
    local title=gui:label(winConfig, 50, 28, 144, 28)
    print("Affichage label OK")

    title:setFontSize(24)
    title:setText("Préférences")

    -- récupération de la date courante
    local today = getToday()

    local imgBack = gui:image(winConfig, "back.png", 20, 30, 18, 18)
    imgBack:onClick(function () switchScreen(today[1], today[2], today[3]) end)

    local lblWeekNum = gui:label(winConfig, 20, 100, 300, 30)
    lblWeekNum:setText("Afficher le n° des semaines")
    
    local chkWeekNum = gui:checkbox(lblWeekNum, 200, 0)
    chkWeekNum:setState(config.displayWeekNum)
    
    local lblWeekEnd = gui:label(winConfig, 20, 150, 300, 30)
    lblWeekEnd:setText("Masquer les week-ends")
    
    local chkWeekEnd = gui:checkbox(lblWeekEnd, 200, 0)
    chkWeekEnd:setState(config.displayBusinessWeek)

    local lblDefaultView = gui:label(winConfig, 20, 200, 300, 60)

    lblDefaultView:setText("Vue par défaut")
    local radioDefaultViewDay = gui:radio(lblDefaultView, 15, 30)
    local lblDefaultViewDay = gui:label(lblDefaultView, 40, 30, 60, 30)
    lblDefaultViewDay:setText("Journée")    

    local radioDefaultViewWeek = gui:radio(lblDefaultView, 110, 30)
    local lblDefaultViewWeek = gui:label(lblDefaultView, 135, 30, 60, 30)
    lblDefaultViewWeek:setText("Semaine")

    local radioDefaultViewMonth = gui:radio(lblDefaultView, 210, 30)
    local lblDefaultViewMonth = gui:label(lblDefaultView, 235, 30, 60, 30)
    lblDefaultViewMonth:setText("Mois")

    local groupRadio ={radioDefaultViewDay, radioDefaultViewWeek, radioDefaultViewMonth}
    radioDefaultViewDay:onClick(function() selectGroupRadio(radioDefaultViewDay, groupRadio) end)
    radioDefaultViewWeek:onClick(function() selectGroupRadio(radioDefaultViewWeek, groupRadio) end)
    radioDefaultViewMonth:onClick(function() selectGroupRadio(radioDefaultViewMonth, groupRadio) end)

    local lblDebutJour = gui:label(winConfig, 20, 280, 150, 20)
    lblDebutJour:setText("Début de journée")

    local lblFinJour = gui:label(winConfig, 160, 280, 150, 20)
    lblFinJour:setText("Fin de journée")

    local vLstHeureDebut = gui:vlist(winConfig, 20, 305, 150, 100)
    vLstHeureDebut:setSpaceLine(0)
    vLstHeureDebut:setSelectionFocus(SELECTION_CENTER)
    vLstHeureDebut:setSelectionColor(COLOR_LIGHT_ORANGE)
    vLstHeureDebut:setAutoSelect(true)

    local vLstHeureFin = gui:vlist(winConfig, 160, 305, 150, 100)
    vLstHeureFin:setSpaceLine(0)
    vLstHeureFin:setSelectionFocus(SELECTION_CENTER)
    vLstHeureFin:setSelectionColor(COLOR_LIGHT_ORANGE)
    vLstHeureFin:setAutoSelect(true)

    for i=0,23 do

        local lblHeureDebut = gui:label(vLstHeureDebut, 30, 0, 80, 18)
        local lblHeureFin = gui:label(vLstHeureFin, 30, 0, 80, 18)

        if i<10 then
            lblHeureDebut:setText("  "..tostring(i)..":00")
            lblHeureFin:setText("  "..tostring(i)..":00")
        else
            lblHeureDebut:setText(tostring(i)..":00")
            lblHeureFin:setText(tostring(i)..":00")
        end
        lblHeureDebut:setHorizontalAlignment(CENTER_ALIGNMENT)
        lblHeureFin:setHorizontalAlignment(CENTER_ALIGNMENT)
    end
    vLstHeureDebut:select(config.day.heureDebut)
    vLstHeureFin:select(config.day.heureFin)

    -- selection initiale des boutons radio
    if config.defaultView =="day" then
        selectGroupRadio(radioDefaultViewDay, groupRadio)
    elseif config.defaultView =="week" then
        selectGroupRadio(radioDefaultViewWeek, groupRadio)
    else
        selectGroupRadio(radioDefaultViewMonth, groupRadio)
    end

    local btnEnregistrer = gui:label(winConfig, 60, 420, 200, 30)
    btnEnregistrer:setRadius(10)
    btnEnregistrer:setBorderSize(1)
    btnEnregistrer:setBackgroundColor(COLOR_LIGHT_GREY)
    btnEnregistrer:setHorizontalAlignment(CENTER_ALIGNMENT)
    btnEnregistrer:setVerticalAlignment (CENTER_ALIGNMENT)
    btnEnregistrer:setText("Enregistrer")

    btnEnregistrer:onClick(function()

        config.displayWeekNum = chkWeekNum:getState()
        config.displayBusinessWeek = chkWeekEnd:getState()

        if radioDefaultViewDay:getState() then
            config.defaultView = "day"
        elseif radioDefaultViewWeek:getState() then
            config.defaultView = "week"
        else
            config.defaultView = "month"
        end
        --config.day={}
        config.day.heureFin = vLstHeureFin:getSelected()
        config.day.heureDebut = vLstHeureDebut:getSelected()
        
        -- sauvegarde de la config en fichier
        saveConfig()

        --retour à l'écran précédent 
        switchScreen(year, month, day)
    end)

end -- displayConfig

-- Gère la sélection unique d'un groupe de bouton radio
function selectGroupRadio(selected, groupRadio)
    if type(groupRadio) ~= "table" then
        return
    end
    for _, radio in ipairs(groupRadio) do
        radio:setState(radio == selected)
    end
end

-- sauvegarde la config en fichier json
function saveConfig()

    local fileConfig = storage:file ("config.json", WRITE)

    if (fileConfig == nil) then
        print("[saveConfig] error saving config file")
        return
    end

    local str_Json = array_to_json(config)
    fileConfig:open()
    fileConfig:write(str_Json)

    fileConfig:close()
    fileConfig = nil

    print("fin printConfig")

end

-- ---------------------------------------
-- AFFICHAGE D'UN MOIS
-- ---------------------------------------

function displayWeek(year, month, day)

    winWeek = manageWindow()
    currentMode = "week"

    displayTopBarre()
    -- sélection du menu actif
    selectMenu(boxImgWeek)

    -- chargement des données du mois 
    loadDataMonth(year, month)

    -- récupération de la date du jour
    local today = getToday()

    local numWeek = getWeekNum(year, month, day)
    local numFirstDay   = getDayOfWeek (year, month, day) -- jour de la semaine du jour à afficher

    local dateDebutSemaine = addDaysToDate({year, month, day}, 1 - numFirstDay)
    local dateFinSemaine = addDaysToDate({year, month, day}, 7 - numFirstDay)

    -- Affichage de la semaine courante
    local lblWeek = gui:label(winWeek, 20, affichageTop, 280, 35)
    lblWeek:setHorizontalAlignment(CENTER_ALIGNMENT)

    local strTitre = "Semaine "..tostring(numWeek).."  du "..tostring(dateDebutSemaine[3])
    if dateDebutSemaine[2] ~= dateFinSemaine[2] then
        strTitre = strTitre .. " "..getMonthShortName(dateDebutSemaine[2])
    end
    strTitre = strTitre .." au "..tostring(dateFinSemaine[3]).." "..getMonthShortName(dateFinSemaine[2])
    
    lblWeek:setText(strTitre)

    -- Affichage des fleches avant et aprés pour naviguer sur le mois précédent / suivant

    local datePreviousWeek = addDaysToDate({year ,month, day}, -7)
    local imgPreviousWeek = gui:image(winWeek, "fleche_gauche.png", 0, affichageTop, 20, 35)
    imgPreviousWeek:onClick(function() displayWeek(datePreviousWeek[1], datePreviousWeek[2], datePreviousWeek[3]) end)


    local dateNextWeek = addDaysToDate({year ,month, day}, 7)
    local imgNextWeek = gui:image(winWeek, "fleche_droite.png", 300, affichageTop, 20, 35)
    imgNextWeek:onClick(function() displayWeek(dateNextWeek[1], dateNextWeek[2], dateNextWeek[3]) end)

    local sizeListeHeures = 350
    local NbHeureToDisplay = config.day.heureFin - config.day.heureDebut +1
    
    -- récupération de l'heure actuelle
    local currentTime =  time:get("h")
    local currentHour = currentTime[1]

    local sizeBoxHeure = int(sizeListeHeures / NbHeureToDisplay)
    local widthBox = 300

    -- Gestion du header
    local espacementBox = 1        -- espacement entre les cellules
    local heigthHeader = 30         -- hauteur fixe pour le header
    local topHeader = affichageTop+ heigthHeader    -- décalage en haut pour l'affichage du header

    -- Définition de la taille du calendrier
    local heightDisplayMonth = 480 - topHeader - heigthHeader- espacementBox - 10--360  -- hauteur de la partie calendrier

    --local nbDaysInMonth = getDaysInMonth(year, month)   -- nombre de jour dans le mois
    --local nbWeeksInMonths = int((6+numFirstDay + nbDaysInMonth -1 )/7)  -- nombre de semaine à afficher pour le mois
    local widthBoxJour              -- largeur de la cellule jour
    local widthWeekNum = 30         -- largeur fixe de la partie num de semaine
    local leftMonth = 40                -- décalage à gauche pour l'affichage du lundi
    local decalageHeure = 10
    local widthDisplayMonth = 310 -  leftMonth  -- largeur de la partie calendrier

    local NbJourstoDisplay = 7      -- Nombre de jours à afficher (dépend de la config avec l'affichagfe ou non des week)end
    -- si config.displayBusinessWeek = true alors on n'affiche pas les week-end
    if config.displayBusinessWeek then NbJourstoDisplay = 5 end

    widthBoxJour = int((widthDisplayMonth- NbJourstoDisplay *espacementBox)/ NbJourstoDisplay)

    -- liste déroulante des heures
    local vListeHeure = gui:vlist(winWeek, decalageHeure, topHeader+heigthHeader+espacementBox, widthBox, sizeListeHeures)
    vListeHeure:setSpaceLine(0)
    vListeHeure:setBackgroundColor(COLOR_LIGHT_GREY)

    local lstBoxHeure = {}

    -- Affichage du header
    for j=1, NbJourstoDisplay do

        -- calcul de la date
        local decalageDays = j - numFirstDay
        local dateCellule = addDaysToDate({year, month, day}, decalageDays)

        -- récupération de la liste des événements
        local lstEvent = getDayEvents(dateCellule[1], dateCellule[2], dateCellule[3])

        local lblHeader = gui:label(winWeek, leftMonth + (j-1)*(widthBoxJour+espacementBox) , topHeader, widthBoxJour, heigthHeader)
        local colorHeader = COLOR_LIGHT_GREY
        if dateCellule[1] == today[1] and dateCellule[2] == today[2] and dateCellule[3] == today[3] then
            colorHeader = COLOR_LIGHT_ORANGE
        end
        lblHeader:setBackgroundColor(colorHeader)

        local headerJour = gui:label(lblHeader, 0, 0, widthBoxJour, 12)
        headerJour:setFontSize(12)
        headerJour:setBackgroundColor(colorHeader)
        headerJour:setHorizontalAlignment(CENTER_ALIGNMENT)
        headerJour:setText(getDayOfWeekShortName(j))

        local headerJourNum = gui:label(lblHeader, 0, 12, widthBoxJour, 18)
        headerJourNum:setFontSize(18)
        headerJourNum:setBackgroundColor(colorHeader)
        headerJourNum:setHorizontalAlignment(CENTER_ALIGNMENT)
        headerJour:setVerticalAlignment(CENTER_ALIGNMENT)
        headerJourNum:setText(tostring(dateCellule[3]))

        -- Affichage des heures
        for i=0,23 do
            local boxHour
            if not lstBoxHeure[i] then
                boxHour = gui:box(vListeHeure, 0, 0, widthBox+1, sizeBoxHeure)
                lstBoxHeure[i] = boxHour

                boxHour:setBorderColor(COLOR_LIGHT_GREY)
                boxHour:setBorderSize(1)
                boxHour:setRadius(1)

                local boxLabelHeure = gui:box(boxHour, 0, 0, leftMonth-decalageHeure, sizeBoxHeure)
                boxLabelHeure:setBorderColor(COLOR_LIGHT_GREY)
                boxLabelHeure:setBorderSize(1)
                boxLabelHeure:setRadius(1)

                local lblHeure = gui:label(boxLabelHeure, 0 , 1, 20, sizeBoxHeure-2)
                lblHeure:setHorizontalAlignment(RIGHT_ALIGNMENT)
                lblHeure:setFontSize(14)
                lblHeure:setText(tostring(i)..":")

                local lblMin = gui:label(boxLabelHeure, 20,1, 18, sizeBoxHeure-2)
                lblMin:setHorizontalAlignment(LEFT_ALIGNMENT)
                lblMin:setTextColor(COLOR_GREY)
                lblMin:setFontSize(10)
                lblMin:setText("00")
            else
                boxHour = lstBoxHeure[i]
            end

            local lblCase = gui:label(boxHour, leftMonth-decalageHeure+(j-1)*(widthBoxJour+espacementBox) , 0, widthBoxJour+espacementBox, sizeBoxHeure)
            lblCase:setBorderColor(COLOR_LIGHT_GREY)
            lblCase:setBorderSize(1)
            lblCase:setRadius(1)
            if config.day.heureDebut >i or config.day.heureFin < i then
                lblCase:setBackgroundColor(COLOR_LIGHT_BLUE)
            end
            -- ------------------------
            -- Affichage des evenements 
            -- ------------------------

            if lstEvent ~= nil then
                for _, ev in pairs(lstEvent) do

                    -- il y a un un event ou troncon d'event sur cette plage horaire
                    if i>= ev.debut.heure and (i < ev.fin.heure or ( ev.fin.heure==i and ev.fin.minute >0 )) then
                        local nomEvent = ""
                        -- gestion du positionnement du haut de l'event dans la case horaire
                        local positionHaut = 0
                        if ev.debut.heure == i then
                            positionHaut = int(ev.debut.minute * sizeBoxHeure / 60)+1
                            -- on met le nom de l'event sur le 1er "troncon"
                            nomEvent = ev.name  
                        elseif i > ev.debut.heure then
                            positionHaut = 0
                        end

                        -- gestion du positionnement du bas de l'event dans la case horaire
                        local positionBas = sizeBoxHeure-1
                        if ev.fin.heure == i then
                            positionBas = int((ev.fin.minute) * sizeBoxHeure / 60)-1
                        elseif ev.fin.heure > i then
                            positionBas = sizeBoxHeure
                        end
                        local lblEvent = gui:label(boxHour, leftMonth-decalageHeure+(j-1)*(widthBoxJour+espacementBox) , positionHaut, widthBoxJour, positionBas - positionHaut)

                        lblEvent:setBackgroundColor(COLOR_LIGHT_ORANGE)
                        lblEvent:setFontSize(14)
                        lblEvent:setText(" "..nomEvent)
                        lblEvent:onClick(
                            function()
                                local ev = getEventByID(ev.UID, dateCellule[1], dateCellule[2], dateCellule[3])
                                if not ev then ev = createEventObject (nil, dateCellule[1], dateCellule[2], dateCellule[3]) end
                                displayEvent(ev)
                            end                        
                        )
                    end
                end

            end -- if lstEvent
        end
        -- on positionne la vliste sur la 1ere heure du jour
        vListeHeure:setIndex(config.day.heureDebut)
    end

end -- displayWeek

-- ---------------------------------------
-- AFFICHAGE D'UN MOIS
-- ---------------------------------------


function displayMonth (year, month)

--    winMonth = gui:window()
--    gui:setWindow(winMonth)
  
    winMonth = manageWindow()

    currentMode = "month"

    displayTopBarre()
    -- sélection du menu actif
    selectMenu(boxImgMonth)

    -- chargement des données du mois 
    loadDataMonth(year, month)

    -- récupération de la date du jour
    local today = getToday()

    -- Affichage du mois courant
    local lblMois = gui:label(winMonth, 20, affichageTop, 280, 35)
    lblMois:setHorizontalAlignment(CENTER_ALIGNMENT)
    lblMois:setText(formatDate({year, month, 1}, "MM yyyy"))

    -- Affichage des fleches avant et aprés pour naviguer sur le mois précédent / suivant
    local previousMonth, previousYear
    if month == 1 then
        previousMonth = 12
        previousYear = year -1
    else
        previousMonth = month-1
        previousYear = year
    end
        
    local imgPreviousMonth = gui:image(winMonth, "fleche_gauche.png", 0, affichageTop, 20, 35)
    imgPreviousMonth:onClick(function() displayMonth(previousYear, previousMonth) end)

    local nextMonth, nextYear
    if month == 12 then
        nextMonth = 1
        nextYear = year +1
    else
        nextMonth = month+1
        nextYear = year
    end

    local imgNextMonth = gui:image(winMonth, "fleche_droite.png", 300, affichageTop, 20, 35)
    imgNextMonth:onClick(function() displayMonth(nextYear, nextMonth) end)

    local espacementBox = 3         -- espacement entre les cellules
    local heigthHeader = 30         -- hauteur fixe pour le header
    local topHeader = affichageTop+ heigthHeader    -- décalage en haut pour l'affichage du header

    -- Définition de la taille du calendrier
    local heightDisplayMonth = 480 - topHeader - heigthHeader- espacementBox - 10--360  -- hauteur de la partie calendrier
    local widthDisplayMonth = 300   -- largeur de la partie calendrier

    local numFirstDay   = getDayOfWeek (year, month, 1) -- jour de la semaine du premier jour du mois
    local nbDaysInMonth = getDaysInMonth(year, month)   -- nombre de jour dans le mois
    local nbWeeksInMonths = int((6+numFirstDay + nbDaysInMonth -1 )/7)  -- nombre de semaine à afficher pour le mois
    local NbJourstoDisplay = 7      -- Nombre de jours à afficher (dépend de la config avec l'affichagfe ou non des week)end
    local widthBoxJour              -- largeur de la cellule jour
    local widthWeekNum = 30         -- largeur fixe de la partie num de semaine
    local leftMonth                 -- décalage à gauche pour l'affichage du lundi

    -- si config.displayBusinessWeek = true alors on n'affiche pas les week-end
    if config.displayBusinessWeek then NbJourstoDisplay = 5 end

    -- Affichage du header avec les jours
    if config.displayWeekNum then
        widthBoxJour = int((widthDisplayMonth-widthWeekNum - NbJourstoDisplay*espacementBox) / NbJourstoDisplay)
        leftMonth = widthWeekNum
    else
        widthBoxJour = int((widthDisplayMonth- NbJourstoDisplay*espacementBox )/ NbJourstoDisplay)
        leftMonth = 0
    end

    -- hauteur des cellules jour
    local heightSemaine = int((heightDisplayMonth - nbWeeksInMonths * espacementBox)/nbWeeksInMonths)
    local heightEvent = 10
    
    -- AMELIORATION POSSIBLE - Calculer le nombre d'event affichable possible

    -- Affichage du header
    for i=1, NbJourstoDisplay do
        local lblHearder = gui:label(winMonth, 10+leftMonth + espacementBox+ (i-1)*(widthBoxJour + espacementBox) , topHeader, widthBoxJour, heigthHeader)
        lblHearder:setBackgroundColor(COLOR_LIGHT_GREY)
        lblHearder:setHorizontalAlignment(CENTER_ALIGNMENT)
        lblHearder:setVerticalAlignment(CENTER_ALIGNMENT)
        lblHearder:setText(getDayOfWeekShortName(i))
    end
    local weekNum = getWeekNum(year, month, 1)

    -- Affichage des cellules
    for i=1, nbWeeksInMonths do

        -- si config.displayWeekNum, on affiche le numéro de semaine
        if config.displayWeekNum then
            local lblNumSemaine = gui:label (winMonth, 10, topHeader+ heigthHeader + espacementBox+ (heightSemaine+ espacementBox) * (i-1),widthWeekNum, heightSemaine)
            lblNumSemaine:setBackgroundColor(COLOR_LIGHT_GREY)
            lblNumSemaine:setHorizontalAlignment(CENTER_ALIGNMENT)
            lblNumSemaine:setVerticalAlignment(CENTER_ALIGNMENT)
            lblNumSemaine:setFontSize(14)
            lblNumSemaine:setText(tostring(weekNum +i -1))
        end

        -- Affichage des cellules
        for j=1, NbJourstoDisplay do
            -- calcul de la date
            local decalageDays = (i-1) * 7 + j - numFirstDay
            local dateCellule = addDaysToDate({year, month, 1}, decalageDays)

            -- Affichage de la cellule
            local lblJourMonth = gui:label (winMonth, 10+leftMonth + espacementBox+ (j-1)*(widthBoxJour + espacementBox), topHeader+ heigthHeader + espacementBox+ (heightSemaine+ espacementBox) * (i-1),widthBoxJour, heightSemaine)
            lblJourMonth:setBorderSize(1)
            lblJourMonth:setBorderColor(COLOR_LIGHT_GREY)
            lblJourMonth:setHorizontalAlignment(RIGHT_ALIGNMENT)
            lblJourMonth:setFontSize(14)
            lblJourMonth:setRadius(1)

            -- Affichage des events (s'il y en a)
            local lstEvent = getDayEvents(dateCellule[1], dateCellule[2], dateCellule[3])
            if lstEvent then
                local nbEvents = #lstEvent
                local idx = 0

                for id, ev in pairs(lstEvent) do
                    local lblEvent =  gui:label(lblJourMonth, 2, 16 +idx *(heightEvent+1), widthBoxJour-4, heightEvent)
                    lblEvent:setBackgroundColor(COLOR_LIGHT_ORANGE)
                    lblEvent:setText(ev.name)
                    lblEvent:setFontSize(10)
                    idx = idx +1
                end
            end

            -- affichage des jours du mois précédent / suivant en une autre couleur
            if dateCellule[2] ~= month then
                lblJourMonth:setTextColor(COLOR_LIGHT_GREY)    
            end
            lblJourMonth:setText(tostring(dateCellule[3]))

            -- highligth de la cellule du jour courant
            if (today[1] ==  dateCellule[1] and today[2] ==  dateCellule[2] and today[3] ==  dateCellule[3]) then
                lblJourMonth:setBorderColor(COLOR_BLUE)
                --lblJourMonth:setBackgroundColor(COLOR_LIGHT_BLUE)
            end

            lblJourMonth:onClick( function() displayDay(dateCellule[1], dateCellule[2], dateCellule[3]) end)
        end
    end
end --displayMonth


-- ---------------------------------------
-- GESTION DES DATA 
-- ---------------------------------------

-- Enregistrement d'un event dans la structure Data
function saveEvent(event)

    if not data[event.debut.year] then data[event.debut.year] = {} end
    if not data[event.debut.year][event.debut.month] then data[event.debut.year][event.debut.month] = {} end
    if not data[event.debut.year][event.debut.month][event.debut.day] then data[event.debut.year][event.debut.month][event.debut.day] = {} end
    if not data[event.debut.year][event.debut.month][event.debut.day][event.UID] then 
        data[event.debut.year][event.debut.month][event.debut.day][event.UID] = {}
    end
    --table.insert(data[event.debut.year][event.debut.month][event.debut.day][event.UID], event)
    data[event.debut.year][event.debut.month][event.debut.day][event.UID] = event

    saveDataFile(event.debut.year,event.debut.month)
end


-- create an array based on its json representation
function json_to_array(jsonString)

    local function parse(jsonStr)
        local array = {}
        jsonStr = jsonStr:match("^%s*%.+")

        jsonStr = json:sub(2, -2) -- Remove brackets
        local current = array
        local stack = {}

        for value in jsonStr:gmatch("{[^,%[%]]+}") do
            value = value:gsub('^%s*(.-)%s*$', '%1') -- Trim whitespace

            if value == "{" then
                local newArray = {}
                table.insert(current, newArray)
                table.insert(stack, current)
                current = newArray
            elseif value == "}" then
                current = table.remove(stack)
            else
                table.insert(current, value:match('^%s*"(.*)"%s*$') or value)
            end
        end

        return array
    end
    return parse(jsonString)
end


-- create a json string representing the given array
function array_to_json(array)


    local result = "{"
    local first = true

    if type(array) == "table" then
        for k, v in pairs(array) do

            if not first then
                result = result .. ", "
            end
            first = false
     
            if type(v) == "table" then
                result = result..'"'..tostring(k) ..'":'..array_to_json(v)
            elseif type(v) == "string" then
                result = result..'"'..tostring(k) ..'":'.. '"' .. v:gsub('"', '\\"') .. '"'
            elseif type(v) == "number" then
                result = result..'"'..tostring(k) ..'":' .. tostring(v)
            elseif type(v) == "boolean" then
                result = result..'"'..tostring(k) ..'":' .. tostring(v)
            else
                result = result..'"'..tostring(k) ..'":'.. "null"
            end
        end
    elseif type(array) == "string" then
        result = result..'"'..tostring(k) ..'":'.. '"' .. array:gsub('"', '\\"') .. '"'
    elseif type(array) == "number" then
        result = result..'"'..tostring(k) ..'":'.. tostring(array)
    elseif type(array) == "boolean" then
        result = result..'"'..tostring(k) ..'":'.. tostring(array)
    else
        result = result..'"'..tostring(k) ..'":'.. "null"
    end

    result = result .. "}"
    return result
end

-- Sauvegarde la structure data pour un mois donné dans un fichier au format json
-- si le fichier existe déjà, les données sont écrasées itégralement
function saveDataFile(year, month)

    if data[year] then
        if data[year][month] then
            local filename = PATH_DATA.."/"..tostring(year)..tostring(month)..".dat"
            saveTable(filename, data[year][month])
        end
    end

end

-- Chargement des fichiers event dans la structure data
-- les fichiers event sont stockés dans PATH_DATA
-- on compte un fichier event par mois, au format json, avec le nom {year}{month}.dat
-- pour un mois donné, on charge les données du mois, du mois précédent et du mois suivant si 'noLoadingAdjacent' n'est pas indiqué

function loadDataMonth(year, month, noLoadingAdjacent)

    local filename = PATH_DATA.."/"..tostring(year)..tostring(month)..".dat"
    local fileData
    local strData

    if data[year] and data[year][month] then
        print("données déjà chargées pour le mois "..tostring(month).. " de "..tostring(year))
        return
    end

    -- par défaut, on charge les mois précédent et suivants
    if noLoadingAdjacent == nil then noLoadingAdjacent = false end

    -- check if a data file exists
    if storage:isFile(filename) then
    local dataLoad = loadTable(filename)

    if not dataLoad then 
        print("Error loading data from "..filename)
        return
     end

    if not data[year] then data[year] = {} end 
    if not data[year][month] then data[year][month] = {} end 
    
    data[year][month]=dataLoad

        if (not noLoadingAdjacent) then
            -- chargement des données du mois précédent
            local previousMonth, previousYear
            if month == 1 then
                previousMonth = 12
                previousYear = year - 1
            else
                previousMonth = month -1
                previousYear = year
            end

            loadDataMonth(previousYear, previousMonth, true)
            -- chargement des données du mois suivant
            local nextMonth, nextYear
            if month == 12 then
                nextMonth = 1
                nextYear = year + 1
            else
                nextMonth = month + 1
                nextYear = year
            end
            loadDataMonth(nextYear, nextMonth, true)
        end
    else
        return
    end

end --loadDataMonth

-- retourne la liste d'événements existant pour un jour donné
-- retourne nil si aucun évenment n'existe
function getDayEvents(year, month, day)
    local result = nil
    if data[year] then
       if  data[year][month] then
            if  data[year][month][day] then
--                print("des données existent pour "..tostring(day).."/"..tostring(month).."/"..tostring(year))
                result =  data[year][month][day]
            end
        end
    end
    --debugPrint(result)
    return result
end

-- récupére l'objet event sur un jour donné avec son UID
function getEventByID(UID, year, month, day)
    local result = nil
    if data[year] then
       if  data[year][month] then
            if  data[year][month][day] then
                if  data[year][month][day][UID] then
                    result =  data[year][month][day][UID]
                end
            end
        end
    end
    return result

end

-- Suppression d'un event de la table data et retourne cet élément
function deleteEvent(UID, year, month, day)
    
    local element = data[year][month][day][UID]
    if element then
        data[year][month][day][UID] = nil
    end
    return element 
end

-- Créer un nouvel événement 
function createEventObject(name, yearDebut, monthDebut, dayDebut, heureDebut, minuteDebut, yearFin, monthFin, dayFin, heureFin, minuteFin)

    event = {}
    event.debut = {}
    event.fin   = {}
    event.UID = createUID()

    --{"s","mi","h","d","mo","y"};
    local date= time:get("y,mo,d,h,mi")
    year = date[1]
    month = date[2]
    day = date[3]
    heure = date[4]
    minute = date[5]

    -- TODO: gestion de la cohérence des dates & heures entre début et fin

    if not name then event.name = "" else event.name = name end
--    if not description then event.description = "" else event.description = description end

    if not yearDebut then event.debut.year = year else event.debut.year = yearDebut end
    if not monthDebut then event.debut.month = month else event.debut.month = monthDebut end
    if not dayDebut then event.debut.day = day else event.debut.day = dayDebut end
    if not heureDebut then event.debut.heure = heure else event.debut.day = heureDebut end
    if not minuteDebut then event.debut.minute = 0 else event.debut.day = minuteDebut end

    if not yearFin then event.fin.year = year else event.fin.year = yearFin end
    if not monthFin then event.fin.month = month else event.fin.month = monthFin end
    if not dayFin then event.fin.day = day else event.fin.day = dayFin end
    if not heureFin then event.fin.heure = heure+1 else event.fin.day = heureFin end
    if not minuteFin then event.fin.minute = 0 else event.fin.day = minuteFin end

    return event
end

-- ---------------------------------------
-- AFFICHAGE D'UN JOUR
-- ---------------------------------------
function displayDay(year, month, day)

    currentMode = "day"

    -- création d'une nouvelle fenetre pour le jour
--    winDay = gui:window()
--    gui:setWindow(winDay)

    winDay = manageWindow()

    -- cration de la top barre
    displayTopBarre()

    -- sélection du menu actif
    selectMenu(boxImgDay)

    -- chargement des données du mois 
    loadDataMonth(year, month)

    -- --------------------
    -- création de l'écran
    -- --------------------
    
    -- Label du jour sélectionné
    local lblJour = gui:label(winDay, 10, affichageTop, 200, 25)
    lblJour:setHorizontalAlignment(LEFT_ALIGNMENT)

    str = formatDate({year, month, day}, "DD dd MM")
    lblJour:setText(str)

    local sizeListeHeures = 350
    local NbHeureToDisplay = config.day.heureFin - config.day.heureDebut +1
    
    -- récupération de l'heure actuelle
    local currentTime =  time:get("h")
    local currentHour = currentTime[1]

    local today = getToday()

    local sizeBoxHeure = int(sizeListeHeures / NbHeureToDisplay)
    local widthBox = 300

    -- liste déroulante des heures
    vListeHeure = gui:vlist(winDay, 10, 85, widthBox, sizeListeHeures)
    vListeHeure:setSpaceLine(0)
    vListeHeure:setBackgroundColor(COLOR_LIGHT_GREY)

    -- récupération de la liste des événements
    local lstEvent = getDayEvents(year, month, day)

    -- Affichage des heures
    for i=0,23 do
        local boxHour = gui:box(vListeHeure, 0, 0, widthBox, sizeBoxHeure)

        local colorBackGround = COLOR_WHITE
        local colorTextHeure = COLOR_BLACK
        local colorTextMin = COLOR_GREY
        local colorLine = COLOR_GREY

        if (i==currentHour and today[1] == year and today[2] == month and today[3] == day ) then
            colorLine = COLOR_RED
            colorBackGround = COLOR_RED
            colorTextHeure = COLOR_WHITE
            colorTextMin = COLOR_WHITE
        end

        boxHour:setBackgroundColor(colorLine)

        local lblDummy = gui:label(boxHour, 0 , 1, widthBox, sizeBoxHeure-1)
        if (i < config.day.heureDebut or i>config.day.heureFin) then
            lblDummy:setBackgroundColor(COLOR_LIGHT_BLUE)
        else
            lblDummy:setBackgroundColor(COLOR_WHITE)
        end

        local lblHeure = gui:label(boxHour, 0 , 1, 20, sizeBoxHeure-2)
        lblHeure:setBackgroundColor(colorBackGround)    
        lblHeure:setTextColor(colorTextHeure)
        lblHeure:setHorizontalAlignment(RIGHT_ALIGNMENT)
        lblHeure:setFontSize(14)
        lblHeure:setText(tostring(i)..":")

        local lblMin = gui:label(boxHour, 20,1, 18, sizeBoxHeure-2)
        lblMin:setHorizontalAlignment(LEFT_ALIGNMENT)
        lblMin:setTextColor(colorTextMin)
        lblMin:setBackgroundColor(colorBackGround)    
        lblMin:setFontSize(10)
        lblMin:setText("00")

        -- ------------------------
        -- Affichage des evenements 
        -- ------------------------

        if lstEvent ~= nil then
            for _, ev in pairs(lstEvent) do
                -- il y a un un event ou troncon d'event sur cette plage horaire
                if i>= ev.debut.heure and (i < ev.fin.heure or ( ev.fin.heure==i and ev.fin.minute >0 )) then
                    local nomEvent = ""
                    -- gestion du positionnement du haut de l'event dans la case horaire
                    local positionHaut = 0
                    if ev.debut.heure == i then
                        positionHaut = int(ev.debut.minute * sizeBoxHeure / 60)+1
                        -- on met le nom de l'event sur le 1er "troncon"
                        nomEvent = ev.name  
                    elseif i > ev.debut.heure then
                        positionHaut = 0
                    end

                    -- gestion du positionnement du bas de l'event dans la case horaire
                    local positionBas = sizeBoxHeure-1
                    if ev.fin.heure == i then
                        positionBas = int((ev.fin.minute) * sizeBoxHeure / 60)-1
                    elseif ev.fin.heure > i then
                        positionBas = sizeBoxHeure
                    end
                    
                    local lblEvent = gui:label(boxHour, 41, positionHaut, widthBox - 50, positionBas - positionHaut)
                    --local lblEvent = gui:label(boxHour, 41, 0, widthBox - 50, sizeBoxHeure)
                    lblEvent:setBackgroundColor(COLOR_LIGHT_ORANGE)
                    lblEvent:setFontSize(14)
                    lblEvent:setText(" "..nomEvent)

                    lblEvent:onClick(
                        function()
                            local ev = getEventByID(ev.UID, year, month, day)
                            if not ev then ev = createEventObject (nil, year, month, day) end
                            displayEvent(ev)
                        end                        
                    )
                end
            end
        end -- if lstEvent
    end

    -- on positionne la vliste sur la 1ere heure du jour
    vListeHeure:setIndex(config.day.heureDebut)

    -- ------------------------------------------
    -- Affichage des jours de la semaine en cours

    local currentDayWeek = getDayOfWeek(year, month, day)
    local nbDaysWeekToDisplay = 7
    if (config.displayBusinessWeek) then
        nbDaysWeekToDisplay = 5
    end

    local widthBoxDay = int((winDay:getWidth()-40)/nbDaysWeekToDisplay)
    vListDays = gui:hlist(winDay, 20, 435, widthBox, 40)
    vListDays:setSpaceLine(0)

    -- ajout des fleches de navigation d'une semaine en + ou en -
    local datePreviousWeek = addDaysToDate({year, month, day}, -nbDaysWeekToDisplay)
    local imgPreviousWeek = gui:image(winDay, "fleche_gauche.png", 0, 440, 20, 35)
    imgPreviousWeek:onClick(function() displayDay(datePreviousWeek[1], datePreviousWeek[2], datePreviousWeek[3]) end)

    local dateNextWeek = addDaysToDate({year, month, day}, nbDaysWeekToDisplay)
    local imgNextWeek = gui:image(winDay, "fleche_droite.png", 300, 440, 20, 35)
    imgNextWeek:onClick(function() displayDay(dateNextWeek[1], dateNextWeek[2], dateNextWeek[3]) end)

    for i=1, nbDaysWeekToDisplay do
        local date = addDaysToDate({year, month, day}, i-currentDayWeek)
        local boxDayWeek = gui:box(vListDays, 0, 5, widthBoxDay, 35)
        local lblDayOfWeek = gui:label(boxDayWeek, 2, 2, widthBoxDay-4, 12, COLOR_WHITE)
        lblDayOfWeek:setFontSize(9)
        lblDayOfWeek:setHorizontalAlignment(CENTER_ALIGNMENT)
        lblDayOfWeek:setText(getDayOfWeekName(i))

        local lblDay = gui:label(boxDayWeek, 4, 11, widthBoxDay-8, 20, COLOR_WHITE)
        lblDay:setHorizontalAlignment(CENTER_ALIGNMENT)
        lblDay:setFontSize(15)
        lblDay:setText(tostring(date[3]))

        if (currentDayWeek == i) then
            boxDayWeek:setRadius(5)
            boxDayWeek:setBackgroundColor(COLOR_LIGHT_ORANGE)
            lblDayOfWeek:setBackgroundColor(COLOR_LIGHT_ORANGE)
            lblDay:setBackgroundColor(COLOR_LIGHT_ORANGE)
            --lblDayOfWeek
        else
            boxDayWeek:onClick(function() displayDay(date[1], date[2], date[3]) end)
        end
    end
end --displayDay

-- ---------------------------------------
-- ECRAN DETAIL / NOUVEL EVENEMENT
-- ---------------------------------------

function displayEvent(event)

--    winNewEvent = gui:window()
--    gui:setWindow(winNewEvent)
    winNewEvent = manageWindow()

    local isNew = false

    local titre=""
    if not event then
         event = createEventObject() 
         isNew = true
    end

    if event.name == nil or event.name == "" then 
        titre = "Nouvel évènement" 
        event.name = ""
        isNew = true
    end

    -- Message d'erreur
    local lblMsgError = gui:label(winNewEvent, 30, 380, 260, 40)
    lblMsgError:setFontSize(14)
    lblMsgError:setHorizontalAlignment(CENTER_ALIGNMENT)
    lblMsgError:setTextColor(COLOR_RED)

    -- Bouton retour arrière
    imgBack = gui:image(winNewEvent, "back.png", 20, 30, 18, 18)
    imgBack:onClick(function () switchScreen(year, month, day) end)

    -- input  Nom de l'event
    local inputName = gui:input(winNewEvent, 60, 10, 250, 40)
    inputName:setTitle(titre)
    inputName:setText(event.name)
    inputName:onClick(
        function ()
            local keyboard = gui:keyboard("Nom de l'évènement", event.name)
            inputName:setText(keyboard)
            event.name = keyboard
            lblMsgError:setText("")
        end
    )
    -- -----------------------------------
    -- Gestion de la date & heure de début

    local lblDebut = gui:label(winNewEvent, 20, 80, 100, 20)
    lblDebut:setText("Début")

    local lblDateHeureDebut = gui:label(winNewEvent, 120, 80, 180, 20)
    lblDateHeureDebut:setHorizontalAlignment(RIGHT_ALIGNMENT)
    lblDateHeureDebut:setText(formatDate({event.debut.year, event.debut.month, event.debut.day, event.debut.heure, event.debut.minute},"DD dd sh hh:mi"))

    local lblTraitDebut = gui:label(winNewEvent, 20, 101, 280, 1)
    lblTraitDebut:setBackgroundColor(COLOR_GREY)

    local lblFin = gui:label(winNewEvent, 20, 230, 100, 20)
    lblFin:setText("Fin")

    local lblDateHeureFin = gui:label(winNewEvent, 120, 230, 180, 20)
    lblDateHeureFin:setHorizontalAlignment(RIGHT_ALIGNMENT)
    lblDateHeureFin:setText(formatDate({event.fin.year, event.fin.month, event.fin.day, event.fin.heure, event.fin.minute},"DD dd sh hh:mi"))

    local lblTraitFin = gui:label(winNewEvent, 20, 251, 280, 1)
    lblTraitFin:setBackgroundColor(COLOR_GREY)

    -- Liste des dates de début
    local vListeDateDebutNewEvent = gui:vlist(winNewEvent, 20, 110, 140, 100)
    vListeDateDebutNewEvent:setSpaceLine(0)
    vListeDateDebutNewEvent:setAutoSelect(true)
    vListeDateDebutNewEvent:setSelectionFocus(SELECTION_CENTER)
    vListeDateDebutNewEvent:setSelectionColor(COLOR_LIGHT_ORANGE)

    -- Liste des dates de Fin
    local vListeDateFinNewEvent = gui:vlist(winNewEvent, 20, 260, 140, 100)
    vListeDateFinNewEvent:setSpaceLine(0)
    vListeDateFinNewEvent:setAutoSelect(true)
    vListeDateFinNewEvent:setSelectionFocus(SELECTION_CENTER)
    vListeDateFinNewEvent:setSelectionColor(COLOR_LIGHT_ORANGE)

    local nbJourToDisplay = 30
    local lstDates ={}
    
    vListeDateDebutNewEvent:onSelect(function ()
        local date = lstDates[vListeDateDebutNewEvent:getSelected()]
        event.debut.year = date[1]
        event.debut.month = date[2]
        event.debut.day = date[3]
        
        lblDateHeureDebut:setText(formatDate(event.debut,"DD dd sh hh:mi"))
        lblMsgError:setText("")
    end)

    vListeDateFinNewEvent:onSelect(function ()
        local date = lstDates[vListeDateFinNewEvent:getSelected()]
        event.fin.year = date[1]
        event.fin.month = date[2]
        event.fin.day = date[3]
        
        lblDateHeureFin:setText(formatDate(event.fin,"DD dd sh hh:mi"))
        lblMsgError:setText("")
    end)


    -- Affichage des dates Debut et Fin
    for i=0, 2*nbJourToDisplay do
        local date = addDaysToDate({event.debut.year, event.debut.month, event.debut.day}, i-nbJourToDisplay)
        lstDates[i] = date
        local strDate = formatDate(date, "DD dd sh").." "

        local lblDateDebut = gui:label(vListeDateDebutNewEvent, 0, 0, 140, 17)
        lblDateDebut:setFontSize(16)
        lblDateDebut:setHorizontalAlignment(RIGHT_ALIGNMENT)
        lblDateDebut:setText(strDate)

        local lblDateFin = gui:label(vListeDateFinNewEvent, 0, 0, 140, 17)
        lblDateFin:setFontSize(16)
        lblDateFin:setHorizontalAlignment(RIGHT_ALIGNMENT)
        lblDateFin:setText(strDate)

    end
    
    -- selection de la date du jour de début
    vListeDateDebutNewEvent:select(nbJourToDisplay)
    vListeDateFinNewEvent:select(nbJourToDisplay)

    -- Liste des heures de Début et Fin de l'event
    local vListeHeureDebutNewEvent = gui:vlist(winNewEvent, 200, 110, 40, 100)
    vListeHeureDebutNewEvent:setSpaceLine(0)
    vListeHeureDebutNewEvent:setAutoSelect(true)
    vListeHeureDebutNewEvent:setSelectionFocus(SELECTION_CENTER)
    vListeHeureDebutNewEvent:setSelectionColor(COLOR_LIGHT_ORANGE)
    vListeHeureDebutNewEvent:onSelect(function ()
        event.debut.heure = vListeHeureDebutNewEvent:getSelected()
        lblDateHeureDebut:setText(formatDate(event.debut,"DD dd sh hh:mi"))
        lblMsgError:setText("")
    end
)
    local vListeHeureFinNewEvent = gui:vlist(winNewEvent, 200, 260, 40, 100)
    vListeHeureFinNewEvent:setSpaceLine(0)
    vListeHeureFinNewEvent:setAutoSelect(true)
    vListeHeureFinNewEvent:setSelectionFocus(SELECTION_CENTER)
    vListeHeureFinNewEvent:setSelectionColor(COLOR_LIGHT_ORANGE)
    vListeHeureFinNewEvent:onSelect(
        function ()
            event.fin.heure = vListeHeureFinNewEvent:getSelected()
            lblDateHeureFin:setText(formatDate(event.fin,"DD dd sh hh:mi"))
            lblMsgError:setText("")
        end
    )
    -- Affichage des Heures
    for i=0, 23 do
        local lblHeureDebutNewEvent = gui:label(vListeHeureDebutNewEvent, 0, 0, 40, 16)
        lblHeureDebutNewEvent:setFontSize(16)
        lblHeureDebutNewEvent:setHorizontalAlignment(CENTER_ALIGNMENT)
        lblHeureDebutNewEvent:setText(tostring(i))

        local lblHeureFinNewEvent = gui:label(vListeHeureFinNewEvent, 0, 0, 40, 16)
        lblHeureFinNewEvent:setFontSize(16)
        lblHeureFinNewEvent:setHorizontalAlignment(CENTER_ALIGNMENT)
        lblHeureFinNewEvent:setText(tostring(i))
    end

    vListeHeureDebutNewEvent:select(event.debut.heure)
    vListeHeureFinNewEvent:select(event.fin.heure)

    -- gestion des heures du début de l'évent
    local vListeMinDebutNewEvent = gui:vlist(winNewEvent, 260, 110, 40, 100)
    vListeMinDebutNewEvent:setSpaceLine(0)
    vListeMinDebutNewEvent:setAutoSelect(true)
    vListeMinDebutNewEvent:setSelectionFocus(SELECTION_CENTER)
    vListeMinDebutNewEvent:setSelectionColor(COLOR_LIGHT_ORANGE)
    vListeMinDebutNewEvent:onSelect(
        function ()
            event.debut.minute = vListeMinDebutNewEvent:getSelected()*5
            lblDateHeureDebut:setText(formatDate(event.debut,"DD dd sh hh:mi"))
            lblMsgError:setText("")
        end
    )
    local vListeMinFinNewEvent = gui:vlist(winNewEvent, 260, 260, 40, 100)
    vListeMinFinNewEvent:setSpaceLine(0)
    vListeMinFinNewEvent:setAutoSelect(true)
    vListeMinFinNewEvent:setSelectionFocus(SELECTION_CENTER)
    vListeMinFinNewEvent:setSelectionColor(COLOR_LIGHT_ORANGE)
    vListeMinFinNewEvent:onSelect(
        function ()
            event.fin.minute = vListeMinFinNewEvent:getSelected()*5
            lblDateHeureFin:setText(formatDate(event.fin,"DD dd sh hh:mi"))
            lblMsgError:setText("")
        end
    )

    -- Affichage des Minutes
    for i=0, 55, 5 do
        local lblMinuteDebutNewEvent = gui:label(vListeMinDebutNewEvent, 0, 0, 40, 13)
        lblMinuteDebutNewEvent:setFontSize(16)
        lblMinuteDebutNewEvent:setHorizontalAlignment(CENTER_ALIGNMENT)
        lblMinuteDebutNewEvent:setText(tostring(i))

        local lblMinuteDebutFinEvent = gui:label(vListeMinFinNewEvent, 0, 0, 40, 13)
        lblMinuteDebutFinEvent:setFontSize(16)
        lblMinuteDebutFinEvent:setHorizontalAlignment(CENTER_ALIGNMENT)
        lblMinuteDebutFinEvent:setText(tostring(i))
    end

    vListeMinDebutNewEvent:select(int(event.debut.minute / 5))
    vListeMinFinNewEvent:select(int(event.fin.minute / 5))

    if not isNew then
        local btnSupprimer = gui:label(winNewEvent, 60, 425, 200, 22)
        btnSupprimer:setRadius(10)
        btnSupprimer:setBorderSize(1)
        btnSupprimer:setBackgroundColor(COLOR_LIGHT_GREY)
        btnSupprimer:setHorizontalAlignment(CENTER_ALIGNMENT)
        btnSupprimer:setVerticalAlignment (CENTER_ALIGNMENT)
        btnSupprimer:setText("Supprimer")
        btnSupprimer:onClick(
            function () 
                local oldEvent = deleteEvent(event.UID, event.debut.year, event.debut.month, event.debut.day)
                saveDataFile(oldEvent.debut.year, oldEvent.debut.month)
                switchScreen(oldEvent.debut.year, oldEvent.debut.month, oldEvent.debut.day)
            end
        )
    end

    local btnEnregistrer = gui:label(winNewEvent, 60, 450, 200, 22)
    btnEnregistrer:setRadius(10)
    btnEnregistrer:setBorderSize(1)
    btnEnregistrer:setBackgroundColor(COLOR_LIGHT_GREY)
    btnEnregistrer:setHorizontalAlignment(CENTER_ALIGNMENT)
    btnEnregistrer:setVerticalAlignment (CENTER_ALIGNMENT)
    btnEnregistrer:setText("Enregistrer")
    btnEnregistrer:onClick(
        function()
            local compare = compareEventDates(event.debut, event.fin)

            if event.name == "" then
                lblMsgError:setText("Renseigner le nom de l'évènement")
            elseif  compare < 1 then
                lblMsgError:setText("la date de fin doit être supérieure à la date de début")
            else
                saveEvent(event)
                switchScreen(event.debut.year, event.debut.month, event.debut.day)
            end

        end
    )

end --newEvent


-- ---------------------------------------
-- HELPER DE L'AFFICHAGE
-- ---------------------------------------

function switchScreen(year, month, day)
    if (currentMode == "day") then
        displayDay(year, month, day)
    elseif (currentMode == "week") then
        displayWeek(year, month, day)
    else
        displayMonth(year, month)
    end
end --switchScreen


-- Création de la barre de menu en haut sur la fenetre active
function displayTopBarre()

    activeWin = gui:getWindow()

    local today = getToday()

    boxImgParametre = gui:box(activeWin, 10, 15, 25, 25)
    imgParametre = gui:image(boxImgParametre, "menu.png", 0, 0, 25, 25)
    imgParametre:setTransparentColor(COLOR_WHITE)
    imgParametre:onClick(displayConfig)

    boxImgDay = gui:box(activeWin, 80, 15, 25, 25)
    imgDay   = gui:image(boxImgDay, "day.png", 0, 0, 25, 25)
    imgDay:setTransparentColor(COLOR_WHITE)
    imgDay:onClick(function() displayDay(today[1], today[2], today[3]) end)

    boxImgWeek = gui:box(activeWin, 130, 15, 25, 25)
    imgWeek = gui:image(boxImgWeek, "week.png", 0, 0, 25, 25)
    imgWeek:setTransparentColor(COLOR_WHITE)
    imgWeek:onClick(function() displayWeek(today[1], today[2], today[3]) end)

    boxImgMonth = gui:box(activeWin, 180, 15, 25, 25)
    imgMonth = gui:image(boxImgMonth, "month.png", 0, 0, 25, 25)
    imgMonth:setTransparentColor(COLOR_WHITE)
    imgMonth:onClick(function() displayMonth(today[1], today[2]) end)

    boxImgAdd = gui:box(activeWin, 280, 15, 25, 25)
    boxImgAdd:setBackgroundColor(COLOR_DARK)
    boxImgAdd:setRadius(20)
    
    local imgAdd = gui:image(boxImgAdd, "plus.png", 6, 6, 12, 12, COLOR_DARK)
    boxImgAdd:onClick(
        function()
            local event = createEventObject(nil, today[1], today[2], today[3])
            displayEvent(event) 
        end
    )

    menu = {boxImgParametre, boxImgDay, boxImgWeek, boxImgMonth}

end --displayTopBarre


-- highlight de la box sélectionnée
function selectMenu(selectedBox)

    for _, box in ipairs(menu) do
        if box == selectedBox then
            box:setBackgroundColor(COLOR_LIGHT_ORANGE)
        else
            box:setBackgroundColor(COLOR_WHITE)
        end
    end

end






-- ---------------------------------------
-- FONCTIONS HELPER
-- ---------------------------------------

-- Create a unique identifier for the event
function createUID()

    math.randomseed(time:monotonic())
    return time:monotonic()..math.random(999999)
end



--renvoi x tronqué
function int(x)
    return math.floor(x)
end


-- compare two date ate the event format
-- returns :
--      -1  if eventDebut > eventFin
--       1  if eventFin > eventDebut
--       0  if eventFin = eventDebut
function compareEventDates(eventDebut, eventFin)

    if eventFin.year ~= eventDebut.year then
        return eventFin.year < eventDebut.year and -1 or 1
    elseif eventFin.month ~= eventDebut.month then
        return eventFin.month < eventDebut.month and -1 or 1
    elseif eventFin.day ~= eventDebut.day then
        return eventFin.day < eventDebut.day and -1 or 1
    elseif eventFin.heure ~= eventDebut.heure then
        return eventFin.heure < eventDebut.heure and -1 or 1
    elseif eventFin.minute ~= eventDebut.minute then
        return eventFin.minute < eventDebut.minute and -1 or 1
    else
        return 0
    end

end


-- Format a date given a specific pattern
--      yyyy    year
--      mm      month
--      MM      month Full Name
--      sh      month Short Name
--      dd      day
--      DD      day Of Week
--      hh      heure
--      mi      minute

function formatDate (date, pattern) 

    if (date.year) then m_year =  date.year else m_year = date[1] end
    if (date.month) then m_month =  date.month else m_month = date[2] end
    if (date.day) then m_day =  date.day else m_day = date[3] end
    if (date.heure) then m_heure =  date.heure elseif date[4] then m_heure = date[4] else m_heure = 0 end
    if (date.minute) then m_minute =  date.minute elseif date[5] then m_minute = date[5] else m_minute = 0 end

    local year = m_year
    local month = string.format("%02d", m_month)
    local day = string.format("%02d", m_day)
    local heure = string.format("%02d", m_heure)
    local minute = string.format("%02d", m_minute)
    local dayOfWeek = getDayOfWeekName(getDayOfWeek(m_year, m_month, m_day))      --  PYM
    local monthName = getMonthName(m_month)
    local monthShortName = getMonthShortName(m_month)
    
        local result = pattern:gsub("yyyy", year)
                      :gsub("mm", month)
                      :gsub("MM", monthName)
                      :gsub("sh", monthShortName)
                      :gsub("dd", day)
                      :gsub("DD", dayOfWeek)
                      :gsub("hh", heure)
                      :gsub("mi", minute)
        return result
    
end --formatDate

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