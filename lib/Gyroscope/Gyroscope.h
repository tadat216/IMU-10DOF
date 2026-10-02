#pragma once
#include <stdint.h>
#include "ITG3205.h"

class Gyroscope {
public:
    bool begin();
    bool update();

    bool calibrateOffset();
    // Đơn vị: độ/giây (°/s), đã trừ offset
    void getGyro(float &x, float &y, float &z) const;

private:
    ITG3205 chip;

    // --------------- Cấu hình chip ---------------
    static constexpr uint8_t SMPLRT_DIV = 4; // F_sample = 1kHz / (4 + 1) = 200Hz

    // --------------- Quy đổi ---------------
    // Sensitivity ±2000°/s của ITG-3200: 14.375 LSB/(°/s)
    static constexpr float LSB_PER_DPS = 14.375f;

    // --------------- Hiệu chỉnh offset lúc khởi động ---------------
    static constexpr int16_t N_SAMPLE = 1000;
    static constexpr uint32_t SETTLE_MS = 500;
    // Thời gian đợi lấy mẫu mới
    static constexpr uint32_t READY_TIMEOUT_MS = 20;

    // Chờ chip báo có mẫu mới, false nếu I2C lỗi hoặc timeout
    bool waitDataReady(uint32_t timeout_ms);

    // --------------- Dữ liệu ---------------
    float gyro_x = 0, gyro_y = 0, gyro_z = 0;
    float offset_x = 0, offset_y = 0, offset_z = 0;
};
