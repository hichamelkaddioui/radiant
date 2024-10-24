#ifndef UTILS_H
#define UTILS_H

void debug(int printEveryMs, const char *format, ...);
float ease(unsigned long currentTime, unsigned long duration, float startValue, float endValue, float power);
float gate(unsigned long currentTime, float topValue, float bottomValue, unsigned long halfPeriod);
float wave(unsigned long currentTime, float startValue, float endValue, unsigned long period);
int unsigned long closestDivisor(int a, int b);

#endif
