#ifndef ROLE_BASE_H
#define ROLE_BASE_H

#include <Arduino.h>
#include "config.h"
#include "../gps_handler.h"
#include "../protocol/protocol.h"
#include "../clock_sync.h"

class Role
{
public:
    // Structure for logging data on the receiver
    struct LogEntry
    {
        uint32_t sequenceNumber;
        int64_t senderTimestamp;
        int64_t receiverTimestamp;
        int64_t rawLatency;
        int64_t correctedLatency;
        int8_t rssi;
        int8_t configuredTxPower;
        GPSData gpsData;
    };

    Role(Protocol *protocol, GPSHandler *gpsHandler);
    virtual ~Role();

    // Initialize the role
    virtual bool begin() = 0;

    // Execute role operations in main loop
    virtual void loop() = 0;

protected:
    Protocol *protocol;
    GPSHandler *gpsHandler;
    ClockSync clockSync;

    // Flag to indicate if the role is initialized
    bool initialized;
};

#endif // ROLE_BASE_H