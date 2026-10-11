#pragma once
#include <stdint.h>
#include "ITG3205.h"

class Gyroscope {
public:
    bool begin();
    bool update();

    bool calibrate();

    // Lưu / nạp offset vào flash (NVS). load false nếu chưa có hoặc dữ liệu không hợp lệ
    bool saveCalibration();
    bool loadCalibration();

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
    // Trong cửa sổ mẫu, max - min của mỗi trục không được vượt ngưỡng này (°/s).
    // Nhiễu 0.38 °/s rms (datasheet, LPF 100 Hz) nên peak-to-peak ~ 2-3 °/s
    static constexpr float STILL_RANGE_MAX_DPS = 3.0f;
    // Thời gian tối đa để user giữ yên chip đủ N_SAMPLE mẫu
    static constexpr uint32_t CAL_TIMEOUT_MS = 20000;
    // Khoảng cách tối thiểu giữa 2 lần in cảnh báo chuyển động
    static constexpr uint32_t CAL_PRINT_INTERVAL_MS = 500;

    // --------------- Lưu offset vào flash ---------------
    // Chỉ dùng kiểu 4 byte để struct không có padding. Đổi cấu trúc thì tăng CAL_VERSION.
    struct CalData {
        uint32_t version;
        float offset[3];
    };
    static constexpr uint32_t CAL_VERSION = 1;
    static constexpr const char *CAL_NAMESPACE = "gyro";
    static constexpr const char *CAL_KEY = "cal";
    // Datasheet ITG-3200: Initial ZRO +-40 °/s, ZRO theo nhiệt độ thêm +-40 °/s
    static constexpr float CAL_OFFSET_MAX_DPS = 80.0f;

    // Chờ chip báo có mẫu mới, false nếu I2C lỗi hoặc timeout
    bool waitDataReady(uint32_t timeout_ms);

    // --------------- Dữ liệu ---------------
    float gyro[3] = {}; // giá trị x, y, z (°/s)
    float offset[3] = {}; // off set của x, y, z
};
