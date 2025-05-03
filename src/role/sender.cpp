#include "sender.h"

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

    // Initialize clock synchronization
    if (!clockSync.begin()) {
        Serial.println("Failed to initialize clock synchronization.");
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

    // Perform initial clock synchronization with receiver
    if (!performInitialClockSync())
    {
        Serial.println("Failed to perform initial clock synchronization.");
        Serial.println("Continuing anyway, but latency measurements may be inaccurate.");
    }

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

    // Set sender timestamp (microseconds)
    packet.senderTimestamp_us = esp_timer_get_time();

    // Populate sender GPS data
    packet.latitude = gpsHandler->state.lat;
    packet.longitude = gpsHandler->state.lng;
    packet.altitude = gpsHandler->state.alt;
    packet.satellites = gpsHandler->state.num_sats;
    packet.hdop = gpsHandler->state.hdop;

    // Fill payload with non-repeating pattern (simulating MAVLink telemetry)
    for (int i = 0; i < PACKET_SIZE; i++)
    {
        packet.payload[i] = (uint8_t)((i + sequenceNumber) % 256);
    }
}

bool SenderRole::performInitialClockSync()
{
    Serial.println("Performing initial clock synchronization...");

    // Wait for the receiver to be ready
    Serial.println("Waiting for the receiver to be ready for sync...");
    delay(2000);

    // Perform clock synchronization
    int64_t offset = protocol->performClockSync();

    if (offset == 0)
    {
        Serial.println("Clock synchronization failed.");
        return false;
    }

    Serial.printf("Initial clock offset: %lld microseconds\n", offset);
    return true;
}