#include "lua_network.hpp"

#include "lua_file.hpp"

#include <iostream>
#include <memory>
#include <string_view>

// --- LuaHttpClient Implementation ---

LuaHttpClient::LuaHttpClient(LuaFile* lua) : m_lua(lua) {}

LuaHttpClient::~LuaHttpClient() {}

void LuaHttpClient::get(const std::string& url, sol::table callbacks)
{
    auto request = std::make_shared<Network::Request>();
    request->url = url;
    request->method = Network::HttpMethod::GET;

    sol::function on_response = callbacks["on_response"];
    sol::function on_data = callbacks["on_data"];
    sol::function on_complete = callbacks["on_complete"];

    auto response_data = std::make_shared<std::string>();

    request->on_data = [response_data, on_data](const char* data, int len)
    {
        response_data->append(data, len);
        if (on_data)
            on_data(std::string_view(data, len));
    };

    if (on_response)
    {
        request->on_response = [on_response](int http_code)
        {
            on_response(http_code);
        };
    }

    if (on_complete)
    {
        request->on_complete = [on_complete, response_data](Network::NetworkStatus status)
        {
            on_complete(static_cast<int>(status), *response_data);
        };
    }

    Network::submitRequest(request);
}

void LuaHttpClient::post(const std::string& url, const std::string& body, sol::table options)
{
    auto request = std::make_shared<Network::Request>();
    request->url = url;
    request->method = Network::HttpMethod::POST;
    request->post_body = body;

    if (options)
    {
        sol::table headers = options["headers"];
        if (headers)
            for (auto& pair : headers)
                request->headers[pair.first.as<std::string>()] = pair.second.as<std::string>();

        sol::function on_response = options["on_response"];
        sol::function on_data = options["on_data"];
        sol::function on_complete = options["on_complete"];

        auto response_data = std::make_shared<std::string>();

        request->on_data = [response_data, on_data](const char* data, int len)
        {
            response_data->append(data, len);
            if (on_data)
                on_data(std::string_view(data, len));
        };

        if (on_response)
        {
            request->on_response = [on_response](int http_code)
            {
                on_response(http_code);
            };
        }

        if (on_complete)
        {
            request->on_complete = [on_complete, response_data](Network::NetworkStatus status)
            {
                on_complete(static_cast<int>(status), *response_data);
            };
        }
    }

    Network::submitRequest(request);
}

// --- LuaNetwork Implementation ---

LuaNetwork::LuaNetwork(LuaFile* lua) : m_lua(lua) {}

std::shared_ptr<LuaHttpClient> LuaNetwork::createHttpClient()
{
    return std::make_shared<LuaHttpClient>(m_lua);
}
