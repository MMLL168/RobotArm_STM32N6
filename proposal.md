# proposal.md
# NUCLEO-N657X0-Q 分階段、模組化 pin 規劃提案

## 專案目標
建立一個可逐步擴充的 NUCLEO-N657X0-Q 專案，包含：
- 影像：OV7670 no FIFO
- 顯示：SPI LCD
- 音訊：INMP441 + MAX98357A
- 感測：MPU6050 + VL53L0X
- 動作：4 路 Servo
- 基礎控制：UART / LED / Button

設計原則：
1. 一次只開一個模組，CubeMX 分階段 Generate Code。
2. 同一模組的腳位盡量相鄰，方便走線與排線。
3. 盡量使用 NUCLEO-N657X0-Q 的 Arduino / ST morpho 已引出的腳位。
4. 影像腳優先保留，其他模組配合影像做讓位。
5. 若發生衝突，以「影像 > 音訊 > 顯示 > 感測 > Servo > 其他控制」順序處理。

## Board 前提
NUCLEO-N657X0-Q 提供：
- Arduino Uno V3 / ST Zio 介面
- ST morpho headers
- Camera module FPC / MIPI20 compatible connector
- ST-LINK
- user LEDs
- user button
- reset button
- USB / Ethernet 等板載功能
這代表板子適合做多模組 bring-up，但 pin conflict 必須靠分階段規劃解決。

## 分階段開發策略
### Phase 0：最小系統
先建立最小可編譯、可燒錄、可 log 的基底工程。

功能：
- System clock
- UART log
- LED blink
- Button input

驗證：
- 編譯成功
- 燒錄成功
- UART 可輸出
- LED 可閃爍

### Phase 1：I2C + 基本感測器
先建立 I2C 共用匯流排，接後續感測器與相機控制。

功能：
- I2C1
- MPU6050
- VL53L0X
- OV7670 SCCB

驗證：
- I2C 掃描成功
- 感測器可讀值
- SCCB 可初始化 OV7670

### Phase 2：LCD 顯示
先把 LCD 跑起來，讓系統有可視化回饋。

功能：
- SPI LCD
- 背光控制
- 基本文字與圖形顯示

驗證：
- LCD 點亮
- 畫面可刷新
- SPI 穩定

### Phase 3：Servo PWM
先做 4 路伺服馬達控制。

功能：
- 4 路 PWM
- 固定 50Hz
- 獨立 channel

驗證：
- 每路可單獨控制
- 不抖動
- 與其他外設無衝突

### Phase 4：Audio
先做收音，再做播放。

功能：
- INMP441
- MAX98357A
- I2S 或 SAI

驗證：
- 錄音資料流正常
- 播放正常
- 與影像無衝突

### Phase 5：OV7670 影像
最後再加入平行相機資料路徑。

功能：
- XCLK
- RESET
- PWDN
- PCLK
- HSYNC
- VSYNC
- D0~D7

驗證：
- 影像資料可擷取
- Frame timing 正常
- 不影響其他模組

---

## 建議 pin map

## A. 控制與共用區
### I2C 共用匯流排
- I2C1_SCL = PH9
- I2C1_SDA = PC1

用途：
- MPU6050
- VL53L0X
- OV7670 SCCB

理由：
- 兩條線相鄰且好找。
- 可共用多個裝置。
- 方便用 Arduino 介面或 Morpho 拉線。

### UART 除錯
- UART_TX = PA9
- UART_RX = PA10

用途：
- log
- debug
- CLI

理由：
- 方便維持最小系統除錯通道。
- 不應與影像腳共用。

### 基本 LED / Button
- LED1 = 板載預設 LED
- USER_BUTTON = 板載預設按鍵

用途：
- 狀態顯示
- 模式切換
- 測試觸發

---

## B. 顯示模組區
### SPI LCD
建議集中在同一區，方便走線。

- LCD_SCK  = PH5
- LCD_MOSI = PH7
- LCD_CS   = PD12
- LCD_DC   = PE7
- LCD_RST  = PE1
- LCD_BL   = PE14

說明：
- LCD 不需要 MISO。
- CS / DC / RST / BL 都用 GPIO。
- 這組維持簡潔、方便調整。

---

## C. 音訊模組區
### INMP441
建議先只做錄音，腳位先保留可調整空間。

- AUDIO_BCLK = 待定
- AUDIO_LRCK = 待定
- AUDIO_SD_IN = 待定

### MAX98357A
- AUDIO_SD_OUT = 待定

策略：
- 先讓 CubeMX 找不衝突的 I2S/SAI 腳位。
- 音訊不要硬塞到影像核心腳旁邊。
- 若 I2S 衝突，優先改 SAI 方案。

---

