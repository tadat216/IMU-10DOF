// 
#pragma once
#include <stdint.h>
#include "ISensor.h"
#include "I2CBus.h"
#include "Arduino.h"

class ITG3205 : public ISensor {
public:
    bool begin() override;
    bool update() override;
    const char *name() const override;

    void getGyro(float &x, float &y, float &z);

private:

    // REGISTER
    const uint8_t DEV_ADD = 0x68;
    const uint8_t REG_WHO_AM_I = 0x00;

    const uint8_t REG_SMPLRT_DIV = 0x15;
    // Tài liệu Register 21 – Sample Rate Divider
    // F_sample = F_internal / (SMPLRT_DIV + 1)
    // Với F_internal = 1K
    // F_internal: Tốc độ lấy mẫu nội bộ của chip
    // F_sample: Tốc độ ghi data vào thanh ghi
    const uint8_t SMPLRT_DIV = 4; // F_sample = 200

    const uint8_t REG_DLPF_FS = 0x16;
    // FS_SEL: Gyro Full-Scale Range ±2000°/sec
    const uint8_t FS_SEL = 0x03;
    const uint8_t FS_SEL_SHIFT = 3;
    // DLPF_CFG = 0 -> F_internal = 8kHz
    // DLPF_CFG = 1-6 -> F_internal = 1kHz
    // 3 bit đầu tiên
    const uint8_t DLPF_CFG = 3;

    const uint8_t REG_INT_CFG = 0x17;
    // enable để flag RAW_DATA_RDY được cập nhật
    const uint8_t BIT_RAW_RDY_EN = 0;

    const uint8_t REG_INT_STATUS = 0x1A;
    // Bit flag này để Master hỏi slave đã có data mới chưa, 
    // ngay sau khi hỏi xong nếu là 1 thì flag sẽ tự 
    // set lại về 0
    // Tần suất đọc của master phải lớn hơn slave
    // Mục đích
    // 1. Không đọc trùng mẫu vì cơ chế tự set vè 0
    // 2. Có thể kiểm tra việc đọc sót mẫu vì ta biết được 
    // F_sample -> kiểm tra khoảng thời gian phát hiện có mẫu
    // mới có cách nhau lâu hơn thời gian ghi mẫu
    // không, nếu có thì master đang bỏ sót mẫu
    const uint8_t BIT_RAW_DATA_RDY = 0;

    const uint8_t REG_GYRO = 0x1D;
    const uint8_t GYRO_LEN = 6;

    const uint8_t REG_PWR_MGM = 0x3E;
    const uint8_t CLK_SEL_VAL = 1; 

    // Variables
    // Lấy mẫu
    const int16_t N_SAMPLE = 1000;
    const uint32_t SETTLE_MS = 500;
    float gyro_x = 0;
    float gyro_y = 0;
    float gyro_z = 0;
    float offset_x = 0;
    float offset_y = 0;
    float offset_z = 0;

    const float LSB = 14.375;
};