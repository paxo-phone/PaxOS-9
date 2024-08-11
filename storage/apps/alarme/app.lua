local tNow

-- récupération de la fenetre
local win = gui:window()
local winAlarme

-- Définition des couleurs de l'horloge
local BACKGROUND_COLOR = COLOR_WHITE
local couleur_clock = COLOR_DARK
local couleur_aiguille_sec = COLOR_RED

-- liste Alarme
listAlarme ={
    {"lever 1", "7:30", true}, 
    {"lever 2", "8:30", false},
    {"lever 3", "9:30", true}
}


-- -------------------------------------------
--    FONCTION HELPERS
-- -------------------------------------------



function int(x)
    return math.floor(x)
end

-- renvoi une string au format HH:MM:SS
function convert_to_time(h, min, sec)
    local strHeure = h
    -- Gestion Heure
    if tonumber(h) < 10 then
        if tonumber(h) == nil then
            strHeure = "00"
        else
            strHeure = "0" .. h
        end
    end

    local strMin = min
    -- Gestion Minute
    if tonumber(min) < 10 then
        if tonumber(min) == nil then
            strMin = "00"
        else
            strMin = "0" .. min
        end
    end

    local strSec = sec
    -- Gestion Seconde
    if tonumber(sec) < 10 then
        if tonumber(sec) == nil then
            strSec = "00"
        else
            strSec = "0" .. sec
        end
    end

    return strHeure .. ":" .. strMin .. ":" .. strSec
end -- convert_to_time


-- Fonction d'affichage de l'heure en digital, et en mode horloge
function afficheHeure()

    -- recupere l'heure
    tNow = time:get("h,mi,s")

    local heure = tNow[1]
    local minute = tNow[2]
    local seconde = tNow[3]

    local now = convert_to_time(heure, minute, seconde) --= table.concat(tNow, ":")
    heure_label:setText(now)

    idRefreshClock = time:setTimeout(afficheHeure, 1000)
end -- afficheHeure


-- ---------------------------
--     Ecran principal
-- ---------------------------

-- Affichage initiale de l'écran
function init()

    -- Titre 
    local title = gui:label(win, 35, 10, 144, 28)
    title:setFontSize(24)
    title:setText("Alarme")

    -- Label de l'heure courante
    heure_label = gui:label(win, 0, 50, 320, 30)
    heure_label:setHorizontalAlignment(CENTER_ALIGNMENT)

    -- Liste des alarmes
    lstAlarme = gui:vlist(win, 70, 110, 250, 280)


    for i, value in pairs(listAlarme) do

        -- créer une box pour la ligne de l'alarme
        local case = gui:box(lstAlarme, 0, 0, 250, 25)

        -- Affichage de l'heure
        local heure = gui:label(case, 0, 0, 100, 18)
        heure:setText(value[2])
        heure:setFontSize(16)

        -- affichafe du nom
        local nom = gui:label(case, 100, 0, 100, 18)
        nom:setText(value[1])
        nom:setFontSize(16)
        nom:onClick(function() changeName(nom) end)

        -- afffichage du bouton d'activation
        local btnToggleAlarme = gui:switch(case, 200, 0, 18, 18)
        if value[3] then
            btnToggleAlarme:setState(true)
            heure:setTextColor(COLOR_BLACK)
            nom:setTextColor(COLOR_BLACK)
        else
            btnToggleAlarme:setState(false)
            heure:setTextColor(COLOR_GREY)
            nom:setTextColor(COLOR_GREY)

        end 
        btnToggleAlarme:onClick(function () fnToggleAlarme(btnToggleAlarme, heure, nom) end)

    end

    -- bouton +
    local add = gui:box(win, 250, 410, 40, 40)
    add:setBackgroundColor(COLOR_DARK)
    add:setRadius(20)
    local icon_plus = gui:image(add, "plus.png", 14, 14, 12, 12, COLOR_DARK)
    add:onClick(addAlarme)

    --add:onClick(newContact)

    -- bouton de test
    testAlarme = gui:button(win, 35, 400, 200, 38)
    testAlarme:setText("Test")
    testAlarme:onClick(
            closeAlarme
            
            --    time:setTimeout(
            --             gui:showInfoMessage("TimeOut") 
            --        , 
            --        10000
            --    )
            
        )
    gui:setWindow(win)

end -- init


function fnToggleAlarme(btn, lblHeure, lblNom)
        if btn:getState() then
            lblHeure:setTextColor(COLOR_BLACK)
            lblNom:setTextColor(COLOR_BLACK)
        else
            lblHeure:setTextColor(COLOR_GREY)
            lblNom:setTextColor(COLOR_GREY)
        end
end


function changeName(label)
    
    local keyboard = gui:keyboard("Placeholder", label:getText())
    label:setText(keyboard)
end

-- Fonction pour fermer la fenetre de l'application 
-- lance le timer si l'alarme est active
function closeAlarme()
        -- get the time diff
        id = time:setTimeout( function() ringAlarme() end, 10000)
        print("id"..id)
end -- function closeAlarme



-- Fonction de lancement de l'alarme
function ringAlarme()
    print("ringAlarme")
    gui:showInfoMessage("TimeOut")
    hardware:setVibrator(true);
end

-- ---------------------------------------
-- Ecran Ajout nouvelle alarme
-- ---------------------------------------

function addAlarme()

    print("addAlarme")

end -- addAlarme



function run()
    init()
    afficheHeure()
end --run
