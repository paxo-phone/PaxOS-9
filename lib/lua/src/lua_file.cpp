#include "lua_file.hpp"

#include <gui.hpp>
#include <filestream.hpp>
#include <path.hpp>
#include <hardware.hpp>
#include <threads.hpp>
#include <json.hpp>


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

void LuaFile::run()
{
    lua_setallocf(lua.lua_state(), custom_allocator, NULL);

    // Charger le fichier lua
    storage::FileStream file(filename.str(), storage::READ);
    std::string code = file.read();
    file.close();

    // Charger le module dans l'environnement Lua
    lua.open_libraries(sol::lib::base);
    lua.open_libraries(sol::lib::math);
    lua.open_libraries(sol::lib::table);
    lua.open_libraries(sol::lib::string);


    // Lire la configuration
    storage::FileStream file2(manifest.str(), storage::READ);
    std::string conf = file2.read();
    file2.close();

    std::cout << "conf: " << conf << std::endl;

    nlohmann::json confJson = nlohmann::json::parse(conf);

    // en fonction de la configuration, choisir les permissions
    perms.acces_files = confJson["acces_files"] == "true";
    perms.acces_files_root = confJson["acces_files_root"] == "true";
    perms.acces_gsm = confJson["acces_gsm"] == "true";
    perms.acces_gui = confJson["acces_gui"] == "true";
    perms.acces_hardware = confJson["acces_hardware"] == "true";
    perms.acces_time = confJson["acces_time"] == "true";
    perms.acces_web = confJson["acces_web"] == "true";
    perms.acces_web_paxo = confJson["acces_web_paxo"] == "true";

    confJson.clear();

    // en fonction des permissions, charger certains modules

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
            "file", &LuaStorage::file,
            "mkdir", &LuaStorage::newDir,
            "mvFile", &LuaStorage::renameFile,
            "mvDir", &LuaStorage::renameDir,
            "rmFile", &LuaStorage::deleteFile,
            "rmDir", &LuaStorage::deleteDir,
            "isDir", &LuaStorage::isDir,
            "isFile", &LuaStorage::isFile,
            "listDir", &LuaStorage::listDir
        );


        lua["storage"] = &lua_storage;
    }

    if (perms.acces_gui)   // si gui est autorisé
    {
        lua.new_usertype<LuaGui>("gui",
            "box", &LuaGui::box,
            "image", &LuaGui::image,
            "label", &LuaGui::label,
            "input", &LuaGui::input,
            "button", &LuaGui::button,
            "window", &LuaGui::window,
            "switch", &LuaGui::switchb,
            "radio", &LuaGui::radio,
            "vlist", &LuaGui::verticalList,
            "hlist", &LuaGui::horizontalList,
            "checkbox", &LuaGui::checkbox,
            "setWindow", &LuaGui::setMainWindow
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

        lua.new_usertype<LuaBox>("LuaBox",
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
            "add", &LuaVerticalList::add,
            sol::base_classes, sol::bases<LuaWidget>());

        lua.new_usertype<LuaHorizontalList>("LuaHList",
            "add", &LuaHorizontalList::add,
            sol::base_classes, sol::bases<LuaWidget>());

        lua.set("LEFT_ALIGNMENT", Label::Alignement::LEFT);
        lua.set("RIGHT_ALIGNMENT", Label::Alignement::RIGHT);
        lua.set("CENTER_ALIGNMENT", Label::Alignement::CENTER);
        lua.set("UP_ALIGNMENT", Label::Alignement::UP);
        lua.set("DOWN_ALIGNMENT", Label::Alignement::DOWN);

        lua.new_usertype<LuaWindow>("LuaWindow",
            sol::base_classes, sol::bases<LuaWidget>());
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
        luaGSM["getNumber"] = &LuaGSM::getNumber;
        luaGSM["getCallState"] = &LuaGSM::getCallState;

        lua["gsm"] = luaGSM;
    }

    /*if(perms.acces_web)
    {
        lua.new_usertype<LuaNetwork>("network",
            "HttpClient", &LuaNetwork::createHttpClient
        );

        lua["network"] = &lua_network;

        lua.new_usertype<LuaHttpClient>("HttpClient",
            "get", &LuaHttpClient::get,
            "post", &LuaHttpClient::post,
            sol::meta_function::garbage_collect, sol::destructor([](LuaHttpClient& obj) { obj.~LuaHttpClient(); })
        );
    }*/

    try
    {
        lua.script(code, sol::script_throw_on_error);
        lua.script("run()", sol::script_throw_on_error);

        while (/*!home_button::isPressed() && */lua_gui.mainWindow != nullptr)
        {
            lua_gui.update();
            lua_time.update();
        }
    }
    catch (const sol::error& e)
    {
        std::cerr << "Erreur Lua : " << e.what() << std::endl;

        gui::elements::Window win;
        Label *label = new Label(0, 0, 320, 400);
        
        label->setText(e.what());
        win.addChild(label);
        
        Button *btn = new Button(35, 420, 250, 38);
        btn->setText("Quitter");
        win.addChild(btn);

        while (true)
        {
            win.updateAll();
            if(btn->isTouched())
            {
                return;
            }
        }
    }
}