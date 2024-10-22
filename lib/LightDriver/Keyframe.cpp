#include <Keyframe.h>

float KeyframeList::update(unsigned long currentTime)
{
    Keyframe *startKey = nullptr;
    Keyframe *endKey = nullptr;

    for (size_t i = 0; i < _count - 1; ++i)
    {
        if (currentTime >= _keyframes[i].time && currentTime < _keyframes[i + 1].time)
        {
            startKey = &_keyframes[i];
            endKey = &_keyframes[i + 1];
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
};

void KeyframeList::destroy()
{
    delete[] _keyframes;
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
