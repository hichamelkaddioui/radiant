#include <Arduino.h>
#include <Scene.h>
#include <Neopixel.h>
#include <Utils.h>
#include <Flash.h>
#include <Serialize.h>

std::vector<Keyframe> generateImpulses(unsigned long startTime, float power, int numImpulses, int step = 100)
{
    std::vector<Keyframe> keyframes;

    for (unsigned long i = 0; i < numImpulses; i++)
    {
        keyframes.push_back(Keyframe(startTime + i * step, 0, Curve::ease(power)));
        keyframes.push_back(Keyframe(startTime + (i + 1) * step - 1, 255, Curve::linear()));
    }

    return keyframes;
};

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

NeoPixel pixel;
RP2040Flash flash;
const size_t BUFFER_SIZE = 1024 * 4;
uint8_t buffer[BUFFER_SIZE];
const uint32_t FLASH_ADDRESS = 0x000000;

void setup()
{
    Serial.begin(9600);

    while (!Serial)
        ;

    blinkThreeTimes();

    flash.begin();

    // Create the scenes

    Scene hue = Scene({
        Keyframe(0, 65535, Curve::linear()),
        Keyframe(2 * 1000, 0, Curve::ease(0.5f)),
        Keyframe(5 * 1000, 65535, Curve::wave(50, 0, 2500)),
        Keyframe(9 * 1000, 0, Curve::linear()),
    });

    Scene sat = Scene({
        Keyframe(0, 255, Curve::linear()),
    });

    Scene val;

    val.addKeyframes({
        Keyframe(0, 0, Curve::wave(50, 255, 2500)),
    });

    val.addKeyframes(generateImpulses(2000, 2.0f, 30));

    // val.addKeyframes(Scene::generateImpulses(3000, 6.0f, 10));
    val.addKeyframes({
        Keyframe(5000, 128, Curve::ease(0.5f)),
        Keyframe(6000, 255, Curve::linear()),
        Keyframe(7000, 0, Curve::wave(50, 255, 2500)),
        Keyframe(10000, 128, Curve::ease(2.0f)),
    });

    val.addKeyframes(generateImpulses(11000, 2.0f, 10, 500));

    pixel = NeoPixel(hue, sat, val);

    // Serialize pixel

    size_t pixelSize = serializeNeoPixel(pixel, buffer);
    flash.write(FLASH_ADDRESS, buffer, pixelSize);

    debug(1, "[setup] Pixel size: %lu", pixelSize);

    // Deserialize pixel

    NeoPixel pixel2 = NeoPixel();
    flash.read(FLASH_ADDRESS, buffer, pixelSize);
    deserializeNeoPixel(pixel2, buffer);

    pixel2.dump();
}

void loop()
{
    pixel.loop();
}
