#ifndef SERIALIZE_H
#define SERIALIZE_H

#include <Arduino.h>
#include <Scene.h>
#include <Neopixel.h>

size_t serializeScene(const Scene scene, uint8_t *buffer);
size_t deserializeScene(Scene &scene, const uint8_t *buffer);
size_t serializeNeoPixel(const NeoPixel &pixel, uint8_t *buffer);
size_t deserializeNeoPixel(NeoPixel &pixel, const uint8_t *buffer);

#endif