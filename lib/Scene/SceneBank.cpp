#include <Utils.h>
#include <SceneBank.h>

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
    const auto &lightIt = ledBank._leds.find(lightId);

    if (lightIt == ledBank._leds.end() || lightIt->second == nullptr)
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
    const auto &graphIt = graphBank._graphs.find(graphId);

    if (graphIt == graphBank._graphs.end() || graphIt->second == nullptr)
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

SceneBank SceneBank::createDummy(const LedBank &ledBank, const GraphBank &graphBank)
{
    SceneBank bank;

    Sequence *hueA;
    Sequence *hueB;
    Sequence *brightnessA;
    Sequence *brightnessB;
    Scene *scene;

    Graph *up = graphBank._graphs.at(DefaultGraph::UP), *sine = graphBank._graphs.at(DefaultGraph::SINE), *gate = graphBank._graphs.at(DefaultGraph::GATE);
    Led *led13 = ledBank._leds.at(13), *led12 = ledBank._leds.at(12), *led666 = ledBank._leds.at(666);

    scene = new Scene();
    hueA = new Sequence(PlaybackMode::ONCE, {up, 0, 127, 500}, 60);
    hueB = new Sequence(PlaybackMode::REPEAT, {up, 70, 255, 5 * 1000});
    brightnessA = new Sequence(PlaybackMode::ONCE, {gate, 255, 0, 500, 1.0f / 10.0f}, 60);
    brightnessB = new Sequence(PlaybackMode::REPEAT, {sine, 0, 255, 1000}, 60);

    scene->_ledEffects[13] = LedEffect(led13, hueA, hueB, brightnessA, brightnessB);
    scene->_ledEffects[12] = LedEffect(led12, hueB, brightnessA, brightnessB, hueA);
    scene->_ledEffects[666] = LedEffect(led666, brightnessA, brightnessB, hueA, hueB);

    bank._scenes[12] = scene;

    scene = new Scene();
    hueA = new Sequence(PlaybackMode::REPEAT, {up, 0, 255, 10 * 1000}, 60);
    hueB = new Sequence(PlaybackMode::REPEAT, {up, 70, 255, 5 * 1000});
    brightnessA = new Sequence(PlaybackMode::REPEAT, {sine, 0, 255, 13 * 1000}, 60);
    brightnessB = new Sequence(PlaybackMode::REPEAT, {sine, 0, 255, 1000});

    scene->_ledEffects[13] = LedEffect(led13, hueA, hueB, brightnessA, brightnessB);
    scene->_ledEffects[12] = LedEffect(led12, hueB, brightnessA, brightnessB, hueA);
    scene->_ledEffects[666] = LedEffect(led666, brightnessA, brightnessB, hueA, hueB);

    bank._scenes[13] = scene;

    return bank;
}