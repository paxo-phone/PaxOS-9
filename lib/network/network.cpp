#include "network.hpp"
#include "threads.hpp"
#include "delay.hpp"
#include "gsm2.hpp"

#ifdef ESP_PLATFORM
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#endif

#include <deque>

#define NETWORK_TAG "Network"

namespace Network
{
    // --- Internal State and Singleton ---

    class NetworkManager {
    public:
        // Singleton access
        static NetworkManager& getInstance() {
            static NetworkManager instance;
            return instance;
        }

        void init();
        void submitRequest(std::shared_ptr<Request> request);
        void setRoutingPolicy(RoutingPolicy policy) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_policy = policy;
        }
        RoutingPolicy getRoutingPolicy() {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_policy;
        }

        // Wi-Fi Management
        void enableWifi();
        void disableWifi();
        bool connectWifi(const std::string& ssid, const std::string& password);
        void disconnectWifi();
        bool isWifiConnected() { return m_isWifiConnected; }

        NetworkInterface getBestAvailableInterface();

        // Main processing loop, called by the thread
        void processQueue();

    private:
        NetworkManager() = default;
        ~NetworkManager() = default;
        NetworkManager(const NetworkManager&) = delete;
        NetworkManager& operator=(const NetworkManager&) = delete;

        void _executeRequest(std::shared_ptr<Request> request);
        void _executeWifiRequest(std::shared_ptr<Request> request);
        void _executeCellularRequest(std::shared_ptr<Request> request);
        void _completeCurrentRequest(NetworkStatus status);
        
        // ESP-IDF http client event handler
        #ifdef ESP_PLATFORM
        static esp_err_t _httpEventHandler(esp_http_client_event_t* evt);
        #endif

        // Wi-Fi event handler
        #ifdef ESP_PLATFORM
        static void _wifiEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
        #endif


        std::mutex m_mutex;
        std::deque<std::shared_ptr<Request>> m_requestQueue;
        std::shared_ptr<Request> m_currentRequest = nullptr;
        RoutingPolicy m_policy = RoutingPolicy::WIFI_PREFERRED;
        uint32_t m_nextRequestId = 0;

        // Wi-Fi State
        bool m_isWifiEnabled = false;
        bool m_isWifiConnected = false;
        
        #ifdef ESP_PLATFORM
        esp_http_client_handle_t m_httpClient = nullptr;
        #endif
    };

    // --- Singleton Public API Implementation ---

    void init() {
        NetworkManager::getInstance().init();
    }
    void submitRequest(std::shared_ptr<Request> request) {
        NetworkManager::getInstance().submitRequest(request);
    }
    void enableWifi() { NetworkManager::getInstance().enableWifi(); }
    void disableWifi() { NetworkManager::getInstance().disableWifi(); }
    bool connectWifi(const std::string& ssid, const std::string& password) {
        return NetworkManager::getInstance().connectWifi(ssid, password);
    }
    void disconnectWifi() { NetworkManager::getInstance().disconnectWifi(); }
    void setRoutingPolicy(RoutingPolicy policy) {
        NetworkManager::getInstance().setRoutingPolicy(policy);
    }
    RoutingPolicy getRoutingPolicy() {
        return NetworkManager::getInstance().getRoutingPolicy();
    }
    NetworkInterface getBestAvailableInterface() {
        return NetworkManager::getInstance().getBestAvailableInterface();
    }
    bool isWifiConnected() {
        return NetworkManager::getInstance().isWifiConnected();
    }

    // --- Thread Function ---
    void network_thread(void* arg) {
        ESP_LOGI(NETWORK_TAG, "Network processing thread started.");
        while (true) {
            NetworkManager::getInstance().processQueue();
            PaxOS_Delay(20); // Yield to other tasks
        }
    }

    // --- NetworkManager Method Implementations ---

    void NetworkManager::init() {
        #ifdef ESP_PLATFORM
        ESP_LOGI(NETWORK_TAG, "Initializing Network Manager...");
        // Initialize TCP/IP adapter
        esp_netif_init();
        // Initialize event loop
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        // Register Wi-Fi event handlers
        ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &_wifiEventHandler, this));
        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &_wifiEventHandler, this));
        #endif
    }
    
    void NetworkManager::submitRequest(std::shared_ptr<Request> request) {
        std::lock_guard<std::mutex> lock(m_mutex);
        request->id = m_nextRequestId++;
        m_requestQueue.push_back(request);
        ESP_LOGI(NETWORK_TAG, "Queued request ID %d for URL: %s", request->id, request->url.c_str());
    }

    NetworkInterface NetworkManager::getBestAvailableInterface() {
        std::lock_guard<std::mutex> lock(m_mutex);
        switch (m_policy) {
            case RoutingPolicy::WIFI_PREFERRED:
                if (m_isWifiConnected) return NetworkInterface::WIFI;
                if (Gsm::isConnected()) return NetworkInterface::CELLULAR;
                break;
            case RoutingPolicy::WIFI_ONLY:
                if (m_isWifiConnected) return NetworkInterface::WIFI;
                break;
            case RoutingPolicy::CELLULAR_ONLY:
                if (Gsm::isConnected()) return NetworkInterface::CELLULAR;
                break;
            case RoutingPolicy::NONE:
                return NetworkInterface::NONE;
        }
        return NetworkInterface::NONE;
    }

    void NetworkManager::processQueue() {
        // This function is the heart of the processing thread.
        // It's designed to be called in a tight loop.

        // 1. Check if we are busy. If so, do nothing.
        if (m_currentRequest != nullptr) {
            return;
        }

        // 2. Try to get a new request from the queue
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_requestQueue.empty()) {
                return; // Nothing to do
            }
            m_currentRequest = m_requestQueue.front();
            m_requestQueue.pop_front();
        }

        // 3. We have a request, let's process it
        if (m_currentRequest) {
            ESP_LOGI(NETWORK_TAG, "Processing request ID %d...", m_currentRequest->id);
            _executeRequest(m_currentRequest);
        }
    }
    
    void NetworkManager::_executeRequest(std::shared_ptr<Request> request) {
        NetworkInterface interface = getBestAvailableInterface();

        switch (interface) {
            case NetworkInterface::WIFI:
                ESP_LOGI(NETWORK_TAG, "Routing request ID %d via Wi-Fi.", request->id);
                _executeWifiRequest(request);
                break;
            case NetworkInterface::CELLULAR:
                ESP_LOGI(NETWORK_TAG, "Routing request ID %d via Cellular.", request->id);
                _executeCellularRequest(request);
                break;
            case NetworkInterface::NONE:
            default:
                ESP_LOGE(NETWORK_TAG, "No route available for request ID %d.", request->id);
                _completeCurrentRequest(NetworkStatus::NO_ROUTE);
                break;
        }
    }

    void NetworkManager::_completeCurrentRequest(NetworkStatus status) {
        if (m_currentRequest) {
            ESP_LOGI(NETWORK_TAG, "Completing request ID %d with status %d.", m_currentRequest->id, static_cast<int>(status));
            if (m_currentRequest->on_complete) {
                m_currentRequest->on_complete(status);
            }
            // Clear current request to allow the next one to be processed
            m_currentRequest = nullptr;
        }
    }

    // --- Wi-Fi Specific Implementation ---
