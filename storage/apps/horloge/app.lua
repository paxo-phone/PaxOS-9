local tNow
local tChrono = 0
local tMinuteur

-- récupération de la fenetre
local win = gui:window()
local title

-- gestion de l'horloge
local clock
local offsetAngle = 270

-- gestion du minuteur
local minuteur
local minuteurHeure = 0
local minuteurMinute = 0
local minuteurSeconde = 0

local Mode

local MinuteurEnCours = false
local ChronoEnCours = false
local HorlogeEnCours = false

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
    initHorloge()
end --run


-- Init de l'écran
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
    boxMinuteur:onClick(function() initMinuteur() end)

    imgMinuteur = gui:image(boxMinuteur, "img_minuteur.png", 0, 0, 30, 30)
    lblMinuteur= gui:label(boxMinuteur, 0, 30, 30, 30)
    lblMinuteur:setText("Minuteur")   
    lblMinuteur:setFontSize(8)
    
    -- -------------
    -- Specificité pour le chrono
    -- -------------
    
    -- affichage des boutons de gestion du chrono et du minuteur
    btnStartStop = gui:button(win, 40, 360, 80, 30)
    btnStartStop:setText("Start")
    btnStartStop:onClick(
        function()
            runChronoMinuteur()
        end
    )

    btnReset = gui:button(win, 200, 360, 80, 30)
    btnReset:setText("Reset")
    btnReset:onClick(
        function()
            resetChronoMinuteur()
        end
    )

    --Affichage du label pour le décompte du minuteur
    decompteMinuteur = gui:label(win, 0, 100, 320, 140)
    decompteMinuteur:setFontSize(40)
    decompteMinuteur:setHorizontalAlignment(CENTER_ALIGNMENT)
    

    -- -----------------------------
    -- Spécificité pour le Minuteur
    -- -----------------------------
        
    minuteur = gui:canvas(win, 0, 70, 320, 280)
    minuteur:fillRect(0, 0, 320, 280, COLOR_WHITE)

    -- Gestion des heures
    vListHeure = gui:vlist(minuteur, 55, 10, 50, 250)
    vListHeure:setSpaceLine(3)

    -- Gestion des minutes
    vListMinute = gui:vlist(minuteur, 135, 10, 50, 250)
    vListMinute:setSpaceLine(3)

    -- Gestion des secondes
    vListSeconde = gui:vlist(minuteur, 215, 10, 50, 250)
    vListSeconde:setSpaceLine(3)



    --Activation de la fenetre
    gui:setWindow(win)

end -- init


-- Gere l'affichage des éléments de l'écrean en fonction du mode actif [Horloge, Chrono, Minuteur]
function setEcranMode()

    if (Mode == "Horloge") then
        -- masque les boutons du chrono
        btnStartStop:disable()
        btnReset:disable()

        -- masque le minuteur s'il est définit
        minuteur:disable()
        
        -- affiche l'horloge
        clock:enable()

        if (ChronoEnCours) then
            stopChrono()
        end
        if (MinuteurEnCours) then
            stopMinuteur()
        end

    end
    if (Mode == "Chrono") then

        -- Affiche les boutons start / reset
        btnStartStop:enable()
        btnReset:enable()
    
        -- masque le minuteur 
        minuteur:disable()
        
        -- affiche l'horloge
        clock:enable()
        if (MinuteurEnCours) then
            stopMinuteur()
        end
        if (HorlogeEnCours) then
            stopHorloge()
        end

    end
    
    if (Mode == "Minuteur") then
        -- Arrete les timer Chrono et Horloge
        if (ChronoEnCours) then
            stopChrono()
        end
        if (HorlogeEnCours) then
            stopHorloge()
        end

        -- masque l'horloge
        clock:disable()
        
        -- Affiche les boutons start / reset
        btnStartStop:enable()
        btnReset:enable()

        minuteur:enable()

    end
end

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
    HorlogeEnCours = true
end -- afficheHorloge


-- InitHorloge: Initialisation de l'écran pour l'horloge
function initHorloge()
    -- Modification du titre
    title:setText("Horloge")

    Mode = "Horloge"
    setEcranMode()

    -- affiche l'horloge
    afficheHorloge()

end

----------------------------------------------------------------------
-- Gestion du mode CHRONO
----------------------------------------------------------------------

