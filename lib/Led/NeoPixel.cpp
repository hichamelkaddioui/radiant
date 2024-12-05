#include <Utils.h>
#include <NeoPixel.h>

NeoPixel::~NeoPixel()
{
    delete _strip;
}

void NeoPixel::setup()
{
    _strip = new Adafruit_NeoPixel(1, _pin);
    _strip->begin();
    _strip->show();

    debug(1, "[setup neo pixel] pin: %d", _strip->getPin());
}

void NeoPixel::setRgb(uint8_t r, uint8_t g, uint8_t b)
{
    _strip->setPixelColor(0, Adafruit_NeoPixel::Color(r, g, b));
    _strip->show();
}

size_t NeoPixel::serialize(int id, uint8_t *buffer) const
{
    size_t offset = 0;

    // Pin
    memcpy(buffer + offset, &_pin, sizeOfInt);
    offset += sizeOfInt;

    debug(1, "[serialize neo pixel] id: %d, pin: %d", id, _pin);

    return offset;
}

size_t NeoPixel::deserialize(int id, const uint8_t *buffer)
{
    size_t offset = 0;

    // Pin
    memcpy(&_pin, buffer + offset, sizeOfInt);
    offset += sizeOfInt;

    debug(1, "[deserialize neo pixel] id: %d, pin: %d", id, _pin);

    return offset;
}