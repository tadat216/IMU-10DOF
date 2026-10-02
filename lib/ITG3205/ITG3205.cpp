#include "ITG3205.h"

// Register WHO_AM_I

/**
 * ID [6:1]
 * Giá trị mong đợi là 0x34 (0b110100). Đây là giá trị đã dịch về bit 0,
 * còn giá trị cả thanh ghi là 0x68 (ID nằm ở bit 6:1).
 */
bool ITG3205::getDeviceId(uint8_t &id){
    return I2CBus::readBits(DEV_ADD, REG_WHO_AM_I, WHO_AM_I_IDX, WHO_AM_I_LEN, id);
}

// Register SMPLRT_DIV

/**
 * SMPLRT_DIV [7:0]
 * F_sample = F_internal / (SMPLRT_DIV + 1)
 * F_internal: Tốc độ lấy mẫu nội bộ của chip, set bởi DLPF_CFG
 * F_sample: Tốc độ ghi data vào thanh ghi
 * @see setDLPFBandWidth
 */
bool ITG3205::setSampleRateDivider(uint8_t divider){
    return I2CBus::writeByte(DEV_ADD, REG_SMPLRT_DIV, divider);
}

// Register DLPF_FS

/**
 * FS_SEL [4:3]
 * FS_SEL     Gyro Full-Scale Range
 * 0-2       Reserved
 * 3         ±2000°/sec
 * @see FS_SEL_2000
 */
bool ITG3205::setFullScaleRange(uint8_t range){
    return I2CBus::writeBits(DEV_ADD, REG_DLPF_FS, FS_SEL_IDX, FS_SEL_LEN, range);
}

/**
 * DLPF_CFG [2:0]
 * The DLPF_CFG parameter sets the digital low pass filter
 * configuration. It also determines the internal sampling
 * rate used by the device as shown in the table below.
 * | DLPF_CFG | Low Pass Filter Bandwidth | Internal Sample Rate |
 * |-|-|-|
 * | 0        |  256Hz                    | 8kHz |
 * | 1        |  188Hz                    | 1kHz |
 * | 2        |  98Hz                     | 1kHz |
 * | 3        |  42Hz                     | 1kHz |
 * | 4        |  20Hz                     | 1kHz |
 * | 5        |  10Hz                     | 1kHz |
 * | 6        |  5Hz                      | 1kHz |
 * | 7        |  Reserved                 | Reserved |
 * @see DLPF_CFG_BW*
 */
bool ITG3205::setDLPFBandWidth(uint8_t bandwidth){
    return I2CBus::writeBits(DEV_ADD, REG_DLPF_FS, DLPF_CFG_IDX, DLPF_CFG_LEN, bandwidth);
}

// Register INT_CFG

/**
 * RAW_RDY_EN [0]
 * Bật ngắt "raw data ready" (ngắt báo có data mới trong thanh ghi gyro).
 * @see REG_INT_CFG
 * @see RAW_RDY_EN_IDX
 */
bool ITG3205::setIntDataReadyEnableMode(bool enable){
    return I2CBus::writeBit(DEV_ADD, REG_INT_CFG, RAW_RDY_EN_IDX, enable);
}

// Register INT_STATUS

/**
 * RAW_DATA_RDY [0]
 * Flag cho biết chip đã ghi data mới vào thanh ghi gyro chưa.
 * Chỉ ghi vào ready khi đọc I2C thành công.
 * @see setIntDataReadyEnableMode
 */
bool ITG3205::getIntDataReadyStatus(bool &ready){
    uint8_t bit = 0;
    if(!I2CBus::readBit(DEV_ADD, REG_INT_STATUS, RAW_DATA_RDY_IDX, bit)) return false;
    ready = bit;
    return true;
}

// Gyro registers

/**
 * Lấy raw 16 bit của gyro, big-endian (byte High trước, byte Low sau)
 */
bool ITG3205::getGyroRawData(int16_t &x, int16_t &y, int16_t &z){
    uint8_t data[GYRO_LEN];
    if(!I2CBus::readBytes(DEV_ADD, REG_GYRO, GYRO_LEN, data)) return false;
    x = (int16_t)((data[0] << 8) | data[1]);
    y = (int16_t)((data[2] << 8) | data[3]);
    z = (int16_t)((data[4] << 8) | data[5]);
    return true;
}

// Register PWR_MGM

/**
 * CLK_SEL [2:0]
 * The CLK_SEL setting determines the device clock source, as follows:
 *
 * | CLK_SEL | Clock Source |
 * |-|-|
 * | 0 |  Internal oscillator |
 * | 1 |  PLL with X Gyro reference |
 * | 2 |  PLL with Y Gyro reference |
 * | 3 |  PLL with Z Gyro reference |
 * | 4 |  PLL with external 32.768kHz reference |
 * | 5 |  PLL with external 19.2MHz reference |
 * | 6 |  Reserved |
 * | 7 |  Reserved |
 * On power up, the ITG-3200 defaults to the internal
 * oscillator. It is highly recommended that the device
 * is configured to use one of the gyros (or an external
 * clock) as the clock reference, due to the improved
 * stability.
 * @see CLK_SEL_*
 */
bool ITG3205::setDeviceClockSource(uint8_t selection){
    return I2CBus::writeBits(DEV_ADD, REG_PWR_MGM, CLK_SEL_IDX, CLK_SEL_LEN, selection);
}
