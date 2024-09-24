
local filename = "data/notes.dat"
local oldWin

local data = {}



-- -------------------------------------------
-- Fonction initiale de l'application Notes
-- -------------------------------------------

function run()
    
    loadDataFile()
    initNotes()
end


-- -------------------------------------------
-- gestion des datas
-- -------------------------------------------

function loadDataFile()
-- check if a data file exists

    if storage:isFile(filename) then
        data = loadTable(filename)
        sortTable()

        if not data then data = {} end
    else
        print("[loadDataFile] file not found "..filename)
    end

end


-- Tri la table Data
function sortTable()

    table.sort(data, function(a,b)       
        if not a.closed and not b.closed or a.closed and  b.closed then 
            return a.UID > b.UID
        elseif a.closed and not b.closed then 
            return false 
        elseif not a.closed and b.closed then
            return true
        else
            return a.UID > b.UID
        end
    end)
end


-- Efface les notes qui sont "closed"
function cleanData()

    local tmpData = {}
    for i, note in ipairs(data) do 
        if not note.closed then
            table.insert(tmpData, note)
        end
    end
    data = tmpData
    saveTable(filename,data)
end

-- -------------------------------------------
-- Ecran Principal
-- -------------------------------------------

function initNotes()    

    win = manageWindow()

    -- sortTable()

    local Frame = gui:box(win, 0, 0, 320, 65)
    Frame:setBackgroundColor(COLOR_WARNING)
    Frame:onClick(initNotes)

    local TitleApp = gui:label(Frame, 40, 15, 144, 28)
    TitleApp:setText("Mes Notes")
    TitleApp:setBackgroundColor(COLOR_WARNING)
    TitleApp:setFontSize(32)

    local addLabel = gui:label(win, 270, 420, 40, 40)
    addLabel:setRadius(20)
    addLabel:setBackgroundColor(COLOR_BLACK)

    local addIcon = gui:image(addLabel, "plus.png", 12, 12, 16, 16, COLOR_BLACK)
    addLabel:onClick(editNote)

    local SettingsIcon = gui:image(Frame, "menu.png", 10, 20, 20, 20, COLOR_WARNING)
    SettingsIcon:onClick(Settings)

    local lstNotes = gui:vlist(win, 20, 80, 290, 320)
    lstNotes:setSpaceLine(5)

    local dataNote = {}
    local nbNotes=0

    local hauteurNote = 35

    for i, note in ipairs(data) do
        nbNotes = nbNotes+1
        local boxNote = gui:box(lstNotes, 0, 0, 290, hauteurNote)
        boxNote:setBorderSize(1)
        boxNote:setBorderColor(COLOR_LIGHT_GREY)
        dataNote[i]={}

        local chkClosed = gui:checkbox(boxNote, 5, 5)
        chkClosed:onClick(
            function()
                local check = dataNote[i].check
                local label = dataNote[i].label
                note.closed = check:getState()
                writeNote(label, note, hauteurNote)
                saveTable(filename, data)
            end
        )
        chkClosed:setState(note.closed)

        local lblNoteTitle = gui:canvas(boxNote, 30, 1, 258, hauteurNote-2)
        lblNoteTitle:fillRect(0, 0, 258, hauteurNote, COLOR_WHITE)

        dataNote[i].check = chkClosed
        dataNote[i].label = lblNoteTitle
        writeNote(lblNoteTitle, note, hauteurNote)

        lblNoteTitle:onClick(
            function() 
                if not dataNote[i].check:getState() then
                    editNote(note.UID)  
                end
            end
        )
    end

    if nbNotes==0 then
        local lblEmpty = gui:label(lstNotes, 0, 0, 290, 30)
        lblEmpty:setText("aucune note")
        lblEmpty:setTextColor(COLOR_GREY)
    end

end

function launchNote(check, note, hauteurNote)
    if not check.getState() then
        editNote(note.UID)
    end
end

