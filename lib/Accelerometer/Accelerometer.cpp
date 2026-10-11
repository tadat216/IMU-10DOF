#include <Arduino.h>
#include <Preferences.h>
#include <math.h>
#include "Accelerometer.h"

bool Accelerometer::begin(){
    if(!chip.testConnection()) return false;
    // Cấu hình khi còn ở standby, rồi mới bật measure
    if(!chip.setRange(chip.RANGE_2G)) return false;
    if(!chip.setFullResolution(true)) return false;
    if(!chip.setDataRate(chip.RATE_100HZ)) return false;
    if(!chip.setMeasureMode(true)) return false;
    // Turn-on time ~ 1.1 ms + 1/ODR (~11 ms ở 100 Hz)
    delay(SETTLE_MS);
    return true;
}

/**
 * Chờ chip báo có mẫu mới (DATA_READY), poll flag INT_SOURCE.
 * false nếu I2C lỗi hoặc quá timeout_ms mà chưa có mẫu mới.
 */
bool Accelerometer::waitDataReady(uint32_t timeout_ms){
    uint32_t start = millis();
    bool ready = false;
    while(!ready){
        if(!chip.getDataReadyStatus(ready)) return false;
        if(!ready && millis() - start > timeout_ms) return false;
    }
    return true;
}

bool Accelerometer::update(){
    int16_t raw[3];
    if(!chip.getAccelRawData(raw[0], raw[1], raw[2])) return false;
    for(int i = 0; i < 3; i++){
        accel[i] = (raw[i] / LSB_PER_G - offset[i]) / scale[i];
    }
    return true;
}

/**
 * Chờ user xoay chip sao cho trục `axis` hướng lên (isPositive) hoặc xuống,
 * rồi giữ yên, trả về giá trị trung bình của trục đó (g) qua `data`.
 *
 * Một mẫu hợp lệ khi trục đích > CAL_AXIS_MIN_G đúng chiều và hai trục còn lại
 * < CAL_OTHER_MAX_G. Cần CAL_N_SAMPLE mẫu hợp lệ liên tiếp, trong đó max - min
 * của mỗi trục <= CAL_RANGE_MAX_G. Vi phạm điều kiện nào thì bỏ cửa sổ cũ.
 * false nếu I2C lỗi hoặc quá CAL_TIMEOUT_MS mà chưa đủ mẫu.
 * Chỉ ghi `data` khi thành công.
 */
bool Accelerometer::calibrateSingleDir(uint8_t axis, bool isPositive, float &data){
    enum { WRONG_DIR, SHAKING, HOLDING };

    const float dir = isPositive ? 1.0f : -1.0f;
    float sum[3] = {}, lo[3] = {}, hi[3] = {};
    uint16_t count = 0;

    uint32_t lastPrintMs = 0;
    uint32_t start = millis();

    while(millis() - start < CAL_TIMEOUT_MS){
        if(!waitDataReady(READY_TIMEOUT_MS)) return false;
        if(!update()) return false;

        // Hướng: trục chính đúng chiều, hai trục còn lại gần 0
        bool oriented = dir * accel[axis] > CAL_AXIS_MIN_G;
        for(uint8_t i = 0; i < 3; i++){
            if(i != axis && fabsf(accel[i]) > CAL_OTHER_MAX_G) oriented = false;
        }

        int8_t state = HOLDING;
        if(!oriented){
            count = 0;
            state = WRONG_DIR;
        }else{
            // Rung: max - min vượt ngưỡng thì bắt đầu cửa sổ mới từ mẫu này
            for(uint8_t i = 0; i < 3 && count > 0; i++){
                if(max(hi[i], accel[i]) - min(lo[i], accel[i]) > CAL_RANGE_MAX_G){
                    count = 0;
                    state = SHAKING;
                }
            }
            if(count == 0){
                for(uint8_t i = 0; i < 3; i++){
                    sum[i] = 0;
                    lo[i] = hi[i] = accel[i];
                }
            }
            for(uint8_t i = 0; i < 3; i++){
                sum[i] += accel[i];
                lo[i] = min(lo[i], accel[i]);
                hi[i] = max(hi[i], accel[i]);
            }
            count++;
        }

        if(count >= CAL_N_SAMPLE){
            data = sum[axis] / count;
            return true;
        }

        // In giá trị x, y, z kèm trạng thái, tối đa mỗi CAL_PRINT_INTERVAL_MS để không làm chậm vòng đọc
        if(millis() - lastPrintMs >= CAL_PRINT_INTERVAL_MS){
            Serial.printf("x=%+.2f y=%+.2f z=%+.2f g | ", accel[0], accel[1], accel[2]);
            switch(state){
                case WRONG_DIR:
                    Serial.printf("Chưa đúng hướng, hãy xoay chip sao cho %c = %c1.0 (hai trục còn lại ~ 0)\n",
                                  "xyz"[axis], isPositive ? '+' : '-');
                    break;
                case SHAKING:
                    Serial.println("Chip đang rung, hãy giữ yên");
                    break;
                default:
                    Serial.println("Đúng hướng, giữ yên...");
            }
            lastPrintMs = millis();
        }
    }
    return false;
}

