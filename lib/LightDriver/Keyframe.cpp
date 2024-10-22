#include <Keyframe.h>
#include <vector>

float KeyframeList::update(unsigned long currentTime)
{
    Keyframe *startKey = nullptr;
    Keyframe *endKey = nullptr;

    for (auto it = _keyframes.begin(); it != _keyframes.end() - 1; ++it)
    {
        bool isKeyframeInterval = currentTime >= it->time && currentTime < (it + 1)->time;
        if (isKeyframeInterval)
        {
            startKey = &(*it);
            endKey = &(it[1]);
            break;
        }
    }

    if (startKey && endKey)
    {
        float duration = endKey->time - startKey->time;
        float elapsed = currentTime - startKey->time;
        float t = constrain(elapsed / duration, 0.0f, 1.0f);
        return interpolate(startKey->value, endKey->value, t, startKey->methodsAndParams);
    }

    return 0.0f;
}

float KeyframeList::interpolate(float start, float end, float t, const std::vector<InterpolationMethodPair> &methodsAndParams)
{
    float result = start;

    for (const auto &methodPair : methodsAndParams)
    {
        InterpolationMethod method = methodPair.first;
        const InterpolationParams &params = methodPair.second;

        switch (method)
        {
        case POWER:
            result = result + pow(t, params.powerValue) * (end - result);
            break;
        case GATE:
            if (t < params.cutoffTime / (end - start))
            {
                result = start;
            }
            else
            {
                result = end;
            }
            break;
        case LINEAR:
        default:
            result = result + (end - result) * t;
            break;
        }
    }

    return result;
}
