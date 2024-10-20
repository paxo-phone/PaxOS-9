```markdown
# Paxos Lua API Documentation

This document details the Lua API available within the Paxos operating system. This API allows developers to create applications using Lua, leveraging the system's capabilities for GUI development, file system access, hardware interaction, event handling, and more.

## Modules

The Lua API is organized into several modules:

* **gui:** Provides functions for creating and managing graphical user interface elements.
* **storage:** Enables access to the Paxos file system.
* **hardware:** Offers limited interaction with the device's hardware.
* **time:** Facilitates time-related operations, including scheduling events.
* **events:** Allows registering callbacks for system events like calls, messages, and battery status changes.
* **gsm:** (Paxos ESP32 only) Provides access to GSM functionalities like calls, SMS, and contacts.
* **json:** Offers JSON parsing and manipulation capabilities.

## Modules Documentation

### 1. `gui` Module

This module allows creating graphical user interface elements.

**Example:**

```lua
local win = gui.window()  -- Create a window
local box = gui.box(win, 10, 10, 100, 50)  -- Create a box inside the window
gui.setWindow(win) -- Set the window active
```

**Available Functions:**

* `window()`: Creates a new window. Returns a `LuaWindow` object.
* `box(parent, x, y, width, height)`: Creates a box within the `parent` widget.
* `canvas(parent, x, y, width, height)`: Creates a canvas for drawing within the `parent` widget.
* `image(parent, path, x, y, width, height, background)`: Creates an image widget. `path` can be relative to the app directory or absolute.
* `label(parent, x, y, width, height)`: Creates a text label.
* `input(parent, x, y)`: Creates a text input field.
* `button(parent, x, y, width, height)`: Creates a button.
* `verticalList(parent, x, y, width, height)`: Creates a vertically scrollable list.
* `horizontalList(parent, x, y, width, height)`: Creates a horizontally scrollable list.
* `checkbox(parent, x, y)`: Creates a checkbox widget.
* `switch(parent, x, y)`: Creates a switch widget.
* `radio(parent, x, y)`: Creates a radio button widget.
* `del(widget)`: Deletes a widget and its children.
* `setWindow(window)`: Sets the main window for the application.
* `getWindow()`: Returns the main window.
* `keyboard(placeholder, defaultText)`: Displays a virtual keyboard and returns the entered text.
* `showInfoMessage(msg)`: Displays an info message box.
* `showWarningMessage(msg)`: Displays a warning message box.
* `showErrorMessage(msg)`: Displays an error message box.

**Widget Methods (Common to most widgets):**

These methods are available on most `gui` objects, like `LuaBox`, `LuaCanvas`, `LuaButton`, etc.

* `setX(x)`, `setY(y)`, `setWidth(width)`, `setHeight(height)`: Set the widget's dimensions and position.
* `getX()`, `getY()`, `getWidth()`, `getHeight()`: Get the widget's dimensions and position.
* `setBackgroundColor(color)`, `setBorderColor(color)`, `setBorderSize(size)`, `setRadius(radius)`: Set visual properties. Available color constants are listed below in the GUI Constants section.
* `enable()`, `disable()`: Enable or disable user interaction.
* `isEnabled()`: Checks if the widget is enabled.
* `isTouched()`: Checks if the widget is currently being touched.
* `onClick(function)`: Sets a callback function to be executed when the widget is clicked.
* `clear()`: Removes all child widgets.

**Specific Widget Methods:**

* **LuaBox:**  `setRadius(radius)`: Sets the corner radius.
* **LuaCanvas:**
    * `setPixel(x, y, color)`: Sets a pixel's color.
    * `drawRect(x, y, w, h, color)`, `fillRect(...)`: Draws or fills a rectangle.
    * `drawCircle(x, y, radius, color)`, `fillCircle(...)`: Draws or fills a circle.
    * `drawRoundRect(x, y, w, h, radius, color)`, `fillRoundRect(...)`: Draws or fills a rounded rectangle.
    * `drawPolygon(vertices, color)`, `fillPolygon(...)`: Draws or fills a polygon. `vertices` is a Lua table of `{x, y}` coordinate pairs.
    * `drawLine(x1, y1, x2, y2, color)`: Draws a line.
    * `drawText(x, y, text, color)`: Draws text.
    * `drawTextCentered(x, y, text, color, horizontallyCentered, verticallyCentered)`: Draws centered text.
    * `drawTextCenteredInRect(x, y, w, h, text, color, horizontallyCentered, verticallyCentered)`: Draws text centered within a rectangle.
    * `getTouch()`: Returns a table containing the touch coordinates relative to the canvas. `{x, y}`
    * `onTouch(function)`: Sets a callback function executed when the canvas is touched, passing the touch coordinates as a table argument.
* **LuaImage:** `setTransparentColor(color)`: Sets the transparent color for the image.
* **LuaLabel:**
    * `setText(text)`, `getText()`: Sets or gets the label's text.
    * `setFontSize(size)`: Sets the font size.
    * `getTextWidth()`, `getTextHeight()`: Gets the dimensions of the text.
    * `setVerticalAlignment(alignment)`, `setHorizontalAlignment(alignment)`: Sets the text alignment.
    * `setTextColor(color)`: Sets the text color.  Alignment constants are defined below.
* **LuaInput:**
    * `setText(text)`, `getText()`:  Sets or gets the input text.
    * `setPlaceholder(text)`: Sets placeholder text.
    * `setTitle(text)`: Sets the title for the input.
    * `onChange(function)`: sets a callback to be called when the text changes.
* **LuaButton:**
    * `setText(text)`, `getText()`: Sets or gets the button text.
    * `setIcon(path)`: Sets an icon for the button.  Path can be relative or absolute.
    * `setTheme(theme)`:  Sets the button theme (true for dark, false for light).
    * `format()`: Refreshes the button's layout.
* **LuaSwitch:**
    * `setState(state)`, `getState()`: Sets or gets the switch state (true/false).
    * `onChange(function)`: callback called when the switch state changes.
* **LuaRadio:** `setState(state)`, `getState()`: Sets or gets the radio button state (true/false).
* **LuaCheckbox:** `setState(state)`, `getState()`: Sets or gets the checkbox state (true/false).
* **LuaVerticalList:**
    * `setIndex(index)`: Sets the currently selected index.
    * `setSpaceLine(line)`: Sets the spacing between list items.
    * `setSelectionFocus(focus)`: Sets the selection focus (UP/CENTER). Selection constants are defined below.
    * `getSelected()`: Returns the index of the selected item.
    * `select(index)`: Selects an item at the given index and triggers the `onSelect` callback.
    * `setSelectionColor(color)`: Sets the selection highlight color.
    * `setAutoSelect(autoSelect)`: Enables/disables automatic selection on touch.
    * `onSelect(function)`: Sets a callback function to be executed when an item is selected.
* **LuaHorizontalList:**
    * `setSpaceLine(line)`: Sets the spacing between list items.



#### GUI Constants

* **Alignment:** `LEFT_ALIGNMENT`, `RIGHT_ALIGNMENT`, `CENTER_ALIGNMENT`, `UP_ALIGNMENT`, `DOWN_ALIGNMENT`
* **Colors:**  `COLOR_DARK`, `COLOR_LIGHT`, `COLOR_SUCCESS`, `COLOR_WARNING`, `COLOR_ERROR`, `COLOR_WHITE`, `COLOR_BLACK`, `COLOR_RED`, `COLOR_GREEN`, `COLOR_BLUE`, `COLOR_YELLOW`, `COLOR_GREY`, `COLOR_MAGENTA`, `COLOR_CYAN`, `COLOR_VIOLET`, `COLOR_ORANGE`, `COLOR_PINK`, `COLOR_LIGHT_ORANGE`, `COLOR_LIGHT_GREEN`, `COLOR_LIGHT_BLUE`, `COLOR_LIGHT_GREY`
* **Selection Focus:** `SELECTION_UP`, `SELECTION_CENTER`


### 2. `storage` Module

This module provides file system access.

**Example:**

```lua
local file = storage.file("my_file.txt", storage.WRITE)  -- Open a file for writing
file:open()
file:write("Hello, Paxos!\n")
file:close()

