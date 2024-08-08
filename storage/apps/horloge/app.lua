local tNow
local tChrono = 0

-- récupération de la fenetre
local win = gui:window()
local title

-- gestion de l'horloge
local clock
local offsetAngle = 270

-- ID des Timers
local idTimerHorloge
local idTimerChrono
local idTimerMinuteur

local oldCoordHeure = {}
local oldCoordMinute = {}
local oldCoordSeconde = {}

-- gestion du 1er affichage
local toBeRefreshed = false

-- Définition des couleurs de l'horloge
local BACKGROUND_COLOR = COLOR_WHITE
local couleur_clock = COLOR_DARK
local couleur_aiguille_sec = COLOR_RED


----------------------------------------------------------------------
-- Gestion de l'application Horloge
----------------------------------------------------------------------

--Fonction principale appelée au lancement de l'application
function run()
    init()
    drawClock()
    --afficheHorloge()
    initHorloge()
end --run


-- Init de l'horloge
-- Création des composants graphiques de l'application
function init()

    --Affichage du Titre
    title = gui:label(win, 35, 10, 144, 28)
    title:setFontSize(24)

    --Affichage du label pour l'heure
    heure_label = gui:label(win, 0, 50, 320, 70)
    heure_label:setFontSize(20)
    heure_label:setHorizontalAlignment(CENTER_ALIGNMENT)

    --Affichage des images de fonctions

    -- Box Horloge
    boxHorloge = gui:box (win, 65, 410, 30, 60)
    boxHorloge:onClick(function() initHorloge() end )
    imgHorloge = gui:image(boxHorloge, "img_horloge.png", 0, 0, 30, 30)
    lblHorloge = gui:label(boxHorloge, 0, 30, 30, 30)
    lblHorloge:setFontSize(8)
    lblHorloge:setText("Horloge")   

    -- Box Chrono
    boxChrono= gui:box (win, 145, 410, 30, 60)
    boxChrono:onClick(function() initChrono() end)
    imgChrono = gui:image(boxChrono, "img_chrono.png", 0, 0, 30, 30)
    lblChrono = gui:label(boxChrono, 0, 30, 30, 30)
    lblChrono:setFontSize(8)
    lblChrono:setText("Chrono")   

    -- Box Minuteur
    boxMinuteur = gui:box (win, 225, 410, 30, 60)
    imgMinuteur = gui:image(boxMinuteur, "img_minuteur.png", 0, 0, 30, 30)
    lblMinuteur= gui:label(boxMinuteur, 0, 30, 30, 30)
    lblMinuteur:setText("Minuteur")   
    lblMinuteur:setFontSize(8)
    


    -- -------------
    -- Specificité pour le chrono
    -- -------------
    
    -- affichage des boutons de gestion du chrono
    btnChrono = gui:button(win, 40, 360, 80, 30)
    btnChrono:setText("Start")
    btnChrono:onClick(
        function()
            runChrono()
        end
    )

    btnResetChrono = gui:button(win, 200, 360, 80, 30)
    btnResetChrono:setText("Reset")
    btnResetChrono:onClick(
        function()
            resetChrono()
        end
    )
    
    
    
    --Activation de la fenetre
    gui:setWindow(win)

end -- init





----------------------------------------------------------------------
-- Fonctions générique d'affichage  de l'horloge
----------------------------------------------------------------------

--Fonction helper int
-- Renvoi x tronqué
function int(x)
    return math.floor(x)
end

-- dessine le canevas de l'horloge vide (sans les aiguilles)
function drawClock()
    centreX = int(win:getWidth() * 0.5)
    centreY = centreX --int (win:getHeight() * 0.5)
    border = 15
    radius = int(centreX - border * 2 - 20)

    local size_major = 10
    local size_minor = 2
    local espacement = 5

    -- draw clock circle
    clock = gui:canvas(win, 0, 70, int(win:getWidth() - 40), int(win:getWidth() - 40))
    clock:fillRect(0, 0, int(win:getWidth()), int(win:getWidth()), BACKGROUND_COLOR)
    clock:drawCircle(centreX, centreY, radius, couleur_clock)

    -- loop pour les 5 min
    for i = 0, 11 do
        -- convert angle in radian
        degres = math.rad(i * 30 + offsetAngle)

        clock:drawLine(
            int(centreX + math.cos(degres) * (radius - (size_major + espacement))),
            int(centreY + math.sin(degres) * (radius - (size_major + espacement))),
            int(centreX + math.cos(degres) * (radius - espacement)),
            int(centreY + math.sin(degres) * (radius - espacement)),
            couleur_clock
        )
    end -- loop
    -- loop pour les 1 min
    for i = 0, 59 do
        -- convert angle in radian
        degres = math.rad(i * 6 + offsetAngle)

        clock:drawLine(
            int(centreX + math.cos(degres) * (radius - (size_minor + espacement))),
            int(centreY + math.sin(degres) * (radius - (size_minor + espacement))),
            int(centreX + math.cos(degres) * (radius - espacement)),
            int(centreY + math.sin(degres) * (radius - espacement)),
            couleur_clock
        )
    end -- loop
end -- drawClock

