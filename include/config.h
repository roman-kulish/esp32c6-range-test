#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// WiFi Configuration
#ifndef WIFI_CHANNEL
#define WIFI_CHANNEL 6
#endif

// Power Configuration (dBm)
#ifndef TX_POWER
#define TX_POWER 30 // Australian regulatory max is 30 dBm EIRP (1W)
#endif

// UART configuration for GPS module
#define GPS_BAUD_RATE 115200
#define GPS_RX_PIN 4
#define GPS_TX_PIN 5

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

#define PROTOCOL_WIFI_4 1
#define PROTOCOL_WIFI_6 2
#define PROTOCOL_WIFI_LR 3
#define PROTOCOL_ESP_NOW 4

// AP and STA IP addresses (static)
#define AP_IP "192.168.4.1"
#define STA_IP "192.168.4.2"
#define NETMASK "255.255.255.0"

#define DATA_PORT 44444 // UDP port for data
#define SYNC_PORT 44445 // UDP port for clock sync

#endif // CONFIG_H