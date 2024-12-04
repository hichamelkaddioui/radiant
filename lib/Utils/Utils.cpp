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

void debugByteArray(const uint8_t *data, size_t length)
{
#ifdef DEBUG
    for (int i = 0; i < length; i++)
    {
        if (i % 8 == 0 && i != 0)
            Serial.print("\n\t");
        else
            Serial.print("\t");

        Serial.print("0x");

        if (data[i] < 16)
            Serial.print("0");

        Serial.print(data[i], HEX);
    }

    Serial.println();
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

/**
 * Converts two bytes to a floating point number.
 *
 * The two bytes are interpreted as a 14-bit signed or unsigned number.
 * If the number is signed, its range is [-1, 1], otherwise it is [0, 1].
 *
 * \param msb The most significant byte of the 14-bit number.
 * \param lsb The least significant 7 bits of the 14-bit number.
 * \param isSigned If true, the number is interpreted as signed, otherwise it is unsigned.
 *
 * \returns The floating point representation of the 14-bit number.
 */
float twoBytesToFloat(byte msb, byte lsb, bool isSigned = false)
{
    float result = constrain((msb << 7 | (lsb & 0x7F)) / 16383.0f, 0.0f, 1.0f);

    if (isSigned)
        return result * 2.0f - 1.0f;

    return result;
}

/**
 * Converts three bytes to an unsigned long.
 *
 * The three bytes are interpreted as a 21-bit unsigned number.
 *
 * \param msb The most significant byte of the 21-bit number.
 * \param lsb The middle byte of the 21-bit number.
 * \param lsb2 The least significant 7 bits of the 21-bit number.
 *
 * \returns The unsigned long representation of the 21-bit number.
 */
unsigned long threeBytesToLong(byte msb, byte lsb, byte lsb2)
{
    unsigned long result =
        (static_cast<unsigned long>(msb) << 14) |
        (static_cast<unsigned long>(lsb) << 7) |
        static_cast<unsigned long>(lsb2);
    return result;
}
