global_list = {}

function checkAlarm()

    for i, v in ipairs(global_list) do
        local h = tonumber(v.time:sub(1, 2))
        local m = tonumber(v.time:sub(4, 5))
        if(h == time:get("h")[1] and m == time:get("mi")[1] and v.enabled == 1) then
            global_list[i].enabled = 0
            saveTable("../../alarms.tab", global_list)

            launch("alarme", {tostring(i)})
        end
    end
end

function run()
    print("run")
    if storage:isFile("../../alarms.tab") then
        global_list = loadTable("../../alarms.tab")
    else
        saveTable("../../alarms.tab", global_list)
    end

    time:setInterval(checkAlarm, 1000)
end