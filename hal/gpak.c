#include "gpak.h"

#define PAGE_LIMIT 0x0E
#define PAGE_DELAY 25

static uint8_t _bus;
static uint8_t _slave_address;

void gpak_init(uint8_t bus, uint8_t slave_address) {
    _bus = bus;
    _slave_address = slave_address;
}

uint8_t gpak_i2caddr(uint8_t area) {
    if(!_slave_address) return 0x88;
    else return (_slave_address << 3) + area;
}

void gpak_read(uint8_t area, uint8_t offset, uint8_t* data, uint16_t length) {
    uint8_t addr = gpak_i2caddr(area);
    while(length > 0) {
        uint8_t xferlen = (length > 16) ? 16 : length;
        I2C0ReadCmd(addr, &offset, 1, data, xferlen);
        offset += xferlen;
        length -= xferlen;
        data += xferlen;
    }
}

void gpak_write(uint8_t area, uint8_t offset, uint8_t* data, uint16_t length) {
    uint8_t addr = gpak_i2caddr(area);
    //if((offset + length) >= (PAGE_LIMIT << 4)) return;
    while(length > 0) {
        uint8_t xferlen = (length > 16) ? 16 : length;
        I2C0SendCmd(addr, &offset, 1, data, xferlen);
        WaitMSDMTimer(PAGE_DELAY);
        offset += xferlen;
        length -= xferlen;
        data += xferlen;
    }
}

void gpak_write_reg(uint8_t reg, uint8_t value) {
    uint8_t addr = gpak_i2caddr(REG_CONFIG);
    uint8_t data[1] = { value };
    I2C0SendCmd(addr, &reg, 1, data, 1);
}

uint8_t gpak_read_reg(uint8_t reg) {
    uint8_t addr = gpak_i2caddr(REG_CONFIG);
    uint8_t data[1] = { 0 };
    I2C0ReadCmd(addr, &reg, 1, data, 1);
    return data[0];
}

void gpak_erase(uint8_t area) {
    static const uint8_t cmd = 0xE3;
    uint8_t addr = gpak_i2caddr(area);
    uint8_t data[1] = { 0 };
    for(int i=0; i<16; i++) {
        //if(i >= PAGE_LIMIT) break;
        if(area == NVM_CONFIG) data[0] = 0x80 + i;
        else if(area == EEPROM) data[0] = 0x90 + i;
        I2C0SendCmd(addr, &cmd, 1, data, 1);
        WaitMSDMTimer(PAGE_DELAY);
    }
}
