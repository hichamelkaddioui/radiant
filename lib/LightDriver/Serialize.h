#ifndef SERIALIZE_H
#define SERIALIZE_H

#include <Arduino.h>
#include <Scene.h>

size_t serializeScene(Scene scene, uint8_t *buffer);
size_t deserializeScene(Scene &scene, const uint8_t *buffer);

#endif