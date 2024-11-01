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

std::vector<Keyframe> generateDecreasingImpulses(unsigned long startTime, unsigned long period = 20, int minValue = 0, int maxValue = 255)
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
}

std::vector<Keyframe> generateIncreasingImpulses(unsigned long startTime, unsigned long startPeriod = 1200, int minValue = 0, int maxValue = 255)
{
    unsigned long offset = 0;
    unsigned long period = startPeriod;
    std::vector<Keyframe> keyframes;

    while (30 < period)
    {
        keyframes.push_back(Keyframe(startTime + offset, minValue, Curve::gate(minValue, maxValue, period)));
        period *= 0.9;
        offset += period * 2;
    }

    return keyframes;
}

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

NeoPixel pixel, pixelFromFlash;

void setup()
{
    Serial.begin(9600);

#ifdef DEBUG
    while (!Serial)
        ;
#endif

    blinkThreeTimes();

    SceneMap scenes;

    std::vector<Keyframe> hueImpulses = generateIncreasingImpulses(0, 2000, UTILS_HUE_GOLDEN_YELLOW, UTILS_HUE_AQUA);
    scenes[1312] = Scene();
    scenes[1312].addKeyframes(hueImpulses);
    scenes[1312].addKeyframes({
        Keyframe(35 * 1000, UTILS_HUE_AQUA, Curve::gate(UTILS_HUE_GOLDEN_YELLOW, UTILS_HUE_AQUA, 35)),
        Keyframe(45 * 1000, UTILS_HUE_AQUA, Curve::linear()),
    });

    // Create the scenes
    Scene hue, sat, val;

    hue.addKeyframes(hueImpulses);
    hue.addKeyframes({
        Keyframe(35 * 1000, UTILS_HUE_AQUA, Curve::gate(UTILS_HUE_GOLDEN_YELLOW, UTILS_HUE_AQUA, 35)),
        Keyframe(45 * 1000, UTILS_HUE_AQUA, Curve::linear()),
    });
    sat.addKeyframes({Keyframe(0, 255, Curve::linear())});
    val.addKeyframes({
        Keyframe(0, 255, Curve::linear()),
        Keyframe(38 * 1000 - 1, 255, Curve::linear()),
        Keyframe(38 * 1000, 0, Curve::linear()),
        Keyframe(45 * 1000, 0, Curve::linear()),
    });

    pixel = NeoPixel(scenes[1312], sat, val);

    // Serialize pixel

    RP2040Flash flash;
    const uint32_t FLASH_ADDRESS = 0x000000;
    flash.begin();

    const size_t BUFFER_SIZE = 1024 * 4;
    uint8_t buffer[BUFFER_SIZE];

    // size_t pixelSize = serializeNeoPixel(pixel, buffer);
    // flash.write(FLASH_ADDRESS, buffer, pixelSize);
    // debug(1, "[setup] Pixel size: %lu", pixelSize);

    // Serialize scenes

    size_t sceneMapSize = serializeSceneMap(scenes, buffer);
    flash.write(FLASH_ADDRESS, buffer, sceneMapSize);
    debug(1, "[setup] Scene map size: %lu", sceneMapSize);

    // Deserialize scenes

    SceneMap newScenes;
    flash.read(FLASH_ADDRESS, buffer, sceneMapSize);
    deserializeSceneMap(newScenes, buffer);

    // Deserialize pixel

    // flash.read(FLASH_ADDRESS, buffer, pixelSize);
    // deserializeNeoPixel(pixelFromFlash, buffer);
    // pixelFromFlash.dump();

    debug(1, "[setup] setup done");
}

void loop()
{
    pixel.loop();
}
