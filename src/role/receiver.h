#ifndef RECEIVER_H
#define RECEIVER_H

#include "role.h"

class ReceiverRole : public Role
{
public:
    ReceiverRole(Protocol *protocol, GPSHandler *gpsHandler);
    virtual ~ReceiverRole();

    // Initialize the receiver role
    virtual bool begin() override;

    // Execute receiver operations in main loop
    virtual void loop() override;

private:
    // Last received sequence number
    uint32_t lastSequenceNumber;

    // Rolling window packet loss statistics
    uint32_t packetCounter;
    uint32_t lostPackets;
    unsigned long statisticsTimer;

    // Packet reception callback
    static void onPacketReceived(const Protocol::TestPacket &packet, int8_t rssi);

    // Pointer to the receiver instance (for static callbacks)
    static ReceiverRole *instance;

    // Process received packet
    void processPacket(const Protocol::TestPacket &packet, int8_t rssi);

    // Calculate packet loss statistics
    void calculatePacketLoss(uint32_t sequenceNumber);

    // Log packet data to file
    void logPacketData(const LogEntry &entry);
};

#endif // RECEIVER_H