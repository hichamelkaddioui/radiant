#include <FastLED.h>
#include <Scene.h>
#include <Utils.h>

#ifndef SCENE_DEBUG
#define SCENE_DEBUG 0
#endif

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

size_t serializeScene(Scene *scene, uint8_t *buffer, LedBank *ledBank, GraphBank *graphBank)
{
    size_t offset = 0;

    size_t ledEffectCount = scene->_ledEffects.size();
    memcpy(buffer + offset, &ledEffectCount, sizeOfSizeT);
    offset += sizeOfSizeT;
    debug(1, "[serialize scene] %lu LED effects", ledEffectCount);

    for (const LedEffect &ledEffect : scene->_ledEffects)
    {
        // Find the led
        int ledId = ledBank->getLedId(ledEffect.led);

        if (ledId == -1)
        {
            debug(1, "[serialize scene] led not found");
            continue;
        }

        memcpy(buffer + offset, &ledId, sizeOfInt);
        offset += sizeOfInt;

        offset += ledEffect.hueA->serialize(buffer + offset, graphBank);
        offset += ledEffect.hueB->serialize(buffer + offset, graphBank);
        offset += ledEffect.brightnessA->serialize(buffer + offset, graphBank);
        offset += ledEffect.brightnessB->serialize(buffer + offset, graphBank);
    }

    return offset;
}

size_t deserializeScene(Scene &scene, const uint8_t *buffer, LedBank *ledBank, GraphBank *graphBank)
{
    size_t offset = 0;

    size_t ledEffectCount = 0;
    memcpy(&ledEffectCount, buffer + offset, sizeOfSizeT);
    offset += sizeOfSizeT;
    debug(1, "[deserialize scene] %lu LED effects", ledEffectCount);

    scene._ledEffects.reserve(ledEffectCount);
    scene._ledEffects.clear();

    for (size_t i = 0; i < ledEffectCount; i++)
    {
        LedEffect ledEffect;

        int ledId = 0;
        memcpy(&ledId, buffer + offset, sizeOfInt);
        offset += sizeOfInt;

        ledEffect.led = ledBank->_bank.at(ledId);

        if (ledEffect.led == nullptr)
        {
            debug(1, "[deserialize scene] led not found");
            continue;
        }

        ledEffect.hueA = new Sequence();
        ledEffect.hueB = new Sequence();
        ledEffect.brightnessA = new Sequence();
        ledEffect.brightnessB = new Sequence();

        offset += ledEffect.hueA->deserialize(buffer + offset, graphBank);
        offset += ledEffect.hueB->deserialize(buffer + offset, graphBank);
        offset += ledEffect.brightnessA->deserialize(buffer + offset, graphBank);
        offset += ledEffect.brightnessB->deserialize(buffer + offset, graphBank);

        scene._ledEffects.push_back(ledEffect);
    }

    return offset;
}

size_t SceneBank::serialize(uint8_t *buffer, LedBank *ledBank, GraphBank *graphBank)
{
    size_t offset = 0, sceneCount = _scenes.size();
    memcpy(buffer + offset, &sceneCount, sizeOfSizeT);
    offset += sizeOfSizeT;
    debug(1, "[serialize scene bank] serializing, %lu scenes in bank", sceneCount);

    for (Scene *scene : _scenes)
    {
        offset += serializeScene(scene, buffer + offset, ledBank, graphBank);
    }

    return offset;
}

size_t SceneBank::deserialize(const uint8_t *buffer, LedBank *ledBank, GraphBank *graphBank)
{
    size_t offset = 0, sceneCount = 0;
    memcpy(&sceneCount, buffer + offset, sizeOfSizeT);
    offset += sizeOfSizeT;
    debug(1, "[deserialize scene bank] deserializing, %lu scenes in bank", sceneCount);

    for (size_t i = 0; i < sceneCount; i++)
    {
        Scene *scene = new Scene();
        offset += deserializeScene(*scene, buffer + offset, ledBank, graphBank);
        _scenes.push_back(scene);
    }

    return offset;
}