
local selectedVille     -- ville sélectionnée pour la météo
local selectedLatitude  -- latitude de la ville sélectionnée
local selectedLongitude -- longitude de la ville sélectionnée

local vListVilles   -- liste des villes recherchée (dans l'écran de recherche des villes)
local vDataMeteo    -- liste des données météo affichées

local modeDisplay = "day"
local lblDate, lblSwitch

-- --------------------------------------------
--   Main function of the application
-- --------------------------------------------

-- run principal de l'application
function run()
    -- read saved Town
    initVille()

    -- init the Meteo Screen
    initMeteo()

    -- if no town is selected (from saved file), display the search screen
    if (selectedVille == nill) then
        initRechercheVille()
    end
end


--Fonction helper int : renvoi x tronqué
function int(x)
    return math.floor(x)
end

-- renvoie la valeur MIN d'un tableau
function min(arr)

    local result = math.huge
    for i = 1, #arr  do
        result = result < arr[i] and result or arr[i]
    end
    return result
end

-- renvoie la valeur MIN d'un tableau
function max(arr)
    local result=0
    for i = 1, #arr  do
        if (tonumber(arr[i]) > result) then
            result = tonumber(arr[i])
        end
    end
    return result
end

-- récupère le jour de la semaine en fonction de la date
-- format américain : le 1er jours est le dimanche
function get_day_of_week(yy, mm, dd) 
    local mmx = mm
  
    if (mm == 1) then  mmx = 13; yy = yy-1  end
    if (mm == 2) then  mmx = 14; yy = yy-1  end
  
    local val8 = dd + (mmx*2) +  math.floor(((mmx+1)*3)/5)   + yy + math.floor(yy/4)  - math.floor(yy/100)  + math.floor(yy/400) + 2
    local val9 = math.floor(val8/7)
    local dw = val8-(val9*7) 
  
    if (dw == 0) then
      dw = 7
    end
    
    return dw
  end
  
-- Extrait du format yyyy-mm-ddThh:min
-- et renvoie une chaine de caractères au bon format
--      time        hh          (18h)
--      shortDate   dd/mm       (17/08)
--      longDate    ddd dd mmm  (samedi 17 aout)
function extractDateTime(strDateTime, format)

    local monthsName = {"Janvier", "Fevrier", "Mars", "Avril", "Mai", "Juin", "Juillet", "Août", "Septembre", "Octobre", "Novembre", "Decembre"}
    local daysOfWeek = { "Dimanche", "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi" }
    
    local year, month, day, hour, minute

    local result = ""

    if (strDateTime) then
        _,_,year,month,day,hour,minute =string.find(strDateTime, "(%d+)-(%d+)-(%d+)T(%d+):(%d+)")
        if (year == nil) then
            _,_,year,month,day =string.find(strDateTime, "(%d+)-(%d+)-(%d+)")
        end
    end
    
    if (format == "time") then
        result = hour.."h"
    elseif (format == "shortDate") then
        -- result = day.." "..monthsName[tonumber(month)]
        result = day.."/"..month
    elseif (format == "longDate") then
        result = daysOfWeek[tonumber(get_day_of_week(year, month, day))].." "..tostring(day).." "..monthsName[tonumber(month)]
    end

    return result
end --extractDateTime


-- --------------------------------------------
--   Gestion de l'écran principal
-- --------------------------------------------

function initMeteo()

    if win == nil then
        win = gui:window()
    end

        --Affichage du Titre
    lblTitle = gui:label(win, 15, 10, 200, 28)
    lblTitle:setFontSize(24)

    lblCoord = gui:label(win, 15, 38, 200, 20)
    lblCoord:setFontSize(16)

    -- Affichage de la ville sélectionnée
    printVille()

    local imgVille = gui:image(win, "arrowr.png", 280, 10, 20, 20)
    imgVille:onClick(initRechercheVille)

    -- récupération et affichage des données météo pour la ville sélectionnée
    if (selectedVille ~= nil) then
        displayMeteoData()
    end

    -- Affichage de la fenetre principale
    gui:setWindow(win)
end


-- Affichage de la ville sélectionnée, ainsi que la latitud et la longitude
function printVille()

    if (selectedVille ~= nil) then
        lblTitle:setText("Météo "..selectedVille)
        if (selectedLatitude ~= nil and selectedLongitude ~= nil ) then
            lblCoord:setText("lat: "..tostring(selectedLatitude).."  -  long: "..tostring(selectedLongitude))
        end
    end
end


-- Extrait les données météo du json et les affiche
function displayMeteoData()

    gui:setWindow(win)
    printVille()

    -- On appelle l'API à la journée ou la semaine
    local json_str
    if (modeDisplay == "week") then
        json_str = getMeteoDataWeek()
    elseif(modeDisplay == "day") then
        json_str = getMeteoDataDay()
    else
        displayError("displayMeteoData : mode "..modeDisplay.." invalide")
        return
    end
 
 
    local json_obj = Json:new(json_str)
    
    -- gestion des erreurs - si erreur il y a
    if (json_obj:has_key("error") ) then

        json_error = json_obj["error"]
        if json_error:get() == "true" then
            displayError(json_obj["reason"]:get())
            return
        end
    else
        -- si pas d'erreur, on masque le label d'erreur
        hideError()
    end

    -- on a des données horaires
    if (json_obj:has_key("hourly") or json_obj:has_key("daily")) then
    
        local json_dateTime, json_hours, json_temperature, json_temperatureMax, json_temperatureMin
        local dataTemperature, dataTemperatureMax, dataTemperatureMin, dataTime
        local mode
        local dataToDisplay = {}

        if (json_obj:has_key("hourly")) then
            json_dateTime = json_obj["hourly"]
            mode = "hourly"

            -- temperature horaire
            dataTemperature = json_dateTime:has_key("temperature_2m")
            json_temperature = json_dateTime["temperature_2m"]
    
        end
        if (json_obj:has_key("daily")) then
            json_dateTime = json_obj["daily"]
            mode = "daily"

            -- temperature Max
            dataTemperatureMax = json_dateTime:has_key("temperature_2m_max")
            json_temperatureMax = json_dateTime["temperature_2m_max"]
    
            -- temperature Min
            dataTemperatureMin = json_dateTime:has_key("temperature_2m_min")
            json_temperatureMin = json_dateTime["temperature_2m_min"]
    
        end

        dataTime = json_dateTime:has_key("time")
        json_hours = json_dateTime["time"]

        if (mode == "hourly") then
            dataProbabilityPrecipitation = json_dateTime:has_key("precipitation_probability")
            json_probaPrecipitation = json_dateTime["precipitation_probability"]
        elseif (mode == "daily") then
            dataProbabilityPrecipitation = json_dateTime:has_key("precipitation_probability_max")
            json_probaPrecipitation = json_dateTime["precipitation_probability_max"]
        end

        dataWeatherCode = json_dateTime:has_key("weather_code")
        json_weatherCode = json_dateTime["weather_code"]
      
        local firstDate = nil
        
        -- gestion des heures
        dataToDisplay.hours = nil
        if (dataTime) then

            dataToDisplay.hours = {}

            -- On recupere la 1ere date pour l'afficher pleinement
            -- on enleve les " des string récupérées
            if (mode == "hourly") then
                firstDate = extractDateTime(string.gsub(json_hours:at(0):get(),"\"", ""), "longDate")
            
                -- on extrait uniquement les heuress
                for i=0, json_hours:size()-1 do
                    local strTime = extractDateTime(string.gsub(json_hours:at(i):get(),"\"", ""), "time")
                    dataToDisplay.hours[i] = strTime
                end
            end

            if (mode == "daily") then            
                
                firstDate = "Semaine du "..extractDateTime(string.gsub(json_hours:at(0):get(),"\"", ""), "shortDate")

                -- on extrait uniquement les date courtes
                for i=0, json_hours:size()-1 do
                    local strTime = extractDateTime(string.gsub(json_hours:at(i):get(),"\"", ""), "shortDate")
                    dataToDisplay.hours[i] = strTime
                end
            end
        end
        

        -- gestion des temperature horaire
        dataToDisplay.temperature = nil
        if (dataTemperature) then

            dataToDisplay.temperature = {}

            for i=0, json_temperature:size()-1 do
                local temp = json_temperature:at(i):get()
                dataToDisplay.temperature[i] = tonumber(temp)
            end
        end

        -- gestion des temperature daily Max
        dataToDisplay.temperatureMax = nil
        if (dataTemperatureMax) then
            dataToDisplay.temperatureMax = {}
            for i=0, json_temperatureMax:size()-1 do
                local temp = json_temperatureMax:at(i):get()
                dataToDisplay.temperatureMax[i] = tonumber(temp)
            end
        end

         -- gestion des temperature daily Min
         dataToDisplay.temperatureMin = nil
         if (dataTemperatureMin) then
            dataToDisplay.temperatureMin = {}
            for i=0, json_temperatureMin:size()-1 do
                local temp = json_temperatureMin:at(i):get()
                dataToDisplay.temperatureMin[i] = tonumber(temp)
            end
        end

        -- gestion de la probablité de précipitation
        dataToDisplay.probaPrecipitation = nil
        if (dataProbabilityPrecipitation) then
            dataToDisplay.probaPrecipitation = {}
            for i=0, json_probaPrecipitation:size()-1 do
                local temp = json_probaPrecipitation:at(i):get()
                dataToDisplay.probaPrecipitation[i] = tonumber(temp)
            end
        end
        
        -- gestion Weather Code
        dataToDisplay.weatherCode = nil
        if (dataWeatherCode) then
            dataToDisplay.weatherCode = {}
            for i=0, json_weatherCode:size()-1 do
                local temp = json_weatherCode:at(i):get()
                dataToDisplay.weatherCode[i] = tonumber(temp)
            end
        end        

            displayHourlyMeteo(dataToDisplay, firstDate)
    end

end --displayMeteoData


-- Switch le mode d'affichage, passant de "day" à "week"
function switchDisplayMeteoData()
    if (modeDisplay == "day") then
        modeDisplay = "week"
    else
        modeDisplay = "day"
    end
    displayMeteoData()

end


-- Affiche les données météo sur l'écran
function displayHourlyMeteo(dataToDisplay, day)

    if (vDataMeteo == nil) then
        vDataMeteo = gui:vlist(win, 15, 100, 285, 360)
        vDataMeteo:setSpaceLine(0)
    else
        vDataMeteo:clear()
        if (lblDate ~= nil) then
            lblDate:disable()
            lblSwitch:disable()
        end
    end

    if (day ~= nil) then
        -- Affichage de la date
        if  (lblDate == nil) then
            lblDate = gui:label(win, 15, 70, 300, 25)
        end
        lblDate:setText(day)
        lblDate:enable()
        
        if (lblSwitch == nil) then
            lblSwitch = gui:label(win, 240, 72, 60, 15)
            lblSwitch:setFontSize(12)
            lblSwitch:setTextColor(COLOR_BLUE)
            lblSwitch:setHorizontalAlignment(CENTER_ALIGNMENT)
            lblSwitch:setVerticalAlignment(CENTER_ALIGNMENT)
            lblSwitch:setBorderSize(1)
            lblSwitch:setRadius(10)
            lblSwitch:setBorderColor(COLOR_GREY)
            lblSwitch:onClick(switchDisplayMeteoData)
            lblSwitch:onClick(switchDisplayMeteoData)
        end
            if (modeDisplay == "day") then
            lblSwitch:setText(" Semaine... ")
        else
            lblSwitch:setText(" Jour... ")
        end
        lblSwitch:enable()
    end

    local lstBox = {}

    -- Affichage des heures
    if (dataToDisplay.hours ~= nil) then
        local i = 0
        for _, hour in ipairs(dataToDisplay.hours) do

            -- ajout de la box
            local boxHour = gui:box(vDataMeteo, 0, 0, 300, 30)
            lstBox[i] = boxHour

            -- ajout heure/day
            local lblHeure = gui:label(boxHour, 0, 0, 300, 30)
            lblHeure:setFontSize(14)
            lblHeure:setVerticalAlignment(CENTER_ALIGNMENT)
            lblHeure:setHorizontalAlignment(LEFT_ALIGNMENT)
            lblHeure:setText(hour)

            -- gestion de l'alternance des rayures
            if (i%2 == 0) then
                lblHeure:setBackgroundColor(COLOR_LIGHT_GREY)
            end
            i = i+1
        end
    end --dataToDisplay.hours

    -- Affichage des images correspondant aux Weather Code 
    if (dataToDisplay.weatherCode ~= nil) then
        local i=0
        local color
        
        for _, code in ipairs(dataToDisplay.weatherCode) do            
            -- recuperation de la box
            local boxHour = lstBox[i]

            if (i%2 == 0) then
                color = COLOR_LIGHT_GREY
            else
                color = COLOR_WHITE
            end

            imgCode = gui:image(boxHour, "images/meteo_"..tonumber(code)..".png", 50, 5, 20, 20, color)

            i=i+1
        end
    end --dataToDisplay.weatherCode

        -- Affichage des images correspondant aux probabilités de precipitation 
        if (dataToDisplay.probaPrecipitation ~= nil) then

            local i=0            
            local color = COLOR_WHITE
            for _, proba in ipairs(dataToDisplay.probaPrecipitation) do            
                -- recuperation de la box
                local boxHour = lstBox[i]
    
                if (i%2 == 0) then
                    color = COLOR_LIGHT_GREY
                else
                    color = COLOR_WHITE
                end

                if (tonumber(proba) == 0) then
                    local imgRain = gui:image(boxHour, "images/no-rain.png", 85, 5, 20, 30, color)
                else
                    local imgRain = gui:image(boxHour, "images/rain.png", 85, 5, 20, 20, color)
                    lblProba = gui:label(boxHour, 110, 5, 30, 20)
                    lblProba:setFontSize(12)
                    lblProba:setBackgroundColor(color)
                    lblProba:setVerticalAlignment(CENTER_ALIGNMENT)
                    lblProba:setText(tostring(proba).."%")
                end
                i=i+1
            end
        end --dataToDisplay.weatherCode

        -- Affichage des images correspondant aux Weather Code 
    if (dataToDisplay.temperature ~= nil) then

        local i=0
        local color = COLOR_WHITE

        local minTemp = int(min(dataToDisplay.temperature))
        local maxTemp = int(max(dataToDisplay.temperature))

        for _, temp in ipairs(dataToDisplay.temperature) do            
            -- recuperation de la box
            local boxHour = lstBox[i]
            
            if (i%2 == 0) then
                colorBackGround = COLOR_LIGHT_GREY
            else
                colorBackGround = COLOR_WHITE
            end
            
            -- gestion du décalage de la temperature
            local decalage = 0
            -- largeur de la partie "temperature"
            local largeur = 105
            decalage = int(largeur * ( int(temp) - minTemp ) / (maxTemp - minTemp))

            -- gestion de la couleur de la temperature
            if (temp<0) then
                color = COLOR_BLUE
            elseif (temp < 10) then
                color = COLOR_LIGHT_BLUE
            elseif (temp < 20) then
                color = COLOR_LIGHT_ORANGE
            elseif (temp < 30) then
                color = COLOR_ORANGE
            else
                color = COLOR_RED
            end

            local lblTemperature = gui:label(boxHour, 140+decalage, 2, 28, 28, colorBackGround)
            lblTemperature:setRadius(14)
            lblTemperature:setHorizontalAlignment(CENTER_ALIGNMENT)
            lblTemperature:setVerticalAlignment(CENTER_ALIGNMENT)
            lblTemperature:setBackgroundColor(color)
            lblTemperature:setFontSize(10)
            lblTemperature:setText(tostring(int(temp)).."°")

            i=i+1
        end
    end

    -- Affichage des temperature Max
    if (dataToDisplay.temperatureMax ~= nil) then
        local i=0
        local colorBackGround            

        for _, tempMax in ipairs(dataToDisplay.temperatureMax) do            
        -- recuperation de la box
            local boxHour = lstBox[i]

            if (i%2 == 0) then
                colorBackGround = COLOR_LIGHT_GREY
            else
                colorBackGround = COLOR_WHITE
            end

            local lblTemperatureMax = gui:label(boxHour, 150, 0, 30, 30, colorBackGround)
            lblTemperatureMax:setHorizontalAlignment(CENTER_ALIGNMENT)
            lblTemperatureMax:setVerticalAlignment(CENTER_ALIGNMENT)
            lblTemperatureMax:setFontSize(15)
            lblTemperatureMax:setBackgroundColor(colorBackGround)
            lblTemperatureMax:setTextColor(COLOR_RED)
            lblTemperatureMax:setText(tostring(int(tempMax)).."°")

            i=i+1
        end
    end --dataToDisplay.temperatureMax

    -- Affichage des temperature Max
       if (dataToDisplay.temperatureMin ~= nil) then
        local i=0
        local colorBackGround            

        for _, tempMin in ipairs(dataToDisplay.temperatureMin) do            
        -- recuperation de la box
            local boxHour = lstBox[i]

            if (i%2 == 0) then
                colorBackGround = COLOR_LIGHT_GREY
            else
                colorBackGround = COLOR_WHITE
            end

            local lblTemperatureMin = gui:label(boxHour, 180, 0, 30, 30, colorBackGround)
            lblTemperatureMin:setHorizontalAlignment(CENTER_ALIGNMENT)
            lblTemperatureMin:setVerticalAlignment(CENTER_ALIGNMENT)
            lblTemperatureMin:setFontSize(15)
            lblTemperatureMin:setBackgroundColor(colorBackGround)
            lblTemperatureMin:setTextColor(COLOR_BLUE)
            lblTemperatureMin:setText(tostring(int(tempMin)).."°")

            i=i+1
        end
    end --dataToDisplay.temperatureMin

end


-- Appel les API de Open Meteo pour la ville sélectionnée
-- Renvoi la retour au format JSON
function getMeteoData()

    local url = "https://api.open-meteo.com/v1/forecast?latitude="..tostring(selectedLatitude).."&longitude="..tostring(selectedLongitude)

    -- Option à ajouter à l'url
    local codeInterpretationMeteo = "hourly=weather_code"
    local codeTimeZoneAuto = "timezone=auto"
    local codeDataUnJour = "forecast_days=1"
    local codeTemperature = "hourly=temperature_2m"

--    url = url.."&"..codeInterpretationMeteo.."&"..codeTimeZoneAuto.."&"..codeDataUnJour.."&"..codeTemperature.."&"...."&"..
  
    local urlDaily = 'https://api.open-meteo.com/v1/forecast?latitude=48.8534&longitude=2.3488&hourly=temperature_2m,precipitation_probability,precipitation,weather_code,wind_speed_10m&forecast_hours=24&timezone=auto'
    local resultDaily = '{"latitude":48.86,"longitude":2.3399997,"generationtime_ms":0.08296966552734375,"utc_offset_seconds":7200,"timezone":"Europe/Paris","timezone_abbreviation":"CEST","elevation":43.0,"hourly_units":{"time":"iso8601","temperature_2m":"°C","precipitation_probability":"%","precipitation":"mm","weather_code":"wmo code","wind_speed_10m":"km/h"},"hourly":{"time":["2024-08-17T00:00","2024-08-17T01:00","2024-08-17T02:00","2024-08-17T03:00","2024-08-17T04:00","2024-08-17T05:00","2024-08-17T06:00","2024-08-17T07:00","2024-08-17T08:00","2024-08-17T09:00","2024-08-17T10:00","2024-08-17T11:00","2024-08-17T12:00","2024-08-17T13:00","2024-08-17T14:00","2024-08-17T15:00","2024-08-17T16:00","2024-08-17T17:00","2024-08-17T18:00","2024-08-17T19:00","2024-08-17T20:00","2024-08-17T21:00","2024-08-17T22:00","2024-08-17T23:00"],"temperature_2m":[22.9,22.6,22.2,21.6,21.3,21.0,20.7,20.5,20.6,20.6,20.3,19.8,20.0,19.9,19.7,19.8,19.8,19.9,19.9,19.0,18.6,18.4,18.1,17.9],"precipitation_probability":[23,41,58,68,77,87,80,72,65,53,41,29,44,59,74,78,83,87,84,80,77,80,84,87],"precipitation":[0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.10,0.80,5.40,4.90,5.60,7.30,4.60,1.60,0.50,0.20,0.50,0.00,0.00,0.00,0.00],"weather_code":[3,3,3,3,3,3,3,3,3,61,61,63,63,63,63,63,61,61,61,61,61,3,3,3],"wind_speed_10m":[4.8,5.2,4.9,4.9,4.8,5.4,4.3,3.5,2.6,3.4,5.2,3.9,3.2,4.2,2.9,9.0,8.2,10.5,7.9,10.8,7.4,6.3,5.7,5.6]}}'
    -- faire un appel à l'url

    -- url pour vue à 7 jours
    local url7jours = "https://api.open-meteo.com/v1/forecast?latitude=48.8534&longitude=2.3488&daily=weather_code,temperature_2m_max,temperature_2m_min,precipitation_probability_max&forecast_hours=24"
    --local url7jours = 'https://api.open-meteo.com/v1/forecast?latitude=48.8534&longitude=2.3488&daily=weather_code,temperature_2m_max,temperature_2m_min,precipitation_hours,wind_speed_10m_max,wind_gusts_10m_max'
    local result7jours = '{"latitude":48.86,"longitude":2.3399997,"generationtime_ms":0.09894371032714844,"utc_offset_seconds":0,"timezone":"GMT","timezone_abbreviation":"GMT","elevation":43.0,"daily_units":{"time":"iso8601","weather_code":"wmo code","temperature_2m_max":"°C","temperature_2m_min":"°C","precipitation_probability_max":"%"},"daily":{"time":["2024-08-19","2024-08-20","2024-08-21","2024-08-22","2024-08-23","2024-08-24","2024-08-25"],"weather_code":[3,80,3,2,3,81,2],"temperature_2m_max":[23.3,22.6,21.6,25.0,24.9,26.5,20.8],"temperature_2m_min":[13.1,14.8,12.3,12.3,14.6,16.4,13.7],"precipitation_probability_max":[0,94,0,0,3,19,10]}}'


    local urlError = 'https://api.open-meteo.com/v1/forecast?latitude=48.8534&longitude=2.3488&hourly=temperature_2m,precipitation_probability,precipitation,weather_code,wind_speed_10m&forecast_hours=24&timezone=aut'
    local resultError = '{"reason":"Invalid timezone","error":true}'

    return result7jours

end

-- Appel API pour récupérer les données Météo pour Une Semaine
function getMeteoDataWeek()

    local url = "https://api.open-meteo.com/v1/forecast?latitude=48.8534&longitude=2.3488&daily=weather_code,temperature_2m_max,temperature_2m_min,precipitation_probability_max&forecast_hours=24"
    local result = '{"latitude":48.86,"longitude":2.3399997,"generationtime_ms":0.09894371032714844,"utc_offset_seconds":0,"timezone":"GMT","timezone_abbreviation":"GMT","elevation":43.0,"daily_units":{"time":"iso8601","weather_code":"wmo code","temperature_2m_max":"°C","temperature_2m_min":"°C","precipitation_probability_max":"%"},"daily":{"time":["2024-08-19","2024-08-20","2024-08-21","2024-08-22","2024-08-23","2024-08-24","2024-08-25"],"weather_code":[3,80,3,2,3,81,2],"temperature_2m_max":[23.3,22.6,21.6,25.0,24.9,26.5,20.8],"temperature_2m_min":[13.1,14.8,12.3,12.3,14.6,16.4,13.7],"precipitation_probability_max":[0,94,0,0,3,19,10]}}'

    return result
end

-- Appel API pour récupérer les données Météo pour Une journée
function getMeteoDataDay()

    local url = 'https://api.open-meteo.com/v1/forecast?latitude=48.8534&longitude=2.3488&hourly=temperature_2m,precipitation_probability,precipitation,weather_code,wind_speed_10m&forecast_hours=24&timezone=auto'
    local result = '{"latitude":48.86,"longitude":2.3399997,"generationtime_ms":0.08296966552734375,"utc_offset_seconds":7200,"timezone":"Europe/Paris","timezone_abbreviation":"CEST","elevation":43.0,"hourly_units":{"time":"iso8601","temperature_2m":"°C","precipitation_probability":"%","precipitation":"mm","weather_code":"wmo code","wind_speed_10m":"km/h"},"hourly":{"time":["2024-08-17T00:00","2024-08-17T01:00","2024-08-17T02:00","2024-08-17T03:00","2024-08-17T04:00","2024-08-17T05:00","2024-08-17T06:00","2024-08-17T07:00","2024-08-17T08:00","2024-08-17T09:00","2024-08-17T10:00","2024-08-17T11:00","2024-08-17T12:00","2024-08-17T13:00","2024-08-17T14:00","2024-08-17T15:00","2024-08-17T16:00","2024-08-17T17:00","2024-08-17T18:00","2024-08-17T19:00","2024-08-17T20:00","2024-08-17T21:00","2024-08-17T22:00","2024-08-17T23:00"],"temperature_2m":[22.9,22.6,22.2,21.6,21.3,21.0,20.7,20.5,20.6,20.6,20.3,19.8,20.0,19.9,19.7,19.8,19.8,19.9,19.9,19.0,18.6,18.4,18.1,17.9],"precipitation_probability":[23,41,58,68,77,87,80,72,65,53,41,29,44,59,74,78,83,87,84,80,77,80,84,87],"precipitation":[0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.10,0.80,5.40,4.90,5.60,7.30,4.60,1.60,0.50,0.20,0.50,0.00,0.00,0.00,0.00],"weather_code":[3,3,3,3,3,3,3,3,3,61,61,63,63,63,63,63,61,61,61,61,61,3,3,3],"wind_speed_10m":[4.8,5.2,4.9,4.9,4.8,5.4,4.3,3.5,2.6,3.4,5.2,3.9,3.2,4.2,2.9,9.0,8.2,10.5,7.9,10.8,7.4,6.3,5.7,5.6]}}'

    return result
end



function interpretWheatherCode(code)
--    0	            Clear sky
--    1, 2, 3	    Mainly clear, partly cloudy, and overcast
--    45, 48	    Fog and depositing rime fog
--    51, 53, 55	Drizzle: Light, moderate, and dense intensity
--    56, 57	    Freezing Drizzle: Light and dense intensity
--    61, 63, 65	Rain: Slight, moderate and heavy intensity
--    66, 67	    Freezing Rain: Light and heavy intensity
--    71, 73, 75	Snow fall: Slight, moderate, and heavy intensity
--    77	        Snow grains
--    80, 81, 82	Rain showers: Slight, moderate, and violent
--    85, 86	    Snow showers slight and heavy
--    95 	        Thunderstorm: Slight or moderate
--    96, 99 	    Thunderstorm with slight and heavy hail

    
end


-- Affiche un message d'erreur
function displayError(errorTxt)

    if (lblError == nil) then
        lblError = gui:label(win, 50, 200, 250, 50)
    end
    lblError:enable()
    lblError:setTextColor(COLOR_RED)
    lblError:setHorizontalAlignment(CENTER_ALIGNMENT)
    lblError:setText("Error:\n"..errorTxt)
end


-- masque le message d'erreur
function hideError()
    if (lblError ~= nil) then
        lblError:disable()
    end
end

-- --------------------------------------------
--   Gestion de la recherche de la ville
-- --------------------------------------------

function initRechercheVille()

    -- Création du nouvel écran
    if winRecherche == nil then
        winRecherche = gui:window()
    end

    btnBack = gui:image(winRecherche, "back.png", 20, 20, 18, 18)
    btnBack:onClick(displayMeteoData)

    inputVille = gui:input(winRecherche, 60, 20)
    if (selectedVille == nil) then
        inputVille:setText("")
    else
        inputVille:setText(selectedVille)
    end
    inputVille:setTitle("Ville") 
    inputVille:onClick(saisieVille)

    vListVilles = gui:vlist(winRecherche, 15, 100, 295, 400)
    vListVilles:setBackgroundColor(COLOR_LIGHT_GREY)
    vListVilles:setSpaceLine(10)

    if (selectedVille ~= nil and selectedVille ~= "") then
        rechercherVille(selectedVille)
    end


    -- Affichage de l'écran
    gui:setWindow(winRecherche)

end -- initMeteo

-- utilisation du keyboard pour saisir la ville à rechercher
function saisieVille()

    local keyboard = gui:keyboard("Ville", inputVille:getText())
    inputVille:setText(keyboard)
    selectedVille = keyboard
    if (selectedVille ~= nil and selectedVille ~= "") then
        rechercherVille(selectedVille)
    end

end

-- gestion de la recherche des coordonées d'une villes
function rechercherVille(txt)

    if (txt == "") then
        return
    end
    
    local json_str = getCoordData(ville)
--    print(json_str)

    local json_obj = Json:new(json_str)
    local oldSelection = nil

    -- check if we have some results
    if (json_obj:has_key("results")) then
--        print("we have some results")
        clearList()
        json_results = json_obj["results"]

        for num = 0, json_results:size()-1 do
            local name          = string.gsub(json_results:at(num)["name"]:get(),"\"", "")
            local country_code  = string.gsub(json_results:at(num)["country_code"]:get(),"\"", "")
            local county        = string.gsub(json_results:at(num)["admin1"]:get(),"\"", "")
            local latitude      = json_results:at(num)["latitude"]:get()
            local longitude     = json_results:at(num)["longitude"]:get()
            
            local boxVille = displaySelectionVille(num, name, country_code, county, latitude, longitude)
            boxVille:onClick(
                function () 
                    selectionneVille(boxVille, oldSelection, name, latitude, longitude) 
                    oldSelection = boxVille
                end
            )
        end
    else
        print("no results found")
    end

end

-- Appel de l'API de geocoding de Open Meteo
-- renvoi le json renvoyé par l'api
function getCoordData(ville)

    -- API geocoding 
    local url = "https://geocoding-api.open-meteo.com/v1/search?name="..selectedVille.."&count=5&language=fr&format=json"

    -- Appel de l'URL en HTTP GET

    -- result valid avec Paris
    local result = '{"results":[{"id":2988507,"name":"Paris","latitude":48.85341,"longitude":2.3488,"elevation":42.0,"feature_code":"PPLC","country_code":"FR","admin1_id":3012874,"admin2_id":2968815,"timezone":"Europe/Paris","population":2138551,"postcodes":["75001","75020","75002","75003","75004","75005","75006","75007","75008","75009","75010","75011","75012","75013","75014","75015","75016","75017","75018","75019"],"country_id":3017382,"country":"France","admin1":"Île-de-France","admin2":"Paris"},{"id":4717560,"name":"Paris","latitude":33.66094,"longitude":-95.55551,"elevation":183.0,"feature_code":"PPLA2","country_code":"US","admin1_id":4736286,"admin2_id":4705086,"timezone":"America/Chicago","population":24782,"postcodes":["75460","75461","75462"],"country_id":6252001,"country":"États Unis","admin1":"Texas","admin2":"Comté de Lamar"},{"id":4647963,"name":"Paris","latitude":36.302,"longitude":-88.32671,"elevation":157.0,"feature_code":"PPLA2","country_code":"US","admin1_id":4662168,"admin2_id":4628829,"timezone":"America/Chicago","population":10150,"postcodes":["38242"],"country_id":6252001,"country":"États Unis","admin1":"Tennessee","admin2":"Comté de Henry"},{"id":4303602,"name":"Paris","latitude":38.2098,"longitude":-84.25299,"elevation":257.0,"feature_code":"PPLA2","country_code":"US","admin1_id":6254925,"admin2_id":4285233,"timezone":"America/New_York","population":9870,"postcodes":["40361","40362"],"country_id":6252001,"country":"États Unis","admin1":"Kentucky","admin2":"Comté de Bourbon"},{"id":4246659,"name":"Paris","latitude":39.61115,"longitude":-87.69614,"elevation":220.0,"feature_code":"PPLA2","country_code":"US","admin1_id":4896861,"admin2_id":4237672,"admin3_id":4246667,"timezone":"America/Chicago","population":8432,"postcodes":["61944"],"country_id":6252001,"country":"États Unis","admin1":"Illinois","admin2":"Comté d Edgar","admin3":"Township of Paris"}],"generationtime_ms":1.429081}'
    
    -- result en erreur - sans correspondance des villes
    -- local result ={'generationtime_ms':0.88202953}

    return result

end

-- vide la liste de villes
function clearList()
    vListVilles:clear()
end


-- ajoute une ville dans la liste de villes sélectionnables
function displaySelectionVille(row, name, country_code, county, latitude, longitude)

    local boxVille = gui:label(vListVilles, 0, 0, 290, 60)
    boxVille:setBackgroundColor(COLOR_LIGHT_GREY)
    boxVille:setRadius(10)
    boxVille:setFontSize(16)

    local txt = name..", "..country_code..", "..county.."\n".."  - latitude: "..latitude.."\n".."  - longitude: "..longitude
    boxVille:setText(txt)

    return boxVille
end


-- ---------------------------------------------------------------------------------------------
-- selction de la ville
-- si la ville est déjà sélectionnée, on valide la sélection et on retourne à l'écran principale
-- ---------------------------------------------------------------------------------------------
function selectionneVille(selected, oldSelected, strVille, strLatitude, strLongitude)

    -- si la ville est déjà sélectionnée, on valide la sélection
    if(selected == oldSelected) then
        selectedVille = strVille
        selectedLatitude = tonumber(strLatitude)
        selectedLongitude = tonumber(strLongitude)
        --gui:setWindow(win)
        saveVille()
        displayMeteoData()
        return
    end

    -- on désecletionne l'ancienne ville
    if (oldSelected ~= nil) then
        oldSelected:setBackgroundColor(COLOR_LIGHT_GREY)    
    end
    -- on sélectionne la nouvelle
    selected:setBackgroundColor(COLOR_ORANGE) 

end
-- --------------------------------------------
--   Gestion de la recherche de la ville
-- --------------------------------------------

    -- read the data from the saved file
    function initVille()


    selectedVille = nil
    selectedLatitude = nil
    selectedLongitude = nil

    local fileMeteo = storage:file ("meteo.json", READ)
    if (not storage:isFile("meteo.json")) then
        print("pas de fichier")
        return
    end

    fileMeteo:open()
    str_meteo = fileMeteo:readAll()
    fileMeteo:close()
    fileMeteo = nil

    json_meteo = Json:new(str_meteo)

    if (json_meteo:has_key("ville")) then
        selectedVille = string.gsub(json_meteo["ville"]:get(),"\"", "")
        selectedLatitude = tonumber(json_meteo["latitude"]:get())
        selectedLongitude = tonumber(json_meteo["longitude"]:get())
    else
        print("pas json_meteo:has(ville)")
    end
end

function saveVille()

    local fileMeteo = storage:file ("meteo.json", WRITE)

    if (fileMeteo == nil) then
        print("error saving meteo file")
        return
    end

    local str_Json = '{"ville":\"'..selectedVille..'\", "latitude":'..selectedLatitude..', "longitude":'..selectedLongitude..'}'
    --json_meteo = Json:new(str_Json)

--    json_meteo["ville"] = selectedVille
--    json_meteo["latitude"] = selectedLatitude
--    json_meteo["longitude"] = selectedLongitude
    fileMeteo:open()
    fileMeteo:write(str_Json)

    fileMeteo:close()

end