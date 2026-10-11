#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include "Accelerometer.h"
#include "Gyroscope.h"

const int SDA_PIN = 8;
const int SCL_PIN = 7;

// Bảng dữ liệu: số dòng, khoảng cách giữa 2 dòng, và số dòng giữa 2 lần in lại tiêu đề
const int TABLE_ROWS = 100;
const uint32_t TABLE_INTERVAL_MS = 100;
const int TABLE_HEADER_EVERY = 20;

Accelerometer accel;
Gyroscope gyro;

void printMenu() {
  Serial.println();
  Serial.println("========== MENU ==========");
  Serial.println("1: In bảng 100 dòng dữ liệu (accel, gyro)");
  Serial.println("2: Hiệu chỉnh accel (xoay chip theo hướng dẫn)");
  Serial.println("3: Hiệu chỉnh gyro (đặt chip nằm yên)");
  Serial.println("4: Hiệu chỉnh la bàn (chưa hỗ trợ)");
  Serial.println("5: Hiệu chỉnh tất cả (gyro -> accel -> la bàn)");
  Serial.println("Gõ số rồi nhấn Enter:");
}

// Bỏ các ký tự user gõ trong lúc chương trình đang bận
void clearSerialInput() {
  while(Serial.available()) Serial.read();
}

bool calibrateGyro() {
  Serial.println("Hiệu chỉnh gyro: đặt chip nằm yên, không chạm vào...");
  if(gyro.calibrate()){
    Serial.println("Hiệu chỉnh gyro xong");
    return true;
  }
  Serial.println("Hiệu chỉnh gyro thất bại, giữ offset cũ");
  return false;
}

bool calibrateAccel() {
  if(accel.calibrate()){
    Serial.println("Hiệu chỉnh accel xong");
    return true;
  }
  Serial.println("Hiệu chỉnh accel thất bại, giữ hiệu chỉnh cũ");
  return false;
}

void calibrateMag() {
  Serial.println("Chưa có driver la bàn (VCM5883L), bỏ qua");
}

// Gyro trước vì cần chip nằm yên, accel và la bàn cần xoay chip. Dừng nếu một bước thất bại.
void calibrateAll() {
  if(!calibrateGyro()) return;
  if(!calibrateAccel()) return;
  calibrateMag();
}

void printTableHeader() {
  Serial.println("  #  |  ax (g)  ay (g)  az (g) | |a| (g) |  gx (°/s)  gy (°/s)  gz (°/s)");
}

void printTable() {
  float a[3], g[3];
  for(int i = 0; i < TABLE_ROWS; i++){
    if(i % TABLE_HEADER_EVERY == 0) printTableHeader();

    if(!accel.update() || !gyro.update()){
      Serial.println("Không lấy được data của accel hoặc gyro, dừng in");
      return;
    }
    accel.getAccel(a[0], a[1], a[2]);
    gyro.getGyro(g[0], g[1], g[2]);
    // Đứng yên thì |a| ~ 1 g, gyro ~ 0 °/s
    float mag = sqrtf(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
    Serial.printf("%3d  | %7.3f %7.3f %7.3f | %6.3f  | %9.3f %9.3f %9.3f\n",
                  i + 1, a[0], a[1], a[2], mag, g[0], g[1], g[2]);
    delay(TABLE_INTERVAL_MS);
  }
}

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

  // Nạp hiệu chỉnh đã lưu trong flash. Không nạp được thì dùng mặc định,
  // thời điểm hiệu chỉnh do user quyết định qua menu.
  if(gyro.loadCalibration()){
    Serial.println("Đã nạp offset gyro từ flash");
  }
  else{
    Serial.println("Chưa có offset gyro hợp lệ trong flash, dùng mặc định (chọn 3 để hiệu chỉnh)");
  }

  if(accel.loadCalibration()){
    Serial.println("Đã nạp hiệu chỉnh accel từ flash");
  }
  else{
    Serial.println("Chưa có hiệu chỉnh accel hợp lệ trong flash, dùng mặc định (chọn 2 để hiệu chỉnh)");
  }

  clearSerialInput();
  printMenu();
}

void loop() {
  if(!Serial.available()) return;

  char c = Serial.read();
  // Serial Monitor có thể gửi kèm xuống dòng sau phím gõ
  if(c == '\r' || c == '\n' || c == ' ') return;

  switch(c){
    case '1': printTable(); break;
    case '2': calibrateAccel(); break;
    case '3': calibrateGyro(); break;
    case '4': calibrateMag(); break;
    case '5': calibrateAll(); break;
    default:
      Serial.printf("Lệnh không hợp lệ: '%c'\n", c);
  }

  clearSerialInput();
  printMenu();
}
