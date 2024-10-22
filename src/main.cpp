#include <Arduino.h>
#include <Keyframe.h>
#include <LightLoop.h>
#include <Utils.h>

LightLoop lightLoop;

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

void setup()
{
    Serial.begin(9600);

    while (!Serial)
        ;

    flashThreeTimes();

    // Create the keyframe list
    lightLoop.setup(new KeyframeList(KeyframeList::dummyKeyframes(10)), new KeyframeList(), new KeyframeList());
}

void loop()
{
    lightLoop.loop();

    debug(100, "(%lu) Hue: %d, Saturation: %d, Value: %d", millis(), lightLoop.getColor().hue, lightLoop.getColor().sat, lightLoop.getColor().val);

    if (millis() == 5000)
    {
        lightLoop.setColor({255, 255, 255});
    }
}
