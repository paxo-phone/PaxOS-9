function run()
    print("run baground testbc app, no displayed: show conter to 10 and run settings app")
    print(time)
    local counter = 0

    local function printCounter()
        counter = counter + 1
        print("Counter: " .. counter .. " sec")
        if(counter == 10) then
            launch("settings")
        end
    end

    time:setInterval(printCounter, 1000)
end