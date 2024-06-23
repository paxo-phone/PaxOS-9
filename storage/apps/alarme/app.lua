local tNow
local win
local winAlarme

local clock
local offsetAngle = 270
local idRefreshClock

local oldCoordHeure = {}
local oldCoordMinute = {}
local oldCoordSeconde = {}

local toBeRefreshed = false
local btnToggleAlarme

local BACKGROUND_COLOR = COLOR_WHITE

function int(x)
    return math.floor(x)
end


-- dessine l'horloge
function drawClock()

    centreX = int (win:getWidth() * 0.5)
    centreY = centreX --int (win:getHeight() * 0.5)
    border = 15
    radius = int(centreX - border *2 -20)

    -- couleir d'affichage du cadre
    couleur_clock = COLOR_DARK

    local size_major = 10
    local size_minor = 2
    local espacement = 5

    -- draw clock circle
    clock = gui:canvas(win, 0, 35, int(win:getWidth() - 40), int(win:getWidth()-40))
    -- clock:onClick (function() end)

    --clock:onClick(function() rien() end)

    clock:fillRect(0,0,int(win:getWidth()), int(win:getWidth()),BACKGROUND_COLOR)
--    drawRect_canvas:drawRect(5, 5, 15, 15, COLOR_WARNING)
    --drawCircle_canvas:setBackgroundColor(COLOR_WHITE)
    clock:drawCircle(centreX, centreY, radius, couleur_clock)


    -- loop pour les 5 min
    for i=0,11 do
        -- convert angle in radian
        degres = math.rad(i*30 + offsetAngle)
        
        clock:drawLine(
            int(centreX + math.cos(degres) * (radius - (size_major + espacement))),
            int(centreY + math.sin(degres) * (radius - (size_major + espacement))),
            int(centreX + math.cos(degres) * (radius - espacement)),
            int(centreY + math.sin(degres) * (radius -  espacement)),
            couleur_clock
        )
    end -- loop
    -- loop pour les 1 min
    for i=0,59 do
        -- convert angle in radian
        degres = math.rad(i*6+ offsetAngle)
        
        clock:drawLine(
            int(centreX + math.cos(degres) * (radius - (size_minor + espacement))),
            int(centreY + math.sin(degres) * (radius - (size_minor + espacement))),
            int(centreX + math.cos(degres) * (radius - espacement)),
            int(centreY + math.sin(degres) * (radius -  espacement)),
            couleur_clock
        )
    end -- loop

end -- drawClock


-- renvoi une string au format HH:MM:SS
function convert_to_time (h, min, sec)

    local strHeure = h
    -- Gestion Heure
    if tonumber(h) < 10 then
        if tonumber(h) == nil then
            strHeure = "00"    
        else
            strHeure = "0"..h
        end
    end
    
    local strMin = min
    -- Gestion Minute
    if tonumber(min) < 10 then
        if tonumber(min) == nil then
            strMin = "00"    
        else
            strMin = "0"..min
        end
    end

    local strSec = sec
    -- Gestion Seconde
    if tonumber(sec) < 10 then
        if tonumber(sec) == nil then
            strSec = "00"    
        else
            strSec = "0"..sec
        end
    end

    return strHeure .. ":" .. strMin .. ":" .. strSec
end -- convert_to_time


-- Recopie une structure dans une autre
function copyStruct (source, dest)

   --  print("copyStruct ".. tostring(pairs(source)))
    for i in pairs(source) do
    -- print("dest["..i.."]=".. source[i])
        dest[i] = source[i]
    end

end -- function copyStruct


