#include <Arduino.h>
#include <Scene.h>
#include <Neopixel.h>

size_t serializeKeyframe(const Keyframe keyframe, uint8_t *buffer)
{
    size_t offset = 0;
    memcpy(buffer + offset, &keyframe.time, sizeof(unsigned long));
    offset += sizeof(unsigned long);
    memcpy(buffer + offset, &keyframe.value, sizeof(float));
    offset += sizeof(float);
    memcpy(buffer + offset, &keyframe.curve.type, sizeof(CurveType));
    offset += sizeof(CurveType);

    // Serialize curve coefficients based on the type
    switch (keyframe.curve.type)
    {
    case EASE:
        memcpy(buffer + offset, &keyframe.curve.coefficient.powerValue, sizeof(float));
        offset += sizeof(float);
        break;
    case WAVE:
    case GATE:
        memcpy(buffer + offset, &keyframe.curve.coefficient.rangeAndPeriod.min, sizeof(float));
        offset += sizeof(float);
        memcpy(buffer + offset, &keyframe.curve.coefficient.rangeAndPeriod.max, sizeof(float));
        offset += sizeof(float);
        memcpy(buffer + offset, &keyframe.curve.coefficient.rangeAndPeriod.period, sizeof(unsigned long));
        offset += sizeof(unsigned long);
        break;
    case LINEAR:
        // No additional data for linear
        break;
    }

    return offset;
}

size_t serializeKeyframes(const std::vector<Keyframe> keyframes, uint8_t *buffer)
{
    size_t offset, i = 0;

    // Serialize number of keyframes
    size_t keyframeCount = keyframes.size();
    memcpy(buffer, &keyframeCount, sizeof(size_t));
    offset += sizeof(size_t);

    for (const Keyframe &keyframe : keyframes)
    {
        offset += serializeKeyframe(keyframe, buffer + offset);

        if (keyframe.curve.type == CurveType::EASE)
            debug(1, "[serialize keyframe] (%2d/%2d)\t%5lu\t%12f\t%s\tpw=%f", i + 1, keyframeCount, keyframe.time, keyframe.value, "ease", keyframe.curve.coefficient.powerValue);
        if (keyframe.curve.type == CurveType::WAVE)
            debug(1, "[serialize keyframe] (%2d/%2d)\t%5lu\t%12f\t%s\tmin=%f\tmax=%f\tperiod=%lu", i + 1, keyframeCount, keyframe.time, keyframe.value, "wave", keyframe.curve.coefficient.rangeAndPeriod.min, keyframe.curve.coefficient.rangeAndPeriod.max, keyframe.curve.coefficient.rangeAndPeriod.period);
        if (keyframe.curve.type == CurveType::GATE)
            debug(1, "[serialize keyframe] (%2d/%2d)\t%5lu\t%12f\t%s\tmin=%f\tmax=%f\tperiod=%lu", i + 1, keyframeCount, keyframe.time, keyframe.value, "gate", keyframe.curve.coefficient.rangeAndPeriod.min, keyframe.curve.coefficient.rangeAndPeriod.max, keyframe.curve.coefficient.rangeAndPeriod.period);
        if (keyframe.curve.type == CurveType::LINEAR)
            debug(1, "[serialize keyframe] (%2d/%2d)\t%5lu\t%12f\t%s", i + 1, keyframeCount, keyframe.time, keyframe.value, "linear");

        i++;
    }

    return offset;
}

size_t serializeScene(const Scene &scene, uint8_t *buffer)
{
    size_t offset = 0;
    offset += sizeof(size_t); // Reserve space for the scene size

    SceneMode mode = scene.getMode();
    std::vector<Keyframe> keyframes = scene.getKeyframes();

    debug(1, "[serialize scene] mode: %s, %lu keyframes", mode == SceneMode::LOOP ? "LOOP" : "TRIGGER", keyframes.size());

    // Serialize mode
    memcpy(buffer + offset, &mode, sizeof(SceneMode));
    offset += sizeof(SceneMode);

    // Serialize each keyframe
    offset += serializeKeyframes(keyframes, buffer + offset);

    // Store the total scene size at the beginning of the buffer
    memcpy(buffer, &offset, sizeof(size_t));

    debug(1, "[serialize scene] scene size: %lu", offset);

    // Return the total size of the serialized data
    return offset;
}

