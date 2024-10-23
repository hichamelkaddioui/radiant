#ifndef UTILS_H
#define UTILS_H

void debug(int printEveryMs, const char *format, ...);
float ease(unsigned long currentTime, unsigned long totalTime, float startValue, float endValue, float power);
float gate(unsigned long currentTime, float topValue, float bottomValue, unsigned long halfPeriod);

#endif
