require "gestionDebug.lua"


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
    else
        print("[Notesdata file not found")
    end

end


-- -------------------------------------------
-- Ecran Principal
-- -------------------------------------------

function initNotes()    

    win = manageWindow()

    local Frame = gui:box(win, 0, 0, 320, 65)
    Frame:setBackgroundColor(COLOR_WARNING)
    Frame:onClick(initNotes)

    local TitleApp = gui:label(Frame, 20, 20, 144, 28)
    TitleApp:setText("Mes Notes")
    TitleApp:setBackgroundColor(COLOR_WARNING)
    TitleApp:setFontSize(32)

    local addLabel = gui:label(Frame, 270, 20, 40, 40, COLOR_WARNING)
    addLabel:setRadius(20)
    addLabel:setTransparentColor(COLOR_WARNING)
    addLabel:setBackgroundColor(COLOR_BLACK)

    local addIcon = gui:image(addLabel, "plus.png", 12, 12, 16, 16, COLOR_BLACK)

    addLabel:onClick(editNote)

    local SettingsBox = gui:box(win, 225, 115, 75, 30)
    SettingsBox:setBackgroundColor(COLOR_WARNING)
    SettingsBox:setRadius(10)

    local SettingsIcon = gui:image(SettingsBox, "Settings.png", 26, 5, 20, 20, COLOR_WARNING)
    SettingsBox:onClick(Settings)


    debugPrint(data)
    local listO = gui:vlist(win, 35, 150, 250, 280)

    for i, note in pairs(data) do

        print( "notes "..tostring(i))
        local case = gui:box(listO, 0, 36, 250, 36, COLOR_LIGHT_BLUE)
        case:setBackgroundColor(COLOR_LIGHT_BLUE)
        case:setRadius(10)
        local lblNoteTitle = gui:label(case, 21, 0, 230, 18)
        lblNoteTitle:setBackgroundColor(COLOR_LIGHT_BLUE)
        lblNoteTitle:setText(note.Title)
        lblNoteTitle:setFontSize(16)

        case:onClick(function() editNote(note.UID) end)
    end

end



-- Récupère la note identifiée par UID
-- si UID est nil ou la note UID n'exoste pas, créé une nouvelle note vierge et la renvoi
function getNote(UID)

    local note
    if not UID or not data.UID then
        note = {}
        note.UID = createUID()
        note.Title = ""
        note.TextNote = ""
        note.actif = true
    else
        note = data.UID
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

    local lblTitleNote = gui:input(winNote, 35, 121, 250, 40)
    lblTitleNote:setTitle("Titre")
    lblTitleNote:onClick(
        function () 
            local keyboard = gui:keyboard("Titre de la note", newNote.Title)
            lblTitleNote:setText(keyboard)
            newNote.Title = keyboard
        end
    )

    local TextNote = gui:input(winNote, 35, 216, 250, 40)
    TextNote:setTitle("Note")
    TextNote:onClick(function () 
        local keyboard = gui:keyboard("Titre de la note", newNote.TextNote)
        TextNote:setText(keyboard)
        newNote.TextNote = keyboard
        end
    )

    edit = gui:button(winNote, 35, 350, 250, 38)
    edit:setText("Sauvegarder")
    edit:onClick(function()

        debugPrint(newNote)
--        data[newNote.UID]={}
        data[newNote.UID] = newNote
--        table.insert(data, newNote)
        saveTable(filename, data)
        --SaveNoteData(data)
        initNotes()
    end)

    cancel = gui:button(winNote, 35, 394, 250, 38)
    cancel:setText("Annuler")
    cancel:onClick(initNotes)

end 




function Settings()

    print(1)
    local winSetting = manageWindow()

    local Frame = gui:box(winSetting, 0, 0, 320, 65)
    Frame:setBackgroundColor(COLOR_WARNING)
    print(2)


    local TitleApp = gui:label(Frame, 20, 20, 250, 28)
    TitleApp:setText("A propos de Notes")
    TitleApp:setBackgroundColor(COLOR_WARNING)
    TitleApp:setFontSize(32)

    print(3)

    local leaveWin = gui:label(winSetting, 10, 80, 144, 28)
    leaveWin:setText("< Mes Notes")
    leaveWin:setFontSize(18)
    leaveWin:setTextColor(COLOR_GREY)
    leaveWin:onClick(initNotes)

    local updateLabel = gui:label(winSetting, 60, 430, 250, 35)
    updateLabel:setText("Notes V.1.0.0 - Created by M2LC")
    updateLabel:setFontSize(16)
    updateLabel:setTextColor(COLOR_GREY)
    print(4)

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
