-- ----------------------
--   Main function of the application
-- ----------------------


-- Gestion des couleurs
local colorResult = COLOR_WHITE
local colorButtonNumber = COLOR_GREY
local colorOperation = COLOR_ORANGE
local colorSelectedOperation = COLOR_LIGHT_ORANGE
local colorTopRow = COLOR_LIGHT_GREY


local num1 = 0
local num2 = nil

local nouveauNombre = true
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

     sizeButton = int( (win:getWidth() - 3* spaceBox - 2*spaceBord) / 4)
     widthResult = 300
     heightResult = 70
     sizeLabelOperation = 20
     sizeLabelResult = 40
    
    -- Création de la partie résultat

    boxResultat = gui:box(win, spaceBord, spaceBord, widthResult, heightResult)
    boxResultat:setBorderColor(colorButtonNumber)
    boxResultat:setBorderSize(2)
    boxResultat:setRadius(10)
    

    lblResultat = gui:label(win, spaceBord+boxRadiux, spaceBord+int(boxRadiux/2), widthResult-boxRadiux-spaceBord, sizeLabelResult)
    lblResultat:setFontSize(sizeLabelResult)
    lblResultat:setHorizontalAlignment(RIGHT_ALIGNMENT)
    lblResultat:setText("0")

    lblOperation = gui:label(win, spaceBord+boxRadiux, heightResult +spaceBord - int(boxRadiux/2) -sizeLabelOperation, widthResult-spaceBord-boxRadiux, sizeLabelOperation)
    lblOperation:setFontSize(sizeLabelOperation-2)
    lblOperation:setHorizontalAlignment(RIGHT_ALIGNMENT)

    -- Special
    keyC = drawKey(1, 1, "C", colorTopRow, erase)
    keyPlusMoins= drawKey(1, 2, "+/-", colorTopRow, plusMoins)
    keyPourcentage = drawKey(1, 3, "%", colorTopRow, percent)

    -- Opérations
    keyDivide = drawKey(1, 4, "/", colorOperation, divide)
    --selectionButton(keyDivide)


    keyMultiply = drawKey(2, 4, "x", colorOperation, multiply)
    keySubstract = drawKey(3, 4, "-", colorOperation, substract)
    keyAdd = drawKey(4, 4, "+", colorOperation, add)
    keyEqual = drawKey(5, 4, "=", colorOperation, equal)
    
    keyComma = drawKey(5, 3, ".", colorButtonNumber, comma)

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
    local lblKey = gui:label(win, spaceBox + (colonne -1 ) *(sizeButton+spaceBox) , heightResult+ 2*spaceBox + (ligne -1) *(sizeButton+spaceBox), sizeButton, sizeButton)
    lblKey:setFontSize(40)
    lblKey:setHorizontalAlignment(CENTER_ALIGNMENT)
    lblKey:setVerticalAlignment(CENTER_ALIGNMENT)
    lblKey:setBackgroundColor(colorBouton)
    lblKey:setRadius(radiusBtn)
    lblKey:setText(strkey)

    lblKey:onClick(function () callback(arg) end)

    return lblKey

end


-- Affichage d'un bouton double
function drawDoubleKey(ligne, colonne, strkey, colorBouton, callback, arg)

    local radiusBtn = 10

    local lblKey = gui:label(win, spaceBox + (colonne -1 ) *(sizeButton+spaceBox) , heightResult+ 2*spaceBox + (ligne -1) *(sizeButton+spaceBox), sizeButton*2 +spaceBox, sizeButton)
    lblKey:setFontSize(40)
    lblKey:setHorizontalAlignment(CENTER_ALIGNMENT)
    lblKey:setVerticalAlignment(CENTER_ALIGNMENT)
    lblKey:setBackgroundColor(colorBouton)
    lblKey:setRadius(radiusBtn)
    lblKey:setText(strkey)

    lblKey:onClick(function () callback(arg) end)

    return lblKey

end