size_t deserializeKeyframe(Keyframe &keyframe, const uint8_t *buffer)
{
    size_t offset = 0;
    memcpy(&keyframe.time, buffer + offset, sizeof(unsigned long));
    offset += sizeof(unsigned long);
    memcpy(&keyframe.value, buffer + offset, sizeof(float));
    offset += sizeof(float);
    memcpy(&keyframe.curve.type, buffer + offset, sizeof(CurveType));
    offset += sizeof(CurveType);

    // Deserialize curve coefficients based on the type
    switch (keyframe.curve.type)
    {
    case EASE:
        memcpy(&keyframe.curve.coefficient.powerValue, buffer + offset, sizeof(float));
        offset += sizeof(float);
        break;
    case WAVE:
    case GATE:
        memcpy(&keyframe.curve.coefficient.rangeAndPeriod.min, buffer + offset, sizeof(float));
        offset += sizeof(float);
        memcpy(&keyframe.curve.coefficient.rangeAndPeriod.max, buffer + offset, sizeof(float));
        offset += sizeof(float);
        memcpy(&keyframe.curve.coefficient.rangeAndPeriod.period, buffer + offset, sizeof(unsigned long));
        offset += sizeof(unsigned long);
        break;
    case LINEAR:
        // No additional data for linear
        break;
    }

    return offset;
}

size_t deserializeKeyframes(std::vector<Keyframe> &keyframes, const uint8_t *buffer)
{
    size_t offset = 0;
    size_t keyframeCount = 0;
    memcpy(&keyframeCount, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);

    debug(1, "[deserialize scene] %lu keyframes", keyframeCount);

    keyframes.reserve(keyframeCount);
    keyframes.clear();

    for (size_t i = 0; i < keyframeCount; i++)
    {
        Keyframe keyframe;
        offset += deserializeKeyframe(keyframe, buffer + offset);

        if (keyframe.curve.type == CurveType::EASE)
            debug(1, "[deserialize keyframe] (%2d/%2d)\t%5lu\t%12f\t%s\tpw=%f", i + 1, keyframeCount, keyframe.time, keyframe.value, "ease", keyframe.curve.coefficient.powerValue);
        if (keyframe.curve.type == CurveType::WAVE)
            debug(1, "[deserialize keyframe] (%2d/%2d)\t%5lu\t%12f\t%s\tmin=%f\tmax=%f\tperiod=%lu", i + 1, keyframeCount, keyframe.time, keyframe.value, "wave", keyframe.curve.coefficient.rangeAndPeriod.min, keyframe.curve.coefficient.rangeAndPeriod.max, keyframe.curve.coefficient.rangeAndPeriod.period);
        if (keyframe.curve.type == CurveType::GATE)
            debug(1, "[deserialize keyframe] (%2d/%2d)\t%5lu\t%12f\t%s\tmin=%f\tmax=%f\tperiod=%lu", i + 1, keyframeCount, keyframe.time, keyframe.value, "gate", keyframe.curve.coefficient.rangeAndPeriod.min, keyframe.curve.coefficient.rangeAndPeriod.max, keyframe.curve.coefficient.rangeAndPeriod.period);
        if (keyframe.curve.type == CurveType::LINEAR)
            debug(1, "[deserialize keyframe] (%2d/%2d)\t%5lu\t%12f\t%s", i + 1, keyframeCount, keyframe.time, keyframe.value, "linear");

        keyframes.push_back(keyframe);
    }

    return offset;
}