/**
 * @brief
 * Giả sử cảm biến có sai số trên mỗi trục theo kiểu tuyến tính:
 * read_value = scale * correct_value + offset (y = ax + b)
 * Với một trục cố định, khi đặt hướng lên hoặc xuống trùng phương với trục
 * thì lúc này correct_value = -1 (xuống) hoặc +1 (lên), ta có phương trình:
 * - Hướng lên: up = scale + offset
 * - Hướng xuống: down = -scale + offset 
 * 
 * Phương trình có 2 ẩn  scale và offset:
 * - Cộng 2 vế: up + down = 2offset, suy ra offset = (up + down) / 2
 * - Trừ 2 vế: up - down = 2scale, suy ra scale = (up - down) / 2
 *
 * Giá trị đã hiệu chỉnh: correct_value = (read_value - offset) / scale
 *
 * Cách để lấy up và down:
 * - Lần lượt đặt cảm biến theo 6 góc x, y, z và hướng lên &
 * xuống sao cho giá trị đúng theo hướng dẫn trong và giữ nguyên vài giây để lấy 1 
 * số lượng mẫu và tính trung bình để ra giá trị tương ứng của trục đó.
 * 
 * @param maxFailPerAxis số lần retry khi hiệu chỉnh mỗi trục
 * @param maxFailTotal số lần retry tổng cộng
 */
bool Accelerometer::calibrate(uint8_t maxFailPerAxis, uint8_t maxFailTotal){
    // update() đang áp offset/scale cũ, nên phải về mặc định để lấy giá trị thô.
    // Giữ bản cũ để khôi phục nếu hiệu chỉnh thất bại.
    float oldOffset[3], oldScale[3];
    for(uint8_t i = 0; i < 3; i++){
        oldOffset[i] = offset[i];
        oldScale[i] = scale[i];
        offset[i] = 0;
        scale[i] = 1;
    }

    uint8_t totalFailed = 0;
    const char *axs = "xyz";
    const char *sign = "-+";
    for(uint8_t a = 0; a < 3; a++){
        float data[2];
        for(uint8_t p = 0; p < 2; p++){
            uint8_t retry = 0;
            for(int f = 0; f < maxFailPerAxis; f++){  
                Serial.printf("Vui lòng hiệu chỉnh để giá trị g ở trục %c gần với %c1.0 nhất, sau đó giữ nguyên chip để lấy mẫu", axs[a], sign[p]);
                
                if(calibrateSingleDir(a, p, data[p])) break;
                
                Serial.printf("Lấy mẫu thất bại, thử lại %d/%d (Max %d/%d)\n", f+1, maxFailPerAxis, totalFailed, maxFailTotal);
                
                totalFailed++;
                if(totalFailed == maxFailTotal){
                    Serial.println("Hiệu chỉnh thất bại");
                    for(uint8_t i = 0; i < 3; i++){
                        offset[i] = oldOffset[i];
                        scale[i] = oldScale[i];
                    }
                    return false;
                }
            }
            Serial.printf("Lấy mẫu hoàn thành!");
        }
        offset[a] = (data[0] + data[1]) / 2;
        scale[a] = (data[1] - data[0]) / 2;
    }
    Serial.println("Hiệu chỉnh hoàn thành");
    // Ghi flash thất bại thì giá trị trong RAM vẫn đúng, nên không coi là hiệu chỉnh thất bại
    if(saveCalibration()) Serial.println("Đã lưu hiệu chỉnh vào flash");
    else Serial.println("Cảnh báo: không lưu được hiệu chỉnh vào flash");
    return true;
}

bool Accelerometer::saveCalibration(){
    CalData data;
    data.version = CAL_VERSION;
    for(uint8_t i = 0; i < 3; i++){
        data.offset[i] = offset[i];
        data.scale[i] = scale[i];
    }

    Preferences prefs;
    if(!prefs.begin(CAL_NAMESPACE, false)) return false;
    size_t written = prefs.putBytes(CAL_KEY, &data, sizeof(data));
    prefs.end();
    return written == sizeof(data);
}

/**
 * Đọc vào struct tạm, kiểm tra xong mới chép vào offset/scale,
 * nên nếu thất bại thì giá trị hiện tại của class không bị thay đổi.
 */
bool Accelerometer::loadCalibration(){
    Preferences prefs;
    if(!prefs.begin(CAL_NAMESPACE, true)) return false;

    CalData data;
    bool ok = prefs.getBytesLength(CAL_KEY) == sizeof(data)
           && prefs.getBytes(CAL_KEY, &data, sizeof(data)) == sizeof(data);
    prefs.end();
    
    if(!ok || data.version != CAL_VERSION) return false;

    for(uint8_t i = 0; i < 3; i++){
        if(!isfinite(data.offset[i])) return false;
    }

    for(uint8_t i = 0; i < 3; i++){
        offset[i] = data.offset[i];
        scale[i] = data.scale[i];
    }
    return true;
}

const char *Accelerometer::name() const {
    return "ADXL345";
}

void Accelerometer::getAccel(float &x, float &y, float &z) const {
    x = accel[0];
    y = accel[1];
    z = accel[2];
}
