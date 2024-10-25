#include <Arduino.h>
#include <Scene.h>

size_t serializeScene(Scene scene, uint8_t *buffer)
{
    size_t offset = 0;
    size_t sceneSizeOffset = offset;
    offset += sizeof(size_t); // Reserve space for the scene size

    SceneMode mode = scene.getMode();
    std::vector<Keyframe> keyframes = scene.getKeyframes();

    // Serialize mode
    memcpy(buffer + offset, &mode, sizeof(SceneMode));
    offset += sizeof(SceneMode);

    // Serialize number of keyframes
    size_t keyframeCount = keyframes.size();
    memcpy(buffer + offset, &keyframeCount, sizeof(size_t));
    offset += sizeof(size_t);

    // Serialize each keyframe
    for (const Keyframe &keyframe : keyframes)
    {
        memcpy(buffer + offset, &keyframe.time, sizeof(unsigned long));
        offset += sizeof(unsigned long);
        memcpy(buffer + offset, &keyframe.value, sizeof(float));
        offset += sizeof(float);
        memcpy(buffer + offset, &keyframe.curve.type, sizeof(CurveType));
        offset += sizeof(CurveType);

        // Serialize curve coefficients based on the type
        size_t coefficientsLength = 0;
        switch (keyframe.curve.type)
        {
        case EASE:
            coefficientsLength = sizeof(float);
            memcpy(buffer + offset, &keyframe.curve.coefficient.powerValue, coefficientsLength);
            break;
        case WAVE:
        case GATE:
            coefficientsLength = sizeof(float) * 2 + sizeof(unsigned long);
            memcpy(buffer + offset, &keyframe.curve.coefficient.rangeAndPeriod, coefficientsLength);
            break;
        case LINEAR:
            // No additional data for linear
            break;
        }
        offset += coefficientsLength;
    }

    // Store the total scene size at the beginning of the buffer
    memcpy(buffer + sceneSizeOffset, &offset, sizeof(size_t));

    // Return the total size of the serialized data
    return offset;
}

size_t deserializeScene(Scene &scene, const uint8_t *buffer)
{
    size_t offset = 0;
    size_t sceneSize;
    memcpy(&sceneSize, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);

    SceneMode mode;
    std::vector<Keyframe> keyframes;

    // Deserialize mode
    memcpy(&mode, buffer + offset, sizeof(SceneMode));
    offset += sizeof(SceneMode);

    // Deserialize number of keyframes
    size_t keyframeCount;
    memcpy(&keyframeCount, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);

    keyframes.clear();                // Clear existing keyframes
    keyframes.reserve(keyframeCount); // Reserve space for keyframes

    // Deserialize each keyframe
    for (size_t i = 0; i < keyframeCount; ++i)
    {
        Keyframe keyframe;
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
        {
            float min, max;
            unsigned long period;
            memcpy(&min, buffer + offset, sizeof(float));
            offset += sizeof(float);
            memcpy(&max, buffer + offset, sizeof(float));
            offset += sizeof(float);
            memcpy(&period, buffer + offset, sizeof(unsigned long));
            offset += sizeof(unsigned long);
            keyframe.curve.coefficient = CurveCoefficient(min, max, period);
            break;
        }
        case LINEAR:
            // No additional data for linear
            break;
        }
        keyframes.push_back(keyframe);
    }

    // Store the deserialized scene
    scene = Scene(keyframes, mode);

    // Return the length of the serialized scene
    return sceneSize;
}

}
