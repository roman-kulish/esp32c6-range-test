#ifndef WIFI4_H
#define WIFI4_H

#include "protocol.h"
#include <WiFi.h>
#include <AsyncUDP.h>

class WiFiProtocol : public Protocol
{
public:
    // WiFi protocol modes
    enum WiFiMode
    {
        WIFI_PROTO_802_11N = 0,  // WiFi 4 (802.11n)
        WIFI_PROTO_802_11AX = 1, // WiFi 6 (802.11ax)
        WIFI_PROTO_LR = 2        // WiFi Long Range
    };

    WiFiProtocol(WiFiMode mode, uint8_t channel, int8_t txPower, bool isAccessPoint);
    virtual ~WiFiProtocol();

    // Initialize the WiFi 4 protocol
    virtual bool begin() override;

    // Send a test packet via WiFi
    virtual bool sendPacket(const TestPacket &packet) override;

    // Perform clock synchronization via WiFi
    virtual int64_t performClockSync() override;

    // Set callback for packet reception
    virtual bool setPacketCallback(PacketReceivedCallback callback) override;

    // Set callback for clock sync packets
    virtual bool setClockSyncCallback(ClockSyncCallback callback) override;

    // Get protocol type
    virtual ProtocolType getType() const override;

private:
    // WiFi protocol mode
    WiFiMode wifiMode;

    // Packet callback function pointer
    static PacketReceivedCallback packetCallback;

    // Clock sync callback function pointer
    static ClockSyncCallback clockSyncCallback;

    // UDP socket for data transmission
    AsyncUDP udp;

    // Flag to indicate if this device is an Access Point
    const bool isAP;

    // IP address of the peer
    IPAddress peerIP;

    // Initialize as Access Point
    bool initAsAP();

    // Initialize as Station
    bool initAsStation();

    // Configure WiFi protocol based on selected mode
    bool configureWiFiProtocol();

    // Handle incoming UDP packet
    void handleUDPPacket(AsyncUDPPacket packet);
};

#endif // WIFI4_H