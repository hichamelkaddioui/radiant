#include <Arduino.h>
#include <Utils.h>
#include <Graph.h>

Keyframe::Keyframe(float time, float value, float curve)
{
    _time = constrain(time, 0.0f, 1.0f);
    _value = constrain(value, 0.0f, 1.0f);
    _curve = constrain(curve, -1.0f, 1.0f);
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
    float result = (1 + sin(TWO_PI * t / _period)) / 2.0f;

    return result;
}

float GraphGate::valueAt(float t)
{
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

GraphBank defaultGraphBank()
{
    GraphBank result;

    // Increasing from 0 to 1
    Keyframe fromZero = Keyframe(0.0f, 0.0f, 0.0f);
    Keyframe fromZeroExp = Keyframe(0.0f, 0.0f, 0.5f);
    Keyframe fromZeroLog = Keyframe(0.0f, 0.0f, -0.5f);
    Keyframe toOne = Keyframe(1.0f, 1.0f, 0.0f);

    GraphKeyframe *constant = new GraphKeyframe({fromZero});
    GraphKeyframe *up = new GraphKeyframe({fromZero, toOne});
    GraphKeyframe *upExp = new GraphKeyframe({fromZeroExp, toOne});
    GraphKeyframe *upLog = new GraphKeyframe({fromZeroLog, toOne});

    // Decreasing from 1 to 0
    Keyframe fromOne = Keyframe(0.0f, 1.0f, 0.0f);
    Keyframe fromOneExp = Keyframe(0.0f, 1.0f, 0.5f);
    Keyframe fromOneLog = Keyframe(0.0f, 1.0f, -0.5f);
    Keyframe toZero = Keyframe(1.0f, 0.0f, 0.0f);

    GraphKeyframe *down = new GraphKeyframe({fromOne, toZero});
    GraphKeyframe *downExp = new GraphKeyframe({fromOneExp, toZero});
    GraphKeyframe *downLog = new GraphKeyframe({fromOneLog, toZero});

    // Oscillate
    GraphSine *sine = new GraphSine();
    GraphGate *gate = new GraphGate();

    result[DefaultGraph::CONSTANT] = constant;
    result[DefaultGraph::UP] = up;
    result[DefaultGraph::UP_EXP] = upExp;
    result[DefaultGraph::UP_LOG] = upLog;
    result[DefaultGraph::DOWN] = down;
    result[DefaultGraph::DOWN_EXP] = downExp;
    result[DefaultGraph::DOWN_LOG] = downLog;
    result[DefaultGraph::SINE] = sine;
    result[DefaultGraph::GATE] = gate;

    return result;
}
