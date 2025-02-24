#include <Utils.h>
#include <SceneBank.h>

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

void SceneBank::restart()
{
    Scene *currentScene = getCurrentScene();

    if (currentScene == nullptr)
        return;

    debug(1, "[scene bank] restarting scene bank");

    currentScene->restart();
}

void SceneBank::at(byte velocity)
{
    const auto &sceneIt = _scenes.find(velocity);

    if (sceneIt == _scenes.end() || sceneIt->second == nullptr)
    {
        debug(1, "[scene bank] play scene id %d: scene not found", velocity);
        return;
    }

    currentSceneId = velocity;
    sceneIt->second->restart();
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

void SceneBank::update()
{
    Scene *currentScene = getCurrentScene();

    if (currentScene == nullptr)
        return;

    currentScene->update();
}

size_t SceneBank::serialize(uint8_t *buffer, const LedBank &ledBank, const GraphBank &graphBank) const
{
    size_t offset = 0, sceneCount = 0;

    offset += sizeOfSizeT;

    for (const auto &it : _scenes)
    {
        if (it.second->_ledEffects.empty())
            continue;

        memcpy(buffer + offset, &it.first, sizeOfInt);
        offset += sizeOfInt;
        offset += it.second->serialize(buffer + offset, ledBank, graphBank);

        sceneCount++;
    }

    memcpy(buffer, &sceneCount, sizeOfSizeT);
    debug(1, "[serialize scene bank] serialized, %lu scenes in bank", sceneCount);

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

const char *getSceneName(int messageId)
{
    switch (messageId)
    {
    case 5:
        return "hue A";
    case 6:
        return "brightness A";
    case 7:
        return "hue B";
    case 8:
        return "brightness B";
    default:
        return "unknown";
    }
}

void SceneBank::sysExSetHueBrightness(const uint8_t *buffer, size_t length, const LedBank &ledBank, const GraphBank &graphBank)
{
    if (length != 6 && length != 14)
    {
        debug(1, "[SysEx] [scene] invalid length %lu", length);
        return;
    }

    int index = 0;

    int messageId = buffer[index];
    int sceneId = static_cast<int>(buffer[++index]);
    int lightId = static_cast<int>(buffer[++index]);
    PlaybackMode mode = (PlaybackMode)buffer[++index];
    int note = static_cast<int>(buffer[++index]);

    Scene *scene;
    Led *led;
    Sequence *sequence;

    const char *module = getSceneName(messageId);

    const auto &sceneIt = _scenes.find(sceneId);
    if (sceneIt == _scenes.end() || sceneIt->second == nullptr)
    {
        debug(1, "[SysEx] [%s] scene %d not found", module, sceneId);
        return;
    }
    scene = sceneIt->second;

    const auto &lightIt = ledBank._leds.find(lightId);
    if (lightIt == ledBank._leds.end() || lightIt->second == nullptr)
    {
        debug(1, "[SysEx] [%s] light %d not found", module, lightId);
        return;
    }
    led = lightIt->second;

    if (mode < PlaybackMode::ONCE || mode > PlaybackMode::EXTERNAL_CONTROL)
    {
        debug(1, "[SysEx] [%s] scene %d > light %d: invalid mode %d", module, sceneId, lightId, mode);
        return;
    }

    if (mode == PlaybackMode::EXTERNAL_CONTROL)
    {
        sequence = new Sequence(note);
        debug(1, "[SysEx] [%s] scene %d > light %d: note %d, mode %s", module, sceneId, lightId, note, "EXTERNAL_CONTROL");
    }
    else if (mode == PlaybackMode::REPEAT || mode == PlaybackMode::ONCE)
    {
        int graphId = static_cast<int>(buffer[++index]);
        int min = static_cast<int>(buffer[++index]) * 2;
        int max = static_cast<int>(buffer[++index]) * 2;
        unsigned long duration = threeBytesToLong(buffer[++index], buffer[++index], buffer[++index]);
        float period = twoBytesToFloat(buffer[++index], buffer[++index]);

        const auto &graphIt = graphBank._graphs.find(graphId);
        if (graphIt == graphBank._graphs.end() || graphIt->second == nullptr)
        {
            debug(1, "[SysEx] [%s] graph %d not found", module, graphId);
            return;
        }

        sequence = new Sequence(mode, {graphIt->second, min, max, duration, period}, note);
        debug(1, "[SysEx] [%s] scene %d > light %d: note %d, mode %d, graph %d, min %d, max %d, duration %lu, period %0.2f", module, sceneId, lightId, note, mode, graphId, min, max, duration, period);
    }

    scene->sysExSetHueBrightness(messageId, lightId, led, sequence);
    scene->dump();
}

SceneBank SceneBank::createDummy(const LedBank &ledBank, const GraphBank &graphBank)
{
    SceneBank bank;

    Sequence *hueA;
    Sequence *hueB;
    Sequence *brightnessA;
    Sequence *brightnessB;
    Scene *scene;

    Graph *constant = graphBank._graphs.at(DefaultGraph::CONSTANT),
          *up = graphBank._graphs.at(DefaultGraph::UP),
          *upExp = graphBank._graphs.at(DefaultGraph::UP_EXP),
          *down = graphBank._graphs.at(DefaultGraph::DOWN),
          *sine = graphBank._graphs.at(DefaultGraph::SINE),
          *gate = graphBank._graphs.at(DefaultGraph::GATE);
    Led *left = ledBank._leds.at(0),
        *right = ledBank._leds.at(1),
        *neoPixel = ledBank._leds.at(2);

    /** Scene 1 */

    scene = new Scene();
    hueA = new Sequence(PlaybackMode::REPEAT, {constant, UTILS_HUE_ORANGE, UTILS_HUE_ORANGE, 5 * 1000}, 60);
    brightnessA = new Sequence(PlaybackMode::ONCE, {up, 255, 0, 500}, 60);
    hueB = new Sequence(PlaybackMode::REPEAT, {constant, UTILS_HUE_RED, UTILS_HUE_RED, 5 * 1000}, 60);
    brightnessB = new Sequence(PlaybackMode::ONCE, {up, 0, 255, 500}, 60);

    scene->_ledEffects[0] = LedEffect(left, hueA, hueB, brightnessA, brightnessB);
    scene->_ledEffects[1] = LedEffect(right, hueA, hueB, brightnessA, brightnessB);
    scene->_ledEffects[2] = LedEffect(neoPixel, hueA, hueB, brightnessA, brightnessB);

    bank._scenes[1] = scene;

    /** Scene 2 */

    scene = new Scene();
    hueA = new Sequence(PlaybackMode::REPEAT, {constant, UTILS_HUE_AQUA, UTILS_HUE_AQUA, 2000});
    brightnessA = new Sequence(PlaybackMode::ONCE, {gate, 255, 0, 400, 1.0f / 10.0f}, 60);
    hueB = new Sequence(PlaybackMode::REPEAT, {constant, UTILS_HUE_RED, UTILS_HUE_RED, 5 * 1000});
    brightnessB = new Sequence(PlaybackMode::ONCE, {up, 0, 255, 500}, 60);

    scene->_ledEffects[0] = LedEffect(left, hueA, hueB, brightnessA, brightnessB);
    scene->_ledEffects[1] = LedEffect(right, hueA, hueB, brightnessA, brightnessB);
    scene->_ledEffects[2] = LedEffect(neoPixel, hueA, hueB, brightnessA, brightnessB);

    bank._scenes[2] = scene;

    /** Scene 3 */

    scene = new Scene();
    hueA = new Sequence(PlaybackMode::ONCE, {up, UTILS_HUE_AQUA, UTILS_HUE_INDIGO, 300}, 62);
    brightnessA = new Sequence(PlaybackMode::ONCE, {gate, 255, 0, 400, 1.0f / 10.0f}, 60);
    hueB = new Sequence(PlaybackMode::ONCE, {up, UTILS_HUE_RED, UTILS_HUE_SOFT_GREEN, 100}, 62);
    brightnessB = new Sequence(PlaybackMode::ONCE, {up, 0, 255, 500}, 60);

    scene->_ledEffects[0] = LedEffect(left, hueA, hueB, brightnessA, brightnessB);
    scene->_ledEffects[1] = LedEffect(right, hueA, hueB, brightnessA, brightnessB);
    scene->_ledEffects[2] = LedEffect(neoPixel, hueA, hueB, brightnessA, brightnessB);

    bank._scenes[3] = scene;

    /** Scene 4 */

    scene = new Scene();
    // Slow sine turned off by kick
    hueB = new Sequence(PlaybackMode::REPEAT, {sine, UTILS_HUE_PEACH, UTILS_HUE_SOFT_GREEN, 10 * 1000});
    brightnessB = new Sequence(PlaybackMode::ONCE, {up, 0, 255, 500}, 60);

    // Strobe-ish
    hueA = new Sequence(PlaybackMode::ONCE, {constant, UTILS_HUE_LIGHT_BLUE, UTILS_HUE_LIGHT_BLUE, 5000});
    brightnessA = new Sequence(PlaybackMode::ONCE, {gate, 255, 0, 400, 1.0f / 10.0f}, 0x42);
    scene->_ledEffects[0] = LedEffect(left, hueA, hueB, brightnessA, brightnessB);

    // Strobe-ish
    hueA = new Sequence(PlaybackMode::ONCE, {constant, UTILS_HUE_LIGHT_BLUE, UTILS_HUE_LIGHT_BLUE, 5000});
    brightnessA = new Sequence(PlaybackMode::ONCE, {gate, 255, 0, 400, 1.0f / 10.0f}, 0x3f);
    scene->_ledEffects[1] = LedEffect(left, hueA, hueB, brightnessA, brightnessB);

    scene->_ledEffects[2] = LedEffect(neoPixel, hueA, hueB, brightnessA, brightnessB);

    bank._scenes[4] = scene;

    /** Scene 5 */

    scene = new Scene();
    // Slow sine turned off by kick
    hueB = new Sequence(PlaybackMode::REPEAT, {sine, UTILS_HUE_INDIGO, UTILS_HUE_SOFT_GREEN, 10 * 1000});
    brightnessB = new Sequence(PlaybackMode::ONCE, {up, 0, 255, 500}, 60);

    // Strobe-ish
    hueA = new Sequence(PlaybackMode::ONCE, {constant, UTILS_HUE_LIGHT_BLUE, UTILS_HUE_LIGHT_BLUE, 5000});
    brightnessA = new Sequence(PlaybackMode::ONCE, {gate, 255, 0, 200, 1.0f / 5.0f}, 0x41);
    scene->_ledEffects[0] = LedEffect(left, hueA, hueB, brightnessA, brightnessB);

    // Strobe-ish
    hueA = new Sequence(PlaybackMode::ONCE, {constant, UTILS_HUE_LIGHT_BLUE, UTILS_HUE_LIGHT_BLUE, 5000});
    brightnessA = new Sequence(PlaybackMode::ONCE, {gate, 255, 0, 200, 1.0f / 5.0f}, 0x45);
    scene->_ledEffects[1] = LedEffect(left, hueA, hueB, brightnessA, brightnessB);

    scene->_ledEffects[2] = LedEffect(neoPixel, hueA, hueB, brightnessA, brightnessB);

    bank._scenes[5] = scene;

    /** Scene 6 */

    scene = new Scene();
    hueA = new Sequence(PlaybackMode::REPEAT, {sine, UTILS_HUE_BLUE, UTILS_HUE_TEAL, 5 * 1000}, 60);
    brightnessA = new Sequence(PlaybackMode::ONCE, {up, 255, 0, 150}, 0x3c);
    hueB = new Sequence(PlaybackMode::REPEAT, {sine, UTILS_HUE_BLUE, UTILS_HUE_TEAL, 5 * 1000}, 60);
    brightnessB = new Sequence(PlaybackMode::ONCE, {up, 0, 255, 150}, 0x45);

    scene->_ledEffects[0] = LedEffect(left, hueA, hueB, brightnessA, brightnessB);
    scene->_ledEffects[1] = LedEffect(right, hueA, hueB, brightnessA, brightnessB);
    scene->_ledEffects[2] = LedEffect(neoPixel, hueA, hueB, brightnessA, brightnessB);

    bank._scenes[6] = scene;

    return bank;
}