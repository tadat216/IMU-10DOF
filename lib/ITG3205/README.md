# ITG-3205

Tài liệu tra cứu nhanh cho driver [ITG3205](ITG3205.h). Nguồn: datasheet ITG-3200 rev 1.4 (ITG-3205 dùng chung bộ thanh ghi).

## 1. Tổng quan

- Con quay hồi chuyển 3 trục X, Y, Z, đo **tốc độ góc** (°/s), không đo góc trực tiếp.
- ADC 16 bit cho mỗi trục, dải đo cố định **±2000 °/s**, độ nhạy **14.375 LSB/(°/s)**.
- Có thêm cảm biến nhiệt độ (280 LSB/°C, offset -13200 LSB tại 35 °C).
- Giao tiếp I2C tối đa 400 kHz. Chip luôn là slave.
- Nguồn VDD 2.1–3.6 V. VLOGIC (1.71 V đến VDD) quyết định mức logic của I2C và phải ≤ VDD.

> **LSB** (Least Significant Bit) ở đây là đơn vị nhỏ nhất của giá trị số thô, hay một "count". Độ nhạy 14.375 LSB/(°/s) nghĩa là khi tốc độ quay tăng 1 °/s thì số nguyên 16 bit có dấu (ghép từ 2 thanh ghi 8 bit) tăng 14.375 đơn vị.
>
> Vì sao dải đo là 2000 °/s: 14.375 × 2000 = 28750, vừa nằm trong khoảng số nguyên 16 bit có dấu (±32767).

Công thức đổi sang °/s: `rate = raw / 14.375`

## 2. Địa chỉ I2C

| AD0 | Địa chỉ 7 bit |
|---|---|
| GND | `0x68` (1101000) |
| VDD/VLOGIC | `0x69` (1101001) |

Thanh ghi WHO_AM_I (0x00): chỉ **bit 6..1** là ID (`110100`), bit 7 và bit 0 không xác định. Vì vậy phải so sánh `(val & 0x7E) == 0x68`, không so sánh thẳng cả byte.

## 3. Thanh ghi dùng trong driver

| Địa chỉ | Tên | Giá trị dùng | Ý nghĩa |
|---|---|---|---|
| 0x00 | WHO_AM_I | đọc | Kiểm tra chip |
| 0x15 | SMPLRT_DIV | 4 | Chia tần số lấy mẫu |
| 0x16 | DLPF_FS | `(3 << 3) \| 3` | FS_SEL (bit 4:3) và DLPF_CFG (bit 2:0) |
| 0x17 | INT_CFG | bit 0 = 1 | Bật RAW_RDY_EN để cờ dữ liệu mới hoạt động |
| 0x1A | INT_STATUS | đọc bit 0 | RAW_DATA_RDY: có mẫu mới |
| 0x1D–0x22 | GYRO_X/Y/Z H/L | đọc 6 byte | Dữ liệu gyro |
| 0x3E | PWR_MGM | 1 | Chọn clock PLL theo gyro X |

Thanh ghi nhiệt độ (0x1B, 0x1C) nằm ngay trước gyro nhưng driver chưa đọc.

### Dữ liệu gyro

- Mỗi trục gồm 2 byte, **byte cao trước**, định dạng bù hai: `int16_t((H << 8) | L)`.
- Đọc liên tục 6 byte từ 0x1D (burst read), con trỏ thanh ghi tự tăng.
- Thứ tự: X_H, X_L, Y_H, Y_L, Z_H, Z_L.

## 4. Tần số lấy mẫu và bộ lọc

```
F_sample = F_internal / (SMPLRT_DIV + 1)
```

`F_internal` phụ thuộc DLPF_CFG:

| DLPF_CFG | Băng thông lọc | F_internal |
|---|---|---|
| 0 | 256 Hz | 8 kHz |
| 1 | 188 Hz | 1 kHz |
| 2 | 98 Hz | 1 kHz |
| **3** | **42 Hz** | **1 kHz** |
| 4 | 20 Hz | 1 kHz |
| 5 | 10 Hz | 1 kHz |
| 6 | 5 Hz | 1 kHz |
| 7 | Reserved | Reserved |

Cấu hình hiện tại: DLPF_CFG = 3, SMPLRT_DIV = 4 → **F_sample = 1 kHz / 5 = 200 Hz** (5 ms một mẫu).

