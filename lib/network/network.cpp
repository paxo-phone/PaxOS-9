#include "network.hpp"

#include "delay.hpp"
#include "gsm2.hpp"
#include "threads.hpp"

#include <iostream>

#ifdef ESP_PLATFORM
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "esp_wifi.h"
#include "nvs_flash.h" // Added for NVS initialization

#include <esp_tls.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

extern "C"
{
#include "esp_crt_bundle.h"
}

#else // !ESP_PLATFORM
#include <curl/curl.h>
#endif

#include <deque>

#define NETWORK_TAG "Network"

namespace Network
{
    // --- Internal State and Singleton ---

    class NetworkManager
    {
      public:
        // Singleton access
        static NetworkManager& getInstance()
        {
            static NetworkManager instance;
            return instance;
        }

        void init();
        void submitRequest(std::shared_ptr<Request> request);

        void setRoutingPolicy(RoutingPolicy policy)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_policy = policy;
        }

        RoutingPolicy getRoutingPolicy()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_policy;
        }

        // Wi-Fi Management
        void enableWifi();
        void disableWifi();
        bool connectWifi(const std::string& ssid, const std::string& password);
        void disconnectWifi();

        bool isWifiConnected()
        {
            return m_isWifiConnected;
        }

        NetworkInterface getBestAvailableInterface();

        // Main processing loop, called by the thread
        void processQueue();

      private:
        NetworkManager() = default;

        ~NetworkManager()
        {
#ifndef ESP_PLATFORM
            curl_global_cleanup();
#endif
        }

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
        static void _wifiEventHandler(
            void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data
        );
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
#else
        CURL* m_curlEasyHandle = nullptr;
#endif
    };

    // --- Singleton Public API Implementation ---

    void init()
    {
        NetworkManager::getInstance().init();
    }

    void submitRequest(std::shared_ptr<Request> request)
    {
        NetworkManager::getInstance().submitRequest(request);
    }

    void enableWifi()
    {
        NetworkManager::getInstance().enableWifi();
    }

    void disableWifi()
    {
        NetworkManager::getInstance().disableWifi();
    }

    bool connectWifi(const std::string& ssid, const std::string& password)
    {
        return NetworkManager::getInstance().connectWifi(ssid, password);
    }

    void disconnectWifi()
    {
        NetworkManager::getInstance().disconnectWifi();
    }

    void setRoutingPolicy(RoutingPolicy policy)
    {
        NetworkManager::getInstance().setRoutingPolicy(policy);
    }

    RoutingPolicy getRoutingPolicy()
    {
        return NetworkManager::getInstance().getRoutingPolicy();
    }

    NetworkInterface getBestAvailableInterface()
    {
        return NetworkManager::getInstance().getBestAvailableInterface();
    }

    bool isWifiConnected()
    {
        return NetworkManager::getInstance().isWifiConnected();
    }

    // --- Thread Function ---
    void network_thread(void* arg)
    {
        std::cout << "[" << NETWORK_TAG << "] Network processing thread started." << std::endl;
        while (true)
        {
            NetworkManager::getInstance().processQueue();
            PaxOS_Delay(20); // Yield to other tasks
        }
    }

    // --- NetworkManager Method Implementations ---

#ifdef ESP_PLATFORM
    // SNTP initialization for time sync
    void initialize_sntp(void)
    {
        std::cout << "Initializing SNTP" << std::endl;
        esp_sntp_setoperatingmode((esp_sntp_operatingmode_t) SNTP_OPMODE_POLL);
        esp_sntp_setservername(0, "pool.ntp.org");
        esp_sntp_init();

        int retry = 0;
        const int retry_count = 10;
        while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count)
        {
            std::cout << "Waiting for system time to be set... (" << retry << "/" << retry_count
                      << ")" << std::endl;
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }

        if (retry == retry_count)
        {
            std::cout << "Failed to update system time" << std::endl;
        }
        else
        {
            time_t now;
            struct tm timeinfo;
            time(&now);
            localtime_r(&now, &timeinfo);
            char buf[64];
            strftime(buf, sizeof(buf), "%c", &timeinfo);
            std::cout << "The current date/time is: " << buf << std::endl;
        }
    }
