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

    void load(); // load the contacts
    void save(); // save the contacts

    std::vector<contact>
        listContacts();         // local contact list (synchronized using load and save functions)
    void addContact(contact c); // add a new contact {name, phone}
    void deleteContact(std::string name);          // delete a contact by name
    void editContact(std::string name, contact c); // edit a contact by name
    contact getContact(std::string name);          // get a contact by name
    contact getByNumber(std::string number);       // get a contact by number
}; // namespace Contacts

#endif // CONTACTS_HPP