-- Initialisation du mode Chrono
function initChrono()

    -- Modification du titre
    title:setText("Chronomètre")

    -- Arrete le tmer de l'horloge
    --stopHorloge()

    Mode = "Chrono"
    setEcranMode()


    -- Initialisation du chrono
    tChrono = 0
    afficheChrono()

end --initChrono


-- Arrete le tmer de l'horloge
function stopHorloge()
    time:removeTimeout(idTimerHorloge)
    HorlogeEnCours = false
end


-- Fonction d'affichage du chrono sur le cadran et sur le texte
function afficheChrono()

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

    -- set timer to refresh in 1 sec
    idTimerChrono = time:setTimeout(runChrono, 1000)
    ChronoEnCours = true
end --runChrono


-- reset du chrono
function resetChrono()
    tChrono = 0
    afficheChrono()
end --resetChrono


-- Stop le chrono
function stopChrono()
    time:removeTimeout(idTimerChrono)
    ChronoEnCours = false

    --Si le chrono est en route, modifie le texte et le callback
    btnStartStop:setText("Start")
    btnStartStop:onClick(
        function()
            runChronoMinuteur()
        end
    )
    -- afficheChrono()
end --resetChrono



----------------------------------------------------------------------
-- Gestion du mode Minuteur
----------------------------------------------------------------------

function initMinuteur()
    
--    local winMinuteur = gui:window()

    -- Modification du titre
    title:setText("Minuteur")

    Mode = "Minuteur"
    setEcranMode()

    -- rempli les heures
    for i=0,23 do
        local lblHeure = gui:label(vListHeure, 0, i*20, 50, 20)
        lblHeure:setFontSize(20)
        lblHeure:setHorizontalAlignment(CENTER_ALIGNMENT)
        lblHeure:setText(tostring(i))
        lblHeure:onClick(
            function() 
                if (lbOldlHeure) then
                    lbOldlHeure:setBackgroundColor(COLOR_WHITE)
                end
                lblHeure:setBackgroundColor(COLOR_LIGHT_GREY)
                lbOldlHeure = lblHeure
                vListHeure:setIndex(i)
                minuteurHeure = i
                local strTimer = convert_to_time(minuteurHeure, minuteurMinute, minuteurSeconde) 
                heure_label:setText(strTimer) 
                tMinuteur = minuteurHeure * 3600 + minuteurMinute*60 +minuteurSeconde
            end
        )
        -- gestion de l'init
        if (i==minuteurHeure) then
            vListHeure:setIndex(i)
            lblHeure:setBackgroundColor(COLOR_LIGHT_GREY)
            lbOldlHeure = lblHeure
            --minuteurHeure = 0
        end
    end

    -- rempli les minutes
    for i=0, 59 do
        local lblMinute = gui:label(vListMinute, 0, i*20, 50, 20)
        lblMinute:setFontSize(20)
        lblMinute:setHorizontalAlignment(CENTER_ALIGNMENT)
        lblMinute:setText(tostring(i))
        lblMinute:onClick(
            function() 
                if (lbOldMinute) then
                    -- on supprime le "focus" de l'ancienne sélection
                    lbOldMinute:setBackgroundColor(COLOR_WHITE) 
                end
                -- coloriage de la nouvelle sélection
                lblMinute:setBackgroundColor(COLOR_LIGHT_GREY)
                -- sauvegarde de la sélection
                lbOldMinute = lblMinute
                -- sélection dans la liste (on remat la liste en haut)
                vListMinute:setIndex(i)
                -- sauvegarde de la minute sélectionnée
                minuteurMinute = i
                -- calcul du timer en seconde et affichage au format heure
                local strTimer = convert_to_time(minuteurHeure, minuteurMinute, minuteurSeconde) 
                tMinuteur = minuteurHeure * 3600 + minuteurMinute*60 + minuteurSeconde
                heure_label:setText(strTimer) 
            end
        )
        -- gestion de l'init
        if (i==minuteurMinute) then
            vListMinute:setIndex(i)
            lblMinute:setBackgroundColor(COLOR_LIGHT_GREY)
            lbOldMinute = lblMinute
            --minuteurMinute = 0
        end
        
    end

    -- rempli les secondes
    for i=0, 59 do
        local lblSeconde = gui:label(vListSeconde, 0, i*20, 50, 20)
        lblSeconde:setFontSize(20)
        lblSeconde:setHorizontalAlignment(CENTER_ALIGNMENT)
        lblSeconde:setText(tostring(i))
        lblSeconde:onClick(
            function() 
                if (lbOldSeconde) then
                    lbOldSeconde:setBackgroundColor(COLOR_WHITE)
                end
                lblSeconde:setBackgroundColor(COLOR_LIGHT_GREY)
                lbOldSeconde = lblSeconde
                minuteurSeconde = i
                vListSeconde:setIndex(i)
                local strTimer = convert_to_time(minuteurHeure, minuteurMinute, minuteurSeconde) 
                heure_label:setText(strTimer) 
                tMinuteur = minuteurHeure * 3600 + minuteurMinute*60 +minuteurSeconde
            end
        )
        -- gestion de l'init
        if (i==minuteurSeconde) then
            vListSeconde:setIndex(i)
            lblSeconde:setBackgroundColor(COLOR_LIGHT_GREY)
            lbOldSeconde = lblSeconde
            --minuteurSeconde = 0
        end
    end
    
    local strTimer = convert_to_time(minuteurHeure, minuteurMinute, minuteurSeconde) 
    heure_label:setText(strTimer) 