#endif

    void NetworkManager::init()
    {
#ifdef ESP_PLATFORM
        ESP_LOGI(NETWORK_TAG, "Initializing Network Manager...");
        // Initialize NVS
        esp_err_t nvs_ret = nvs_flash_init();
        if (nvs_ret == ESP_ERR_NVS_NO_FREE_PAGES || nvs_ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ESP_ERROR_CHECK(nvs_flash_init());
        }
        else
        {
            ESP_ERROR_CHECK(nvs_ret);
        }
        // Initialize TCP/IP adapter
        esp_netif_init();
        // Initialize event loop
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        ESP_ERROR_CHECK(esp_tls_init_global_ca_store());
        // Register Wi-Fi event handlers
        ESP_ERROR_CHECK(
            esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &_wifiEventHandler, this)
        );
        ESP_ERROR_CHECK(
            esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &_wifiEventHandler, this)
        );
#else
        std::cout << "[" << NETWORK_TAG << "] Initializing Network Manager for PC..." << std::endl;
        curl_global_init(CURL_GLOBAL_DEFAULT);
        // On PC, we can assume "Wi-Fi" is always "connected" if we have a network interface.
        m_isWifiEnabled = true;
        m_isWifiConnected = true;
#endif
    }

    void NetworkManager::submitRequest(std::shared_ptr<Request> request)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        request->id = m_nextRequestId++;
        m_requestQueue.push_back(request);
        std::cout << "[" << NETWORK_TAG << "] Queued request ID " << request->id
                  << " for URL: " << request->url << std::endl;
    }

    NetworkInterface NetworkManager::getBestAvailableInterface()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::cout << "[" << NETWORK_TAG
                  << "] Determining best available interface based on current policy and "
                     "connection states."
                  << std::endl;
        switch (m_policy)
        {
        case RoutingPolicy::WIFI_PREFERRED:
            std::cout << "[" << NETWORK_TAG << "] Current policy: WIFI_PREFERRED." << std::endl;
            if (m_isWifiConnected)
            {
                std::cout << "[" << NETWORK_TAG
                          << "] Wi-Fi is connected. Selecting Wi-Fi interface." << std::endl;
                return NetworkInterface::WIFI;
            }
            if (Gsm::isConnected())
            {
                std::cout << "[" << NETWORK_TAG
                          << "] Cellular is connected. Selecting Cellular interface." << std::endl;
                return NetworkInterface::CELLULAR;
            }
            break;
        case RoutingPolicy::WIFI_ONLY:
            std::cout << "[" << NETWORK_TAG << "] Current policy: WIFI_ONLY." << std::endl;
            if (m_isWifiConnected)
            {
                std::cout << "[" << NETWORK_TAG
                          << "] Wi-Fi is connected. Selecting Wi-Fi interface." << std::endl;
                return NetworkInterface::WIFI;
            }
            break;
        case RoutingPolicy::CELLULAR_ONLY:
            std::cout << "[" << NETWORK_TAG << "] Current policy: CELLULAR_ONLY." << std::endl;
            if (Gsm::isConnected())
            {
                std::cout << "[" << NETWORK_TAG
                          << "] Cellular is connected. Selecting Cellular interface." << std::endl;
                return NetworkInterface::CELLULAR;
            }
            break;
        case RoutingPolicy::NONE:
            std::cout << "[" << NETWORK_TAG
                      << "] Current policy: NONE. No network requests allowed." << std::endl;
            return NetworkInterface::NONE;
        }
        std::cout << "[" << NETWORK_TAG
                  << "] No available interfaces based on current policy and connection states."
                  << std::endl;
        return NetworkInterface::NONE;
    }

    void NetworkManager::processQueue()
    {
        // This function is the heart of the processing thread.
        // It's designed to be called in a tight loop.

        // 1. Check if we are busy. If so, do nothing.
        if (m_currentRequest != nullptr)
            return;

        // 2. Try to get a new request from the queue
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_requestQueue.empty())
                return; // Nothing to do
            m_currentRequest = m_requestQueue.front();
            m_requestQueue.pop_front();
        }

        // 3. We have a request, let's process it
        if (m_currentRequest)
        {
            std::cout << "[" << NETWORK_TAG << "] Processing request ID " << m_currentRequest->id
                      << "..." << std::endl;
            _executeRequest(m_currentRequest);
        }
    }

    void NetworkManager::_executeRequest(std::shared_ptr<Request> request)
    {
        NetworkInterface bestInterface = getBestAvailableInterface();

        switch (bestInterface)
        {
        case NetworkInterface::WIFI:
            std::cout << "[" << NETWORK_TAG << "] Routing request ID " << request->id
                      << " via Wi-Fi." << std::endl;
            _executeWifiRequest(request);
            break;
        case NetworkInterface::CELLULAR:
            std::cout << "[" << NETWORK_TAG << "] Routing request ID " << request->id
                      << " via Cellular." << std::endl;
            _executeCellularRequest(request);
            break;
        case NetworkInterface::NONE:
        default:
            std::cout << "[ERROR:" << NETWORK_TAG << "] No route available for request ID "
                      << request->id << "." << std::endl;
            _completeCurrentRequest(NetworkStatus::NO_ROUTE);
            break;
        }
    }

    void NetworkManager::_completeCurrentRequest(NetworkStatus status)
    {
        if (m_currentRequest)
        {
            std::cout << "[" << NETWORK_TAG << "] Completing request ID " << m_currentRequest->id
                      << " with status " << static_cast<int>(status) << "." << std::endl;
            if (m_currentRequest->on_complete)
                m_currentRequest->on_complete(status);
            // Clear current request to allow the next one to be processed
            m_currentRequest = nullptr;
        }
    }

    // --- Wi-Fi Specific Implementation ---
