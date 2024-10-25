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
 * duration of duration milliseconds. The time at which the easing
 * happens is given by currentTime.
 *
 * The easing function is a simple power function where the power is
 * given by the power parameter.
 *
 * \param currentTime The current time, in milliseconds.
 * \param duration The total duration of the easing, in milliseconds.
 * \param startValue The starting value of the easing.
 * \param endValue The ending value of the easing.
 * \param power The power of the easing function.
 *
 * \returns The eased value.
 */
float ease(unsigned long currentTime, unsigned long duration, float startValue, float endValue, float power)
{
    float normalizedTime = (float)currentTime / (float)duration;
    float coefficient = pow(normalizedTime, power);

    return startValue * (1 - coefficient) + endValue * coefficient;
}

/**
 * Generates a gate signal with the given minimum and maximum values, with a given half-period.
 *
 * The given value is the value of the gate signal at the given current time.
 *
 * The gate signal is a square wave with a period of 2 * halfPeriod milliseconds. The gate
 * signal is max for the first halfPeriod milliseconds and min for the second halfPeriod
 * milliseconds.
 *
 * \param currentTime The current time, in milliseconds.
 * \param min The minimum value of the gate signal.
 * \param max The maximum value of the gate signal.
 * \param halfPeriod The half-period of the gate signal, in milliseconds.
 *
 * \returns The value of the gate signal at the given current time.
 */
float gate(unsigned long currentTime, float min, float max, unsigned long halfPeriod)
{
    unsigned long period = halfPeriod * 2;
    if ((currentTime % period) < halfPeriod)
    {
        return max;
    }
    else
    {
        return min;
    }
}

/**
 * Generates a sine wave with the given minimum and maximum values over the given period.
 *
 * The given value is the value of the sine wave at the given current time.
 *
 * \param currentTime The current time, in milliseconds.
 * \param min The minimum value of the sine wave.
 * \param max The maximum value of the sine wave.
 * \param period The period of the sine wave.
 *
 * \returns The value of the sine wave at the given current time.
 */
float wave(unsigned long currentTime, float min, float max, unsigned long period)
{
    double amplitude = (max - min) / 2.0;
    double offset = (max + min) / 2.0;

    return amplitude * sin((TWO_PI / period) * currentTime) + offset;
}

/**
 * Finds the period that is closest to the given initial period,
 * given the total duration.
 *
 * The function works by finding the two divisors of the total duration
 * that are closest to the given initial period. It then returns the
 * period that is associated with the closest divisor.
 *
 * \param total The total duration.
 * \param initialPeriod The initial period.
 *
 * \returns The period that is closest to the given initial period.
 */
unsigned long findNearestPeriod(unsigned long total, unsigned long initialPeriod)
{
    // Edge cases
    if (total == 0 || total < initialPeriod || initialPeriod == 0)
    {
        return initialPeriod;
    }

    int N = total / initialPeriod;
    int lowerDivisor = 1;
    int upperDivisor = total;

    // Find the lower and upper divisors
    for (int i = 1; i <= sqrt(total); i++)
    {
        if (total % i == 0)
        {
            if (i <= N)
            {
                lowerDivisor = max(lowerDivisor, i);
            }
            if (total / i <= N)
            {
                upperDivisor = min(upperDivisor, total / i);
            }
        }
    }

    unsigned long nearestDivisor = (labs((long)initialPeriod - (long)lowerDivisor) <= labs((long)initialPeriod - (long)upperDivisor)) ? lowerDivisor : upperDivisor;
    unsigned long nearestPeriod = total / nearestDivisor;

    return nearestPeriod;
}
