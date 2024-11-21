#include <FastLED.h>
#include <Scene.h>
#include <Utils.h>

void Scene::update()
{
    for (LedEffect ledEffect : _ledEffects)
    {
        Led *led = ledEffect.led;

        if (led == nullptr)
        {
            continue;
        }

        if (ledEffect.hueA == nullptr || ledEffect.hueB == nullptr || ledEffect.brightnessA == nullptr || ledEffect.brightnessB == nullptr)
        {
            continue;
        }

        int hueA = ledEffect.hueA->update();
        int hueB = ledEffect.hueB->update();
        int brightnessA = ledEffect.brightnessA->update();
        int brightnessB = ledEffect.brightnessB->update();

        // TODO: Implement convolution between A and B

        CRGB colorRGB;
        hsv2rgb_rainbow(CHSV(hueA, 255, brightnessA), colorRGB);
        led->setRgb(colorRGB.r, colorRGB.g, colorRGB.b);
    }
}

void Scene::restart()
{
    for (LedEffect ledEffect : _ledEffects)
    {
        ledEffect.hueA->restart();
        ledEffect.hueB->restart();
        ledEffect.brightnessA->restart();
        ledEffect.brightnessB->restart();
    }
}

void Scene::onNotePlayed(uint8_t note, uint8_t velocity)
{
    for (LedEffect ledEffect : _ledEffects)
    {
        ledEffect.hueA->onNotePlayed(note, velocity);
        ledEffect.hueB->onNotePlayed(note, velocity);
        ledEffect.brightnessA->onNotePlayed(note, velocity);
        ledEffect.brightnessB->onNotePlayed(note, velocity);
    }
}

void Scene::dump()
{
    debug(1, "[scene] Dumping scene, number of led effects: %d", _ledEffects.size());

    for (LedEffect ledEffect : _ledEffects)
    {
        ledEffect.hueA->dump();
        ledEffect.hueB->dump();
        ledEffect.brightnessA->dump();
        ledEffect.brightnessB->dump();
    }
}

void SceneBank::next()
{
    currentScene = (currentScene + 1) % _scenes.size();
    _scenes[currentScene]->restart();
}

void SceneBank::previous()
{
    currentScene = (currentScene - 1) % _scenes.size();
    _scenes[currentScene]->restart();
}

void SceneBank::restart()
{
    _scenes[currentScene]->restart();
}

void SceneBank::update()
{
    _scenes[currentScene]->update();
}

Scene *SceneBank::getCurrentScene()
{
    return _scenes[currentScene];
}