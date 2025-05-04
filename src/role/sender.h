#ifndef SENDER_H
#define SENDER_H

#include "role.h"

class SenderRole : public Role
{
public:
    SenderRole(Protocol *protocol, GPSHandler *gpsHandler);
    virtual ~SenderRole();

    // Initialize the sender role
    virtual bool begin() override;

    // Execute sender operations in main loop
    virtual void loop() override;

private:
    // Sequence number for packets
    uint32_t sequenceNumber;

    // Timestamp of last packet sent
    unsigned long lastPacketTime;

    // Prepare test packet
    void prepareTestPacket(Protocol::TestPacket &packet);
};

#endif // SENDER_H