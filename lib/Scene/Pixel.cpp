#include <Utils.h>
#include <FastLED.h>
#include <Pixel.h>

void Pixel::setup()
{
    _strip.begin();
    _strip.show();
}

void Pixel::loop(const GraphBank &graphBank)
{
    uint8_t hue = _hue.update(graphBank);
    uint8_t brightness = _brightness.update(graphBank);

    CRGB colorRGB;
    hsv2rgb_rainbow(CHSV(hue, 255, brightness), colorRGB);

    _strip.setPixelColor(0, Adafruit_NeoPixel::Color(colorRGB.r, colorRGB.g, colorRGB.b));
    _strip.show();
}

void Pixel::onNotePlayed(uint8_t note, uint8_t velocity)
{
    debug(1, "[pixel] Note played %d, velocity %d", note, velocity);
    _hue.onNotePlayed(note, velocity);
    _brightness.onNotePlayed(note, velocity);
}

void Pixel::dump()
{
    _hue.dump();
    _brightness.dump();
}