#ifndef UTILS_H
#define UTILS_H

void debug(int printEveryMs, const char *format, ...);
float ease(unsigned long currentTime, unsigned long duration, float startValue, float endValue, float power);
float gate(unsigned long currentTime, float min, float max, unsigned long halfPeriod);
float wave(unsigned long currentTime, float min, float max, unsigned long period);
unsigned long findNearestPeriod(unsigned long total, unsigned long initialPeriod);

#endif
