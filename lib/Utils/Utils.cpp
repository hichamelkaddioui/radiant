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

    char buffer[800];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    Serial.println(buffer);
    va_end(args);
#endif
}

/**
 * Interpolates a value between two points, given a curve coefficient.
 *
 * If curve is zero, this performs a linear interpolation. Otherwise, it
 * performs a curve interpolation based on an exponential function.
 * 
 * See: https://docs.cycling74.com/legacy/max7/refpages/function#setcurve
 *
 * \param t The time value between startX and endX.
 * \param startX The start time of the interpolation.
 * \param startY The start value of the interpolation.
 * \param endX The end time of the interpolation.
 * \param endY The end value of the interpolation.
 * \param curve The curve coefficient, in the range [-1, 1].
 *
 * \returns The interpolated value at time t.
 */
float interpolate(float t, float startX, float startY, float endX, float endY, float curve)
{
    float valueRange = endY - startY;
    float domain = endX - startX;

    if (std::abs(curve) < 0.001)
    {
        return ((t - startX) / domain) * valueRange + startY;
    }

    if (curve < 0.0)
    {
        float gx = (endX - t) / domain;
        float hp = std::pow((1e-20 - curve) * 1.2, 0.41) * 0.91;
        float fp = hp / (1.0 - hp);
        float gp = (std::exp(fp * gx) - 1.0) / (std::exp(fp) - 1.0);

        return endY - gp * valueRange;
    }
    else
    {
        float gx = (t - startX) / domain;
        float hp = std::pow((curve + 1e-20) * 1.2, 0.41) * 0.91;
        float fp = hp / (1.0 - hp);
        float gp = (std::exp(fp * gx) - 1.0) / (std::exp(fp) - 1.0);

        return gp * valueRange + startY;
    }
}
