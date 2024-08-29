-- Minesweeper Game in Lua

-- Configuration
local level = '{"easy":{"gridSize":8, "mineCount":7, "colorNiveau": 1024},"medium":{"gridSize":12, "mineCount":25, "colorNiveau":31}, "hard":{"gridSize":14, "mineCount":37, "colorNiveau":63488}}'

-- valeur par défaut de la taille de la grille et du nb de mines
local gridSize = 8
local mineCount = 7
local colorNiveau = COLOR_GREEN

-- grille du démineur
local grid = {}
local espacementBox
local sizeCase

-- Element 
local imgStatut, boxActionFlag, lblNbMine
local lblTimer, idTimer
local flagAction = false
local isGameOver

function run()

    -- affiche l'écran de sélection du niveau
    afficheSelectionNiveau()

end

--Fonction helper int : renvoi x tronqué
function int(x)
    return math.floor(x)
end


-- ------------------------------------------------
--     GESTION DE L'ECRAN DE SELECTION DU NIVEAU 
-- ------------------------------------------------


-- Initialise l'écran du jeu
function afficheSelectionNiveau()


        winSelectionNiveau = gui:window()

        lblTitle = gui:label(winSelectionNiveau, 15, 10, 200, 28)
        lblTitle:setFontSize(24)
        lblTitle:setText("Démineur")

        local lblEasy = gui:label(winSelectionNiveau, 50, 150, 200, 30)
        lblEasy:setFontSize(20)
        lblEasy:setHorizontalAlignment(CENTER_ALIGNMENT)
        lblEasy:setVerticalAlignment(CENTER_ALIGNMENT)
        lblEasy:setBorderSize(1)
        lblEasy:setRadius(10)
        lblEasy:setBackgroundColor(COLOR_LIGHT_GREY)
        lblEasy:setText("easy")
        lblEasy:onClick(function() selectionNiveau("easy") end)

        local lblMedium = gui:label(winSelectionNiveau, 50, 200, 200, 30)
        lblMedium:setFontSize(20)
        lblMedium:setHorizontalAlignment(CENTER_ALIGNMENT)
        lblMedium:setVerticalAlignment(CENTER_ALIGNMENT)
        lblMedium:setBorderSize(1)
        lblMedium:setRadius(10)
        lblMedium:setBackgroundColor(COLOR_LIGHT_GREY)
        lblMedium:setText("medium")
        lblMedium:onClick(function() selectionNiveau("medium") end)

        local lblHard = gui:label(winSelectionNiveau, 50, 250, 200, 30)
        lblHard:setFontSize(20)
        lblHard:setHorizontalAlignment(CENTER_ALIGNMENT)
        lblHard:setVerticalAlignment(CENTER_ALIGNMENT)
        lblHard:setBorderSize(1)
        lblHard:setRadius(10)
        lblHard:setBackgroundColor(COLOR_LIGHT_GREY)
        lblHard:setText("hard")
        lblHard:onClick(function() selectionNiveau("hard") end)

    gui:setWindow(winSelectionNiveau)

end


function selectionNiveau(niveau)

    local json_obj = Json:new(level)

    if (json_obj:has_key(niveau)) then
        gridSize = json_obj[niveau]:get_int("gridSize")
        mineCount = json_obj[niveau]:get_int("mineCount")
        colorNiveau = json_obj[niveau]:get_int("colorNiveau")
    end

    prepareGame()

end

-- ------------------------------------------------
--     GESTION DE L'ECRAN DE JEU
-- ------------------------------------------------

function prepareGame()
--    initGrid()
    --placeMines()
    initBoard()
    placeMines()
    calculateAdjacentMines()
end

