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
     * TEST OF SERIALIZATION + FLASH
     * 
     * Create local banks, serialize them, and write everything to flash
     */
    size_t offset = 0, length = 0;
    uint8_t buffer[1024]{};

    // LedBank serialization
    LedBank localLedBank;
    NeoPixel *pixel = new NeoPixel(16);
    localLedBank._bank[0] = pixel;

    length = localLedBank.serialize(buffer);
    debug(1, "[setup] [led bank] serialized %d bits", length);
    offset += length;

    localLedBank.clear();

    // GraphBank serialization
    GraphBank localGraphBank = defaultGraphBank();
    localGraphBank._bank[9] = new GraphKeyframe({Keyframe(0.0f, 0.13f, 0.0f), Keyframe(1.0f, 0.12f, 0.0f)});

    length = localGraphBank.serialize(buffer + offset);
    debug(1, "[setup] [graph bank] serialized %d bits", length);
    offset += length;

    // SceneBank serialization
    Sequence *hue = new Sequence(
        {localGraphBank._bank[DefaultGraph::SINE], 0, 255, 9 * 1000},
        PlaybackMode::REPEAT,
        {61});
    Sequence *brightness = new Sequence(
        {localGraphBank._bank[DefaultGraph::SINE], 50, 100, 10 * 1000, 0.5f},
        PlaybackMode::REPEAT);

    LedEffect *pixelEffect = new LedEffect(localLedBank._bank[0], hue, hue, brightness, brightness);

    Scene *firstScene = new Scene();
    firstScene->_ledEffects.push_back(*pixelEffect);

    SceneBank localSceneBank;
    localSceneBank._scenes.push_back(firstScene);

    length = localSceneBank.serialize(buffer + offset, localLedBank, localGraphBank);
    debug(1, "[setup] [scene bank] serialized %d bits", length);
    offset += length;

    localSceneBank.getCurrentScene()->dump();

    // Write to flash
    flash.write(0x0, buffer, offset);

    // Reset buffer
    offset = 0, length = 0;
    memset(buffer, 0, 1024);

    // Read from flash
    flash.read(0x0, buffer, 1024);

    // DESERIALIZE

    // LedBank setup
    length = lb.deserialize(buffer);
    debug(1, "[setup] deserialized %d bits for led bank", length);
    offset += length;

    lb.setup();

    // GraphBank setup
    gb = defaultGraphBank();
    debug(1, "[setup] [graph bank] number of graphs in new graph bank before: %d", gb._bank.size());
    length = gb.deserialize(buffer + offset);
    debug(1, "[setup] [graph bank] deserialized %d bits", length);
    debug(1, "[setup] [graph bank] number of graphs in new graph bank after: %d", gb._bank.size());
    offset += length;

    // Print value at 0.0f and 1.0f
    GraphKeyframe *keyframeGraph = dynamic_cast<GraphKeyframe *>(const_cast<Graph *>(gb._bank[9]));
    if (keyframeGraph != nullptr)
        debug(1, "[setup] number of keyframes in graph #9: %d, value at 0.0f: %f, value at 1.0f: %f", keyframeGraph->_keyframes.size(), keyframeGraph->valueAt(0.0f), keyframeGraph->valueAt(1.0f));

    // SceneBank deserialization
    length = sb.deserialize(buffer + offset, lb, gb);
    debug(1, "[setup] [scene bank] deserialized %d bits", length);

    sb.getCurrentScene()->dump();

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

    screen.displaySceneData((*currentScene));
    screen.loop();
}
