#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include "Accelerometer.h"
#include "Gyroscope.h"

const int SDA_PIN = 8;
const int SCL_PIN = 7;

Accelerometer accel;
Gyroscope gyro;

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.begin(115200);
  // Chờ IMU ổn định
  delay(3000);
  if(!accel.begin()){
    Serial.printf("Lỗi khởi chạy %s\n", accel.name());
  }
  if(!gyro.begin()){
    Serial.println("Lỗi khởi chạy gyro");
  }

  // Gyro: nạp offset đã lưu trong flash, chưa có thì đo lúc đứng yên (~5 s), đặt chip nằm yên trên bàn
  if(gyro.loadCalibration()){
    Serial.println("Đã nạp offset gyro từ flash");
  }
  else{
    Serial.println("Chưa có offset gyro hợp lệ trong flash");
    Serial.println("Hiệu chỉnh gyro: đặt chip nằm yên, không chạm vào...");
    if(gyro.calibrate()){
      Serial.println("Hiệu chỉnh gyro xong");
    }
    else{
      Serial.println("Hiệu chỉnh gyro thất bại, dùng offset mặc định");
    }
  }

  // Accel: nạp hiệu chỉnh đã lưu trong flash, chưa có thì xoay chip theo hướng dẫn trên Serial
  if(accel.loadCalibration()){
    Serial.println("Đã nạp hiệu chỉnh accel từ flash");
  }
  else{
    Serial.println("Chưa có hiệu chỉnh accel hợp lệ trong flash");
    accel.calibrate();
  }
}

void loop() {
  float a[3] = {}, g[3] = {};
  bool accelOk = accel.update();
  bool gyroOk = gyro.update();

  if(!accelOk) Serial.println("Không lấy được data của accel");
  if(!gyroOk) Serial.println("Không lấy được data của gyro");

  if(accelOk && gyroOk){
    accel.getAccel(a[0], a[1], a[2]);
    gyro.getGyro(g[0], g[1], g[2]);
    // Đứng yên thì |a| ~ 1 g, gyro ~ 0 °/s
    float mag = sqrtf(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
    Serial.printf("(g) x = %7.3f, y = %7.3f, z = %7.3f | |a| = %.3f || (°/s) x = %8.3f, y = %8.3f, z = %8.3f\n",
                  a[0], a[1], a[2], mag, g[0], g[1], g[2]);
  }
  delay(100);
}
