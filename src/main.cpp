#include <vector>
#include <MidiSerial.h>
#include <Led.h>
#include <NeoPixel.h>
#include <LedStrip.h>
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
    LedBank localLedBank;
    NeoPixel *pixel13 = new NeoPixel(16);
    localLedBank._bank[13] = pixel13;
    NeoPixel *pixel12 = new NeoPixel(-1);
    localLedBank._bank[12] = pixel12;
    LedStrip *ledStrip = new LedStrip(-1, -1, -1);
    localLedBank._bank[666] = ledStrip;

    // GraphBank
    GraphBank localGraphBank = defaultGraphBank();
    // Create special graph #9
    localGraphBank._bank[9] = new GraphKeyframe({Keyframe(0.0f, 0.13f, 0.0f), Keyframe(1.0f, 0.12f, 0.0f)});

    // SceneBank
    SceneBank localSceneBank;
    Sequence *hueA;
    Sequence *hueB;
    Sequence *brightnessA;
    Sequence *brightnessB;
    Scene *scene;

    // SceneBank
    scene = new Scene();
    hueA = new Sequence(PlaybackMode::ONCE, {localGraphBank._bank[DefaultGraph::UP], 0, 127, 500}, 60);
    hueB = new Sequence(PlaybackMode::REPEAT, {localGraphBank._bank[DefaultGraph::UP], 70, 255, 5 * 1000});
    brightnessA = new Sequence(PlaybackMode::ONCE, {localGraphBank._bank[DefaultGraph::GATE], 255, 0, 500, 1.0f / 10.0f}, 60);
    brightnessB = new Sequence(PlaybackMode::REPEAT, {localGraphBank._bank[DefaultGraph::SINE], 0, 255, 1000}, 60);

    scene->_ledEffects[13] = LedEffect(pixel13, hueA, hueB, brightnessA, brightnessB);
    scene->_ledEffects[12] = LedEffect(pixel12, hueB, brightnessA, brightnessB, hueA);
    scene->_ledEffects[666] = LedEffect(ledStrip, brightnessA, brightnessB, hueA, hueB);

    localSceneBank._scenes[12] = scene;

    scene = new Scene();
    hueA = new Sequence(PlaybackMode::REPEAT, {localGraphBank._bank[DefaultGraph::UP], 0, 255, 10 * 1000}, 60);
    hueB = new Sequence(PlaybackMode::REPEAT, {localGraphBank._bank[DefaultGraph::UP], 70, 255, 5 * 1000});
    brightnessA = new Sequence(PlaybackMode::REPEAT, {localGraphBank._bank[DefaultGraph::SINE], 0, 255, 13 * 1000}, 60);
    brightnessB = new Sequence(PlaybackMode::REPEAT, {localGraphBank._bank[DefaultGraph::SINE], 0, 255, 1000});

    scene->_ledEffects[13] = LedEffect(pixel13, hueA, hueB, brightnessA, brightnessB);
    scene->_ledEffects[12] = LedEffect(pixel12, hueB, brightnessA, brightnessB, hueA);
    scene->_ledEffects[666] = LedEffect(ledStrip, brightnessA, brightnessB, hueA, hueB);

    localSceneBank._scenes[13] = scene;

    // Serialize
    offset += localLedBank.serialize(buffer);
    offset += localGraphBank.serialize(buffer + offset);
    offset += localSceneBank.serialize(buffer + offset, localLedBank, localGraphBank);

    // Write to flash
    flash.write(0x0, buffer, offset);

    debugByteArray(buffer, offset);

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
    midiSerial.loop(lb, gb, sb);
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

    screen.loop(sb);
}
