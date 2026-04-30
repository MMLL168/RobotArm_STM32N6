# proposal.md
# NUCLEO-N657X0-Q 分階段、模組化 pin 規劃提案

## 專案目標
建立一個可逐步擴充的 NUCLEO-N657X0-Q 機械手臂平台，預計整合：

- 影像：OV7670（並行）或板載 MIPI CSI-2 模組（擇一，見「影像策略」）
- 顯示：SPI LCD（ILI9341 / ST7789，TX-only simplex）
- 音訊：INMP441（I2S 收音）+ MAX98357A（I2S 播放）
- 感測：MPU6050（IMU）+ VL53L0X（ToF）
- 動作：4 路 Servo（Clamp / Fore-Aft / Gripper-Lift / Left-Right）
- 基礎控制：UART log / LED / Button

設計原則：

1. 一次只開一個模組，CubeMX 分階段 Generate Code，每階段都要能跑、能燒、能 log。
2. 同一模組腳位盡量集中，方便走線與排線。
3. 優先使用 NUCLEO-N657X0-Q 已引出的 Arduino UNO V3 / ST Zio / ST morpho 腳位。
4. 影像腳位優先保留，其他模組退讓。
5. 衝突處理順序：影像 > 音訊 > 顯示 > 感測 > Servo > 其他控制。
6. 板級已保留腳位（debug、flash、UCPD、相機 connector、user LED/button）一律不動。

---

## Board 前提：NUCLEO-N657X0-Q

板上提供：

- Arduino UNO V3 / ST Zio
- ST morpho 雙排 header（pin 完整引出）
- FPC 相機接頭（MIPI CSI-2，連 MB1939 模組）
- ST-LINK / V3E（含 VCP）
- 三顆 user LED：**LD1 (green) = PG0、LD2 (orange) = PG10、LD3 (red) = PG8**
- USER button：**PC13**
- USB / Ethernet 等板載功能
- 外部 OctoSPI flash（XSPI2 / port P2，已用於 FSBL+Appli boot）

代表這片板子適合做多模組 bring-up，但 **多數高階 GPIO 已經被板級或 IP 預先佔用**，pin conflict 必須靠分階段規劃解決。

### 板級不可用 / 已保留腳位

下列腳位在 CubeMX 已 `Locked=true` 或屬板級固定路由，**規劃時直接視為不存在**：

| 區塊 | 腳位 | 用途 |
|---|---|---|
| SWD / JTAG | PA13、PA14、PA15、PB5（SWO）| Debug |
| TRACE | PB0、PB3、PB6、PB7、PE3 | ETM trace |
| OCTOSPI flash (XSPI2 P2) | PN0–PN12 | FSBL + Appli boot flash |
| USB-PD / UCPD1 | PA5、PA7、PA10、PA11、PD2、PD10 | UCPD ISENSE / VSENSE / INT / PWR_EN |
| FSBL I2C2 | PB10、PB11 | 板級 I2C2，FSBL 階段使用 |
| MIPI CSI 連接器 | CSI_CKP/N、CSI_D0P/N、CSI_D1P/N | FPC 相機模組 |
| 相機 reset | PO5 (CAM_NRST) | FPC connector 上的 RESET |
| User LEDs | PG0、PG10、PG8 | 板載 LD1 / LD2 / LD3 |
| User Button | PC13 | 板載 USER button |
| Audio MCO | PC9 (AUDIOCLK) | 板級 audio master clock 來源 |

> 當 CubeMX 對某腳出現紅色衝突，先檢查是否落在這張表內。

---

## 影像策略：OV7670 vs 板載 MIPI

NUCLEO-N657X0-Q 的 FPC 相機接頭是 **MIPI CSI-2**（DCMIPP + CSI 子系統），不是並行 8-bit。
這代表兩條路：

- **路線 A（建議第一階段）：板載 MIPI 模組（IMX335 / MB1939）**
  - 直接走 FPC，PSSI/DCMIPP/CSI 已經在 .ioc 配好。
  - 不佔用 morpho 腳位。
  - 影像腳完全不衝突。
- **路線 B：OV7670（並行 D0–D7 + PCLK/HSYNC/VSYNC + XCLK）**
  - 需要從 morpho header 走線。
  - 至少佔用 11 隻 GPIO，必須避開 trace、flash、UCPD、user LED/Button。
  - 適合 OV7670 練手或低成本 demo。

