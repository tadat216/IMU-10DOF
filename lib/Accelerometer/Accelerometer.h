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

private:
    ADXL345 chip;

    // --------------- Quy đổi ---------------
    // Full resolution: 3.9 mg/LSB (~256 LSB/g) ở mọi range
    static constexpr float LSB_PER_G = 256.0f;

    // Chờ sau khi bật measure (~15 ms > 1.1 ms + 1/100 Hz)
    static constexpr uint32_t SETTLE_MS = 15;

    // --------------- Dữ liệu ---------------
    float accel_x = 0, accel_y = 0, accel_z = 0;
};
