#include <Neopixel.h>

void NeoPixel::setup()
{
    _strip.begin();
    _strip.show();
}

void NeoPixel::loop()
{
    _color.hue = _hueState.update();
    _color.sat = _saturationState.update();
    _color.val = _valueState.update();
    _strip.setPixelColor(0, Adafruit_NeoPixel::ColorHSV(_color.hue, _color.sat, _color.val));
    _strip.show();
}