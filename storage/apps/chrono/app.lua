function run()
    local win = gui:window()
    gui:setWindow(win)

    local timeLabel = gui:label(win, 100, 50, 220, 60)
    timeLabel:setFontSize(30)
    timeLabel:setText("00:00.00")

    local lapsList = gui:vlist(win, 70, 130, 260, 240)

    local startBtn = gui:button(win, 40, 400, 60, 40)
    startBtn:setText("Start")

    local resetBtn = gui:button(win, 130, 400, 60, 40)
    resetBtn:setText("Reset")

    local lapBtn = gui:button(win, 220, 400, 60, 40)
    lapBtn:setText("Lap")
    
    local running = false
    local start_ts = 0
    local elapsed = 0
    local interval_id
    local lap_count = 0

    local function formatTime(ms)
        local totalHundredths = math.floor(ms / 10)
        local hh = totalHundredths % 100
        local sec = math.floor(totalHundredths / 100) % 60
        local min = math.floor(totalHundredths / 6000)
        return string.format("%02d:%02d.%02d", min, sec, hh)
    end

    local function updateDisplay()
        local now = time:monotonic()
        local current = elapsed
        if running then
            current = current + (now - start_ts)
        end
        timeLabel:setText(formatTime(current))
    end

    local function startTicker()
        if interval_id then
            time:removeInterval(interval_id)
        end
        interval_id = time:setInterval(updateDisplay, 50)
    end

    startBtn:onClick(function()
        if not running then
            start_ts = time:monotonic()
            running = true
            startBtn:setText("Pause")
            startTicker()
        else
            elapsed = elapsed + (time:monotonic() - start_ts)
            running = false
            startBtn:setText("Start")
            if interval_id then
                time:removeInterval(interval_id)
            end
        end
    end)

    resetBtn:onClick(function()
        running = false
        start_ts = 0
        elapsed = 0
        timeLabel:setText("00:00.00")
        startBtn:setText("Start")
        lap_count = 0
        gui:del(lapsList)
        lapsList = gui:vlist(win, 70, 130, 260, 240)
        if interval_id then
            time:removeInterval(interval_id)
        end
    end)

    lapBtn:onClick(function()
        if running then
            lap_count = lap_count + 1
            local now = elapsed + (time:monotonic() - start_ts)
            local label = gui:label(lapsList, 0, 0, 240, 17)
            label:setText(string.format("%d. %s", lap_count, formatTime(now)))
            label:setFontSize(20)
        end
    end)
end

function quit()
    gui:setWindow(nil)
end