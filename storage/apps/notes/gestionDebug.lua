-- ------------------
-- FONCTION DE DEBUG
-- ------------------


function debugPrint(t, indent)

    local debugType = true

    if not indent then print("[DEBUG] "..getVariableName(t))end
    indent = indent or 0
    local spacing = string.rep("  ", indent)

    if type(t) == "table" then
        for k, v in pairs(t) do
            if type(v) == "table" then
                if debugType then
                    print(spacing .."("..type(k)..") "..tostring(k) .. ":")
                else
                    print(spacing ..tostring(k))
                end
                debugPrint(v, indent + 1)
            else
                if debugType then
                    print(spacing .."("..type(k)..") "..tostring(k) .. ": " .. tostring(v) .." ("..type(v)..")")
                else
                    print(spacing ..tostring(k) .. ": " .. tostring(v))
                end
            end
        end
    else
        if debugType then
            print(spacing .. "("..type(t)..") "..tostring(t))
        else
            print(spacing ..tostring(t))            
        end
    end
end


function getVariableName(var)
    local name
--    for k, v in pairs(_G) do
    for k, v in pairs(_G) do
            if v == var then
            name = k
            break
        end
    end
    if name then
        return name
    else
        return "Variable not found"
    end
end



function printEvent(event)

    print("event: ".. event.name)
    print("date debut: ".. tostring(event.debut.day).."/".. tostring(event.debut.month).."/".. tostring(event.debut.year))
    print(  " debut: "..tostring(event.debut.heure)..":"..tostring(event.debut.minute) )
    print(  " fin: "..tostring(event.fin.heure)..":"..tostring(event.fin.minute) )

end

