#ifndef CONTACTS_HPP
#define CONTACTS_HPP

#include <string>
#include <vector>

#define CONTACTS_LOCATION "/apps/contacts"

namespace Contacts
{
    struct contact
    {
        std::string name;
        std::string phone;
    };

    void load();
    void save();
    
    std::vector<contact> listContacts();
    void addContact(contact c);
    void deleteContact(std::string name);
    void editContact(std::string name, contact c);
    contact getContact(std::string name);
    contact getByNumber(std::string number);
};

#endif // CONTACTS_HPP
