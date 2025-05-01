#include <Arduino.h>
#include "config.h"
#include "GPS.h"
#include "WiFiProtocol.h"

GPSHandler gpsHandler;
Protocol *protocol = nullptr;

void setup()
{
    Serial.begin(115200);

    Serial.println();
    Serial.println("============================================");
    Serial.println("Drone Mesh Network - Point-to-Point Test");
    Serial.println("============================================");

    bool isSender = false;

#if defined(SENDER)
    isSender = true;
    Serial.println("Role: Sender");
#else
    Serial.println("Role: Receiver");
#endif

#if defined(WIFI4)
    Serial.println("Protocol: WiFi 4 (802.11n)");
    WiFiProtocol *wifiProtocol = new WiFiProtocol(WiFiProtocol::WiFiMode::WIFI_PROTO_802_11N, WIFI_CHANNEL, TX_POWER, isSender);
    protocol = wifiProtocol;
#elif defined(WIFI6)
    Serial.println("Protocol: WiFi 6 (802.11ax)");
    WiFiProtocol *wifiProtocol = new WiFiProtocol(WiFiProtocol::WiFiMode::WIFI_PROTO_802_11AX, WIFI_CHANNEL, TX_POWER, isSender);
    protocol = wifiProtocol;
#elif defined(WIFI_LR)
    Serial.println("Protocol: WiFi Long Range");
    WiFiProtocol *wifiProtocol = new WiFiProtocol(WiFiProtocol::WiFiMode::WIFI_PROTO_802_LR, WIFI_CHANNEL, TX_POWER, isSender);
    protocol = wifiProtocol;
#elif defined(ESPNOW)
    Serial.println("Protocol: ESP-NOW");
    protocol = new ESPNOWProtocol(WIFI_CHANNEL, TX_POWER);
#else
    Serial.println("ERROR: Protocol not properly defined!");
    while (1)
    {
        delay(1000);
    } // Hang
#endif

    Serial.printf("TX Power: %d dBm\n", TX_POWER);
    Serial.printf("WiFi Channel: %d\n", WIFI_CHANNEL);
    Serial.printf("Packet Size: %d bytes\n", PACKET_SIZE);
    Serial.printf("Packet Rate: %d Hz\n", PACKET_RATE);
    Serial.println("============================================");

    Serial1.begin(GPS_BAUD_RATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

    gpsHandler.gnss_mode = 77; // Enable GPS + Galileo + Beidou + GLONASS
    gpsHandler.rate_ms = 100;
    gpsHandler.save_config = 2;

    gpsHandler.begin(&Serial1);
}

void loop()
{
    static uint32_t ts = millis();

    gpsHandler.update();

    if (millis() - ts > 1000)
    {
        ts = millis();
        Serial.printf("tow:%d dt:%d sats:%d lat:%d lng:%d alt:%d hacc:%d vacc:%d fix:%d\n", (int)gpsHandler.state.time_week_ms, (int)gpsHandler.timing.average_delta_us, (int)gpsHandler.state.num_sats, (int)gpsHandler.state.lat, (int)gpsHandler.state.lng, (int)gpsHandler.state.alt, (int)gpsHandler.state.horizontal_accuracy, (int)gpsHandler.state.vertical_accuracy, (int)gpsHandler.state.status);
    }
}