local file = storage.file("my_file.txt", storage.READ) -- open a file for reading
file:open()
local content = file:readAll()
print(content)
file:close()


local list = storage.listDir("./")
for i, item in ipairs(list) do
    if storage.isDir(item) then print(item .. ": directory")
    elseif storage.isFile(item) then print(item .. ": file")
    end
end
```



**Available Functions:**

* `file(filename, mode)`: Creates a `LuaStorageFile` object. `mode` can be `storage.READ`, `storage.WRITE`, or `storage.APPEND`.
* `newDir(path)`: Creates a new directory.
* `mvFile(oldpath, newpath)`, `mvDir(...)`: Renames a file or directory.
* `rmFile(path)`, `rmDir(path)`: Deletes a file or directory.
* `isDir(path)`, `isFile(path)`: Checks if a path is a directory or file.
* `listDir(path)`: Returns a table of filenames in a directory.

**`LuaStorageFile` Methods:**

* `open()`: Opens the file.
* `close()`: Closes the file.
* `write(text)`: Writes text to the file.
* `readChar()`: Reads a single character.
* `readLine()`: Reads a line.
* `readAll()`: Reads the entire file content.


### 3. `hardware` Module (If permitted)

Provides limited hardware interaction.

**Example:**

```lua
hardware.flash(true) -- Enable the flash LED
```

**Available Functions:**

* `flash(state)`: Controls the flash LED (if available).

### 4. `time` Module (If permitted)

Handles time-related tasks.

**Example:**

```lua
local intervalId = time.setInterval(function() print("Tick!") end, 1000)  -- Print "Tick!" every second

