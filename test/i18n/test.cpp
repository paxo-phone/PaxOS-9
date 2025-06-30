#include "i18n.hpp"

#include <cassert>
#include <gtest/gtest.h>
#include <iostream>

TEST(I18NTest, Translations1)
{
    i18n::setLang(i18n::FR);
    i18n::setTextDomain("resources/translations_1/base");

    EXPECT_EQ(i18n::getText("animal"), "chien");
    EXPECT_EQ(i18n::getText("drink"), "café");
    EXPECT_EQ(
        i18n::getText("quote"),
        "Nous sommes ce que nous pensons.\nTout ce que nous sommes naît\navec "
        "nos pensées.\nAvec nos pensées, nous\ncréons le monde."
    );
    EXPECT_EQ(i18n::getText("hello"), "Bonjour monde !");

    EXPECT_EQ(i18n::getLang(), i18n::FR);

    i18n::setLang(i18n::EN);
    i18n::setTextDomain("resources/translations_1/base");

    EXPECT_EQ(i18n::getText("animal"), "dog");
    EXPECT_EQ(i18n::getText("drink"), "coffee");
    EXPECT_EQ(
        i18n::getText("quote"),
        "We are what we think.\nAll that we are arises\nwith our "
        "thoughts.\nWith our thoughts, we\nmake the world."
    );
    EXPECT_EQ(i18n::getText("hello"), "Hello, world !");

    EXPECT_EQ(i18n::getLang(), i18n::EN);
}

TEST(I18NTest, Translations2)
{
    i18n::setLang(i18n::FR);
    i18n::setTextDomain("resources/translations_2/base");

    EXPECT_EQ(i18n::getText("title"), "Titre");
    EXPECT_EQ(i18n::getText("home"), "Accueil");
    EXPECT_EQ(i18n::getText("about"), "À propos");
    EXPECT_EQ(i18n::getText("contact"), "Contact");
    EXPECT_EQ(i18n::getText("welcome message"), "Bonjour\nà tous !");

    EXPECT_EQ(i18n::getLang(), i18n::FR);

    i18n::setLang(i18n::EN);
    i18n::setTextDomain("resources/translations_2/base");

    EXPECT_EQ(i18n::getText("title"), "Title");
    EXPECT_EQ(i18n::getText("home"), "Home");
    EXPECT_EQ(i18n::getText("about"), "About");
    EXPECT_EQ(i18n::getText("contact"), "Contact");
    EXPECT_EQ(i18n::getText("welcome message"), "Hello\neveryone !");

    EXPECT_EQ(i18n::getLang(), i18n::EN);
}
