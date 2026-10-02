#include "I2CBus.h"

bool I2CBus::writeBit(uint8_t devAdd, uint8_t regAdd, uint8_t bitIndex, uint8_t value, TwoWire &wireObj){
    return writeBits(devAdd, regAdd, bitIndex, 1, value, wireObj);
}

bool I2CBus::writeBits(uint8_t devAdd, uint8_t regAdd, uint8_t bitStart, uint8_t len, uint8_t value, TwoWire &wireObj){
    uint8_t reg = 0;
    if(!readByte(devAdd, regAdd, reg, wireObj)) return false;
    uint8_t mask = (1 << len) - 1 << bitStart;
    reg = (reg & ~mask) | (value << bitStart);
    return writeByte(devAdd, regAdd, reg, wireObj);
}

bool I2CBus::writeByte(uint8_t devAdd, uint8_t regAdd, uint8_t value, TwoWire &wireObj){
    wireObj.beginTransmission(devAdd);
    wireObj.write(regAdd);
    wireObj.write(value);
    uint8_t error = wireObj.endTransmission();
    if(error){
        return false;
    }
    return true;
}

bool I2CBus::readByte(uint8_t devAdd, uint8_t regAdd, uint8_t &data, TwoWire &wireObj){
    return readBytes(devAdd, regAdd, 1, &data, wireObj);
}

bool I2CBus::readBytes(uint8_t devAdd, uint8_t regAdd, uint8_t len, uint8_t *data, TwoWire &wireObj){
    wireObj.beginTransmission(devAdd);
    wireObj.write(regAdd);
    uint8_t error = wireObj.endTransmission(false);

    if(error){
        return false;
    }
    
    // requestFrom được gọi bên trong TwoWire có param stop = true
    // Sau khi gọi hàm requestFrom thì sẽ nhả bus
    if(wireObj.requestFrom(devAdd, len) != len){
        return false;
    }

    for(uint8_t count = 0; count < len; count++){
        data[count] = wireObj.read();
    }
    
    return true;
}

bool I2CBus::readBit(uint8_t devAdd, uint8_t regAdd, uint8_t bitIndex, uint8_t &data, TwoWire &wireObj){
    return readBits(devAdd, regAdd, bitIndex, 1, data, wireObj);
}

bool I2CBus::readBits(uint8_t devAdd, uint8_t regAdd, uint8_t bitStart, uint8_t len, uint8_t &data, TwoWire &wireObj){
    bool success = readByte(devAdd, regAdd, data, wireObj);
    data &= (1 << len) - 1 << bitStart;
    data >>= bitStart;
    return success;
}