-- Fonction d'affichage de l'heure en digital, et en mode horloge
function afficheHeure ()

    -- recupere l'heure
    tNow = time:get("h,mi,s")

    local heure = tNow[1]
    local minute = tNow[2]
    local seconde = tNow[3]

    -- local now = convert_to_time(heure, minute, seconde) --= table.concat(tNow, ":")
    -- heure_label:setText(now)

    -- Efface les anciennes aiguilles
    -- Si c'est le 1er affichage, alors il n'y a rien à effacer...
    if toBeRefreshed then
        clock:drawLine(oldCoordHeure.orig_x,oldCoordHeure.orig_y, oldCoordHeure.dest_x,oldCoordHeure.dest_y,BACKGROUND_COLOR)
        clock:drawLine(oldCoordMinute.orig_x,oldCoordMinute.orig_y, oldCoordMinute.dest_x,oldCoordMinute.dest_y,BACKGROUND_COLOR)
        clock:drawLine(oldCoordSeconde.orig_x,oldCoordSeconde.orig_y, oldCoordSeconde.dest_x,oldCoordSeconde.dest_y,BACKGROUND_COLOR)
    end
    toBeRefreshed = true

    -- affichage aiguille Heure

    -- calcul de l'angle pour une heure (avec la variation des minute)
    -- une cadran = 12 h de 60 min
    local nbMinute = heure * 60 + minute
    local radian = math.rad (nbMinute * 360 / (12*60) + offsetAngle)

    local coordHeure = {
        orig_x = centreX,
        orig_y = centreY,
        dest_x = int (centreX + math.cos(radian) * radius * 0.5),
        dest_y = int (centreY + math.sin(radian) * radius * 0.5)
    }

    clock:drawLine(coordHeure.orig_x,coordHeure.orig_y, coordHeure.dest_x,coordHeure.dest_y,COLOR_DARK)
    
    -- Sauvegarde de la position de l'aiguille pour pouvoir l'effacer au prochaine rafraichissement
    copyStruct(coordHeure, oldCoordHeure)

    -- affichage aiguille Minute
    radian = math.rad (minute * 360 / 60 +offsetAngle)

    local coordMinute = {
        orig_x = centreX,
        orig_y = centreY,
        dest_x = int(centreX + math.cos(radian) * radius * 0.75),
        dest_y = int(centreY + math.sin(radian) * radius * 0.75)
    }

    clock:drawLine(coordMinute.orig_x,coordMinute.orig_y, coordMinute.dest_x,coordMinute.dest_y,COLOR_DARK)

    -- Sauvegarde de la position de l'aiguille pour pouvoir l'effacer au prochaine rafraichissement
    copyStruct(coordMinute, oldCoordMinute)

    -- affichage aiguille Seconde
    radian = math.rad (seconde * 360 / 60 +offsetAngle)

    local coordSeconde = {
        orig_x = centreX,
        orig_y = centreY,
        dest_x = int(centreX + math.cos(radian) * radius * 0.75),
        dest_y = int(centreY + math.sin(radian) * radius * 0.75)
    }

    clock:drawLine(coordSeconde.orig_x,coordSeconde.orig_y, coordSeconde.dest_x,coordSeconde.dest_y, COLOR_ERROR)
    -- Sauvegarde de la position de l'aiguille pour pouvoir l'effacer au prochaine rafraichissement
    copyStruct(coordSeconde, oldCoordSeconde)

    -- set timer to refresh in 1 sec
    idRefreshClock = time:setTimeout(afficheHeure, 1000)

end -- afficheHeure


function init()

    -- récupération de la fenetre
    win = gui:window()

    local title=gui:label(win, 35, 10, 144, 28)
    title:setFontSize(24)
    title:setText("Alarme")

    -- Switch activatin alarme
    btnToggleAlarme = gui:switch(win, 250,400)
    btnToggleAlarme:onClick (function() end)

    -- bouton
    testAlarme = gui:button(win, 35, 420, 250, 38)
    testAlarme:setText("Test")
    testAlarme:onClick(
        function ()
            closeAlarme()
        end
    );
    
    gui:setWindow(win)


end -- init


-- Fonction pour fermer la fenetre de l'application Alarme
function closeAlarme()
        
    if (btnToggleAlarme:getState()) then
        ringAlarme()
    end
end -- function closeAlarme



function ringAlarme()

    --     hardware::setVibrator(true);
    print("ringAlarme")

    winAlarme = gui:window()
    gui:setWindow(winAlarme)

    time:removeTimeout(idRefreshClock);


    popupAlarme = gui:canvas(winAlarme, 0, 0, 320, 480)
    popupAlarme:fillRect(0,0,320, 480,COLOR_BLUE)

    print("drawText")
    popupAlarme:drawText(5, 5, "test", COLOR_RED)
    print("drawText OK")

    print("drawTextCentered")
    popupAlarme:drawTextCentered(5, 5, "test", COLOR_RED, true, true)
    print("drawTextCentered OK")


    local back = gui:image(win, "back.png", 30, 30, 18, 18)
    back:onClick(function() 
        time:setTimeout(
            function () 
                --gui:del(winAlarme) 
                --gui:setWindow(win) 
                gui:del(popupAlarme)
                idRefreshClock = time:setTimeout(afficheHeure, 1000)

            end, 
            0
        ) 
    end)

end

function run()

    init()
    drawClock()
    afficheHeure()

end --run