#include "role.h"
#include <sys/time.h> // Required for gettimeofday, settimeofday, adjtime
#include <cmath>      // Required for fabs
#include <inttypes.h> // Required for PRIdMAX

// Offset between Unix epoch (1/1/1970) and GPS epoch (6/1/1980) in seconds
const uint64_t GPS_EPOCH_OFFSET_SECONDS = 315964800UL;

// Current GPS leap second offset (GPS time is ahead of UTC)
// As of early 2024, this is 18 seconds. Check for current value!
const int GPS_UTC_LEAP_SECONDS = 18;

// Seconds in one week
const uint64_t SECONDS_PER_WEEK = (7 * 24 * 3600UL);

bool gps_time_to_timeval(uint16_t time_week, uint32_t time_week_ms, struct timeval *tv_out)
{
    if (tv_out == NULL)
    {
        return false;
    }

    // Calculate total seconds from GPS epoch start
    uint64_t gps_seconds_total = ((uint64_t)time_week * SECONDS_PER_WEEK) + (time_week_ms / 1000);

    // Calculate Unix seconds (accounting for epoch difference and leap seconds to get UTC)
    uint64_t unix_seconds_total = gps_seconds_total + GPS_EPOCH_OFFSET_SECONDS - GPS_UTC_LEAP_SECONDS;

    // Calculate microseconds part
    uint32_t unix_microseconds = (time_week_ms % 1000) * 1000;

    tv_out->tv_sec = (time_t)unix_seconds_total;
    tv_out->tv_usec = (suseconds_t)unix_microseconds;

    return true; // Indicate success
}

Role::Role(Protocol *protocol, GPSHandler *gpsHandler)
    : protocol(protocol), gpsHandler(gpsHandler),
      lastSyncTimeMs(0), initialized(false)
{
}

Role::~Role()
{
    // Base class destructor
}

void Role::syncTimeWithGPS(bool force)
{
    // Check if GPS handler is valid, has a fix, and a valid week number
    if (!gpsHandler || !gpsHandler->hasFix() || gpsHandler->state.time_week == 0)
    {
        // Serial.println("Time Sync: No GPS fix, skipping.");
        return;
    }

    struct timeval gps_tv;
    if (!gps_time_to_timeval(gpsHandler->state.time_week, gpsHandler->state.time_week_ms, &gps_tv))
    {
        Serial.println("Time Sync: Failed to convert GPS time to timeval.");
        return;
    }

    struct timeval esp_tv;
    if (gettimeofday(&esp_tv, NULL) != 0)
    {
        Serial.println("Time Sync: Failed to get ESP32 system time.");
        return;
    }

    // Calculate offset in seconds (floating point for precision)
    int64_t offset_us = ((int64_t)esp_tv.tv_sec - (int64_t)gps_tv.tv_sec) * 1000000L +
                        ((int64_t)esp_tv.tv_usec - (int64_t)gps_tv.tv_usec);

    // Serial.printf("Time Sync: Current offset: %.6f s\n", offset_s);

    if (force || llabs(offset_us) > LARGE_OFFSET_THRESHOLD_US)
    {
        // Large offset or forced sync: Use settimeofday
        if (settimeofday(&gps_tv, NULL) == 0)
        {
            Serial.printf("Time Sync: Large offset (%lld us) or forced. Setting time directly.\n", offset_us);
        }
        else
        {
            Serial.println("Time Sync: Error calling settimeofday().");
        }
    }
    else if (offset_us != 0)
    {
        // Offset is small, apply gradual adjustment via adjtime
        // Calculate the full delta needed (opposite sign of offset)
        int64_t delta_us = -offset_us;

        // Create a timeval struct for the delta
        struct timeval tv_delta;
        tv_delta.tv_sec = delta_us / 1000000L;
        tv_delta.tv_usec = delta_us % 1000000L;

        // Normalize the timeval struct
        if (tv_delta.tv_usec < 0)
        {
            tv_delta.tv_sec -= 1;
            tv_delta.tv_usec += 1000000L;
        }

        if (adjtime(&tv_delta, NULL) != 0)
        {
            Serial.println("Clock Adjust: adjtime() failed.");
        }
        else
        {
            Serial.printf("Clock Adjust: Small offset (%lld us). Adjusting via adjtime() with delta %lld us.\n", offset_us, delta_us);
        }
    }
    else
    {
        // Clock is already synchronized within measurement limits
        // Serial.println("Clock Adjust: Already synchronized.");
    }
}