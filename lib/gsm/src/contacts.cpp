#include "contacts.hpp"

#include <filestream.hpp>
#include <iostream>
#include <json.hpp>
#include <path.hpp>

using namespace storage;

namespace Contacts
{
    std::vector<contact> contactList;

    void load()
    {
        storage::Path path(CONTACTS_LOCATION);
        storage::FileStream stream((path / "list.json").str(), storage::Mode::READ);
        std::string file = stream.read();
        stream.close();

        nlohmann::json json;
        if (nlohmann::json::accept(file))
        {
            try
            {
                json = nlohmann::json::parse(file);
                for (const auto& contact : json)
                {
                    Contacts::contact newContact;
                    newContact.name = contact["name"];
                    newContact.phone = contact["phone"];

                    contactList.push_back(newContact);
                }
            }
            catch (const nlohmann::json::exception& e)
            {
                std::cerr << "Error parsing contacts: " << e.what() << std::endl;
            }
        }
    }

    void save()
    {
        nlohmann::json json;

        for (const auto& contact : contactList)
        {
            nlohmann::json contactJson;
            contactJson["name"] = contact.name;
            contactJson["phone"] = contact.phone;

            json.push_back(contactJson);
        }

        std::string file = json.dump();

        storage::Path path(CONTACTS_LOCATION);
        storage::FileStream stream((path / "list.json").str(), storage::Mode::WRITE);
        stream.write(file);
        stream.close();
    }

    std::vector<contact> listContacts()
    {
        return contactList;
    }

    void addContact(contact c)
    {
        contactList.push_back(c);
    }

    void deleteContact(std::string name)
    {
        contactList.erase(
            std::remove_if(
                contactList.begin(),
                contactList.end(),
                [name](const contact& c)
                {
                    return c.name == name;
                }
            ),
            contactList.end()
        );
    }

    void editContact(std::string name, contact c)
    {
        for (auto& contact : contactList)
        {
            if (contact.name == name)
            {
                contact = c;
                break;
            }
        }
    }

    contact getContact(std::string name)
    {
        for (const auto& contact : contactList)
            if (contact.name == name)
                return contact;

        // Return an empty contact if no match is found
        return {"", ""};
    }

    contact getByNumber(std::string number)
    {
        for (const auto& contact : contactList)
            if (contact.phone == number)
                return contact;

        // Return an empty contact if no match is found
        return {"", ""};
    }
}; // namespace Contacts