-- --------------------------------------
-- GESTION DES TOUCHES ET FONCTIONS MATHS
-- --------------------------------------


-- ajoute la touche clickée au nombre affiché
function clickKey(key)
    local result = lblResultat:getText()

    -- Si c'est un nouveau nombre, on l'enregistre dans num1
    if (nouveauNombre) then
        num1 = tonumber(result)
        nouveauNombre = false
        if (num2 ~= nil) then
            num2 = nil
        end
        lblResultat:setText(key)
    else
            -- si on a plus de 14 caractères, on arrete la saisie
        if (string.len(result) > 13) then
            return    
        else
            -- sinon on concatene le nombre
            lblResultat:setText(lblResultat:getText()..key)
        end
    end

    -- gestion du signe négatif devant le 1er chiffre 0
    if (result == "0") then
        lblResultat:setText(key)
        
    elseif (result == "-0") then
        lblResultat:setText("-"..key)
    end

      --  lblResultat:setText(key)
    
    if (num2 ~= nil) then
        displayOperation(false)
    end



end --clickKey

-- Efface l'écran
function erase()
    lblResultat:setText("0")
    unslectButtons()
    displayOperation(false)
    num1 = 0
    num2 = nil
end

-- divise le nombre affiché par 100 (si différent de 0)
function percent()
    local result = tonumber(lblResultat:getText())
    if (result ~= 0) then
        lblResultat:setText(tostring(result/100))
    end

    if (nouveauNombre) then
        displayOperation(false)
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
    if (nouveauNombre) then
        displayOperation(false)
    end
end


-- division
function divide()
    nouveauNombre = true
    operation = "/"
    num1 = tonumber(lblResultat:getText())
    num2 = nil
    displayOperation(true)
    selectionButton(keyDivide)
end

-- addition
function add()
    nouveauNombre = true
    operation = "+"
    num1 = tonumber(lblResultat:getText())
    num2 = nil

    displayOperation(true)
    selectionButton(keyAdd)
end

-- multiplication
function multiply()
    nouveauNombre = true
    operation = "x"
    num1 = tonumber(lblResultat:getText())
    num2 = nil

    displayOperation(true)
    selectionButton(keyMultiply)
end

-- soustraction
function substract()
    nouveauNombre = true
    operation = "-"
    num1 = tonumber(lblResultat:getText())
    num2 = nil

    displayOperation(true)
    selectionButton(keySubstract)

end

-- egal - calcule de l'opération sélectionnée
function equal()

    -- On déseclectionne les boutons d'opération
    unslectButtons()

    -- si le 2eme nombre est vide, alors, on prend la saisie pour num2
    if (num2 == nil) then
        num2 = tonumber(lblResultat:getText())
    end
        local result
        if (operation == "+") then
            result= num1 + num2
        elseif (operation == "-") then
            result= num1 - num2
        elseif (operation == "/") then
            result= num1 / num2
        elseif (operation == "x") then
            result= num1 * num2
        else
            return
        end
        lblResultat:setText(tostring(result))
        
        -- affichage de l'opération efefctuée
        displayOperation(true)
        
        -- sauvegarde du résultat pour les opérations suivantes
        num1 = result
        nouveauNombre = true    

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
function displayOperation(display)

    if (display) then
        if (num2 == nil) then
            lblOperation:setText(tostring(num1).." "..operation)
        else
            lblOperation:setText(tostring(num1).." "..operation.." "..tostring(num2).." =")
        end
    else
        lblOperation:setText("")
    end
end

function unslectButtons()
    -- déselecionne tous
    keyDivide:setBackgroundColor(colorOperation)
    keyAdd:setBackgroundColor(colorOperation)
    keyMultiply:setBackgroundColor(colorOperation)
    keySubstract:setBackgroundColor(colorOperation)

end


function selectionButton(selectedKey)
    unslectButtons()
    selectedKey:setBackgroundColor(COLOR_LIGHT_ORANGE)

end