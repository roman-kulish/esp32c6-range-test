#include "espnow.h"
#include "esp_wifi.h"

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

    // Set channel
    WiFi.channel(channel);

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
    peerInfo.channel = channel;
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

int64_t ESPNOWProtocol::performClockSync()
{
    if (!espnowInitialized || !peerRegistered)
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
        esp_err_t result = esp_now_send(peerMac, (uint8_t *)&syncPacket, sizeof(SyncPacket));

        if (result == ESP_OK)
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

bool ESPNOWProtocol::setPacketCallback(PacketReceivedCallback callback)
{
    packetCallback = callback;
    return true;
}

bool ESPNOWProtocol::setClockSyncCallback(ClockSyncCallback callback)
{
    clockSyncCallback = callback;
    return true;
}

Protocol::ProtocolType ESPNOWProtocol::getType() const
{
    return Protocol::ProtocolType::PROTO_ESPNOW;
}

// Corrected class name typo: ESPNOWProtocol instead of ESPNowProtocol
const char *ESPNOWProtocol::getProtocolName() const
{
    return "ESP-NOW";
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
    else if (dataLen == sizeof(int64_t))
    {
        // It's a sync packet
        int64_t senderTimestamp = *reinterpret_cast<const int64_t *>(data);
        int64_t receiverTimestamp = esp_timer_get_time(); // Receiver timestamp

        // Call the clock sync callback if registered (using the instance pointer)
        if (instance->clockSyncCallback)
        {
            instance->clockSyncCallback(senderTimestamp, receiverTimestamp);
        }

        // Send ack packet back
        // (This would be implemented in a full solution)
    }
}