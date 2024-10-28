#include <Neopixel.h>
#include <FastLED.h>

void NeoPixel::setup()
{
    _strip.begin();
    _strip.show();
}

void NeoPixel::loop()
{
    _color.hue = _hueScene.update();
    _color.sat = _saturationScene.update();
    _color.val = _valueScene.update();
    CRGB colorRGB;
    hsv2rgb_rainbow(_color, colorRGB);
    _strip.setPixelColor(0, Adafruit_NeoPixel::Color(colorRGB.r, colorRGB.g, colorRGB.b));
    _strip.show();
}

void NeoPixel::trigger()
{
    _hueScene.trigger();
    _saturationScene.trigger();
    _valueScene.trigger();
}

void NeoPixel::dump()
{
    _hueScene.dump();
    _saturationScene.dump();
    _valueScene.dump();
}
