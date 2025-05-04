#ifndef ROLE_BASE_H
#define ROLE_BASE_H

#include <Arduino.h>
#include "config.h"
#include "../gps_handler.h"
#include "../protocol/protocol.h"

class Role
{
public:
    // Structure for logging data on the receiver
    struct LogEntry
    {
        const char *protocolName;
        uint32_t sequenceNumber;
        int64_t senderTimestamp_us;
        int64_t receiverTimestamp_us;
        int64_t latency_us;
        int8_t rssi_dBm;
        int8_t configuredTxPower_dBm;
        uint8_t configuredChannel;
        int64_t receiverGPS_timestamp_us;
        double receiverGPS_latitude;
        double receiverGPS_longitude;
        double receiverGPS_altitude;
        uint8_t receiverGPS_satellites;
        double receiverGPS_hdop;
        double senderGPS_latitude;
        double senderGPS_longitude;
        double senderGPS_altitude;
        uint8_t senderGPS_satellites;
        double senderGPS_hdop;
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

    // Flag to indicate if the role is initialized
    bool initialized;
};

#endif // ROLE_BASE_H