#include <vector>
#include <MidiSerial.h>
#include <Led.h>
#include <NeoPixel.h>
#include <Graph.h>
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

    // Led setup
    NeoPixel *pixel = new NeoPixel();
    pixel->setup();
    lb[0] = pixel;

    Sequence *hue = new Sequence(
        {gb._bank[DefaultGraph::SINE], 0, 255, 9 * 1000},
        PlaybackMode::REPEAT,
        {61});

    Sequence *brightness = new Sequence(
        {gb._bank[DefaultGraph::SINE], 50, 100, 10 * 1000, 0.5f},
        PlaybackMode::REPEAT);

    LedEffect *pixelEffect = new LedEffect(lb[0], hue, hue, brightness, brightness);
    Scene *firstScene = new Scene();
    firstScene->_ledEffects.push_back(*pixelEffect);
    firstScene->dump();

    sb._scenes.push_back(firstScene);

    gb._bank[9] = new GraphKeyframe({Keyframe(0.0f, 0.13f, 0.0f), Keyframe(1.0f, 0.12f, 0.0f)});

    uint8_t buffer[1024]{};
    size_t bitsInBuffer = gb.serialize(buffer);
    debug(1, "[setup] serialized %d bits", bitsInBuffer);

    GraphBank gb2{};
    gb2 = defaultGraphBank();
    debug(1, "[setup] Number of graphs in gb2: %d", gb2._bank.size());
    size_t bitsInBuffer2 = gb2.deserialize(buffer);
    debug(1, "[setup] Read %d bits, number of graphs in gb2: %d", bitsInBuffer2, gb2._bank.size());

    GraphKeyframe *keyframeGraph = dynamic_cast<GraphKeyframe *>(const_cast<Graph *>(gb2._bank[9]));

    if (keyframeGraph != nullptr)
        debug(1, "[setup] number of keyframes in gb2[9]: %d, value at 0.0f: %f", keyframeGraph->_keyframes.size(), keyframeGraph->valueAt(0.0f));

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
