#include <Arduino.h>
#include <Utils.h>
#include <Flash.h>
#include <Pixel.h>

void blinkThreeTimes(int delayTime = 100)
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

GraphBank gb = defaultGraphBank();
Pixel pixel;

void setup()
{
    Serial.begin(9600);

#ifdef DEBUG
    while (!Serial)
        ;
#endif

    blinkThreeTimes();

    pixel._hue = Sequence(0, 255, 10000, 1, PlaybackMode::REPEAT);
    pixel._brightness = Sequence(0, 200, 3000, 3, PlaybackMode::ONCE);
    pixel.setup();
    pixel.dump();

    debug(1, "[setup] setup done");
}

void loop()
{
    pixel.loop(gb);
}
