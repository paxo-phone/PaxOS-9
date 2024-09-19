//
// Created by Charles on 11/09/2024.
//

#include "DebugSAX.hpp"

#include <libsystem.hpp>

namespace json {
    DebugSAX::DebugSAX() = default;

    bool DebugSAX::null() {
        return true;
    }

    bool DebugSAX::boolean(bool val) {
        return true;
    }

    bool DebugSAX::number_integer(number_integer_t val) {
        return true;
    }

    bool DebugSAX::number_unsigned(number_unsigned_t val) {
        return true;
    }

    bool DebugSAX::number_float(number_float_t val, const string_t &s) {
        return true;
    }

    bool DebugSAX::string(string_t &val) {
        return true;
    }

    bool DebugSAX::binary(binary_t &val) {
        return true;
    }

    bool DebugSAX::start_object(std::size_t elements) {
        return true;
    }

    bool DebugSAX::key(string_t &val) {
        return true;
    }

    bool DebugSAX::end_object() {
        return true;
    }

    bool DebugSAX::start_array(std::size_t elements) {
        return true;
    }

    bool DebugSAX::end_array() {
        return true;
    }

    bool DebugSAX::parse_error(std::size_t position, const std::string &last_token,
                                const nlohmann::detail::exception &ex) {

        libsystem::log("Parse error, at " + std::to_string(position) + ", last token: " + last_token + ". Error: " + ex.what());

        return false;

    }
} // json