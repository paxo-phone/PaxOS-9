local lblTestCouleur
local groupRadio
local radioText,radioBackground, radioBorder
local textColor, backgroundColor, borderColor
local sliderR, sliderG, sliderB


function int(x)
    return math.floor(x)
end

function initColorScreen()

    local winColor = manageWindow()

    local btnBack = gui:image(winColor, "back.png",10, 20, 20, 20, color.white)
    btnBack:onClick(run)

    local title = gui:label(winColor, 40, 20, 280, 40)
    title:setText("Couleurs")
    title:setTextColor(97, 183, 157)
    title:setFontSize(30)

    local topRadio = 80

    -- Gestion des boutons radios
    radioText = gui:radio(winColor, 20, topRadio)
    local lblRadioText = gui:label(winColor, 45, topRadio, 50, 20)
    lblRadioText:setText("Texte")

    radioBackground = gui:radio(winColor, 100, topRadio)
    local lblRadioBackground = gui:label(winColor, 125, topRadio, 90, 20)
    lblRadioBackground:setText("Background")

    radioBorder = gui:radio(winColor, 220, topRadio)
    local lblRadioBorder = gui:label(winColor, 245, topRadio, 50, 20)
    lblRadioBorder:setText("Border")

    groupRadio = {radioText,radioBackground, radioBorder }
    radioText:onClick(function () selectRadio(groupRadio, radioText) end)
    radioBackground:onClick(function () selectRadio(groupRadio, radioBackground) end)
    radioBorder:onClick(function () selectRadio(groupRadio, radioBorder) end)

    -- radioText:setState(true)

    -- Gestin grille de couleurs
    local colors = {color.success, color.warning, color.error, color.white, color.black, color.red, color.green, color.blue, color.yellow, color.grey, color.magenta, color.cyan, color.violet, color.orange, color.pink, color.lightOrange, color.lightGreen, color.lightBlue, color.lightGrey} --, color.greenPaxo }

    -- Label Test Couleurs
    lblTestCouleur = gui:label(winColor, 200, 370, 50, 60)
    lblTestCouleur:setText("Test")
    lblTestCouleur:setBorderSize(1)
    lblTestCouleur:setHorizontalAlignment(CENTER_ALIGNMENT)
    lblTestCouleur:setVerticalAlignment(CENTER_ALIGNMENT)

    textColor = settings.getTextColor()
    backgroundColor = settings.getBackgroundColor()
    borderColor = settings.getBorderColor()

    lblTestCouleur:setTextColor(textColor)
    lblTestCouleur:setBackgroundColor(backgroundColor)
    lblTestCouleur:setBorderColor(borderColor)

    -- Add Save button
    local btnSave = gui:button(winColor, 260, 405, 50, 30)
    btnSave:setText("Save")
    btnSave:onClick(saveColor)


    local width = 300
    local height = 200
    local gridRow = 4
    local gridCol = 5


    local grille = gui:canvas(winColor, 10, 140, width, height)
    grille:onTouch(
        function (coord)

            local x = int(coord[1] / (width/gridCol))
            local y = int(coord[2] / (height/gridRow))

            local selectedColor = color.white 
            if y*gridCol+x <= #colors then
                selectedColor = colors[y*gridCol+x +1]
            end
            setPredefinedColor(selectedColor)
        end
    )

    -- Création de la grille de couleur
    for j=0,gridRow-1 do 
        for i=0,gridCol-1 do 
            local couleur = color.white
            if j*gridCol+i < #colors then
                couleur = colors[j*gridCol+i +1]
            end

            grille:fillRect(int(i * width/gridCol)+1, int(j* height / gridRow )+1, int(width/gridCol )-2 , int(height/gridRow)-2, couleur)

        end
    end

    -- slider Rouge
    sliderR = gui:slider(winColor, 20, 370, 150, 20, 0, 255, 100)
    sliderR:setValueColor(color.red)
    sliderR:onChange(setRGBColor)

    -- slider Green
    sliderG = gui:slider(winColor, 20, 400, 150, 20, 0, 255, 100)
    sliderG:setValueColor(color.green)
    sliderG:onChange(setRGBColor)

    -- Slider Blue
    sliderB = gui:slider(winColor, 20, 430, 150, 20, 0, 255, 100)
    sliderB:setValueColor(color.blue)
    sliderB:onChange(setRGBColor)

    selectRadio(groupRadio, radioText)
end

function setRGBColor()

    local couleur =color.toColor(sliderR:getValue(),sliderG:getValue(),sliderB:getValue() )
    if radioText:getState() then 
        textColor = couleur
        lblTestCouleur:setTextColor(textColor)
    elseif radioBackground:getState() then
        backgroundColor = couleur
        lblTestCouleur:setBackgroundColor(backgroundColor)
    elseif radioBorder:getState() then
        borderColor = couleur
        lblTestCouleur:setBorderColor(borderColor)
    else
        return
    end

end

function setPredefinedColor(col)
    -- print("setPredefinedColor")

    if radioText:getState() then 
        textColor = col
        lblTestCouleur:setTextColor(textColor)
    elseif radioBackground:getState() then
        backgroundColor = col
        lblTestCouleur:setBackgroundColor(backgroundColor)
    elseif radioBorder:getState() then
        borderColor = col
        lblTestCouleur:setBorderColor(borderColor)
    else
        return
    end
        
    local selectedColor = table.pack(color.toRGB(col))
    if selectedColor.n ==3 then
        sliderR:setValue(selectedColor[1])
        sliderG:setValue(selectedColor[2])
        sliderB:setValue(selectedColor[3])
    end

end




function saveColor()
    settings.setBackgroundColor(backgroundColor, true)
    settings.setTextColor(textColor, true)
    settings.setBorderColor(borderColor, true)

    run()

end



-- select the radio button, and unselect the other ones
-- adjust the sliders values based on the rigth color
function selectRadio(groupRadio, radio)
    for i,r in ipairs(groupRadio) do
        r:setState(r == radio)        
    end
    
    local selectedColor
    if radio == radioBackground then
        selectedColor = table.pack(color.toRGB(backgroundColor))
    elseif radio == radioBorder then
        selectedColor = table.pack(color.toRGB(borderColor))
    elseif radio == radioText then
        selectedColor = table.pack(color.toRGB(textColor))
    else
        return
    end
    if selectedColor.n ==3 then
        sliderR:setValue(selectedColor[1])
        sliderG:setValue(selectedColor[2])
        sliderB:setValue(selectedColor[3])
    end


end
