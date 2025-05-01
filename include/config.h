#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Protocol enumeration
enum Protocol
{
    PROTO_WIFI4 = 1,
    PROTO_WIFI6 = 2,
    PROTO_WIFI_LR = 3,
    PROTO_ESPNOW = 4
};

// Device role enumeration
enum DeviceRole
{
    ROLE_SENDER = 1,
    ROLE_RECEIVER = 2
};

// Select protocol based on build flags
#if defined(PROTOCOL) && defined(WIFI4)
#define ACTIVE_PROTOCOL PROTO_WIFI4
#define PROTOCOL_NAME "WiFi 4 (802.11n)"
#elif defined(PROTOCOL) && defined(WIFI6)
#define ACTIVE_PROTOCOL PROTO_WIFI6
#define PROTOCOL_NAME "WiFi 6 (802.11ax)"
#elif defined(PROTOCOL) && defined(WIFI_LR)
#define ACTIVE_PROTOCOL PROTO_WIFI_LR
#define PROTOCOL_NAME "WiFi Long Range"
#elif defined(PROTOCOL) && defined(ESPNOW)
#define ACTIVE_PROTOCOL PROTO_ESPNOW
#define PROTOCOL_NAME "ESP-NOW"
#else
#error "Protocol not defined. Please select a protocol in platformio.ini"
#endif

// Select device role based on build flags
#if defined(DEVICE_ROLE) && defined(SENDER)
#define ACTIVE_ROLE ROLE_SENDER
#define ROLE_NAME "Sender"
#elif defined(DEVICE_ROLE) && defined(RECEIVER)
#define ACTIVE_ROLE ROLE_RECEIVER
#define ROLE_NAME "Receiver"
#else
#error "Device role not defined. Please select a role in platformio.ini"
#endif

// WiFi Configuration
#ifndef WIFI_CHANNEL
#define WIFI_CHANNEL 6
#endif

// Power Configuration (dBm)
#ifndef TX_POWER
#define TX_POWER 20 // Default to 20 dBm (100 mW) if not specified
#endif

// Australian regulatory max is 30 dBm EIRP (1W)
#define MAX_TX_POWER 30

// UART configuration for GPS module
#define GPS_BAUD_RATE 115200
#define GPS_RX_PIN 4 // Adjust according to your wiring
#define GPS_TX_PIN 5 // Adjust according to your wiring

// Test packet configuration
#ifndef PACKET_SIZE
#define PACKET_SIZE 75 // Default packet size in bytes (simulating MAVLink telemetry)
#endif

#ifndef PACKET_RATE
#define PACKET_RATE 10 // Default packet sending rate in Hz
#endif

// Clock synchronization configuration
#define SYNC_PING_COUNT 10 // Number of pings to send for initial synchronization
#define SYNC_TIMEOUT 5000  // Timeout in ms for each ping/ack exchange

// Network credentials
#define WIFI_SSID "DroneMeshTest"
#define WIFI_PASSWORD "testpassword"

// AP and STA IP addresses (static)
#define AP_IP "192.168.4.1"
#define STA_IP "192.168.4.2"
#define NETMASK "255.255.255.0"

// Data structure for test packets
struct TestPacket
{
    uint32_t sequenceNumber;      // Incrementing sequence number
    int64_t senderTimestamp;      // High-resolution sender timestamp (microseconds)
    uint8_t payload[PACKET_SIZE]; // Fixed payload of 75 bytes
};

// Data structure for GPS information
struct GPSData
{
    float latitude;
    float longitude;
    float altitude;
    int satellites;
    float hdop;
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int centisecond;
    bool valid;
};

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

// CSV header format for log entries
#define CSV_HEADER "ReceivedTimestamp_ms,SequenceNumber,SenderTimestamp_us,ReceiverTimestamp_us,RawLatency_us,CorrectedLatency_ms,RSSI_dBm,ConfiguredTxPower_dBm,GPS_Timestamp_UTC,GPS_Lat,GPS_Lon,GPS_Alt,GPS_Sats,GPS_HDOP"

#endif // CONFIG_H