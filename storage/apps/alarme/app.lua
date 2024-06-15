local tNow
local win
local heure_label
local clock
local offsetAngle = 270

-- VERRUE  VIRER
local heure = 6 --tNow[0]
local minute = 7 --tNow[1]
local seconde = 9 --tNow[2]

local oldCoordHeure = {}
local oldCoordMinute = {}
local oldCoordSeconde = {}

local toBeRefreshed = false

local BACKGROUND_COLOR = COLOR_WHITE

function int(x)
    return math.floor(x)
end


-- dessine l'horloge
function drawClock()

    centreX = int (win:getWidth() * 0.5)
    centreY = centreX --int (win:getHeight() * 0.5)
    border = 15
    radius = int(centreX - border *2)

    -- couleir d'affichage du cadre
    couleur_clock = COLOR_DARK

    local size_major = 10
    local size_minor = 2
    local espacement = 5

    -- draw clock circle
    clock = gui:canvas(win, 0, 15, int(win:getWidth()), int(win:getWidth()))
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

    print("copyStruct ".. tostring(pairs(source)))
    for i in pairs(source) do
        print("dest["..i.."]=".. source[i])
        dest[i] = source[i]
    end

--    oldCoordHeure = {
--        orig_x = coordHeure.orig_x,
--        orig_y = coordHeure.orig_y,
--        dest_x = coordHeure.dest_x,
--        dest_y = coordHeure.dest_y
--    }
end -- function copyStruct


-- Fonction d'affichage de l'heure en digital, et en mode horloge
function afficheHeure ()

    -- recupere l'heure
    tNow = time:get("h,mi,s")
--    tNow = {heure,minute,seconde}

    local now = convert_to_time(heure, minute, seconde) --= table.concat(tNow, ":")
    heure_label:setText(now)

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

--    oldCoordHeure = {
--        orig_x = coordHeure.orig_x,
--        orig_y = coordHeure.orig_y,
--        dest_x = coordHeure.dest_x,
--        dest_y = coordHeure.dest_y
--    }

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

--    oldCoordMinute = {
--        orig_x = coordMinute.orig_x,
--        orig_y = coordMinute.orig_y,
--        dest_x = coordMinute.dest_x,
--        dest_y = coordMinute.dest_y
--    }

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

    --    oldCoordSeconde = {
--        orig_x = coordSeconde.orig_x,
--        orig_y = coordSeconde.orig_y,
--        dest_x = coordSeconde.dest_x,
--        dest_y = coordSeconde.dest_y
--    }

    -- set timer to refresh in 1 sec
    time:setTimeout(afficheHeure, 1000)

    -- incrément manuel des secondes 
    seconde = seconde +1


end -- afficheHeure


function init()

    win = gui:window()


    heure_label = gui:label(win, 0, 0, 320, 15)
    heure_label:setHorizontalAlignment(CENTER_ALIGNMENT)
    heure_label:setFontSize(15)

    --drawClock()

    gui:setWindow(win)


end -- init

function run()

    init()

    drawClock()

    afficheHeure()


end --run