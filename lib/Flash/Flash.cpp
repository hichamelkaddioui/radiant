#include <Utils.h>
#include <Flash.h>

void RP2040Flash::begin()
{
    _flash.begin();
    debug(1, "[flash] Flash initialized. JEDEC ID: 0x%08X, flash size: %lu MB", _flash.getJEDECID(), _flash.size() / (1024 * 1024));
}

void RP2040Flash::read(uint32_t address, uint8_t *data, uint32_t length)
{
    debug(1, "[flash] Reading %lu bytes from address 0x%08X", length, address);
    _flash.readBuffer(address, data, length);
}

void RP2040Flash::write(uint32_t address, const uint8_t *data, uint32_t length)
{
    debug(1, "[flash] Writing %lu bytes to address 0x%08X", length, address);
    _flash.eraseBlock(address / 4096);
    uint8_t *copy = new uint8_t[length];
    memcpy(copy, data, length);
    _flash.writeBuffer(address, copy, length);
}
