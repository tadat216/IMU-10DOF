#include <Arduino.h>
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

bool Accelerometer::update(){
    int16_t raw_x, raw_y, raw_z;
    if(!chip.getAccelRawData(raw_x, raw_y, raw_z)) return false;
    accel_x = raw_x / LSB_PER_G;
    accel_y = raw_y / LSB_PER_G;
    accel_z = raw_z / LSB_PER_G;
    return true;
}

const char *Accelerometer::name() const {
    return "ADXL345";
}

void Accelerometer::getAccel(float &x, float &y, float &z) const {
    x = accel_x;
    y = accel_y;
    z = accel_z;
}
