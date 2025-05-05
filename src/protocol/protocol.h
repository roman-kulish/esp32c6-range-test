#ifndef PROTOCOL_BASE_H
#define PROTOCOL_BASE_H

#include <Arduino.h>
#include "config.h"

class Protocol
{
public:
    enum ProtocolType

    {
        PROTO_WIFI4 = 1,
        PROTO_WIFI6 = 2,
        PROTO_WIFI_LR = 3,
        PROTO_ESPNOW = 4
    };

    // Data structure for test packets
    struct TestPacket
    {
        uint32_t sequenceNumber;    // Incrementing sequence number
        int64_t senderTimestamp_us; // High-resolution sender timestamp (microseconds)
        double latitude;
        double longitude;
        double altitude_mm;
        int satellites;
        uint32_t horizontalAccuracy_mm;
        uint8_t payload[PACKET_SIZE]; // Fixed payload of 75 bytes
    };

    using PacketReceivedCallback = void (*)(const TestPacket &packet, int8_t rssi);

    Protocol(uint8_t channel, int8_t txPower);
    virtual ~Protocol();

    // Initialize the protocol
    virtual bool begin() = 0;

    // For sender: send a test packet
    virtual bool sendPacket(const TestPacket &packet) = 0;

    // For receiver: set callback for packet reception
    virtual bool setPacketCallback(PacketReceivedCallback callback) = 0;

    // Check if the protocol has been successfully initialized
    bool isInitialized() const;

    // Get the configured transmit power
    int8_t getTransmitPower() const;

    // Get the configured channel
    uint8_t getChannel() const;

    // Get protocol type
    virtual ProtocolType getType() const = 0;

    // Get protocol name as string
    virtual const char *getProtocolName() const = 0;

protected:
    uint8_t channel;
    int8_t txPower;
    bool initialized;
};

#endif // PROTOCOL_BASE_H