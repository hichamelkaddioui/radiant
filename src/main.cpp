#include <vector>
#include <MidiSerial.h>
#include <Led.h>
#include <NeoPixel.h>
#include <Graph.h>
#include <Utils.h>

MidiSerial midiSerial;
Oled screen;
GraphBank gb = defaultGraphBank();
LedBank lb, lb2;
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

    /**
     * LED SETUP + SERIALIZE AND DESERIALIZE
     */
    NeoPixel *pixel = new NeoPixel(16);
    lb._bank[0] = pixel;
    // pixel->setup();

    size_t bitsInBuffer = 0;
    uint8_t buffer[1024]{};

    // LedBank serialization
    bitsInBuffer = lb.serialize(buffer);
    debug(1, "[setup] serialized %d bits for led bank", bitsInBuffer);

    bitsInBuffer = lb2.deserialize(buffer);
    debug(1, "[setup] deserialized %d bits for led bank", bitsInBuffer);

    for (const auto &it : lb2._bank)
    {
        it.second->setup();
    }

    /**
     * GRAPH BANK SERIALIZATION
     */

    // Create a new graph
    gb._bank[9] = new GraphKeyframe({Keyframe(0.0f, 0.13f, 0.0f), Keyframe(1.0f, 0.12f, 0.0f)});

    // Reset buffer
    memset(buffer, 0, 1024);

    // Serialize
    bitsInBuffer = gb.serialize(buffer);
    debug(1, "[setup] [graph bank] serialized %d bits", bitsInBuffer);

    // Create a new graph bank
    GraphBank newGraphBank{};
    newGraphBank = defaultGraphBank();
    debug(1, "[setup] [graph bank] number of graphs in new graph bank before: %d", newGraphBank._bank.size());

    // Deserialize in the new graph bank
    bitsInBuffer = newGraphBank.deserialize(buffer);
    debug(1, "[setup] [graph bank] deserialized %d bits", bitsInBuffer);
    debug(1, "[setup] [graph bank] number of graphs in new graph bank after: %d", newGraphBank._bank.size());

    // Print value at 0.0f and 1.0f
    GraphKeyframe *keyframeGraph = dynamic_cast<GraphKeyframe *>(const_cast<Graph *>(newGraphBank._bank[9]));
    if (keyframeGraph != nullptr)
        debug(1, "[setup] number of keyframes in graph #9: %d, value at 0.0f: %f, value at 1.0f: %f", keyframeGraph->_keyframes.size(), keyframeGraph->valueAt(0.0f), keyframeGraph->valueAt(1.0f));

    /**
     * SCENE SETUP
     */

    // Local scene bank
    SceneBank sbLocal;

    Sequence *hue = new Sequence(
        {gb._bank[DefaultGraph::SINE], 0, 255, 9 * 1000},
        PlaybackMode::REPEAT,
        {61});

    Sequence *brightness = new Sequence(
        {gb._bank[DefaultGraph::SINE], 50, 100, 10 * 1000, 0.5f},
        PlaybackMode::REPEAT);

    LedEffect *pixelEffect = new LedEffect(lb2._bank[0], hue, hue, brightness, brightness);
    Scene *firstScene = new Scene();
    firstScene->_ledEffects.push_back(*pixelEffect);

    sbLocal._scenes.push_back(firstScene);

    // Reset buffer
    memset(buffer, 0, 1024);

    sbLocal.getCurrentScene()->dump();

    // Scene serialization
    bitsInBuffer = sbLocal.serialize(buffer, &lb2, &gb);
    debug(1, "[setup] [scene bank] serialized %d bits", bitsInBuffer);

    // Scene deserialization
    bitsInBuffer = sb.deserialize(buffer, &lb2, &gb);
    debug(1, "[setup] [scene bank] deserialized %d bits", bitsInBuffer);

    sb.getCurrentScene()->dump();

    // Scene setup
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
