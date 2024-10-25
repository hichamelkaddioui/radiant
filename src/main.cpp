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
        Keyframe(25 * 1000, 0, Curve::linear()),
    });

    Scene sat = Scene({
        Keyframe(0, 255, Curve::linear()),
    });

    Scene val = Scene({
        Keyframe(0, 0, Curve::wave(50, 255, 2500)),
    });

    val.addKeyframes(Scene::impulses(1000, 0.5f, 5));
    val.addKeyframes(Scene::impulses(2000, 2.0f, 5));
    val.addKeyframes(Scene::impulses(3000, 6.0f, 10));
    val.addKeyframes({
        Keyframe(4000, 255, Curve::linear()),
    });

    pixel = NeoPixel(hue, sat, val);
}

void loop()
{
    pixel.loop();
}
