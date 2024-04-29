local direction = "down"

function int(x)
    return math.floor(x)
end

local canvaSize = {w = 310, h = 470}
local gridSize = {w = int(canvaSize.w/20), h = int(canvaSize.h/20)}
local factor = canvaSize.w / gridSize.w

local snake = {{x=3, y=2}, {x=2, y=2}, {x=1, y=2}}
local food = {x=math.random(gridSize.w), y=math.random(gridSize.h)}

function drawSnake()
    for i, part in ipairs(snake) do
        drawRect_canvas:fillRect(int(part.x*factor), int(part.y*factor), 15, 15, COLOR_WARNING)
    end
end

function drawFood()
    drawRect_canvas:fillRect(int(food.x*factor), int(food.y*factor), 15, 15, COLOR_ERROR)
end

function updateSnake()
    local head = {x=snake[1].x, y=snake[1].y}

    if direction == "right" then
        head.x = head.x + 1
    elseif direction == "left" then
        head.x = head.x - 1
    elseif direction == "up" then
        head.y = head.y - 1
    elseif direction == "down" then
        head.y = head.y + 1
    end

    -- Vérifier les collisions avec les bords
    if head.x < 1 or head.x > gridSize.w or head.y < 1 or head.y > gridSize.h then
        gui:setWindow(nil)
    end

    -- Vérifier les collisions avec le corps du serpent
    for i = 2, #snake do
        if head.x == snake[i].x and head.y == snake[i].y then
            gui:setWindow(nil)
            os.exit() -- Arrêter le jeu
        end
    end

    table.insert(snake, 1, head)

    if snake[1].x == food.x and snake[1].y == food.y then
        -- Générer une nouvelle position pour la nourriture
        -- qui n'est pas sur le serpent
        repeat
            food = {x=math.random(gridSize.w), y=math.random(gridSize.h)}
        until not isFoodOnSnake(food)
    else
        local tail = table.remove(snake)
        -- Effacer le dernier point de la queue
        drawRect_canvas:fillRect(int(tail.x*factor), int(tail.y*factor), 15, 15, COLOR_DARK)
    end

    -- Afficher que la tête
    drawRect_canvas:fillRect(int(head.x*factor), int(head.y*factor), 15, 15, COLOR_WARNING)
end

function isFoodOnSnake(food)
    for i, part in ipairs(snake) do
        if food.x == part.x and food.y == part.y then
            return true
        end
    end
    return false
end

function update()
    updateSnake()
    drawFood()
end

function run()
    win=gui:window()

    drawRect_canvas = gui:canvas(win, 5, 5, 310, 470)

    drawRect_canvas:onTouch(function(a)
        local diff = {x=snake[1].x*factor-a[1], y=snake[1].y*factor-a[2]}
        print(diff.x)
        print(diff.y)
        if (math.abs(diff.x) > math.abs(diff.y)) then
            if(direction ~= "right" and direction ~= "left") then
                if(diff.x < 0) then
                    direction = "right"
                else
                    direction = "left"
                end
            end
        else
            if(direction ~= "down" and direction ~= "up") then
                if(diff.y < 0) then
                    direction = "down"
                else
                    direction = "up"
                end
            end
        end
        print(direction)
    end)

    drawRect_canvas:fillRect(0, 0, 310, 470, COLOR_DARK)
    drawSnake()
    drawFood()

    time:setInterval(update, 300)

    gui:setWindow(win)
end
