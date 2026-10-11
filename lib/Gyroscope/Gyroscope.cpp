#include <Arduino.h>
#include <Preferences.h>
#include <math.h>
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
 * Đo offset lúc đứng yên: trung bình N_SAMPLE mẫu raw liên tiếp, đổi sang °/s.
 * Phải giữ cảm biến đứng yên trong lúc gọi (~N_SAMPLE * (SMPLRT_DIV + 1) ms).
 * Mỗi mẫu chỉ đọc sau khi chip báo data mới, nên không sót và không đọc trùng.
 *
 * Phát hiện giữ yên: trong cửa sổ mẫu, max - min của mỗi trục phải <= STILL_RANGE_MAX_DPS.
 * Vượt ngưỡng (chip bị chạm, rung, xoay) thì bỏ cửa sổ cũ và đếm lại từ đầu.
 * false nếu I2C lỗi hoặc quá CAL_TIMEOUT_MS mà chưa có đủ N_SAMPLE mẫu yên.
 * Chỉ ghi offset khi thành công, nên thất bại thì giữ offset cũ.
 */
bool Gyroscope::calibrate(){
    // Chờ clock PLL và bộ lọc DLPF ổn định
    delay(SETTLE_MS);

    // |raw| <= 32768, N_SAMPLE = 1000 nên int32_t không tràn
    int32_t sum[3] = {};
    int16_t lo[3] = {}, hi[3] = {};
    int16_t count = 0;
    const int rangeMaxRaw = (int)(STILL_RANGE_MAX_DPS * LSB_PER_DPS);

    uint32_t lastPrintMs = 0;
    uint32_t start = millis();

    while(count < N_SAMPLE){
        if(millis() - start > CAL_TIMEOUT_MS) return false;

        int16_t raw[3] = {};
        if(!waitDataReady(READY_TIMEOUT_MS)) return false;
        if(!chip.getGyroRawData(raw[0], raw[1], raw[2])) return false;

        // Cửa sổ mới bắt đầu từ mẫu này
        if(count == 0){
            for(uint8_t j = 0; j < 3; j++){
                sum[j] = 0;
                lo[j] = hi[j] = raw[j];
            }
        }

        bool still = true;
        for(uint8_t j = 0; j < 3; j++){
            lo[j] = min(lo[j], raw[j]);
            hi[j] = max(hi[j], raw[j]);
            if(hi[j] - lo[j] > rangeMaxRaw) still = false;
        }

        if(!still){
            // In tối đa mỗi CAL_PRINT_INTERVAL_MS để không làm chậm vòng đọc
            if(millis() - lastPrintMs >= CAL_PRINT_INTERVAL_MS){
                Serial.printf("Chip đang chuyển động (max-min x=%.2f y=%.2f z=%.2f °/s, ngưỡng %.2f), hãy giữ yên\n",
                              (hi[0] - lo[0]) / LSB_PER_DPS, (hi[1] - lo[1]) / LSB_PER_DPS,
                              (hi[2] - lo[2]) / LSB_PER_DPS, STILL_RANGE_MAX_DPS);
                lastPrintMs = millis();
            }
            count = 0;
            continue;
        }

        for(uint8_t j = 0; j < 3; j++){
            sum[j] += raw[j];
        }
        count++;
    }

    for(uint8_t i = 0; i < 3; i++){
        offset[i] = 1.0f * sum[i] / N_SAMPLE / LSB_PER_DPS;
    }

    // Ghi flash thất bại thì offset trong RAM vẫn đúng, nên không coi là hiệu chỉnh thất bại
    if(saveCalibration()) Serial.println("Đã lưu offset gyro vào flash");
    else Serial.println("Cảnh báo: không lưu được offset gyro vào flash");
    return true;
}

bool Gyroscope::saveCalibration(){
    CalData data;
    data.version = CAL_VERSION;
    for(uint8_t i = 0; i < 3; i++){
        data.offset[i] = offset[i];
    }

    Preferences prefs;
    if(!prefs.begin(CAL_NAMESPACE, false)) return false;
    size_t written = prefs.putBytes(CAL_KEY, &data, sizeof(data));
    prefs.end();
    return written == sizeof(data);
}

/**
 * Đọc vào struct tạm, kiểm tra xong mới chép vào offset,
 * nên nếu thất bại thì offset hiện tại của class không bị thay đổi.
 */
bool Gyroscope::loadCalibration(){
    Preferences prefs;
    if(!prefs.begin(CAL_NAMESPACE, true)) return false;

    CalData data;
    bool ok = prefs.getBytesLength(CAL_KEY) == sizeof(data)
           && prefs.getBytes(CAL_KEY, &data, sizeof(data)) == sizeof(data);
    prefs.end();
    if(!ok || data.version != CAL_VERSION) return false;

    for(uint8_t i = 0; i < 3; i++){
        if(!isfinite(data.offset[i]) || fabsf(data.offset[i]) > CAL_OFFSET_MAX_DPS) return false;
    }

    for(uint8_t i = 0; i < 3; i++){
        offset[i] = data.offset[i];
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