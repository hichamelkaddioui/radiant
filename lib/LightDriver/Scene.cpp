#include <Arduino.h>
#include <Scene.h>
#include <Utils.h>

/**
 * Appends the given keyframes to the internal keyframe list.
 *
 * This can be used to extend an existing scene with new keyframes, or to
 * concatenate multiple scenes.
 *
 * @param keyframes The keyframes to be added to the scene.
 */
void Scene::addKeyframes(std::vector<Keyframe> keyframes)
{
    _keyframes.insert(_keyframes.end(), keyframes.begin(), keyframes.end());
}

/**
 * Updates the scene and returns the interpolated value at the current time.
 *
 * @return The current value of the scene.
 */
float Scene::update()
{
    if (_keyframes.size() < 2)
    {
        if (_keyframes.size() == 1)
            return _keyframes[0].value;

        return 0.0f;
    }

    Keyframe *startKey = nullptr;
    Keyframe *endKey = nullptr;

    for (auto it = _keyframes.begin(); it != _keyframes.end() - 1; ++it)
    {
        bool isKeyframeInterval = _chrono.hasPassed(it->time) && !_chrono.hasPassed((it + 1)->time);

        if (isKeyframeInterval)
        {
            startKey = &(*it);
            endKey = &(it[1]);
            break;
        }
    }

    Keyframe lastKeyframe = _keyframes.back();
    bool isAfterLastKeyframe = _chrono.hasPassed(lastKeyframe.time);

    // If we're after the last keyframe
    if (isAfterLastKeyframe)
    {
        // In trigger, we wait for a trigger event and return the last keyframe value
        if (_mode == SceneMode::TRIGGER)
        {
            return lastKeyframe.value;
        }
        // In loop, we restart the loop and return the first keyframe value
        else if (_mode == SceneMode::LOOP)
        {
            _chrono.restart();
            return _keyframes.begin()->value;
        }
    }

    unsigned long duration = endKey->time - startKey->time;

    if (duration == 0)
    {
        return startKey->value;
    }

    unsigned long elapsed = _chrono.elapsed();
    unsigned long currentTime = elapsed - startKey->time;
    float startValue = startKey->value;
    float endValue = endKey->value;
    Curve curve = startKey->curve;
    float interpolatedValue = interpolate(currentTime, duration, startValue, endValue, curve);

    return interpolatedValue;
};

/**
 * Interpolates a value between two keyframes based on the specified curve type.
 *
 * Depending on the curve type, this function applies the appropriate interpolation
 * method (ease, wave, gate, or linear) to calculate the intermediate value between
 * the start and end keyframes at a given time.
 *
 * @param currentTime The current time, relative to the start of the keyframe interval, in milliseconds.
 * @param duration The duration of the keyframe interval, in milliseconds.
 * @param startValue The starting value of the interpolation.
 * @param endValue The ending value of the interpolation.
 * @param curve The curve defining the interpolation type and its coefficients.
 * @return The interpolated value at the specified current time.
 */
float Scene::interpolate(unsigned long currentTime, unsigned long duration, float startValue, float endValue, Curve curve)
{
    CurveType type = curve.type;
    CurveCoefficient coefficient = curve.coefficient;

    switch (type)
    {
    case EASE:
        return ease(currentTime, duration, startValue, endValue, coefficient.powerValue);
    case WAVE:
        return wave(currentTime, coefficient.rangeAndPeriod.min, coefficient.rangeAndPeriod.max, coefficient.rangeAndPeriod.period /* findNearestPeriod(duration, coefficient.rangeAndPeriod.period) */);
    case GATE:
        return gate(currentTime, coefficient.rangeAndPeriod.min, coefficient.rangeAndPeriod.max, coefficient.rangeAndPeriod.period /* findNearestPeriod(duration, coefficient.rangeAndPeriod.period) */);
    case LINEAR:
    default:
        return ease(currentTime, duration, startValue, endValue, 1.0f);
    }
}

/**
 * Triggers a scene to start or restart from the beginning.
 *
 * Only applicable when the scene mode is set to SceneMode::TRIGGER.
 *
 * This function has no effect when the scene mode is set to SceneMode::LOOP.
 */
void Scene::trigger()
{
    if (_mode == SceneMode::TRIGGER)
        _chrono.restart();
}
