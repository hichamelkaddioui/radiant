#ifndef PIXEL_H
#define PIXEL_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Sequence.h>

class Pixel
{
public:
    Pixel(int pin = 16) : _strip(1, pin) {}

    Sequence _hue;
    Sequence _brightness;

    void setup();
    void loop(const GraphBank &graphBank);
    void onNotePlayed(uint8_t note, uint8_t velocity);
    void dump();

private:
    Adafruit_NeoPixel _strip;
};

#endif