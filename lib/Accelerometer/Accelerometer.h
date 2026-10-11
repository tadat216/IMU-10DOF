#pragma once
#include <stdint.h>
#include "ADXL345.h"

class Accelerometer {
public:
    bool begin();
    bool update();
    const char *name() const;

    // Đơn vị: g
    void getAccel(float &x, float &y, float &z) const;

    bool calibrate(uint8_t maxFailPerAxis = 3, uint8_t maxFailTotal = 10);

    // Lưu / nạp offset + scale vào flash (NVS). load false nếu chưa có hoặc dữ liệu không hợp lệ
    bool saveCalibration();
    bool loadCalibration();

private:
    ADXL345 chip;

    // --------------- Quy đổi ---------------
    // Full resolution: 3.9 mg/LSB (~256 LSB/g) ở mọi range
    static constexpr float LSB_PER_G = 256.0f;

    // Chờ sau khi bật measure (~15 ms > 1.1 ms + 1/100 Hz)
    static constexpr uint32_t SETTLE_MS = 15;

    // Thời gian đợi lấy mẫu mới (chu kỳ mẫu ở 100 Hz là 10 ms)
    static constexpr uint32_t READY_TIMEOUT_MS = 20;

    // --------------- Hiệu chỉnh ---------------
    // Số mẫu liên tiếp hợp lệ cần có (100 mẫu ở 100 Hz = 1 s)
    static constexpr uint16_t CAL_N_SAMPLE = 100;
    // Thời gian tối đa để user xoay chip đúng hướng và giữ yên
    static constexpr uint32_t CAL_TIMEOUT_MS = 20000;
    // Trục đang hiệu chỉnh phải vượt ngưỡng này theo đúng chiều (g)
    static constexpr float CAL_AXIS_MIN_G = 0.85f;
    // Hai trục còn lại không được vượt ngưỡng này (g)
    static constexpr float CAL_OTHER_MAX_G = 0.15f;
    // Trong cửa sổ mẫu, max - min của mỗi trục không được vượt ngưỡng này (g)
    static constexpr float CAL_RANGE_MAX_G = 0.05f;
    // Khoảng cách tối thiểu giữa 2 lần in giá trị và trạng thái (200 ms = 5 lần/giây)
    static constexpr uint32_t CAL_PRINT_INTERVAL_MS = 200;

    // --------------- Lưu hiệu chỉnh vào flash ---------------
    // Chỉ dùng kiểu 4 byte để struct không có padding. Đổi cấu trúc thì tăng CAL_VERSION.
    struct CalData {
        uint32_t version;
        float offset[3];
        float scale[3];
    };
    static constexpr uint32_t CAL_VERSION = 1;
    static constexpr const char *CAL_NAMESPACE = "accel";
    static constexpr const char *CAL_KEY = "cal";
    // Ngưỡng hợp lệ: ADXL345 full-res ~256 LSB/g (232-286), offset 0 g điển hình +-40 mg
    static constexpr float CAL_OFFSET_MAX_G = 1.0f;
    static constexpr float CAL_SCALE_MIN = 0.8f;
    static constexpr float CAL_SCALE_MAX = 1.2f;

    // Chờ chip báo có mẫu mới, false nếu I2C lỗi hoặc timeout
    bool waitDataReady(uint32_t timeout_ms);

    // --------------- Dữ liệu ---------------
    float accel[3] = {};
    float offset[3] = {};
    float scale[3] = {1, 1, 1};

    bool calibrateSingleDir(uint8_t axis, bool isPositive, float &data);
};