end -- initMinuteur

function runMinuteur()

    -- si le timer arrive à 0, on fait sonner l'alarme    
    if (tMinuteur == 0) then
        MinuteurEnCours = false
        ringMinuteur()
        heure_label:enable()
        minuteur:enable()
        decompteMinuteur:disable()


    else -- sinon, on affiche le minuteur et on repart pour 1 sec
        afficheMinuteur()
        idTimerMinuteur = time:setTimeout(runMinuteur, 1000)
        MinuteurEnCours = true
    end  

    tMinuteur = tMinuteur - 1


end --runMinuteur

function afficheMinuteur()

    local heure = int(tMinuteur / 3600)
    local minute = int(60* (tMinuteur / 3600 - int(tMinuteur / 3600)))
    local seconde = tMinuteur % 60
    local now = convert_to_time(heure, minute, seconde) 

    -- Affichage du chrono dans le label
    --heure_label:setText(now)
    decompteMinuteur:setText(now)
    
end


-- appel lorsque le minuteur arrive à zéro
function ringMinuteur()
    --stopMinuteur()
    gui:showInfoMessage("Fin minuteur !")
    resetMinuteur()
    btnStartStop:setText("Start")
    btnStartStop:onClick(
        function() 
            --decompteMinuteur:enable()
            --minuteur:disable()
            runChronoMinuteur() 
        end)

end --ringMinuteur


function resetMinuteur()
--    minuteur:enable()
    -- heure_label:enable()
    -- local strTimer = convert_to_time(minuteurHeure, minuteurMinute, minuteurSeconde) 
    -- heure_label:setText(strTimer) 

    --decompteMinuteur:disable()
    tMinuteur = minuteurHeure * 3600 + minuteurMinute*60 +minuteurSeconde

    if (MinuteurEnCours) then
        stopChronoMinuteur()
    end
    minuteur:enable()
    decompteMinuteur:disable()
    heure_label:enable()
end

-- Arrete le timer du minuteur
function stopMinuteur()
    if (MinuteurEnCours) then
        time:removeTimeout(idTimerMinuteur)
        MinuteurEnCours = false
    end
end




-- lance les fonctions run pour le chrono ou le minuteur
function runChronoMinuteur()

    -- Si le chrono / moniteur erst en route, modifie le texte du bouton et prépare le stop
    btnStartStop:setText("Stop")
    btnStartStop:onClick(
        function()
            stopChronoMinuteur()
        end
    )

    if (Mode == "Chrono") then
        runChrono()
    elseif (Mode == "Minuteur") then
        decompteMinuteur:enable()
        minuteur:disable()
        heure_label:disable()

        runMinuteur()
    end

end -- run


function resetChronoMinuteur()

    if (Mode == "Chrono") then
        resetChrono()
    elseif (Mode == "Minuteur") then
        resetMinuteur()
    else
        -- ras
    end

end


function stopChronoMinuteur()
    --Si le chrono est en route, modifie le texte et le callback
    btnStartStop:setText("Start")
    btnStartStop:onClick(function() runChronoMinuteur() end)

    if (Mode == "Chrono") then
        stopChrono()
    elseif (Mode == "Minuteur") then
        stopMinuteur()
    end
end
