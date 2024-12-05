#include <Utils.h>
#include <NeoPixel.h>
#include <Led.h>

void LedBank::setup()
{
    for (const auto &it : _bank)
    {
        it.second->setup();
    }
}

void LedBank::clear()
{
    for (const auto &it : _bank)
    {
        delete it.second;
    }

    _bank.clear();
}

size_t LedBank::serialize(uint8_t *buffer) const
{
    size_t offset = 0, ledCount = 0;
    offset += sizeOfSizeT;

    debug(1, "[serialize led bank] serializing, %lu leds in bank", _bank.size());

    for (const auto &it : _bank)
    {
        Led *led = it.second;

        if (led == nullptr)
            continue;

        int type = led->getType();
        int id = it.first;

        // Type
        LedType ledType = led->getType();
        memcpy(buffer + offset, &ledType, sizeof(LedType));
        offset += sizeof(LedType);

        // ID
        memcpy(buffer + offset, &id, sizeOfInt);
        offset += sizeOfInt;

        // Led
        offset += led->serialize(id, buffer + offset);

        debug(1, "[serialize led] id: %d", id);

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
        LedType ledType;
        memcpy(&ledType, buffer + offset, sizeof(LedType));
        offset += sizeof(LedType);

        if (ledType == LedType::LED_NEOPIXEL)
        {
            int id = 0;
            memcpy(&id, buffer + offset, sizeOfInt);
            offset += sizeOfInt;

            NeoPixel *pixel = new NeoPixel();
            offset += pixel->deserialize(id, buffer + offset);

            _bank[id] = pixel;
        }
    }

    return offset;
}