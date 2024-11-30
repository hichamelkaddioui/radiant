#include <Led.h>
#include <Adafruit_NeoPixel.h>

class NeoPixel : public Led
{
public:
    NeoPixel(){};
    NeoPixel(int pin) : _pin(pin){};
    ~NeoPixel();

    int _pin;
    void setup() override;
    void setRgb(uint8_t r, uint8_t g, uint8_t b) override;
    LedType getType() override { return LedType::LED_NEOPIXEL; }

private:
    Adafruit_NeoPixel *_strip;
};
