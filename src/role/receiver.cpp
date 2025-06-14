#include "receiver.h"
#include <sys/time.h>

// Initialize static member
ReceiverRole *ReceiverRole::instance = nullptr;

ReceiverRole::ReceiverRole(Protocol *protocol, GPSHandler *gpsHandler)
    : Role(protocol, gpsHandler),
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

    // Set callback for packet reception
    protocol->setPacketCallback(onPacketReceived);

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

    unsigned long currentTime = millis();

    // Periodically synchronize time with GPS
    if (currentTime - lastSyncTimeMs >= SYNC_INTERVAL_MS)
    {
        lastSyncTimeMs = currentTime;
        syncTimeWithGPS();
    }

    // Print packet loss statistics every 10 seconds
    if (currentTime - statisticsTimer >= 10000)
    {
        statisticsTimer = currentTime;

        if (packetCounter > 0)
        {
            float lossRate = (float)lostPackets / (float)(lostPackets + packetCounter) * 100.0f;
            Serial.printf("Packet statistics: Received %lu, Lost %lu, Loss rate %.2f%%\n",
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

void ReceiverRole::processPacket(const Protocol::TestPacket &packet, int8_t rssi)
{
    // Record receive timestamp
    int64_t receiverTimestamp_us;
    struct timeval tv_now;
    if (gettimeofday(&tv_now, NULL) == 0)
    {
        receiverTimestamp_us = (int64_t)tv_now.tv_sec * 1000000L + tv_now.tv_usec;
    }
    else
    {
        Serial.println("Receiver: Failed to get time of day for packet timestamp!");
        receiverTimestamp_us = 0; // Indicate error or invalid time
    }

    // Calculate raw and corrected latency
    int64_t latency_us = (receiverTimestamp_us != 0 && packet.senderTimestamp_us != 0) ? (receiverTimestamp_us - packet.senderTimestamp_us) : 0;

    // Create log entry
    LogEntry entry;

    entry.protocolName = protocol->getProtocolName();
    entry.sequenceNumber = packet.sequenceNumber;
    entry.senderTimestamp_us = packet.senderTimestamp_us;
    entry.receiverTimestamp_us = receiverTimestamp_us;
    entry.latency_us = latency_us; // Use the calculated latency
    entry.rssi_dBm = rssi;         // Store the RSSI
    entry.configuredTxPower_dBm = protocol->getTransmitPower();
    entry.configuredChannel = protocol->getChannel();
    entry.receiverGPS_latitude = gpsHandler->state.lat / 1e7;
    entry.receiverGPS_longitude = gpsHandler->state.lng / 1e7;
    entry.receiverGPS_altitude_mm = gpsHandler->state.alt;
    entry.receiverGPS_satellites = gpsHandler->state.num_sats;
    entry.receiverGPS_horizontalAccuracy_mm = gpsHandler->state.horizontal_accuracy;
    entry.senderGPS_latitude = packet.latitude;
    entry.senderGPS_longitude = packet.longitude;
    entry.senderGPS_altitude_mm = packet.altitude_mm;
    entry.senderGPS_satellites = packet.satellites;
    entry.senderGPS_horizontalAccuracy_mm = packet.horizontalAccuracy_mm;

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

            Serial.printf("Detected %lu dropped packets (seq %lu -> %lu)\n",
                          dropped, lastSequenceNumber, sequenceNumber);
        }
    }
}

void ReceiverRole::logPacketData(const LogEntry &entry)
{
    // Calculate distance between sender and receiver
    double distance_m = GPSHandler::calculateDistance(
        entry.receiverGPS_latitude, entry.receiverGPS_longitude,
        entry.senderGPS_latitude, entry.senderGPS_longitude);

    char csvLine[512];
    sprintf(csvLine, "%lu,%s,%lu,%lld,%lld,%lld,%d,%d,%d,%.6f,%.6f,%.2f,%u,%.2f,%.6f,%.6f,%.2f,%u,%.2f,%.2f",
            millis(), // Receiver local ms timestamp (useful for ordering)
            entry.protocolName,
            entry.sequenceNumber,
            entry.senderTimestamp_us,
            entry.receiverTimestamp_us,
            entry.latency_us,
            entry.rssi_dBm,
            entry.configuredTxPower_dBm,
            entry.configuredChannel,
            entry.receiverGPS_latitude,
            entry.receiverGPS_longitude,
            entry.receiverGPS_altitude_mm / 1000.0f,
            entry.receiverGPS_satellites,
            entry.receiverGPS_horizontalAccuracy_mm / 1000.0f,
            entry.senderGPS_latitude,
            entry.senderGPS_longitude,
            entry.senderGPS_altitude_mm / 1000.0f,
            entry.senderGPS_satellites,
            entry.senderGPS_horizontalAccuracy_mm / 1000.0f,
            distance_m);

    // Log to Serial (even if SD card logging failed)
    Serial.println(csvLine);
}