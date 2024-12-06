#include <FastLED.h>
#include <Scene.h>
#include <Utils.h>

Scene::~Scene()
{
    for (const auto &it : _ledEffects)
    {
        LedEffect ledEffect = it.second;

        delete ledEffect.hueA;
        delete ledEffect.hueB;
        delete ledEffect.brightnessA;
        delete ledEffect.brightnessB;
    }

    _ledEffects.clear();
}

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

        if (ledBank._bank.find(ledId) == ledBank._bank.end())
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

        ledEffect.led = ledBank._bank.at(ledId);

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

void SceneBank::next()
{
    const auto &next = std::next(_scenes.find(currentSceneId));

    if (next == _scenes.end())
    {
        currentSceneId = _scenes.begin()->first;
        _scenes.begin()->second->restart();

        return;
    }

    currentSceneId = next->first;
    next->second->restart();
}

void SceneBank::restart()
{
    Scene *currentScene = getCurrentScene();

    if (currentScene == nullptr)
        return;

    debug(1, "[scene bank] restarting scene bank");

    currentScene->restart();
}

void SceneBank::update()
{
    Scene *currentScene = getCurrentScene();

    if (currentScene == nullptr)
        return;

    currentScene->update();
}

Scene *SceneBank::getCurrentScene() const
{
    const auto &it = _scenes.find(currentSceneId);

    if (it == _scenes.end())
    {
        debug(1, "[scene bank] no scene found");

        return _scenes.begin()->second;
    }

    return it->second;
}

size_t SceneBank::serialize(uint8_t *buffer, const LedBank &ledBank, const GraphBank &graphBank) const
{
    size_t offset = 0, sceneCount = _scenes.size();
    memcpy(buffer + offset, &sceneCount, sizeOfSizeT);
    offset += sizeOfSizeT;
    debug(1, "[serialize scene bank] serializing, %lu scenes in bank", sceneCount);

    for (const auto &it : _scenes)
    {
        memcpy(buffer + offset, &it.first, sizeOfInt);
        offset += sizeOfInt;
        offset += it.second->serialize(buffer + offset, ledBank, graphBank);
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
        int id = 0;
        memcpy(&id, buffer + offset, sizeOfInt);
        offset += sizeOfInt;

        Scene *scene = new Scene();
        offset += scene->deserialize(buffer + offset, ledBank, graphBank);

        debug(1, "[deserialized scene] id: %d", id);

        _scenes[id] = scene;
    }

    currentSceneId = _scenes.begin()->first;

    return offset;
}

void SceneBank::sysExCreate(const uint8_t *buffer, size_t length)
{
    debug(1, "[SysEx] [scene] reading %lu bytes", length);

    int sceneId = buffer[0];

    debug(1, "[SysEx] [scene] scene id is %d", sceneId);

    Scene *scene = new Scene();

    const auto it = _scenes.find(sceneId);

    if (it != _scenes.end())
    {
        delete it->second;

        debug(1, "[SysEx] [scene] deleted previous scene with id %d", sceneId);
    }
    else
    {
        debug(1, "[SysEx] [scene] no previous scene with id %d", sceneId);
    }

    _scenes[sceneId] = scene;
}

void SceneBank::sysExSetHueBrightness(const uint8_t *buffer, size_t length, const LedBank &ledBank, const GraphBank &graphBank)
{
    int index = 0;

    int messageId = buffer[index];
    String parameter = messageId % 2 == 1 ? "hue" : "brightness";
    String ab = messageId < 7 ? "A" : "B";
    String moduleString = parameter + " " + ab;
    const char *module = moduleString.c_str();

    int sceneId = static_cast<int>(buffer[++index]);
    const auto &sceneIt = _scenes.find(sceneId);

    if (sceneIt == _scenes.end() || sceneIt->second == nullptr)
    {
        debug(1, "[SysEx] [%s] scene %d not found", module, sceneId);
        return;
    }

    int lightId = static_cast<int>(buffer[++index]);
    const auto &lightIt = ledBank._bank.find(lightId);

    if (lightIt == ledBank._bank.end() || lightIt->second == nullptr)
    {
        debug(1, "[SysEx] [%s] light %d not found", module, lightId);
        return;
    }

    PlaybackMode mode = (PlaybackMode)buffer[++index];
    int note = static_cast<int>(buffer[++index]);

    if (mode == PlaybackMode::EXTERNAL_CONTROL)
    {
        Sequence *sequence = new Sequence(note);
        sceneIt->second->sysExSetHueBrightness(messageId, lightId, sequence);

        debug(1, "[SysEx] [%s] scene %d > light %d: note %d, mode %d", module, sceneId, lightId, note, mode);

        return;
    }

    int graphId = static_cast<int>(buffer[++index]);
    const auto &graphIt = graphBank._bank.find(graphId);

    if (graphIt == graphBank._bank.end() || graphIt->second == nullptr)
    {
        debug(1, "[SysEx] [%s] graph %d not found", module, graphId);
        return;
    }

    int min = static_cast<int>(buffer[++index]) * 2;
    int max = static_cast<int>(buffer[++index]) * 2;
    unsigned long duration = threeBytesToLong(buffer[++index], buffer[++index], buffer[++index]);
    float period = twoBytesToFloat(buffer[++index], buffer[++index]);

    Sequence *sequence = new Sequence(mode, {graphIt->second, min, max, duration, period}, note);
    sceneIt->second->sysExSetHueBrightness(messageId, lightId, sequence);

    debug(1, "[SysEx] [%s] scene %d > light %d: note %d, mode %d, graph %d, min %d, max %d, duration %lu, period %0.2f", module, sceneId, lightId, note, mode, graphId, min, max, duration, period);
}