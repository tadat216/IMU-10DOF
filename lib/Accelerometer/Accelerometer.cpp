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
    int16_t raw[3];
    if(!chip.getAccelRawData(raw[0], raw[1], raw[3])) return false;
    for(int i = 0; i < 3; i++){
        accel[i] = raw[i] / LSB_PER_G;
    }
    return true;
}

bool Accelerometer::calibrate(uint8_t axis, bool isPositive){
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
