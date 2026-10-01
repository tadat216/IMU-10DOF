#pragma once
#include <stdint.h>
#include <Wire.h>

namespace I2CBus {
    bool writeBit(uint8_t devAdd, uint8_t regAdd, uint8_t value, uint8_t bit_index, TwoWire &wireObj = Wire);
    bool writeByte(uint8_t devAdd, uint8_t regAdd, uint8_t value, TwoWire &wireObj = Wire);
    bool readByte(uint8_t devAdd, uint8_t regAdd, uint8_t &data, TwoWire &wireObj = Wire);
    bool readBytes(uint8_t devAdd, uint8_t regAdd, uint8_t len, uint8_t *data, TwoWire &wireObj = Wire);
    bool readBit(uint8_t devAdd, uint8_t regAdd, uint8_t bit_index, uint8_t &data, TwoWire &wireObj = Wire);
}