size_t deserializeScene(Scene &scene, const uint8_t *buffer)
{
    size_t offset = 0;
    size_t sceneSize;
    memcpy(&sceneSize, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);

    debug(1, "[deserialize scene] scene size: %lu", sceneSize);

    // Deserialize mode
    memcpy(&scene._mode, buffer + offset, sizeof(SceneMode));
    offset += sizeof(SceneMode);

    debug(1, "[deserialize scene] mode: %s", scene._mode == SceneMode::LOOP ? "LOOP" : "TRIGGER");

    // Deserialize number of keyframes
    offset += deserializeKeyframes(scene._keyframes, buffer + offset);

    return sceneSize;
}

size_t serializeNeoPixel(const NeoPixel &pixel, uint8_t *buffer)
{
    // Reserve space for the pixel size
    size_t offset = 0;
    offset += sizeof(size_t);

    Scene hueScene = pixel._hueScene;
    Scene satScene = pixel._saturationScene;
    Scene valScene = pixel._valueScene;

    debug(1, "[serialize pixel] scene hue, offset %lu", offset);
    offset += serializeScene(hueScene, buffer + offset);

    debug(1, "[serialize pixel] scene sat, offset %lu", offset);
    offset += serializeScene(satScene, buffer + offset);

    debug(1, "[serialize pixel] scene val, offset %lu", offset);
    offset += serializeScene(valScene, buffer + offset);

    // Store the total pixel size at the beginning of the buffer
    memcpy(buffer, &offset, sizeof(size_t));

    // Return the total size of the serialized data
    return offset;
}

size_t deserializeNeoPixel(NeoPixel &pixel, const uint8_t *buffer)
{
    size_t offset = 0;
    size_t pixelSize;
    memcpy(&pixelSize, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);

    debug(1, "[deserialize pixel] pixel size: %lu", pixelSize);

    debug(1, "[deserialize pixel] Starting hue scene at offset: %lu", offset);
    size_t hueSize = deserializeScene(pixel._hueScene, buffer + offset);
    debug(1, "[deserialize pixel] Hue scene size: %lu", hueSize);
    offset += hueSize;

    debug(1, "[deserialize pixel] Starting sat scene at offset: %lu", offset);
    size_t satSize = deserializeScene(pixel._saturationScene, buffer + offset);
    debug(1, "[deserialize pixel] Sat scene size: %lu", satSize);
    offset += satSize;

    debug(1, "[deserialize pixel] Starting val scene at offset: %lu", offset);
    size_t valSize = deserializeScene(pixel._valueScene, buffer + offset);
    debug(1, "[deserialize pixel] Val scene size: %lu", valSize);
    offset += valSize;

    return pixelSize;
}

size_t serializeSceneMap(const SceneMap &scenesMap, uint8_t *buffer)
{
    size_t offset = 0;

    // Reserve space for the number of scenes
    size_t sceneCount = scenesMap.size();
    memcpy(buffer + offset, &sceneCount, sizeof(size_t));
    offset += sizeof(size_t);
    debug(1, "[serialize scene map] %lu scenes", sceneCount);

    // Serialize each scene
    for (const auto &it : scenesMap)
    {
        debug(1, "[serialize scene map item] %d", it.first);
        memcpy(buffer + offset, &it.first, sizeof(int));
        offset += sizeof(int);
        offset += serializeScene(it.second, buffer + offset);
    }

    return offset;
}

size_t deserializeSceneMap(SceneMap &scenesMap, const uint8_t *buffer)
{
    size_t offset = 0;
    size_t sceneCount;
    memcpy(&sceneCount, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);
    debug(1, "[deserialize scene map] %lu scenes", sceneCount);

    for (size_t i = 0; i < sceneCount; i++)
    {
        int id;
        memcpy(&id, buffer + offset, sizeof(int));
        offset += sizeof(int);

        debug(1, "[deserialize scene item] %d", id);
        offset += deserializeScene(scenesMap[id], buffer + offset);
    }

    return offset;
}
