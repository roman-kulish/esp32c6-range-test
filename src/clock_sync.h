#ifndef CLOCK_SYNC_H
#define CLOCK_SYNC_H

#include <Arduino.h>

class ClockSync
{
public:
    ClockSync();

    // Initialize clock synchronization
    bool begin();

    // For sender: initiate clock synchronization with receiver
    int64_t performSyncAsSender();

    // For receiver: participate in clock synchronization initiated by sender
    int64_t calculateOffsetAsReceiver(int64_t senderTimestamp, int64_t localReceiveTime);

    // Calculate the corrected latency
    int64_t calculateCorrectedLatency(int64_t rawLatency, int64_t offset);

private:
    bool initialized;
    int64_t lastOffset;
    unsigned long lastSyncTime;

    // Structure for ping/ack packets
    struct SyncPacket
    {
        int64_t t1; // Original sender timestamp
        int64_t t2; // Receiver timestamp (when received)
    };
};

#endif // CLOCK_SYNC_H