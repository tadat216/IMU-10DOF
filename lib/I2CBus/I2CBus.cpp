#include "I2CBus.h"

bool I2CBus::writeBit(uint8_t devAdd, uint8_t regAdd, uint8_t value, uint8_t bit_index, TwoWire &wireObj){
    // Đọc - sửa bit - ghi lại để không xoá các bit khác
    uint8_t reg = 0;
    if(!readByte(devAdd, regAdd, reg, wireObj)){
        return false;
    }
    if(value){
        reg |= (1 << bit_index);
    }
    else{
        reg &= ~(1 << bit_index);
    }
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

bool I2CBus::readBit(uint8_t devAdd, uint8_t regAdd, uint8_t bit_index, uint8_t &data, TwoWire &wireObj){
    if(readBytes(devAdd, regAdd, 1, &data, wireObj)){
        data &= (1 << bit_index);
        return true;
    }
    return false;
}