本提案以「先驗證路線 A，再保留路線 B 擴充能力」為原則：路線 A 先用，路線 B 的腳位仍預留但 **僅在路線 A 不採用時啟用**。兩者擇一啟用、不會同時上電。

---

## 分階段開發策略

### Phase 0：最小系統
建立可編譯、可燒錄、可 log 的基底工程。

功能：
- System clock：400 MHz AXI、200 MHz APB（沿用目前 .ioc）
- LPUART1 log（PE5/PE6，ST-Link VCP）
- LED1 (PG0) blink
- USER button (PC13) input

驗收：
- `Appli/build/Debug/STM32N6_Appli.bin` 編譯成功
- 簽章 + 燒錄 FSBL+Appli 後可從 XSPI2 boot
- VCP 115200 baud 收到 `Hello` log
- LED1 約 1 Hz 閃爍
- 按 USER 按鈕 log 出 `BTN`

### Phase 1：I2C + 基本感測器
建立 I2C 共用匯流排，作為後續所有 I2C 裝置（含 OV7670 SCCB）的單一通道。

功能：
- I2C1（PH9 SCL / PC1 SDA），400 kHz fast-mode
- MPU6050（addr `0x68`）
- VL53L0X（addr `0x29`）
- OV7670 SCCB 預留（addr `0x42 / 0x43`，僅在路線 B 啟用）

驗收：
- I2C1 bus scan 列出至少 `0x68`、`0x29`
- MPU6050 `WHO_AM_I = 0x68`
- VL53L0X 量得 1 cm – 1 m 距離
- 全部裝置可在同一條 bus 同時運作 ≥ 1 分鐘無 NACK

### Phase 2：SPI LCD 顯示（已完成 bring-up）
讓系統有可視化回饋，方便後續模組看狀態。

功能：
- SPI5 simplex（TX-only，無 MISO），8-bit，目前 8 MBits/s
- ILI9341 driver（240×320）
- DC / CS / RST / BL 為 GPIO

驗收（已通過）：
- LCD 點亮、刷新無撕裂
- SPI5 在 8 MBits/s 下穩定 ≥ 5 分鐘
- 與 I2C1 同時運作無干擾

### Phase 3：Servo PWM
4 路 50 Hz PWM，對應機械手臂 4 自由度。

功能：
- 4 路 PWM，週期 20 ms（period=19999、prescaler=399、clock=400 MHz/2）
- 預設脈寬 1500 µs（中位）
- 三顆 timer 分配（受 AF 限制，無法 4 通道擠在同一個 advanced timer）：
  - **TIM1_CH1 / CH2** → SERVO_1 / SERVO_2
  - **TIM2_CH3** → SERVO_3
  - **TIM16_CH1** → SERVO_4

驗收：
- 4 路可獨立 1000 µs ↔ 2000 µs 掃動
- 同時動作不抖動、不掉脈衝
- 與 SPI5 / I2C1 同時運作無 timer 衝突

### Phase 4：Audio（INMP441 + MAX98357A）
先 bring-up I2S，之後再做 ASR / 播放邏輯。

功能：
- SAI1（建議方案）或 I2S，主時鐘來源沿用 PC9 `AUDIOCLK`
- 收音：INMP441（24-bit Philips I2S，需要 BCLK / LRCK / SD_IN）
- 播放：MAX98357A（16-bit I2S，BCLK / LRCK / SD_OUT，可與 INMP441 共用 BCLK / LRCK）

驗收：
- INMP441 可錄到 16 kHz / 16-bit PCM ≥ 5 秒、RMS 符合預期
- MAX98357A 可播 1 kHz 純音，無破音
- 與 Phase 1–3 全部模組同時運作無 underrun / overrun

### Phase 5：影像（路線 A 板載 MIPI 為主）
最後再加入相機。

路線 A 功能：
- DCMIPP + CSI 子系統（pins 已 locked，無 morpho 腳位影響）
- CAM_NRST = PO5 控制相機 reset
- I2C1 上的 SCCB / I²C 控制通道

路線 B 功能（僅在不採用 A 時啟用）：
- XCLK、PCLK、HSYNC、VSYNC、D0–D7 由 morpho 引出（見「OV7670 並行接腳」表）
- SCCB 共用 I2C1（PH9 / PC1）

驗收：
- 連續擷取 ≥ 30 秒、frame timing 正確
- 不影響 Phase 0–4 任何模組

### Phase 6：整合 / RTOS（可選）
所有模組單獨能跑後，再決定是否加 FreeRTOS。

