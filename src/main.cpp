#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include "Accelerometer.h"

const int SDA_PIN = 4;
const int SKL_PIN = 5;

Accelerometer accel;

void setup() {
  Wire.begin(SDA_PIN, SKL_PIN);
  Serial.begin(115200);
  // Chờ IMU ổn định
  delay(2000);
  if(!accel.begin()){
    Serial.printf("Lỗi khởi chạy %s\n", accel.name());
  }
}

void loop() {
  if(!accel.update()){
    Serial.println("Không lấy được data của accel");
  }
  else{
    float a[3];
    accel.getAccel(a[0], a[1], a[2]);
    // Đứng yên thì |a| ~ 1 g
    float mag = sqrtf(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
    Serial.printf("(g) x = %7.3f, y = %7.3f, z = %7.3f | |a| = %.3f\n", a[0], a[1], a[2], mag);
  }
  delay(100);
}
