#ifndef LED_STRIP_H
#define LED_STRIP_H

#include <Led.h>

class LedStrip : public Led
{
public:
    LedStrip(){};
    LedStrip(int pinR, int pinG, int pinB) : pinR(pinR), pinG(pinG), pinB(pinB){};

    void setup() override;
    void setRgb(uint8_t r, uint8_t g, uint8_t b) override;
    LedType getType() override { return LedType::LED_STRIP; }
    size_t serialize(int id, uint8_t *buffer) const;
    size_t deserialize(int id, const uint8_t *buffer);

private:
    int pinR;
    int pinG;
    int pinB;
};

#endif