功能：
- FreeRTOS task：log、sensor、servo、audio、video
- DMA + IRQ 規劃（避免 SAI / SPI / DCMIPP 互搶 priority）
- Watchdog / brown-out

驗收：
- ≥ 30 分鐘 soak 不 hang
- task CPU usage 量測完成
- 重要事件可由 LCD 即時回饋

---

## 建議 pin map

### A. 控制與共用區

#### I2C 共用匯流排（已固定）
- I2C1_SCL = **PH9**
- I2C1_SDA = **PC1**

用途：MPU6050、VL53L0X、OV7670 SCCB（路線 B）。
理由：板上 SCL/SDA 兩腳相鄰；I2C1 在 N6 上有完整 fast-mode+ 支援。

#### UART 除錯（修正：使用 LPUART1，走 ST-Link VCP）
- LPUART1_TX = **PE5**（AF3）
- LPUART1_RX = **PE6**（AF3）

用途：log / debug / CLI。
理由：NUCLEO-N657X0-Q 的 ST-Link VCP **就是接到 PE5/PE6**。原本提案寫的 PA9/PA10 是 USB-UCPD 區，板上不可用。

#### 基本 LED / Button（補上實際腳位）
- LED1（green） = **PG0**（板載 LD1）
- LED2（orange）= **PG10**（板載 LD2）
- LED3（red）   = **PG8**（板載 LD3）
- USER_BUTTON   = **PC13**（板載 USER）

用途：狀態顯示、模式切換、測試觸發。

---

### B. 顯示模組區（已固定，bring-up 完成）

#### SPI LCD（SPI5，TX-only simplex）
- LCD_SCK  = **PH5**（SPI5_SCK，AF）
- LCD_MOSI = **PH7**（SPI5_MOSI，AF）
- LCD_CS   = **PD12**（GPIO，預設 high）
- LCD_DC   = **PE7**（GPIO，預設 low）
- LCD_RST  = **PE1**（GPIO，預設 high）
- LCD_BL   = **PE14**（GPIO，預設 high）

說明：
- LCD 不需 MISO，SPI5 設成 simplex `2LINES_TXONLY`。
- 目前 baud rate 8 MBits/s（`prescaler=8`、`SPI5CLK=64 MHz HSI`）。
- CS / DC / RST / BL 為純 GPIO。

---

### C. 音訊模組區（具體化提案）

NUCLEO-N657X0-Q 上 PC9 已被預設為 **AUDIOCLK**（master clock 來源），所以 audio 子系統建議走 SAI1，把 INMP441 與 MAX98357A 共用同一組 BCLK / LRCK，僅 SD pin 各自分開。

#### 建議 pin（**待 CubeMX AF 驗證**）：

| 訊號 | 建議腳 | AF | 說明 |
|---|---|---|---|
| AUDIO_MCLK  | PC9   | Audio_ClockIn | 已固定 |
| AUDIO_BCLK  | PE5*  | SAI1_SCK_A    | 與 LPUART1 衝突，**改 PA8 / PF7 / PG7 之一** |
| AUDIO_LRCK  | PE4   | SAI1_FS_A     | 候選 |
| AUDIO_SD_IN | PE3*  | SAI1_SD_B     | **與 TRACED0 衝突，改 PF6 / PG10** |
| AUDIO_SD_OUT| PE6*  | SAI1_SD_A     | 與 LPUART1 衝突，**改 PD6 / PF8** |

> ★ 上表標 `*` 的腳會與已用模組衝突，列出來是為了提醒「不能用」；最終腳位以 CubeMX 在 SAI1_Block_A / Block_B 下逐一試出 AF 不衝突的組合為準。
> 策略：
> - 兩裝置共用 BCLK + LRCK（同步 timing）。
> - INMP441 的 SD 接 SAI1 Block B（接收）。
> - MAX98357A 的 SD 接 SAI1 Block A（傳送）。
> - 若 SAI1 AF 找不到不衝突組合，退而使用 SPI/I2S（SPI3 / SPI6 在 N6 上有 I2S 模式）。

---

### D. 動作模組區（已固定，timer 分配補上）

4 路 servo 對應機械手臂的四個自由度。受 AF 限制，無法 4 channel 擠在同一個 advanced timer，故拆 3 顆 timer：

