#include <Arduino.h>
#include <Utils.h>
#include <Graph.h>

Keyframe::Keyframe(float time, float value, float curve)
{
    _time = constrain(time, 0.0f, 1.0f);
    _value = constrain(value, 0.0f, 1.0f);
    _curve = constrain(curve, -1.0f, 1.0f);
}

size_t GraphKeyframe::serialize(uint8_t *buffer) const
{
    size_t offset = 0;

    size_t keyframeCount = _keyframes.size();
    memcpy(buffer, &keyframeCount, sizeOfSizeT);
    offset += sizeOfSizeT;
    debug(1, "[serialize keyframes] %lu keyframes", keyframeCount);

    for (const Keyframe &keyframe : _keyframes)
    {
        memcpy(buffer + offset, &keyframe._time, sizeOfFloat);
        offset += sizeOfFloat;
        memcpy(buffer + offset, &keyframe._value, sizeOfFloat);
        offset += sizeOfFloat;
        memcpy(buffer + offset, &keyframe._curve, sizeOfFloat);
        offset += sizeOfFloat;
        debug(1, "[serialize keyframe] (x,y) = (%f, %f)", keyframe._time, keyframe._value);
    }

    return offset;
}

size_t GraphKeyframe::deserialize(const uint8_t *buffer)
{
    size_t offset = 0, keyframeCount = 0;

    memcpy(&keyframeCount, buffer + offset, sizeOfSizeT);
    offset += sizeOfSizeT;
    debug(1, "[deserialize keyframes] %lu keyframes", keyframeCount);

    _keyframes.reserve(keyframeCount);
    _keyframes.clear();

    for (size_t i = 0; i < keyframeCount; i++)
    {
        Keyframe keyframe;
        memcpy(&keyframe._time, buffer + offset, sizeOfFloat);
        offset += sizeOfFloat;
        memcpy(&keyframe._value, buffer + offset, sizeOfFloat);
        offset += sizeOfFloat;
        memcpy(&keyframe._curve, buffer + offset, sizeOfFloat);
        offset += sizeOfFloat;
        debug(1, "[deserialize keyframe] (x,y) = (%f, %f)", keyframe._time, keyframe._value);

        _keyframes.push_back(keyframe);
    }

    return offset;
}

float GraphKeyframe::valueAt(float t)
{
    size_t keyframeCount = _keyframes.size();

    // No keyframe - should not happen
    if (keyframeCount == 0)
        return 0.0f;

    // One keyframe: return its value
    if (keyframeCount == 1)
        return _keyframes[0]._value;

    // Find the first keyframe that is after t
    size_t i = 0;
    for (; i < keyframeCount; i++)
    {
        if (_keyframes[i]._time > t)
            break;
    }

    // No keyframe found - should not happen as there's always at least one keyframe at t = 0.0f
    if (i == 0)
        return _keyframes[0]._value;

    // Last keyframe - should not happen since t < 1.0f
    if (i == keyframeCount)
        return _keyframes[i - 1]._value;

    // Interpolate
    Keyframe k1 = _keyframes[i - 1];
    Keyframe k2 = _keyframes[i];
    float startX = k1._time;
    float startY = k1._value;
    float curve = k1._curve;
    float endX = k2._time;
    float endY = k2._value;
    float result = interpolate(t, startX, startY, endX, endY, curve);

    return result;
}

float GraphSine::valueAt(float t)
{
    float result = (1 + sin(TWO_PI * t / _period - TWO_PI / 4)) / 2.0f;

    return result;
}

float GraphGate::valueAt(float t)
{
    // period = 1/3
    //
    // y
    // 1 ┌────────────             ─────────────             ─────────────
    //   |
    //   |
    // 0 |            ─────────────             ─────────────              ──────────────
    //   ├────────────┬────────────┬────────────┬────────────┬─────────────┬─────────────┬─ t
    //   0           1/6          1/3          1/2          2/3           5/6            1
    //    ┌─>   0            1            2           3              4             5
    //    |
    //    └─────── halfPeriodCount

    float halfPeriodCount = t / (_period / 2.0f);

    // If scaled time is in the first half of the period, return 1, else return 0
    if (static_cast<int>(halfPeriodCount) % 2 == 0)
    {
        return 1.0f; // First half-period: return 1
    }
    else
    {
        return 0.0f; // Second half-period: return 0
    }
}
