-- ----------------------
--   Main function of the application
-- ----------------------


-- Gestion des couleurs
local colorResult = COLOR_WHITE
local colorButtonNumber = COLOR_GREY
local colorOperation = COLOR_ORANGE
local colorTopRow = COLOR_LIGHT_GREY


local num1 = 0
local num2 = 0

local operationEnCours = false
local operation = ""

function run()
    initCalculatrice()
end


--Fonction helper int : renvoi x tronqué
function int(x)
    return math.floor(x)
end

-- Dessine la calculatrice
function initCalculatrice()

    -- Création du nouvel écran
    win = gui:window()

     boxRadiux = 10
     spaceBox = 10
     spaceBord = 10

     sizeButton = 65 --int( (win:getWidth() - 5* spaceBox) / 4)
     widthResult = 300
     heightResult = 80
     sizeLabelOperation = 20
    
    -- Création de la partie résultat

    lblResultat = gui:label(win, spaceBord, spaceBord, widthResult, heightResult)
    lblResultat:setFontSize(40)
    lblResultat:setHorizontalAlignment(RIGHT_ALIGNMENT)
    lblResultat:setBorderColor(colorButtonNumber)
    lblResultat:setBorderSize(2)
    lblResultat:setRadius(10)
    lblResultat:setText("0")

    lblOperation = gui:label(win, spaceBord+boxRadiux, heightResult +spaceBord - boxRadiux -sizeLabelOperation, widthResult-spaceBord-boxRadiux, sizeLabelOperation)
    --lblOperation = gui:label(win, 20, 80, 300, 10)
    lblOperation:setFontSize(sizeLabelOperation-2)
    lblOperation:setHorizontalAlignment(RIGHT_ALIGNMENT)
    --lblOperation:setText("...")
    --lblOperation:setBorderColor(colorButtonNumber)
    --lblOperation:setBorderSize(2)


    -- Special
    keyC = drawKey(1, 1, "C", colorTopRow, erase)
    keyPlusMoins= drawKey(1, 2, "+/-", colorTopRow, plusMoins)
    keyPourcentage = drawKey(1, 3, "%", colorTopRow, percent)

    -- Opérations
    keyDivide = drawKey(1, 4, "/", colorOperation, divide)
    keyMultiply = drawKey(2, 4, "x", colorOperation, multiply)
    keySubstract = drawKey(3, 4, "-", colorOperation, substract)
    keyAdd = drawKey(4, 4, "+", colorOperation, add)
    keyEqual = drawKey(5, 4, "=", colorOperation, equal)
    
    keyComma = drawKey(5, 3, ",", colorButtonNumber, comma)

    -- nombre
    key0 = drawDoubleKey(5, 1, "0", colorButtonNumber, clickKey, "0")
    key1 = drawKey(4, 1, "1", colorButtonNumber, clickKey, "1")
    key2 = drawKey(4, 2, "2", colorButtonNumber, clickKey, "2")
    key3 = drawKey(4, 3, "3", colorButtonNumber, clickKey, "3")
    key4 = drawKey(3, 1, "4", colorButtonNumber, clickKey, "4")
    key5 = drawKey(3, 2, "5", colorButtonNumber, clickKey, "5")
    key6 = drawKey(3, 3, "6", colorButtonNumber, clickKey, "6")
    key7 = drawKey(2, 1, "7", colorButtonNumber, clickKey, "7")
    key8 = drawKey(2, 2, "8", colorButtonNumber, clickKey, "8")
    key9 = drawKey(2, 3, "9", colorButtonNumber, clickKey, "9")

    -- Affichage de l'écran
    gui:setWindow(win)

end -- initCalculatrice


-- Affichage d'un bouton simple
-- ligne: 
function drawKey(ligne, colonne, strkey, colorBouton, callback, arg)

    local radiusBtn = 10

    local boxKey = gui:box(win, spaceBox + (colonne -1 ) *(sizeButton+spaceBox) , heightResult+ 2*spaceBox + (ligne -1) *(sizeButton+spaceBox), sizeButton, sizeButton)
    boxKey:setBorderColor(colorBouton)
    boxKey:setBackgroundColor(colorBouton)
    boxKey:setBorderSize(2)
    boxKey:setRadius(radiusBtn)

