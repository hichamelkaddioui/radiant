#include <vector>
#include <MidiSerial.h>
#include <Led.h>
#include <NeoPixel.h>
#include <Graph.h>
#include <Flash.h>
#include <Utils.h>

MidiSerial midiSerial;
Oled screen;
GraphBank gb = defaultGraphBank();
LedBank lb;
SceneBank sb;

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

void setup1()
{
    // OLED screen
    screen.setup();

    // Flash setup
    RP2040Flash flash;
    flash.begin();

    /**
     * Serialize, write to flash, read from flash and deserialize
     */
    size_t offset = 0;
    uint8_t buffer[1024]{};

    // LedBank
    NeoPixel *pixel = new NeoPixel(16);
    LedBank localLedBank;
    localLedBank._bank[10] = pixel;

    // GraphBank
    GraphBank localGraphBank = defaultGraphBank();
    // Create special graph #9
    localGraphBank._bank[9] = new GraphKeyframe({Keyframe(0.0f, 0.13f, 0.0f), Keyframe(1.0f, 0.12f, 0.0f)});

    // SceneBank
    Sequence *hueA = new Sequence(
        PlaybackMode::ONCE,
        {localGraphBank._bank[DefaultGraph::UP], 0, 127, 500},
        60);
    Sequence *hueB = new Sequence(
        PlaybackMode::REPEAT,
        {localGraphBank._bank[DefaultGraph::UP], 70, 255, 5 * 1000},
        61);
    Sequence *brightnessA = new Sequence(
        PlaybackMode::ONCE,
        {localGraphBank._bank[DefaultGraph::GATE], 255, 0, 500, 1.0f / 10.0f},
        60);
    Sequence *brightnessB = new Sequence(
        PlaybackMode::REPEAT,
        {localGraphBank._bank[DefaultGraph::SINE], 0, 255, 1000},
        61);
    LedEffect *pixelEffect = new LedEffect(pixel, hueA, hueB, brightnessA, brightnessB);

    Scene *firstScene = new Scene();
    firstScene->_ledEffects.push_back(*pixelEffect);

    SceneBank localSceneBank;
    localSceneBank._scenes.push_back(firstScene);
    localSceneBank.getCurrentScene()->dump();

    // Serialize
    offset += localLedBank.serialize(buffer);
    offset += localGraphBank.serialize(buffer + offset);
    offset += localSceneBank.serialize(buffer + offset, localLedBank, localGraphBank);

    // Write to flash
    flash.write(0x0, buffer, offset);

    // Reset buffer
    offset = 0;
    memset(buffer, 0, 1024);

    // Read from flash
    flash.read(0x0, buffer, 1024);

    gb = defaultGraphBank();

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
    Scene *currentScene = sb.getCurrentScene();
    midiSerial.loop(currentScene);
}

void loop1()
{
    Scene *currentScene = sb.getCurrentScene();

    if (currentScene == nullptr)
    {
        debug(23, "[loop] No scene found");
        return;
    }

    currentScene->update();

    screen.displaySceneData(*currentScene);
    screen.loop();
}
