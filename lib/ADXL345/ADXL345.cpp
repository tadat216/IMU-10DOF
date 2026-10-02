#include "ADXL345.h"

// Register DEVID

/**
 * Đọc DEVID và so với giá trị cố định 0xE5.
 * false nếu I2C lỗi hoặc ID không khớp.
 */
bool ADXL345::testConnection(){
    uint8_t id = 0;
    return getDeviceId(id) && id == DEVID_VAL;
}

/**
 * Giá trị mong đợi là 0xE5
 */
bool ADXL345::getDeviceId(uint8_t &id){
    return I2CBus::readByte(DEV_ADD, REG_DEVID, id);
}

// Register DATA_FORMAT

/**
 * Range [1:0]
 * | Range | Measurement Range |
 * |-|-|
 * | 0 | ±2g  |
 * | 1 | ±4g  |
 * | 2 | ±8g  |
 * | 3 | ±16g |
 * @see RANGE_*
 */
bool ADXL345::setRange(uint8_t range){
    return I2CBus::writeBits(DEV_ADD, REG_DATA_FORMAT, RANGE_IDX, RANGE_LEN, range);
}

// Register POWER_CTL

/**
 * Measure [3]
 * 0: standby mode (mặc định khi bật nguồn)
 * 1: measurement mode
 */
bool ADXL345::setMeasureMode(bool enable){
    return I2CBus::writeBit(DEV_ADD, REG_POWER_CTL, MEASURE_IDX, enable);
}

// Accel registers

/**
 * Lấy raw 16 bit của accel, little-endian (byte Low trước, byte High sau)
 */
bool ADXL345::getAccelRawData(int16_t &x, int16_t &y, int16_t &z){
    uint8_t data[ACCEL_LEN];
    if(!I2CBus::readBytes(DEV_ADD, REG_ACCEL, ACCEL_LEN, data)) return false;
    x = (int16_t)((data[1] << 8) | data[0]);
    y = (int16_t)((data[3] << 8) | data[2]);
    z = (int16_t)((data[5] << 8) | data[4]);
    return true;
}
