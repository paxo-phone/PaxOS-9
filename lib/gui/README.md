# Paxos GUI Library Documentation

This document provides comprehensive documentation for the Paxos GUI library, detailing its core components, element types, and usage examples.

## Core Class: ElementBase

The `ElementBase` class serves as the foundation for all GUI elements within the Paxos library. It defines common properties and methods inherited by all other element types.

### Members:

- `m_x`: X-coordinate of the element's top-left corner.
- `m_y`: Y-coordinate of the element's top-left corner.
- `m_width`: Width of the element.
- `m_height`: Height of the element.
- `m_backgroundColor`: Background color of the element.
- `m_borderColor`: Border color of the element.
- `m_borderSize`: Thickness of the element's border.
- `m_borderRadius`: Radius of the element's rounded corners.
- `m_parent`: Pointer to the parent element (if any).
- `m_children`: Vector of child elements.
- `m_isEnabled`: Boolean indicating whether the element is enabled.
- `m_surface`: Pointer to the underlying graphics surface.

### Methods:

- `render()`: Virtual function responsible for rendering the element's visual representation. **Must be overridden by derived classes.**
- `update()`: Updates the element's state and handles user interaction.
- `setX(uint16_t x)`: Sets the X-coordinate.
- `setY(uint16_t y)`: Sets the Y-coordinate.
- `setWidth(uint16_t width)`: Sets the width.
- `setHeight(uint16_t height)`: Sets the height.
- `getX() const`: Returns the X-coordinate.
- `getY() const`: Returns the Y-coordinate.
- `getWidth() const`: Returns the width.
- `getHeight() const`: Returns the height.
- `setBackgroundColor(color_t color)`: Sets the background color.
- `setBorderColor(color_t color)`: Sets the border color.
- `setRadius(uint16_t r)`: Sets the border radius.
- `getRadius() const`: Returns the border radius.
- `setBorderSize(uint16_t size)`: Sets the border size.
- `getBorderSize() const`: Returns the border size.
- `getBackgroundColor() const`: Returns the background color.
- `getBorderColor() const`: Returns the border color.
- `isTouched()`: Returns `true` if the element was touched and released.
- `isFocused(bool forced = false)`: Returns `true` if the element is currently being touched.
- `enable()`: Enables the element.
- `disable()`: Disables the element.
- `setEnabled(bool enabled)`: Sets the enabled state.
- `isEnabled() const`: Returns the enabled state.
- `addChild(ElementBase *child)`: Adds a child element.
- `getSurface()`: Returns a pointer to the surface.
- `forceUpdate()`: Forces a graphical update.


### Example:

```cpp
#include "ElementBase.hpp"

// Create a derived class
class MyElement : public gui::ElementBase {
public:
    MyElement(uint16_t x, uint16_t y, uint16_t width, uint16_t height) : ElementBase() {
        setX(x);
        setY(y);
        setWidth(width);
        setHeight(height);
        setBackgroundColor(COLOR_RED);
    }

    void render() override {
      m_surface->fillRect(0,0, getWidth(), getHeight(), getBackgroundColor());
    }
};

int main() {
    // Initialize graphics (implementation-specific)
    // ...

    // Create an instance of MyElement
    MyElement myElement(10, 10, 50, 50);

    // Add to window
    gui::elements::Window win;
    win.addChild(&myElement);

    while (true)
    {
      win.updateAll();
    }

    return 0;
}
```


## GUI Elements

### 1. Box

A simple rectangular box element.

#### Members:

Inherits all members from `ElementBase`.

#### Methods:

- `Box(uint16_t x, uint16_t y, uint16_t width, uint16_t height)`: Constructor.
- `render() override`: Renders the box.


#### Example:

```cpp
#include "Box.hpp"

gui::elements::Box myBox(10, 20, 100, 50);
myBox.setBackgroundColor(COLOR_BLUE);
myBox.setBorderColor(COLOR_BLACK);
myBox.setBorderSize(2);
myBox.setRadius(5);
```

### 2. Button

A clickable button element.

#### Members:

- `m_label`:  Pointer to the button's label.
- `m_image`: Pointer to the button's image.
- `m_theme`: Boolean representing the button theme (e.g., light or dark).


#### Methods:

- `Button(uint16_t x, uint16_t y, uint16_t width, uint16_t height)`: Constructor.
- `render() override`: Renders the button.
- `setText(std::string text)`: Sets the button text.
- `setIcon(storage::Path path)`: Sets the button icon.
- `getText()`: Returns the button text.
- `setTheme(bool value)`: Sets the button theme.
- `onClick() override`: Called when the button is clicked.
- `onReleased() override`: Called when the button is released