#ifdef ESP_PLATFORM
    void NetworkManager::enableWifi() {
        if (m_isWifiEnabled) return;
        ESP_LOGI(NETWORK_TAG, "Enabling Wi-Fi.");
        esp_netif_create_default_wifi_sta();
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_start());
        m_isWifiEnabled = true;
    }
    
    void NetworkManager::disableWifi() {
        if (!m_isWifiEnabled) return;
        ESP_LOGI(NETWORK_TAG, "Disabling Wi-Fi.");
        disconnectWifi();
        ESP_ERROR_CHECK(esp_wifi_stop());
        ESP_ERROR_CHECK(esp_wifi_deinit());
        m_isWifiEnabled = false;
    }

    bool NetworkManager::connectWifi(const std::string& ssid, const std::string& password) {
        if (!m_isWifiEnabled) {
            enableWifi();
        }
        
        ESP_LOGI(NETWORK_TAG, "Connecting to Wi-Fi SSID: %s", ssid.c_str());
        wifi_config_t wifi_config = {};
        strncpy((char*)wifi_config.sta.ssid, ssid.c_str(), sizeof(wifi_config.sta.ssid));
        strncpy((char*)wifi_config.sta.password, password.c_str(), sizeof(wifi_config.sta.password));
        
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        esp_err_t err = esp_wifi_connect();
        return err == ESP_OK;
    }

    void NetworkManager::disconnectWifi() {
        if (m_isWifiConnected) {
            ESP_LOGI(NETWORK_TAG, "Disconnecting from Wi-Fi.");
            ESP_ERROR_CHECK(esp_wifi_disconnect());
        }
    }

    void NetworkManager::_wifiEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
        NetworkManager* self = static_cast<NetworkManager*>(arg);
        if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
            ESP_LOGI(NETWORK_TAG, "Wi-Fi station started.");
        } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
            ESP_LOGI(NETWORK_TAG, "Wi-Fi disconnected.");
            self->m_isWifiConnected = false;
        } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
            ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
            ESP_LOGI(NETWORK_TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
            self->m_isWifiConnected = true;
        }
    }
    
    esp_err_t NetworkManager::_httpEventHandler(esp_http_client_event_t* evt) {
        NetworkManager* self = &NetworkManager::getInstance();
        if (!self->m_currentRequest) {
            return ESP_OK; // Should not happen
        }
        auto request = self->m_currentRequest;

        switch(evt->event_id) {
            case HTTP_EVENT_ERROR:
                ESP_LOGE(NETWORK_TAG, "HTTP_EVENT_ERROR");
                self->_completeCurrentRequest(NetworkStatus::CONNECTION_FAILED);
                break;
            case HTTP_EVENT_ON_CONNECTED:
                ESP_LOGI(NETWORK_TAG, "HTTP_EVENT_ON_CONNECTED");
                break;
            case HTTP_EVENT_HEADER_SENT:
                ESP_LOGI(NETWORK_TAG, "HTTP_EVENT_HEADER_SENT");
                break;
            case HTTP_EVENT_ON_HEADER:
                // This event is where we learn the status code.
                if (request->on_response) {
                    request->on_response(esp_http_client_get_status_code(evt->client));
                }
                ESP_LOGI(NETWORK_TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
                break;
            case HTTP_EVENT_ON_DATA:
                ESP_LOGI(NETWORK_TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
                if (request->on_data) {
                    request->on_data((const char*)evt->data, evt->data_len);
                }
                break;
            case HTTP_EVENT_ON_FINISH:
                ESP_LOGI(NETWORK_TAG, "HTTP_EVENT_ON_FINISH");
                self->_completeCurrentRequest(NetworkStatus::OK);
                break;
            case HTTP_EVENT_DISCONNECTED:
                ESP_LOGI(NETWORK_TAG, "HTTP_EVENT_DISCONNECTED");
                // This event can fire after a successful request. We only treat it as an error
                // if the request wasn't already completed by ON_FINISH.
                if (self->m_currentRequest != nullptr) {
                   self->_completeCurrentRequest(NetworkStatus::REQUEST_FAILED);
                }
                break;
        }
        return ESP_OK;
    }

    void NetworkManager::_executeWifiRequest(std::shared_ptr<Request> request) {
        esp_http_client_config_t config = {};
        config.url = request->url.c_str();
        config.event_handler = _httpEventHandler;
        config.user_data = this; // Pass manager instance
        config.disable_auto_redirect = true;

        m_httpClient = esp_http_client_init(&config);
        
        // Set headers
        for(const auto& header : request->headers) {
            esp_http_client_set_header(m_httpClient, header.first.c_str(), header.second.c_str());
        }

        // Set method and post data
        if (request->method == HttpMethod::POST) {
            esp_http_client_set_method(m_httpClient, HTTP_METHOD_POST);
            esp_http_client_set_post_field(m_httpClient, request->post_body.c_str(), request->post_body.length());
        } else {
            esp_http_client_set_method(m_httpClient, HTTP_METHOD_GET);
        }

        esp_err_t err = esp_http_client_perform(m_httpClient);

        if (err != ESP_OK) {
            ESP_LOGE(NETWORK_TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
            _completeCurrentRequest(NetworkStatus::REQUEST_FAILED);
        }

        esp_http_client_cleanup(m_httpClient);
        m_httpClient = nullptr;
    }
#else
    // Dummy implementations for non-ESP platforms
    void NetworkManager::init() {}
    void NetworkManager::enableWifi() {}
    void NetworkManager::disableWifi() {}
    bool NetworkManager::connectWifi(const std::string&, const std::string&) { return false; }
    void NetworkManager::disconnectWifi() {}
    void NetworkManager::_executeWifiRequest(std::shared_ptr<Request> request) {
        // On native, we can't do a real Wi-Fi request, so fail it.
        _completeCurrentRequest(NetworkStatus::MODULE_ERROR);
    }
#endif


    // --- Cellular Specific Implementation ---

    void NetworkManager::_executeCellularRequest(std::shared_ptr<Request> request) {
        ESP_LOGI(NETWORK_TAG, "Executing cellular request (placeholder).");
        
        //
        // *** IMPORTANT ***
        // The current Gsm::httpGet is not suitable for this generic module.
        // It's recommended to add a new, more powerful function to gsm2.cpp/hpp.
        //
        // PROPOSED ADDITION TO gsm2.hpp:
        //
        // struct GsmHttpRequest {
        //     std::string url;
        //     Network::HttpMethod method;
        //     std::map<std::string, std::string> headers;
        //     std::string post_body;
        //     std::function<void(int http_code)> on_response;
        //     std::function<void(const char* data, int len)> on_data;
        //     std::function<void(Network::NetworkStatus status)> on_complete;
        // };
        // void httpGenericRequest(GsmHttpRequest gsm_request);
        //
        
        // Placeholder implementation using the existing, limited Gsm::httpGet
        // We will adapt our generic callbacks to the ones Gsm::httpGet expects.
        
        Gsm::HttpGetCallbacks callbacks;
        
        // This is a rough translation. The Gsm module should be updated to handle this better.
        callbacks.on_data = [this](const std::string_view& data) {
            if (m_currentRequest && m_currentRequest->on_data) {
                m_currentRequest->on_data(data.data(), data.length());
            }
        };

        callbacks.on_complete = [this]() {
            // NOTE: The Gsm::httpGet doesn't provide a status code on completion.
            // We assume OK for this placeholder. This should be improved in the Gsm module.
            this->_completeCurrentRequest(NetworkStatus::OK);
        };
        
        // Gsm::httpGet doesn't support POST or headers. This is a major limitation to address.
        if (request->method == HttpMethod::POST) {
            ESP_LOGE(NETWORK_TAG, "Gsm::httpGet does not support POST. Failing request.");
            _completeCurrentRequest(NetworkStatus::MODULE_ERROR);
            return;
        }

        Gsm::httpGet(request->url, callbacks);
    }

} // namespace Network