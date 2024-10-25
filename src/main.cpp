#include <Arduino.h>
#include <Scene.h>
#include <Neopixel.h>
#include <Utils.h>
#include <Flash.h>
#include <Serialize.h>

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
const size_t BUFFER_SIZE = 1024;
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
    val.addKeyframes(Scene::impulses(1000, 0.5f, 5));
    val.addKeyframes(Scene::impulses(2000, 2.0f, 5));
    val.addKeyframes(Scene::impulses(3000, 6.0f, 10));
    val.addKeyframes({
        Keyframe(4000, 255, Curve::linear()),
    });

    pixel = NeoPixel(hue, sat, val);

    // Serialize the Scene to a buffer
    size_t serializedSize = serializeScene(hue, buffer);
    Serial.print("Serialized size: ");
    Serial.print(serializedSize);
    Serial.println(" bytes.");

    // Write the serialized Scene to flash memory
    flash.write(FLASH_ADDRESS, buffer, serializedSize);
    Serial.println("Scene written to flash memory.");

    // Prepare to read the data back from flash
    uint8_t *readBuffer = flash.read(FLASH_ADDRESS, serializedSize);
    Serial.println("Scene read from flash memory.");

    // Create a new Scene object for deserialization
    Scene newScene;
    serializedSize = deserializeScene(newScene, readBuffer);
    Serial.print("Deserialized size: ");
    Serial.print(serializedSize);
    Serial.println(" bytes.");

    newScene.dump();

    pixel = NeoPixel(newScene, sat, val);
}

void loop()
{
    pixel.loop();
}
