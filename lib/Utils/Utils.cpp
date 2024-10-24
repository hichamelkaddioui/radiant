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
 * Gates a value between topValue and bottomValue based on the current time.
 *
 * This function simulates a gate behavior by alternating the output value between topValue and bottomValue
 * based on the current time and the duration of halfPeriod.
 *
 * \param currentTime The current time, in milliseconds.
 * \param topValue The value to return when the gate is high.
 * \param bottomValue The value to return when the gate is low.
 * \param halfPeriod Half of the period for the gate behavior.
 *
 * \returns The gated value based on the current time and gate behavior.
 */
float gate(unsigned long currentTime, float topValue, float bottomValue, unsigned long halfPeriod)
{
    unsigned long period = halfPeriod * 2;
    if ((currentTime % period) < halfPeriod)
    {
        // The gate is high, return topValue
        return topValue;
    }
    else
    {
        // The gate is low, return bottomValue
        return bottomValue;
    }
}

/**
 * Waves a value between startValue and endValue over the duration of period milliseconds.
 *
 * This function simulates a wave behavior by oscillating the output value between startValue and endValue
 * based on the current time and the duration of period.
 *
 * \param currentTime The current time, in milliseconds.
 * \param startValue The starting value of the wave.
 * \param endValue The ending value of the wave.
 * \param period The duration of the wave, in milliseconds.
 *
 * \returns The waved value based on the current time and wave behavior.
 */
float wave(unsigned long currentTime, float startValue, float endValue, unsigned long period)
{
    double amplitude = (endValue - startValue) / 2.0;
    double offset = (endValue + startValue) / 2.0;

    return amplitude * sin((TWO_PI / period) * currentTime) + offset;
}

int unsigned long closestDivisor(int a, int b)
{
    if (a == 0 || b == 0)
    {
        return 0; // Handle edge cases
    }

    int N = a / b; // Integer division

    if (N == 0)
    {
        return 1; // If a < b, return 1 as the smallest possible divisor
    }

    int lowerDivisor = 1;
    int upperDivisor = a;

    // Find the lower and upper divisors
    for (int i = 1; i <= sqrt(a); i++)
    {
        if (a % i == 0)
        {
            if (i <= N)
            {
                lowerDivisor = max(lowerDivisor, i);
            }
            if (a / i <= N)
            {
                upperDivisor = min(upperDivisor, a / i);
            }
        }
    }

    // Return the divisor closest to b
    unsigned long result = (abs(b - lowerDivisor) <= abs(b - upperDivisor)) ? lowerDivisor : upperDivisor;

    return result;
}
