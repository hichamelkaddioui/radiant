#include <vector>
#include <MidiSerial.h>
#include <Led.h>
#include <NeoPixel.h>
#include <Graph.h>
#include <Serialize.h>
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

    while (!Serial)
        ;

    // MIDI Serial
    midiSerial.setup();
}

void setup1()
{
    // OLED screen
    screen.setup();

    // Led setup
    NeoPixel *pixel = new NeoPixel();
    pixel->setup();
    lb[0] = pixel;

    Sequence *hue = new Sequence(gb[DefaultGraph::UP_EXP], UTILS_HUE_AQUA, UTILS_HUE_GREEN, 1000, PlaybackMode::REPEAT, 1.0f, 61);
    Sequence *brightness = new Sequence(gb[DefaultGraph::UP], 0, 255, 10 * 1000, PlaybackMode::ONCE, 1.0f, 60);

    LedEffect *pixelEffect = new LedEffect(lb[0], hue, hue, brightness, brightness);

    Scene *firstScene = new Scene();
    firstScene->_ledEffects.push_back(*pixelEffect);
    sb._scenes.push_back(firstScene);

    firstScene->dump();

    gb[9] = new GraphKeyframe({Keyframe(0.0f, 0.13f, 0.0f), Keyframe(1.0f, 0.12f, 0.0f)});

    uint8_t buffer[1024]{};
    serializeGraphBank(gb, buffer);

    GraphBank gb2{};
    deserializeGraphBank(gb2, buffer);

    GraphKeyframe *keyframeGraph = dynamic_cast<GraphKeyframe *>(const_cast<Graph *>(gb2[9]));

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
