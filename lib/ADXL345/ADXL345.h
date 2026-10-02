// Doc: https://www.analog.com/media/en/technical-documentation/data-sheets/ADXL345.pdf

#pragma once
#include <stdint.h>
#include "I2CBus.h"

class ADXL345 {
public:

    // --------------- Giá trị truyền vào các hàm set ---------------

    // Range [1:0]: Measurement range
    static constexpr uint8_t RANGE_2G  = 0x00; // ±2g
    static constexpr uint8_t RANGE_4G  = 0x01; // ±4g
    static constexpr uint8_t RANGE_8G  = 0x02; // ±8g
    static constexpr uint8_t RANGE_16G = 0x03; // ±16g

    // --------------- API ---------------

    // Register DEVID: đọc được và đúng giá trị mong đợi
    bool testConnection();

    // Register DEVID
    bool getDeviceId(uint8_t &id);

    // Register DATA_FORMAT
    bool setRange(uint8_t range);

    // Register POWER_CTL
    bool setMeasureMode(bool enable);

    // Registers DATAX0..DATAZ1
    bool getAccelRawData(int16_t &x, int16_t &y, int16_t &z);

private:

    // --------------- REGISTERs ---------------
    // Địa chỉ I2C 7-bit: 0x53 khi chân SDO/ALT ADDRESS nối GND, 0x1D khi nối mức cao
    static constexpr uint8_t DEV_ADD = 0x53;

    // Register 0x00 – DEVID
    static constexpr uint8_t REG_DEVID = 0x00;
    static constexpr uint8_t DEVID_VAL = 0xE5;

    // Register 0x2D – POWER_CTL
    static constexpr uint8_t REG_POWER_CTL = 0x2D;

    // Measure [3]
    static constexpr uint8_t MEASURE_IDX = 3;

    // Register 0x31 – DATA_FORMAT
    static constexpr uint8_t REG_DATA_FORMAT = 0x31;

    // Range [1:0]
    static constexpr uint8_t RANGE_IDX = 0;
    static constexpr uint8_t RANGE_LEN = 2;

    // Register 0x32..0x37 – 6 registers liên tiếp của accel (X0, X1, Y0, Y1, Z0, Z1)
    static constexpr uint8_t REG_ACCEL = 0x32;
    static constexpr uint8_t ACCEL_LEN = 6;
};
