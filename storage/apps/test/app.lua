function run()
    win = gui:window()

    local f = storage:file("manifest.json",READ)
    f:open()
    print("file:") print(f)
    print(f:readAll())
    f:close()
    f = nil

    local json_str = '{"name": {"age": 30, "city": "New York"}}'
    local json_obj = Json:new(json_str)

    json_obj["name"]:remove("city")
    print(json_obj:has_key("name")) -- print true
    print(json_obj:has_key("names")) -- print false

    print(json_obj["name"]:get_int("age")) -- print 30
    json_obj["name"]["age"] = json_obj["name"]:get_int("age") + 1 -- use :get(void) to get in string
    print(json_obj["name"]:get_int("age")) -- print 31

    local time = time:get("d,mo,y,h,mi,s")


    local str_time = table.concat(time, ",")

    todo_label = gui:label(win, 0, 0, 320, 480)
    todo_label:setHorizontalAlignment(CENTER_ALIGNMENT)
    todo_label:setFontSize(15)
    todo_label:setText(json_obj:get() .. "\n" .. str_time)


    -- Canvas tests
    setPixel_canvas = gui:canvas(win, 5, 215, 25, 25)
    setPixel_canvas:setPixel(12, 12, COLOR_WARNING)

    drawRect_canvas = gui:canvas(win, 35, 215, 25, 25)
    drawRect_canvas:drawRect(5, 5, 15, 15, COLOR_WARNING)

    fillRect_canvas = gui:canvas(win, 65, 215, 25, 25)
    fillRect_canvas:fillRect(5, 5, 15, 15, COLOR_WARNING)

    drawCircle_canvas = gui:canvas(win, 95, 215, 25, 25)
    drawCircle_canvas:drawCircle(12, 12, 10, COLOR_WARNING)

    fillCircle_canvas = gui:canvas(win, 125, 215, 25, 25)
    fillCircle_canvas:fillCircle(12, 12, 10, COLOR_WARNING)

    drawRoundRect_canvas = gui:canvas(win, 155, 215, 25, 25)
    drawRoundRect_canvas:drawRoundRect(5, 5, 15, 15, 3, COLOR_WARNING)

    fillRoundRect_canvas = gui:canvas(win, 185, 215, 25, 25)
    fillRoundRect_canvas:fillRoundRect(5, 5, 15, 15, 3, COLOR_WARNING)

    drawPolygon_canvas = gui:canvas(win, 215, 215, 25, 25)
    drawPolygon_canvas:drawPolygon({{4, 4}, {4, 18}, {18, 18}}, COLOR_WARNING)

    fillConvexPolygon_canvas = gui:canvas(win, 245, 215, 25, 25)
    fillConvexPolygon_canvas:fillPolygon({{4, 4}, {4, 18}, {18, 18}}, COLOR_WARNING)

    drawText_canvas = gui:canvas(win, 275, 215, 25, 25)
    drawText_canvas:drawText(0, 0, "E", COLOR_WARNING)

    drawTextCentered_canvas = gui:canvas(win, 5, 245, 25, 25)
    drawTextCentered_canvas:drawTextCentered(0, 0, "E", COLOR_WARNING, true, true)

    drawTextCenteredInBounds_canvas = gui:canvas(win, 35, 245, 50, 25)
    drawTextCenteredInBounds_canvas:drawRect(0, 0, 40, 20, COLOR_WARNING)
    drawTextCenteredInBounds_canvas:drawTextCenteredInBounds(0, 0, 40, 20, "E", COLOR_WARNING, true, true)

    gui:setWindow(win)
end