#ifdef ESP_PLATFORM
    void NetworkManager::enableWifi()
    {
        if (m_isWifiEnabled)
            return;
        std::cout << "[" << NETWORK_TAG << "] Enabling Wi-Fi." << std::endl;
        esp_netif_create_default_wifi_sta();
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_start());
        m_isWifiEnabled = true;
    }

    void NetworkManager::disableWifi()
    {
        if (!m_isWifiEnabled)
            return;
        std::cout << "[" << NETWORK_TAG << "] Disabling Wi-Fi." << std::endl;
        disconnectWifi();
        ESP_ERROR_CHECK(esp_wifi_stop());
        ESP_ERROR_CHECK(esp_wifi_deinit());
        m_isWifiEnabled = false;
    }

    bool NetworkManager::connectWifi(const std::string& ssid, const std::string& password)
    {
        if (!m_isWifiEnabled)
            enableWifi();

        std::cout << "[" << NETWORK_TAG << "] Connecting to Wi-Fi SSID: " << ssid << std::endl;
        wifi_config_t wifi_config = {};
        strncpy((char*) wifi_config.sta.ssid, ssid.c_str(), sizeof(wifi_config.sta.ssid));
        strncpy(
            (char*) wifi_config.sta.password,
            password.c_str(),
            sizeof(wifi_config.sta.password)
        );

        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        esp_err_t err = esp_wifi_connect();
        return err == ESP_OK;
    }

    void NetworkManager::disconnectWifi()
    {
        if (m_isWifiConnected)
        {
            std::cout << "[" << NETWORK_TAG << "] Disconnecting from Wi-Fi." << std::endl;
            ESP_ERROR_CHECK(esp_wifi_disconnect());
        }
    }

    void NetworkManager::_wifiEventHandler(
        void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data
    )
    {
        NetworkManager* self = static_cast<NetworkManager*>(arg);
        if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
        {
            std::cout << "[" << NETWORK_TAG << "] Wi-Fi station started." << std::endl;
        }
        else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
        {
            std::cout << "[" << NETWORK_TAG << "] Wi-Fi disconnected." << std::endl;
            self->m_isWifiConnected = false;
        }
        else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
        {
            ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
            std::cout << "[" << NETWORK_TAG
                      << "] Got IP:" << ip4addr_ntoa((const ip4_addr_t*) &event->ip_info.ip)
                      << std::endl;
            self->m_isWifiConnected = true;
            initialize_sntp(); // Sync time after Wi-Fi connects
        }
    }

    esp_err_t NetworkManager::_httpEventHandler(esp_http_client_event_t* evt)
    {
        NetworkManager* self = &NetworkManager::getInstance();
        if (!self->m_currentRequest)
            return ESP_OK; // Should not happen
        auto request = self->m_currentRequest;

        switch (evt->event_id)
        {
        case HTTP_EVENT_ERROR:
            std::cout << "[ERROR:" << NETWORK_TAG << "] HTTP_EVENT_ERROR" << std::endl;
            self->_completeCurrentRequest(NetworkStatus::CONNECTION_FAILED);
            break;
        case HTTP_EVENT_ON_CONNECTED:
            std::cout << "[" << NETWORK_TAG << "] HTTP_EVENT_ON_CONNECTED" << std::endl;
            break;
        case HTTP_EVENT_HEADER_SENT:
            std::cout << "[" << NETWORK_TAG << "] HTTP_EVENT_HEADER_SENT" << std::endl;
            break;
        case HTTP_EVENT_ON_HEADER:
            // This event is where we learn the status code.
            if (request->on_response)
                request->on_response(esp_http_client_get_status_code(evt->client));
            std::cout << "[" << NETWORK_TAG << "] HTTP_EVENT_ON_HEADER, key=" << evt->header_key
                      << ", value=" << evt->header_value << std::endl;
            break;
        case HTTP_EVENT_ON_DATA:
            std::cout << "[" << NETWORK_TAG << "] HTTP_EVENT_ON_DATA, len=" << evt->data_len
                      << std::endl;
            if (request->on_data)
                request->on_data((const char*) evt->data, evt->data_len);
            break;
        case HTTP_EVENT_ON_FINISH:
            std::cout << "[" << NETWORK_TAG << "] HTTP_EVENT_ON_FINISH" << std::endl;
            self->_completeCurrentRequest(NetworkStatus::OK);
            break;
        case HTTP_EVENT_DISCONNECTED:
            std::cout << "[" << NETWORK_TAG << "] HTTP_EVENT_DISCONNECTED" << std::endl;
            // This event can fire after a successful request. We only treat it as an error
            // if the request wasn't already completed by ON_FINISH.
            if (self->m_currentRequest != nullptr)
                self->_completeCurrentRequest(NetworkStatus::REQUEST_FAILED);
            break;
        }
        return ESP_OK;
    }

    void NetworkManager::_executeWifiRequest(std::shared_ptr<Request> request)
    {
        esp_http_client_config_t config = {};

        // Use the URL directly - don't do custom encoding as it may cause issues
        config.url = request->url.c_str();

        config.event_handler = _httpEventHandler;
        config.user_data = this; // Pass manager instance
        config.disable_auto_redirect = true;

        // FIX: Properly configure SSL/TLS certificate bundle
        config.use_global_ca_store = true;
        config.crt_bundle_attach = esp_crt_bundle_attach;

        // Additional SSL/TLS configuration for better compatibility
        config.skip_cert_common_name_check = false;
        config.timeout_ms = 30000; // 30 second timeout
        config.buffer_size = 4096;
        config.buffer_size_tx = 1024;

        // IMPORTANT: Enable SNI and configure TLS properly
        // config.use_secure_element = false;
        config.transport_type = HTTP_TRANSPORT_OVER_SSL;
        config.is_async = false;

        // Set user agent to avoid potential blocking
        config.user_agent = "ESP32-HTTPClient/1.0";

        printf("==================================================\n");
        printf("Memory before HTTP Perform:\n");
        printf(
            "Internal RAM Free: %d, Largest Block: %d\n",
            heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
            heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL)
        );
        printf(
            "PSRAM Free: %d, Largest Block: %d\n",
            heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
            heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM)
        );
        printf("==================================================\n");

        m_httpClient = esp_http_client_init(&config);

        // Set headers
        for (const auto& header : request->headers)
            esp_http_client_set_header(m_httpClient, header.first.c_str(), header.second.c_str());

        // Set method and post data
        if (request->method == HttpMethod::POST)
        {
            esp_http_client_set_method(m_httpClient, HTTP_METHOD_POST);
            esp_http_client_set_post_field(
                m_httpClient,
                request->post_body.c_str(),
                request->post_body.length()
            );
        }
        else
        {
            esp_http_client_set_method(m_httpClient, HTTP_METHOD_GET);
        }

        esp_err_t err = esp_http_client_perform(m_httpClient);

        if (err != ESP_OK)
        {
            std::cout << "[ERROR:" << NETWORK_TAG
                      << "] HTTP request failed: " << esp_err_to_name(err) << std::endl;
            _completeCurrentRequest(NetworkStatus::REQUEST_FAILED);
        }

        esp_http_client_cleanup(m_httpClient);
        m_httpClient = nullptr;
    }
