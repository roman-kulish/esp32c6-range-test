#include "espnow.h"
#include "esp_wifi.h"

const uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Define and initialize the static instance pointer
ESPNOWProtocol *ESPNOWProtocol::instance = nullptr;

ESPNOWProtocol::ESPNOWProtocol(uint8_t channel, int8_t txPower) // Initializer list order matches declaration order in espnow.h
    : Protocol(channel, txPower), peerRegistered(false), espnowInitialized(false)
{
    // Get local MAC address
    WiFi.macAddress(macAddress);
}

ESPNOWProtocol::~ESPNOWProtocol()
{
    // Clean up ESP-NOW
    if (espnowInitialized)
    {
        esp_now_deinit();
    }
}

bool ESPNOWProtocol::begin()
{
    Serial.println("Initializing ESP-NOW protocol...");

    // Store the instance pointer for static callbacks
    instance = this;

    // Set device as WiFi Station
    WiFi.mode(WIFI_STA);

    // Set region code to Australia
    if (esp_wifi_set_country_code("AU", true) != ESP_OK)
    {
        Serial.println("Failed to set country code");
        return false;
    }
    Serial.println("Country code set to AU");

    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Error initializing ESP-NOW");
        return false;
    }

#if defined(WIFI_LR)
    // WiFi Long Range
    Serial.println("Setting protocol to LR");
    if (esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR) != ESP_OK)
    {
        Serial.println("Failed to set protocol");
        return false;
    }
#endif

    // Register callbacks
    esp_now_register_send_cb(ESPNOWProtocol::onDataSent);
    esp_now_register_recv_cb(ESPNOWProtocol::onDataReceived);

    // Set transmit power
    if (esp_wifi_set_max_tx_power(txPower) != ESP_OK)
    {
        Serial.println("Failed to set TX power");
        return false;
    }
    Serial.print("Max TX power set to: ");
    Serial.println(txPower);

    espnowInitialized = true;
    initialized = true;

    registerPeer(broadcastAddress);

    // Print local MAC address
    Serial.print("Local MAC Address: ");
    for (int i = 0; i < 6; i++)
    {
        Serial.printf("%02X", macAddress[i]);
        if (i < 5)
            Serial.print(":");
    }
    Serial.println();

    Serial.println("ESP-NOW initialized successfully");
    return true;
}

bool ESPNOWProtocol::registerPeer(const uint8_t *peerMac)
{
    if (!espnowInitialized)
    {
        return false;
    }

    // Copy peer MAC address
    memcpy(this->peerMac, peerMac, 6);

    // Register peer
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, peerMac, 6);
    peerInfo.encrypt = false; // No encryption for this test

    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("Failed to add peer");
        return false;
    }

    peerRegistered = true;
    Serial.println("Peer registered successfully");
    return true;
}

bool ESPNOWProtocol::sendPacket(const TestPacket &packet)
{
    if (!espnowInitialized || !peerRegistered)
    {
        return false;
    }

    // Send packet via ESP-NOW
    esp_err_t result = esp_now_send(peerMac, (uint8_t *)&packet, sizeof(TestPacket));

    return (result == ESP_OK);
}

bool ESPNOWProtocol::setPacketCallback(PacketReceivedCallback callback)
{
    packetCallback = callback;
    return true;
}

Protocol::ProtocolType ESPNOWProtocol::getType() const
{
    return Protocol::ProtocolType::PROTO_ESPNOW;
}

const char *ESPNOWProtocol::getProtocolName() const
{
#if defined(WIFI_LR)
    return "ESP-NOW (WiFi Long Range)";
#else
    return "ESP-NOW";
#endif
}

const uint8_t *ESPNOWProtocol::getMacAddress() const
{
    return macAddress;
}

// Static member function implementation for data sent callback
void ESPNOWProtocol::onDataSent(const uint8_t *macAddr, esp_now_send_status_t status)
{
    // Handle send callback (for debugging)
    if (status != ESP_NOW_SEND_SUCCESS)
    {
        Serial.println("ESP-NOW send failed");
    }
    // Can access instance via instance_ if needed, e.g., instance_->someMethod();
}

// Static member function implementation for data received callback
void ESPNOWProtocol::onDataReceived(const esp_now_recv_info_t *info, const uint8_t *data, int dataLen)
{
    // Check if the instance pointer is valid before accessing members
    if (!instance)
    {
        return;
    }

    // Check if packet is a test packet or a sync packet
    if (dataLen == sizeof(Protocol::TestPacket))
    {
        // It's a test packet
        const Protocol::TestPacket *packet = reinterpret_cast<const Protocol::TestPacket *>(data);

        // Get RSSI from the recv_info struct
        int8_t rssi = (info && info->rx_ctrl) ? info->rx_ctrl->rssi : -127; // Default to low value if info is null

        // Call the packet callback if registered
        if (instance->packetCallback)
        {
            instance->packetCallback(*packet, rssi);
        }
    }
}