Băng thông càng hẹp thì càng ít nhiễu nhưng độ trễ pha càng lớn.

> FS_SEL phải đặt bằng 3. Các giá trị 0–2 là reserved, và giá trị mặc định lúc bật nguồn (0) không dùng được.

## 5. Cờ RAW_DATA_RDY

- Bật bằng bit RAW_RDY_EN (INT_CFG bit 0). Nếu không bật thì cờ trong INT_STATUS không bao giờ lên.
- Cờ **tự xoá khi master đọc INT_STATUS** (với INT_ANYRD_2CLEAR = 0).
- Lợi ích:
  1. Không đọc trùng một mẫu, nhờ cơ chế tự xoá.
  2. Phát hiện bỏ sót mẫu: nếu hai lần thấy cờ lên cách nhau lâu hơn `1/F_sample` thì master đang đọc chậm hơn chip.
- Master phải đọc nhanh hơn F_sample, nếu không sẽ mất mẫu.
- Không bắt buộc dùng cờ: có thể đọc thanh ghi dữ liệu bất cứ lúc nào, nhưng khi đó có thể nhận lại mẫu cũ.

## 6. Clock và thời gian ổn định

- Lúc bật nguồn chip chạy bằng dao động nội (kém chính xác). Datasheet khuyến nghị chuyển sang PLL theo một trục gyro (`CLK_SEL` = 1, 2, 3) để ổn định hơn.
- Sau khi đổi clock, PLL cần ~1 ms, và ZRO (zero-rate output) cần ~50 ms để ổn định.
- Chip cần ≥ 20 ms sau khi cấp nguồn mới đọc/ghi thanh ghi được.
- Driver chờ `SETTLE_MS` (500 ms) sau khi cấu hình rồi mới lấy mẫu offset.

## 7. Hiệu chuẩn offset

Gyro khi đứng yên vẫn xuất ra một giá trị khác 0 (zero-rate output, tối đa ±40 °/s ở 25 °C, và đổi theo nhiệt độ).

Driver xử lý bằng cách trong `begin()`:

1. Giữ chip đứng yên, lấy `N_SAMPLE` (1000) mẫu mỗi khi cờ RAW_DATA_RDY lên.
2. `offset = tổng / N_SAMPLE` cho từng trục.
3. `update()` luôn trừ offset: `rate = raw / 14.375 - offset`.

Lưu ý:
- `begin()` reset offset về 0 trước khi lấy mẫu, nếu không sẽ tính trên dữ liệu đã trừ offset cũ.
- Giới hạn thời gian là `N_SAMPLE × 10` ms. Hết thời gian mà chưa đủ mẫu thì `begin()` trả `false`.
- Offset trôi theo nhiệt độ, nên khi chip nóng lên giá trị lúc đứng yên có thể lệch dần khỏi 0.

## 8. Độ chính xác mong đợi

Đo thực tế lúc đứng yên (100 mẫu, DLPF 42 Hz):

| Trục | Trung bình | Độ lệch chuẩn |
|---|---|---|
| X | 0.002 °/s | 0.051 °/s |
| Y | 0.005 °/s | 0.051 °/s |
| Z | 0.005 °/s | 0.046 °/s |

- 1 LSB ≈ 0.07 °/s, nên nhiễu ~0.05 °/s tức chưa tới 1 LSB.
- Datasheet: nhiễu RMS 0.38 °/s ở DLPF 100 Hz.
- Nếu tích phân thành góc thì sẽ trôi ~2–4 độ/phút. Muốn giảm cần kết hợp với accel (complementary, Madgwick...).

## 9. Chiều quay

Chiều dương của từng trục theo hình hướng trục ở mục 4.1 datasheet. Chiều quay chưa được kiểm chứng bằng thực nghiệm. Xoay chip quanh từng trục để xác nhận dấu.

## 10. Giao thức I2C (nhắc lại)

- **Ghi 1 byte:** `S | AD+W | RA | DATA | P`
- **Đọc nhiều byte:** `S | AD+W | RA | Sr | AD+R | DATA... | NACK | P`. Giữa hai pha là *repeated start*, không phải STOP.
- Master trả ACK sau mỗi byte, trừ byte cuối trả NACK.
- Ghi/đọc một bit trong thanh ghi phải theo kiểu **đọc – sửa bit – ghi lại** để không làm mất các bit khác.
