#include <Utils.h>
#include <NeoPixel.h>
#include <Led.h>

size_t serializeNeoPixel(Led *led, uint8_t *buffer, int id)
{
    size_t offset = 0;

    NeoPixel *pixel = dynamic_cast<NeoPixel *>(const_cast<Led *>(led));

    if (pixel == nullptr)
        return 0;

    // Type
    LedType neoPixelType = LedType::LED_NEOPIXEL;
    memcpy(buffer + offset, &neoPixelType, sizeof(LedType));
    offset += sizeof(LedType);

    // ID
    memcpy(buffer + offset, &id, sizeOfInt);
    offset += sizeOfInt;

    // Pin
    memcpy(buffer + offset, &pixel->_pin, sizeOfInt);
    offset += sizeOfInt;

    debug(1, "[serialize neo pixel] id: %d, pin: %d", id, pixel->_pin);

    return offset;
}

size_t deserializeNeoPixel(NeoPixel &pixel, const uint8_t *buffer)
{
    size_t offset = 0;

    int pin = 0;
    memcpy(&pin, buffer + offset, sizeOfInt);
    offset += sizeOfInt;
    pixel._pin = pin;

    return offset;
}

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
        int type = led->getType();
        int id = it.first;

        if (type == LedType::LED_NEOPIXEL)
        {
            offset += serializeNeoPixel(led, buffer + offset, id);

            ledCount++;
        }
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
            offset += deserializeNeoPixel(*pixel, buffer + offset);
            debug(1, "[deserialized neo pixel] id: %d, pin: %d", id, pixel->_pin);

            _bank[id] = pixel;
        }
    }

    return offset;
}