#include <Arduino.h>
#include <Scene.h>
#include <Neopixel.h>
#include <Utils.h>

void flashThreeTimes(int delayTime = 100)
{
    pinMode(LED_BUILTIN, OUTPUT);

    for (int i = 0; i < 3; i++)
    {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(delayTime);
        digitalWrite(LED_BUILTIN, LOW);
        delay(delayTime);
    }
}

NeoPixel pixel;

void setup()
{
    Serial.begin(9600);

    while (!Serial)
        ;

    flashThreeTimes();

    // Create the scenes

    Scene hue = Scene({
        Keyframe(0, 65535, Curve::linear()),
        Keyframe(5 * 1000, 0, Curve::gate(60521, 10922, 100)),
        Keyframe(10 * 1000, 0, Curve::gate(60521 / 2, 10922, 100)),
        Keyframe(15 * 1000, 60521, Curve::ease(3.0f)),
        Keyframe(20 * 1000, 0, Curve::linear()),
        Keyframe(201 * 100, 0, Curve::ease(4.0f)),
        Keyframe(22 * 1000, 21845, Curve::linear()),
        Keyframe(221 * 100, 0, Curve::ease(4.0f)),
        Keyframe(24 * 1000, 21845, Curve::linear()),
        Keyframe(25 * 1000, 0, Curve::linear()),
    });

    Scene sat = Scene({
        Keyframe(0, 255, Curve::linear()),
    });

    Scene val = Scene({
        Keyframe(0, 100, Curve::linear()),
        Keyframe(5 * 1000, 0, Curve::ease(3.0f)),
        Keyframe(55 * 100, 255, Curve::linear()),
        Keyframe(55.1 * 100, 0, Curve::ease(3.0f)),
        Keyframe(60 * 100, 255, Curve::linear()),
        Keyframe(60.1 * 100, 0, Curve::ease(3.0f)),
        Keyframe(65 * 100, 255, Curve::linear()),
        Keyframe(65.1 * 100, 0, Curve::ease(3.0f)),
        Keyframe(70 * 100, 255, Curve::linear()),
        Keyframe(75.1 * 100, 0, Curve::ease(3.0f)),
        Keyframe(80 * 100, 255, Curve::gate(0, 100, 30)),
        Keyframe(10 * 1000, 255, Curve::gate(0, 255, 60)),
        Keyframe(12 * 1000, 0, Curve::gate(0, 255, 100)),
        Keyframe(15 * 1000, 255, Curve::gate(0, 255, 60)),
        Keyframe(18 * 1000, 255, Curve::linear()),
        Keyframe(25 * 1000, 255, Curve::linear()),

    });

    pixel = NeoPixel(hue, sat, val);
}

void loop()
{
    pixel.loop();
}
