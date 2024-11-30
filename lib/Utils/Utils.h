#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

#define UTILS_HUE_RED 0
#define UTILS_HUE_ORANGE 30
#define UTILS_HUE_PEACH 35
#define UTILS_HUE_GOLDEN_YELLOW 60
#define UTILS_HUE_GREEN 85
#define UTILS_HUE_TEAL 120
#define UTILS_HUE_SOFT_GREEN 125
#define UTILS_HUE_AQUA 140
#define UTILS_HUE_BLUE 170
#define UTILS_HUE_INDIGO 200
#define UTILS_HUE_LIGHT_BLUE 215
#define UTILS_HUE_WARM_PINK 210

const size_t sizeOfSizeT = sizeof(size_t);
const size_t sizeOfInt = sizeof(int);
const size_t sizeOfLong = sizeof(unsigned long);
const size_t sizeOfFloat = sizeof(float);
const size_t sizeOfByte = sizeof(uint8_t);

void debug(int printEveryMs, const char *format, ...);
float interpolate(float t, float startX, float startY, float endX, float endY, float curve);

#endif
