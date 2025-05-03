#include "wifi.h"
#include "esp_wifi.h"

// Static callback pointers
WiFiProtocol::PacketReceivedCallback WiFiProtocol::packetCallback = nullptr;
WiFiProtocol::ClockSyncCallback WiFiProtocol::clockSyncCallback = nullptr;

WiFiProtocol::WiFiProtocol(WiFiMode mode, uint8_t channel, int8_t txPower, bool isAccessPoint)
    : Protocol(channel, txPower), wifiMode(mode), isAP(isAccessPoint)
{
    // Set peer IP - will be updated during initialization
    peerIP = IPAddress(0, 0, 0, 0);
}

WiFiProtocol::~WiFiProtocol()
{
    // Clean up WiFi
    WiFi.disconnect();
}

bool WiFiProtocol::begin()
{
    Serial.println("Initializing WiFi ...");

    // Set WiFi mode first (AP or STA)
    WiFi.mode(isAP ? WIFI_AP : WIFI_STA);
    Serial.println(isAP ? "Configuring as Access Point..." : "Configuring as Station...");

    // Set region code to Australia
    if (esp_wifi_set_country_code("AU", true) != ESP_OK)
    {
        Serial.println("Failed to set country code");
        return false;
    }
    Serial.println("Country code set to AU");

    // Configure the specific WiFi protocol (before starting AP/STA)
    if (!configureWiFiProtocol())
    {
        Serial.println("Failed to configure WiFi protocol");
        return false;
    }
    Serial.println("WiFi protocol configured");

    // Initialize WiFi based on role (AP or Station)
    bool success = isAP ? initAsAP() : initAsStation();
    if (!success)
    {
        return false; // initAsAP/initAsStation will print specific errors
    }

    // Set transmit power
    if (esp_wifi_set_max_tx_power(txPower) != ESP_OK)
    {
        Serial.println("Failed to set TX power");
        return false;
    }
    Serial.print("Max TX power set to: ");
    Serial.println(txPower);

    // Begin listening for UDP packets
    if (udp.listen(isAP ? DATA_PORT : SYNC_PORT))
    {
        Serial.print("UDP listening on port ");
        Serial.println(isAP ? DATA_PORT : SYNC_PORT);

        // Set up callback for incoming packets
        udp.onPacket([this](AsyncUDPPacket packet)
                     { handleUDPPacket(packet); });
    }
    else
    {
        Serial.println("Failed to start UDP listener");
        return false;
    }

    initialized = true;

    // Print connection details
    if (isAP)
    {
        Serial.print("AP IP address: ");
        Serial.println(WiFi.softAPIP());
    }
    else
    {
        Serial.print("Station IP address: ");
        Serial.println(WiFi.localIP());

        // Set peer IP to the AP's IP
        peerIP.fromString(AP_IP);
    }

    Serial.println("WiFi protocol initialized successfully");
    return true;
}

bool WiFiProtocol::initAsAP()
{
    IPAddress apIP;
    IPAddress gateway;
    IPAddress subnet;

    apIP.fromString(AP_IP);
    gateway.fromString(AP_IP);
    subnet.fromString(NETMASK);

    // Configure soft AP
    WiFi.softAPConfig(apIP, gateway, subnet);

    // Start soft AP
    if (!WiFi.softAP(WIFI_SSID, WIFI_PASSWORD, channel))
    {
        Serial.println("Failed to start soft AP");
        return false;
    }

    Serial.println("WiFi AP started");
    return true;
}

bool WiFiProtocol::initAsStation()
{
    IPAddress staticIP;
    IPAddress gateway;
    IPAddress subnet;

    staticIP.fromString(STA_IP);
    gateway.fromString(AP_IP);
    subnet.fromString(NETMASK);

    // Configure static IP
    WiFi.config(staticIP, gateway, subnet);

    // Connect to AP
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD, channel);

    // Wait for connection
    int timeout = 20; // 10 seconds timeout
    while (WiFi.status() != WL_CONNECTED && timeout > 0)
    {
        delay(500);
        Serial.print(".");
        timeout--;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("\nFailed to connect to WiFi AP");
        return false;
    }
    Serial.println("\nConnected to WiFi AP");

    if (!WiFi.setSleep(false)) {
        Serial.println("Failed to disable WiFi sleep");
        return false;
    }

    return true;
}

