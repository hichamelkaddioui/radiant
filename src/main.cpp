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

    // Scene setup
    debug(1, "[setup] Setup complete");
}

void loop()
{
    Scene *currentScene = sb.getCurrentScene();
    midiSerial.loop(currentScene);
}

bool dumped = false;

void loop1()
{
    Scene *currentScene = sb.getCurrentScene();

    if (currentScene == nullptr)
    {
        debug(23, "[loop] No scene found");
        return;
    }

    currentScene->update();

    if (!dumped)
    {
        currentScene->dump();
        dumped = true;
    }

    screen.displaySceneData((*currentScene));
    screen.loop();

    debug(23, "[loop] Current scene elapsed: %d", currentScene->_ledEffects[0].brightnessA->elapsed());
}
