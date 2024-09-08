#include "lua_file.hpp"

#include <gui.hpp>
#include <filestream.hpp>
#include <path.hpp>
#include <hardware.hpp>
#include <threads.hpp>
#include <json.hpp>
#include <app.hpp>
#include <contacts.hpp>
#include <libsystem.hpp>
#include <standby.hpp>

#include <fstream>
#include <iostream>


/*
LuaHttpClient::LuaHttpClient(LuaFile* lua)
    : httpClient()
{
    this->lua = lua;
}

LuaHttpClient::~LuaHttpClient()
{
    std::cout << "LuaHttpClient deleted" << std::endl;
}

std::string LuaHttpClient::get(std::string url)
{
    // ajouter la verrification de l'url
    return httpClient.get(url);
}

std::string LuaHttpClient::post(std::string url)
{
    // ajouter la verrification de l'url
    return httpClient.get(url); // a changer quand il y aura le post :/
}

LuaNetwork::LuaNetwork(LuaFile* lua)
{
    this->lua = lua;
}

std::shared_ptr<LuaHttpClient> LuaNetwork::createHttpClient()
{
    return std::make_shared<LuaHttpClient>(lua);
}*/

LuaFile::LuaFile(storage::Path filename, storage::Path manifest)
    :lua_gui(this),
    lua_storage(this),
    lua_time(this)/*,
    lua_network(this)*/
{
    this->filename = filename;
    this->manifest = manifest;
    this->directory = filename / storage::Path("..");
}

LuaFile::~LuaFile()
{
    // prevent a crash if the app is deleted and one or more callbacks are defined
    this->onmessage = sol::nil;
    this->onmessageerror = sol::nil;
    this->oncall = sol::nil;
    this->onlowbattery = sol::nil;
    this->oncharging = sol::nil;

    // libérer les ressources (events, etc)
}

void* custom_allocator(void *ud, void *ptr, size_t osize, size_t nsize) {
    //std::cout << "custom_allocator: " << nsize << std::endl;
    if (nsize == 0) {
        // Free the block
        if (ptr != NULL) {
            free(ptr);
        }
        return NULL;
    } else {
        // Allocate or resize the block
        #ifdef ESP32
            return ps_realloc(ptr, nsize);
        #else
            return realloc(ptr, nsize);
        #endif
    }
}

int sol_exception_handler(lua_State* L, sol::optional<const std::exception&> maybe_exception, sol::string_view description) {
    std::cerr << "An error occurred in Lua: ";
    if (maybe_exception) {
        std::cerr << maybe_exception->what() << std::endl;
    }
    else {
        std::cerr << description << std::endl;
    }
    lua_error(L);
    abort();
    return 0;
}

int custom_panic_handler(lua_State* L) {
    std::shared_ptr<AppManager::App> app = AppManager::get(L);

    app->errors += std::string(lua_tostring(L, -1)) + "\n";
    app->app_state = AppManager::App::AppState::NOT_RUNNING;

    const char* msg = lua_tostring(L, -1);
    std::cerr << "Lua panic: " << msg << std::endl;
    
    return 0;
}

template <typename T>
void writeBinaryValue(std::ofstream& file, const T& value) {
    file.write(reinterpret_cast<const char*>(&value), sizeof(T));
}

