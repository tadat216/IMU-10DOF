#include "I2CBus.h"

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
