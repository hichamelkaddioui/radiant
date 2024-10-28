#ifndef UTILS_H
#define UTILS_H

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

void debug(int printEveryMs, const char *format, ...);
float ease(unsigned long currentTime, unsigned long duration, float startValue, float endValue, float power);
float gate(unsigned long currentTime, float min, float max, unsigned long halfPeriod);
float wave(unsigned long currentTime, float min, float max, unsigned long period);
unsigned long findNearestPeriod(unsigned long total, unsigned long initialPeriod);

#endif
