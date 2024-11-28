#include <Utils.h>
#include <Serialize.h>

const size_t sizeOfSizeT = sizeof(size_t);
const size_t sizeOfInt = sizeof(int);
const size_t sizeOfFloat = sizeof(float);

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

size_t serializeGraphBank(const GraphBank &bank, uint8_t *buffer)
{
    int graphCount = 0;
    size_t offset = 0;
    offset += sizeOfSizeT;

    debug(1, "[deserialize graph bank] %lu graphs", graphCount);

    for (const auto &it : bank)
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

    memcpy(buffer, &graphCount, sizeOfSizeT);

    return offset;
}

size_t deserializeGraphBank(GraphBank &bank, const uint8_t *buffer)
{
    size_t offset = 0, graphCount = 0;

    memcpy(&graphCount, buffer + offset, sizeOfSizeT);
    offset += sizeOfSizeT;
    debug(1, "[deserialize graph bank] %lu graphs", graphCount);

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

        bank[id] = graph;

        debug(1, "[deserialized graph] wazzup?");
    }

    return offset;
}