#ifndef GPS_HANDLER_H
#define GPS_HANDLER_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include <qqqlab_GPS_UBLOX.h>

class GPSHandler : public AP_GPS_UBLOX
{
public:
    GPSHandler();
    ~GPSHandler();

    void begin(HardwareSerial *serial);
    void update();

    // Calculate distance between two GPS points using Haversine formula
    static double calculateDistance(double lat1, double lon1, double lat2, double lon2);

    // Check if GPS has a fix
    bool hasFix() const;

private:
    HardwareSerial *gpsSerial = nullptr;

    // Implementation of AP_GPS_UBLOX pure virtual methods
    void I_setBaud(int baud) override;
    int I_read(uint8_t *data, size_t len) override;
    int I_write(uint8_t *data, size_t len) override;
    int I_available() override;
    int I_availableForWrite() override;
    uint32_t I_millis() override;
    void I_print(const char *str) override;
};

#endif // GPS_HANDLER_H