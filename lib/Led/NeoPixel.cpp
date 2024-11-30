#include <Utils.h>
#include <NeoPixel.h>

void NeoPixel::setup()
{
    _strip = new Adafruit_NeoPixel(1, _pin);
    _strip->begin();
    _strip->show();

    debug(1, "[setup neo pixel] pin: %d", _strip->getPin());
}

void NeoPixel::setRgb(uint8_t r, uint8_t g, uint8_t b)
{
    _strip->setPixelColor(0, Adafruit_NeoPixel::Color(r, g, b));
    _strip->show();
}

NeoPixel::~NeoPixel()
{
    delete _strip;
}