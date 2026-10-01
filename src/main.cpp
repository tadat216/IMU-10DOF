#include <Arduino.h>
#include <Wire.h>
// #include "I2CBus.h"
#include "ADXL345.h"
#include "ITG3205.h"

const int SDA_PIN = 4;
const int SKL_PIN = 5;

ADXL345 accel;
ITG3205 gyro;

void setup() {
  Wire.begin(SDA_PIN, SKL_PIN);
  Serial.begin(115200);
  if(!accel.begin()){
    Serial.printf("Loi voi chip %s\n", accel.name());
  }
  if(!gyro.begin()){
    Serial.printf("Loi voi chip %s\n", gyro.name());
  }
}

void loop() {
  // byte error, address;
  // int nDevices;

  // Serial.println("Scanning...");

  // nDevices = 0;
  // for(address = 1; address < 127; address++ ) {
  //   // Truyền tín hiệu đến địa chỉ hiện tại
  //   Wire.beginTransmission(address);
  //   error = Wire.endTransmission();

  //   if (error == 0) {
  //     Serial.print("I2C device found at address 0x");
  //     if (address < 16)
  //       Serial.print("0");
  //     Serial.print(address, HEX);
  //     Serial.println("  !");

  //     nDevices++;
  //   }
  //   else if (error == 4) {
  //     Serial.print("Unknow error at address 0x");
  //     if (address < 16)
  //       Serial.print("0");
  //     Serial.println(address, HEX);
  //   }
  // }
  // if (nDevices == 0)
  //   Serial.println("No I2C devices found\n");
  // else
  //   Serial.println("done\n");

  // delay(5000); // Chờ 5 giây trước khi quét lại
  if(!accel.update()){
    Serial.println("Loi update data\n");
  }
  else{
    Serial.printf("accel x = %.3f, y = %.3f, z = %.3f\n", accel.getAccelX(),  accel.getAccelY(),  accel.getAccelZ());
  }

  if(!gyro.update()){
    Serial.println("Loi update gyro\n");
  }
  else{
    float gx, gy, gz;
    gyro.getGyro(gx, gy, gz);
    Serial.printf("gyro  x = %.3f, y = %.3f, z = %.3f (deg/s)\n", gx, gy, gz);
  }
  delay(100);
}