local timeoutId = time.setTimeout(function() print("Timeout!") end, 5000) -- Print "Timeout!" after 5 seconds

time.removeInterval(intervalId)
time.removeTimeout(timeoutId)


local currentTime = time.get("h,mi,s")
print("Current Time")
for k,v in pairs(currentTime) do
    print("Key: ",k , ", Value: ",v)
end

print(time.monotonic())
```



**Available Functions:**

* `monotonic()`: Returns milliseconds since the application started.
* `get(format)`: Returns the current time based on given comma separated identifiers ("s": seconds, "mi": minutes, "h": hours, "d": days, "mo": months, "y": year). Returns Lua table indexed by requested identifiers.
* `setInterval(function, interval)`: Executes a function repeatedly at a given interval (in milliseconds). Returns an ID to remove the interval.
* `setTimeout(function, timeout)`: Executes a function after a given timeout (in milliseconds). Returns an ID to remove the timeout.
* `removeInterval(id)`: Cancels a repeating interval.
* `removeTimeout(id)`: Cancels a timeout.


### 5. `events` Module

Registers callbacks for system events.

**Example:**

```lua
events.onmessage(function(msg) print("New message:", msg) end)
events.onlowbattery(function() print("Battery low!") end)
events.oncharging(function() print("Charging...") end)
events.oncall(function() print("Incoming call!") end)
events.onmessageerror(function() print("Error delivering message!") end)
```

**Available Functions:**

* `onmessage(function)`: Callback for new messages (SMS).
* `onlowbattery(function)`: Callback for low battery warnings.
* `oncharging(function)`: Callback for charging status changes.
* `oncall(function)`: Callback for incoming calls.
* `onmessageerror(function)`: Callback for message delivery errors.


### 6. `gsm` Module (Paxos ESP32 only)

Provides GSM functionalities.

**Example:**

```lua
gsm.newMessage("+33xxxxxxxxxx", "Hello from Lua!")  -- Send an SMS
gsm.newCall("+33xxxxxxxxxx") -- Initiate a call
gsm.endCall()  -- End a call
gsm.acceptCall()  -- Accept an incoming call
gsm.rejectCall() -- Reject an incoming call

local number = gsm.getNumber() -- retrieve current calling number
local call_state = gsm.getCallState() -- retrieve current call state


local messages = gsm.getMessages("+33xxxxxxxxxx")
for i,msg in pairs(messages) do
    print("From " .. msg.who .. " at " .. msg.date .. ":" .. msg.message)
end



-- Contact manipulation example
local contact1 = { name = "John Doe", phone = "+33123456789" }
gsm.addContact(contact1)

local contactList = gsm.listContacts()
for i, contact in ipairs(contactList) do
    print(contact.name, contact.phone)
end

gsm.deleteContact("+33123456789")

