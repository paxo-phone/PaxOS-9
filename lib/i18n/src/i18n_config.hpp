#ifndef I18N_CONFIG_HPP
#define I18N_CONFIG_HPP

#include <string>
#include <cstdint>

namespace i18n {

    enum lang_t : uint8_t {
        EN, // 0
        FR,
        ES,
        DE,
    };

    std::string langToString(const lang_t lang);

    void setLang(lang_t lang);
    lang_t getLang(void);
}

#endif /* I18N_CONFIG_HPP */