--    local lblKey = gui:label(win, int(radiusBtn/2) +spaceBox + (colonne -1 ) *(sizeButton+spaceBox) , int(radiusBtn/2) +heightResult+ 2*spaceBox + (ligne -1) *(sizeButton+spaceBox), sizeButton-radiusBtn, sizeButton-radiusBtn)
    local lblKey = gui:label(boxKey, int(radiusBtn / 2), int(radiusBtn / 2), sizeButton-radiusBtn, sizeButton-radiusBtn)
    lblKey:setFontSize(40)
    lblKey:setHorizontalAlignment(CENTER_ALIGNMENT)
    lblKey:setVerticalAlignment(CENTER_ALIGNMENT)
    lblKey:setBackgroundColor(colorBouton)
    lblKey:setText(strkey)

    boxKey:onClick(function () callback(arg) end)

    return boxKey

end


-- Affichage d'un bouton double
function drawDoubleKey(ligne, colonne, strkey, colorBouton, callback, arg)

    local radiusBtn = 10

    local boxKey = gui:box(win, spaceBox + (colonne -1 ) *(sizeButton+spaceBox) , heightResult+ 2*spaceBox + (ligne -1) *(sizeButton+spaceBox), sizeButton*2 +spaceBox, sizeButton)
    boxKey:setBorderColor(colorBouton)
    boxKey:setBackgroundColor(colorBouton)
    boxKey:setBorderSize(2)
    boxKey:setRadius(radiusBtn)

--    local lblKey = gui:label(win, int(radiusBtn/2) +spaceBox + (colonne -1 ) *(sizeButton+spaceBox) , int(radiusBtn/2) +heightResult+ 2*spaceBox + (ligne -1) *(sizeButton+spaceBox), sizeButton-radiusBtn, sizeButton-radiusBtn)
    local lblKey = gui:label(boxKey, radiusBtn, radiusBtn, 2*sizeButton+spaceBox-radiusBtn, sizeButton-radiusBtn)
    lblKey:setFontSize(40)
    lblKey:setHorizontalAlignment(CENTER_ALIGNMENT)
    lblKey:setVerticalAlignment(CENTER_ALIGNMENT)
    lblKey:setBackgroundColor(colorBouton)
    lblKey:setText(strkey)

    boxKey:onClick(function () callback(arg) end)

    return boxKey

end

-- --------------------------------------
-- GESTION DES TOUCHES ET FONCTIONS MATHS
-- --------------------------------------


-- ajoute la touche clickée au nombre affiché
function clickKey(key)
    local result = lblResultat:getText()

    if (operationEnCours) then
        num1 = tonumber(result)
        lblResultat:setText(key)
        return
    end

    if (result == "0") then
        lblResultat:setText(key)
    else
        lblResultat:setText(lblResultat:getText()..key)
    end
end --clickKey


-- Efface l'écran
function erase()
    lblResultat:setText("0")
end

-- divise le nombre affiché par 100 (si différent de 0)
function percent()
    local result = tonumber(lblResultat:getText())
    if (result ~= 0) then
        lblResultat:setText(tostring(result/100))
    end
end

-- ajoute ou enleve le signe moins devant le nombre
function plusMoins()
    local result = lblResultat:getText()
    if (string.len(result)>0) then
        if (string.sub(result, 0,1) == "-") then
            lblResultat:setText(string.sub(result, 2, -1))
        else
            lblResultat:setText("-"..result)
        end
    else
        return
    end
end


-- division
function divide()
    operationEnCours = true
    operation = "/"

end

-- addition
function add()
    operationEnCours = true
    operation = "+"
end

-- multiplication
function multiply()
    operationEnCours = true
    operation = "x"
end

-- soustraction
function substract()
    operationEnCours = true
    operation = "-"
end

-- egal - calcule de l'opération sélectionnée
function equal()

    if (operationEnCours) then
        num2 = tonumber(lblResultat:getText())
        local result 
        if (operation == "+") then
            result= num1 + num2
        elseif (operation == "-") then
            result= num1 - num2
        elseif (operation == "/") then
            result= num1 / num2
        elseif (operation == "x") then
            result= num1 * num2
        end
        lblResultat:setText(tostring(result))
        
        -- affichage de l'opération efefctuée
        displayOperation()
        
        -- sauvegarde du résultat pour les opérations suivantes
        num1 = result
        operationEnCours = false
    end        
    --lblResultat:setText(key)

end

-- Ajoute une virgule s'il n'y en a pas déjà une
function comma()

    -- on recherche %. avec le caractère d'échappement
    -- si pas trouvé, on ajoute un point au nombre affiché
    if (string.find(lblResultat:getText(), "%.") == nil) then
        lblResultat:setText(lblResultat:getText()..".")
    end
end

-- Affichage de l'opération effectuée
function displayOperation()

    lblOperation:setText(tostring(num1).." "..operation.." "..tostring(num2))

end