#include "lua_widget.hpp"
#include "lua_gui.hpp"

LuaWidget* LuaWidget::rootOfDelete = nullptr;

void LuaWidget::init(gui::ElementBase* obj, LuaWidget* parent)
{
    widget = obj;
    parent->addChild(this);
}

LuaWidget::~LuaWidget()
{
    if(rootOfDelete == nullptr)
        rootOfDelete = this;

    for (LuaWidget* child : children)
    {
        delete child;
    }

    if(rootOfDelete == this && this->parent != nullptr)
    {
        LuaWidget* widget = this;
        gui::ElementBase* reWidget = widget->widget;

        LuaWidget* parent = this->parent;
        gui::ElementBase* reParent = parent->widget;

        for (uint16_t i = 0; i < parent->children.size(); i++)
        {
            if (parent->children[i] == widget)
            {
                parent->children.erase(parent->children.begin() + i);
                break;
            }
        }

        for (uint16_t i = 0; i < reParent->m_children.size(); i++)
        {
            if (reParent->m_children[i] == reWidget)
            {
                reParent->m_children.erase(reParent->m_children.begin() + i);
                break;
            }
        }
    }

    for (uint16_t i = 0; i < gui->widgets.size(); i++)
    {
        if (gui->widgets[i] == this)
        {
            gui->widgets.erase(gui->widgets.begin() + i);
            break;
        }
    }

    if(rootOfDelete == this)
    {
        delete this->widget;
        rootOfDelete = nullptr;
    }
        this->widget = nullptr;
}

void LuaWidget::update()
{
    if(this->widget->getParent() == nullptr)
    {
        this->widget->updateAll();
    }

    specificUpdate();

    if(onClickFunc && this->widget->isTouched())
    {
        std::cout << "[Lua] User Interacted" << std::endl;


    /*#ifdef ESP_PLATFORM
        // Heap information
        uint32_t freeHeap = esp_get_free_heap_size();
        uint32_t totalHeap = ESP.getHeapSize();
        uint32_t minFreeHeap = esp_get_minimum_free_heap_size();
        float heapPercentage = ((float)freeHeap / totalHeap) * 100;

        Serial.println("Heap Memory:");
        Serial.printf("Total: %u bytes\n", totalHeap);
        Serial.printf("Free: %u bytes\n", freeHeap);
        Serial.printf("Used: %u bytes\n", totalHeap - freeHeap);
        Serial.printf("Minimum free ever: %u bytes\n", minFreeHeap);
        Serial.printf("Percentage free: %.2f%%\n\n", heapPercentage);

        // PSRAM information (if available)
        if (psramFound()) {
            uint32_t freePsram = ESP.getFreePsram();
            uint32_t totalPsram = ESP.getPsramSize();
            float psramPercentage = ((float)freePsram / totalPsram) * 100;

            Serial.println("PSRAM:");
            Serial.printf("Total: %u bytes\n", totalPsram);
            Serial.printf("Free: %u bytes\n", freePsram);
            Serial.printf("Used: %u bytes\n", totalPsram - freePsram);
            Serial.printf("Percentage free: %.2f%%\n\n", psramPercentage);
        } else {
            Serial.println("PSRAM not found\n");
        }

        // Flash chip information
        uint32_t flashChipSize = ESP.getFlashChipSize();
        uint32_t flashChipSpeed = ESP.getFlashChipSpeed();

        Serial.println("Flash Chip:");
        Serial.printf("Size: %u bytes\n", flashChipSize);
        Serial.printf("Speed: %u Hz\n\n", flashChipSpeed);

        // Sketch information
        uint32_t sketchSize = ESP.getSketchSize();
        uint32_t freeSketchSpace = ESP.getFreeSketchSpace();

        Serial.println("Sketch:");
        Serial.printf("Size: %u bytes\n", sketchSize);
        Serial.printf("Free space: %u bytes\n\n", freeSketchSpace);
    #endif*/

        // Execute the function
        sol::protected_function_result result = onClickFunc();

        // Check for errors
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "Lua Error: " << err.what() << std::endl;
        }
    }

    /*if(onScrollUpFunc && touch.isSlidingVertically() < -30)
    {
        onScrollUpFunc();
        touch.resetScrollVertical();
    }

    if(onScrollDownFunc && touch.isSlidingVertically() > 30)
    {
        onScrollDownFunc();
        touch.resetScrollVertical();
    }

    if(onScrollRightFunc && touch.isSlidingHorizontally() > 30)
    {
        onScrollRightFunc();
        touch.resetScrollHorizontal();
    }

    if(onScrollLeftFunc && touch.isSlidingHorizontally() < -30)
    {
        onScrollLeftFunc();
        touch.resetScrollHorizontal();
    }*/

    for (int i = 0; i < this->children.size(); i++)
    {
        children[i]->update();
    }
}

void LuaWidget::clear() { 

        while (!this->children.empty()) {
            delete this->children[0];
        }       
}


    void LuaWidget::addChild(LuaWidget* child)
    {
        this->children.push_back(child); 
        child->parent = this;
        this->widget->addChild(child->widget);
    }