void saveTableToBinaryFile(std::ofstream& file, const sol::table& table) {
    // Write the number of key-value pairs
    size_t numPairs = table.size();
    writeBinaryValue(file, numPairs);

    for (const auto& pair : table) {
        sol::object key = pair.first;
        sol::object value = pair.second;

        // Write the key type
        if (key.is<std::string>()) {
            writeBinaryValue(file, static_cast<uint8_t>(1)); // 1 for string key
            std::string keyStr = key.as<std::string>();
            size_t keySize = keyStr.size();
            writeBinaryValue(file, keySize);
            file.write(keyStr.c_str(), keySize);
        } else if (key.is<double>() || key.is<int>()) {
            writeBinaryValue(file, static_cast<uint8_t>(2)); // 2 for numeric key
            double numericKey = key.as<double>();
            writeBinaryValue(file, numericKey);
        } else {
            throw std::runtime_error("Unsupported table key type for binary serialization");
        }

        // Write the value type and value
        if (value.is<std::string>()) {
            writeBinaryValue(file, static_cast<uint8_t>(1)); // 1 for string
            std::string valueStr = value.as<std::string>();
            size_t valueSize = valueStr.size();
            writeBinaryValue(file, valueSize);
            file.write(valueStr.c_str(), valueSize);
        } else if (value.is<double>() || value.is<int>()) {
            writeBinaryValue(file, static_cast<uint8_t>(2)); // 2 for number
            double numericValue = value.as<double>();
            writeBinaryValue(file, numericValue);
        } else if (value.is<bool>()) {
            writeBinaryValue(file, static_cast<uint8_t>(3)); // 3 for boolean
            bool boolValue = value.as<bool>();
            writeBinaryValue(file, boolValue);
        } else if (value.is<sol::nil_t>()) {
            writeBinaryValue(file, static_cast<uint8_t>(4)); // 4 for nil
        } else if (value.is<sol::table>()) {
            writeBinaryValue(file, static_cast<uint8_t>(5)); // 5 for table
            saveTableToBinaryFile(file, value.as<sol::table>()); // Recursively save
        } else {
            throw std::runtime_error("Unsupported table value type for binary serialization");
        }
    }
}

void saveTableToBinaryFile(const std::string& filename, const sol::table& table) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Error opening file for writing: " + filename);
    }

    try {
        saveTableToBinaryFile(file, table);
    } catch (const std::exception& e) {
        file.close();
        throw std::runtime_error(std::string("Error while writing to file: ") + e.what());
    }

    file.close();
}

template <typename T>
T readBinaryValue(std::ifstream& file) {
    T value;
    file.read(reinterpret_cast<char*>(&value), sizeof(T));
    return value;
}

sol::table loadTableFromBinaryFile(sol::state& lua, std::ifstream& file) {
    sol::table table = lua.create_table();

    size_t numPairs = readBinaryValue<size_t>(file);

    for (size_t i = 0; i < numPairs; ++i) {
        // Read key
        uint8_t keyType = readBinaryValue<uint8_t>(file);
        sol::object key;

        if (keyType == 1) { // String key
            size_t keySize = readBinaryValue<size_t>(file);
            std::vector<char> keyBuffer(keySize);
            file.read(keyBuffer.data(), keySize);
            key = sol::make_object(lua, std::string(keyBuffer.data(), keySize));
        } else if (keyType == 2) { // Numeric key
            key = sol::make_object(lua, readBinaryValue<double>(file));
        } else {
            throw std::runtime_error("Unsupported key type in binary file");
        }

        // Read value
        uint8_t valueType = readBinaryValue<uint8_t>(file);

        switch (valueType) {
            case 1: { // String
                size_t valueSize = readBinaryValue<size_t>(file);
                std::vector<char> valueBuffer(valueSize);
                file.read(valueBuffer.data(), valueSize);
                table[key] = std::string(valueBuffer.data(), valueSize);
                break;
            }
            case 2: // Number
                table[key] = readBinaryValue<double>(file);
                break;
            case 3: // Boolean
                table[key] = readBinaryValue<bool>(file);
                break;
            case 4: // Nil
                table[key] = sol::nil;
                break;
            case 5: // Nested table
                table[key] = loadTableFromBinaryFile(lua, file);
                break;
            default:
                throw std::runtime_error("Unsupported value type in binary file");
        }
    }

    return table;
}

sol::table loadTableFromBinaryFile(sol::state& lua, const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Error opening file for reading: " + filename);
    }

    try {
        return loadTableFromBinaryFile(lua, file);
    } catch (const std::exception& e) {
        file.close();
        throw std::runtime_error(std::string("Error while reading from file: ") + e.what());
    }
}