-- Initialise le board 
function initBoard()

    win = gui:window()

    local sizeBord = 10
    espacementBox = 1
    sizeCase = int( (win:getWidth()  - 2 * sizeBord)/gridSize)
    local offsetHaut = win:getHeight() -  win:getWidth() - sizeBord


    local cnvContour = gui:label(win, int(sizeBord/2), int(sizeBord/2), win:getWidth()-sizeBord, win:getHeight()-sizeBord-2)
    cnvContour:setRadius(10)
    cnvContour:setBorderSize(1)
    cnvContour:setBorderColor(colorNiveau)

    local imgHome = gui:image(win, "home.png", 10, 30, 40, 40, COLOR_WHITE)
    imgHome:onClick(afficheSelectionNiveau)

    lblStatut = gui:label(win, 50, 90, 220, 40)
    lblStatut:setFontSize(24)
    lblStatut:setVerticalAlignment(CENTER_ALIGNMENT)
    lblStatut:setHorizontalAlignment(CENTER_ALIGNMENT)
    lblStatut:setText("")

    lblNbMine = gui:label(win, 55, 30, 80, 40)
    lblNbMine:setBackgroundColor(COLOR_BLACK)
    lblNbMine:setTextColor(COLOR_RED)
    lblNbMine:setFontSize(24)
    lblNbMine:setVerticalAlignment(CENTER_ALIGNMENT)
    lblNbMine:setHorizontalAlignment(CENTER_ALIGNMENT)
    lblNbMine:setText(tostring(mineCount))

    imgStatut = gui:image(win, "new.png", 140, 30, 40, 40, COLOR_WHITE)
    imgStatut:onClick(prepareGame)

    lblTimer = gui:label(win, 185, 30, 80, 40)
    lblTimer:setBackgroundColor(COLOR_BLACK)
    lblTimer:setTextColor(COLOR_RED)
    lblTimer:setFontSize(24)
    lblTimer:setVerticalAlignment(CENTER_ALIGNMENT)
    lblTimer:setHorizontalAlignment(CENTER_ALIGNMENT)
    lblTimer:setText("000")

    boxActionFlag = gui:box(win, 270, 30, 40, 40)

    --    imgActionFlag = gui:image(win, "drapeau.png", 270, 30, 40, 40)

    imgActionFlag = gui:image(boxActionFlag, "drapeau.png", 0, 0, 40, 40)
    imgActionFlag:setTransparentColor(COLOR_WHITE)
    imgActionFlag:onClick(setFlagAction)


    for i = 1, gridSize do
        grid[i] = {}

        for j = 1, gridSize do
            local cnvBox = gui:box(win, sizeBord + sizeCase*(i-1) + espacementBox, offsetHaut + sizeCase*(j-1) - espacementBox, sizeCase - espacementBox, sizeCase - espacementBox)
            cnvBox:setBackgroundColor(COLOR_GREY)
            cnvBox:onClick(function() clickCase(i,j) end)

            grid[i][j] = { mine = false, revealed = false, adjacentMines = 0, flag = false, box=cnvBox }

        end
    end

    if (idTimer) then
        time:removeTimeout(idTimer)
    end
    timer()

    isGameOver = false

    gui:setWindow(win)

end


-- Incrément de l'horloge toutes les secondes
function timer ()
    local intTimer = tonumber(lblTimer:getText())
    lblTimer:setText(tostring(intTimer+1))
    idTimer = time:setTimeout(timer, 1000)
end


-- Function to place mines randomly
function placeMines()
    local placedMines = 0
    while placedMines < mineCount do
        local x = math.random(1, gridSize)
        local y = math.random(1, gridSize)
        if not grid[x][y].mine then
            grid[x][y].mine = true
            placedMines = placedMines + 1
        end
    end
end



-- Active / Desactive le flag
function setFlagAction()

    flagAction = not flagAction

    if (flagAction) then
        boxActionFlag:setBackgroundColor(COLOR_LIGHT_ORANGE)
    else 
        boxActionFlag:setBackgroundColor(COLOR_WHITE)
    end

end


-- gestion d'une partie perdue
function gameover()

    isGameOver = true
    -- arret du timer
    if (idTimer) then
        time:removeTimeout(idTimer)
    end

    lblStatut:setText("Perdu !")
    lblStatut:setTextColor(COLOR_RED)

    imgStatut = gui:image(win, "lost.png", 140, 30, 40, 40, COLOR_WHITE)

    -- reveal all mines
    for x = 1, gridSize do
        for y = 1, gridSize do

            if (grid[x][y].mine and not grid[x][y].revealed) then
                local box = grid[x][y].box
                local img = gui:image(box, "bombe.png",0, 0, sizeCase-espacementBox, sizeCase-espacementBox, COLOR_LIGHT_GREY)
            end
        end
    end

end


-- gestion d'une partie gagnée
function hasWon()

    isGameOver = true
    -- arret du timer
    if (idTimer) then
        time:removeTimeout(idTimer)
    end

    lblStatut:setText("gagné !")
    lblStatut:setTextColor(COLOR_GREEN)


end


-- fonciton qui check si on a gagner
function checkHasWon()

    local count = mineCount

    for x = 1, gridSize do
        for y = 1, gridSize do
            if grid[x][y].mine and grid[x][y].flag then
                count = count - 1
            end
        end
    end
    
    return count == 0

