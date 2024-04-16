function run()
    win = gui:window()

    local json_str = '{"name": {"age": 30, "city": "New York"}}'
    local json_obj = Json:new(json_str)

    print(json_obj:has_key("name")) -- print true
    print(json_obj:has_key("names")) -- print false

    print(json_obj["name"]:get_int("age")) -- print 30
    json_obj["name"]["age"] = json_obj["name"]:get_int("age") + 1 -- use :get(void) to get in string
    print(json_obj["name"]:get_int("age")) -- print 31
    
    todo_label = gui:label(win, 0, 0, 320, 480)
    todo_label:setHorizontalAlignment(CENTER_ALIGNMENT)
    todo_label:setFontSize(15)
    todo_label:setText(json_obj:get())

    gui:setWindow(win)
end