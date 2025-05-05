#include "sender.h"
#include <sys/time.h> // Include for gettimeofday and timeval

SenderRole::SenderRole(Protocol *protocol, GPSHandler *gpsHandler)
    : Role(protocol, gpsHandler), sequenceNumber(0), lastPacketTime(0)
{
}

SenderRole::~SenderRole()
{
    // Sender destructor
}

bool SenderRole::begin()
{
    Serial.println("Initializing sender role...");

    // Initialize the protocol
    if (!protocol->begin())
    {
        Serial.println("Failed to initialize protocol.");
        return false;
    }

    // Wait for GPS fix before continuing
    Serial.println("Waiting for GPS fix...");
    while (!gpsHandler->hasFix())
    {
        gpsHandler->update();
        delay(100);
    }
    Serial.println("GPS fix acquired!");

    Serial.println("Performing initial time sync with GPS...");
    syncTimeWithGPS(true); // Force sync

    initialized = true;
    Serial.println("Sender role initialized successfully!");
    return true;
}

void SenderRole::loop()
{
    if (!initialized)
    {
        return;
    }

    unsigned long currentTime = millis();

    // Periodically synchronize time with GPS
    if (currentTime - lastSyncTimeMs >= SYNC_INTERVAL_MS)
    {
        lastSyncTimeMs = currentTime;
        syncTimeWithGPS();
    }

    // Send test packets at the configured rate
    if (currentTime - lastPacketTime >= (1000 / PACKET_RATE))
    {
        lastPacketTime = currentTime;

        // Prepare and send test packet
        Protocol::TestPacket packet;
        prepareTestPacket(packet);

        if (!protocol->sendPacket(packet))
        {
            Serial.println("Failed to send test packet.");
        }

        // Increment sequence number
        sequenceNumber++;
    }
}

void SenderRole::prepareTestPacket(Protocol::TestPacket &packet)
{
    // Set sequence number
    packet.sequenceNumber = sequenceNumber;

    // Set sender timestamp using wall-clock time (microseconds since epoch)
    struct timeval tv_now;
    if (gettimeofday(&tv_now, NULL) == 0)
    {
        packet.senderTimestamp_us = (int64_t)tv_now.tv_sec * 1000000L + tv_now.tv_usec;
    }
    else
    {
        Serial.println("Sender: Failed to get time of day for packet timestamp!");
        packet.senderTimestamp_us = 0; // Indicate error or invalid time
    }

    // Populate sender GPS data
    packet.latitude = gpsHandler->state.lat / 1e7;
    packet.longitude = gpsHandler->state.lng / 1e7;
    packet.altitude_mm = gpsHandler->state.alt;
    packet.satellites = gpsHandler->state.num_sats;
    packet.horizontalAccuracy_mm = gpsHandler->state.horizontal_accuracy;

    // Fill payload with non-repeating pattern (simulating MAVLink telemetry)
    for (int i = 0; i < PACKET_SIZE; i++)
    {
        packet.payload[i] = (uint8_t)((i + sequenceNumber) % 256);
    }
}