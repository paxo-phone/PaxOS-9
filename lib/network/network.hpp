#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

// Forward declare to avoid including heavy headers here
struct esp_http_client;

namespace Network
{
    // --- Public Enums and Typedefs ---

    enum class HttpMethod
    {
        GET,
        POST
    };

    enum class RoutingPolicy
    {
        WIFI_PREFERRED, // Use Wi-Fi if available, otherwise fall back to Cellular
        WIFI_ONLY,      // Only use Wi-Fi
        CELLULAR_ONLY,  // Only use 4G/Cellular
        NONE            // Disable all network requests
    };

    enum class NetworkStatus
    {
        OK,
        TIMEOUT,
        NO_ROUTE, // No available network interface based on policy
        CONNECTION_FAILED,
        DNS_ERROR,
        REQUEST_FAILED, // General failure during request
        BAD_RESPONSE,   // Server returned an error code (4xx, 5xx)
        MODULE_ERROR    // Error within the underlying Wi-Fi or GSM module
    };

    enum class NetworkInterface
    {
        NONE,
        WIFI,
        CELLULAR
    };

    // --- Callback Definitions ---
    // Callback for when the initial response (headers and status code) is received.
    using OnResponseCallback = std::function<void(int http_code)>;
    // Callback for receiving a chunk of the response body.
    using OnDataCallback = std::function<void(const char* data, int len)>;
    // Callback for when the request is fully completed (or has failed).
    using OnCompleteCallback = std::function<void(NetworkStatus status)>;

    // --- Request Structure ---
    struct Request
    {
        uint32_t id;
        std::string url;
        HttpMethod method = HttpMethod::GET;
        std::map<std::string, std::string> headers;
        std::string post_body;

        // Internal handle used by the network manager (e.g., for cURL handle)
        void* internal_handle = nullptr;

        OnResponseCallback on_response;
        OnDataCallback on_data;
        OnCompleteCallback on_complete;
    };

    // --- Public API ---

    /**
     * @brief Initializes the Network module and starts its background processing thread.
     * Must be called once at startup.
     */
    void init();

    /**
     * @brief Submits a new network request to the queue.
     * This function is thread-safe and can be called from anywhere.
     *
     * @param request A shared pointer to a Request object.
     */
    void submitRequest(std::shared_ptr<Request> request);

    // --- Wi-Fi Management ---

    /**
     * @brief Enables the Wi-Fi module.
     */
    void enableWifi();

    /**
     * @brief Disables the Wi-Fi module.
     */
    void disableWifi();

    /**
     * @brief Connects to a Wi-Fi access point.
     *
     * @param ssid The SSID of the network.
     * @param password The password for the network.
     * @return True if the connection process was initiated.
     */
    bool connectWifi(const std::string& ssid, const std::string& password);

    /**
     * @brief Disconnects from the current Wi-Fi network.
     */
    void disconnectWifi();

    /**
     * @brief Checks if the device is currently connected to a Wi-Fi network.
     * @return True if connected, false otherwise.
     */
    bool isWifiConnected();

    // --- Policy Management ---

    /**
     * @brief Sets the routing policy for network requests.
     *
     * @param policy The desired routing policy.
     */
    void setRoutingPolicy(RoutingPolicy policy);

    /**
     * @brief Gets the current routing policy.
     * @return The current RoutingPolicy.
     */
    RoutingPolicy getRoutingPolicy();

    /**
     * @brief Determines the best available network interface based on the current policy and
     * connection states.
     * @return The determined NetworkInterface.
     */
    NetworkInterface getBestAvailableInterface();

    // --- Internal processing thread ---
    // This function is public to be accessible by ThreadManager, but should not be called directly.
    void network_thread(void* arg);

} // namespace Network

#endif // NETWORK_HPP