function writeNote(label, note, hauteurNote)
    local fontSize = 22
    if note.closed then
        label:fillRect(0, 0, 280, hauteurNote, COLOR_WHITE)
        label:drawText(10, 1, note.Title, COLOR_GREY, fontSize)
        label:drawLine(10, 15, math.floor(#note.Title * 12), 15, COLOR_GREY)                    
    else
        label:fillRect(0, 0, 280, hauteurNote, COLOR_WHITE)
        label:drawText(10, 1, note.Title, COLOR_BLACK, fontSize)
    end
    label:drawText(190, hauteurNote-10-1, note.date, COLOR_GREY, 10)
end




-- Récupère la note identifiée par UID
-- si UID est nil ou la note UID n'exoste pas, créé une nouvelle note vierge et la renvoi
function getNote(UID)


    local note
    if not UID or not data[UID] then
        note = {}
        note.UID = createUID()
        note.Title = ""
        note.TextNote = ""
        note.closed = false
    else
        note = data[UID]
    end

    return note

end




-- --------------------------------------------
-- Ecran de création / modification d'une note
-- @param UID ID de la note 
-- --------------------------------------------
function editNote(UID)

    local winNote = manageWindow()

    local Frame = gui:box(winNote, 0, 0, 320, 65)
    Frame:setBackgroundColor(COLOR_WARNING)


    local newNote = getNote(UID)

    local TitleApp = gui:label(Frame, 20, 20, 250, 28)
    TitleApp:setText("Edition d'une Note")
    TitleApp:setBackgroundColor(COLOR_WARNING)
    TitleApp:setFontSize(32)

    local inputTitleNote = gui:input(winNote, 35, 80, 250, 40)
    inputTitleNote:setTitle("Titre")
    inputTitleNote:setText(newNote.Title)
    inputTitleNote:onClick(
        function () 
            local keyboard = gui:keyboard("Titre de la note", newNote.Title)
            inputTitleNote:setText(keyboard)
            newNote.Title = keyboard
        end
    )

--    local TextNote = gui:input(winNote, 35, 216, 250, 40)

    local lblNote = gui:label(winNote, 35, 150, 250, 20)
    lblNote:setFontSize(16)
    lblNote:setTextColor(COLOR_GREY)
    lblNote:setText("Note")

    local TextNote = gui:label(winNote, 35, 170, 250, 200)
    TextNote:setBorderSize(1)
    TextNote:setBorderColor(COLOR_BLACK)
    TextNote:setText(newNote.TextNote)
    TextNote:onClick(function () 
        local keyboard = gui:keyboard("Titre de la note", newNote.TextNote)
        TextNote:setText(keyboard)
        newNote.TextNote = keyboard
        end
    )

    edit = gui:label(winNote, 35, 405, 250, 30)
    edit:setHorizontalAlignment(CENTER_ALIGNMENT)
    edit:setHorizontalAlignment(CENTER_ALIGNMENT)
    edit:setBorderSize(1)
    edit:setBorderColor(COLOR_BLACK)
    edit:setRadius(15)
    edit:setText("Sauvegarder")
    edit:onClick(function()
        newNote.date = getDate()
        data[newNote.UID] = newNote
        saveTable(filename, data)
        initNotes()
    end)

    cancel = gui:label(winNote, 35, 440, 250, 30)
    cancel:setHorizontalAlignment(CENTER_ALIGNMENT)
    cancel:setHorizontalAlignment(CENTER_ALIGNMENT)
    cancel:setBorderColor(COLOR_BLACK)
    cancel:setBorderSize(1)
    cancel:setRadius(15)
    cancel:setText("Annuler")
    cancel:onClick(initNotes)

end 




function Settings()

    local winSetting = manageWindow()

    local Frame = gui:box(winSetting, 0, 0, 320, 65)
    Frame:setBackgroundColor(COLOR_WARNING)

    local TitleApp = gui:label(Frame, 40, 15, 250, 28)
    TitleApp:setText("A propos ...")
    TitleApp:setBackgroundColor(COLOR_WARNING)
    TitleApp:setFontSize(32)

    local imgBack = gui:image(winSetting, "back.png", 10, 15, 20, 30, COLOR_WARNING )
    imgBack:onClick(initNotes)


    local nbNotesClosed = 0
    local nbNotes = 0
    for i, note in pairs(data) do 
        nbNotes = nbNotes+1
        if note.closed then nbNotesClosed = nbNotesClosed +1 end
    end

    local lblNbNotes = gui:label(winSetting, 30, 150, 200, 40)
    lblNbNotes:setText("Nombre de Notes:")

    local lblCountNbNotes = gui:label(winSetting, 270, 150, 30, 40)
    lblCountNbNotes:setText(tostring(nbNotes))

    local lblNbNotesClosed = gui:label(winSetting, 30, 200, 200, 40)
    lblNbNotesClosed:setText("Nombre de Notes cloturées:")

    local lblCountNbNotesClosed = gui:label(winSetting, 270, 200, 30, 40)
    lblCountNbNotesClosed:setText(tostring(nbNotesClosed))



    local btnEffacerNotesClosed = gui:label(winSetting, 50, 420, 220, 30)
    btnEffacerNotesClosed:setRadius(15)
    btnEffacerNotesClosed:setBorderSize(1)
    btnEffacerNotesClosed:setBorderColor(COLOR_GREY)
    btnEffacerNotesClosed:setHorizontalAlignment(CENTER_ALIGNMENT)
    btnEffacerNotesClosed:setVerticalAlignment (CENTER_ALIGNMENT)
    btnEffacerNotesClosed:setText("Effacer les notes cloturées")
    btnEffacerNotesClosed:onClick(function() cleanData() Settings() end)

end



-- ----------------------------------------------
-- function manageWindow
--
--   Créer une nouvelle fenetre et l'affiche
--   supprime l'ancienne fenetre affichée
--   @return  nouvelle fenetre créée
--   @prerequisite variable oldWin est définie
-- ----------------------------------------------
function manageWindow()
    local win

    win = gui:window()
    gui:setWindow(win)

    if oldWin then 
        gui:del(oldWin) 
        oldWin  =nil 
    end
    
    oldWin = win
    return win
end


-- Créer un UID unique
function createUID()
    math.randomseed(time:monotonic())
    return "N"..time:monotonic()..math.random(99999)
end


function getDate()

    local today = time:get("y,mo,d,h,mi,s")
--    return {today[1], today[2], today[3]}
    
        local year = today[1]
        local month = string.format("%02d", today[2])
        local day = string.format("%02d", today[3])
        local heure = string.format("%02d", today[4])
        local minute = string.format("%02d", today[5])
    
        return day.."/"..month.."/"..year.." "..heure..":"..minute

end

