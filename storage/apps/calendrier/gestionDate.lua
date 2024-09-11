local monthsName = {"Janvier", "Fevrier", "Mars", "Avril", "Mai", "Juin", "Juillet", "Août", "Septembre", "Octobre", "Novembre", "Decembre"}
local monthsShortName = {"Jan", "Fev", "Mars", "Avr", "Mai", "Juin", "Juil", "Août", "Sep", "Oct", "Nov", "Dec"}

local daysOfWeek = { "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi", "Dimanche" }
local daysOfWeekShort = { "lun", "mar", "mer", "jeu", "ven", "sam", "dim" }


function getDayOfWeekName (m_day)
    return daysOfWeek[m_day]
end

function getDayOfWeekShortName (m_day)
    return daysOfWeekShort[m_day]
end

function getMonthName(m_month)
    return monthsName[m_month]
end


function getMonthShortName(m_month)
    return monthsShortName[m_month]
end

-- Retourne le numero de la semaine d'une date donnée
function getWeekNum(year, month, day)
    local daysInMonth = {31, isLeapYear(year) and 29 or 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
    local totalDays = 0

    for m = 1, month - 1 do
        totalDays = totalDays + daysInMonth[m]
    end

    totalDays = totalDays + day

    return int((totalDays - 1) / 7) + 1
end

-- Add days "dayToAdd" to the "originalDate" 
-- return new date to and array year, month, day
function addDaysToDate(originalDate, daysToAdd)

    local year = originalDate[1]
    local month = originalDate[2]
    local day = originalDate[3]

    -- Ensure input date is valid
    if year < 1 or month < 1 or month > 12 or day < 1 or day > getDaysInMonth(year, month) then
        return nil, "Invalid input date"
    end

    local newYear, newMonth, newDay = year, month, day

    while daysToAdd ~= 0 do
        if daysToAdd > 0 then
            -- Adding days
            local daysInCurrentMonth = getDaysInMonth(newYear, newMonth)
            if newDay + daysToAdd <= daysInCurrentMonth then
                newDay = newDay + daysToAdd
                break
            else
                daysToAdd = daysToAdd - (daysInCurrentMonth - newDay + 1)
                newDay = 1
                newMonth = newMonth + 1
                if newMonth > 12 then
                    newMonth = 1
                    newYear = newYear + 1
                end
            end
        else
            -- Subtracting days
            if newDay + daysToAdd >= 1 then
                newDay = newDay + daysToAdd
                break
            else
                daysToAdd = daysToAdd + newDay
                newMonth = newMonth - 1
                if newMonth < 1 then
                    newMonth = 12
                    newYear = newYear - 1
                end
                newDay = getDaysInMonth(newYear, newMonth)
            end
        end
    end

    return {newYear, newMonth, newDay}
end -- addDaysToDate


-- return an array with the current year, month and day
function getToday()

    local today = time:get("y,mo,d")
    return {today[1], today[2], today[3]}

end

-- récupère le jour de la semaine en fonction de la date
-- format européen : le 1er jours est le lundi
function getDayOfWeek(yy, mm, dd) 
    local mmx = mm
  
    if (mm == 1) then  mmx = 13; yy = yy-1  end
    if (mm == 2) then  mmx = 14; yy = yy-1  end
  
    local val8 = dd -1 + (mmx*2) +  math.floor(((mmx+1)*3)/5)   + yy + math.floor(yy/4)  - math.floor(yy/100)  + math.floor(yy/400) + 2
    local val9 = math.floor(val8/7)
    local dw = val8-(val9*7)
  
    if (dw == 0) then
      dw = 7
    end
    
    return dw
end


  -- Détermine si year est une année bixestile ou non
function isLeapYear(year)
    return year % 4 == 0 and (year % 100 ~= 0 or year % 400 == 0)
end


-- renvoi le nombre de jours d'un mois et année donnée
function getDaysInMonth(year, month)
    local days_in_month = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }   
    local d = days_in_month[month]
    
    if month == 2 and isLeapYear(year) then d=29 end
    return d  
end