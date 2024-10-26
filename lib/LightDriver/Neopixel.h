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
} CHSV;

class NeoPixel
{
public:
    NeoPixel(int pin = 16) : _strip(1, pin){};
    NeoPixel(Scene hueScene, Scene saturationScene, Scene valueScene, int pin = 16) : _strip(1, pin), _hueScene(hueScene), _saturationScene(saturationScene), _valueScene(valueScene){};

    CHSV _color;
    Scene _hueScene;
    Scene _saturationScene;
    Scene _valueScene;

    void setup();
    void loop();
    void trigger();
    void dump();

private:
    Adafruit_NeoPixel _strip;
};

#endif