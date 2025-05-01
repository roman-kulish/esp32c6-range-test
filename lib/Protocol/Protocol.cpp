#include "Protocol.h"

Protocol::Protocol(uint8_t channel, int8_t txPower)
    : channel(channel), txPower(txPower), initialized(false)
{

    // Ensure TX power is within regulatory limits
    if (txPower > TX_POWER)
    {
        this->txPower = TX_POWER;
        Serial.printf("Warning: TX power capped to regulatory limit of %d dBm\n", TX_POWER);
    }
}

Protocol::~Protocol()
{
    // Virtual destructor for proper cleanup in derived classes
}

bool Protocol::isInitialized() const
{
    return initialized;
}

int8_t Protocol::getTransmitPower() const
{
    return txPower;
}

uint8_t Protocol::getChannel() const
{
    return channel;
}