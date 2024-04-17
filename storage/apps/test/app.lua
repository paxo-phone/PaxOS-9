function run()
    win = gui:window()
    
    todo_label = gui:label(win, 35, 35, 85, 41)
    todo_label:setHorizontalAlignment(LEFT_ALIGNMENT)
    todo_label:setFontSize(36)
    todo_label:setText("Todo")

    -- tasks = {
    --     "Vivamus laoreet.",
    --     "Suspendisse eu ligula.",
    --     "Curabitur ligula sapien",
    --     "Praesent egestas tristique"
    -- }

    tasks_container = gui:box(win, 35, 100, 229, 260)
    tasks_container:setMainColor(COLOR_DARK)


    gui:setWindow(win)
end