#### Example:

```cpp
#include "Button.hpp"

gui::elements::Button myButton(10, 20, 80, 30);
myButton.setText("Click Me");

// In the update loop:
if (myButton.isTouched()) {
    // Handle button click
}
```

### 3. Canvas

A drawable surface for custom graphics.

#### Members:

Inherits members from `ElementBase`.

#### Methods:

- `Canvas(uint16_t x, uint16_t y, uint16_t width, uint16_t height)`: Constructor.
- `render() override`: Dummy render function (drawing is done directly on the surface).
- `setPixel(int16_t x, int16_t y, color_t color)`: Sets a pixel's color.
- `drawRect(...)`, `fillRect(...)`, `drawCircle(...)`, `fillCircle(...)`, `drawRoundRect(...)`, `fillRoundRect(...)`, `drawPolygon(...)`, `fillPolygon(...)`, `drawLine(...)`: Drawing functions.
- `drawText(...)`: Draw text with optional font size.
- `drawTextCentered(...)`: Draw centered text with optional alignment parameters.

#### Example:

```cpp
#include "Canvas.hpp"

gui::elements::Canvas myCanvas(10, 20, 200, 100);
myCanvas.setPixel(50, 50, COLOR_RED);
myCanvas.drawLine(0, 0, 100, 50, COLOR_BLUE);
std::string text = "Hello";
myCanvas.drawText(0,0, text, COLOR_BLACK);
```

### 4. Checkbox

A checkbox element.

#### Members:

- `m_state`: Boolean indicating whether the checkbox is checked.

#### Methods:

- `Checkbox(uint16_t x, uint16_t y)`: Constructor.
- `render() override`: Renders the checkbox.
- `setState(bool state)`: Sets the checked state.
- `getState()`: Returns the checked state.

#### Example:

```cpp
#include "Checkbox.hpp"

gui::elements::Checkbox myCheckbox(10, 20);

// In the update loop:
if (myCheckbox.isTouched()) {
    bool isChecked = myCheckbox.getState();
    // ...
}
```

### 5. Filter (Deprecated)

Marked as deprecated; avoid using.

### 6. Image

Displays an image from storage.

#### Members:

- `m_path`: Path to the image file.

#### Methods:

- `Image(storage::Path path, uint16_t x, uint16_t y, uint16_t width, uint16_t height)`: Constructor.
- `render() override`: Renders the image.
- `load()`: Loads the image from the specified path.

#### Example:

```cpp
#include "Image.hpp"

gui::elements::Image myImage(storage::Path("/path/to/image.png"), 10, 20, 50, 50);
myImage.load();
```

### 7. Input

A text input field.

#### Members:

- `m_text`: The inputted text.
- `m_placeHolder`: Placeholder text displayed when the input is empty.

#### Methods:

- `Input(uint16_t x, uint16_t y)`: Constructor.
- `render() override`: Renders the input field.
- `setText(const std::string& text)`: Sets the input text.
- `setPlaceHolder(const std::string& text)`: Sets the placeholder text.
- `getText()`: Returns the input text.
- `getPlaceHolder()`: Returns the placeholder text.


#### Example:

```c++
#include "Input.hpp"

gui::elements::Input myInput(10, 20);
myInput.setPlaceHolder("Enter text...");

// In the update loop:
if (myInput.isTouched()) {
   std::string inputText = myInput.getText();
  // ...
}
```


### 8. Keyboard

An on-screen keyboard for text input.

#### Members:
(Numerous members related to keyboard layout and state - see Keyboard.hpp for full list)

#### Methods:

- `Keyboard(const std::string &defaultText = "")`: Constructor.
- `render() override`: Renders the keyboard.
- `widgetUpdate() override`: Handles key presses and other keyboard interactions.
- `getText()`: Returns the entered text and clears the keyboard buffer.
- `hasExitKeyBeenPressed()`: Checks if the exit key has been pressed.
- `setPlaceholder(const std::string &placeholder)`: Sets placeholder text for the input field.


#### Example:

```cpp
#include "Keyboard.hpp"

gui::elements::Keyboard keyboard;

// In the update loop:
keyboard.updateAll();

if (keyboard.hasExitKeyBeenPressed()) {
   std::string enteredText = keyboard.getText();
   // ...
}
```


### 9. Label

Displays static text.

#### Members:

- `m_text`: The text to display.
- `m_textColor`: Color of the text.
- `m_fontSize`: Size of the font.

#### Methods:

- `Label(uint16_t x, uint16_t y, uint16_t width, uint16_t height)`: Constructor.
- `render() override`: Renders the label.
- `setText(const std::string& text)`: Sets the text.
- `getText() const`: Gets the text.
- `setTextColor(color_t color)`: Sets text color.
- `setFontSize(uint16_t fontSize)`: Set font size.
- `setHorizontalAlignment(Alignement alignment)`: Sets horizontal alignment.
- `setVerticalAlignment(Alignement alignment)`: Sets vertical alignment.



#### Example:

```cpp
#include "Label.hpp"

gui::elements::Label myLabel(10, 20, 100, 20);
myLabel.setText("Hello, world!");
myLabel.setTextColor(COLOR_GREEN);
myLabel.setHorizontalAlignment(gui::elements::Label::CENTER);
```


### 10. List (VerticalList & HorizontalList)


Creates scrollable lists of elements.


#### Members: (VerticalList)
- `m_lineSpace`: Spacing between list items.
- `m_focusedIndex`: Index of the currently focused item.


#### Methods: (VerticalList)

- `VerticalList(uint16_t x, uint16_t y, uint16_t width, uint16_t height)`: Constructor.
- `render() override`: Renders the list.
- `add(ElementBase* widget)`: Adds an element to the list.
- `setSpaceLine(uint16_t y)`: Sets spacing between lines.
- `onScrollUp()`: Called when scrolling up.
- `onScrollDown()`: Called when scrolling down.

#### Methods: (HorizontalList)

- `HorizontalList(uint16_t x, uint16_t y, uint16_t width, uint16_t height)`: Constructor.
- `render() override`: Renders the list.
- `add(ElementBase* widget)`: Adds an element to the list.
- `setSpaceLine(uint16_t y)`: Sets spacing between lines.


#### Example (VerticalList):

```cpp
#include "List.hpp"
#include "Label.hpp"


gui::elements::VerticalList myList(10, 20, 100, 200);

for (int i = 0; i < 20; ++i) {
  gui::elements::Label* label = new gui::elements::Label(0, 0, 80, 20);
  label->setText("Item " + std::to_string(i));
  myList.add(label);
}


// In the update loop:
myList.updateAll();
```


### 11. Radio

A radio button element.


#### Members:

- `m_state`: Boolean indicating whether the radio button is selected.

#### Methods:

- `Radio(uint16_t x, uint16_t y)`: Constructor.
- `render() override`: Renders the radio button.
- `setState(bool state)`: Sets the selected state.
- `getState()`:  Returns the selected state.

#### Example:


```cpp
#include "Radio.hpp"

gui::elements::Radio myRadio(10, 20);

// In the update loop:
if (myRadio.isTouched()) {
   bool isSelected = myRadio.getState();
    // ...
}
```




### 12. Switch

A toggle switch element.

#### Members:

- `m_state`: Boolean indicating the switch state (on/off).

#### Methods:

- `Switch(uint16_t x, uint16_t y)`: Constructor.
- `render() override`: Renders the switch.
- `setState(bool state)`: Sets the switch state.
- `getState()`: Returns the switch state.

#### Example:

```c++
#include "Switch.hpp"

gui::elements::Switch mySwitch(10, 20);

// In the update loop:
if (mySwitch.isTouched()) {
    bool isOn = mySwitch.getState();
    // ...
}
```

### 13. Window

A top-level window element.

#### Members:
- `windows`: A static vector containing all created windows.


#### Methods:

- `Window()`: Constructor.
- `render() override`: Renders the window.

#### Example:

```c++
#include "Window.hpp"

gui::elements::Window myWindow;

// Add other elements to the window using myWindow.addChild(...);

// In the main loop:
while(true) {
    myWindow.updateAll();
}

```


## GuiManager

A singleton class providing utility functions for the GUI.


#### Methods:

- `getInstance()`: Returns the singleton instance.
- `getWindow()`: Returns the main window.
- `showInfoMessage(const std::string& msg )`: Shows an info message popup.
- `showWarningMessage(const std::string& msg )`: Shows a warning message popup.
- `showErrorMessage(const std::string& msg )`: Shows an error message popup.



#### Example:

```c++
#include "GuiManager.hpp"

GuiManager& guiManager = GuiManager::getInstance();
gui::elements::Window& mainWindow = guiManager.getWindow();

// Add some elements to mainWindow

// ... later in the code
guiManager.showErrorMessage("Something went wrong!");

while(true) {
    mainWindow.updateAll();
}
```


This documentation provides a basic overview of the Paxos GUI library.  Refer to the individual header files (`.hpp`) for more detailed information about each class and its members. Remember to consult the graphics library documentation for specifics on color definitions and other graphics-related functions.