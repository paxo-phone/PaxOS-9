#ifndef I18N_TRANSLATE_HPP
#define I18N_TRANSLATE_HPP

#include <string>

namespace i18n
{

    void setTextDomain(const std::string& domain);
    std::string getText(const std::string& key);

} // namespace i18n

#endif /* I18N_TRANSLATE_HPP */