bool WiFiProtocol::configureWiFiProtocol()
{
    // Determine the correct interface (AP or STA)
    wifi_interface_t interface = isAP ? WIFI_IF_AP : WIFI_IF_STA;

    // Configure WiFi protocol based on selected mode
    switch (wifiMode)
    {
    case WIFI_PROTO_802_11N:
        // WiFi 4 (802.11n)
        Serial.println("Setting protocol to 802.11 B/G/N");
        return esp_wifi_set_protocol(interface, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N) == ESP_OK;

    case WIFI_PROTO_802_11AX:
        // WiFi 6 (802.11ax)
        Serial.println("Setting protocol to 802.11 B/G/N/AX");
        return esp_wifi_set_protocol(interface, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_11AX) == ESP_OK;

    case WIFI_PROTO_LR:
        // WiFi Long Range
        Serial.println("Setting protocol to LR");
        return esp_wifi_set_protocol(interface, WIFI_PROTOCOL_LR) == ESP_OK;

    default:
        Serial.println("Unknown WiFi protocol mode");
        return false;
    }
}

bool WiFiProtocol::sendPacket(const TestPacket &packet)
{
    if (!initialized || peerIP == IPAddress(0, 0, 0, 0))
    {
        return false;
    }

    // Send packet via UDP
    return udp.writeTo((uint8_t *)&packet, sizeof(TestPacket), peerIP, DATA_PORT) > 0;
}

int64_t WiFiProtocol::performClockSync()
{
    if (!initialized || peerIP == IPAddress(0, 0, 0, 0))
    {
        return 0;
    }

    // This is a simplified implementation of the clock sync process
    // For a full implementation, we would need to send multiple ping/ack exchanges

    const int syncAttempts = SYNC_PING_COUNT;
    int64_t totalOffset = 0;
    int successfulPings = 0;

    for (int i = 0; i < syncAttempts; i++)
    {
        // Create a sync packet
        int64_t t1 = esp_timer_get_time(); // Send timestamp

        // Structure for sync packet
        struct SyncPacket
        {
            int64_t t1;
        } syncPacket;

        syncPacket.t1 = t1;

        // Send sync packet
        if (udp.writeTo((uint8_t *)&syncPacket, sizeof(SyncPacket), peerIP, SYNC_PORT) > 0)
        {
            // Wait for acknowledgment (this is simplified)
            delay(100);
            successfulPings++;
        }

        // In a real implementation, we would wait for the ack packet and calculate the offset
        // For now, we'll just assume the offset is 0
    }

    if (successfulPings > 0)
    {
        // Return the average offset
        return totalOffset / successfulPings;
    }

    return 0; // No successful pings
}

bool WiFiProtocol::setPacketCallback(PacketReceivedCallback callback)
{
    packetCallback = callback;
    return true;
}

bool WiFiProtocol::setClockSyncCallback(ClockSyncCallback callback)
{
    clockSyncCallback = callback;
    return true;
}

Protocol::ProtocolType WiFiProtocol::getType() const
{
    return ProtocolType::PROTO_WIFI4;
}

void WiFiProtocol::handleUDPPacket(AsyncUDPPacket packet)
{
    // Get source IP
    IPAddress sourceIP = packet.remoteIP();

    // Update peer IP if not set
    if (peerIP == IPAddress(0, 0, 0, 0))
    {
        peerIP = sourceIP;
        Serial.print("Peer IP set to: ");
        Serial.println(peerIP.toString());
    }

    // Check packet type based on size
    if (packet.length() == sizeof(TestPacket))
    {
        // It's a test packet
        const TestPacket *testPacket = reinterpret_cast<const TestPacket *>(packet.data());

        // Get RSSI
        int8_t rssi = WiFi.RSSI();

        // Call the packet callback if registered
        if (packetCallback)
        {
            packetCallback(*testPacket, rssi);
        }
    }
    else if (packet.length() == sizeof(int64_t))
    {
        // It's a sync packet
        int64_t senderTimestamp = *reinterpret_cast<const int64_t *>(packet.data());
        int64_t receiverTimestamp = esp_timer_get_time();

        // Call the clock sync callback if registered
        if (clockSyncCallback)
        {
            clockSyncCallback(senderTimestamp, receiverTimestamp);
        }

        // Send ack packet back (in a full implementation)
    }
}