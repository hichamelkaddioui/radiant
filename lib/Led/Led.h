#ifndef LED_NG_H
#define LED_NG_H

#include <vector>
#include <map>
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

enum LedType
{
    LED_NEOPIXEL
};

class Led
{
public:
    virtual void setup() = 0;
    virtual LedType getType() = 0;
    virtual void setRgb(uint8_t r, uint8_t g, uint8_t b) = 0;
};

class LedBank
{
public:
    ~LedBank() { clear(); }

    std::map<int, Led *> _bank;
    void setup();
    void clear();
    size_t serialize(uint8_t *buffer) const;
    size_t deserialize(const uint8_t *buffer);
};

#endif