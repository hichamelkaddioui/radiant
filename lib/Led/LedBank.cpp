#include <Utils.h>
#include <NeoPixel.h>
#include <LedStrip.h>
#include <LedBank.h>

void LedBank::setup()
{
    for (const auto &it : _leds)
    {
        it.second->setup();
    }
}

size_t LedBank::serialize(uint8_t *buffer) const
{
    size_t offset = 0, ledCount = 0;
    offset += sizeOfSizeT;

    debug(1, "[serialize led bank] serializing, %lu leds in bank", _leds.size());

    for (const auto &it : _leds)
    {
        Led *led = it.second;

        if (led == nullptr)
            continue;

        // Type
        LedType ledType = led->getType();
        memcpy(buffer + offset, &ledType, sizeof(LedType));
        offset += sizeof(LedType);

        // ID
        int id = it.first;
        memcpy(buffer + offset, &id, sizeOfInt);
        offset += sizeOfInt;

        // Led
        offset += led->serialize(id, buffer + offset);

        ledCount++;
    }

    debug(1, "[serialize led bank] %lu leds serialized", ledCount);
    memcpy(buffer, &ledCount, sizeOfSizeT);

    return offset;
}

size_t LedBank::deserialize(const uint8_t *buffer)
{
    size_t offset = 0, ledCount = 0;

    memcpy(&ledCount, buffer + offset, sizeOfSizeT);
    offset += sizeOfSizeT;
    debug(1, "[deserialize led bank] deserializing %lu leds", ledCount);

    for (size_t i = 0; i < ledCount; i++)
    {
        // Type
        LedType ledType;
        memcpy(&ledType, buffer + offset, sizeof(LedType));
        offset += sizeof(LedType);

        // ID
        int id = 0;
        memcpy(&id, buffer + offset, sizeOfInt);
        offset += sizeOfInt;

        // Led
        switch (ledType)
        {
        case LedType::LED_NEOPIXEL:
        {
            NeoPixel *pixel = new NeoPixel();
            offset += pixel->deserialize(id, buffer + offset);
            _leds[id] = pixel;
            break;
        }
        case LedType::LED_STRIP:
        {
            LedStrip *strip = new LedStrip();
            offset += strip->deserialize(id, buffer + offset);
            _leds[id] = strip;
            break;
        }
        }
    }

    return offset;
}

void LedBank::sysExCreate(const uint8_t *buffer, size_t length)
{
    debug(1, "[SysEx] [led] reading %lu bytes", length);

    int lightId = static_cast<int>(buffer[0]);
    int pinR = static_cast<int>(buffer[1]);
    int pinG = static_cast<int>(buffer[2]);
    int pinB = static_cast<int>(buffer[3]);

    LedStrip *strip = new LedStrip(pinR, pinG, pinB);

    const auto it = _leds.find(lightId);

    if (it != _leds.end())
    {
        delete it->second;

        debug(1, "[SysEx] [led] deleted previous led with id %d", lightId);
    }
    else
    {
        debug(1, "[SysEx] [led] no previous led with id %d", lightId);
    }

    _leds[lightId] = strip;

    debug(1, "[SysEx] [led] stored led id %d, pin R %d, pin G %d, pin B %d", lightId, pinR, pinG, pinB);
}