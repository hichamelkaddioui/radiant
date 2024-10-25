#include <Arduino.h>
#include <Scene.h>
#include <vector>
#include <Utils.h>

String interpolationName(CurveType curveType)
{
    String interpolationType;
    switch (curveType)
    {
    case EASE:
        return "ease";
    case WAVE:
        return "sine";
    case GATE:
        return "gate";
    case LINEAR:
        return "linear";
    default:
        return "unknown";
    }
}

/**
 * Updates the scene by determining the current keyframe interval and interpolating
 * the value based on the current time and the specified curve.
 *
 * This function iterates through the keyframes to find the current interval between
 * two keyframes. If the current time is after the last keyframe, it behaves 
 * differently based on the scene mode. In TRIGGER mode, it returns the last
 * keyframe's value. In LOOP mode, it restarts the scene and returns the first
 * keyframe's value. Otherwise, it interpolates the value between the start and
 * end keyframes of the current interval.
 *
 * \returns The interpolated or final keyframe value based on the scene's mode and
 * current time.
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

    // debug(20, "[%f -> %f]\t%f\t(%s)\t%lu/%lu", startValue, endValue, interpolatedValue, interpolationName(curve.type).c_str(), currentTime, duration);

    return interpolatedValue;
};

/**
 * Interpolates a value between startValue and endValue over the duration of
 * totalTime milliseconds based on the given curve.
 *
 * \param currentTime The current time, in milliseconds.
 * \param totalTime The total duration of the easing, in milliseconds.
 * \param startValue The starting value of the easing.
 * \param endValue The ending value of the easing.
 * \param curve The curve to use for the easing.
 *
 * \returns The eased value.
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
        return wave(currentTime, coefficient.rangeAndPeriod.min, coefficient.rangeAndPeriod.max, findNearestPeriod(duration, coefficient.rangeAndPeriod.period));
    case GATE:
        return gate(currentTime, coefficient.rangeAndPeriod.min, coefficient.rangeAndPeriod.max, coefficient.rangeAndPeriod.period /* findNearestPeriod(duration, coefficient.rangeAndPeriod.period) */);
    case LINEAR:
    default:
        return ease(currentTime, duration, startValue, endValue, 1.0f);
    }
}

/**
 * Adds the given keyframes to the scene.
 *
 * The given keyframes are inserted at the end of the current keyframes. The
 * keyframes are sorted by time.
 *
 * \param keyframes The keyframes to add to the scene.
 */
void Scene::addKeyframes(std::vector<Keyframe> keyframes)
{
    _keyframes.insert(_keyframes.end(), keyframes.begin(), keyframes.end());
}

/**
 * Triggers the scene, starting the animation from the beginning.
 *
 * The scene will be restarted from the first keyframe.
 */
void Scene::trigger()
{
    if (_mode == SceneMode::TRIGGER)
        _chrono.restart();
}
