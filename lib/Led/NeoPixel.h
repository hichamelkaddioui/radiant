#include <Led.h>
#include <Adafruit_NeoPixel.h>

class NeoPixel : public Led
{
public:
    NeoPixel(int pin = 16) : _strip(1, pin) {}
    void setup() override;
    void setRgb(uint8_t r, uint8_t g, uint8_t b) override;

private:
    Adafruit_NeoPixel _strip;
};
