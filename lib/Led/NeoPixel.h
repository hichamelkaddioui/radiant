#ifndef LED_NEOPIXEL_H
#define LED_NEOPIXEL_H

#include <Led.h>
#include <Adafruit_NeoPixel.h>

class NeoPixel : public Led
{
public:
    NeoPixel(){};
    NeoPixel(int pin) : _pin(pin){};
    ~NeoPixel();

    void setup() override;
    void setRgb(uint8_t r, uint8_t g, uint8_t b) override;
    LedType getType() override { return LedType::LED_NEOPIXEL; }
    size_t serialize(int id, uint8_t *buffer) const;
    size_t deserialize(int id, const uint8_t *buffer);

private:
    int _pin;
    Adafruit_NeoPixel *_strip;
};

#endif