#include <ITG3205.h>

bool ITG3205::begin(){
    uint8_t wai_val = 0;
    if(!I2CBus::readByte(DEV_ADD, REG_WHO_AM_I, wai_val)){
        return false;
    }
    // Chỉ Bit6..Bit1 là ID (110100), bit 7 và bit 0 không xác định
    if((wai_val & 0x7E) != DEV_ADD){
        return false;
    }
    offset_x = offset_y = offset_z = 0;
    // B1. Config các value
    // Set f_sample
    if(!I2CBus::writeByte(DEV_ADD, REG_SMPLRT_DIV, SMPLRT_DIV)){
        return false;
    }
    // Set scale range and digital low pass filter configuration
    if(!I2CBus::writeByte(DEV_ADD, REG_DLPF_FS, (FS_SEL << FS_SEL_SHIFT) | DLPF_CFG)){
        return false;
    }
    if(!I2CBus::writeBit(DEV_ADD, REG_INT_CFG, 1, BIT_RAW_RDY_EN)){
        return false;
    }
    if(!I2CBus::writeByte(DEV_ADD, REG_PWR_MGM, CLK_SEL_VAL)){
        return false;
    }

    // B2. Tính offset
    // F_sample = 200 Hz -> 1/200 = 0.005s = 5ms
    // Chờ ESP32 chạy được ít nhất BOOT_WAIT_MS để chip ổn định sau khi cấp nguồn
    // PLL (1ms) và ZRO (50ms) cần ổn định sau khi đổi CLK_SEL ở trên
    delay(SETTLE_MS);
    uint32_t start_sample_time = millis();
    int16_t sample_taken = 0;
    float s_x = 0, s_y = 0, s_z = 0;
    Serial.print("Bắt đầu quá trình tính toán offset con quay, vui lòng để chip đứng yên\n");
    while(sample_taken < N_SAMPLE){
        uint8_t is_sample_ready = 0;
        // Time limit
        if(millis() - start_sample_time > N_SAMPLE * 10){
            return false;
        }
        if(!I2CBus::readBit(DEV_ADD, REG_INT_STATUS, BIT_RAW_DATA_RDY, is_sample_ready)){
            return false;
        }
        if(!is_sample_ready){
            delay(1);
            continue;
        }
        if(!update()){
            return false;
        }
        s_x += gyro_x;
        s_y += gyro_y;
        s_z += gyro_z;
        sample_taken++;
        delay(1);
    }
    offset_x = s_x / N_SAMPLE;
    offset_y = s_y / N_SAMPLE;
    offset_z = s_z / N_SAMPLE;
    Serial.print("Hoàn thành lấy offset\n");
    return true;
}

void ITG3205::getGyro(float &x, float &y, float &z){
    x = gyro_x;
    y = gyro_y;
    z = gyro_z;
}

bool ITG3205::update(){
    uint8_t data[GYRO_LEN];
    if(!I2CBus::readBytes(DEV_ADD, REG_GYRO, GYRO_LEN, data)) return false;
    gyro_x = int16_t((data[0] << 8) | data[1]) / LSB - offset_x;
    gyro_y = int16_t((data[2] << 8) | data[3]) / LSB - offset_y;
    gyro_z = int16_t((data[4] << 8) | data[5]) / LSB - offset_z;
    return true;
}

const char* ITG3205::name() const {
    return "ITG3205";
}
