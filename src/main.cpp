#include <MidiSerial.h>
#include <Oled.h>
#include <Flash.h>
#include <SceneBank.h>
#include <Utils.h>

MidiSerial midiSerial;
Oled screen;
GraphBank gb;
LedBank lb;
SceneBank sb;
RP2040Flash flash;

static const int BUFFER_SIZE = 1024 * 4;
static const int PIXEL_INDEX = 0;

void setup()
{
    // USB Serial
    Serial.begin(115200);

    // MIDI Serial
    midiSerial.setup();

    while (!Serial && millis() < 5000)
        ;
}

void createDummy()
{
    size_t offset = 0;
    uint8_t buffer[BUFFER_SIZE]{};

    // Create banks
    LedBank localLedBank = LedBank::createDummy();
    GraphBank localGraphBank = GraphBank::createDummy();
    SceneBank localSceneBank = SceneBank::createDummy(localLedBank, localGraphBank);

    // Serialize
    offset += localLedBank.serialize(buffer);
    offset += localGraphBank.serialize(buffer + offset);
    offset += localSceneBank.serialize(buffer + offset, localLedBank, localGraphBank);

    // Write to flash
    flash.write(0x0, buffer, offset);
}

void setup1()
{
    // OLED screen
    screen.setup();

    // Flash setup
    flash.begin();

    // createDummy();

    // Read from flash
    size_t offset = 0;
    uint8_t buffer[BUFFER_SIZE]{};
    flash.read(0x0, buffer, BUFFER_SIZE);

    // Deserialize
    offset += lb.deserialize(buffer);
    offset += gb.deserialize(buffer + offset);
    offset += sb.deserialize(buffer + offset, lb, gb);

    debug(1, "[setup] read %d bytes from flash", offset);

    // Dump current scene
    sb.getCurrentScene()->dump();

    // Setup led bank
    lb.setup();

    // Done!
    debug(1, "[setup] Setup complete");
}

void loop()
{
    midiSerial.loop(lb, gb, sb, flash);
}

void loop1()
{
    Scene *currentScene = sb.getCurrentScene();

    currentScene->update();

    screen.loop(sb);
}
