#ifndef SERIALIZE_H
#define SERIALIZE_H

#include <Arduino.h>
#include <Scene.h>

size_t serializeKeyframe(const Keyframe &keyframe, uint8_t *buffer);
size_t deserializeKeyframe(Keyframe &keyframe, const uint8_t *buffer);
size_t serializeKeyframes(const std::vector<Keyframe> keyframes, uint8_t *buffer);
size_t deserializeKeyframes(std::vector<Keyframe> &keyframes, const uint8_t *buffer);
size_t serializeGraph(const Graph &graph, uint8_t *buffer);
size_t deserializeGraph(Graph &graph, const uint8_t *buffer);
size_t serializeGraphBank(const GraphBank &bank, uint8_t *buffer);
size_t deserializeGraphBank(GraphBank &bank, const uint8_t *buffer);

#endif