#include "ADXL345.h"

bool ADXL345::begin() {
    uint8_t dev_data = 0;
    if(!I2CBus::readByte(ADXL345_::I2C_ADDR, ADXL345_::DEVID_REG, dev_data) || dev_data != ADXL345_::DEVID_VAL){
        return false;
    }
    // Chỉnh range
    if(!I2CBus::writeByte(ADXL345_::I2C_ADDR, ADXL345_::RA_DATA_FORMAT, ADXL345_::RANGE_2G)){
        return false;
    }
    // Bật measure mode
    if(!I2CBus::writeByte(ADXL345_::I2C_ADDR, ADXL345_::RA_POWER_CTL, (1 << ADXL345_::MEASURE_BIT))){
        return false;
    }
    return true;
}

bool ADXL345::update() {
    uint8_t dev_data = 0;
    if(!I2CBus::readByte(ADXL345_::I2C_ADDR, ADXL345_::DEVID_REG, dev_data) || dev_data != ADXL345_::DEVID_VAL){
        return false;
    }
    uint8_t buffer[6];
    if(!I2CBus::readBytes(ADXL345_::I2C_ADDR, ADXL345_::RA_ACCEL_DATA, ADXL345_::RA_ACCEL_DATA_LEN, buffer)){
        return false;
    }
    
    _rawX = (buffer[1] << 8) | buffer[0];
    _rawY = (buffer[3] << 8) | buffer[2];
    _rawZ = (buffer[5] << 8) | buffer[4];
    _x = _rawX / ADXL345_::LSB_PER_G_2G;
    _y = _rawY / ADXL345_::LSB_PER_G_2G;
    _z = _rawZ / ADXL345_::LSB_PER_G_2G;
    return true;
}

const char* ADXL345::name() const {
    return "ADXL345";
}
