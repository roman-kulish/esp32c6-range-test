#include "gps.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

GPSHandler::GPSHandler() : gpsSerial(nullptr) {}

GPSHandler::~GPSHandler()
{
    if (gpsSerial)
    {
        gpsSerial->end();
    }
}

void GPSHandler::begin(HardwareSerial *serial)
{
    gpsSerial = serial;
}

void GPSHandler::update()
{
    if (!gpsSerial)
        return; // Not initialized

    AP_GPS_UBLOX::update();
}

bool GPSHandler::hasFix() const
{
    return this->state.status >= GPS_Status::GPS_OK_FIX_3D;
}

double GPSHandler::calculateDistance(double lat1, double lon1, double lat2, double lon2)
{
    const double earthRadiusKm = 6371.0;

    // Convert degrees to radians
    lat1 = lat1 * M_PI / 180.0;
    lon1 = lon1 * M_PI / 180.0;
    lat2 = lat2 * M_PI / 180.0;
    lon2 = lon2 * M_PI / 180.0;

    // Differences
    double dLat = lat2 - lat1;
    double dLon = lon2 - lon1;

    // Haversine formula
    double a = sin(dLat / 2) * sin(dLat / 2) +
               cos(lat1) * cos(lat2) *
                   sin(dLon / 2) * sin(dLon / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    double distance = earthRadiusKm * c;

    // Convert to meters
    return distance * 1000.0;
}

void GPSHandler::I_setBaud(int baud)
{
    if (gpsSerial)
    {
        gpsSerial->updateBaudRate(baud);
    }
}

int GPSHandler::I_read(uint8_t *data, size_t len)
{
    if (gpsSerial)
    {
        return gpsSerial->read(data, len);
    }

    return -1;
}

int GPSHandler::I_write(uint8_t *data, size_t len)
{
    if (gpsSerial)
    {
        return gpsSerial->write(data, len);
    }

    return -1;
}

int GPSHandler::I_available()
{
    if (gpsSerial)
    {
        return gpsSerial->available();
    }

    return 0;
}

int GPSHandler::I_availableForWrite()
{
    if (gpsSerial)
    {
        return gpsSerial->availableForWrite();
    }

    return 0;
}

uint32_t GPSHandler::I_millis()
{
    return millis();
}

void GPSHandler::I_print(const char *str)
{
    Serial.print("[AP_GPS_UBLOX] ");
    Serial.print(str);
}