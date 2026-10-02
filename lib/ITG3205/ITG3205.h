#pragma once
#include <stdint.h>
#include "I2CBus.h"

class ITG3205 {
public:

    // --------------- Giá trị truyền vào các hàm set ---------------

    // FS_SEL: Gyro Full-Scale Range
    static constexpr uint8_t FS_SEL_2000 = 0x03; // ±2000°/sec

    // DLPF_CFG: Low Pass Filter Bandwidth / Internal Sample Rate
    static constexpr uint8_t DLPF_CFG_BW256 = 0x00; // 8kHz
    static constexpr uint8_t DLPF_CFG_BW188 = 0x01; // 1kHz
    static constexpr uint8_t DLPF_CFG_BW98  = 0x02; // 1kHz
    static constexpr uint8_t DLPF_CFG_BW42  = 0x03; // 1kHz
    static constexpr uint8_t DLPF_CFG_BW20  = 0x04; // 1kHz
    static constexpr uint8_t DLPF_CFG_BW10  = 0x05; // 1kHz
    static constexpr uint8_t DLPF_CFG_BW5   = 0x06; // 1kHz

    // CLK_SEL: Device clock source
    static constexpr uint8_t CLK_SEL_INTERNAL = 0x00;
    static constexpr uint8_t CLK_SEL_PLL_X    = 0x01;
    static constexpr uint8_t CLK_SEL_PLL_Y    = 0x02;
    static constexpr uint8_t CLK_SEL_PLL_Z    = 0x03;
    static constexpr uint8_t CLK_SEL_PLL_EXT_32K768 = 0x04;
    static constexpr uint8_t CLK_SEL_PLL_EXT_19M2   = 0x05;

    // --------------- API ---------------

    // Register WHO_AM_I
    bool getDeviceId(uint8_t &id);

    // Register SMPLRT_DIV
    bool setSampleRateDivider(uint8_t divider);

    // Register DLPF_FS
    bool setFullScaleRange(uint8_t range);
    bool setDLPFBandWidth(uint8_t bandwidth);

    // Register INT_CFG
    bool setIntDataReadyEnableMode(bool enable);

    // Register INT_STATUS
    bool getIntDataReadyStatus(bool &ready);

    // Registers Gyro
    bool getGyroRawData(int16_t &x, int16_t &y, int16_t &z);

    // Register PWR_MGM
    bool setDeviceClockSource(uint8_t selection);

private:

    // --------------- REGISTERs ---------------
    static constexpr uint8_t DEV_ADD = 0x68;

    // Register 00 – WHO_AM_I
    // ID [6:1]
    static constexpr uint8_t REG_WHO_AM_I = 0x00;
    static constexpr uint8_t WHO_AM_I_IDX = 1;
    static constexpr uint8_t WHO_AM_I_LEN = 6;

    // Register 21 – Sample Rate Divider
    static constexpr uint8_t REG_SMPLRT_DIV = 0x15;

    // Register 22 – DLPF, Full Scale
    static constexpr uint8_t REG_DLPF_FS = 0x16;

    // FS_SEL [4:3]
    static constexpr uint8_t FS_SEL_IDX = 3;
    static constexpr uint8_t FS_SEL_LEN = 2;

    // DLPF_CFG [2:0]
    static constexpr uint8_t DLPF_CFG_IDX = 0;
    static constexpr uint8_t DLPF_CFG_LEN = 3;

    // Register 23 – Interrupt Configuration
    static constexpr uint8_t REG_INT_CFG = 0x17;
    static constexpr uint8_t RAW_RDY_EN_IDX = 0;

    // Register 26 – Interrupt Status
    static constexpr uint8_t REG_INT_STATUS = 0x1A;
    static constexpr uint8_t RAW_DATA_RDY_IDX = 0;

    // Register 29..34 – 6 registers liên tiếp của Gyro (X_H, X_L, Y_H, Y_L, Z_H, Z_L)
    static constexpr uint8_t REG_GYRO = 0x1D;
    static constexpr uint8_t GYRO_LEN = 6;

    // Register 62 – Power Management
    static constexpr uint8_t REG_PWR_MGM = 0x3E;

    // CLK_SEL [2:0]
    static constexpr uint8_t CLK_SEL_IDX = 0;
    static constexpr uint8_t CLK_SEL_LEN = 3;
};
