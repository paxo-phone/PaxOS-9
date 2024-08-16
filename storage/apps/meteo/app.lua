local ville
local vListVilles
local selectedLatitude
local selectedLongitude 


-- --------------------------------------------
--   Main function of the application
-- --------------------------------------------

function run()
    initVille()
    initRechercheVille()
end


--Fonction helper int : renvoi x tronqué
function int(x)
    return math.floor(x)
end

-- --------------------------------------------
--   Gestion de la recherche de la ville
-- --------------------------------------------

function initRechercheVille()

    -- Création du nouvel écran
    if winRecherche == nil then
        winRecherche = gui:window()
    end

    inputVille = gui:input(winRecherche, 10, 10)
    --inputVille = gui:label(winRecherche, 20, 20, 150, 20)
    inputVille:setText(ville)
    inputVille:setTitle("Ville") 
    inputVille:onClick(function() saisieVille() end)


    -- btnRechercher = gui:label(winRecherche, 10, 80, 130, 30)
    btnRechercher = gui:image(winRecherche, "loupe.png", 270, 20, 24, 24, COLOR_WHITE)
    btnRechercher:setBorderSize(2)
    btnRechercher:setBorderColor(COLOR_BLACK)
    --btnRechercher:setText("Rechercher")
    --btnRechercher:setHorizontalAlignment(CENTER_ALIGNMENT)
    --btnRechercher:setRadius(5)
    --btnRechercher:setBackgroundColor(COLOR_LIGHT_GREY)
    btnRechercher:onClick(function() rechercherVille(inputVille:getText()) end)

    vListVilles = gui:vlist(winRecherche, 10, 80, 300, 400)
    vListVilles:setBackgroundColor(COLOR_LIGHT_GREY)
    vListVilles:setSpaceLine(10)

    -- Affichage de l'écran
    gui:setWindow(winRecherche)

end -- initMeteo

-- utilisation du keyboard pour saisir la ville à rechercher
function saisieVille()

    local keyboard = gui:keyboard("Ville", inputVille:getText())
    inputVille:setText(keyboard)
    ville = keyboard

end

-- gestion de la recherche des coordonées d'une villes
function rechercherVille(txt)

    if (txt == "") then
        return
    end
    
    local json_str = searchCoord(ville)
--    print(json_str)

    local json_obj = Json:new(json_str)
    local oldSelection = nil
    -- check if we have some results
    if (json_obj:has_key("results")) then
