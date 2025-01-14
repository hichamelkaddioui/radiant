#ifndef LED_BANK_H
#define LED_BANK_H

#include <map>
#include <Arduino.h>
#include <Led.h>

class LedBank
{
public:
    std::map<int, Led *> _leds;

    void setup();
    size_t serialize(uint8_t *buffer) const;
    size_t deserialize(const uint8_t *buffer);
    void sysExCreate(const uint8_t *buffer, size_t length);

    static LedBank createDummy();
};

#endif