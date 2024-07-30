#include "lua_file.hpp"

#include <gui.hpp>
#include <filestream.hpp>
#include <path.hpp>
#include <hardware.hpp>
#include <threads.hpp>
#include <json.hpp>
#include <app.hpp>
#include <contacts.hpp>


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
    AppManager::App& app = AppManager::get(L);

    app.errors += std::string(lua_tostring(L, -1)) + "\n";
    app.app_state = AppManager::App::AppState::NOT_RUNNING;

    const char* msg = lua_tostring(L, -1);
    std::cerr << "Lua panic: " << msg << std::endl;
    
    return 0;
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
            "keyboard", &LuaGui::keyboard
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
            "setMainColor", &LuaWidget::setBackgroundColor,
            "enable", &LuaWidget::enable,
            "disable", &LuaWidget::disable,
            "isTouched", &LuaWidget::isTouched,
            "onClick", &LuaWidget::onClick
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
            sol::base_classes, sol::bases<LuaWidget>());

        lua.new_usertype<LuaHorizontalList>("LuaHList",
            //"add", &LuaHorizontalList::add,
            sol::base_classes, sol::bases<LuaWidget>());

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

            lua.set("COLOR_LIGHT_GREEN", COLOR_LIGHT_GREEN);
            lua.set("COLOR_LIGHT_BLUE", COLOR_LIGHT_BLUE);
            lua.set("COLOR_LIGHT_GREY", COLOR_LIGHT_GREY);

        lua.set_function("launch", sol::overload([&](std::string name, std::vector<std::string> arg)
            {
                AppManager::get(name).run(false, arg);

                return true;
            },
            [&](std::string name)
            {
                AppManager::get(name).run(false, {});

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

    lua["run"](arg);
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
    sol::protected_function func = lua["quit"];
    if (!func.valid())
        return;

    lua["quit"](arg);
}

void LuaFile::loop()
{
    //lua_gui.update();   // add App Priority To Acces Gui
    lua_time.update();
}