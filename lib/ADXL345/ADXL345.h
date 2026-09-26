// Doc: https://www.analog.com/media/en/technical-documentation/data-sheets/ADXL345.pdf

#pragma once
#include <stdint.h>
#include "ISensor.h"
#include "I2CBus.h"

namespace ADXL345_
{
    constexpr uint8_t I2C_ADDR = 0x53;

    constexpr uint8_t DEVID_REG = 0x00;
    constexpr uint8_t DEVID_VAL = 0xE5;

    constexpr uint8_t RA_POWER_CTL = 0x2D;
    constexpr uint8_t MEASURE_BIT = 0x03;

    constexpr uint8_t RA_DATA_FORMAT = 0x31;
    constexpr uint8_t RANGE_BIT = 1;
    constexpr uint8_t RANGE_LEN = 2;
    constexpr uint8_t RANGE_2G = 0b00;
    constexpr float LSB_PER_G_2G  = 256.0f;

    constexpr uint8_t RA_ACCEL_DATA = 0x32;
    constexpr uint8_t RA_ACCEL_DATA_LEN = 6; 
}

class ADXL345 : public ISensor
{
public:
    bool begin() override;
    bool update() override;
    const char *name() const override;

    float getAccelX() const { return _x; };
    float getAccelY() const { return _y; };
    float getAccelZ() const { return _z; };

private:
    int16_t _rawX = 0;
    float _x = 0.0f;
    int16_t _rawY = 0;
    float _y = 0.0f;
    int16_t _rawZ = 0;
    float _z = 0.0f;
};