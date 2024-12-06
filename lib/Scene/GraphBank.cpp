#include <Arduino.h>
#include <Utils.h>
#include <GraphBank.h>

GraphBank::GraphBank()
{
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

    _graphs[DefaultGraph::CONSTANT] = constant;
    _graphs[DefaultGraph::UP] = up;
    _graphs[DefaultGraph::UP_EXP] = upExp;
    _graphs[DefaultGraph::UP_LOG] = upLog;
    _graphs[DefaultGraph::DOWN] = down;
    _graphs[DefaultGraph::DOWN_EXP] = downExp;
    _graphs[DefaultGraph::DOWN_LOG] = downLog;
    _graphs[DefaultGraph::SINE] = sine;
    _graphs[DefaultGraph::GATE] = gate;
}

int GraphBank::getGraphId(Graph *graph) const
{
    for (const auto &it : _graphs)
    {
        if (it.second == graph)
            return it.first;
    }

    return -1;
}

size_t GraphBank::serialize(uint8_t *buffer) const
{
    size_t offset = 0, graphCount = 0;
    offset += sizeOfSizeT;

    for (const auto &it : _graphs)
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
        offset += keyframeGraph->serialize(buffer + offset);

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

        GraphKeyframe *keyframeGraph = new GraphKeyframe();
        offset += keyframeGraph->deserialize(buffer + offset);
        debug(1, "[deserialized graph] id: %d, number of keyframes: %lu", id, keyframeGraph->_keyframes.size());

        _graphs[id] = keyframeGraph;
    }

    return offset;
}

void GraphBank::sysExCreate(const uint8_t *buffer, size_t length)
{
    debug(1, "[SysEx] [graph] reading %lu bytes", length);

    // The keyframes list is two bytes after the begining of the SysEx message and the last byte is SysEx end
    size_t lengthOfKeyframes = length - 3;

    // Ensure the length of the keyframes list is a multiple of 6
    if (lengthOfKeyframes % 6 != 0)
    {
        debug(1, "[SysEx] [graph] keyframes length is not a multiple of 6: %lu", lengthOfKeyframes);
        return;
    }

    int sceneId = static_cast<int>(buffer[0]);
    int graphId = static_cast<int>(buffer[1]);
    int storeId = 4 * sceneId + graphId + DefaultGraph::LAST;

    debug(1, "[SysEx] [graph] scene id is %d, graph id is %d, store id is %d", sceneId, graphId, storeId);

    // Skip 2 bytes
    buffer = buffer + 2;
    length = length - 2;

    // Create keyframes
    std::vector<Keyframe> keyframes;

    for (size_t i = 0; i < length - 1; i = i + 6)
    {
        float x, y, c;
        x = twoBytesToFloat(buffer[i], buffer[i + 1]);
        y = twoBytesToFloat(buffer[i + 2], buffer[i + 3]);
        c = twoBytesToFloat(buffer[i + 4], buffer[i + 5], true);

        debug(1, "[SysEx] [graph] x: %f, y: %f, c: %f", x, y, c);

        // Create the keyframe
        Keyframe keyframe(x, y, c);
        keyframes.push_back(keyframe);
    }

    // Create graph
    GraphKeyframe *graph = new GraphKeyframe(keyframes);

    const auto it = _graphs.find(storeId);

    if (it != _graphs.end())
    {
        delete it->second;

        debug(1, "[SysEx] [graph] deleted previous graph with id %d", storeId);
    }
    else
    {
        debug(1, "[SysEx] [graph] no previous graph with id %d", storeId);
    }

    _graphs[storeId] = graph;

    debug(1, "[SysEx] [graph] stored graph id %d, number of keyframes: %lu", storeId, graph->_keyframes.size());
}
