#include <NeoPixel.h>

void NeoPixel::setup()
{
    _strip.begin();
    _strip.show();
}

void NeoPixel::setRgb(uint8_t r, uint8_t g, uint8_t b)
{
    _strip.setPixelColor(0, Adafruit_NeoPixel::Color(r, g, b));
    _strip.show();
}