| Servo 邏輯名 | MCU Pin | Timer / Channel | 預設脈寬 |
|---|---|---|---|
| SERVO_1（Clamp）        | **PE9**  | TIM1_CH1   | 1500 µs |
| SERVO_2（Fore-Aft）     | **PE11** | TIM1_CH2   | 1500 µs |
| SERVO_3（Gripper-Lift）  | **PA2**  | TIM2_CH3   | 1500 µs |
| SERVO_4（Left-Right）   | **PA3**  | TIM16_CH1  | 1500 µs |

共用設定：
- Period = 19999、Prescaler = 399（→ 400 MHz / 400 = 1 MHz tick → 20 ms PWM 週期）
- 三顆 timer 之間獨立 clock source，互不影響。
- TIM1 是 advanced timer，需注意 break / dead-time 預設要關。

---

### E. 影像模組區

#### 路線 A：板載 MIPI 模組（已固定）
- CSI_CKP / CSI_CKN：clock lane（板級固定）
- CSI_D0P / CSI_D0N：data lane 0（板級固定）
- CSI_D1P / CSI_D1N：data lane 1（板級固定）
- CAM_NRST = **PO5**（GPIO output，預設 low → reset；放開為 high 才能初始化）

說明：MIPI CSI-2 不佔 morpho 腳位，不衝突任何 Phase 0–4 模組。

#### 路線 B：OV7670 並行（保留，僅在不採用 A 時啟用）

控制腳：

| 訊號 | 建議腳 | 備註 |
|---|---|---|
| OV7670_XCLK  | PC9 (AUDIOCLK) | 與 audio MCO 衝突 → 改 MCO2 / 自行 PWM |
| OV7670_RESET | PD0 | |
| OV7670_PWDN  | PE0 | |
| OV7670_SIOC  | PH9（與 I2C1_SCL 共用） | |
| OV7670_SIOD  | PC1（與 I2C1_SDA 共用） | |

資料腳（11 隻 GPIO，需避開 trace / flash / UCPD / user LED）：

| 訊號 | 建議腳 | 備註 |
|---|---|---|
| OV7670_PCLK  | PG1 | |
| OV7670_HSYNC | PG3 | |
| OV7670_VSYNC | PE6 | **與 LPUART1_RX 衝突，啟用路線 B 時要改成 LPUART1 改別腳或用 SWO log** |
| OV7670_D0    | PD7 | |
| OV7670_D1    | PC6 | |
| OV7670_D2    | PC8 | |
| OV7670_D3    | PE10 | |
| OV7670_D4    | PE8 | |
| OV7670_D5    | PE4 | |
| OV7670_D6    | PF5 | |
| OV7670_D7    | PF1 | |

---

## Pin 規劃總表（已對齊現況）

| 模組 | 訊號 | MCU Pin | Timer / IP | 狀態 |
|---|---|---:|---|---|
| LPUART | TX | PE5 | LPUART1 (AF3) | 固定（VCP） |
| LPUART | RX | PE6 | LPUART1 (AF3) | 固定（VCP） |
| LED | LD1 | PG0 | GPIO | 板級固定 |
| LED | LD2 | PG10 | GPIO | 板級固定 |
| LED | LD3 | PG8 | GPIO | 板級固定 |
| Button | USER | PC13 | GPIO | 板級固定 |
| I2C | SCL | PH9 | I2C1 | 固定 |
| I2C | SDA | PC1 | I2C1 | 固定 |
| LCD | SCK | PH5 | SPI5 | 固定 |
| LCD | MOSI | PH7 | SPI5 | 固定 |
| LCD | CS | PD12 | GPIO | 固定 |
| LCD | DC | PE7 | GPIO | 固定 |
| LCD | RST | PE1 | GPIO | 固定 |
| LCD | BL | PE14 | GPIO | 固定 |
| Servo | S1 (Clamp) | PE9 | TIM1_CH1 | 固定 |
| Servo | S2 (Fore-Aft) | PE11 | TIM1_CH2 | 固定 |
| Servo | S3 (Gripper-Lift) | PA2 | TIM2_CH3 | 固定 |
| Servo | S4 (Left-Right) | PA3 | TIM16_CH1 | 固定 |
| Audio | MCLK | PC9 | AUDIOCLK | 固定 |
| Audio | BCLK | TBD | SAI1_SCK_A | CubeMX 確認 AF |
| Audio | LRCK | TBD | SAI1_FS_A | CubeMX 確認 AF |
| Audio | SD_IN | TBD | SAI1_SD_B | CubeMX 確認 AF |
| Audio | SD_OUT | TBD | SAI1_SD_A | CubeMX 確認 AF |
| Camera (A) | CSI lanes | CSI_CK/D0/D1 P/N | CSI / DCMIPP | 板級固定 |
| Camera (A) | CAM_NRST | PO5 | GPIO | 板級固定 |
| Camera (B) | XCLK | PC9 | MCO / TIM | 與 AUDIOCLK 衝突 |
| Camera (B) | RESET | PD0 | GPIO | 預留 |
| Camera (B) | PWDN | PE0 | GPIO | 預留 |
| Camera (B) | PCLK | PG1 | DCMI / GPIO | 預留 |
| Camera (B) | HSYNC | PG3 | DCMI | 預留 |
| Camera (B) | VSYNC | PE6 | DCMI | **與 LPUART1_RX 衝突** |
| Camera (B) | D0–D7 | PD7/PC6/PC8/PE10/PE8/PE4/PF5/PF1 | DCMI | 預留 |