## D. 動作模組區
### 4 路 Servo
建議放在可 PWM 的 timer 腳，且盡量集中在同一側。

- SERVO_1 = PE9
- SERVO_2 = PE11
- SERVO_3 = PA2
- SERVO_4 = PA3

說明：
- 先做基本 PWM。
- 後續再依 CubeMX AF 與板級可用性微調。
- Servo 不應占用影像資料腳。

---

## E. 影像模組區
### OV7670 控制腳
- OV7670_XCLK  = PC9
- OV7670_RESET = PD0
- OV7670_PWDN  = PE0
- OV7670_SIOC  = PH9
- OV7670_SIOD  = PC1

### OV7670 資料腳
- OV7670_PCLK  = PG1
- OV7670_HSYNC = PG3
- OV7670_VSYNC = PE6
- OV7670_D0    = PD7
- OV7670_D1    = PC6
- OV7670_D2    = PC8
- OV7670_D3    = PE10
- OV7670_D4    = PE8
- OV7670_D5    = PE4
- OV7670_D6    = PF5
- OV7670_D7    = PF1

規劃原則：
- 影像腳固定後不要再動。
- 影像腳優先保留。
- 若後續還有衝突，優先搬動其他模組，不動影像核心腳。

---

## Pin 規劃總表

| 模組 | 訊號 | MCU Pin | 狀態 |
|---|---|---:|---|
| I2C | SCL | PH9 | 固定 |
| I2C | SDA | PC1 | 固定 |
| UART | TX | PA9 | 建議固定 |
| UART | RX | PA10 | 建議固定 |
| LCD | SCK | PH5 | 固定 |
| LCD | MOSI | PH7 | 固定 |
| LCD | CS | PD12 | 固定 |
| LCD | DC | PE7 | 固定 |
| LCD | RST | PE1 | 固定 |
| LCD | BL | PE14 | 固定 |
| Servo | S1 | PE9 | 固定 |
| Servo | S2 | PE11 | 固定 |
| Servo | S3 | PA2 | 固定 |
| Servo | S4 | PA3 | 固定 |
| Audio | BCLK | 待定 | CubeMX 再找 |
| Audio | LRCK | 待定 | CubeMX 再找 |
| Audio | SD_IN | 待定 | CubeMX 再找 |
| Audio | SD_OUT | 待定 | CubeMX 再找 |
| OV7670 | XCLK | PC9 | 固定 |
| OV7670 | RESET | PD0 | 固定 |
| OV7670 | PWDN | PE0 | 固定 |
| OV7670 | SIOC | PH9 | 與 I2C 共用 |
| OV7670 | SIOD | PC1 | 與 I2C 共用 |
| OV7670 | PCLK | PG1 | 固定 |
| OV7670 | HSYNC | PG3 | 固定 |
| OV7670 | VSYNC | PE6 | 固定 |
| OV7670 | D0 | PD7 | 固定 |
| OV7670 | D1 | PC6 | 固定 |
| OV7670 | D2 | PC8 | 固定 |
| OV7670 | D3 | PE10 | 固定 |
| OV7670 | D4 | PE8 | 固定 |
| OV7670 | D5 | PE4 | 固定 |
| OV7670 | D6 | PF5 | 固定 |
| OV7670 | D7 | PF1 | 固定 |

---

## CubeMX 實作方式
### 方式 1：分階段生成
1. 開一個最小專案。
2. 每次只加入一個模組。
3. 每次 Generate Code 後立刻驗證。
4. 有衝突就回上一版 `.ioc`。

### 方式 2：模組化文件管理
建議同時維護：
- `proposal.md`
- `pin_map.md`
- `bringup_checklist.md`

### 建議檔名
- `base.ioc`
- `i2c_stage.ioc`
- `lcd_stage.ioc`
- `servo_stage.ioc`
- `audio_stage.ioc`
- `camera_stage.ioc`

---

## 風險與處理順序
### 優先避免衝突的順序
1. 影像
2. 音訊
3. LCD
4. Servo
5. 其他控制

### 若衝突時的處理
- 先動音訊。
- 再動 LCD。
- 再動 Servo。
- 最後才動 I2C 共用腳。
- 影像核心腳盡量不動。

---

## 驗收條件
1. 每個模組可獨立啟動。
2. 同一模組腳位盡量相鄰。
3. 沒有 pin 重複衝突。
4. CubeMX 可以逐階段 Generate Code。
5. 專案可以長期擴充，不需要整包重排。

## 備註
這份提案的核心不是一次把所有功能塞滿，而是先建立一個可以穩定成長的 STM32N657X0 平台。  
若之後 AF 或板級連接器有變動，優先以 CubeMX 與板級手冊再做一次確認。