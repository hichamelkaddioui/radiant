#ifndef FLASH_H
#define FLASH_H

#include <Adafruit_SPIFlash.h>

class RP2040Flash
{
public:
    RP2040Flash() : _flash(Adafruit_SPIFlash(
                        new Adafruit_FlashTransport_RP2040(
                            Adafruit_FlashTransport_RP2040::CPY_START_ADDR,
                            Adafruit_FlashTransport_RP2040::CPY_SIZE))) {}
    void begin();
    u_int8_t *read(uint32_t address, uint32_t length);
    void write(uint32_t address, uint8_t *data, uint32_t length);

private:
    Adafruit_SPIFlash _flash;
};

#endif