--        print("we have some results")
        clearList()
        json_results = json_obj["results"]

        for num = 0, json_results:size()-1 do
            local name          = json_results:at(num)["name"]:get()
            local country_code  = json_results:at(num)["country_code"]:get()
            local county        = json_results:at(num)["admin1"]:get()
            local latitude      = json_results:at(num)["latitude"]:get()
            local longitude     = json_results:at(num)["longitude"]:get()
            
            local boxVille = displaySelectionVille(num, name, country_code, county, latitude, longitude)
            print(boxVille)
            boxVille:onClick(
                function () 
--                    if (oldSelection ~= nil) then
--                        oldSelection:setBackgroundColor(COLOR_LIGHT_GREY)    
--                    end
--                    boxVille:setBackgroundColor(COLOR_ORANGE) 
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
function searchCoord(ville)

    -- API geocoding 
    local url = "https://geocoding-api.open-meteo.com/v1/search?name="..ville.."&count=5&language=fr&format=json"

    -- Appel de l'URL en HTTP GET

    -- result valid avec Paris
    local result = '{"results":[{"id":2988507,"name":"Paris","latitude":48.85341,"longitude":2.3488,"elevation":42.0,"feature_code":"PPLC","country_code":"FR","admin1_id":3012874,"admin2_id":2968815,"timezone":"Europe/Paris","population":2138551,"postcodes":["75001","75020","75002","75003","75004","75005","75006","75007","75008","75009","75010","75011","75012","75013","75014","75015","75016","75017","75018","75019"],"country_id":3017382,"country":"France","admin1":"Île-de-France","admin2":"Paris"},{"id":4717560,"name":"Paris","latitude":33.66094,"longitude":-95.55551,"elevation":183.0,"feature_code":"PPLA2","country_code":"US","admin1_id":4736286,"admin2_id":4705086,"timezone":"America/Chicago","population":24782,"postcodes":["75460","75461","75462"],"country_id":6252001,"country":"États Unis","admin1":"Texas","admin2":"Comté de Lamar"},{"id":4647963,"name":"Paris","latitude":36.302,"longitude":-88.32671,"elevation":157.0,"feature_code":"PPLA2","country_code":"US","admin1_id":4662168,"admin2_id":4628829,"timezone":"America/Chicago","population":10150,"postcodes":["38242"],"country_id":6252001,"country":"États Unis","admin1":"Tennessee","admin2":"Comté de Henry"},{"id":4303602,"name":"Paris","latitude":38.2098,"longitude":-84.25299,"elevation":257.0,"feature_code":"PPLA2","country_code":"US","admin1_id":6254925,"admin2_id":4285233,"timezone":"America/New_York","population":9870,"postcodes":["40361","40362"],"country_id":6252001,"country":"États Unis","admin1":"Kentucky","admin2":"Comté de Bourbon"},{"id":4246659,"name":"Paris","latitude":39.61115,"longitude":-87.69614,"elevation":220.0,"feature_code":"PPLA2","country_code":"US","admin1_id":4896861,"admin2_id":4237672,"admin3_id":4246667,"timezone":"America/Chicago","population":8432,"postcodes":["61944"],"country_id":6252001,"country":"États Unis","admin1":"Illinois","admin2":"Comté d Edgar","admin3":"Township of Paris"}],"generationtime_ms":1.429081}'
    
    
    
    -- result en erreur - sans correspondance des villes
    -- local result ={'generationtime_ms':0.88202953}

    return result

end


function clearList()
    vListVilles:clear()
end

function displaySelectionVille(row, name, country_code, county, latitude, longitude)

    local boxVille = gui:label(vListVilles, 0, 0, 300, 60)
    boxVille:setBackgroundColor(COLOR_LIGHT_GREY)
    boxVille:setRadius(10)
    boxVille:setFontSize(16)
    local txt = name..", "..country_code..", "..county.."\n".."  - latitude: "..latitude.."\n".."  - longitude: "..longitude
    boxVille:setText(txt)
--    local boxVille = gui:box(vListVilles, 0, row*60, 300, 60)
--    boxVille:setBackgroundColor(COLOR_LIGHT_GREY)

--    lblName = gui:label(boxVille, 0, 0, 300, 20)
--    lblName:setText(name..", "..tostring(country_code)..", "..tostring(county))
--    lblName:setBackgroundColor(COLOR_LIGHT_GREY)

--    lblLatitude = gui:label(boxVille, 20, 20, 300, 20)
--    lblLatitude:setText("- latitude: "..tostring(latitude))
--    lblLatitude:setBackgroundColor(COLOR_LIGHT_GREY)

--    lblLongitude = gui:label(boxVille, 20, 40, 300, 20)
--    lblLongitude:setText("- longitude: "..tostring(longitude))
--    lblLongitude:setBackgroundColor(COLOR_LIGHT_GREY)

    return boxVille
end

function selectionneVille(selected, oldSelected, strVille, strLatitude, strLongitude)

    if(selected == oldSelected) then
        ville = strVille
        latitude = tonumber(strLatitude)
        longitude = tonumber(strLongitude)
        
        print("on selectionne la ville: "..ville.." latitude: "..strLatitude.." longitude: "..strLongitude)
    end

    if (oldSelected ~= nil) then
        oldSelected:setBackgroundColor(COLOR_LIGHT_GREY)    
    end
    selected:setBackgroundColor(COLOR_ORANGE) 
    --oldSelection = boxVille

end
-- --------------------------------------------
--   Gestion de la recherche de la ville
-- --------------------------------------------

function initVille()

    -- read the data from the saved file
    ville = "Paris"
    selectedLatitude = 48.85341
    selectedLongitude = 2.3488
end