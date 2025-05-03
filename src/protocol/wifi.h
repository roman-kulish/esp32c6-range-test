#ifndef WIFI_PROTOCOL_H
#define WIFI_PROTOCOL_H

#include "protocol.h"
#include <WiFi.h>
#include <AsyncUDP.h>

class WiFiProtocol : public Protocol
{
public:
    WiFiProtocol(ProtocolType proto, uint8_t channel, int8_t txPower, bool isAccessPoint);
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

    // Get protocol name as string
    virtual const char *getProtocolName() const override;

private:
    // WiFi protocol mode
    ProtocolType proto;

    // Packet callback function pointer
    PacketReceivedCallback packetCallback = nullptr; // Instance member

    // Clock sync callback function pointer
    ClockSyncCallback clockSyncCallback = nullptr; // Instance member

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

#endif // WIFI_PROTOCOL_H