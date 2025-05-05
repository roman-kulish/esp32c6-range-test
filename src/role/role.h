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
        uint32_t receiverGPS_horizontalAccuracy_mm;;
        double senderGPS_latitude;
        double senderGPS_longitude;
        double senderGPS_altitude;
        uint8_t senderGPS_satellites;
        uint32_t senderGPS_horizontalAccuracy_mm;
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

    // Interval for checking/syncing time with GPS (milliseconds)
    static const uint32_t SYNC_INTERVAL_MS = 30000; // Sync every 30 seconds

    // Threshold for large time offset (microseconds). Offsets larger than this trigger a hard settimeofday().
    static const int64_t LARGE_OFFSET_THRESHOLD_US = 1000000L; // 1 second

    // Timestamp of the last time sync attempt
    unsigned long lastSyncTimeMs;

    // Flag to indicate if the role is initialized
    bool initialized;

    // Attempt to synchronize ESP32 time with GPS time
    void syncTimeWithGPS(bool force = false);
};

#endif // ROLE_BASE_H