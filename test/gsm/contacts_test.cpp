#include "contacts.hpp"

#include <gtest/gtest.h>

// Test pour ajouter des contacts
TEST(ContactsTest, AddContacts)
{
    Contacts::contact contact1 = {"hello", "12345"};
    Contacts::contact contact2 = {"world", "67890"};

    Contacts::addContact(contact1);
    Contacts::addContact(contact2);

    auto contacts = Contacts::listContacts();
    ASSERT_EQ(contacts.size(), 2);

    ASSERT_EQ(contacts[0].name, "hello");
    ASSERT_EQ(contacts[0].phone, "12345");

    ASSERT_EQ(contacts[1].name, "world");
    ASSERT_EQ(contacts[1].phone, "67890");
}

// Test pour récupérer les contacts
TEST(ContactsTest, GetContact)
{
    Contacts::contact contact = Contacts::getContact("hello");
    ASSERT_EQ(contact.name, "hello");
    ASSERT_EQ(contact.phone, "12345");

    contact = Contacts::getContact("world");
    ASSERT_EQ(contact.name, "world");
    ASSERT_EQ(contact.phone, "67890");
}
