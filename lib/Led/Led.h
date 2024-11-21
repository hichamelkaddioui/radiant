#ifndef LED_NG_H
#define LED_NG_H

#include <vector>
#include <map>
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

class Led
{
public:
    virtual void setup() = 0;
    virtual void setRgb(uint8_t r, uint8_t g, uint8_t b) = 0;
};

typedef std::map<int, Led *> LedBank;

#endif