void LuaFile::load()
{
    StandbyMode::triggerPower();

    lua_setallocf(lua.lua_state(), custom_allocator, NULL);

    // Charger le module dans l'environnement Lua
    lua.open_libraries(sol::lib::base);
    lua.open_libraries(sol::lib::math);
    lua.open_libraries(sol::lib::table);
    lua.open_libraries(sol::lib::string);

    // Lire la configuration
    storage::FileStream file2(manifest.str(), storage::READ);
    std::string conf = file2.read();
    file2.close();

    if(!nlohmann::json::accept(conf))
    {
        std::cerr << "Les permissions de l'app ne sont pas définies ou sont invalides" << std::endl;
        std::cerr << "Conf: " << conf << " in " << manifest.str() << std::endl;
        return;
    }

    nlohmann::json confJson = nlohmann::json::parse(conf);

    // en fonction de la configuration, choisir les permissions
    perms.acces_files = std::find(confJson["access"].begin(), confJson["access"].end(), "files") != confJson["access"].end();
    perms.acces_files_root = std::find(confJson["access"].begin(), confJson["access"].end(), "files_root") != confJson["access"].end();
    perms.acces_gsm = std::find(confJson["access"].begin(), confJson["access"].end(), "gsm") != confJson["access"].end();
    perms.acces_gui = std::find(confJson["access"].begin(), confJson["access"].end(), "gui") != confJson["access"].end();
    perms.acces_hardware = std::find(confJson["access"].begin(), confJson["access"].end(), "hardware") != confJson["access"].end();
    perms.acces_time = std::find(confJson["access"].begin(), confJson["access"].end(), "time") != confJson["access"].end();
    perms.acces_web = std::find(confJson["access"].begin(), confJson["access"].end(), "web") != confJson["access"].end();
    perms.acces_web_paxo = std::find(confJson["access"].begin(), confJson["access"].end(), "web_paxo") != confJson["access"].end();

    confJson.clear();

    // en fonction des permissions, charger certains modules

    lua.set_function("nonothing", []() {
    });

    lua["require"] = [&](const std::string& filename) -> sol::object {
        storage::Path lib(filename);

        // Load the file
        sol::load_result chunk = lua.load_file(this->lua_storage.convertPath(lib).str());
        if (!chunk.valid()) {
            sol::error err = chunk;
            throw std::runtime_error("Error loading module '" + filename + "': " + err.what());
        }

        // 4. Execute the loaded chunk and return its results
        return chunk(); 
    };

    lua["saveTable"] = [&](const std::string& filename, const sol::table& table)
    {
        saveTableToBinaryFile(lua_storage.convertPath(filename).str(), table);
    };

    lua["loadTable"] = [&](const std::string& filename)
    {
        return loadTableFromBinaryFile(lua, lua_storage.convertPath(filename).str());
    };

    if (perms.acces_hardware)   // si hardware est autorisé
    {
        lua.new_usertype<LuaHardware>("hardware",
            "flash", &LuaHardware::flash
        );

        lua["hardware"] = &lua_hardware;
    }

    if (perms.acces_files)   // si storage est autorisé
    {
        lua.new_usertype<LuaStorageFile>("File",
            "open", &LuaStorageFile::open,
            "close", &LuaStorageFile::close,
            "write", &LuaStorageFile::write,
            "readChar", &LuaStorageFile::readChar,
            "readLine", &LuaStorageFile::readLine,
            "readAll", &LuaStorageFile::readAll
        );

        lua.new_usertype<LuaStorage>("storage",
            "file", &LuaStorage::file,  // return a (new LuaStorageFile)
            "mkdir", &LuaStorage::newDir,
            "mvFile", &LuaStorage::renameFile,
            "mvDir", &LuaStorage::renameDir,
            "rmFile", &LuaStorage::deleteFile,
            "rmDir", &LuaStorage::deleteDir,
            "isDir", &LuaStorage::isDir,
            "isFile", &LuaStorage::isFile,
            "listDir", &LuaStorage::listDir
        );

        lua.set("READ", 0);
        lua.set("APPEND", 2);
        lua.set("WRITE", 1);
        
        auto json_ud = lua.new_usertype<LuaJson>("Json",
            "new", sol::constructors<LuaJson(std::string)>(),
            "get", &LuaJson::get,
            "is_null", &LuaJson::is_null,
            "size", &LuaJson::size,
            "has_key", &LuaJson::has_key,
            "remove", &LuaJson::remove,
            "get_int", &LuaJson::get_int,
            "get_double", &LuaJson::get_double,
            "get_bool", &LuaJson::get_bool,
            "get_string", &LuaJson::get_string,
            "set_int", &LuaJson::set_int,
            "set_double", &LuaJson::set_double,
            "set_bool", &LuaJson::set_bool,
            "__index", sol::overload(
                static_cast<LuaJson (LuaJson::*)(std::string)>( &LuaJson::op ),
                static_cast<int (LuaJson::*)(std::string)>( &LuaJson::get_int ),
                static_cast<double (LuaJson::*)(std::string)>( &LuaJson::get_double ),
                static_cast<bool (LuaJson::*)(std::string)>( &LuaJson::get_bool )
            ),
            "__newindex", sol::overload(
                static_cast<void (LuaJson::*)(std::string, int)>( &LuaJson::set_int ),
                static_cast<void (LuaJson::*)(std::string, double)>( &LuaJson::set_double ),
                static_cast<void (LuaJson::*)(std::string, bool)>( &LuaJson::set_bool ),
                static_cast<void (LuaJson::*)(std::string, std::string)>( &LuaJson::set )
            )
        );

        lua["Json"] = json_ud;

        lua["storage"] = &lua_storage;
    }

    if (perms.acces_gui)   // si gui est autorisé
    {
        lua.new_usertype<LuaGui>("gui",
            "box", &LuaGui::box,
            "canvas", &LuaGui::canvas,
            "image", sol::overload(
                [](LuaGui* gui, LuaWidget* parent, std::string path, int x, int y, int width, int height) -> LuaImage* {
                    return gui->image(parent, path, x, y, width, height, COLOR_WHITE);
                },
                &LuaGui::image
            ),
            "label", &LuaGui::label,
            "input", &LuaGui::input,
            "button", &LuaGui::button,
            "window", &LuaGui::window,
            "switch", &LuaGui::switchb,
            "radio", &LuaGui::radio,
            "vlist", &LuaGui::verticalList,
            "hlist", &LuaGui::horizontalList,
            "checkbox", &LuaGui::checkbox,
            "del", &LuaGui::del,
            "setWindow", &LuaGui::setMainWindow,
            "getWindow", &LuaGui::getMainWindow,            
            "keyboard", &LuaGui::keyboard,
            "showInfoMessage", &LuaGui::showInfoMessage,
            "showWarningMessage", &LuaGui::showWarningMessage,
            "showErrorMessage", &LuaGui::showErrorMessage
        );

        lua["gui"] = &lua_gui;

        lua.new_usertype<LuaWidget>("widget",
            "setX", &LuaWidget::setX,
            "setY", &LuaWidget::setY,
            "setWidth", &LuaWidget::setWidth,
            "setHeight", &LuaWidget::setHeight,
            "getX", &LuaWidget::getX,
            "getY", &LuaWidget::getY,
            "getWidth", &LuaWidget::getWidth,
            "getHeight", &LuaWidget::getHeight,
            "setBackgroundColor", &LuaWidget::setBackgroundColor,
            "setBorderColor",&LuaWidget::setBorderColor,
            "setRadius",&LuaWidget::setRadius,
            "setBorderSize", &LuaWidget::setBorderSize,
            "enable", &LuaWidget::enable,
            "disable", &LuaWidget::disable,
            "isEnabled", &LuaWidget::isEnabled,
            "isTouched", &LuaWidget::isTouched,
            "onClick", &LuaWidget::onClick,
            "getChildAtIndex", &LuaWidget::getChildAtIndex
        );

        lua.new_usertype<LuaWindow>("LuaWindow",
            sol::constructors<LuaWindow(*)()>(), // Empty constructor
            sol::base_classes, sol::bases<LuaWidget>(),
            
            sol::meta_function::garbage_collect,
            sol::destructor(LuaWindow::delete_LuaWindow)
        );

        lua.new_usertype<LuaBox>("LuaBox",
            "setRadius", &LuaBox::setRadius,
            sol::base_classes, sol::bases<LuaWidget>());

        lua.new_usertype<LuaCanvas>("LuaCanvas",
            "setPixel", &LuaCanvas::setPixel,
            "drawRect", &LuaCanvas::drawRect,
            "fillRect", &LuaCanvas::fillRect,
            "drawCircle", &LuaCanvas::drawCircle,
            "fillCircle", &LuaCanvas::fillCircle,
            "drawRoundRect", &LuaCanvas::drawRoundRect,
            "fillRoundRect", &LuaCanvas::fillRoundRect,
            "drawPolygon", &LuaCanvas::drawPolygon,
            "fillPolygon", &LuaCanvas::fillPolygon,
            "drawLine", &LuaCanvas::drawLine,
            "drawText", &LuaCanvas::drawText,
            "drawTextCentered", &LuaCanvas::drawTextCentered,
            "drawTextCenteredInBounds", &LuaCanvas::drawTextCenteredInRect,
            "getTouch", &LuaCanvas::getTouch,
            "onTouch", &LuaCanvas::onTouch,
            sol::base_classes, sol::bases<LuaWidget>());

        lua.new_usertype<LuaImage>("LuaImage",
            "setTransparentColor", &LuaImage::setTransparentColor,

            sol::base_classes, sol::bases<LuaWidget>());

        lua.new_usertype<LuaLabel>("LuaLabel",
            "setText", &LuaLabel::setText,
            "getText", &LuaLabel::getText,
            "setFontSize", &LuaLabel::setFontSize,
            "getTextHeight", &LuaLabel::getTextHeight,
            "getTextWidth", &LuaLabel::getTextWidth,
            "setVerticalAlignment", &LuaLabel::setVerticalAlignment,
            "setHorizontalAlignment", &LuaLabel::setHorizontalAlignment,
            "setTextColor", &LuaLabel::setTextColor,
            sol::base_classes, sol::bases<LuaWidget>());

        lua.new_usertype<LuaInput>("LuaInput",
            "setText", &LuaInput::setText,
            "getText", &LuaInput::getText,
            "setPlaceHolder", &LuaInput::setPlaceHolder,
            "setTitle", &LuaInput::setTitle,
            sol::base_classes, sol::bases<LuaWidget>());
        
        lua.new_usertype<LuaButton>("LuaButton",
            "setText", &LuaButton::setText,
            "getText", &LuaButton::getText,
            "setIcon", &LuaButton::setIcon,
            "setTheme", &LuaButton::setTheme,
            "format", &LuaButton::format,
            sol::base_classes, sol::bases<LuaWidget>());

        lua.new_usertype<LuaSwitch>("LuaSwitch",
            "setState", &LuaSwitch::setState,
            "getState", &LuaSwitch::getState,
            sol::base_classes, sol::bases<LuaWidget>());

        lua.new_usertype<LuaRadio>("LuaRadio",
            "setState", &LuaRadio::setState,
            "getState", &LuaRadio::getState,
            sol::base_classes, sol::bases<LuaWidget>());

        lua.new_usertype<LuaCheckbox>("LuaCheckbox",
            "setState", &LuaCheckbox::setState,
            "getState", &LuaCheckbox::getState,
            sol::base_classes, sol::bases<LuaWidget>());

        lua.new_usertype<LuaVerticalList>("LuaVList",
            "setIndex", &LuaVerticalList::setIndex,
            "setSpaceLine", &LuaVerticalList::setSpaceLine,
            "setSelectionFocus", &LuaVerticalList::setFocus,
            "getSelected", &LuaVerticalList::getSelected,
            "select", &LuaVerticalList::select,
            "setSelectionColor", &LuaVerticalList::setSelectionColor,
            "setAutoSelect", &LuaVerticalList::setAutoSelect,
            sol::base_classes, sol::bases<LuaWidget>());

        lua.new_usertype<LuaHorizontalList>("LuaHList",
            //"add", &LuaHorizontalList::add,
            "setSpaceLine", &LuaHorizontalList::setSpaceLine,
            sol::base_classes, sol::bases<LuaWidget>());

        lua.set("SELECTION_UP", VerticalList::SelectionFocus::UP);
        lua.set("SELECTION_CENTER", VerticalList::SelectionFocus::CENTER);

        lua.set("LEFT_ALIGNMENT", Label::Alignement::LEFT);
        lua.set("RIGHT_ALIGNMENT", Label::Alignement::RIGHT);
        lua.set("CENTER_ALIGNMENT", Label::Alignement::CENTER);
        lua.set("UP_ALIGNMENT", Label::Alignement::UP);
        lua.set("DOWN_ALIGNMENT", Label::Alignement::DOWN);

        lua.set("COLOR_DARK", COLOR_DARK);
        lua.set("COLOR_LIGHT", COLOR_LIGHT);
        lua.set("COLOR_SUCCESS", COLOR_SUCCESS);
        lua.set("COLOR_WARNING", COLOR_WARNING);
        lua.set("COLOR_ERROR", COLOR_ERROR);

        lua.set("COLOR_WHITE", COLOR_WHITE);
        lua.set("COLOR_BLACK", COLOR_BLACK);
        lua.set("COLOR_RED", COLOR_RED);
        lua.set("COLOR_GREEN", COLOR_GREEN);
        lua.set("COLOR_BLUE", COLOR_BLUE);

        lua.set("COLOR_YELLOW", COLOR_YELLOW);
        lua.set("COLOR_GREY", COLOR_GREY);
        lua.set("COLOR_MAGENTA", COLOR_MAGENTA);
        lua.set("COLOR_CYAN", COLOR_CYAN);
        lua.set("COLOR_VIOLET", COLOR_VIOLET);
        lua.set("COLOR_ORANGE", COLOR_ORANGE);
        lua.set("COLOR_GREY", COLOR_GREY);
        lua.set("COLOR_PINK", COLOR_PINK);

        lua.set("COLOR_LIGHT_ORANGE", COLOR_LIGHT_ORANGE);
        lua.set("COLOR_LIGHT_GREEN", COLOR_LIGHT_GREEN);
        lua.set("COLOR_LIGHT_BLUE", COLOR_LIGHT_BLUE);
        lua.set("COLOR_LIGHT_GREY", COLOR_LIGHT_GREY);

        lua.set_function("launch", sol::overload([&](std::string name, std::vector<std::string> arg)
            {
                try{
                    AppManager::get(name)->run(false, arg);
                }
                catch(std::runtime_error &e) {
                    std::cerr << "Erreur: " << e.what() << std::endl;
                    // Ajout message d'erreur
                    GuiManager &guiManager = GuiManager::getInstance();
                    guiManager.showErrorMessage(e.what());
                }

                return true;
            },
            [&](std::string name)
            {
                try{
                    AppManager::get(name)->run(false, {});
                }
                catch(std::runtime_error &e) {
                    std::cerr << "Erreur: " << e.what() << std::endl;
                    // Ajout message d'erreur
                    GuiManager &guiManager = GuiManager::getInstance();
                    guiManager.showErrorMessage(e.what());

                }

                return true;
            }
        )
        );
    }

    if(perms.acces_time)
    {
        lua.new_usertype<LuaTime>("time",
            "monotonic", &LuaTime::monotonic,
            "get", &LuaTime::get,
            "setInterval", &LuaTime::setInterval,
            "setTimeout", &LuaTime::setTimeout,
            "removeInterval", &LuaTime::removeInterval,
            "removeTimeout", &LuaTime::removeTimeout
        );

        lua["time"] = &lua_time;
    }

    if(perms.acces_gsm)
    {
        sol::table luaGSM = lua.create_table();

        luaGSM["newMessage"] = &LuaGSM::newMessage;
        luaGSM["newCall"] = &LuaGSM::newCall;
        luaGSM["endCall"] = &LuaGSM::endCall;
        luaGSM["acceptCall"] = &LuaGSM::acceptCall;
        luaGSM["rejectCall"] = &LuaGSM::rejectCall;
        luaGSM["getNumber"] = &LuaGSM::getNumber;
        luaGSM["getCallState"] = &LuaGSM::getCallState;

        luaGSM["saveContacts"] = &Contacts::save;
        luaGSM["listContacts"] = &Contacts::listContacts;
        luaGSM["deleteContact"] = &Contacts::deleteContact;
        luaGSM["editContact"] = &Contacts::editContact;
        luaGSM["getContact"] = &Contacts::getContact;
        luaGSM["getContactByNumber"] = &Contacts::getByNumber;
        luaGSM["addContact"] = &Contacts::addContact;

        lua.new_usertype<Contacts::contact>("Contact",
            "name", &Contacts::contact::name,
            "phone", &Contacts::contact::phone
        );

        
        lua.new_usertype<Conversations::Message>("Message",
            "message", &Conversations::Message::message,
            "who", &Conversations::Message::who,
            "date", &Conversations::Message::date
        );
        
        luaGSM["getMessages"] = [&](const std::string& number) {
            return LuaGSM::getMessages(number, lua);
        };

        lua["gsm"] = luaGSM;
    }

    {   // load events
        sol::table luaEvents = lua.create_table();

        luaEvents["oncall"] = [&](sol::protected_function func) { this->oncall = func; };
        luaEvents["onmessage"] = [&](sol::protected_function func) { this->onmessage = func; std::cout << "onmessage event registered" << std::endl; };
        luaEvents["onlowbattery"] = [&](sol::protected_function func) { this->onlowbattery = func; };
        luaEvents["oncharging"] = [&](sol::protected_function func) { this->oncharging = func; };
        luaEvents["onmessageerror"] = [&](sol::protected_function func) { this->onmessageerror = func; };

        lua["events"] = luaEvents;
    }

    lua.set_panic(custom_panic_handler);
    //lua.set_exception_handler(sol_exception_handler);
}

