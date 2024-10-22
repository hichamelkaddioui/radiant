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

    int numberOfKeyframes = 2;

    // Create the keyframes
    Keyframe *keyframes = new Keyframe[numberOfKeyframes];
    keyframes[0] = KeyframeList::K(0, 0, {KeyframeList::IMP(POWER, 2.0f)});
    keyframes[1] = KeyframeList::K(2000, 255);

    // Create the keyframe list
    KeyframeList *keyframesList = new KeyframeList(numberOfKeyframes, keyframes);
    lightLoop.setup(keyframesList, new KeyframeList(), new KeyframeList());

    debug(1, "Keyframes list created with %d keyframes", keyframesList->_count);
}

void loop()
{
    lightLoop.loop();

    debug(100, "Hue: %d, Saturation: %d, Value: %d", lightLoop.getColor().hue, lightLoop.getColor().sat, lightLoop.getColor().val);

    if (millis() == 5000)
    {
        lightLoop.setColor({255, 255, 255});
    }
}