---

## CubeMX 實作方式

### 方式 1：分階段生成
1. 開最小專案 (Phase 0)。
2. 每次只加一個模組。
3. 每次 Generate Code 後立刻燒錄 + 驗證。
4. 有衝突就回上一版 `.ioc`。

### 方式 2：模組化文件管理
維護以下三份文件：
- `proposal.md`（本文件，總體規劃）
- `pin_map.md`（總表 + 衝突歷史，可從本文件 §「Pin 規劃總表」拆出來）
- `bringup_checklist.md`（每階段驗收勾選表）

### 建議 .ioc snapshot 命名
- `base.ioc`（Phase 0）
- `i2c_stage.ioc`（Phase 1）
- `lcd_stage.ioc`（Phase 2，**= 目前 STM32N6.ioc**）
- `servo_stage.ioc`（Phase 3）
- `audio_stage.ioc`（Phase 4）
- `camera_stage.ioc`（Phase 5）

每次過階段在 git 新建 tag，例如 `bringup/phase2-lcd`。

---

## 風險與處理順序

### 衝突優先處理順序
1. 影像核心腳（CSI / DCMI 路徑）
2. 音訊（SAI MCLK 已固定）
3. LCD（SPI5 + GPIO，已 bring-up）
4. Servo（timer AF 緊張）
5. 其他控制（UART / LED / Button）

### 衝突解法
- 先動音訊（SAI 候選腳多，AF 彈性高）。
- 再動 Servo（TIM1/TIM2/TIM16 之外還有 TIM3/TIM4/TIM15/TIM17 可換）。
- 再動 LCD（SPI5 改 SPI4 / SPI3 也可）。
- 最後才動 I2C 共用腳（一旦改，全部 I2C 裝置與 SCCB 都要重接）。
- **影像核心腳、板級保留腳、debug / flash 腳不動。**

### 已知會踩到的雷
- **路線 B + LPUART1 同時啟用 → PE6 衝突**（VSYNC vs LPUART1_RX）。要嘛改用 ETM trace SWO 做 log，要嘛把 LPUART log 移到 USART2 等其他腳。
- **OV7670 XCLK 與 PC9 AUDIOCLK 衝突** → 用 MCO2 (PA15) 不行（debug 腳）；改用 TIM PWM 由其他 GPIO 給 24 MHz。
- **TIM1 是 advanced timer**，break input / dead-time 預設關閉，否則 PWM 不會出。
- **I2C1 同時掛 MPU6050 + VL53L0X + 路線 B SCCB**，OV7670 SCCB 雖然名為 I²C 但 timing 較鬆，bus 速度要遷就 SCCB 安全範圍（≤ 100 kHz）。

---

## 驗收條件（總整理）

每個 Phase 必須符合以下「整體驗收」才算過關：

1. 每個模組可獨立啟動。
2. 同一模組腳位盡量相鄰。
3. 沒有 pin 重複衝突；CubeMX `.ioc` 能 generate 不出紅。
4. CubeMX 可以逐階段 Generate Code，且 generate 後 git diff 只動該階段相關檔案。
5. 專案可長期擴充，不需要整包重排。
6. **每階段都能在 LCD 或 VCP 上看到該模組的 live 狀態。**

## 備註

- 這份提案的核心不是一次把所有功能塞滿，而是先建立一個可以穩定成長的 STM32N657X0 平台。
- 板級腳位以 NUCLEO-N657X0-Q UM3300 user manual 與目前 .ioc 為準；若 ST 之後 release 板級 BSP 變更，本文要同步 review。
- 所有「TBD」腳位都要用 CubeMX 的 AF 對照表逐一確認，本文僅給候選與衝突警告。
