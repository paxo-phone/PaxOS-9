//
// Created by Charles on 11/09/2024.
//

#ifndef CUSTOMSAX_HPP
#define CUSTOMSAX_HPP

#include "json.hpp"

namespace json {
    class DebugSAX: public nlohmann::json_sax<nlohmann::json> {
    public:
        DebugSAX();

        bool null() override;

        bool boolean(bool val) override;

        bool number_integer(number_integer_t val) override;

        bool number_unsigned(number_unsigned_t val) override;

        bool number_float(number_float_t val, const string_t &s) override;

        bool string(string_t &val) override;

        bool binary(binary_t &val) override;

        bool start_object(std::size_t elements) override;

        bool key(string_t &val) override;

        bool end_object() override;

        bool start_array(std::size_t elements) override;

        bool end_array() override;

        bool parse_error(std::size_t position, const std::string &last_token,
                         const nlohmann::detail::exception &ex) override;
    };
} // json

#endif //CUSTOMSAX_HPP
