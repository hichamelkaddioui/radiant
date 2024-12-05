#include <Arduino.h>
#include <Utils.h>
#include <LedStrip.h>

void LedStrip::setup()
{
    pinMode(pinR, OUTPUT);
    pinMode(pinG, OUTPUT);
    pinMode(pinB, OUTPUT);
}

void LedStrip::setRgb(uint8_t r, uint8_t g, uint8_t b)
{
    analogWrite(pinR, r);
    analogWrite(pinG, g);
    analogWrite(pinB, b);
}

size_t LedStrip::serialize(int id, uint8_t *buffer) const
{
    size_t offset = 0;

    // Pin R
    memcpy(buffer + offset, &pinR, sizeOfInt);
    offset += sizeOfInt;
    // Pin G
    memcpy(buffer + offset, &pinG, sizeOfInt);
    offset += sizeOfInt;
    // Pin B
    memcpy(buffer + offset, &pinB, sizeOfInt);
    offset += sizeOfInt;

    debug(1, "[serialize led strip] id: %d, pinR: %d, pinG: %d, pinB: %d", id, pinR, pinG, pinB);

    return offset;
}

size_t LedStrip::deserialize(int id, const uint8_t *buffer)
{
    size_t offset = 0;

    // Pin R
    memcpy(&pinR, buffer + offset, sizeOfInt);
    offset += sizeOfInt;
    // Pin G
    memcpy(&pinG, buffer + offset, sizeOfInt);
    offset += sizeOfInt;
    // Pin B
    memcpy(&pinB, buffer + offset, sizeOfInt);
    offset += sizeOfInt;

    debug(1, "[deserialize led strip] id: %d, pinR: %d, pinG: %d, pinB: %d", id, pinR, pinG, pinB);

    return offset;
}
