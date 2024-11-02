#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <map>

class Graph
{
public:
    virtual float valueAt(float t) = 0;
};

class Keyframe
{
public:
    Keyframe(float time, float value, float curve);

    float _time;
    float _value;
    float _curve;
};

class GraphKeyframe : public Graph
{
public:
    GraphKeyframe(std::vector<Keyframe> keyframes) : _keyframes(keyframes){};
    float valueAt(float t) override;

private:
    std::vector<Keyframe> _keyframes;
};

class GraphSine : public Graph
{
public:
    float valueAt(float t) override;
};

class GraphGate : public Graph
{
public:
    float valueAt(float t) override;

private:
    float _cutoff = 0.5f;
};

typedef std::map<int, Graph *> GraphBank;

GraphBank defaultGraphBank();

#endif