#else // !ESP_PLATFORM
    // --- PC/Simulator (cURL) Implementation ---

    // cURL callback to write received data into a string or handle it
    static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp)
    {
        // userp points to the current Request object
        auto request = static_cast<Network::Request*>(userp);
        size_t realsize = size * nmemb;
        if (request && request->on_data)
            request->on_data(static_cast<const char*>(contents), realsize);
        return realsize;
    }

    // cURL callback to handle headers
    static size_t header_callback(char* buffer, size_t size, size_t nitems, void* userdata)
    {
        auto request = static_cast<Network::Request*>(userdata);
        size_t realsize = size * nitems;
        std::string header(buffer, realsize);

        // Try to parse HTTP status code from the first line (e.g., "HTTP/1.1 200 OK")
        if (header.rfind("HTTP/", 0) == 0)
        {
            long status_code = 0;
            curl_easy_getinfo(
                static_cast<CURL*>(request->internal_handle),
                CURLINFO_RESPONSE_CODE,
                &status_code
            );
            if (request && request->on_response)
                request->on_response(status_code);
        }
        return realsize;
    }

    void NetworkManager::enableWifi()
    {
        std::cout << "[" << NETWORK_TAG << "] Wi-Fi is conceptually enabled on PC." << std::endl;
        m_isWifiEnabled = true;
        m_isWifiConnected = true; // Assume connected
    }

    void NetworkManager::disableWifi()
    {
        std::cout << "[" << NETWORK_TAG << "] Wi-Fi is conceptually disabled on PC." << std::endl;
        m_isWifiEnabled = false;
        m_isWifiConnected = false;
    }

    bool NetworkManager::connectWifi(const std::string&, const std::string&)
    {
        std::cout << "[" << NETWORK_TAG << "] Connecting to Wi-Fi on PC (simulation)." << std::endl;
        m_isWifiConnected = true; // Assume connection is always successful
        return true;
    }

    void NetworkManager::disconnectWifi()
    {
        std::cout << "[" << NETWORK_TAG << "] Disconnecting from Wi-Fi on PC (simulation)."
                  << std::endl;
        m_isWifiConnected = false;
    }

    void NetworkManager::_executeWifiRequest(std::shared_ptr<Request> request)
    {
        m_curlEasyHandle = curl_easy_init();
        if (!m_curlEasyHandle)
        {
            std::cout << "[ERROR:" << NETWORK_TAG << "] curl_easy_init() failed." << std::endl;
            _completeCurrentRequest(NetworkStatus::MODULE_ERROR);
            return;
        }
        request->internal_handle = m_curlEasyHandle;

        // Set URL
        curl_easy_setopt(m_curlEasyHandle, CURLOPT_URL, request->url.c_str());

        // --- SSL Verification Setup ---
        // Enforce peer verification. This is the default, but it's good to be explicit.
        curl_easy_setopt(m_curlEasyHandle, CURLOPT_SSL_VERIFYPEER, 1L);
        // Provide the path to the certificate bundle.
        // This file must be present in the same directory as the executable.
        curl_easy_setopt(m_curlEasyHandle, CURLOPT_CAINFO, "cacert.pem");

        // Set headers
        struct curl_slist* headers = nullptr;
        for (const auto& header : request->headers)
        {
            std::string header_string = header.first + ": " + header.second;
            headers = curl_slist_append(headers, header_string.c_str());
        }
        if (headers)
            curl_easy_setopt(m_curlEasyHandle, CURLOPT_HTTPHEADER, headers);

        // Set method and post data
        if (request->method == HttpMethod::POST)
        {
            curl_easy_setopt(m_curlEasyHandle, CURLOPT_POSTFIELDS, request->post_body.c_str());
            curl_easy_setopt(m_curlEasyHandle, CURLOPT_POSTFIELDSIZE, request->post_body.length());
        }

        // Set callbacks
        curl_easy_setopt(m_curlEasyHandle, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(m_curlEasyHandle, CURLOPT_WRITEDATA, request.get());
        curl_easy_setopt(m_curlEasyHandle, CURLOPT_HEADERFUNCTION, header_callback);
        curl_easy_setopt(m_curlEasyHandle, CURLOPT_HEADERDATA, request.get());

        // Follow redirects
        curl_easy_setopt(m_curlEasyHandle, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(m_curlEasyHandle, CURLOPT_VERBOSE, 1L);

        // Perform the request
        CURLcode res = curl_easy_perform(m_curlEasyHandle);

        if (res != CURLE_OK)
        {
            std::cout << "[ERROR:" << NETWORK_TAG
                      << "] curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            _completeCurrentRequest(NetworkStatus::REQUEST_FAILED);
        }
        else
        {
            _completeCurrentRequest(NetworkStatus::OK);
        }

        // Cleanup
        if (headers)
            curl_slist_free_all(headers);
        curl_easy_cleanup(m_curlEasyHandle);
        m_curlEasyHandle = nullptr;
        request->internal_handle = nullptr;
    }
#endif

    // --- Cellular Specific Implementation ---

    void NetworkManager::_executeCellularRequest(std::shared_ptr<Request> request)
    {
        Gsm::HttpRequest gsmRequest;
        gsmRequest.url = request->url;
        gsmRequest.method =
            (request->method == HttpMethod::POST) ? Gsm::HttpMethod::POST : Gsm::HttpMethod::GET;
        gsmRequest.headers = request->headers;
        gsmRequest.body = request->post_body;

        gsmRequest.on_response = [this](int http_code)
        {
            if (m_currentRequest && m_currentRequest->on_response)
                m_currentRequest->on_response(http_code);
        };

        gsmRequest.on_data = [this](const std::string_view& data)
        {
            if (m_currentRequest && m_currentRequest->on_data)
                m_currentRequest->on_data(data.data(), data.length());
        };

        gsmRequest.on_complete = [this](Gsm::HttpResult result)
        {
            NetworkStatus status = NetworkStatus::OK;
            switch (result)
            {
            case Gsm::HttpResult::OK:
                status = NetworkStatus::OK;
                break;
            case Gsm::HttpResult::TIMEOUT:
                status = NetworkStatus::TIMEOUT;
                break;
            case Gsm::HttpResult::CONNECTION_FAILED:
                status = NetworkStatus::CONNECTION_FAILED;
                break;
            case Gsm::HttpResult::DNS_ERROR:
                status = NetworkStatus::DNS_ERROR;
                break;
            case Gsm::HttpResult::SERVER_ERROR:
            case Gsm::HttpResult::NOT_FOUND: // Treat 404 as a bad response
                status = NetworkStatus::BAD_RESPONSE;
                break;
            case Gsm::HttpResult::BUSY:
            case Gsm::HttpResult::INIT_FAILED:
            case Gsm::HttpResult::MODULE_ERROR:
            case Gsm::HttpResult::READ_ERROR:
            default:
                status = NetworkStatus::MODULE_ERROR;
                break;
            }
            this->_completeCurrentRequest(status);
        };

        Gsm::httpRequest(std::move(gsmRequest));
    }

} // namespace Network
