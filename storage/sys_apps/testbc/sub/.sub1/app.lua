function run()
    print("run sub1 app in background: will display counter2 to 10 then run the settings app")
    local counter = 0

    local function printCounter()
        counter = counter + 1
        print("Counter 2: " .. counter .. " sec")
        if(counter == 10) then
            launch("settings")
        end
    end

    time:setInterval(printCounter, 1000)
end