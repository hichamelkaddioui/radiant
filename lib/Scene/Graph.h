#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <map>

enum DefaultGraph
{
    CONSTANT = 0,
    UP = 1,
    UP_EXP = 2,
    UP_LOG = 3,
    DOWN = 4,
    DOWN_EXP = 5,
    DOWN_LOG = 6,
    SINE = 7,
    GATE = 8,
    LAST,
};

class Graph
{
public:
    virtual float valueAt(float t) = 0;
};

class Keyframe
{
public:
    Keyframe(){};
    Keyframe(float time, float value, float curve);

    float _time;
    float _value;
    float _curve;
};

class GraphKeyframe : public Graph
{
public:
    GraphKeyframe(){};
    GraphKeyframe(std::vector<Keyframe> keyframes) : _keyframes(keyframes){};
    std::vector<Keyframe> _keyframes;
    float valueAt(float t) override;
};

class PeriodicGraph : public Graph
{
public:
    float _period;
    virtual float valueAt(float t) = 0;
};

class GraphSine : public PeriodicGraph
{
public:
    float valueAt(float t) override;
};

class GraphGate : public PeriodicGraph
{
public:
    float valueAt(float t) override;
};

class GraphBank
{
public:
    GraphBank() : _bank(){};

    std::map<int, Graph *> _bank;

    void clear();
    int getGraphId(Graph *graph) const;
    size_t serialize(uint8_t *buffer) const;
    size_t deserialize(const uint8_t *buffer);
    void sysExCreate(const uint8_t *buffer, size_t length);
};

GraphBank defaultGraphBank();

#endif