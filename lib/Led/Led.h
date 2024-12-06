#ifndef LED_H
#define LED_H

#include <Arduino.h>

enum LedType
{
    LED_NEOPIXEL,
    LED_STRIP,
};

class Led
{
public:
    virtual void setup() = 0;
    virtual LedType getType() = 0;
    virtual void setRgb(uint8_t r, uint8_t g, uint8_t b) = 0;
    virtual size_t serialize(int id, uint8_t *buffer) const = 0;
    virtual size_t deserialize(int id, const uint8_t *buffer) = 0;
};

#endif