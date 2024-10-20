# PaxOS GSM Module Documentation

This document details the implementation of the GSM module within the PaxOS operating system. The module handles communication with a GSM/GPRS modem, enabling functionalities like sending/receiving SMS messages, making/receiving calls, and managing MMS.

## 1. `contacts.cpp` / `contacts.hpp`

This module manages contact information. Contacts are stored in a JSON file (`/apps/contacts/list.json`) and loaded into memory for quick access.

### Data Structures

* **`Contacts::contact`:**  Represents a single contact with `name` (std::string) and `phone` (std::string) fields.

### Functions

* **`Contacts::load()`:** Loads contacts from the `list.json` file into the `contactList` vector. Handles JSON parsing errors.
* **`Contacts::save()`:** Saves the `contactList` to the `list.json` file in JSON format.
* **`Contacts::listContacts()`:** Returns a copy of the `contactList`.
* **`Contacts::addContact(contact c)`:** Adds a new contact to the `contactList`.
* **`Contacts::deleteContact(std::string name)`:** Deletes a contact with the given name from the `contactList`.
* **`Contacts::editContact(std::string name, contact c)`:** Edits the contact with the given name, updating its information with the provided `contact` object.
* **`Contacts::getContact(std::string name)`:** Returns the `contact` object associated with the given name. Returns an empty contact if not found.
* **`Contacts::getByNumber(std::string number)`:** Returns the `contact` object associated with the given phone number. Returns an empty contact if not found.


## 2. `conversation.cpp` / `conversation.hpp`

This module handles conversation data, including loading and saving messages for each conversation.

### Data Structures

* **`Conversations::Message`:** Represents a single message with `message` (std::string), `who` (bool indicating sender - false for self, true for other), and `date` (std::string) fields.
* **`Conversations::Conversation`:** Represents a conversation with a specific `number` (std::string) and a vector of `messages` (std::vector<Message>).

### Functions

* **`Conversations::loadConversation(const storage::Path &filePath, Conversation &conv)`:** Loads a conversation from the specified JSON file.  Handles file errors and JSON parsing exceptions.
* **`Conversations::saveConversation(const storage::Path &filePath, const Conversation &conv)`:** Saves a conversation to the specified JSON file. Limits the number of saved messages to `MAX_MESSAGES` (40), keeping the most recent ones. Creates the necessary directories if they don't exist.


## 3. `gsm.cpp` / `gsm.hpp`

This module provides the core GSM functionality, interacting directly with the GSM modem. It manages calls, SMS, MMS, network status, and battery level.

### Constants

* `BAUDRATE`: Default baud rate for serial communication (921600).
* `MESSAGES_LOCATION`: Directory for storing message data (`apps/messages/data`).
* `MESSAGES_IMAGES_LOCATION`: Directory for storing MMS images (`apps/messages/images`).
* `MESSAGES_NOTIF_LOCATION`: File storing unread message notifications (`apps/messages/unread.txt`).
* `MAX_MESSAGES`: Maximum number of messages stored per conversation (40).

### Data Structures

* **`GSM::Request`:** Represents a request to be executed by the GSM module, with a `function` (std::function<void(void)>) and a `priority` (enum).
* **`GSM::Key`:** Represents a key string to watch for in the GSM modem's responses, with a `key` (std::string) and a `function` (std::function<void()>) to execute when the key is found.
* **`GSM::State`:** Represents the current state of the GSM module (call state, call failure, calling number).
* **`GSM::Message`:** Represents an SMS message, with `number`, `message`, and `date`.
* **`GSM::ExternalEvents`:** Namespace containing callback functions for incoming calls, new messages, and message errors.
* **`GSM::Stream::Chunk`:**  Used for streaming MMS data; contains a data pointer and size. (ESP_PLATFORM only)

### Functions

* **`GSM::init()`:** Initializes the GSM modem, including setting baud rate and handling modem power-on (ESP_PLATFORM specific).
* **`GSM::reInit()`:** Re-initializes the serial communication after clock speed changes.
* **`GSM::download(uint64_t timeout = 50)`:** Reads data from the GSM modem's serial port with a timeout.
* **`GSM::send(const std::string &message, const std::string &answerKey, uint64_t timeout = 200)`:** Sends a command to the GSM modem and waits for a specific response key. Returns the modem's full response.
* **`GSM::appendRequest(Request request)`:** Adds a request to the queue of requests to be processed by the GSM module.
* **`GSM::process()`:** Processes received data, checking for key strings and executing associated functions.
* **`GSM::checkRequest()`:** Executes pending requests in the queue, prioritizing high-priority requests.
* **`GSM::onRinging()`:** Handles incoming call events, updating the module's state.
* **`GSM::onHangOff()`:** Handles call hang-up events.
* **`GSM::onMessage()`:** Handles incoming SMS/MMS messages. Decodes PDUs, saves messages to conversations, and triggers the `onNewMessage` callback.
* **`GSM::sendMessage(const std::string &number, const std::string &message)`:** Sends an SMS message. Saves the sent message to the conversation history.
* **`GSM::newMessage(std::string number, std::string message)`:** Queues a message to be sent.
* **`GSM::sendCall(const std::string &number)`:** Initiates a call to the specified number.
* **`GSM::newCall(std::string number)`:** Queues a call to be made.
* **`GSM::endCall()`:** Ends the current call.
* **`GSM::acceptCall()`:** Accepts an incoming call.
* **`GSM::rejectCall()`:** Rejects an incoming call.
* **`GSM::getVoltage()`:** Retrieves the battery voltage.
* **`GSM::getBatteryLevel()`:** Calculates and returns the battery level (0.0 - 1.0) based on voltage.
* **`GSM::updateHour()`:** Updates the current time from the GSM modem.
* **`GSM::getHour()`:** Queues a request to update the time.
* **`GSM::getNetworkStatus()`:** Returns the network signal quality.
* **`GSM::updateNetworkQuality()`:** Updates the network signal quality.
* **`GSM::getNetworkQuality()`:** Queues a request to update the network quality.
* **`GSM::isFlightMode()`:** Checks if flight mode is enabled.
* **`GSM::setFlightMode(bool mode)`:** Sets the flight mode.
* **`GSM::run()`:** Main loop of the GSM module. Handles initialization, request processing, and event handling.
* **`GSM::getHttpMMS(std::string number, std::string url)`:** Downloads and processes MMS messages (ESP_PLATFORM only).


## 4. `message.cpp` / `message.hpp`

Provides functions for loading and saving messages from/to JSON files. This module seems redundant given the similar functionality in `conversation.cpp/hpp`, and its usage isn't clear within the provided code.

### Data Structures

* **`Message::Message`:**  Same structure as `Conversations::Message`.

### Functions

* **`Message::loadMessages(const std::string& filePath, std::vector<Message>& messages)`:** Loads messages from a JSON file.
* **`Message::saveMessages(const std::string& filePath, const std::vector<Message>& messages)`:** Saves messages to a JSON file.


## 5. `pdu.cpp` / `pdu.hpp`

This module handles the decoding of PDU-formatted messages received from the GSM modem.

### Data Structures

* **`PDU`:** Contains the decoded information from a PDU string: `sender` (std::string), `message` (std::string), `url` (std::string for MMS URLs), and `type` (enum PDU_type: SMS, MMS, UNKNOWN).

### Functions

* **`decodePDU(std::string pdu)`:** Decodes a PDU string and returns a `PDU` object containing the extracted information (sender, message, URL, type).