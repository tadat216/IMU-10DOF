#pragma once
#include <stdint.h>
#include <Wire.h>

namespace I2CBus {
    bool writeBits(uint8_t devAdd, uint8_t regAdd, uint8_t bitStart, uint8_t len, uint8_t value, TwoWire &wireObj = Wire);
    bool writeBit(uint8_t devAdd, uint8_t regAdd, uint8_t bitIndex, uint8_t value, TwoWire &wireObj = Wire);
    bool writeByte(uint8_t devAdd, uint8_t regAdd, uint8_t value, TwoWire &wireObj = Wire);
    bool readByte(uint8_t devAdd, uint8_t regAdd, uint8_t &data, TwoWire &wireObj = Wire);
    bool readBytes(uint8_t devAdd, uint8_t regAdd, uint8_t len, uint8_t *data, TwoWire &wireObj = Wire);
    bool readBit(uint8_t devAdd, uint8_t regAdd, uint8_t bitIndex, uint8_t &data, TwoWire &wireObj = Wire);
    bool readBits(uint8_t devAdd, uint8_t regAdd, uint8_t bitStart, uint8_t len, uint8_t &data, TwoWire &wireObj = Wire);
}