-- Dessine l'heure souhaitée sur l'horloge
function drawHorloge (hh, mm, ss)

    -- Efface les anciennes aiguilles
     -- Si c'est le 1er affichage, alors il n'y a rien à effacer...
     if toBeRefreshed then
         clock:drawLine(
             oldCoordHeure.orig_x,
             oldCoordHeure.orig_y,
             oldCoordHeure.dest_x,
             oldCoordHeure.dest_y,
             BACKGROUND_COLOR
         )
         clock:drawLine(
             oldCoordMinute.orig_x,
             oldCoordMinute.orig_y,
             oldCoordMinute.dest_x,
             oldCoordMinute.dest_y,
             BACKGROUND_COLOR
         )
         clock:drawLine(
             oldCoordSeconde.orig_x,
             oldCoordSeconde.orig_y,
             oldCoordSeconde.dest_x,
             oldCoordSeconde.dest_y,
             BACKGROUND_COLOR
         )
     end
     toBeRefreshed = true
 
     -- affichage aiguille Heure
 
     -- calcul de l'angle pour une heure (avec la variation des minute)
     -- une cadran = 12 h de 60 min
     local nbMinute = hh * 60 + mm
     local radian = math.rad(nbMinute * 360 / (12 * 60) + offsetAngle)
 
     local coordHeure = {
         orig_x = centreX,
         orig_y = centreY,
         dest_x = int(centreX + math.cos(radian) * radius * 0.5),
         dest_y = int(centreY + math.sin(radian) * radius * 0.5)
     }
 
     clock:drawLine(coordHeure.orig_x, coordHeure.orig_y, coordHeure.dest_x, coordHeure.dest_y, COLOR_DARK)
 
     -- Sauvegarde de la position de l'aiguille pour pouvoir l'effacer au prochaine rafraichissement
     copyStruct(coordHeure, oldCoordHeure)
 
     -- affichage aiguille Minute
     radian = math.rad(mm * 360 / 60 + offsetAngle)
 
     local coordMinute = {
         orig_x = centreX,
         orig_y = centreY,
         dest_x = int(centreX + math.cos(radian) * radius * 0.75),
         dest_y = int(centreY + math.sin(radian) * radius * 0.75)
     }
 
     clock:drawLine(coordMinute.orig_x, coordMinute.orig_y, coordMinute.dest_x, coordMinute.dest_y, COLOR_DARK)
 
     -- Sauvegarde de la position de l'aiguille pour pouvoir l'effacer au prochaine rafraichissement
     copyStruct(coordMinute, oldCoordMinute)
 
     -- affichage aiguille Seconde
     radian = math.rad(ss * 360 / 60 + offsetAngle)
 
     local coordSeconde = {
         orig_x = centreX,
         orig_y = centreY,
         dest_x = int(centreX + math.cos(radian) * radius * 0.75),
         dest_y = int(centreY + math.sin(radian) * radius * 0.75)
     }
 
     clock:drawLine(coordSeconde.orig_x, coordSeconde.orig_y, coordSeconde.dest_x, coordSeconde.dest_y, couleur_aiguille_sec)
     -- Sauvegarde de la position de l'aiguille pour pouvoir l'effacer au prochaine rafraichissement
     copyStruct(coordSeconde, oldCoordSeconde)
 
 end --drawHorloge
 
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

-- Recopie une structure dans une autre
function copyStruct(source, dest)
    --  print("copyStruct ".. tostring(pairs(source)))
    for i in pairs(source) do
        -- print("dest["..i.."]=".. source[i])
        dest[i] = source[i]
    end
end -- function copyStruct


----------------------------------------------------------------------
-- Gestion du mode HORLOGE
----------------------------------------------------------------------
function afficheHorloge()

    -- recupere l'heure
    tNow = time:get("h,mi,s")

    local heure = tNow[1]
    local minute = tNow[2]
    local seconde = tNow[3]

    local now = convert_to_time(heure, minute, seconde) --= table.concat(tNow, ":")
    heure_label:setText(now)

    --Affichage de l'heure dans l'horloge
    drawHorloge (heure, minute, seconde)

    -- set timer to refresh in 1 sec
    idTimerHorloge = time:setTimeout(afficheHorloge, 1000)
end -- afficheHorloge

function initHorloge()
    -- Modification du titre
    title:setText("Horloge")

    btnChrono:disable()
    btnResetChrono:disable()

    afficheHorloge()

end

----------------------------------------------------------------------
-- Gestion du mode CHRONO
----------------------------------------------------------------------

-- Initialisation du mode Chrono
function initChrono()

    -- Modification du titre
    title:setText("Chronomètre")

    btnChrono:enable()
    btnResetChrono:enable()

    -- Arrete le timer de l'horloge
    time:removeTimeout(idTimerHorloge)

    -- Initialisation du chrono
    tChrono = 0
    afficheChrono()

end --initChrono



-- Fonction d'affichage du chrono sur le cadran et sur le texte
function afficheChrono()

    print("afficheChrono")

    -- recupere le chrono et le transforme en hh, mm, ss
    local heure = int(tChrono / 3600)
    local minute = int(60* (tChrono / 3600 - int(tChrono / 3600)))
    local seconde = tChrono % 60
    local now = convert_to_time(heure, minute, seconde) 

    -- Affichage du chrono dans le label
    heure_label:setText(now)

    --Affichage de l'heure dans l'horloge
    drawHorloge (heure, minute, seconde)

end -- afficheChrono



-- Run du chronomètre
function runChrono()
    tChrono = tChrono + 1
    afficheChrono()

    --Si le chrono est en route, modifie le texte et le callback
    btnChrono:setText("Stop")
    btnChrono:onClick(
        function()
            stopChrono()
        end
    )

    -- set timer to refresh in 1 sec
    idTimerChrono = time:setTimeout(runChrono, 1000)
end --runChrono


-- reset du chrono
function resetChrono()
    tChrono = 0
    afficheChrono()
end --resetChrono


-- Stop le chrono
function stopChrono()
    time:removeTimeout(idTimerChrono)

    --Si le chrono est en route, modifie le texte et le callback
    btnChrono:setText("Start")
    btnChrono:onClick(
        function()
            runChrono()
        end
    )
    
    -- afficheChrono()
end --resetChrono
