#include "lua_file.hpp"

#include <gui.hpp>
#include <filestream.hpp>
#include <path.hpp>
#include <hardware.hpp>
#include <threads.hpp>


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

LuaFile::LuaFile(storage::Path filename)
    :lua_gui(this),
    lua_storage(this),
    lua_time(this)/*,
    lua_network(this)*/
{
    this->filename = filename;
    this->directory = filename / storage::Path("..");
}

LuaFile::~LuaFile()
{
    // libérer les ressources (events, etc)
}

void LuaFile::run()
{
    // Charger le fichier lua
    storage::FileStream file(filename.str(), storage::READ);
    std::string code = file.read();
    file.close();

    std::cout << code << std::endl;

    // Charger le module dans l'environnement Lua
    lua.open_libraries(sol::lib::base);
    lua.open_libraries(sol::lib::math);
    lua.open_libraries(sol::lib::table);
    lua.open_libraries(sol::lib::string);

    std::cout << 1 << std::endl;

    // Lire la configuration
    storage::FileStream file2((directory / "conf.txt").str(), storage::READ);
    std::string conf = file2.read();
    file2.close();

    // en fonction de la configuration, choisir les permissions

    // perms....

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
            //"getColor", &LuaWidget::getColor,
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
            //"setBold", &LuaLabel::setBold,
            //"setItalic", &LuaLabel::setItalic,
            "setVerticalAlignment", &LuaLabel::setVerticalAlignment,
            "setHorizontalAlignment", &LuaLabel::setHorizontalAlignment,
            sol::base_classes, sol::bases<LuaWidget>());

        lua.new_usertype<LuaInput>("LuaInput",
            "setText", &LuaInput::setText,
            "getText", &LuaInput::getText,
            //"setFontSize", &LuaInput::setFontSize,
            //"getTextHeight", &LuaInput::getTextHeight,
            "onChange", &LuaInput::onChange,
            //"setVerticalAlignment", &LuaInput::setVerticalAlignment,
            //"setHorizontalAlignment", &LuaInput::setHorizontalAlignment,
            sol::base_classes, sol::bases<LuaWidget>());
        
        lua.new_usertype<LuaButton>("LuaButton",
            "setText", &LuaButton::setText,
            "getText", &LuaButton::getText,
            //"setFontSize", &LuaButton::setFontSize,
            //"getTextHeight", &LuaButton::getTextHeight,
            //"setVerticalAlignment", &LuaButton::setVerticalAlignment,
            //"setHorizontalAlignment", &LuaButton::setHorizontalAlignment,
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

    std::cout << 2 << std::endl;

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
        return;
    }
}