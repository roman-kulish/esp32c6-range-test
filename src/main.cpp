#include <Arduino.h>
#include "config.h"
#include "gps_handler.h"

// Include protocol headers
#include "protocol/wifi.h"
#include "protocol/espnow.h"

// Include role headers
#include "role/sender.h"
#include "role/receiver.h"

#define GNSS_GPS 0x00
#define GNSS_SBAS 0x01
#define GNSS_GALILEO 0x02
#define GNSS_BEIDOU 0x03
#define GNSS_GLONASS 0x06

GPSHandler gpsHandler;
Protocol *protocol = nullptr;
Role *role = nullptr;

void setup()
{
    Serial.begin(115200);

    bool isSender = false;

#if defined(SENDER)
    isSender = true;
    Serial.println("Role: Sender");
#else
    // Wait for Serial port to connect. Needed for native USB port only
    while (!Serial)
    {
        delay(10); // small delay to prevent busy-waiting
    }

    Serial.println("Role: Receiver");
#endif

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    Serial.println();
    Serial.println("============================================");
    Serial.println("Drone Mesh Network - Point-to-Point Test");
    Serial.println("============================================");

    Protocol::ProtocolType proto;

#if defined(PROTOCOL) && PROTOCOL == PROTOCOL_WIFI_4
    Serial.println("Protocol: WiFi 4 (802.11n)");
    proto = Protocol::ProtocolType::PROTO_WIFI4;
#elif defined(PROTOCOL) && PROTOCOL == PROTOCOL_WIFI_6
    Serial.println("Protocol: WiFi 6 (802.11ax)");
    proto = Protocol::ProtocolType::PROTO_WIFI6;
#elif defined(PROTOCOL) && PROTOCOL == PROTOCOL_WIFI_LR
    Serial.println("Protocol: WiFi Long Range");
    proto = Protocol::ProtocolType::PROTO_WIFI_LR;
#elif defined(PROTOCOL) && PROTOCOL == PROTOCOL_ESP_NOW
    Serial.println("Protocol: ESP-NOW");
    proto = Protocol::ProtocolType::PROTO_ESPNOW;
#else
    Serial.println("ERROR: Protocol not properly defined!");
    while (1)
    {
        delay(1000);
    } // Hang
#endif

    switch (proto)
    {
    case Protocol::ProtocolType::PROTO_WIFI4:
    case Protocol::ProtocolType::PROTO_WIFI6:
    case Protocol::ProtocolType::PROTO_WIFI_LR:
        protocol = new WiFiProtocol(proto, WIFI_CHANNEL, TX_POWER, isSender);
        break;

    case Protocol::ProtocolType::PROTO_ESPNOW:
        protocol = new ESPNOWProtocol(WIFI_CHANNEL, TX_POWER);
        break;
    }

    Serial.printf("TX Power: %d dBm\n", TX_POWER);
    Serial.printf("WiFi Channel: %d\n", WIFI_CHANNEL);

    if (isSender)
    {
        Serial.printf("Packet Size: %d bytes\n", PACKET_SIZE);
        Serial.printf("Packet Rate: %d Hz\n", PACKET_RATE);
    }

    Serial.println("============================================");

    Serial1.begin(GPS_BAUD_RATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

    gpsHandler.gnss_mode = (1U << GNSS_GPS) |
                           (1U << GNSS_SBAS) |
                           (1U << GNSS_GALILEO) |
                           (1U << GNSS_BEIDOU) |
                           (1U << GNSS_GLONASS);

    gpsHandler.rate_ms = 100;
    gpsHandler.save_config = 2;

    gpsHandler.begin(&Serial1);

    // Create appropriate role
    if (isSender)
    {
        role = new SenderRole(protocol, &gpsHandler);
    }
    else
    {
        role = new ReceiverRole(protocol, &gpsHandler);
    }

    // Start role operation
    if (!role->begin())
    {
        Serial.println("Failed to initialize role. Check connections and settings.");
        while (1)
        {
            delay(1000);
        } // Hang
    }
}

void loop()
{
    gpsHandler.update();

    digitalWrite(LED_BUILTIN, gpsHandler.hasFix() ? HIGH : LOW);

    role->loop();

    delay(10);
}