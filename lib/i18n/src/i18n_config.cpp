#include "i18n_config.hpp"

#include <array>
#include <string>

namespace i18n
{

    lang_t gLang = EN; // default

    std::string langToString(const lang_t lang)
    {
        std::array<std::string, 4> identifiers({"en", "fr", "es", "de"});
        return identifiers[lang]; // because lang is a uint8_t
    }

    void setLang(lang_t lang)
    {
        gLang = lang;
    }

    lang_t getLang(void)
    {
        return gLang;
    }

} // namespace i18n
