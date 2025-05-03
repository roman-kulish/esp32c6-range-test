#include "clock_sync.h"
#include "config.h"

ClockSync::ClockSync() : initialized(false), lastOffset(0), lastSyncTime(0)
{
}

bool ClockSync::begin()
{
    initialized = true;
    return true;
}

int64_t ClockSync::performSyncAsSender()
{
    // This method will need to use the selected protocol to send
    // ping packets and receive ack packets from the receiver

    // Implementation will be specific to the protocol used (WiFi or ESP-NOW)
    // and will be implemented in the protocol-specific classes

    // This is a placeholder method
    return 0;
}

int64_t ClockSync::calculateOffsetAsReceiver(int64_t senderTimestamp, int64_t localReceiveTime)
{
    // Calculate offset (difference between local and remote clock)
    // This is a basic implementation assuming one-way delay is half of round-trip time
    // In reality, we'd need to implement the full algorithm described in the document

    // For now, we'll just use the difference between the timestamps
    int64_t estimatedOffset = localReceiveTime - senderTimestamp;

    // Store the offset for future use
    lastOffset = estimatedOffset;
    lastSyncTime = millis();

    return estimatedOffset;
}

int64_t ClockSync::calculateCorrectedLatency(int64_t rawLatency, int64_t offset)
{
    // Apply the offset to calculate the corrected latency
    return rawLatency - offset;
}