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

void LuaFile::run(std::vector<std::string> arg)
{
    std::string errors = "";

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

    if(!nlohmann::json::accept(conf))
    {
        errors = "Les permissions de l'app ne sont pas définies ou sont invalides";
    }
    else{
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
                //"add", &LuaVerticalList::add,
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
            lua.set("COLOR_WHITE", COLOR_WHITE);
            lua.set("COLOR_SUCCESS", COLOR_SUCCESS);
            lua.set("COLOR_WARNING", COLOR_WARNING);
            lua.set("COLOR_ERROR", COLOR_ERROR);
            lua.set("COLOR_GREY", COLOR_GREY);

            lua.set_function("launch", sol::overload([&](std::string name, std::vector<std::string> arg)
                {
                    app::AppRequest app = {app::getApp(name), arg};
                    std::cout << "Arguments: " << arg.size() << std::endl;
                    if(app.app.name.empty())
                        return false;
                    
                    app::request = true;
                    app::requestingApp = app;

                    return true;
                },
                [&](std::string name)
                {
                    app::AppRequest app = {app::getApp(name), {}};
                    if(app.app.name.empty())
                        return false;
                    
                    app::request = true;
                    app::requestingApp = app;

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
            lua["arg"] = arg;
            if(arg.size())
                std::cout << "ARGUMENTS!!" << arg[0] << std::endl;
            lua.script(code, sol::script_throw_on_error);   code = "";
            lua.script("run()", sol::script_throw_on_error);

            while (!hardware::getHomeButton() && lua_gui.mainWindow != nullptr)
            {
                lua_gui.update();
                lua_time.update();
                /*try
                {
                    lua.collect_garbage();
                }
                catch(const std::exception& e)
                {
                    std::cerr << e.what() << '\n';
                }*/

                if (app::request)
                {
                    app::request = false;
                    /*for (auto item : lua_gui.widgets)
                        item->widget->free();*/
                    app::runApp(app::requestingApp);
                }
            }

            // si la fonction quit est définie l'appeler avant de fermer l'app
            if (lua["quit"].get<sol::function>()) {
                lua.script("quit()", sol::script_throw_on_error);
            }
        }
        catch (const sol::error& e)
        {
            errors = e.what();
        }
    }

    if(errors.size())
    {
        std::cerr << "Erreur Lua: " << errors << std::endl;

        gui::elements::Window win;
        Label *label = new Label(0, 0, 320, 400);
        
        label->setText(errors);
        win.addChild(label);
        
        Button *btn = new Button(35, 420, 250, 38);
        btn->setText("Quitter");
        win.addChild(btn);

        while (!hardware::getHomeButton())
        {
            win.updateAll();
            if(btn->isTouched())
            {
                return;
            }
        }
    }
}