end

-- decrease or increase nb Mines
-- parameter 
function changeMineNum(increase)

    if isGameOver then
        return
    end

    local nbMine = tonumber(lblNbMine:getText())

    if (increase and nbMine < mineCount) then
        lblNbMine:setText(tostring(nbMine + 1))
    elseif (not increase and nbMine > 0) then
        lblNbMine:setText(tostring(nbMine - 1))
    end
    
end

-- Action sur le clik d'une case
function clickCase(x,y)

    if (isGameOver) then
        return
    end

    -- si la case a déja un flag, on ne fait rien
    if (not flagAction and grid[x][y].flag) then
        return
    end

    local box = grid[x][y].box
    -- si on a activé l'action Flag, on affiche/masque le drapeau et on désactive l'action Flag
    if (flagAction) then
        -- setFlagAction()
        if (grid[x][y].flag) then
            grid[x][y].flag = false
            changeMineNum(true)
            box:clear()
        else
            grid[x][y].flag = true
            local img = gui:image(box, "drapeau.png",0, 0, sizeCase-espacementBox, sizeCase-espacementBox, COLOR_LIGHT_GREY)
            changeMineNum(false)
        end
        if checkHasWon() then
            hasWon()
        end
        return
    end
    
    -- si on tombe sur on mine - gameover
    if (grid[x][y].mine) then
        local box = grid[x][y].box
        local img = gui:image(box, "bombe.png",0, 0, sizeCase-espacementBox, sizeCase-espacementBox, COLOR_RED)
        grid[x][y].revealed = true
        gameover()
    else
        revealCell(x, y)
    end

    -- rafraichement de la grille
    updateGrid()
end

function updateGrid()

    for x = 1, gridSize do
        for y = 1, gridSize do

            if (grid[x][y].revealed and not grid[x][y].flag) then

                local box = grid[x][y].box

                if grid[x][y].mine  then
                    -- on est sur une mine !
                else
                    box:setBackgroundColor(COLOR_LIGHT_GREY)
                    if (grid[x][y].adjacentMines > 0) then
                        local lblCount = gui:label(box, 0, 0, sizeCase-espacementBox, sizeCase-espacementBox)
                        lblCount:setBackgroundColor((COLOR_LIGHT_GREY))
                        lblCount:setVerticalAlignment(CENTER_ALIGNMENT)
                        lblCount:setHorizontalAlignment(CENTER_ALIGNMENT)
                        lblCount:setFontSize(14)
                        if (grid[x][y].adjacentMines == 1) then
                            lblCount:setTextColor(COLOR_BLUE)
                        elseif (grid[x][y].adjacentMines == 2) then
                            lblCount:setTextColor(COLOR_GREEN)
                        elseif (grid[x][y].adjacentMines > 2) then
                            lblCount:setTextColor(COLOR_RED)
                        end

                        lblCount:setText(tostring(grid[x][y].adjacentMines))
                    end
                end
            else
                -- case non revélée
            end
        end
    end
end



-- Function to calculate adjacent mines for each cell
function calculateAdjacentMines()
    local directions = {
        {-1, -1}, {-1, 0}, {-1, 1},
        {0, -1},          {0, 1},
        {1, -1}, {1, 0}, {1, 1}
    }

    for x = 1, gridSize do
        for y = 1, gridSize do
            if not grid[x][y].mine then
                local count = 0
                for _, dir in ipairs(directions) do
                    local nx, ny = x + dir[1], y + dir[2]
                    if nx >= 1 and nx <= gridSize and ny >= 1 and ny <= gridSize and grid[nx][ny].mine then
                        count = count + 1
                    end
                end
                grid[x][y].adjacentMines = count
            end
        end
    end
end




-- Fonction récursive pour révéler les cases adjacentes
function revealCell(x, y)

    if x < 1 or x > gridSize or y < 1 or y > gridSize or grid[x][y].revealed then
        return
    end
    grid[x][y].revealed = true
    if grid[x][y].adjacentMines == 0 and not grid[x][y].mine then
        local directions = {
            {-1, -1}, {-1, 0}, {-1, 1},
            {0, -1},          {0, 1},
            {1, -1}, {1, 0}, {1, 1}
        }
        for _, dir in ipairs(directions) do
            revealCell(x + dir[1], y + dir[2])
        end
    end
end

