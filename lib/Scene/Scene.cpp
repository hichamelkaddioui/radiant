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

        int hue = _ab * hueB + (1 - _ab) * hueA;
        int brightness = _ab * brightnessB + (1 - _ab) * brightnessA;

        CRGB colorRGB;
        hsv2rgb_rainbow(CHSV(hue, 255, brightness), colorRGB);
        led->setRgb(colorRGB.r, colorRGB.g, colorRGB.b);
    }
}

void Scene::restart()
{
    _ab = 0.0f;

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
        ledEffect.hueA->onNotePlayed(note, velocity, "hue A");
        ledEffect.hueB->onNotePlayed(note, velocity, "hue B");
        ledEffect.brightnessA->onNotePlayed(note, velocity, "brightness A");
        ledEffect.brightnessB->onNotePlayed(note, velocity, "brightness B");
    }
}

size_t Scene::serialize(uint8_t *buffer, const LedBank &ledBank, const GraphBank &graphBank) const
{
    size_t offset = 0;

    size_t ledEffectCount = _ledEffects.size();
    memcpy(buffer + offset, &ledEffectCount, sizeOfSizeT);
    offset += sizeOfSizeT;
    debug(1, "[serialize scene] %lu LED effects", ledEffectCount);

    for (const LedEffect &ledEffect : _ledEffects)
    {
        // Find the led
        int ledId = ledBank.getLedId(ledEffect.led);

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

size_t Scene::deserialize(const uint8_t *buffer, const LedBank &ledBank, const GraphBank &graphBank)
{
    size_t offset = 0;

    size_t ledEffectCount = 0;
    memcpy(&ledEffectCount, buffer + offset, sizeOfSizeT);
    offset += sizeOfSizeT;

    _ledEffects.clear();
    _ledEffects.reserve(ledEffectCount);

    for (size_t i = 0; i < ledEffectCount; i++)
    {
        LedEffect ledEffect;

        int ledId = 0;
        memcpy(&ledId, buffer + offset, sizeOfInt);
        offset += sizeOfInt;

        ledEffect.led = ledBank._bank.at(ledId);

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

        _ledEffects.push_back(ledEffect);
    }

    debug(1, "[deserialize scene] %lu LED effects", _ledEffects.size());

    return offset;
}

void Scene::dump()
{
    debug(1, "[scene] Dumping scene, number of led effects: %d", _ledEffects.size());

    for (LedEffect ledEffect : _ledEffects)
    {
        ledEffect.hueA->dump("hue A");
        ledEffect.hueB->dump("hue B");
        ledEffect.brightnessA->dump("brightness A");
        ledEffect.brightnessB->dump("brightness B");
    }
}

void SceneBank::next()
{
    currentScene = (currentScene + 1) % _scenes.size();

    debug(1, "[scene bank] switching to scene %d", currentScene + 1);

    _scenes[currentScene]->restart();
}

void SceneBank::previous()
{
    currentScene = (currentScene - 1) % _scenes.size();
    _scenes[currentScene]->restart();
}

void SceneBank::restart()
{
    debug(1, "[scene bank] restarting scene bank");

    _scenes[currentScene]->restart();
}

void SceneBank::update()
{
    _scenes[currentScene]->update();
}

Scene *SceneBank::getCurrentScene() const
{
    return _scenes[currentScene];
}

size_t SceneBank::serialize(uint8_t *buffer, const LedBank &ledBank, const GraphBank &graphBank) const
{
    size_t offset = 0, sceneCount = _scenes.size();
    memcpy(buffer + offset, &sceneCount, sizeOfSizeT);
    offset += sizeOfSizeT;
    debug(1, "[serialize scene bank] serializing, %lu scenes in bank", sceneCount);

    for (Scene *scene : _scenes)
    {
        offset += scene->serialize(buffer + offset, ledBank, graphBank);
    }

    return offset;
}

size_t SceneBank::deserialize(const uint8_t *buffer, const LedBank &ledBank, const GraphBank &graphBank)
{
    size_t offset = 0, sceneCount = 0;
    memcpy(&sceneCount, buffer + offset, sizeOfSizeT);
    offset += sizeOfSizeT;
    debug(1, "[deserialize scene bank] deserializing, %lu scenes in bank", sceneCount);

    for (size_t i = 0; i < sceneCount; i++)
    {
        Scene *scene = new Scene();
        offset += scene->deserialize(buffer + offset, ledBank, graphBank);
        _scenes.push_back(scene);
    }

    return offset;
}