function escape_json_string(s)
    return s:gsub('\\', '\\\\')
            :gsub('"', '\"')
            :gsub('\n', '\\n')
            :gsub('\r', '\\r')
            :gsub('\t', '\\t')
            :gsub('/', '\\/')
end

function run()
    local win = gui:window()
    gui:setWindow(win)

    local title = gui:label(win, 10, 10, 300, 40)
    title:setText("Gemini Chat")
    title:setFontSize(24)
    title:setHorizontalAlignment(CENTER_ALIGNMENT)

    local status_label = gui:label(win, 10, 420, 300, 20)
    status_label:setText("Ready")
    status_label:setHorizontalAlignment(CENTER_ALIGNMENT)

    local chat_history_list = gui:vlist(win, 10, 60, 300, 300)
    chat_history_list:setBackgroundColor(COLOR_LIGHT_GREY)

    local messages = {}

    local function add_message_to_history(sender, message)
        table.insert(messages, {sender = sender, message = message})
        local box = gui:box(chat_history_list, 0, 0, 300, 0)
        local label = gui:label(box, 5, 5, 290, 0)
        label:setText(sender .. ": " .. message)
        local height = label:getTextHeight() + 10
        box:setHeight(height)
        label:setHeight(height - 10)
        chat_history_list:setIndex(#messages - 1)
    end

    local message_input = gui:input(win, 10, 370, 220, 40)
    message_input:setTitle("Your message")
    message_input:onClick(function()
        gui:keyboard("Your message", message_input:getText(), function(text)
            message_input:setText(text)
        end)
    end)

    local function on_response(status, data)
        print("Response received: " .. tostring(status) .. " - " .. data)
        time:setTimeout(function()
            status_label:setText("Ready")
            print("Status: " .. tostring(status))
            print("Data: " .. data)
            if true then
                print("Status is 0, attempting to parse JSON response.")
                local ok, json = pcall(function() return Json:new(data) end)
                if not ok then
                    add_message_to_history("Error", "Failed to parse JSON response: " .. data)
                    print("Failed to parse JSON response: " .. data)
                    return
                end

                print("Parsed JSON: ")
                print("  - candidates: " .. tostring(json["candidates"]) .. " - " .. json["candidates"]:get())
                print("  - content: " .. json["candidates"][0]["content"]:get())
                print("  - parts: " .. json["candidates"][0]["content"]["parts"]:get())
                print("  - text: " .. json["candidates"][0]["content"]["parts"][0]["text"]:get())
                add_message_to_history("Gemini", json["candidates"][0]["content"]["parts"][0]["text"]:get())

            else
                print("Error status received: " .. tostring(status) .. " - " .. data)
                add_message_to_history("Error", tostring(status) .. " - " .. data)
            end
        end, 0)
    end

    local send_button = gui:button(win, 240, 370, 70, 40)
    send_button:setText("Send")
    send_button:onClick(function()
        local message = message_input:getText()
        if message == "" then return end

        add_message_to_history("You", message)
        message_input:setText("")
        status_label:setText("Sending...")

        local http_client = network:createHttpClient()
        
        local escaped_message = "Answer in the shortest way possible: " .. escape_json_string(message)
        local post_body = string.format('{"contents":[{"parts":[{"text":"%s"}]}]}', escaped_message)

        http_client:post(
            "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key=...",
            post_body,
            {
                on_complete = on_response,
                headers = {["Content-Type"] = "application/json"}
            }
        )
    end)
end