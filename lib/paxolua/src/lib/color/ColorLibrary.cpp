//
// Created by Charles on 11/09/2024.
//

#include "ColorLibrary.hpp"

#include <libsystem.hpp>
#include <LuaEnvironment.hpp>
#include <SOL2/sol.hpp>
#include <color.hpp>

namespace paxolua::lib {
    void ColorLibrary::load(LuaEnvironment *env) {
        sol::table paxo = env->getPaxoNamespace();
        auto color = paxo["color"].get_or_create<sol::table>();

        color.set("dark", COLOR_DARK);
        color.set("light", COLOR_LIGHT);
        color.set("success", COLOR_SUCCESS);
        color.set("warning", COLOR_WARNING);
        color.set("error", COLOR_ERROR);

        color.set("white", COLOR_WHITE);
        color.set("black", COLOR_BLACK);

        color.set("red", COLOR_RED);
        color.set("green", COLOR_GREEN);
        color.set("blue", COLOR_BLUE);

        color.set("yellow", COLOR_YELLOW);
        color.set("grey", COLOR_GREY);
        color.set("magenta", COLOR_MAGENTA);
        color.set("cyan", COLOR_CYAN);
        color.set("violet", COLOR_VIOLET);
        color.set("orange", COLOR_ORANGE);
        color.set("grey", COLOR_GREY);
        color.set("pink", COLOR_PINK);

        color.set("lightOrange", COLOR_LIGHT_ORANGE);
        color.set("lightGreen", COLOR_LIGHT_GREEN);
        color.set("lightBlue", COLOR_LIGHT_BLUE);
        color.set("lightGrey", COLOR_LIGHT_GREY);

        color.set_function("toColor", [&](const uint8_t r, const uint8_t g, const uint8_t b) -> color_t {
           return graphics::packRGB565(r, g, b);
        });

        color.set_function("toRGB", [&](const color_t rgb) -> std::tuple<uint8_t, uint8_t, uint8_t    > {
            uint8_t r, g, b;

            graphics::unpackRGB565(rgb, &r, &g, &b);

            return std::make_tuple(r, g, b);
        });
    }

    void ColorLibrary::update(LuaEnvironment *env) {

    }
} // paxolua::lib