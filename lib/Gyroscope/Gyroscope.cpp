#include <Arduino.h>
#include "Gyroscope.h"

bool Gyroscope::begin(){
    if(!chip.testConnection()) return false;
    if(!chip.setDLPFBandWidth(chip.DLPF_CFG_BW42)) return false;
    if(!chip.setSampleRateDivider(SMPLRT_DIV)) return false;
    if(!chip.setFullScaleRange(chip.FS_SEL_2000)) return false;
    if(!chip.setIntDataReadyEnableMode(true)) return false;
    if(!chip.setDeviceClockSource(chip.CLK_SEL_PLL_X)) return false;
    return true;
}

/**
 * Chờ chip báo có mẫu mới (RAW_DATA_RDY), poll flag INT_STATUS.
 * false nếu I2C lỗi hoặc quá timeout_ms mà chưa có mẫu mới.
 */
bool Gyroscope::waitDataReady(uint32_t timeout_ms){
    uint32_t start = millis();
    bool ready = false;
    while(!ready){
        if(!chip.getIntDataReadyStatus(ready)) return false;
        if(!ready && millis() - start > timeout_ms) return false;
    }
    return true;
}

/**
 * Đo offset lúc đứng yên: trung bình N_SAMPLE mẫu raw, đổi sang °/s.
 * Phải giữ cảm biến đứng yên trong lúc gọi (~N_SAMPLE * (SMPLRT_DIV + 1) ms).
 * Mỗi mẫu chỉ đọc sau khi chip báo data mới, nên không sót và không đọc trùng.
 * Chỉ ghi offset khi đọc đủ mọi mẫu, lỗi giữa chừng thì giữ offset cũ.
 */
bool Gyroscope::calibrateOffset(){
    // Chờ clock PLL và bộ lọc DLPF ổn định
    delay(SETTLE_MS);

    // |raw| <= 32768, N_SAMPLE = 1000 nên int32_t không tràn
    int32_t sum[3] = {};
    for(int16_t i = 0; i < N_SAMPLE; i++){
        int16_t raw[3] = {};
        if(!waitDataReady(READY_TIMEOUT_MS)) return false;
        if(!chip.getGyroRawData(raw[0], raw[1], raw[2])) return false;
        for(uint8_t j = 0; j < 3; j++){
            sum[j] += raw[j];
        }
    }

    for(uint8_t i = 0; i < 3; i++){
        offset[i] = 1.0f * sum[i] / N_SAMPLE / LSB_PER_DPS;
    }

    return true;
}

bool Gyroscope::update(){
    int16_t raw[3] = {};
    if(!chip.getGyroRawData(raw[0], raw[1], raw[2])) return false;
    for(uint8_t i = 0; i < 3; i++){
        gyro[i] = (raw[i] / LSB_PER_DPS) - offset[i];
    }
    return true;
}

void Gyroscope::getGyro(float &x, float &y, float &z) const{
    x = gyro[0], y = gyro[1], z = gyro[2];
}