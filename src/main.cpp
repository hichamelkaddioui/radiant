#include <Arduino.h>
#include <Scene.h>
#include <Neopixel.h>
#include <Utils.h>
#include <Flash.h>
#include <Serialize.h>

std::vector<Keyframe> generateImpulses(unsigned long startTime, float power, int numImpulses, int step = 100, int minValue = 0, int maxValue = 255)
{
    std::vector<Keyframe> keyframes;

    for (unsigned long i = 0; i < numImpulses; i++)
    {
        keyframes.push_back(Keyframe(startTime + i * step, minValue, Curve::ease(power)));
        keyframes.push_back(Keyframe(startTime + (i + 1) * step - 1, maxValue, Curve::linear()));
    }

    return keyframes;
}

std::vector<Keyframe> generateIncreasingImpulses(unsigned long startTime, unsigned long period = 20, int minValue = 0, int maxValue = 255)
{
    std::vector<Keyframe> keyframes;

    int i = 0;
    while (period < 1200)
    {
        keyframes.push_back(Keyframe(startTime + i * period * 2, minValue, Curve::gate(minValue, maxValue, period)));
        period *= 1.2;
        i++;
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
        Keyframe(0, HUE_YELLOW, Curve::linear()),
        Keyframe(1000, HUE_YELLOW, Curve::linear()),
        // Keyframe(0, 0, Curve::wave(HUE_GREEN, HUE_PINK, 1000)),
        // Keyframe(0, UTILS_HUE_RED, Curve::linear()),
        // Keyframe(2 * 1000, UTILS_HUE_PINK, Curve::ease(0.5f)),
        // Keyframe(5 * 1000, UTILS_HUE_RED, Curve::wave(UTILS_HUE_YELLOW, UTILS_HUE_GREEN, 2500)),
        // Keyframe(8 * 1000, 0, Curve::gate(UTILS_HUE_PINK, UTILS_HUE_BLUE / 2, 100)),
        // Keyframe(10 * 1000, 0, Curve::linear()),
    });

    Scene sat = Scene({
        Keyframe(0, 255, Curve::linear()),
    });

    Scene val;

    std::vector<Keyframe> impulses = generateIncreasingImpulses(0);
    val.addKeyframes(impulses);

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
