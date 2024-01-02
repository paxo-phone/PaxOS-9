#include <iostream>
#include <cassert>

#include "../i18n.hpp"

using namespace i18n; // ! important

void test(void)
{
    /* translations_1 */

    setLang(FR);
    setTextDomain("translations_1/base");
        assert(getText("animal") == "chien");
        assert(getText("drink") == "café");
        assert(getText("quote") == "Nous sommes ce que nous pensons.\nTout ce que nous sommes naît\navec nos pensées.\nAvec nos pensées, nous\ncréons le monde.");
        assert(getText("hello") == "Bonjour monde !");
    assert(getLang() == FR);

    setLang(EN);
    setTextDomain("translations_1/base");
        assert(getText("animal") == "dog");
        assert(getText("drink") == "coffee");
        assert(getText("quote") == "We are what we think.\nAll that we are arises\nwith our thoughts.\nWith our thoughts, we\nmake the world.");
        assert(getText("hello") == "Hello, world !");
    assert(getLang() == EN);

    /* translations_2 */


    setLang(FR);
    setTextDomain("translations_2/base");
        assert(getText("title") == "Titre");
        assert(getText("home") == "Accueil");
        assert(getText("about") == "À propos");
        assert(getText("contact") == "Contact");
        assert(getText("welcome message") == "Bonjour\nà tous !");
    assert(getLang() == FR);

    setLang(EN);
    setTextDomain("translations_2/base");
        assert(getText("title") == "Title");
        assert(getText("home") == "Home");
        assert(getText("about") == "About");
        assert(getText("contact") == "Contact");
        assert(getText("welcome message") == "Hello\neveryone !");
    assert(getLang() == EN);
}

// int main(void)
// {
//     test();
//     return 0;
// }