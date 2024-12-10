#ifndef GRAPH_BANK_H
#define GRAPH_BANK_H

#include <map>
#include <Arduino.h>
#include <Graph.h>

class GraphBank
{
public:
    GraphBank();

    std::map<int, Graph *> _graphs;

    int getGraphId(Graph *graph) const;
    size_t serialize(uint8_t *buffer) const;
    size_t deserialize(const uint8_t *buffer);
    void sysExCreate(const uint8_t *buffer, size_t length);

    static GraphBank createDummy();
};

#endif
