#include "receiver.h"

// Initialize static member
ReceiverRole *ReceiverRole::instance = nullptr;

ReceiverRole::ReceiverRole(Protocol *protocol, GPSHandler *gpsHandler)
    : Role(protocol, gpsHandler),
      initialOffset(0),
      lastSequenceNumber(0),
      packetCounter(0),
      lostPackets(0),
      statisticsTimer(0)
{

    // Set static instance pointer
    instance = this;
}

ReceiverRole::~ReceiverRole()
{
    // Clear instance pointer
    instance = nullptr;
}

bool ReceiverRole::begin()
{
    Serial.println("Initializing receiver role...");

    // Initialize the protocol
    if (!protocol->begin())
    {
        Serial.println("Failed to initialize protocol.");
        return false;
    }

    // Initialize clock synchronization
    if (!clockSync.begin())
    {
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

    // Set callback for packet reception
    protocol->setPacketCallback(onPacketReceived);

    // Set callback for clock sync
    protocol->setClockSyncCallback(onClockSyncReceived);

    // Reset statistics timer
    statisticsTimer = millis();

    initialized = true;
    Serial.println("Receiver role initialized successfully!");
    return true;
}

void ReceiverRole::loop()
{
    if (!initialized)
    {
        return;
    }

    // Print packet loss statistics every 10 seconds
    unsigned long currentTime = millis();
    if (currentTime - statisticsTimer >= 10000)
    {
        statisticsTimer = currentTime;

        if (packetCounter > 0)
        {
            float lossRate = (float)lostPackets / (float)(lostPackets + packetCounter) * 100.0f;
            Serial.printf("Packet statistics: Received %u, Lost %u, Loss rate %.2f%%\n",
                          packetCounter, lostPackets, lossRate);

            // Reset counters
            packetCounter = 0;
            lostPackets = 0;
        }
    }
}

void ReceiverRole::onPacketReceived(const Protocol::TestPacket &packet, int8_t rssi)
{
    // Forward to instance method
    if (instance)
    {
        instance->processPacket(packet, rssi);
    }
}

void ReceiverRole::onClockSyncReceived(int64_t senderTimestamp, int64_t receiverTimestamp)
{
    // Forward to instance for clock synchronization
    if (instance)
    {
        instance->initialOffset = instance->clockSync.calculateOffsetAsReceiver(senderTimestamp, receiverTimestamp);
        Serial.printf("Initial clock offset: %lld microseconds\n", instance->initialOffset);
    }
}

void ReceiverRole::processPacket(const Protocol::TestPacket &packet, int8_t rssi)
{
    // Record receive timestamp
    int64_t receiverTimestamp = esp_timer_get_time();

    // Calculate raw and corrected latency
    int64_t rawLatency = receiverTimestamp - packet.senderTimestamp_us;
    int64_t correctedLatency = clockSync.calculateCorrectedLatency(rawLatency, initialOffset);

    // Create log entry
    LogEntry entry;

    entry.protocolName = protocol->getProtocolName();
    entry.sequenceNumber = packet.sequenceNumber;
    entry.senderTimestamp_us = packet.senderTimestamp_us;
    entry.receiverTimestamp_us = receiverTimestamp;
    entry.rawLatency_us = rawLatency;
    entry.correctedLatency_us = correctedLatency;
    entry.rssi_dBm = rssi;
    entry.configuredTxPower_dBm = protocol->getTransmitPower();
    entry.configuredChannel = protocol->getChannel();
    entry.receiverGPS_timestamp_us = gpsHandler->state.last_gps_time_ms;
    entry.receiverGPS_latitude = gpsHandler->state.lat;
    entry.receiverGPS_longitude = gpsHandler->state.lng;
    entry.receiverGPS_altitude = gpsHandler->state.alt;
    entry.receiverGPS_satellites = gpsHandler->state.num_sats;
    entry.receiverGPS_hdop = gpsHandler->state.hdop;
    entry.senderGPS_latitude = packet.latitude;
    entry.senderGPS_longitude = packet.longitude;
    entry.senderGPS_altitude = packet.altitude;
    entry.senderGPS_satellites = packet.satellites;
    entry.senderGPS_hdop = packet.hdop;

    // Log entry data
    logPacketData(entry);

    // Calculate packet loss statistics
    calculatePacketLoss(packet.sequenceNumber);

    // Update last sequence number
    lastSequenceNumber = packet.sequenceNumber;

    // Increment packet counter
    packetCounter++;
}

void ReceiverRole::calculatePacketLoss(uint32_t sequenceNumber)
{
    // Only check for packet loss if we have received at least one packet
    if (packetCounter > 0)
    {
        // Check for dropped packets
        if (sequenceNumber > lastSequenceNumber + 1)
        {
            uint32_t dropped = sequenceNumber - lastSequenceNumber - 1;
            lostPackets += dropped;

            Serial.printf("Detected %u dropped packets (seq %u -> %u)\n",
                          dropped, lastSequenceNumber, sequenceNumber);
        }
    }
}

void ReceiverRole::logPacketData(const LogEntry &entry)
{
    char csvLine[512];
    sprintf(csvLine, "%lu,%s,%u,%lld,%lld,%lld,%.3f,%d,%d,%d,%lld,%.6f,%.6f,%.2f,%u,%.2f,%.6f,%.6f,%.2f,%u,%.2f",
            millis(), // Receiver local ms timestamp (useful for ordering)
            entry.protocolName,
            entry.sequenceNumber,
            entry.senderTimestamp_us,
            entry.receiverTimestamp_us,
            entry.rawLatency_us,
            entry.correctedLatency_us / 1000.0f, // CorrectedLatency_ms
            entry.rssi_dBm,
            entry.configuredTxPower_dBm,
            entry.configuredChannel,
            entry.receiverGPS_timestamp_us, // Raw GPS timestamp
            entry.receiverGPS_latitude,
            entry.receiverGPS_longitude,
            entry.receiverGPS_altitude,
            entry.receiverGPS_satellites,
            entry.receiverGPS_hdop,
            entry.senderGPS_latitude,
            entry.senderGPS_longitude,
            entry.senderGPS_altitude,
            entry.senderGPS_satellites,
            entry.senderGPS_hdop);

    // Log to Serial (even if SD card logging failed)
    Serial.println(csvLine);
}