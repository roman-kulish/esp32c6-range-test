#ifndef ESPNOW_H
#define ESPNOW_H

#include "protocol.h"
#include <esp_now.h>
#include <WiFi.h>

class ESPNOWProtocol : public Protocol
{
public:
    ESPNOWProtocol(uint8_t channel, int8_t txPower);
    virtual ~ESPNOWProtocol();

    // Initialize the ESP-NOW protocol
    virtual bool begin() override;

    // Send a test packet via ESP-NOW
    virtual bool sendPacket(const TestPacket &packet) override;

    // Perform clock synchronization via ESP-NOW
    virtual int64_t performClockSync() override;

    // Set callback for packet reception
    virtual bool setPacketCallback(PacketReceivedCallback callback) override;

    // Set callback for clock sync packets
    virtual bool setClockSyncCallback(ClockSyncCallback callback) override;

    // Get protocol type
    virtual ProtocolType getType() const override;

    // Get protocol name as string
    virtual const char *getProtocolName() const override;

    // Register peer MAC address
    bool registerPeer(const uint8_t *peerMac);

    // Get local MAC address
    const uint8_t *getMacAddress() const;

private:
    // Static instance pointer (assumes only one instance)
    static ESPNOWProtocol *instance;

    // ESP-NOW callback handlers (static members)
    static void onDataSent(const uint8_t *macAddr, esp_now_send_status_t status);
    static void onDataReceived(const esp_now_recv_info_t *info, const uint8_t *data, int dataLen);

    // Packet callback function pointer
    PacketReceivedCallback packetCallback = nullptr; // Instance member

    // Clock sync callback function pointer
    ClockSyncCallback clockSyncCallback = nullptr; // Instance member

    // Local MAC address
    uint8_t macAddress[6];

    // Peer MAC address
    uint8_t peerMac[6];
    bool peerRegistered;

    // ESP-NOW initialization status
    bool espnowInitialized;
};

#endif // ESPNOW_H