gsm.saveContacts()
```



**Available Functions:**

* `newMessage(number, message)`: Sends an SMS.
* `newCall(number)`: Initiates a call.
* `endCall()`: Ends a call.
* `acceptCall()`: Accepts an incoming call.
* `rejectCall()`: Rejects an incoming call.
* `getNumber()`: Gets the calling number for the current or last call.
* `getCallState()`: Get the current call state.  See GSM.h for the state integer values.
* `getMessages(number)`: Gets the messages history with a given number.
* `addContact(contact)`: Adds a new contact (`contact` is a Lua table `{name=..., phone=...}`).
* `deleteContact(phone)` : delete the given contact.
* `listContacts()`: Lists all contacts, returns a lua table of `Contact` objects.
* `saveContacts()`: saves contact list.
* `getContact(index)` : retrieve the contact at given index.
* `editContact(contact)`: Edits an existing contact. The `contact` table should contain the phone number to identify the contact to edit.
* `getContactByNumber(phone)`: Retrieves a contact by phone number.


### 7. `json` Module

Provides JSON parsing and manipulation capabilities.

**Example:**

```lua
local jsonData = '{"name": "Paxos", "version": 1.0}'
local json_object = json.new(jsonData)

print(json_object.name)  -- Access fields like regular Lua tables
print(json_object.version)

json_object.version = 1.1
json_object["new_field"] = "new value"

print(json_object:get()) -- retrieve the json string

local noJson = json.new('{"hello')
print(noJson:get()) -- retrieve json string from a malformed string

-- Nested JSON Example
local nestedJsonData = '{ "person": { "name": "Bob", "age": 30 } }'
local nestedJsonObject = json.new(nestedJsonData)

print(nestedJsonObject.person.name)  -- Access nested fields
print(nestedJsonObject.person.age)

nestedJsonObject.person.age = 31  -- Modify nested fields

local newObject = nestedJsonObject.person -- store sub-table and manipulate it as a regular Json object

newObject.name = "alice"
print(nestedJsonObject:get()) -- print the modified json


local array = json.new('[{"a": 1}, {"a": 2}]')
print(array[1].a) -- 1
print(array[2].a) -- 2


```



**Available Functions:**

* `new(data)`: Creates a new JSON object from a JSON string.

**`LuaJson` Methods:**

* `get()`: Returns the JSON string representation.
* `is_null()`: Returns whether the JSON object is null.
* `size()`:  Returns the number of elements (for arrays and objects).
* `has_key(key)`: Checks if a key exists.
* `remove(key)`: Removes a key-value pair.
* `get_int(key)`, `get_double(key)`, `get_bool(key)`, `get_string(key)`: Get values with type checking.
* `set_int(key, value)`, `set_double(key, value)`, `set_bool(key, value)`: Set values with type checking.



## Lua Specific Functions

* `require(module)`: Loads a Lua module. The module file should be located within the app's directory.
* `saveTable(filename, table)`: Saves a Lua table into a json file.
* `loadTable(filename)`: Loads a Lua table from a json file.
* `launch(appName, args)`: Launches another Paxos application.  `args` is an optional table of string arguments.


## Application Lifecycle Functions

These functions are defined within your Lua application code and are called by the Paxos system during the app's lifecycle.


* `run(args)`: The main entry point for your application. Called when the app is launched. The `args` table contains any command-line arguments passed to the application.
* `background(args)`: Called when the application is sent to the background (if implemented).  Useful for suspending tasks or releasing resources.
* `wakeup(args)`: Called when the application is brought back to the foreground (if `background` is implemented).
* `quit(args)`: Called when the application is being closed. Use this to clean up resources.



## Error Handling

Lua errors within your application can be handled with Lua's `pcall` or `xpcall` functions. Global error handling functions can be implemented by the developer using the `onmessageerror` callback or within the appropriate lifecycle functions. Lua errors will stop the application's execution.


## Permissions

An application's access to certain modules and functionalities is controlled by its manifest file (a JSON file). The manifest specifies which permissions the app requests. The user grants these permissions during installation.  If an app attempts to use a module or function it doesn't have permission for, an error will occur.


This documentation provides an overview of the Paxos Lua API.  Explore and experiment with the API to build your own Paxos applications.
```