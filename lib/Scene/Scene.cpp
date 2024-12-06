#include <FastLED.h>
#include <Scene.h>
#include <Utils.h>

void Scene::update()
{
    for (const auto &it : _ledEffects)
    {
        LedEffect ledEffect = it.second;
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

    for (const auto &it : _ledEffects)
    {
        LedEffect ledEffect = it.second;
        ledEffect.hueA->restart();
        ledEffect.hueB->restart();
        ledEffect.brightnessA->restart();
        ledEffect.brightnessB->restart();
    }
}

void Scene::onNotePlayed(uint8_t note, uint8_t velocity)
{
    for (const auto &it : _ledEffects)
    {
        LedEffect ledEffect = it.second;
        ledEffect.hueA->onNotePlayed(note, velocity, "hue A");
        ledEffect.hueB->onNotePlayed(note, velocity, "hue B");
        ledEffect.brightnessA->onNotePlayed(note, velocity, "brightness A");
        ledEffect.brightnessB->onNotePlayed(note, velocity, "brightness B");
    }
}

size_t Scene::serialize(uint8_t *buffer, const LedBank &ledBank, const GraphBank &graphBank) const
{
    size_t offset = 0, ledEffectCount = _ledEffects.size();
    memcpy(buffer + offset, &ledEffectCount, sizeOfSizeT);
    offset += sizeOfSizeT;
    debug(1, "[serialize scene] %lu LED effects", ledEffectCount);

    for (const auto &it : _ledEffects)
    {
        int ledId = it.first;

        if (ledBank._leds.find(ledId) == ledBank._leds.end())
        {
            debug(1, "[serialize scene] led id: %d not found, skipping led effect", ledId);
            continue;
        }

        memcpy(buffer + offset, &ledId, sizeOfInt);
        offset += sizeOfInt;

        LedEffect ledEffect = it.second;
        offset += ledEffect.hueA->serialize(buffer + offset, graphBank);
        offset += ledEffect.hueB->serialize(buffer + offset, graphBank);
        offset += ledEffect.brightnessA->serialize(buffer + offset, graphBank);
        offset += ledEffect.brightnessB->serialize(buffer + offset, graphBank);
    }

    return offset;
}

size_t Scene::deserialize(const uint8_t *buffer, const LedBank &ledBank, const GraphBank &graphBank)
{
    size_t offset = 0, ledEffectCount = 0;
    memcpy(&ledEffectCount, buffer + offset, sizeOfSizeT);
    offset += sizeOfSizeT;

    _ledEffects.clear();

    for (size_t i = 0; i < ledEffectCount; i++)
    {
        LedEffect ledEffect;

        int ledId = 0;
        memcpy(&ledId, buffer + offset, sizeOfInt);
        offset += sizeOfInt;

        ledEffect.led = ledBank._leds.at(ledId);

        if (ledEffect.led == nullptr)
        {
            debug(1, "[deserialize scene] led %d not found, skipping led effect", ledId);
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

        _ledEffects[ledId] = ledEffect;
    }

    debug(1, "[deserialize scene] %lu LED effects", _ledEffects.size());

    return offset;
}

void Scene::sysExSetHueBrightness(int messageId, int lightId, Sequence *sequence)
{
    LedEffect ledEffect = LedEffect();

    if (_ledEffects.find(lightId) != _ledEffects.end())
        ledEffect = _ledEffects[lightId];

    switch (messageId)
    {
    case 5:
        ledEffect.hueA = sequence;
        break;
    case 6:
        ledEffect.brightnessA = sequence;
        break;
    case 7:
        ledEffect.hueB = sequence;
        break;
    case 8:
        ledEffect.brightnessB = sequence;
        break;
    default:
        break;
    }

    _ledEffects[lightId] = ledEffect;
}

void Scene::dump()
{
    debug(1, "[scene] Dumping scene, number of led effects: %d", _ledEffects.size());

    for (const auto &it : _ledEffects)
    {
        LedEffect ledEffect = it.second;
        ledEffect.hueA->dump("hue A");
        ledEffect.hueB->dump("hue B");
        ledEffect.brightnessA->dump("brightness A");
        ledEffect.brightnessB->dump("brightness B");
    }
}
