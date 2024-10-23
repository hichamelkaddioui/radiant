#include <Arduino.h>

/**
 * Prints a debug message to the serial console.
 *
 * The format string is like the one of printf, and it is followed by
 * the arguments to be printed.
 *
 * \param format The format string.
 */
void debug(int printEveryMs, const char *format, ...)
{
#ifdef DEBUG
    if (millis() % printEveryMs != 0)
        return;

    char buffer[100];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    Serial.println(buffer);
    va_end(args);
#endif
}

/**
 * Eases a value between two bounds over a given duration.
 *
 * The given value is eased from startValue to endValue over the
 * duration of totalTime milliseconds. The time at which the easing
 * happens is given by currentTime.
 *
 * The easing function is a simple power function where the power is
 * given by the power parameter.
 *
 * \param currentTime The current time, in milliseconds.
 * \param totalTime The total duration of the easing, in milliseconds.
 * \param startValue The starting value of the easing.
 * \param endValue The ending value of the easing.
 * \param power The power of the easing function.
 *
 * \returns The eased value.
 */
float ease(float currentTime, float totalTime, float startValue, float endValue, float power)
{
    float coefficient = pow(currentTime / totalTime, power);

    return startValue + (endValue - startValue) * coefficient;
}