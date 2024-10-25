#ifndef NEOPIXEL_H
#define NEOPIXEL_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Scene.h>

typedef struct
{
    uint16_t hue;
    uint8_t sat;
    uint8_t val;

    uint32_t toUint32() const
    {
        return (hue << 16) | (sat << 8) | val;
    }

} CHSV;

class NeoPixel
{
public:
    NeoPixel(int pin = 16) : _strip(1, pin){};
    NeoPixel(Scene hueState, Scene saturationState, Scene valueState, int pin = 16) : _strip(1, pin), _hueState(hueState), _saturationState(saturationState), _valueState(valueState){};

    void setup();
    void loop();
    void setColor(CHSV color) { _color = color; }
    CHSV getColor() { return _color; }

private:
    Adafruit_NeoPixel _strip;
    CHSV _color;
    Scene _hueState;
    Scene _saturationState;
    Scene _valueState;
};

#endif