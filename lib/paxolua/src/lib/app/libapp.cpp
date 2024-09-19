//
// Created by Charles on 11/09/2024.
//

#include "libapp.hpp"

void paxolua::app::load(const sol::state &lua) {
    auto paxo = lua["paxo"].get<sol::table>();

    // paxo.app
    auto app = paxo["app"].get_or_create<sol::table>(sol::new_table());

    // app.set_function("quit", [&]() {
    //     m_commandQueue.push(QUIT);
    // });
}