#define SAFE_CALL(func, lua, arg) do { \
    sol::protected_function_result result = func(arg); \
    if (!result.valid()) { \
        sol::error err = result; \
        const char* what = err.what(); \
        if (what) { \
            printf("Error from Lua: %s\n", what); \
        } else { \
            printf("Unknown Lua error occurred\n"); \
        } \
    } \
} while(0)

void LuaFile::run(std::vector<std::string> arg)
{
    // run the app

    //lua.set_exception_handler(AppManager::pushError);

    std::cout << "Loading Lua File: " << filename.str() << std::endl;
    storage::FileStream file(filename.str(), storage::READ);
    
    std::string code = file.read();
    file.close();

    lua.script(code);   code.clear();  // load and delete the unnecessary code

    SAFE_CALL(lua["run"], lua, arg);
}

void LuaFile::runBackground(std::vector<std::string> arg)
{
    // run the app in the background

    lua.set_exception_handler(AppManager::pushError);

    std::cout << "Loading Lua File In Background: " << filename.str() << std::endl;
    storage::FileStream file(filename.str(), storage::READ);
    std::string code = file.read();
    file.close();

    lua.script(code);   code = "";  // load and delete the unnecessary code

    sol::protected_function func = lua["background"];
    lua["background"](arg);
}

void LuaFile::wakeup(std::vector<std::string> arg)
{
    sol::protected_function func = lua["wakeup"];
    if (!func.valid())
        return;

    lua["wakeup"](arg);
}

void LuaFile::stop(std::vector<std::string> arg)
{
    const sol::protected_function func = lua.get<sol::protected_function>("quit");;
    if (!func.valid())
        return;

    lua["quit"](arg);
}

void LuaFile::loop()
{
    //lua_gui.update();   // add App Priority To Acces Gui
    lua_time.update();
}