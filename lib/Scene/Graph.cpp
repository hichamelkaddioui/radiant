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

size_t serializeKeyframe(const Keyframe &keyframe, uint8_t *buffer)
{
    size_t offset = 0;

    memcpy(buffer + offset, &keyframe._time, sizeOfFloat);
    offset += sizeOfFloat;
    memcpy(buffer + offset, &keyframe._value, sizeOfFloat);
    offset += sizeOfFloat;
    memcpy(buffer + offset, &keyframe._curve, sizeOfFloat);
    offset += sizeOfFloat;

    return offset;
}

size_t deserializeKeyframe(Keyframe &keyframe, const uint8_t *buffer)
{
    size_t offset = 0;
    memcpy(&keyframe._time, buffer + offset, sizeOfFloat);
    offset += sizeOfFloat;
    memcpy(&keyframe._value, buffer + offset, sizeOfFloat);
    offset += sizeOfFloat;
    memcpy(&keyframe._curve, buffer + offset, sizeOfFloat);
    offset += sizeOfFloat;

    return offset;
}

size_t serializeKeyframes(const std::vector<Keyframe> keyframes, uint8_t *buffer)
{
    size_t offset = 0;

    size_t keyframeCount = keyframes.size();
    memcpy(buffer, &keyframeCount, sizeOfSizeT);
    offset += sizeOfSizeT;
    debug(1, "[serialize keyframes] %lu keyframes", keyframeCount);

    for (const Keyframe &keyframe : keyframes)
    {
        debug(1, "[serialize keyframe] (x,y) = (%f, %f)", keyframe._time, keyframe._value);
        offset += serializeKeyframe(keyframe, buffer + offset);
    }

    return offset;
}

size_t deserializeKeyframes(std::vector<Keyframe> &keyframes, const uint8_t *buffer)
{
    size_t offset = 0, keyframeCount = 0;

    memcpy(&keyframeCount, buffer + offset, sizeOfSizeT);
    offset += sizeOfSizeT;
    debug(1, "[deserialize keyframes] %lu keyframes", keyframeCount);

    keyframes.reserve(keyframeCount);
    keyframes.clear();

    for (size_t i = 0; i < keyframeCount; i++)
    {
        Keyframe keyframe;
        offset += deserializeKeyframe(keyframe, buffer + offset);
        debug(1, "[deserialize keyframe] (x,y) = (%f, %f)", keyframe._time, keyframe._value);

        keyframes.push_back(keyframe);
    }

    return offset;
}

size_t serializeGraph(const Graph &graph, uint8_t *buffer)
{
    size_t offset = 0;

    GraphKeyframe *keyframeGraph = dynamic_cast<GraphKeyframe *>(const_cast<Graph *>(&graph));

    if (keyframeGraph == nullptr)
        return 0;

    return serializeKeyframes(keyframeGraph->_keyframes, buffer + offset);
}

size_t deserializeGraph(Graph &graph, const uint8_t *buffer)
{
    size_t offset = 0;

    GraphKeyframe *keyframeGraph = dynamic_cast<GraphKeyframe *>(&graph);

    if (keyframeGraph == nullptr)
        return 0;

    return deserializeKeyframes(keyframeGraph->_keyframes, buffer + offset);
}

size_t GraphBank::serialize(uint8_t *buffer)
{
    int graphCount = 0;
    size_t offset = 0;
    offset += sizeOfSizeT;

    for (const auto &it : _bank)
    {
        if (it.first < 9)
        {
            continue;
        }

        GraphKeyframe *keyframeGraph = dynamic_cast<GraphKeyframe *>(const_cast<Graph *>(it.second));

        if (keyframeGraph == nullptr)
            continue;

        debug(1, "[serialize graph] id: %d, number of keyframes: %lu", it.first, keyframeGraph->_keyframes.size());
        memcpy(buffer + offset, &it.first, sizeOfInt);
        offset += sizeOfInt;
        offset += serializeGraph(*keyframeGraph, buffer + offset);

        graphCount++;
    }

    debug(1, "[serialize graph bank] %lu graphs serialized", graphCount);

    memcpy(buffer, &graphCount, sizeOfSizeT);

    return offset;
}

size_t GraphBank::deserialize(const uint8_t *buffer)
{
    size_t offset = 0, graphCount = 0;

    memcpy(&graphCount, buffer + offset, sizeOfSizeT);
    offset += sizeOfSizeT;
    debug(1, "[deserialize graph bank] reading %lu graphs", graphCount);

    for (size_t i = 0; i < graphCount; i++)
    {
        int id = 0;
        memcpy(&id, buffer + offset, sizeOfInt);
        offset += sizeOfInt;

        if (id < 9)
        {
            continue;
        }

        GraphKeyframe *graph = new GraphKeyframe();
        offset += deserializeGraph(*graph, buffer + offset);
        debug(1, "[deserialized graph] id: %d, number of keyframes: %lu", id, graph->_keyframes.size());

        _bank[id] = graph;
    }

    return offset;
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
    GraphKeyframe *downExp = new GraphKeyframe({fromOneLog, toZero});
    GraphKeyframe *downLog = new GraphKeyframe({fromOneExp, toZero});

    // Oscillate
    GraphSine *sine = new GraphSine();
    GraphGate *gate = new GraphGate();

    result._bank[DefaultGraph::CONSTANT] = constant;
    result._bank[DefaultGraph::UP] = up;
    result._bank[DefaultGraph::UP_EXP] = upExp;
    result._bank[DefaultGraph::UP_LOG] = upLog;
    result._bank[DefaultGraph::DOWN] = down;
    result._bank[DefaultGraph::DOWN_EXP] = downExp;
    result._bank[DefaultGraph::DOWN_LOG] = downLog;
    result._bank[DefaultGraph::SINE] = sine;
    result._bank[DefaultGraph::GATE] = gate;

    return result;
}
