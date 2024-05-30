#include <gtest/gtest.h>
#include "gsm.hpp"

// Test d'initialisation du module GSM
TEST(GSMTest, Initialization) {
    GSM::init();
    ASSERT_TRUE(true);
}

// Test de l'envoi de message
TEST(GSMTest, SendMessage) {
    GSM::newMessage("0612345678", "Test message");
    // std::cout << "msg send : " << GSM::messages.size() << std::endl;
    ASSERT_FALSE(GSM::messages.empty());
    if (!GSM::messages.empty()) {
        ASSERT_EQ(GSM::messages.back().number, "0612345678");
        ASSERT_EQ(GSM::messages.back().message, "Test message");
    }
}

// Test de la réception de message
TEST(GSMTest, ReceiveMessage) {
    GSM::data = "+CMTI: \"SM\",1\n";
    GSM::onMessage();
    // std::cout << "msg receive : " << GSM::messages.size() << std::endl;
    ASSERT_FALSE(GSM::messages.empty());
    if (!GSM::messages.empty()) {
        ASSERT_EQ(GSM::messages.back().number, "0612345678");
        ASSERT_EQ(GSM::messages.back().message, "Test message");
    }
}
