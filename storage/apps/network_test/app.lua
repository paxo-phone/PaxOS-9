function run()
    local win = gui:window()
    gui:setWindow(win)

    local label = gui:label(win, 10, 10, 300, 40)
    label:setText("Network Test")
    label:setFontSize(24)
    label:setHorizontalAlignment(CENTER_ALIGNMENT)

    local status_label = gui:label(win, 10, 60, 300, 40)
    status_label:setText("Press button to start test")
    status_label:setHorizontalAlignment(CENTER_ALIGNMENT)

    local response_box = gui:label(win, 10, 150, 300, 250)
    response_box:setBackgroundColor(COLOR_LIGHT_GREY)
    response_box:setText("")

    local function on_test_complete(status, data)
        if status == 0 then -- OK
            status_label:setText("Request successful!")
            response_box:setText(data)
        else
            status_label:setText("Request failed with status: " .. tostring(status))
            response_box:setText(data)
        end
    end

    local get_button = gui:button(win, 50, 420, 100, 30)
    get_button:setText("GET Test")
    get_button:onClick(function()
        status_label:setText("Sending GET request...")
        response_box:setText("")
        local http_client = network:createHttpClient()
        http_client:get("https://www.google.com", {
            on_complete = on_test_complete
        })
    end)

    local post_button = gui:button(win, 170, 420, 100, 30)
    post_button:setText("POST Test")
    post_button:onClick(function()
        status_label:setText("Sending POST request...")
        response_box:setText("")
        local http_client = network:createHttpClient()
        http_client:post("http://httpbin.org/post", "hello=world", {
            on_complete = on_test_complete
        })
    end)
end