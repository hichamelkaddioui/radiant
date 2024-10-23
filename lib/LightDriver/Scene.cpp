#include <Arduino.h>
#include <Scene.h>
#include <vector>
#include <Utils.h>

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

    // debug(20, "[%f -> %f]\t%lu/%lu\t\t%f", startValue, endValue, currentTime, duration, interpolatedValue);

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
    CurveCoefficients coefficients = curve.coefficients;

    switch (type)
    {
    case EASE:
        return ease(currentTime, duration, startValue, endValue, coefficients.powerValue);
    case GATE:
        return gate(currentTime, startValue, endValue, coefficients.cutoffTime);
    case LINEAR:
    default:
        return ease(currentTime, duration, startValue, endValue, 1.0f);
    }
}
