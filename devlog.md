# Dev Log

## 索引

| 編號 | 行號 | 日期時間 | 問題摘要 |
| --- | --- | --- | --- |
| 001 | L114 | 2026-04-21 11:24:55 | 建立 devlog 紀錄準則 |
| 002 | L120 | 2026-04-21 12:01:29 | 建立 VS Code Debug 與 Live Watch 設定 |
| 003 | L125 | 2026-04-21 12:47:38 | 修正 VS Code tasks 的 CMake 路徑依賴 |
| 004 | L130 | 2026-04-21 12:53:23 | 補齊 CMake preset 的 Ninja 路徑 |
| 005 | L135 | 2026-04-21 12:55:30 | 讓 toolchain 自動尋找 Arm GCC |
| 006 | L140 | 2026-04-21 12:56:11 | 修正 CMake 對 ProgramFiles(x86) 的解析 |
| 007 | L145 | 2026-04-21 12:56:53 | 補上 Windows 工具鏈執行檔副檔名 |
| 008 | L150 | 2026-04-21 13:31:19 | 補齊 Cortex-Debug 的 GDB 路徑設定 |
| 009 | L155 | 2026-04-21 15:16:52 | 新增 OpenOCD Live Watch 啟動設定 |
| 010 | L160 | 2026-04-21 17:06:31 | 改為 under-reset 連線模式以穩定 halt target |
| 011 | L165 | 2026-04-21 19:32:13 | 降低 debug 接管頻率以改善 reset 後 halt |
| 012 | L170 | 2026-04-21 19:55:49 | 對齊 G431 專案的 pinned-probe 與 attach 設定 |
| 013 | L175 | 2026-04-21 20:23:58 | 修正 STM32N6 的 ST-LINK 核心 AP 選擇 |
| 014 | L180 | 2026-04-21 20:36:50 | 改用 Appli RAM image 啟動流程以停在 main |
| 015 | L185 | 2026-04-21 21:33:53 | 改由 Cortex-Debug 接手停在 main 與 Pause 中斷 |
| 016 | L190 | 2026-04-21 21:38:52 | 整理 STM32N6 Debug 避坑排查順序 |
| 017 | L195 | 2026-04-21 21:46:52 | 完成 Phase 0 的 UART log、LED blink 與 Button input |
| 018 | L200 | 2026-04-22 06:56:19 | 將 Phase 0 自訂修改搬回 USER CODE 區塊避免被 CubeMX 覆蓋 |
| 019 | L205 | 2026-04-22 12:24:14 | 補上 Appli RAM image 的 Reset 與 Restart 重載流程 |
| 020 | L210 | 2026-04-22 20:24:44 | 補上 Appli Phase 1 的 I2C1 bring-up 與掃描 |
| 021 | L215 | 2026-04-22 20:30:36 | 將 I2C HAL 開關移到 CMake 以避開 CubeMX 覆蓋 |
| 022 | L220 | 2026-04-23 14:39:09 | 新增 Phase 1 感測器辨識以驗證 MPU6050 與 VL53L0X 接線 |
| 023 | L225 | 2026-04-23 14:53:23 | 補上 Phase 1 感測器讀值與修正按鍵極性 |
| 024 | L230 | 2026-04-23 15:04:20 | 強化 Phase 1 telemetry 以判讀 MPU6050 與 VL53L0X 量測品質 |
| 025 | L235 | 2026-04-23 15:11:08 | 修正 VL53L0X raw status 11 的判讀避免誤報異常 |
| 026 | L240 | 2026-04-23 15:52:04 | 抽離應用層模組以避免 CubeMX 生成覆蓋 |
| 027 | L245 | 2026-04-23 15:58:42 | 補上 CubeMX 生成後遺漏的 TIM 共用時脈設定呼叫 |
| 028 | L250 | 2026-04-23 16:03:33 | 新增 TIM1 CH1 舵機安全起始與按鈕切換控制 |
| 029 | L255 | 2026-04-23 16:16:21 | 依實際 TIMG 時脈校正舵機 PWM 週期與脈寬 |
| 030 | L260 | 2026-04-23 16:24:59 | 放大舵機開合脈寬到 1100 與 1900 us |
| 031 | L265 | 2026-04-23 18:55:26 | 切換到舵機安裝中位模式固定 1500 us |
| 032 | L270 | 2026-04-24 08:10:34 | 新增舵機安全步進量測模式以找出夾爪可用範圍 |
| 033 | L275 | 2026-04-24 08:22:55 | 依實測結果收斂舵機量測窗到 700..1600 us |
| 034 | L280 | 2026-04-24 08:28:04 | 改成按鍵觸發的鬆夾鬆正式動作模式 |
| 035 | L285 | 2026-04-24 08:50:53 | 改成按一下夾住再按一下放開的切換式動作 |
| 036 | L290 | 2026-04-24 08:58:31 | 初始化 Git 並推送到 GitHub 遠端但排除 devlog |
| 037 | L295 | 2026-04-24 09:21:18 | 啟用第二顆舵機 CH2 並固定中位做安全 bring-up |
| 038 | L300 | 2026-04-24 10:25:06 | 暫時把按鍵改成只量 CH2 範圍並固定夾爪不動 |
| 039 | L305 | 2026-04-24 10:45:58 | 將 CH2 量測下限由 1200 放寬到 900 us |
| 040 | L310 | 2026-04-24 10:50:02 | 將 CH2 定成 900 與 1800 的正式前後切換模式 |
| 041 | L315 | 2026-04-24 11:31:23 | 檢查 regenerate 並接入第三顆舵機 CH3 安全量測模式 |
| 042 | L320 | 2026-04-24 12:01:55 | 將第三顆舵機 CH3 量測下限由 1300 放寬到 1000 us |
| 043 | L325 | 2026-04-24 12:04:28 | 將第三顆舵機 CH3 量測上限由 1700 放寬到 1900 us |
| 044 | L330 | 2026-04-24 12:08:05 | 將第三顆舵機 CH3 定成 1000 與 1500 的正式上下切換模式 |
| 045 | L335 | 2026-04-24 12:12:43 | 將舵機命名改成 clamp、fore_aft、gripper_lift 語意 |
| 046 | L340 | 2026-04-24 12:23:22 | 檢查第四顆舵機 regenerate 並接入 CH4 安全量測模式 |
| 047 | L345 | 2026-04-24 14:01:21 | 將第四顆舵機 CH4 量測窗由 1300..1700 放寬到 1000..1900 us |
| 048 | L350 | 2026-04-24 14:04:52 | 將第四顆舵機 CH4 量測窗由 1000..1900 放寬到 700..2300 us |
| 049 | L355 | 2026-04-24 14:35:06 | 將第四顆舵機 CH4 定成 2000 左與 1000 右的正式切換模式 |
| 050 | L360 | 2026-04-24 14:40:37 | 將第四顆舵機命名由 axis4 改成 left_right 語意 |
| 051 | L365 | 2026-04-24 15:07:09 | 補強 VL53L0X single-shot 失敗點診斷 |
| 052 | L370 | 2026-04-24 15:18:07 | 將按鍵改成觸發 4 軸全範圍展示模式 |
| 053 | L375 | 2026-04-24 15:26:06 | 將 left_right 軸移到展示序列最前面 |
| 054 | L380 | 2026-04-24 15:38:53 | 建立機械手臂空間座標、雅各比矩陣與逆運動學模組 |
| 055 | L385 | 2026-04-24 16:05:00 | 將實測不撞範圍升級為工作空間估算硬限制 |
| 056 | L390 | 2026-04-24 20:16:47 | 將實測安全範圍接入舵機實際控制路徑 |
| 057 | L395 | 2026-04-24 20:36:40 | 建立 UART XYZ/PWM 控制鏈與桌面 GUI debug 工具 |
| 058 | L400 | 2026-04-24 20:46:26 | 為 GUI 加入對應動作的機械手臂動態視覺預覽 |
| 059 | L405 | 2026-04-24 20:51:32 | 為 GUI 加入右側與下側捲軸避免小視窗截斷 |
| 060 | L410 | 2026-04-24 20:55:46 | 修正 GUI 最小尺寸與預覽雷達面板的裁切 |
| 061 | L415 | 2026-04-24 21:14:05 | 將預覽圖例移到下方並收斂預覽字級與間距 |
| 062 | L420 | 2026-04-24 21:21:04 | 依視窗寬度自動重排控制區避免右側預覽被壓扁 |
| 063 | L425 | 2026-04-24 21:58:50 | 將 4 軸舵機控制改成多軸同步 minimum-jerk 軌跡引擎 |
| 064 | L430 | 2026-04-24 22:13:52 | 將 XYZ 升級成 Cartesian minimum-jerk 路徑規劃 |
| 065 | L435 | 2026-04-24 22:13:52 | 讓 GUI 接上 firmware 軌跡狀態並自動輪詢 STATUS |
| 066 | L440 | 2026-04-25 07:46:52 | 在 GUI 標示 XYZ 的 conservative 可輸入範圍 |
| 067 | L445 | 2026-04-25 07:51:28 | 讓超界 XYZ 欄位變紅並在送出前硬擋 |
| 068 | L450 | 2026-04-25 07:57:00 | 修正 GUI 顯示 Send 被擋下的即時原因 |
| 069 | L455 | 2026-04-25 08:03:49 | 在 GUI 顯示 PWM/HOME/XYZ 的 FW 命令回覆與 echo |
| 070 | L460 | 2026-04-25 08:12:32 | 將 UART 命令接收改成 IRQ ring buffer 以修正多次設定後失聯 |
| 071 | L465 | 2026-04-25 11:43:00 | 將 GUI 起始手臂圖改成檯燈式展示姿態 |
| 072 | L470 | 2026-04-25 11:49:52 | 將 GUI 預設值改成區間中點並放大預覽下壓範圍 |
| 073 | L475 | 2026-04-25 11:59:29 | 對齊 HOME 與 Load Midpoint 的中點姿態 |
| 074 | L480 | 2026-04-25 12:05:01 | 恢復 GUI 啟動時的檯燈式展示姿態 |
| 075 | L485 | 2026-04-25 12:11:04 | 將 GUI 平台移到最低手臂點下方 |
| 076 | L490 | 2026-04-25 12:20:00 | 將 GUI 預覽改成工業臂外觀並標示關節語意 |
| 077 | L495 | 2026-04-25 12:33:52 | 將 GUI 底座改成較低的 pedestal 與斜撐 |
| 078 | L500 | 2026-04-25 12:46:35 | 將 GUI 中使用者圈選的區段再縮短 |
| 079 | L505 | 2026-04-25 13:03:55 | 將 GUI 黃橘色上臂改成反 2 反摺造型 |
| 080 | L510 | 2026-04-25 13:08:35 | 依參考照片重構 GUI side view 構圖 |
| 081 | L515 | 2026-04-25 13:13:59 | 壓低 GUI 底座並將肩段改成先垂直再水平 |
| 082 | L520 | 2026-04-25 13:18:17 | 將黃色直桿縮到最短並移除橘色上段 |
| 083 | L525 | 2026-04-25 13:21:52 | 修正第一個關節必須與底座實際連接 |
| 084 | L530 | 2026-04-25 13:24:08 | 將第一個關節改成直接坐在底座上 |
| 085 | L535 | 2026-04-25 13:32:23 | 修正 fore 關節只應轉角且擴展置中 side view |
| 086 | L540 | 2026-04-25 13:32:23 | 修正 side view bounds 導致只剩關節的壓縮問題 |
| 087 | L545 | 2026-04-25 13:58:08 | 將 fore 角度範圍改為 30..90 並再壓低底座台 |
| 088 | L550 | 2026-04-25 14:10:46 | 讓 Motion Preview 內容依實際畫布寬度置中 |
| 089 | L555 | 2026-04-25 14:15:06 | 修正首次開啟 GUI 時 preview 尚未置中 |
| 090 | L560 | 2026-04-25 17:14:32 | 新增可點擊的 3D workspace 雲圖與點選移動 |
| 091 | L565 | 2026-04-25 17:24:20 | 將 3D workspace 升級為 hover、旋轉與按住預覽放開送出 |
| 092 | L570 | 2026-04-26 10:01:06 | 放大 3D workspace 點雲並提高整片區域可點性 |
| 093 | L575 | 2026-04-26 10:09:42 | 將 3D workspace 預設改成前視第一人稱投影 |
| 094 | L580 | 2026-04-26 11:56:54 | 新增 Live Follow 模式與可調反應時間欄位 |
| 095 | L585 | 2026-04-26 12:04:53 | 將 devlog.md 正式納入 Git 版本控制 |
| 096 | L590 | 2026-04-26 14:13:08 | 新增 TOF 避障停車與 GUI 障礙物訊息 |
| 097 | L595 | 2026-04-26 14:24:07 | 新增 GUI TOF 距離儀表與 3D 操作引導 |
| 098 | L600 | 2026-04-26 14:37:41 | 新增 VL53L0X 自動恢復與修正 3D 引導排版 |
| 099 | L605 | 2026-04-26 14:48:53 | 新增 Serial Log 暫停與複製按鍵 |
| 100 | L610 | 2026-04-26 14:55:56 | 補強 VL53L0X 初始化失敗步驟診斷 |
| 101 | L615 | 2026-04-26 16:00:16 | 恢復 CubeMX regenerate 後遺失的 LPUART1 IRQ bridge |
| 102 | L620 | 2026-04-26 16:05:01 | 將 LPUART1 IRQ bridge 抽成自有檔案避免 regenerate 覆蓋 |
| 103 | L625 | 2026-04-26 16:20:38 | 補上 XSPI3 spurious IRQ handler 避免 App 初始化被打斷 |
| 104 | L630 | 2026-04-26 16:36:07 | 補齊 XSPI1 與 XSPI2 defensive handler 避免再次落回 Default_Handler |
| 105 | L639 | 2026-04-29 10:00:00 | 對齊 NUCLEO-N657X0-Q 實機板級的 proposal.md 完善 |
| 106 | L644 | 2026-04-29 13:00:00 | 修正 GUI TOF Safety panel 永遠卡在待機與 0 mm |
| 107 | L649 | 2026-04-29 15:00:00 | 過濾 VL53L0X TCC 噪音避免 GUI 在 20 mm 與正常值間跳動 |
| 108 | L655 | 2026-04-30 11:00:00 | 新增 GUI IMU Attitude 姿態儀面板與 firmware 100 ms imu status |

## 記錄

### 001. 建立 devlog 紀錄準則
- 日期時間：2026-04-21 11:24:55
- 問題原因：目前 devlog.md 為空，尚未定義所有修改的統一記錄格式，也沒有索引與對應行號可供快速查找。
- 處理方式：建立 devlog.md 基本格式，在文件最前面新增索引表並記錄條目起始行號；後續所有修改都必須以新增條目的方式記錄日期時間、問題原因與處理方式。
- 備註：後續新增條目時，需同步更新索引中的行號，讓索引與實際內容保持一致。

### 002. 建立 VS Code Debug 與 Live Watch 設定
- 日期時間：2026-04-21 12:01:29
- 問題原因：專案本身已具備 Debug 友善的編譯旗標與 SWD 腳位設定，但工作區缺少 VS Code 的 launch.json 與 tasks.json，導致無法直接從 VS Code 啟動 Cortex-Debug，也沒有自動建置出供 Live Watch 使用的 ELF 路徑。
- 處理方式：新增 .vscode/launch.json，分別為 Appli 與 FSBL 建立 ST-LINK 的 Cortex-Debug 啟動設定，並開啟 liveWatch；同時新增 .vscode/tasks.json，讓 VS Code 可以先用各自的 Debug preset 執行 configure/build，再啟動除錯。

### 003. 修正 VS Code tasks 的 CMake 路徑依賴
- 日期時間：2026-04-21 12:47:38
- 問題原因：PowerShell 環境無法直接解析 cmake 指令，導致 preLaunchTask 即使存在也會在 configure/build 前失敗。
- 處理方式：將 .vscode/tasks.json 中 Appli 與 FSBL 的 configure/build task 改為直接使用 ${env:ProgramFiles}/CMake/bin/cmake.exe，消除對 shell PATH 的依賴。

### 004. 補齊 CMake preset 的 Ninja 路徑
- 日期時間：2026-04-21 12:53:23
- 問題原因：Appli 的 Debug preset 指定使用 Ninja，但 CMake 在目前環境下無法自動解析 Ninja 執行檔位置，導致 configure 階段失敗。
- 處理方式：在根目錄、Appli 與 FSBL 的 CMakePresets.json 中加入 CMAKE_MAKE_PROGRAM，明確指向 Visual Studio 內建的 ninja.exe，讓 configure/build 可直接完成。

### 005. 讓 toolchain 自動尋找 Arm GCC
- 日期時間：2026-04-21 12:55:30
- 問題原因：gcc-arm-none-eabi.cmake 原本假設 arm-none-eabi-gcc 與 arm-none-eabi-g++ 已存在於 PATH，但目前工作區的 PowerShell 環境未繼承該設定，導致 configure 無法識別交叉編譯器。
- 處理方式：在 gcc-arm-none-eabi.cmake 內加入 Windows 專用的工具鏈自動搜尋邏輯，優先搜尋 Arm GNU Toolchain 與 STM32CubeIDE 內建的 GNU tools，找到後改用完整路徑組出 TOOLCHAIN_PREFIX。

### 006. 修正 CMake 對 ProgramFiles(x86) 的解析
- 日期時間：2026-04-21 12:56:11
- 問題原因：CMake 的 $ENV{} 語法無法直接解析帶括號的環境變數名稱，導致 ProgramFiles(x86) 搜尋 pattern 在 configure 前就觸發語法錯誤。
- 處理方式：將該搜尋 pattern 改為 CMake 可直接解析的字面路徑 C:/Program Files (x86)/...，保留其他自動搜尋邏輯不變。

### 007. 補上 Windows 工具鏈執行檔副檔名
- 日期時間：2026-04-21 12:56:53
- 問題原因：自動搜尋邏輯已找到 Arm GNU Toolchain 目錄，但 CMAKE_C_COMPILER 與 CMAKE_CXX_COMPILER 在 Windows 上使用完整路徑時仍需包含 .exe，否則 CMake 會判定該編譯器不存在。
- 處理方式：在 gcc-arm-none-eabi.cmake 中新增 TOOLCHAIN_EXECUTABLE_SUFFIX，於 Windows 環境下統一為 gcc、g++、objcopy、size 等工具補上 .exe。

### 008. 補齊 Cortex-Debug 的 GDB 路徑設定
- 日期時間：2026-04-21 13:31:19
- 問題原因：Cortex-Debug extension 在目前工作區找不到 arm-none-eabi-gdb.exe，導致除錯器在啟動前就中止，即使 Appli 的 Debug preset 已可成功建出 ELF 也無法進入除錯。
- 處理方式：新增 .vscode/settings.json，明確設定 cortex-debug.armToolchainPath 與 cortex-debug.gdbPath，直接指向已安裝的 Arm GNU Toolchain 14.2 rel1 bin 目錄與 arm-none-eabi-gdb.exe。

### 009. 新增 OpenOCD Live Watch 啟動設定
- 日期時間：2026-04-21 15:16:52
- 問題原因：目前的 launch.json 使用 servertype=stlink；依 Cortex-Debug 文件，Live Watch 只對 OpenOCD 生效，因此雖然 ST-LINK 除錯可啟動，但無法滿足 Live Watch 需求。
- 處理方式：保留既有 ST-LINK 啟動設定供一般除錯使用，另外為 Appli 與 FSBL 新增使用 OpenOCD 的 Cortex-Debug 組態，補上 openocd.exe、st_scripts 搜尋路徑、stm32n6x.cfg 與 STM32N657.svd，並只在 OpenOCD 組態中啟用 liveWatch。

### 010. 改為 under-reset 連線模式以穩定 halt target
- 日期時間：2026-04-21 17:06:31
- 問題原因：ST-LINK gdb-server 在預設 launch 流程中以 reset 後 halt 的方式接管 target，但 STM32N657X0HxQ 目前會在 reset 後無法被正常 halt，導致 ST-LINK server 直接中止；同時 OpenOCD 的 STM32N6 腳本明確支援 CONNECT_UNDER_RESET 以避免相同問題。
- 處理方式：為 ST-LINK 的 Appli 與 FSBL 組態加入 serverArgs，啟用 initialize-reset 與較長的 pending halt timeout；同時為 OpenOCD 組態加入 openOCDPreConfigLaunchCommands 與 openOCDLaunchCommands，啟用 CONNECT_UNDER_RESET 與 connect_assert_srst，讓 debug session 以 under-reset 模式接管 target。

### 011. 降低 debug 接管頻率以改善 reset 後 halt
- 日期時間：2026-04-21 19:32:13
- 問題原因：目前 ST-LINK gdb-server 已改為 under-reset 方式接管 target，但仍在 reset 後 halt 階段失敗；這類問題在高 SWD 速度下較常出現在安全開機或早期時脈尚未穩定的目標上。
- 處理方式：將 ST-LINK 的 launch serverArgs 補上 `--frequency 1000`，把 debug 連線速度降到 1000 kHz；同時在 OpenOCD 組態的 pre-config 階段加入 `set CLOCK_FREQ 1000`，讓兩條 debug 路徑都以較保守的速度接管 target。

### 012. 對齊 G431 專案的 pinned-probe 與 attach 設定
- 日期時間：2026-04-21 19:55:49
- 問題原因：比對 `ST_FOC_G431_IMH` 後，該專案會在 launch.json 內固定 ST-LINK 序號、ST-LINK gdb-server 路徑與 STM32CubeProgrammer 路徑；而 N6 專案原本沒有固定 probe，且其 `launch + halt/reset` 流程實測會失敗，但 `attach + 固定序號` 能讓 gdb-server 正常進入 waiting for debugger connection。
- 處理方式：為 N6 的 ST-LINK 組態補上 `serialNumber`、`serverpath`、`stm32cubeprogrammer`，並新增 `Attach Appli (ST-LINK, pinned probe)` 與 `Attach FSBL (ST-LINK, pinned probe)`；另外在 settings.json 補上 `cortex-debug.stm32cubeprogrammer.windows` 與 windows 專用 toolchain 設定，使工作區更接近 G431 的已驗證配置。

### 013. 修正 STM32N6 的 ST-LINK 核心 AP 選擇
- 日期時間：2026-04-21 20:23:58
- 問題原因：比對 STM32N6 的 OpenOCD target 腳本與實測 log 後確認，ST-LINK gdb-server 在預設 `apid=0` 時只能讀到記憶體 AP，GDB 一連上就在 `haltOnConnect()` 階段失敗；改成 `apid=1` 後，server 會辨識 `TrustZone: Active`，並能成功建立 GDB session。
- 處理方式：在所有 ST-LINK 的 launch 與 attach 配置中加入 `serverArgs: ["-m", "1"]`，將 Cortex-Debug 啟動的 ST-LINK gdb-server 固定到 AP1，也就是 STM32N6 的 Cortex-M55 核心所在 access port。

### 014. 改用 Appli RAM image 啟動流程以停在 main
- 日期時間：2026-04-21 20:36:50
- 問題原因：STM32N6 FullSecure 專案的 reset 後停點實際落在 0x1800xxxx 啟動區，該位址不屬於 Appli ELF 的 0x3400xxxx 符號空間，因此 `runToEntryPoint: "main"` 無法命中，暫停與單步也只會落在未知來源的 boot code。
- 處理方式：將 `Debug Appli (ST-LINK)` 改為 RAM image 專用啟動序列，連線後先 `load` Appli ELF 到 0x34000400 起始位址，再設定 VTOR、SP、PC 到 Appli vector table，最後以 `tbreak main` + `continue` 停在 `main()`；同時把 attach 組態名稱改成 running image，避免誤以為 attach 會在進入點停下。

### 015. 改由 Cortex-Debug 接手停在 main 與 Pause 中斷
- 日期時間：2026-04-21 21:33:53
- 問題原因：手動 GDB 驗證可證明 RAM image 載入後確實能命中 `main()`，但在 Cortex-Debug 中把 `continue` 直接放進 `overrideLaunchCommands`，可能會讓 extension 前端狀態與 GDB 實際 stop event 不同步，造成看起來沒有停在 `main()`，且按 Pause 也沒有反應。
- 處理方式：保留 Appli RAM image 的 `load`、VTOR、SP、PC 切換，但移除 `overrideLaunchCommands` 內的 `tbreak main` 與 `continue`，改回使用 Cortex-Debug 內建的 `runToEntryPoint: "main"` 接手進入點停車；另外新增 `gdbInterruptMode: "sigint"`，改用較穩定的 GDB 中斷方式改善 Pause 無反應問題。

### 016. 整理 STM32N6 Debug 避坑排查順序
- 日期時間：2026-04-21 21:38:52
- 問題原因：這次除錯問題不是單一設定錯誤，而是工作區工具鏈路徑、STM32N6 TrustZone AP 選擇、Appli 映像位址空間、以及 Cortex-Debug 與 GDB 停止事件同步四層問題互相遮蔽；若只看表面症狀，會反覆誤判成 GDB、ST-LINK 或 main 斷點本身失效。
- 處理方式：整理下次固定排查順序如下：先確認 Appli Debug build、GDB、ST-LINK gdb-server 與 STM32CubeProgrammer 路徑都正常；再確認 STM32N6 的 ST-LINK 一律走 AP1，也就是 `serverArgs: ["-m", "1"]`；若要進入 Appli source-level debug，必須使用 RAM image 啟動流程，也就是先 `load` Appli ELF，再把 VTOR、SP、PC 切到 `0x34000400` 與 `0x34000404`，最後交回 `runToEntryPoint: "main"`；Attach 組態只用來接已經在跑的映像，不應期待自動停在 `main()`；若 Pause 再次無反應，優先檢查 `gdbInterruptMode: "sigint"` 是否存在，且不要把 `continue` 直接寫進 `overrideLaunchCommands`。

### 017. 完成 Phase 0 的 UART log、LED blink 與 Button input
- 日期時間：2026-04-21 21:46:52
- 問題原因：proposal 的 Phase 0 需要最小可編譯、可燒錄、可 log 的基底工程，但目前 Appli 只有 `MX_GPIO_Init()` 的空殼，沒有板載 LED、USER_BUTTON 與 UART log；另外 Appli 的 CMake 尚未把 HAL UART driver 納入編譯，直接加入 UART 初始化會在連結階段失敗。
- 處理方式：在 Appli 的 CMake 額外加入 `stm32n6xx_hal_uart.c` 與 `stm32n6xx_hal_uart_ex.c`；沿用目前 `.ioc` 已保留給 ST-LINK VCP 的 `LPUART1` on `PE5/PE6` 作為 Phase 0 log 通道，而不是 proposal 內原先建議的 `PA9/PA10`；在 `gpio.c` 初始化板載 LED 與 USER_BUTTON，在 `stm32n6xx_hal_msp.c` 補上 `LPUART1` 的 MSP 設定，並在 `main.c` 建立最小 heartbeat loop：`LD3` 週期閃爍、`LD2` 反映按鍵狀態、UART 每秒輸出 tick 與按鍵變化，完成 Phase 0 的最小 bring-up。

### 018. 將 Phase 0 自訂修改搬回 USER CODE 區塊避免被 CubeMX 覆蓋
- 日期時間：2026-04-22 06:56:19
- 問題原因：上一輪 Phase 0 已能編譯，但部分修改直接落在 CubeMX 會重生的區域，例如 `main.h` 的 private defines、`MX_GPIO_Init()` 的函式本體、`main()` 內 user block 外的區域與 `SystemIsolation_Config()` 的 generated pin attributes；只要重新 Generate Code，這些內容就有高機率被覆蓋。
- 處理方式：將板載 pin define 移回 `main.h` 的 `USER CODE BEGIN Private defines`；把 LED 與按鍵初始化改成 `gpio.c` 的 `Phase0_GPIO_Init()` helper 並放在 `USER CODE BEGIN 2`，再由 `main.c` 的 `USER CODE BEGIN 2` 呼叫；將 `main()` 內的自訂區域變數與 `MX_LPUART1_UART_Init()` 呼叫搬回 user block；另外把 Phase 0 的 GPIO security attribute 改成 `Phase0_ConfigureSystemIsolation()` 並只在 `RIF_Init` 的 user block 內執行，避免下次 CubeMX 生成時把自訂 bring-up 邏輯洗掉。

### 019. 補上 Appli RAM image 的 Reset 與 Restart 重載流程
- 日期時間：2026-04-22 12:24:14
- 問題原因：`Debug Appli (ST-LINK)` 目前只有 `overrideLaunchCommands` 會在啟動時把 Appli ELF 載入到 `0x34000400` 並重設 `VTOR/SP/PC`，但工具列的 Reset 與 VS Code 的 Restart 走的是另一條 Cortex-Debug 內建流程；對 STM32N6 FullSecure 的 RAM image 專案來說，這會把核心帶回 `0x1800xxxx` boot 區，之後 Pause 看到的就只會是 boot code，而不是 Appli 的 `main()`。
- 處理方式：保留現有 `overrideLaunchCommands` 不變，另外在 `launch.json` 為 `Debug Appli (ST-LINK)` 補上 `postResetCommands` 與 `postRestartCommands`，在每次 Reset/Restart 完成後重新 `load` Appli ELF，並再次設定 `VTOR/SP/PC` 指向 `0x34000400` 向量表，讓 Cortex-Debug 既保留自己的 reset 流程，又能在 reset 後回到 Appli RAM image，再由 `runToEntryPoint: "main"` 接手停車。

### 020. 補上 Appli Phase 1 的 I2C1 bring-up 與掃描
- 日期時間：2026-04-22 20:24:44
- 問題原因：proposal 的下一步是 Phase 1 I2C bring-up，但 Appli 端當前只完成 Phase 0；雖然 `.ioc` 已有 I2C1 的 timing 與 PH9/PC1 pin 配置可參考，Appli 仍缺少 HAL_I2C 模組開關、driver source、MSP 與初始化流程，因此無法直接做 I2C 掃描驗證。
- 處理方式：在 Appli 啟用 `HAL_I2C_MODULE_ENABLED` 並把 `stm32n6xx_hal_i2c.c`、`stm32n6xx_hal_i2c_ex.c` 納入 CMake；另外在 `main.h`、`stm32n6xx_hal_msp.c` 與 `main.c` 補上 I2C1 的 PH9/PC1 pin 定義、MSP、初始化與開機掃描 log，沿用 FSBL 已驗證的 `0x30C0EDFF` timing；最後以 Appli Debug preset 執行 clean-first build，確認可成功重新編譯連結。

### 021. 將 I2C HAL 開關移到 CMake 以避開 CubeMX 覆蓋
- 日期時間：2026-04-22 20:30:36
- 問題原因：前一版 Phase 1 bring-up 直接把 `HAL_I2C_MODULE_ENABLED` 寫進 `stm32n6xx_hal_conf.h`；該檔是 CubeMX 生成檔，下次 Generate Code 時這一行有高機率被覆蓋，導致 `main.c` 的 I2C API 宣告與 driver include 消失。
- 處理方式：把 I2C HAL 模組開關改移到 Appli 的 `CMakeLists.txt`，透過 `target_compile_definitions(stm32cubemx INTERFACE HAL_I2C_MODULE_ENABLED)` 讓應用程式與 `STM32_Drivers` 共同繼承；同時把 `stm32n6xx_hal_conf.h` 還原回 CubeMX 的預設註解狀態，並重新 configure + clean-first build 驗證功能不變。

### 022. 新增 Phase 1 感測器辨識以驗證 MPU6050 與 VL53L0X 接線
- 日期時間：2026-04-23 14:39:09
- 問題原因：目前 Appli 的 Phase 1 只有 I2C address scan，雖然能知道匯流排上有裝置應答，但無法直接確認接上的 0x68/0x69 與 0x29 到底是不是 MPU6050 與 VL53L0X，也無法在接線剛完成時快速判斷是哪一顆裝置讀值異常。
- 處理方式：在 `main.c` 的 Phase 1 掃描後追加兩個最小辨識讀取，針對 MPU6050 讀 `WHO_AM_I(0x75)`，針對 VL53L0X 讀 `MODEL_ID(0xC0)`，並把結果直接輸出到 UART log；同時保留原本的全匯流排掃描，讓 bring-up 階段可以同時看到位址與晶片識別資訊。

### 023. 補上 Phase 1 感測器讀值與修正按鍵極性
- 日期時間：2026-04-23 14:53:23
- 問題原因：目前硬體接線已確認正確，但 Appli 還缺少三個最直接的 bring-up 驗證面：USER_BUTTON 狀態判讀極性與實際板上行為相反、MPU6050 尚未從睡眠喚醒並輸出原始 accel/gyro/temperature 資料、VL53L0X 也尚未做最小初始化與單次距離量測，因此只能停留在「位址存在」而不是「功能可用」。
- 處理方式：將 `Phase0_IsButtonPressed()` 改成以 `GPIO_PIN_SET` 判定 pressed，對齊目前 PC13 pulldown 接法；另外在 `main.c` 補上通用 I2C register read/write helper、MPU6050 的最小初始化與 14-byte raw sample 解析，以及 VL53L0X 的 stop-variable 讀取、single-shot ranging 啟動與距離讀值，並在開機後與每秒 telemetry log 中輸出感測器資料，讓 Phase 1 可以直接觀察感測器功能是否正常。

### 024. 強化 Phase 1 telemetry 以判讀 MPU6050 與 VL53L0X 量測品質
- 日期時間：2026-04-23 15:04:20
- 問題原因：上一版 telemetry 已能看到感測器有輸出，但 MPU6050 仍是 raw LSB，不利於直接判斷姿態與角速度量級；同時 VL53L0X 目前出現 20 mm 與 57x mm 交錯的結果，若只印距離值，無法分辨是正常近距離、量測跳變，還是 range status 本身就已經提示異常。
- 處理方式：把 MPU6050 的 accel 與 gyro 轉成 `mg` 與 `mdps` 後直接輸出；另外將 VL53L0X 的 single-shot 讀值擴充為同時輸出 `RESULT_RANGE_STATUS` raw byte、status code、與相對上一筆的 delta，並標記 `ok`、`near-floor`、`jump` 或 `status-check`，讓後續判讀硬體環境與初始化品質時有足夠的訊號可看。

### 025. 修正 VL53L0X raw status 11 的判讀避免誤報異常
- 日期時間：2026-04-23 15:11:08
- 問題原因：根據 ST/Adafruit 的 VL53L0X driver，`RESULT_RANGE_STATUS` 內部 raw device code `11` 代表 `Range Complete`，是正常量測完成；前一版 telemetry 直接把非零 raw code 視為可疑，導致即使距離值穩定，也會被誤標成 `status-check`。
- 處理方式：在 `main.c` 補上 VL53L0X raw device code 到文字說明的映射，並把 `device_code=11` 改視為有效量測；UART log 會改成輸出 `device=range-complete`，只有非 11 的 raw device code 才標記為 `device-error`，避免把正常量測誤判成異常。

### 026. 抽離應用層模組以避免 CubeMX 生成覆蓋
- 日期時間：2026-04-23 15:52:04
- 問題原因：目前 Phase 0/Phase 1 的自訂邏輯分散在 `main.c`、`gpio.c`、`stm32n6xx_hal_msp.c`、`main.h` 等 CubeMX 生成檔中，即使多數位於 USER CODE 區塊，後續切 TIM/PWM 並重新 generate 時仍不利於集中維護，也提高了遺漏或覆蓋自訂程式的風險。
- 處理方式：新增 `Appli/Core/Inc/app_main.h` 與 `Appli/Core/Src/app_main.c`，將 UART/I2C bring-up、LED/Button、MPU6050/VL53L0X telemetry 與 MSP callback 全部抽到獨立應用模組；`main.c` 改成只保留 `App_Init()`、`App_RunLoopIteration()`、`App_ConfigureSystemIsolation()` hook，並同步從 `gpio.c`、`gpio.h`、`stm32n6xx_hal_msp.c`、`main.h` 移除舊的自訂應用碼，最後更新 `Appli/CMakeLists.txt` 並完成 Debug build 驗證。

### 027. 補上 CubeMX 生成後遺漏的 TIM 共用時脈設定呼叫
- 日期時間：2026-04-23 15:58:42
- 問題原因：CubeMX 這次為 TIM1 PWM 新生成了 `PeriphCommonClock_Config()`，裡面會設定 `RCC_TIMPRES_DIV1`；但 `main.c` 內沒有任何地方呼叫這個函式，代表 timer 共用時脈前提可能沒有被套用，會讓後續 50 Hz servo PWM 的時脈假設失去保證。
- 處理方式：在 `main.c` 的 USER CODE 區塊補上 `PeriphCommonClock_Config()` 原型與啟動呼叫，讓這個 post-generate 修正留在可保留區；之後重新建置 `Appli` Debug，確認編譯與連結都成功，僅保留既有的 RWX segment linker warning。

### 028. 新增 TIM1 CH1 舵機安全起始與按鈕切換控制
- 日期時間：2026-04-23 16:03:33
- 問題原因：雖然 CubeMX 已生成 TIM1 PWM，但 `app_main.c` 仍未啟動 `TIM1 CH1`、也沒有任何安全起始脈寬或按鈕邊緣切換控制；若直接進入後續夾爪 bring-up，仍缺少最小可控的伺服測試路徑。
- 處理方式：在 `app_main.c` 納入 `tim.h`，於 `App_Init()` 啟動 `HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1)`，先套用 1500 us 安全起始脈寬，再以 USER button 的上升沿在 1350 us 與 1650 us 之間切換，並加上 150 ms 防彈跳保護與 UART log；完成後強制重建 `Appli` Debug，確認編譯與連結成功。

### 029. 依實際 TIMG 時脈校正舵機 PWM 週期與脈寬
- 日期時間：2026-04-23 16:16:21
- 問題原因：實測示波器顯示 TIM1 CH1 輸出約 8 Hz、high 約 10 ms，這與 `PSC=399 / ARR=19999 / CCR=1500` 在 64 MHz TIMG clock 下的結果一致，表示目前 PWM 已有輸出，但時基不是預期的 1 MHz tick，因此 50 Hz servo 脈波被錯算成低頻長脈寬。
- 處理方式：在 `app_main.c` 內改用 `HAL_RCCEx_GetTIMGFreq()` 讀取實際 timer group clock，於 `App_ServoInit()` 動態重設 `TIM1` prescaler 與 auto-reload，固定建立 1 MHz tick 與 20 ms period，再保留 1350 / 1500 / 1650 us 的舵機脈寬語意；完成後強制重建 `Appli` Debug，確認修正可正常編譯連結。

### 030. 放大舵機開合脈寬到 1100 與 1900 us
- 日期時間：2026-04-23 16:24:59
- 問題原因：目前按鈕切換後夾爪已可見小幅動作，表示 PWM、接線與供電都已打通；但既有 1350 / 1650 us 行程仍偏保守，不足以讓夾爪得到更明顯的開合幅度。
- 處理方式：將 `app_main.c` 內 `APP_SERVO_OPEN_PULSE_US` 與 `APP_SERVO_CLOSE_PULSE_US` 分別調整為 1100 us 與 1900 us，保留 1500 us 安全中位；之後強制重建 `Appli` Debug，確認調整後韌體可正常編譯連結，供實機再測較大的夾爪行程。

### 031. 切換到舵機安裝中位模式固定 1500 us
- 日期時間：2026-04-23 18:55:26
- 問題原因：目前問題已從 PWM 與接線轉為機械校位；若繼續保留按鈕切換開合，夾爪在裝舵盤與連桿時仍可能被誤推到極限，不利於把舵機 spline 對到真正中位。
- 處理方式：在 `app_main.c` 新增 `APP_SERVO_INSTALL_CENTER_MODE`，啟用後開機固定輸出 1500 us 中位脈寬，並將按鈕行為改為只重申 `center hold=1500 us` 而不再推動夾爪；同時更新 UART 提示讓裝配時能直接依照「拆舵盤、上電置中、對齊中點」流程操作，最後強制重建 `Appli` Debug 驗證修改成功。

### 032. 新增舵機安全步進量測模式以找出夾爪可用範圍
- 日期時間：2026-04-24 08:10:34
- 問題原因：若舵機電子中位本來就接近夾爪機構中位，代表限制夾爪開合幅度的主因很可能是連桿幾何或機構行程，而不是 PWM 中位偏掉；此時需要一種能從中位往兩側慢慢探測的安全方式，才能量出真正可用的脈寬範圍而不直接撞機構。
- 處理方式：將 `app_main.c` 的安裝模式改為 range-find 模式，開機仍先停在 1500 us，中位附近用短按單步調整 25 us，長按則回到 1500 us 並反轉下一次步進方向，同時持續在 UART 印出目前 pulse、下一步方向與 1000..2000 us 的保護夾限；之後強制重建 `Appli` Debug，確認量測模式可正常編譯連結，供實機逐步找出夾爪兩端的安全脈寬。

### 033. 依實測結果收斂舵機量測窗到 700..1600 us
- 日期時間：2026-04-24 08:22:55
- 問題原因：實機量測顯示夾爪在 1600 us 已經夾緊，代表 close 側再往上推只會增加撞機構風險；另一方面 open 側在 1000 us 就被保護夾限擋住，卻還沒有完全張開，表示原先的 1000..2000 us 探測窗對這組機構來說方向不對稱，且 open 側保守過頭。
- 處理方式：在 `app_main.c` 將 range-find 模式的量測窗調整為 700..1600 us，並把單步步進從 25 us 縮為 20 us，讓 close 側不再超過已知極限，同時保留足夠向 open 側繼續探測的空間；之後重新建置 `Appli` Debug 驗證修改可正常編譯，供下一輪實機記錄真正的最開脈寬。

### 034. 改成按鍵觸發的鬆夾鬆正式動作模式
- 日期時間：2026-04-24 08:28:04
- 問題原因：夾爪可用範圍已經透過實機確認為 700 us 放開、1600 us 夾緊，因此原本的 range-find 量測模式已達成目的；若繼續保留量測分支，實際操作時仍要靠多次短按與方向切換，不符合最終要用單一按鍵直接執行夾取動作的需求。
- 處理方式：在 `app_main.c` 移除 range-find 控制路徑，正式設定 `APP_SERVO_OPEN_PULSE_US=700` 與 `APP_SERVO_CLOSE_PULSE_US=1600`，改為按下按鍵後啟動非阻塞的「鬆→夾→鬆」序列，開機預設保持放開位置，並在 UART 印出 cycle start、clamp、finish 狀態；之後強制重建 `Appli` Debug，確認正式動作模式可正常編譯連結。

### 035. 改成按一下夾住再按一下放開的切換式動作
- 日期時間：2026-04-24 08:50:53
- 問題原因：雖然「鬆→夾→鬆」序列能驗證夾爪往返，但在實際使用上無法保持夾住狀態，不利於夾取物件後停留；需要把按鍵行為改成具記憶性的 toggle，讓每次按下按鍵只在夾住與放開兩個穩態間切換。
- 處理方式：在 `app_main.c` 移除鬆夾鬆序列 state 與計時邏輯，保留 700 us 放開與 1600 us 夾緊兩個正式脈寬，改為按鍵按下後依目前 `app_servo_state` 在 open/close 間切換，並同步更新 UART log 與 heartbeat 狀態字串；之後強制重建 `Appli` Debug，確認切換式動作模式可正常編譯連結。

### 036. 初始化 Git 並推送到 GitHub 遠端但排除 devlog
- 日期時間：2026-04-24 08:58:31
- 問題原因：工作區原本還不是 Git repository，也沒有設定任何 GitHub remote；使用者要求將目前專案推送到指定的 GitHub 倉庫，但明確要求不要把 `devlog.md` 一起上傳。
- 處理方式：在專案根目錄初始化 Git 並建立 `main` 分支，設定遠端 `origin` 指向 `https://github.com/MMLL168/RobotArm_STM32N6.git`，接著在本機 `.git/info/exclude` 排除 `devlog.md` 與 `Appli/build/`、`FSBL/build/` 生成目錄，再以 `Initial import` 建立第一次 commit 並成功 push 到 `origin/main`；其中 `devlog.md` 仍保留在本機並維持 ignore 狀態，不會被推送。

### 037. 啟用第二顆舵機 CH2 並固定中位做安全 bring-up
- 日期時間：2026-04-24 09:21:18
- 問題原因：CubeMX 生成後，TIM1 CH2 與 PE11 已經正確出現在生成檔中，但應用層 `app_main.c` 仍只會設定與啟動 CH1，因此第二顆「手臂前後」舵機雖然已經有 pin 與 timer channel，實際上還不會輸出 PWM，也沒有安全的上電中位。
- 處理方式：在 `app_main.c` 新增第二顆舵機的中位脈寬常數與目前 pulse 狀態，於 `App_ServoInit()` 同時對 TIM1 CH1 與 CH2 寫入 compare 值並啟動 PWM，其中 CH1 維持原本夾爪 700/1600 us toggle 控制，CH2 固定輸出 1500 us 當作手臂前後舵機的安全中位；同時更新 heartbeat 與 UART 啟動訊息，讓 log 能同時看到 gripper 與 arm 的 pulse，最後強制重建 `Appli` Debug 驗證修改成功。

### 038. 暫時把按鍵改成只量 CH2 範圍並固定夾爪不動
- 日期時間：2026-04-24 10:25:06
- 問題原因：第二顆「手臂前後」舵機已經能在 CH2 上電置中，但若繼續保留按鍵去切換夾爪 CH1，使用者就無法在同一顆按鍵上安全地量測手臂前後軸的可用範圍；因此需要暫時把按鍵功能切到 CH2，並讓 CH1 停在固定放開位置避免干擾。
- 處理方式：在 `app_main.c` 保持 CH1 夾爪輸出固定 700 us 不動，將按鍵邏輯改為只控制 CH2 的安全步進量測模式：短按單步 20 us、長按回到 1500 us 並反轉方向，初始保護窗設定為 1200..1800 us；同時更新 heartbeat 與 UART 提示，讓 log 能直接看到 arm 目前 pulse 與下一步方向，最後強制重建 `Appli` Debug 確認量測模式可正常編譯連結。

### 039. 將 CH2 量測下限由 1200 放寬到 900 us
- 日期時間：2026-04-24 10:45:58
- 問題原因：實機量測顯示 CH2 在 1800 us 的前推端已可接受，但 1200 us 這一側仍不足以達到需要的 clamp 位置，表示原本的 CH2 保護窗下限設得過高，限制了夾緊端的繼續探測。
- 處理方式：在 `app_main.c` 將 `APP_ARM_SERVO_RANGE_MIN_PULSE_US` 從 1200 us 下修為 900 us，保留 20 us 步進與 1800 us 上限不變，讓按鍵量測模式能繼續往夾緊端探測；之後重新建置 `Appli` Debug，確認放寬後的量測窗可正常編譯連結。

### 040. 將 CH2 定成 900 與 1800 的正式前後切換模式
- 日期時間：2026-04-24 10:50:02
- 問題原因：CH2 的兩端點已經透過實機確認為 900 us 與 1800 us 可用，因此原本的量測步進模式已達成任務；若繼續保留量測模式，操作上仍需多次短按與方向切換，不適合作為正式的手臂前後控制。
- 處理方式：在 `app_main.c` 移除 CH2 的步進量測邏輯，改為正式的前後控制模式：短按在 900 us 與 1800 us 間切換，長按回到 1500 us 中位；同時保留 CH1 固定放開不動，更新 heartbeat 與 UART 提示為 arm 的 front/back/center 狀態，最後強制重建 `Appli` Debug 驗證正式模式可正常編譯連結。

### 041. 檢查 regenerate 並接入第三顆舵機 CH3 安全量測模式
- 日期時間：2026-04-24 11:31:23
- 問題原因：CubeMX regenerate 後已新增 PA2/TIM2 CH3，但需要先確認既有的 `main.c` wrapper、`app_main.c` 應用層接點，以及 TIM1 CH1/CH2 的生成內容沒有被覆蓋；同時，新生成的 TIM2 仍只有 CubeMX 預設 PWM，尚未接入既有的安全量測流程，也尚未套用 runtime 的 1 MHz / 20 ms 舵機時基校正。
- 處理方式：先檢查 `main.c` 仍保留 `App_Init()`、`App_RunLoopIteration()`、`PeriphCommonClock_Config()` 與 `App_ConfigureSystemIsolation()` 呼叫，並確認 TIM1 CH1/CH2 與新 TIM2 CH3 的生成內容完整；接著在 `app_main.c` 將按鍵控制暫時切換到第三顆舵機 CH3，保留 CH1 固定放開、CH2 固定 1500 us 中位，讓 CH3 使用短按 20 us 步進、長按回中並反轉方向的安全量測模式，初始保護窗設為 1300..1700 us，並把 TIM2 一併納入 `HAL_RCCEx_GetTIMGFreq()` 的時基校正；最後重建 `Appli` Debug，確認 regenerate 後既有功能未被覆蓋且新量測模式可正常編譯連結。

### 042. 將第三顆舵機 CH3 量測下限由 1300 放寬到 1000 us
- 日期時間：2026-04-24 12:01:55
- 問題原因：實機回報顯示第三顆「夾爪上下」舵機在 1300 us 時仍未到達最低端，表示目前 CH3 量測保護窗下限設得過高，會提早夾住探測範圍，無法繼續往下量出真正可用的最低位置。
- 處理方式：在 `app_main.c` 將 `APP_LIFT_SERVO_RANGE_MIN_PULSE_US` 從 1300 us 下修為 1000 us，保留 20 us 步進、1700 us 上限與長按回中反轉方向的安全量測模式不變；之後重新建置 `Appli` Debug，確認放寬後的 CH3 量測窗可正常編譯連結，供實機繼續往最低端探測。

### 043. 將第三顆舵機 CH3 量測上限由 1700 放寬到 1900 us
- 日期時間：2026-04-24 12:04:28
- 問題原因：實機回報顯示第三顆「夾爪上下」舵機在 1700 us 時仍未到達上方端點，表示目前 CH3 量測保護窗上限設得過低，會過早限制往上方向的探測範圍，無法繼續量出真正可用的最高位置。
- 處理方式：在 `app_main.c` 將 `APP_LIFT_SERVO_RANGE_MAX_PULSE_US` 從 1700 us 上修為 1900 us，保留 20 us 步進、1000 us 下限與長按回中反轉方向的安全量測模式不變；之後重新建置 `Appli` Debug，確認放寬後的 CH3 量測窗可正常編譯連結，供實機繼續往上端探測。

### 044. 將第三顆舵機 CH3 定成 1000 與 1500 的正式上下切換模式
- 日期時間：2026-04-24 12:08:05
- 問題原因：第三顆「夾爪上下」舵機已透過實機確認 1000 us 可到達下方位置，而 1500 us 就已經是上方最高位置，因此原本的步進量測模式與 1900 us 上限已不再符合實際機構範圍，若繼續保留只會讓操作語意混亂並增加誤動作空間。
- 處理方式：在 `app_main.c` 移除 CH3 的步進量測與反轉方向邏輯，正式設定 CH3 以 1000 us 為 bottom、1500 us 為 top；按鍵短按在 top/bottom 間切換，長按固定回到 top 安全位置，同時更新 heartbeat 與 UART 提示為 lift 的正式狀態字串；之後重新建置 `Appli` Debug，確認 CH3 正式模式可正常編譯連結。

### 045. 將舵機命名改成 clamp、fore_aft、gripper_lift 語意
- 日期時間：2026-04-24 12:12:43
- 問題原因：雖然目前三顆舵機的功能已經分別固定，但 `app_main.c` 內仍混有 generic 的 `servo`、抽象的 `arm`、`lift` 與舊的 top/bottom 描述，對照實際機構時不夠直接，會讓後續擴充第四顆舵機或調整行為時難以快速分辨 CH1、CH2、CH3 各自對應的機構語意。
- 處理方式：在 `app_main.c` 將 CH1 改名為 `clamp`、CH2 改名為 `fore_aft`、CH3 改名為 `gripper_lift`，同步調整對應的 pulse 常數、狀態列舉、靜態變數、helper 函式名稱，以及 heartbeat 與 UART 啟動/切換 log 文字；之後重新建置 `Appli` Debug，確認語意重命名後程式仍可正常編譯連結。

### 046. 檢查第四顆舵機 regenerate 並接入 CH4 安全量測模式
- 日期時間：2026-04-24 12:23:22
- 問題原因：CubeMX regenerate 後已新增 PA3/TIM16 CH1 與對應的 `MX_TIM16_Init()`、RIF pin attributes、TIM16 GPIO post-init，但仍需要先確認既有 `main.c` wrapper、TIM1/TIM2 生成內容與前面三顆舵機的 app_main 接點沒有被覆蓋；同時，新生成的 TIM16 只有基礎 PWM，尚未被應用層啟動，也尚未納入既有的 runtime 舵機時基校正與安全量測流程。
- 處理方式：先檢查 `main.c` 仍保留 `App_Init()`、`App_RunLoopIteration()`、`PeriphCommonClock_Config()` 與 `App_ConfigureSystemIsolation()`，並確認 TIM1/TIM2 與新增的 TIM16 生成內容完整；接著在 `app_main.c` 保持 clamp CH1 固定 open、fore_aft CH2 固定 center、gripper_lift CH3 固定 up，將按鍵控制暫時切換到第四顆舵機 CH4 的安全步進量測模式，讓 TIM16 CH1 使用短按 20 us 步進、長按回 1500 us 中位並反轉方向的量測流程，初始保護窗設為 1300..1700 us，並把 TIM16 一併納入 `HAL_RCCEx_GetTIMGFreq()` 的時基校正；最後重建 `Appli` Debug，先修掉新增的未使用狀態警告，再確認整體只剩既有 linker RWX warning。

### 047. 將第四顆舵機 CH4 量測窗由 1300..1700 放寬到 1000..1900 us
- 日期時間：2026-04-24 14:01:21
- 問題原因：第四顆舵機 CH4 目前仍在安全量測模式，但實機回報顯示 1300 us 與 1700 us 都還沒碰到實際可用端點，代表原先保護窗太窄，短按步進時無法再往上下兩側繼續探索，會卡住目前的量測流程。
- 處理方式：在 `app_main.c` 只調整 CH4 的量測上下限常數，將最小脈寬由 1300 us 放寬到 1000 us、最大脈寬由 1700 us 放寬到 1900 us，保留原本 20 us 步進、長按回 1500 us 並反轉方向的安全量測機制不變；之後重新建置 `Appli` Debug，確認修改後仍可正常編譯連結，且沒有新增其他 warning，僅保留既有 linker RWX warning。

### 048. 將第四顆舵機 CH4 量測窗由 1000..1900 放寬到 700..2300 us
- 日期時間：2026-04-24 14:04:52
- 問題原因：在將第四顆舵機 CH4 的量測窗放寬到 1000..1900 us 後，實機回報仍顯示上下兩側都還未碰到實際端點，表示目前量測窗依然不足以涵蓋這顆舵機在機構上的有效行程，若維持原值會讓量測流程持續被上限與下限夾住。
- 處理方式：在 `app_main.c` 進一步把 CH4 的量測下限由 1000 us 放寬到 700 us、上限由 1900 us 放寬到 2300 us，其他量測行為維持不變，仍採用短按 20 us 步進與長按回 1500 us 並反轉方向的安全流程；之後重新建置 `Appli` Debug，確認擴大量測窗後仍可正常編譯連結，且沒有新增其他 warning，僅保留既有 linker RWX warning。

### 049. 將第四顆舵機 CH4 定成 2000 左與 1000 右的正式切換模式
- 日期時間：2026-04-24 14:35:06
- 問題原因：第四顆舵機 CH4 已完成量測，實機確認 2000 us 為最左、1000 us 為最右，表示先前的步進量測模式已完成任務；若繼續保留短按步進與長按反轉方向的探測行為，會讓正式操作時的控制語意不夠直接，也不利於後續和其他已定型舵機保持一致。
- 處理方式：在 `app_main.c` 移除 CH4 的 range-find 常數、方向旗標與步進 helper，改成正式模式常數 `left=2000 us`、`right=1000 us`、`center=1500 us`；按鍵短按改為在 left/right 間切換，長按改為回到 center 安全位置，同步更新 heartbeat 與 UART 啟動 log 文字，讓 CH4 明確顯示目前是 left、right 或 center；之後重新建置 `Appli` Debug，確認正式模式可正常編譯連結，且沒有新增其他 warning，僅保留既有 linker RWX warning。

### 050. 將第四顆舵機命名由 axis4 改成 left_right 語意
- 日期時間：2026-04-24 14:40:37
- 問題原因：第四顆舵機雖然已經定成正式左右切換模式，但程式內仍殘留 generic 的 `axis4` 命名，和前面已經語意化的 `clamp`、`fore_aft`、`gripper_lift` 風格不一致，閱讀 heartbeat、UART log 與 helper 名稱時也無法直接反映這顆軸的用途。
- 處理方式：在 `app_main.c` 將第四顆舵機的常數、狀態列舉、靜態變數、helper 函式與 log 字串從 `axis4` 全數改名為 `left_right`，保留既有的正式行為不變，也就是短按在 left/right 間切換、長按回 center；之後重新建置 `Appli` Debug，確認語意 rename 後仍可正常編譯連結，且沒有新增其他 warning，僅保留既有 linker RWX warning。

### 051. 補強 VL53L0X single-shot 失敗點診斷
- 日期時間：2026-04-24 15:07:09
- 問題原因：目前 TOF telemetry 只會印出泛化的 `[vl53l0x] single-shot read failed`，無法區分失敗是發生在 single-shot prepare、啟動量測、輪詢 `SYSRANGE_START` 清除、等待 interrupt ready 超時，還是最後 readout/clear interrupt，因此當現場看到 TOF 失敗時無法快速判斷是初始化序列卡住、I2C bus error，還是感測器沒有完成量測。
- 處理方式：在 `app_main.c` 的 `Phase1_Vl53l0xReadMeasurement()` 補上分段診斷 log，將失敗拆成 `prepare single-shot failed`、`start single-shot failed`、`poll start failed`、`start timeout`、`poll interrupt failed`、`interrupt timeout`、`readout failed`，並同步輸出 `hi2c1.ErrorCode` 或 timeout 時的暫存器值；原本外層重複的 generic failure log 移除，讓下一次現場重刷後可直接由 UART 看出卡在哪個步驟；之後重新建置 `Appli` Debug，確認診斷補強後仍可正常編譯連結，且沒有新增其他 warning，僅保留既有 linker RWX warning。

### 052. 將按鍵改成觸發 4 軸全範圍展示模式
- 日期時間：2026-04-24 15:18:07
- 問題原因：目前按鍵仍只控制單一軸的局部動作，不符合「按一下就讓 4 軸全部跑一次完整使用範圍」的展示需求；若沿用現有單軸模式，現場觀察時必須多次按鍵才能逐顆確認行程，無法快速看到整體機構動作。
- 處理方式：在 `app_main.c` 將按鍵 release 後的動作改成以單次短按觸發 4 軸展示序列，依序執行 clamp close/open、fore_aft aft/fore/center、gripper_lift down/up、left_right right/left/center，並在每一步輸出對應 UART log；同時補上 CH2 的正式端點常數 `900/1800 us`、將原先只供按鍵單軸操作使用的 left_right helper 收斂為定點狀態移動 helper，更新開機提示文字為 button demo 模式；最後重新建置 `Appli` Debug，確認展示模式可正常編譯連結，且沒有新增其他 warning，僅保留既有 linker RWX warning。

### 053. 將 left_right 軸移到展示序列最前面
- 日期時間：2026-04-24 15:26:06
- 問題原因：雖然 4 軸展示序列已經把 left_right 軸納入 right/left/center 動作，但原本把 CH4 排在 clamp、fore_aft、gripper_lift 之後，現場要等前面幾個步驟都跑完才會看到左右動作，容易被誤判成「左右沒有跑」。
- 處理方式：在 `app_main.c` 將 `App_RunServoFullRangeDemo()` 的順序改成 `left_right right/left/center` 先跑，再接 clamp、fore_aft、gripper_lift 的展示步驟，並把序列起始 log 改成 `left_right-first`，讓 CH4 一按下去就最先動作；之後重新建置 `Appli` Debug，確認展示順序調整後仍可正常編譯連結，且沒有新增其他 warning，僅保留既有 linker RWX warning。

### 054. 建立機械手臂空間座標、雅各比矩陣與逆運動學模組
- 日期時間：2026-04-24 15:38:53
- 問題原因：目前應用層已經能直接用 PWM 驅動 4 顆舵機，但還缺少一個與 CubeMX 生成碼解耦的運動學基礎，無法用一致的空間座標去描述 left_right、fore_aft、gripper_lift 三軸的末端位置，也缺少後續做路徑規劃、末端定位與速度映射所需的雅各比矩陣與逆運動學接口。
- 處理方式：新增 `robot_arm_kinematics.h/.c`，建立以 base 為原點、`+X` 前方、`+Y` 左方、`+Z` 上方的 3D 座標定義，並把 `left_right` 映射成 base yaw、`fore_aft` 映射成 shoulder pitch、`gripper_lift` 映射成 distal pitch；模組內提供正運動學、3x3 位置雅各比矩陣、解析式位置逆運動學，以及帶 damping 的 differential IK 解算；另外在 `app_main.c` 啟動時加入一組 sample geometry 的自檢 log 來驗證 forward/jacobian/IK 路徑已接通，並重新建置 `Appli` Debug 確認可正常編譯連結，僅保留既有 linker RWX warning。

### 055. 將實測不撞範圍升級為工作空間估算硬限制
- 日期時間：2026-04-24 16:05:00
- 問題原因：使用者明確要求先前實測得到的舵機範圍就是不會撞到的安全區，因此後續所有工作空間與抓取區估算都不能再超出這些範圍；若只把它當成口頭假設，而沒有在程式裡變成 joint limit，之後不論是 IK 估算、工作空間取樣或示教點選取，都可能再次把邊界往外推，違反現場已驗證的安全前提。
- 處理方式：在 `robot_arm_kinematics` 補上 joint limit 與 workspace bounds 結構，新增僅在限制內取樣的工作空間包絡估算函式；同時在 `app_main.c` 將 `left_right=1000..2000 us`、`fore_aft=900..1800 us`、`gripper_lift=1000..1500 us` 對應成保守角度窗，啟動時直接印出「絕不超過實測不撞 PWM 視窗」與保守 safe workspace，讓後續空間估算都鎖在已實測安全區內；之後重新建置 `Appli` Debug 驗證可正常編譯連結，僅保留既有 linker RWX warning。

### 056. 將實測安全範圍接入舵機實際控制路徑
- 日期時間：2026-04-24 20:16:47
- 問題原因：前一版雖然已把實測不撞範圍放進工作空間與運動學估算，但實際寫入 PWM compare 的控制路徑仍然沒有硬夾限；這代表只要後續新增 IK 到 PWM 映射、手動命令或其他 demo 路徑時給出超界脈寬，timer 仍可能直接輸出超過現場安全窗的值。
- 處理方式：在 `app_main.c` 的 4 個 `ApplyPulse` helper 前加上共用的 measured-safe-window 夾限函式，將 clamp 鎖在 `700..1600 us`、fore_aft 鎖在 `900..1800 us`、gripper_lift 鎖在 `1000..1500 us`、left_right 鎖在 `1000..2000 us`；若有任何請求值超界，會先輸出 `[servo_safety]` log 再把脈寬夾回實測安全窗，並在啟動 log 明示控制路徑已啟用硬限制；之後重新建置 `Appli` Debug 驗證可正常編譯連結，僅保留既有 linker RWX warning。

### 057. 建立 UART XYZ/PWM 控制鏈與桌面 GUI debug 工具
- 日期時間：2026-04-24 20:36:40
- 問題原因：目前雖然已經有運動學模組與實測安全窗夾限，但還缺少一條可直接把 `xyz` 目標送進 MCU 並轉成 PWM 的控制鏈，也沒有一個方便在不改韌體的情況下做現場 debug 的桌面工具；若沒有這層命令介面與 GUI，後續每次測試都只能靠重新編譯或手動改常數，效率很差，也不利於快速驗證 IK、PWM 夾限與安全工作區。
- 處理方式：在 `app_main.c` 新增 UART 命令處理，支援 `HELP`、`STATUS`、`HOME`、`XYZ x y z` 與 `PWM clamp fore gripper left`，其中 `XYZ` 會先走安全 IK 解算，再映射成 3 軸 PWM 並套用既有實測安全夾限；另外新增 `tools/robot_arm_debug_gui.py`，使用 Tkinter + pyserial 提供 serial 連線、XYZ 送點、手動 PWM slider 與 log 視窗，並補上 `tools/requirements.txt` 與安裝 `pyserial`；最後重新建置 `Appli` Debug，並以 `py_compile` 驗證 GUI 腳本語法，僅保留既有 linker RWX warning。

### 058. 為 GUI 加入對應動作的機械手臂動態視覺預覽
- 日期時間：2026-04-24 20:46:26
- 問題原因：原本的桌面 GUI 只有序列連線、XYZ 欄位與 PWM slider，雖然功能上可用，但缺少一個能直觀看出目前姿態與目標位置的視覺化區塊；使用者明確要求 GUI 要有吸引人的機械手臂圖，且要能對應實際動作，因此不能只停留在純表單和數字控制。
- 處理方式：重做 `tools/robot_arm_debug_gui.py` 的版面，在右側新增動態 Canvas 預覽，使用目前 PWM slider 對應的安全角度窗與簡化幾何模型畫出機械手臂側視圖、夾爪開合、上視 yaw/reach 雷達與 XYZ 目標 marker，並讓 `Send XYZ` 先做本地安全 IK 預測後再更新預覽，`HOME` 與手動 slider 也會同步刷新圖面；最後以 `py_compile` 驗證 GUI 腳本語法正確，沒有新增其他工具錯誤。

### 059. 為 GUI 加入右側與下側捲軸避免小視窗截斷
- 日期時間：2026-04-24 20:51:32
- 問題原因：目前 GUI 版面在加入三欄控制區與大型動態預覽後，若視窗寬高不足，右側預覽或下方控制區會被截掉，使用者無法靠縮放視窗以外的方式看到完整內容，因此需要提供標準捲動機制來保住操作性。
- 處理方式：將 `tools/robot_arm_debug_gui.py` 的最外層版面改成以 Canvas 包住整個內容區，並在主視窗右側新增垂直捲軸、下側新增水平捲軸，透過 `scrollregion` 與內容視窗尺寸同步機制讓介面在視窗太小時可以完整左右與上下捲動；完成後再用 `.venv` 的 Python 執行 `py_compile` 驗證語法，確認沒有引入新的 Tkinter 結構錯誤。

### 060. 修正 GUI 最小尺寸與預覽雷達面板的裁切
- 日期時間：2026-04-24 20:55:46
- 問題原因：實際執行後發現前一版雖然已加上捲軸，但主視窗仍被 `1320x820` 的過大最小尺寸限制住，小螢幕上會直接被作業系統裁掉；另外右側 Motion Preview 的雷達面板圓心與半徑配置超出畫布寬度，導致圖形和說明文字即使在正常視窗下也會被切掉。
- 處理方式：將 `tools/robot_arm_debug_gui.py` 的初始尺寸調整為較保守的 `1280x820`，並把最小尺寸下修到 `900x620`，讓捲軸在小視窗下能真正發揮作用；同時重算預覽 Canvas 內側視圖與雷達面板的區塊寬度、半徑和文字寬度，讓右側雷達圓、狀態說明與圖例都落在畫布邊界內；最後用 `.venv` 的 Python 執行 `py_compile` 驗證語法正確。

### 061. 將預覽圖例移到下方並收斂預覽字級與間距
- 日期時間：2026-04-24 21:14:05
- 問題原因：雖然前一版已修正裁切，但右側雷達面板內仍承載圖例說明，視覺上偏擠，且預覽框下方的狀態文字在小視窗下佔用較多垂直空間，不夠俐落。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 新增獨立的預覽圖例文字列，將原本畫在 Canvas 右側雷達面板內的圖例移到預覽框下方；同時把預覽標題、狀態摘要與雷達區文字的字級略微下修，並收緊預覽 Canvas 與下方文字的間距，讓右側雷達面板更乾淨，小視窗下也更容易閱讀；最後用 `.venv` 的 Python 執行 `py_compile` 驗證語法正確。

### 062. 依視窗寬度自動重排控制區避免右側預覽被壓扁
- 日期時間：2026-04-24 21:21:04
- 問題原因：實際畫面顯示在中等寬度視窗下，雖然整體未再直接裁切，但三個主面板仍被固定塞在同一列，導致右側 Motion Preview 被嚴重壓縮，只剩下一小條可視寬度，等同版面不可用。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 將 `XYZ Target`、`Manual PWM Debug` 與 `Motion Preview` 三塊面板改成由 `command_frame` 寬度驅動的響應式排版，寬視窗維持三欄，中等寬度改成上方兩欄控制加下方整排預覽，較窄時再退成單欄直排；另外用 `.venv` 的 Python 執行 `py_compile` 做語法檢查，並透過 Tk 實例化驗證在約 `1110x760` 寬度下會切到 `medium` 版型，確認預覽區會落到下一列並佔滿兩欄。

### 063. 將 4 軸舵機控制改成多軸同步 minimum-jerk 軌跡引擎
- 日期時間：2026-04-24 21:58:50
- 問題原因：目前機械手臂雖然已具備 Cartesian 座標、逆運動學與安全夾限，但 `HOME`、`XYZ`、`PWM` 與按鍵展示流程仍是直接把目標脈寬一次寫到舵機，造成運動只有「到得了」但沒有「走得順」；這種一步跳到目標的命令型態會讓 RC 舵機在多軸聯動時看起來生硬，無法接近較高階伺服的平順體感。
- 處理方式：在 `Appli/Core/Src/app_main.c` 新增 4 軸同步的 minimum-jerk 軌跡狀態機，根據各軸脈寬差與各自速度上限計算共同移動時間，並在主循環中以固定節奏更新插值後的脈寬，讓 clamp、fore_aft、gripper_lift、left_right 共享同一條平滑時間曲線；同時把 `HOME`、`PWM`、`XYZ` 命令與按鍵 demo 全部改走這條軌跡引擎，保留既有 IK、工作空間與安全脈寬夾限，但消除直接跳 pulse 的旁路；另外補上運動中 `STATUS` 的目標脈寬與剩餘時間輸出，最後對 `Appli/build/Debug` 做 `cmake --build --clean-first --verbose` 乾淨重建，編譯成功，僅保留既有 linker RWX warning。

### 064. 將 XYZ 升級成 Cartesian minimum-jerk 路徑規劃
- 日期時間：2026-04-24 22:13:52
- 問題原因：前一版雖然已把 4 軸舵機切成 joint-space minimum-jerk，但 `XYZ` 指令本質上仍只是先用 IK 解出終點關節，再在關節空間平滑走到終點；這樣每個關節看起來會比較順，但末端在空間中的路徑不保證是直觀的 Cartesian 路徑，和使用者要求的「XYZ 路徑本身也要更好」仍有一層差距。
- 處理方式：在 `Appli/Core/Src/app_main.c` 另加一層 `XYZ` 專用的 Cartesian trajectory planner，先用目前估測 pose 與目標 XYZ 生成 minimum-jerk 的空間路徑，再於每個更新週期對中間 waypoint 重新做 IK 並轉成舵機 pulse 輸出；同時保留原本的 joint-space planner 當 fallback，若某個 Cartesian waypoint 因中途不可達或關節限制而無法安全求解，就自動退回 final target 的 joint-space minimum-jerk，不讓整條 motion 卡死；最後重新 build `Appli`，編譯成功，僅保留既有 linker RWX warning。

### 065. 讓 GUI 接上 firmware 軌跡狀態並自動輪詢 STATUS
- 日期時間：2026-04-24 22:13:52
- 問題原因：原本 `tools/robot_arm_debug_gui.py` 只有本地 slider/IK preview，收到韌體回傳時只是把文字追加到 serial log，無法把板上的實際 motion、remaining time 與 Cartesian target 顯示在 GUI 上，因此雖然控制已經平滑，桌面工具卻看不到 planner 是否正在工作。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 新增 firmware motion/pose/target 三組狀態欄位，解析 `[status]` 與 `[cmd_xyz]` 回傳行，把 `motion=... remaining=...`、估測 pose 與 Cartesian target 直接顯示在預覽區；另外加入不寫入 log 的背景 `STATUS` 輪詢，讓連線後 GUI 可以持續刷新剩餘時間而不必手動一直按 `STATUS`，並用 workspace `.venv` 驗證此 GUI 檔案無 syntax error、且模組可成功 import。

### 066. 在 GUI 標示 XYZ 的 conservative 可輸入範圍
- 日期時間：2026-04-25 07:46:52
- 問題原因：GUI 的 XYZ 輸入區原本只有三個數值欄位，沒有提示目前這支手臂在安全角度窗下對應的大致 X、Y、Z 可設定範圍，使用者每次測試都得靠記憶或反覆試錯，容易送出明顯超界的點位。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 內用與 GUI 本地預覽一致的幾何模型、角度窗與 `z >= 0 mm` 條件做 conservative workspace sampling，算出目前引導範圍 `x=97..240 mm`、`y=-208..208 mm`、`z=0..201 mm`，並把三軸 guide range 直接標在 XYZ 輸入框下方；另外加上一句說明，明確註記這只是由安全關節限位估出的保守包圍範圍，最終是否可達仍由 firmware 的 IK、路徑與安全限制決定；完成後用 workspace `.venv` 驗證 GUI 檔案無 syntax error 且可正常 import。

### 067. 讓超界 XYZ 欄位變紅並在送出前硬擋
- 日期時間：2026-04-25 07:51:28
- 問題原因：前一版雖然已在 GUI 標出 XYZ 的 guide range，但如果使用者輸入超界值，畫面仍沒有足夠直接的警示，而且按下 `Send XYZ` 仍會把命令送往 firmware，導致介面提示和實際操作之間存在落差。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 將 XYZ 三個輸入欄改成可直接控制底色的 entry widget，新增本地 guide range 驗證邏輯，讓非整數或超出 conservative guide range 的欄位立即轉成紅底；同時在 `send_xyz_command` 內加入同一套 guide range 的送出前預檢，若任一軸超界就以 message box 直接阻擋送出，並明確指出是哪些軸超出範圍；最後用 workspace `.venv` 驗證 GUI 檔案無 syntax error、可正常 import，且超界訊息可正確產生，例如 `X must stay within 97..240 mm, got 80 mm`。

### 068. 修正 GUI 顯示 Send 被擋下的即時原因
- 日期時間：2026-04-25 07:57:00
- 問題原因：使用者回報「按 send 沒有動」，說明前一版雖然已加上超界預檢，但實際操作時仍不夠直觀，因為 GUI 沒有把目前是否可送出、以及是因為哪一軸超界而被擋下，持續顯示在 XYZ 區塊內，容易讓人誤以為按鈕失效或 firmware 沒反應。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 補上 `xyz_validation_var` 與對應狀態列，讓 XYZ 區塊會直接顯示 `Ready` 或 `Blocked: ...`；同時把 `Send XYZ` 按鈕和這套本地驗證狀態綁在一起，輸入非法時會自動 disabled，並在使用者真的觸發 send 路徑時額外把阻擋原因寫入 host log 和 message box；最後重新驗證 GUI 檔案無 syntax error，且 helper 測試可正確產生像 `X guide is 97..240 mm, got 80 mm` 這種即時阻擋訊息。

### 069. 在 GUI 顯示 PWM/HOME/XYZ 的 FW 命令回覆與 echo
- 日期時間：2026-04-25 08:03:49
- 問題原因：使用者回報按 `Send PWM` 時看起來「沒有反應」，而 GUI 原本雖然有 serial log 與 firmware status，但沒有把 `FW 是否接受命令`、`FW 實際採用的目標值` 和 `為什麼看起來沒動` 集中顯示在操作區附近，因此就算 FW 已回 `[cmd_pwm]`，也不容易立刻看出來。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 的 XYZ 面板下方新增 `FW Command Ack` 區塊，當主機送出 `PWM`、`HOME`、`XYZ` 時先顯示 `waiting for acknowledgement`；收到 firmware 的 `[cmd_pwm]`、`[cmd_home]`、`[cmd_xyz]` 回應後，立即更新成 `success` 或 `error`，並把 FW echo 的 target 值與 duration 顯示出來；其中 `PWM` 若回 `duration=0 ms`，GUI 會明確顯示 `no movement needed (already at target)`，用來解釋「按了但機構沒動」其實是因為目前值和目標值相同；最後用 workspace `.venv` 驗證 GUI 檔案無 syntax error，且 parser 測試可正確把 `[cmd_pwm] ... duration=0 ms` 轉成對應的成功與 echo 顯示。

### 070. 將 UART 命令接收改成 IRQ ring buffer 以修正多次設定後失聯
- 日期時間：2026-04-25 08:12:32
- 問題原因：使用者回報「設定 3 次後就沒反應」，根因不是 GUI 限制次數，而是 firmware 原本在 `Appli/Core/Src/app_main.c` 用主迴圈 polling 的 `HAL_UART_Receive(..., timeout=0)` 收命令，同時又用 blocking `printf` 持續輸出 `[cmd_*]`、`[status]`、`[tick]` 等 log；當 GUI 背景每 500 ms 自動送 `STATUS`，再疊加使用者連續送 `PWM/XYZ`，新命令可能在 firmware 忙著吐 log 的時間窗內到達，造成 UART RX 漏收或 overrun，表面上就會變成「前幾次正常，後面突然像卡住」。
- 處理方式：在 `Appli/Core/Src/app_main.c` 把 LPUART1 接收改成 1-byte interrupt，IRQ 只負責把收到的 byte 放進 256-byte ring buffer，主迴圈再從 ring buffer 取出資料沿用原本的命令 parser，因此不會在中斷內直接做 `printf` 或運動控制；另外在 `Appli/Core/Src/stm32n6xx_it.c`、`Appli/Core/Inc/stm32n6xx_it.h`、`Appli/Core/Inc/app_main.h` 補上 `LPUART1_IRQHandler` 轉接，並在 UART MSP init/deinit 內啟用與關閉 NVIC；最後強制 clean rebuild `STM32N6_Appli.elf`，編譯與連結通過，僅保留既有的 RWX segment linker warning。

### 071. 將 GUI 起始手臂圖改成檯燈式展示姿態
- 日期時間：2026-04-25 11:43:00
- 問題原因：使用者回報 GUI 手臂圖一開啟就呈現完全水平直臂，但實際想要的起始視覺應該更像桌上檯燈那種有折角的展示姿態；原本 `tools/robot_arm_debug_gui.py` 直接拿 firmware-safe startup PWM `700/1500/1500/1500` 做 local preview，而這組在目前安全模型裡剛好會映成 `yaw=0 deg, shoulder=0 deg, gripper=0 deg`，因此畫面必然是水平。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 增加一組僅供 GUI 啟動畫面使用的展示關節角 `yaw=0 deg, shoulder=15 deg, gripper=-30 deg`，讓預覽一開始呈現較自然的檯燈式折臂；同時保留實際 PWM slider 的 firmware-safe startup 值不變，並在使用者一旦調整 PWM/XYZ、送出 PWM、送出 XYZ、或按 HOME/Load Startup 時立即切回 live preview，避免 GUI 展示姿態干擾真實控制；最後用 workspace `.venv` 驗證 GUI 檔案無 syntax error，且 helper 測試確認新起始展示姿態不再水平，座標為 `x=231.82 mm, z=47.06 mm`。

### 072. 將 GUI 預設值改成區間中點並放大預覽下壓範圍
- 日期時間：2026-04-25 11:49:52
- 問題原因：使用者要求 GUI 預設值應直接落在各自可用範圍的中間值，而不是沿用 startup 或手填值；同時目前 side view 的固定繪圖範圍 `x=-40..255 mm`、`z=-10..215 mm` 太淺，當手臂採用中點姿態或往下壓時，末端很容易超出畫面底部，看起來不像正常的檯燈式折臂預覽。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 將 PWM 預設改成各自區間中點 `1150/1350/1250/1500 us`，並把 XYZ 預設改成 conservative guide range 的中點 `169/0/101 mm`；同時取消舊的 startup-only demo pose，讓預覽從啟動開始就直接跟隨這組中點預設；另外新增 preview bounds 估算邏輯，依 shoulder/gripper 全安全角度範圍取樣 side view 需要的 `x/z` 邊界，再加上 padding 後作為畫布縮放範圍，使手臂下壓時仍完整留在圖內；最後用 workspace `.venv` 驗證 GUI 檔案無 syntax error，且 helper 測試確認新的預設值與預覽範圍為 `XYZ 169/0/101`、`PWM 1150/1350/1250/1500`、`preview z=-200.34..221.07 mm`。

### 073. 對齊 HOME 與 Load Midpoint 的中點姿態
- 日期時間：2026-04-25 11:59:29
- 問題原因：雖然 GUI 啟動後的 slider 預設值已改成各軸中點，但 `tools/robot_arm_debug_gui.py` 的 Load Startup 仍硬寫成舊的 `700/1500/1500/1500`，而 `Appli/Core/Src/app_main.c` 的 HOME 與開機起始姿態也仍沿用同一組 safe startup pulse；因此一按 Load Startup 或 HOME，手臂就會從目前的中點折臂預覽突然跳回接近水平的舊姿態，讓使用者感覺「一按就突變」，也放大了往下操作時的破圖感。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 將 Load Startup 改成載入中點 PWM `1150/1350/1250/1500 us`、按鈕文案改成 `Load Midpoint`、HOME 預覽提示改成 midpoint pose，並讓 HOME ack parser 同時接受舊的 `safe startup pose` 與新的 `default midpoint pose`；在 `Appli/Core/Src/app_main.c` 新增 4 軸 default midpoint pulse 常數，將開機初始化與 HOME 命令都改成同一組中點脈寬，並更新對應 log 文案；最後用 workspace `.venv` 驗證 GUI 檔案無 syntax error、helper 測試確認中點姿態角度為 `yaw=0 / shoulder=-18 / gripper=-30 deg` 不再水平，且強制 clean rebuild `STM32N6_Appli.elf` 通過，僅保留既有的 RWX segment linker warning。

### 074. 恢復 GUI 啟動時的檯燈式展示姿態
- 日期時間：2026-04-25 12:05:01
- 問題原因：前一版把 GUI 啟動預覽完全綁到中點 PWM `1150/1350/1250/1500 us`，但這組脈寬在目前角度映射下會得到 `yaw=0 / shoulder=-18 / gripper=-30 deg`，導致起始畫面末端高度落到 `z=-77.11 mm`，看起來像手臂一開啟就插到桌面下方；這和使用者要的「預設圖像像檯燈」相反，問題出在啟動展示邏輯，而不是 HOME/Load Midpoint 的實際控制路徑。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 恢復 startup-only 的 `_initial_preview_joint_angles`，新增固定展示角 `yaw=0 / shoulder=15 / gripper=-30 deg` 只供 GUI 啟動那一刻使用，並更新預覽說明文字，明確標示「起始畫面是展示姿態，但 PWM 預設仍是中點」；原本的 variable trace 仍會在使用者一調整 PWM 或 XYZ 時自動切回 live preview，因此不會影響後續實際控制；最後用 workspace `.venv` 驗證 GUI 檔案無 syntax error，helper 測試確認起始展示姿態座標為 `x=231.82 mm, z=47.06 mm`，而實際預設 PWM 仍維持 `1150/1350/1250/1500`。

### 075. 將 GUI 平台移到最低手臂點下方
- 日期時間：2026-04-25 12:11:04
- 問題原因：目前 side view 把 `z=0` 直接畫成平台頂面，所以當使用者把手臂往下壓時，即使圖像沒有超出畫布，最低的夾爪點仍會視覺上穿過平台；使用者截圖中的 `clamp=1150 fore_aft=1350 gripper_lift=1000 left_right=1500` 就屬於這種情況，問題在視覺基準線位置，而不是預覽縮放或 IK 本身。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 的 side view 繪圖邏輯中，把原本的平台頂面從 `z=0` 改成預覽範圍最底部附近的固定平台，另外將 `z=0` 改成虛線參考線並標示 `Z=0`；同時補上一個較長的底座與支撐柱，讓肩關節看起來站在平台上而不是懸空；最後用 workspace `.venv` 驗證 GUI 檔案無 syntax error，並用 helper 測試確認使用者截圖那組姿態下 `platform_top_y=311.44`、`lowest_jaw_y=269.29`，最低夾爪點已位於平台上方。

### 076. 將 GUI 預覽改成工業臂外觀並標示關節語意
- 日期時間：2026-04-25 12:20:00
- 問題原因：使用者回報目前 GUI 的 side view 仍太像檯燈，且 `fore_aft` 與 `gripper_lift` 對應到哪個關節在圖上不夠清楚；雖然 firmware 與 kinematics 仍維持 `fore_aft -> shoulder/front-back`、`gripper_lift -> distal/up-down` 的控制定義，但 preview 沒有把這個語意直觀表達出來，導致使用者看圖時會覺得對應錯位。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 將 PWM slider 文案改成 `Fore (Front/Back)` 與 `Gripper (Up/Down)`，並把 preview legend 與 pose summary 改成直接顯示 `fore`、`gripper_lift`；同時重畫 side view 手臂本體，將原本的圓角粗線段改成帶厚度的硬邊連桿、多層金屬外殼、高亮面與腕部工具座，讓外觀更接近工業機械臂；另外新增兩個關節 callout，直接在 shoulder housing 標示 `Fore/Aft Front/back joint`、在 elbow housing 標示 `Gripper Lift Up/down joint`；最後用 workspace `.venv` 驗證 GUI 檔案無 syntax error，並實際建立 `RobotArmDebugGui()` 驗證 preview 可正常初始化，啟動時 legend 與 pose summary 已反映新的關節語意。

### 077. 將 GUI 底座改成較低的 pedestal 與斜撐
- 日期時間：2026-04-25 12:33:52
- 問題原因：使用者指出目前 GUI 的底座仍然太高，和手繪示意不符；根因是 side view 仍保留一根從平台一路撐到 shoulder housing 下方的高立柱，視覺上像高塔，而不是較低的工業機座。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 的 side view 底座繪圖中，將原本直達 shoulder 的高柱改成較低的 pedestal，並新增一支短斜撐連到 shoulder mount；同時在 pedestal 上方加一個低矮設備座，讓底座更像低機座而不是高支柱；最後用 workspace `.venv` 驗證 GUI 檔案無 syntax error，並以 helper 計算確認新 pedestal 高度約 `78.24 px`，明顯低於舊高柱約 `146.24 px`。

### 078. 將 GUI 中使用者圈選的區段再縮短
- 日期時間：2026-04-25 12:46:35
- 問題原因：使用者回報截圖中自己圈選的兩個區域都還偏長，包含底座 pedestal/斜撐，以及前端 forearm 接 wrist/tool 的外殼，造成 side view 仍比實體看起來更修長。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 的 side view 繪圖中，將 pedestal 再往下壓短、縮小上方設備座與斜撐長度；同時把前臂外殼改成在 wrist 前先收成較短的 forearm shell，再補一段較短的 wrist knuckle，並把 tool mount 與夾爪前伸長度一起縮短；最後針對 GUI 檔案執行錯誤檢查，確認本次修改沒有引入新的診斷錯誤。

### 079. 將 GUI 黃橘色上臂改成反 2 反摺造型
- 日期時間：2026-04-25 13:03:55
- 問題原因：使用者指出目前黃橘色 shoulder-to-elbow 上臂仍是一根直桿，和實際想要的反摺工業臂外觀不符；根因是 side view 直接把上臂畫成 shoulder 到 elbow 的單一直線厚板，缺少中段反折輪廓。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 的 side view 繪圖中，將上臂改成由 shoulder 到中繼折點、再由折點到 elbow 的兩段式 dog-leg 外殼，並補上對應 highlight 與小型折點外蓋，讓黃橘色那段視覺上更像反 2；完成後先檢查 GUI 檔案診斷結果，再用 workspace `.venv` 實際建立 `RobotArmDebugGui()`，確認初始化仍成功。

### 080. 依參考照片重構 GUI side view 構圖
- 日期時間：2026-04-25 13:08:35
- 問題原因：使用者指出目前 side view 經過多次局部微調後反而越改越不像，根因是整體構圖仍沿用深色背景下的厚重外殼拼接思路，和參考照片中「淺灰底、圓形關節、細連桿、兩叉夾爪」的輪廓語言差太多。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 的 side view 區塊中，移除原本的格線、金屬外殼、pedestal/brace 工業渲染與多段厚板，改成參考照片風格的重新構圖：以淺灰底板搭配深色描邊、下方梯形立柱與底座、肩/肘/腕三個圓形關節、較細的上臂與前臂連桿、以及兩段折線式雙叉夾爪；同時保留原本 target marker、關節 callout 與 kinematics 映射，讓 preview 仍會隨姿態變動；最後檢查 GUI 檔案無新錯誤，並用 workspace `.venv` 實際建立 `RobotArmDebugGui()` 驗證初始化成功。

### 081. 壓低 GUI 底座並將肩段改成先垂直再水平
- 日期時間：2026-04-25 13:13:59
- 問題原因：使用者指出目前底座仍然太高、視覺上很醜，並要求接近底座的大關節那段手臂改成先垂直、再水平的 90 度結構；根因是重構後的 side view 仍使用偏高的 pedestal，高度視覺集中在底座本體，同時 shoulder 到 elbow 的上臂第一段還是直接往前斜出，沒有先拉出垂直再轉水平的輪廓。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 的 side view 繪圖中，縮短 base plate 與 pedestal 的高度，讓底座整體更低；另外新增一段由 pedestal 接到 shoulder 關節下方的窄直立 stem，並把上臂上緣 polyline 改成先垂直上升、再水平前伸、最後接到 elbow 的折線，讓肩段更接近使用者指定的 90 度垂直/水平外觀；完成後用 workspace `.venv` 實際建立 `RobotArmDebugGui()`，確認 GUI 初始化仍成功。

### 082. 將黃色直桿縮到最短並移除橘色上段
- 日期時間：2026-04-25 13:18:17
- 問題原因：使用者指出截圖中的黃色直立桿仍然過長，而橘色圈出的肩段上緣折線根本不需要；根因是 side view 仍保留一段由 pedestal 往上的長 stem，以及一條額外的 shoulder 上緣 polyline，讓肩部視覺過於複雜。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 的 side view 繪圖中，將 shoulder 下方直立 stem 的頂點改成幾乎貼近 pedestal，只留下最短的短 neck，並把橘色圈到的上緣 polyline 整段移除；同時保留其餘關節、下緣連桿與夾爪構圖，最後用 workspace `.venv` 實際建立 `RobotArmDebugGui()`，確認 GUI 初始化仍成功。

### 083. 修正第一個關節必須與底座實際連接
- 日期時間：2026-04-25 13:21:52
- 問題原因：使用者指出目前第一個大關節和底座之間仍有斷開，違反機構邏輯；根因是前一次把 shoulder 下方的直立 stem 縮得太短，支撐只停在關節下方，沒有真正接入大關節本體。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 的 side view 繪圖中，新增 `shoulder_joint_outer_radius` 作為第一關節外徑基準，並把直立支撐從 pedestal 頂部一路拉到大關節底部內側，同時將支撐的 X 位置對正關節中心，讓第一個關節和底座在圖形上確實連成一體；完成後用 workspace `.venv` 實際建立 `RobotArmDebugGui()`，確認 GUI 初始化仍成功。

### 084. 將第一個關節改成直接坐在底座上
- 日期時間：2026-04-25 13:24:08
- 問題原因：使用者明確要求第一個大關節要直接連接在底座上，不要再有一根中間支撐桿；根因是前一版雖然補回了支撐邏輯，但仍保留一段從 pedestal 到關節底部的直立 stem，不符合使用者要的直接貼合結構。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 的 side view 繪圖中，移除 shoulder 下方的直立 stem，改以新的 `shoulder_draw_y` 把第一個大關節的圓心直接定位在 pedestal 頂面上方，讓關節外圈直接坐落在底座上；同時把 shoulder-to-elbow 的連桿起點改跟著新的關節位置繪製，並保留其餘關節與夾爪構圖；最後用 workspace `.venv` 實際建立 `RobotArmDebugGui()`，確認 GUI 初始化仍成功。

### 085. 修正 fore 關節只應轉角且擴展置中 side view
- 日期時間：2026-04-25 13:32:23
- 問題原因：使用者指出 `fore` 關節在 GUI 裡看起來像是連手臂長度都在變，不符合邏輯；根因是 side view 之前對 X 與 Z 使用不同縮放比例，導致同一根固定長度的連桿在不同角度下螢幕像素長度看起來會改變；另外 side view 區塊本身偏窄，手臂內容也沒有水平置中。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 中把 side view 的 `side_x` / `side_z` 改成共用同一個 `plot_scale`，讓 fore 連桿只呈現角度旋轉而不再視覺變長；同時把 side view 區域左右加寬，並用 `centered_plot_left` 將手臂內容水平置中；另外把第一關節的繪圖中心重新對齊到真實的 `shoulder_y`，讓 shoulder-to-elbow 連桿起點回到正確的運動中心；最後用 workspace `.venv` 驗證 GUI 可初始化，並以三個肩角 `-72 / -18 / 36 deg` 檢查 fore 螢幕長度，結果固定為 `17.206 / 17.206 / 17.206`。

### 086. 修正 side view bounds 導致只剩關節的壓縮問題
- 日期時間：2026-04-25 13:32:23
- 問題原因：使用者回報 side view 雖然要求置中，但機械手臂幾乎消失，只剩關節；根因是 `tools/robot_arm_debug_gui.py` 的 `_estimate_preview_bounds()` 把水平 padding 寫在 shoulder 取樣迴圈裡，導致 `max_x` 每輪都被重複放大，最終 side view 範圍膨脹到 `min_x=-567.67`、`max_x=1199.16`，整支手臂被壓縮成幾乎看不見。
- 處理方式：將 `_estimate_preview_bounds()` 的水平 padding 移出取樣迴圈，只在完成整體邊界收集後套用一次，讓 side view 保持合理範圍；修正後 preview bounds 回到 `min_x=-87.67`、`max_x=285.00`，手臂輪廓可正常顯示，同時保留前一版的等比例縮放與水平置中；最後用 workspace `.venv` 驗證 GUI 可初始化，確認修正後沒有新錯誤。

### 087. 將 fore 角度範圍改為 30..90 並再壓低底座台
- 日期時間：2026-04-25 13:58:08
- 問題原因：使用者要求 `fore` 角度範圍改成 `30..90 deg`，且 side view 的底座台不要那麼高，只要高一點點；根因是 GUI 與 firmware 先前都仍沿用舊的 `fore` 安全角度範圍，而 side view 的 base plate 高度也還偏厚。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 中將 `FORE_AFT_MIN_DEG` / `FORE_AFT_MAX_DEG` 改為 `30.0 / 90.0`，並把 `STARTUP_PREVIEW_SHOULDER_DEG` 調整到新範圍內的 `60.0 deg`；同時將 side view 的 `base_plate_top_y` / `base_plate_bottom_y` 再壓低，讓底座台只保留較薄的一層；另外為避免 GUI 與實機控制不一致，在 `Appli/Core/Src/app_main.c` 也將 `APP_FORE_AFT_SAFE_MIN_DEG` / `APP_FORE_AFT_SAFE_MAX_DEG` 對齊成 `30.0f / 90.0f`；最後驗證 GUI 端 `fore` 最小/最大脈寬確實映射到 `30.0 / 90.0 deg`、GUI 可正常初始化，且 Appli build 重新執行後結果為 `ninja: no work to do`。

### 088. 讓 Motion Preview 內容依實際畫布寬度置中
- 日期時間：2026-04-25 14:10:46
- 問題原因：使用者回報畫面沒有置中；根因是 `tools/robot_arm_debug_gui.py` 的 `_draw_preview()` 一直用固定的 `PREVIEW_CANVAS_WIDTH` / `PREVIEW_CANVAS_HEIGHT` 當作繪圖尺寸，即使 Tkinter grid 已把 preview canvas 撐寬，side view 與 radar 仍固定從左上角開始畫，導致右側留下大片空白，視覺上整組 preview 偏左。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 中將 `_draw_preview()` 改為優先使用 `preview_canvas.winfo_width()` / `winfo_height()` 取得實際畫布尺寸，並以原本的 560x360 設計尺寸計算 `content_left` / `content_top` 偏移量，將 side view、radar 與背景裝飾整組水平置中；另外先用 Problems 檢查確認檔案沒有新錯誤，再用 workspace `.venv` 啟動 GUI 並執行 preview 重繪驗證，實際得到 `canvas_width=645`、`side_left=56.5`，確認內容不再卡在原本的固定左邊界 `14`。

### 089. 修正首次開啟 GUI 時 preview 尚未置中
- 日期時間：2026-04-25 14:15:06
- 問題原因：使用者回報調整尺寸後 preview 會置中，但剛執行的第一個畫面仍然沒有置中；根因是 `RobotArmDebugGui.__init__()` 在 UI 尚未完成 geometry layout 時就先呼叫 `_update_preview()`，當時 `preview_canvas.winfo_width()` 仍為 `1`，第一次繪圖只能退回固定預設寬度，之後就算 canvas 真正撐開，也沒有自動再重畫一次。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 為 `preview_canvas` 新增 `<Configure>` 事件處理，記錄最後一次 canvas 尺寸，只有在尺寸真的改變且大於 `1x1` 時才用 `after_idle(self._update_preview)` 重新繪製，讓首次 layout 完成後自動重畫成置中位置；最後用 workspace `.venv` 建立 GUI 實測，在不手動呼叫 `_update_preview()` 的情況下驗證初始繪圖仍是 `width=1 side_left=14.0`，但 layout 完成後已自動更新成 `width=1219 side_left=343.5`，確認首次開啟畫面也能置中。

### 090. 新增可點擊的 3D workspace 雲圖與點選移動
- 日期時間：2026-04-25 17:14:32
- 問題原因：使用者希望在 GUI 的手臂預覽旁新增 3D 空間圖，直接顯示有效可移動區域，並可用滑鼠點擊 3D 區域後讓手臂移動到該點；原本 `tools/robot_arm_debug_gui.py` 的 preview 只有 side view 與 2D yaw/reach radar，沒有 3D 工作區表示，也沒有任何 canvas 點擊到 XYZ 命令鏈的互動入口。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 中改以安全關節範圍取樣建立可達工作區點雲，並先過濾成「整數毫米後仍能被現有 `_solve_safe_ik()` 安全解出」的有效點，再將右側 preview 面板改畫成 3D workspace cloud，顯示 X/Y/Z 軸、目前姿態點、目標點與可點擊的 cyan reachable cloud；同時替 `preview_canvas` 加上滑鼠點擊事件，點擊時會在 3D 雲圖中選取最近的有效樣本點，回填 XYZ 欄位，若 serial 已連線則直接走既有 `send_xyz_command()` 命令鏈送出 `XYZ x y z`；最後用 workspace `.venv` 驗證 GUI 可正常建立、3D panel 共有 `1353` 個有效 click candidates，且在模擬 serial 已連線時點擊 3D 雲圖後會實際送出 `XYZ 183 0 171`。

### 091. 將 3D workspace 升級為 hover、旋轉與按住預覽放開送出
- 日期時間：2026-04-25 17:24:20
- 問題原因：使用者要求「全都要」，也就是除了 3D workspace 點選外，還要補上滑鼠 hover 顯示目標、拖曳旋轉視角，以及左鍵按住時先做平滑動畫預覽、放開按鍵才真正發送命令；上一版雖然已能點擊取點，但仍是單擊即送，沒有 hover 資訊，也無法改變 3D 視角。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 中將 preview canvas 的互動改成完整 pointer state：`<Motion>` 會顯示最近有效 XYZ，`<ButtonPress-3>/<B3-Motion>` 會更新 3D workspace 的 azimuth 與 tilt 以旋轉視角，`<ButtonPress-1>/<B1-Motion>` 會在有效點間切換 preview target，並使用獨立的 preview joint override 加上 `after(16, ...)` 動畫插值做平滑預覽，直到 `<ButtonRelease-1>` 才提交目前目標；若 serial 已連線則放開時直接送出既有 `XYZ x y z` 命令，未連線則只更新本地 XYZ 與 PWM 預覽；另外補上手動輸入、PWM/XYZ/HOME 送出與關閉視窗時的動畫清理，避免 drag preview 狀態殘留；最後用 workspace `.venv` 驗證 hover 可得到 `hover=(166, 96, 204)`、右鍵拖曳可將視角由 `38.0` 轉到 `56.0`、左鍵按住時預覽訊息為 `3D workspace preview x=183 y=0 z=171 mm | hold to scrub, release to send`，放開後實際送出 `XYZ 183 0 171`。

### 092. 放大 3D workspace 點雲並提高整片區域可點性
- 日期時間：2026-04-26 10:01:06
- 問題原因：使用者指出依照目前畫面，3D workspace 的可達點雲看起來偏小，很多有效區域不容易點中；根因是原本的 3D 投影縮放邊界把過長的 X/Y/Z 軸導引也一起算進去，導致左下角保留了過多空白、真正的可達點雲被壓縮；另外點雲取樣密度與 click pick 半徑也偏保守，使邊緣區域更難點。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 中將 3D workspace 的投影邊界改為以點雲本體加上縮短後的軸線導引為主，並補上固定比例 padding；同時把工作區取樣密度由 `13` 提升到 `17`、量化間距由 `8 mm` 收斂到 `6 mm`，讓點雲更密；另外把 click pick 半徑由 `20 px` 提高到 `28 px`、點半徑也同步放大，並減少 panel 上下文字佔用的 plot 空間，讓可達區域在同一面板內放得更大；最後用 workspace `.venv` 驗證 GUI 檔案無錯誤、有效點數由 `1353` 提升到 `3023`，且放大後左鍵按住/放開送出仍可正常送出 `XYZ 140 107 201`。

### 093. 將 3D workspace 預設改成前視第一人稱投影
- 日期時間：2026-04-26 10:09:42
- 問題原因：使用者希望 3D workspace 的畫面像「自己站在底座後面往前看」的第一人稱方向，這樣點選目標會更直覺；根因是原本預設投影仍是斜視混合 X/Y 的 oblique 視角，左右方向同時混入 X 與 Y，對操作時的空間直覺不夠直接。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 中將 3D workspace 的預設視角改為 `azimuth=0`，並把投影公式改成前視風格：畫面左右主要對應 `Y`、上下主要對應 `Z`，前進方向 `X` 只透過較小的 depth tilt 影響垂直位置，讓畫面更接近使用者站在底座後方向前看的感受；最後用 workspace `.venv` 驗證預設視角已變成 `0.0 / 0.28`，投影座標符合 `left -> x=-100`、`up -> y=100`、`forward -> y=-28`，且左鍵按住/放開送出仍可正常送出 `XYZ 140 107 201`。

### 094. 新增 Live Follow 模式與可調反應時間欄位
- 日期時間：2026-04-26 11:56:54
- 問題原因：使用者要求保留原本「點擊按住預覽、放開執行」的模式，同時新增一個可切換的模式，讓滑鼠移動到哪裡，手臂就即時跟到哪裡，並提供一個可由使用者調整的反應時間欄位；根因是現有 3D workspace 互動只有 press-drag-release 這一條操作鏈，沒有模式切換，也沒有任何送命令節流參數可以調整即時跟隨的更新速度。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 的 Motion Preview 區塊新增 `3D Mode` 下拉選單與 `Reaction (ms)` 欄位，支援 `Hold Preview / Release Send` 與 `Live Follow` 兩種模式；在 Live Follow 模式下，滑鼠移動到有效工作區點時會先即時做本地平滑預覽，再依使用者設定的反應時間節流發送最新的 `XYZ x y z` 命令，未連線時則只更新本地預覽；另外保留原本左鍵按住預覽、放開送出的舊模式，並在模式切換、手動輸入、HOME/PWM/XYZ 指令與視窗關閉時清理 pending 動畫與排程，避免狀態殘留；最後用 workspace `.venv` 驗證 GUI 檔案無錯誤、舊模式仍可送出 `XYZ 123 -161 169`、切到 Live Follow 且 reaction 設為 `0` 時滑鼠移動會立即送出 `XYZ 140 107 201`，而將 reaction 調成 `200 ms` 後新的 hover target 會被掛到 pending target 並成功節流，不會立刻多送一次命令。

### 095. 將 devlog.md 正式納入 Git 版本控制
- 日期時間：2026-04-26 12:04:53
- 問題原因：使用者要求 `devlog.md` 也要推到遠端並納入版本控制；根因是這個檔案先前被本機的 `.git/info/exclude` 規則忽略，因此雖然內容已持續維護，但一直沒有被 Git 追蹤，也不會跟著 commit/push 上遠端。
- 處理方式：確認 `devlog.md` 目前屬於 ignored 且 untracked 狀態後，將本次處理補記到 `devlog.md`，接著以強制加入追蹤的方式把它正式納入 Git 版本控制並提交到遠端；由於忽略規則只存在本機 `.git/info/exclude`，一旦檔案成為 tracked file，後續修改就會正常出現在 Git 狀態與提交流程中。

### 096. 新增 TOF 避障停車與 GUI 障礙物訊息
- 日期時間：2026-04-26 14:13:08
- 問題原因：使用者表示 VL53L0X 先前已可量測，希望在手臂動作中加入避障；目前 firmware 只會每秒輸出 TOF telemetry，並不會在偵測到近距離障礙物時停止 cartesian / servo / demo 動作，GUI 也沒有可直接辨識的障礙物提示。
- 處理方式：在 `Appli/Core/Src/app_main.c` 新增以 VL53L0X 為基礎的 obstacle monitor，於手臂運動期間以 100 ms 週期讀取單次量測；當量測有效且距離低於 120 mm 時，立即中止 `app_cartesian_trajectory`、`app_servo_trajectory` 與 demo 狀態，讓手臂停在當前姿態，並透過 UART 額外輸出 `[safety] obstacle ... action=stop` 與 `[status] obstacle=...` 狀態列；同時在 `tools/robot_arm_debug_gui.py` 新增 obstacle status / safety event 解析與 GUI 顯示欄位，收到安全事件時會直接顯示「有障礙物，手臂已停止」。最後以 `python -m py_compile` 驗證 GUI 腳本語法，並以 `CMake` Debug preset 重新建置 Appli，確認 firmware 可正常編譯連結。

### 097. 新增 GUI TOF 距離儀表與 3D 操作引導
- 日期時間：2026-04-26 14:24:07
- 問題原因：使用者希望把 TOF 偵測距離直接畫在 Motion Preview 左側空白處，並讓畫面更明確引導去操作右側 3D workspace；目前 GUI 雖然已有障礙物文字狀態，但左側仍留白，3D 區塊的互動入口也不夠醒目。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 的 Motion Preview 左側新增 `TOF Safety` 視覺卡片，以量表、狀態色與大字距離值顯示目前 TOF range、auto-stop threshold、clear 區間與裝置名稱；同時在卡片底部加入 `Click 3D Cloud` CTA，並在右側 `3D Workspace` 標題區新增 `CLICK + DRAG 3D` badge、改寫引導文案與底部提示文字，讓使用者更容易理解可直接點選 cyan cloud 做預覽與送出。最後以 `python -m py_compile` 驗證 GUI 腳本語法正確。

### 098. 新增 VL53L0X 自動恢復與修正 3D 引導排版
- 日期時間：2026-04-26 14:37:41
- 問題原因：使用者回報 GUI 持續顯示 `obstacle=disabled ... device=sensor-not-ready`，代表 VL53L0X 在開機後沒有進入 ready 狀態，避障監控實際上尚未啟用；同時新的 3D badge 會與標題說明文字互相覆蓋，影響可讀性。
- 處理方式：在 `Appli/Core/Src/app_main.c` 新增 VL53L0X 背景自動恢復機制，當開機初次掃描未能讓 TOF 進入 ready 狀態時，主迴圈會每 1.5 秒重新偵測並重試初始化；恢復成功後會立即量測一次並主動輸出最新 status，讓 GUI 能從 `sensor-not-ready` 自動切回正常障礙物狀態。另在 `tools/robot_arm_debug_gui.py` 調整 3D badge 尺寸、文案與標題說明區寬度，並下移 3D plot 起點，避免 badge 壓到引導文字。最後以 `python -m py_compile` 驗證 GUI 腳本語法，並以 `CMake` Debug preset 重新建置 Appli，確認 firmware 可正常編譯連結。

### 099. 新增 Serial Log 暫停與複製按鍵
- 日期時間：2026-04-26 14:48:53
- 問題原因：使用者希望在 GUI 的 `Serial Log` 區塊補上暫停鍵與複製鍵；目前 log 視窗只能持續自動捲動與清空，無法在觀察某段輸出時先暫停畫面，也無法一鍵把目前 log 內容複製出去。
- 處理方式：在 `tools/robot_arm_debug_gui.py` 的底部控制列新增 `Pause Log` 與 `Copy Log` 按鍵；暫停模式下仍會持續解析 firmware 回傳內容並更新 GUI 狀態，但先把新 log 緩存在記憶體中，等按鈕切回 `Resume Log` 再一次補回畫面，避免漏資料。`Copy Log` 會將目前畫面內容連同暫停期間尚未顯示的緩衝行一起複製到 clipboard；另外 `Clear Log` 也同步清掉暫存緩衝。最後以 `python -m py_compile` 驗證 GUI 腳本語法正確。

### 100. 補強 VL53L0X 初始化失敗步驟診斷
- 日期時間：2026-04-26 14:55:56
- 問題原因：使用者回報 TOF 長時間只顯示 `obstacle=disabled ... sensor-not-ready`，且背景恢復只會印出泛化的 `recovery init failed`；目前雖能知道 VL53L0X 沒有成功進入 ready 狀態，但無法判斷是卡在 `MODEL_ID`、private page 切換、`stop_variable` 讀取，還是 IRQ 設定等哪一步。
- 處理方式：在 `Appli/Core/Src/app_main.c` 的 `Phase1_Vl53l0xInit()` 內，將每個初始化 read/write 步驟拆成獨立失敗診斷 log，失敗時直接輸出 `step` 名稱、寄存器位址、寫入值與 `HAL_I2C_GetError()`；這樣下一次 recovery 失敗時，UART 會直接指出具體卡點，不再只剩 summary。最後以 `CMake` Debug preset 重新建置 Appli，確認 firmware 可正常編譯連結。

### 101. 恢復 CubeMX regenerate 後遺失的 LPUART1 IRQ bridge
- 日期時間：2026-04-26 16:00:16
- 問題原因：使用者在依 proposal 完成 LCD 的 CubeMX 設定並重新生成後，GUI 開始收不到資料；根因是這次 regenerate 把 `Appli/Core/Src/stm32n6xx_it.c` 與 `Appli/Core/Inc/stm32n6xx_it.h` 中原本負責把 `LPUART1_IRQHandler()` 導回 `App_LPUART1_IRQHandler()` 的 glue code 刪掉了，導致板端雖然仍可用 `printf` 從 LPUART1 送資料，但 GUI 送進來的 `STATUS`、`XYZ`、`PWM` 等命令不再進入 `HAL_UART_IRQHandler()` 與 RX ring buffer 流程，因此看起來像 GUI 完全收不到互動資料。
- 處理方式：在 `Appli/Core/Inc/stm32n6xx_it.h` 的 USER CODE 區塊重新引入 `app_main.h` 並補回 `LPUART1_IRQHandler()` 宣告，再在 `Appli/Core/Src/stm32n6xx_it.c` 的 USER CODE 區塊恢復 `LPUART1_IRQHandler()`，讓它重新呼叫 `App_LPUART1_IRQHandler()`；這樣之後即使再由 CubeMX 生成，IRQ bridge 也會留在 USER CODE 區塊中不被覆蓋。最後先用 VS Code Problems 確認兩個檔案無錯誤，再以 `cmake --build --preset Debug --clean-first` 重建 Appli，確認韌體可正常編譯連結。

### 102. 將 LPUART1 IRQ bridge 抽成自有檔案避免 regenerate 覆蓋
- 日期時間：2026-04-26 16:05:01
- 問題原因：使用者希望把這次真正爆出問題的 UART IRQ bridge 再往外包一層，避免下次 CubeMX regenerate 又把修好的內容刪掉；根因是把 `LPUART1_IRQHandler()` 直接補回 `stm32n6xx_it.c/h` 雖然能暫時修好，但這兩個檔案本質上仍屬於 CubeMX 生成面，之後每次重新生成都還是存在再被覆蓋的風險。
- 處理方式：在不會被 CubeMX 重寫的 `Appli/CMakeLists.txt` 新增自有來源檔 `Appli/Core/Src/app_irq_bridge.c`，由這個檔案提供 `__attribute__((weak)) void LPUART1_IRQHandler(void)`，並在函式內轉呼叫 `App_LPUART1_IRQHandler()`；同時把先前臨時補在 `Appli/Core/Src/stm32n6xx_it.c` 與 `Appli/Core/Inc/stm32n6xx_it.h` 的 UART glue code 移除，讓生成檔恢復乾淨、真正的 bridge 改由使用者自有檔案持有。因為這個 bridge 現在不再依附 CubeMX 生成檔，下次 regenerate 就算再洗掉 `stm32n6xx_it`，LPUART1 handler 仍會由自有檔案提供。最後用 VS Code Problems 確認相關檔案無錯誤，並以 `cmake --build --preset Debug --clean-first` 重建 Appli，確認韌體可正常編譯連結。

### 103. 補上 XSPI3 spurious IRQ handler 避免 App 初始化被打斷
- 日期時間：2026-04-26 16:20:38
- 問題原因：使用者回報 GUI 重新連上 COM 埠後仍沒有任何資料；進一步用 GDB 檢查 target 當下 PC，發現 Appli 並不是卡在 UART，而是在 `App_Init()` 期間被意外打進 `XSPI3_IRQHandler` 的 startup 預設無限迴圈。這顆 IRQ 並非 Appli 自己使用，但在 LCD/CubeMX 調整後的整體啟動序列中會被觸發，導致主程式在 `Phase1_LogI2CScan()` 或 `App_LogRobotArmKinematicsModel()` 途中被打斷，GUI 因而看起來完全收不到板端資料。
- 處理方式：先在 `Appli/Core/Src/app_main.c` 的 `App_Init()` 最前面補上清除繼承 IRQ 狀態的防禦碼，再以 GDB 重載映像驗證；確認單純 clear/disable 仍不足後，改在使用者自有的 `Appli/Core/Src/app_irq_bridge.c` 新增 `XSPI3_IRQHandler()`，在第一次 spurious IRQ 打進來時主動 `HAL_NVIC_DisableIRQ(XSPI3_IRQn)` 與 `NVIC_ClearPendingIRQ(XSPI3_IRQn)`，避免再落回 startup 的 `Default_Handler`。最後以 `cmake --build --preset Debug --clean-first` 重建 Appli，並用 ST-LINK GDB server 重新 load 映像後驗證程式已可順利走到 `App_RunLoopIteration()`，表示初始化流程不再被 XSPI3 中斷打斷。

### 104. 補齊 XSPI1 與 XSPI2 defensive handler 避免再次落回 Default_Handler
- 日期時間：2026-04-26 16:36:07
- 問題原因：在前一版只補上 `XSPI3_IRQHandler()` 之後，使用者仍回報「依樣跑不到」，而且目前除錯畫面仍停在 startup 的 `Default_Handler`。重新檢查最終 ELF 符號後發現，只有 `XSPI3_IRQHandler` 被真正 override，`XSPI1_IRQHandler` 與 `XSPI2_IRQHandler` 仍然是 weak alias 指回 `Default_Handler`；同時先前 GDB reload 前的實際停點也曾落在 `XSPI2_IRQHandler`，表示 LCD/CubeMX 更新後不只 XSPI3，其他非 Appli 自有的 XSPI 線也可能帶著 inherited/spurious IRQ 進來。
- 處理方式：把 `Appli/Core/Src/app_irq_bridge.c` 擴充成統一的 XSPI defensive bridge，新增 `XSPI1_IRQHandler()` 與 `XSPI2_IRQHandler()`，並讓 XSPI1/2/3 都共用同一個 helper，在中斷進入時立即 disable 與 clear pending bit，避免再回落到 startup weak handler；另外把 `Appli/Core/Src/app_main.c` 的 `App_ClearInheritedInterruptState()` 一併擴充為進入 App 時先清 `XSPI1_IRQn`、`XSPI2_IRQn`、`XSPI3_IRQn`。完成後需重新 build 並確認最終 ELF 內 `XSPI1_IRQHandler`、`XSPI2_IRQHandler`、`XSPI3_IRQHandler` 都已經是強符號，不再指向 `Default_Handler`。
- 備註：後續以 `arm-none-eabi-nm -n STM32N6_Appli.elf` 比對位址驗證，`Default_Handler` 在 `0x3400fbe4`，而 `LPUART1_IRQHandler` (`0x3401437e`)、`XSPI1_IRQHandler` (`0x3401438a`)、`XSPI2_IRQHandler` (`0x34014398`)、`XSPI3_IRQHandler` (`0x340143a6`) 全部落在 `app_irq_bridge.o` 程式區、不再指向 weak alias，IRQ 防禦層落地完成。

### 105. 對齊 NUCLEO-N657X0-Q 實機板級的 proposal.md 完善
- 日期時間：2026-04-29 10:00:00
- 問題原因：原 `proposal.md` 為早期紙上規劃，跟現行 `STM32N6.ioc` 與 NUCLEO-N657X0-Q 的板級實況對不上：UART 寫成 PA9/PA10 (USART1)，但板上 ST-Link VCP 實際走 LPUART1 PE5/PE6，且 PA9–PA15 是 USB-UCPD/SWD 區不可佔用；4 路 Servo 沒寫對應 timer，與目前 .ioc 的 TIM1_CH1/CH2、TIM2_CH3、TIM16_CH1 對不齊；Audio 區整段標「待定」沒給候選腳，未提及 .ioc 已固定的 PC9 = AUDIOCLK；Camera 區把 OV7670 並行寫成主路徑，沒分清楚 NUCLEO-N657X0-Q 的 FPC 接頭其實是 MIPI CSI-2 (CSI_CKP/N、D0P/N、D1P/N) 而 PO5 = CAM_NRST 已板級固定；user LED/Button 寫「板載預設」沒對應 LD1=PG0 / LD2=PG10 / LD3=PG8 / USER=PC13；板級已保留腳位（debug、TRACE、OCTOSPI flash PN0–PN12、UCPD、FSBL I2C2、AUDIOCLK、CAM_NRST）也沒列出，後續 CubeMX 衝突排查會反覆踩雷。
- 處理方式：在 `proposal.md` 重寫整份規劃：補上「板級不可用 / 已保留腳位」總表；新增「影像策略」章節分路線 A（板載 MIPI CSI-2）與路線 B（OV7670 並行），並標出 PE6 VSYNC 與 LPUART1_RX、PC9 AUDIOCLK 與 OV7670 XCLK 的兩個關鍵衝突；以 SAI1 + PC9 AUDIOCLK 為基準，提出 INMP441/MAX98357A 共用 BCLK/LRCK 的 Audio 候選腳並標出哪些候選會與既有模組衝突；Servo 補上 timer/channel 對應、period=19999 與 prescaler=399 的來源與「為何拆 3 顆 timer」的 AF 限制說明；UART 改為 LPUART1 PE5/PE6 對齊 ST-Link VCP；LED/Button 補上實機腳位；每個 Phase 加上可量測的驗收條件，Phase 2 LCD 標為已完成 bring-up，新增 Phase 6「整合 / FreeRTOS」章節與「已知會踩到的雷」整理。

### 106. 修正 GUI TOF Safety panel 永遠卡在待機與 0 mm
- 日期時間：2026-04-29 13:00:00
- 問題原因：使用者回報 GUI 連上 COM 後 TOF Safety 卡片一直顯示「TOF STANDBY / 0 mm / Waiting for valid range / device pending」，沒有任何即時對應數值。比對後是兩個串連 bug：（1）`Appli/Core/Src/app_main.c` 的 `App_LogRobotArmStatus()` 只送合併版 `[status] xyz=...,obstacle=clear/0mm`，但 `tools/robot_arm_debug_gui.py` 的 `STATUS_OBSTACLE_PATTERN` regex 期待獨立 line `[status] obstacle=<state> range=<n> mm threshold=<n> mm clear=<n> mm device=<name>`，所以 GUI 永遠不會中、`_obstacle_monitor_state` 一直停在初始 `unknown / pending`；（2）`App_UpdateObstacleMonitor()` 在 `!App_HasActiveMotion() && !app_obstacle_detected` 時 early-return，閒置時連量都不量，因此即便 regex 對得起來、距離也只在動作中才會更新。
- 處理方式：在 `Appli/Core/Src/app_main.c` 新增 `App_PublishObstacleStatus()` helper，每 100 ms（`APP_OBSTACLE_MONITOR_INTERVAL_MS`）發一條 GUI 規格的 `[status] obstacle=<state> range=<n> mm threshold=120 mm clear=140 mm device=<name>` line；state 詞彙對齊 GUI 的 `disabled` / `detected` / `clear` / `unknown` 四色帶；`App_UpdateObstacleMonitor()` 把 idle 早回拿掉，改成 idle 也照樣量距 + publish，僅當 `App_HasActiveMotion()` 為真時才走 motion stop 邏輯；`phase1_vl53l0x_ready=false` 時 publish `disabled / sensor-not-ready`，讓 GUI 切到灰色「TOF OFFLINE」、量測讀失敗 publish `disabled / read-failed`；量測成功時用 `Phase1_Vl53l0xDeviceCodeString()` 帶實際 device code (`range-complete` 等) 出去。最後以 `cmake --build --preset Debug` 重建 Appli，僅保留既有的 `target_pulses` 未使用警告與 RWX segment 警告。

### 107. 過濾 VL53L0X TCC 噪音避免 GUI 在 20 mm 與正常值間跳動
- 日期時間：2026-04-29 15:00:00
- 問題原因：上一版讓 GUI 看得到即時 TOF 值之後，使用者回報數字會在 `20 mm`（`device=tcc`）與 `~420 mm`（`device=range-complete`）之間反覆跳動，視覺上極不穩定。比對 firmware UART 日誌確認感測器約每 4–5 次量測會丟出一次 `device_code=8`（Target Consistency Check 失敗）；VL53L0X spec 在這種情況會把距離 clip 成最低 20 mm 並用 status 旗標告知該次量測不可信，這個 20 mm 是 sensor 自己標的 dropout、不是真實近距離。原本 `App_UpdateObstacleMonitor()` 不分青紅皂白把 TCC 讀值當有效量測往 GUI 送，並寫進 `app_obstacle_last_measurement`，於是顯示閃爍、後續判斷也會被噪音污染。
- 處理方式：在 `Appli/Core/Src/app_main.c` 的 `App_UpdateObstacleMonitor()` 拿到量測之後加 `range_status_code != VL53L0X_DEVICEERROR_RANGECOMPLETE` 過濾分支：噪音讀值不更新 `app_obstacle_last_measurement`、不重新計算 detection、改以「上一筆有效量測的 range + 當前 detected/clear 旗標」重發 `[status]` 給 GUI（沒有任何有效量測史時才 fallback 為 `unknown / 0 / <error-code>`）；只有 `RANGECOMPLETE` 的量測才會走原本的 obstacle/clear 判定與 `[safety]` 轉換 log；同時把 `obstacle_now` / `clear_now` 條件簡化為純距離比較，因為 RANGECOMPLETE 已在過濾分支前確認。最後以 `cmake --build --preset Debug` 重建 Appli 通過。

### 108. 新增 GUI IMU Attitude 姿態儀面板與 firmware 100 ms imu status
- 日期時間：2026-04-30 11:00:00
- 問題原因：MPU6050 已經能讀，但 GUI 只有每秒一次的 `[mpu6050] acc=… gyro=…` raw log，沒有任何即時可視化；夾爪上的姿態變化（pitch/roll）、有沒有在抖動、目前溫度都得自己讀數字解釋，跟 TOF Safety 卡片那種「一眼就能看狀態」差太多。使用者要求比照 TOF 卡片做一張 IMU 姿態儀，並把版面改成「TOF + IMU 一排、手臂 side view + 3D workspace 一排」的兩列佈局。
- 處理方式：firmware 端在 `Appli/Core/Src/app_main.c` 新增 `App_PublishImuStatus()` helper 與 `App_UpdateImuMonitor()`（與既有 `App_UpdateObstacleMonitor()` 對稱），每 100 ms 從 `Phase1_Mpu6050ReadRawSample()` 取一筆，用 `atan2f` 算 pitch / roll、`sqrtf(gx²+gy²+gz²)` 算 gyro magnitude，再依 `shaking > tilted > level` 優先序分類成 `disabled / shaking / tilted / level / unknown`，輸出 `[status] imu=<state> pitch=<deg> roll=<deg> gyro_mag=<mdps> temp_dc=<dC>`；threshold 用 `APP_IMU_LEVEL_DEG=5`、`APP_IMU_TILT_DEG=30`、`APP_IMU_SHAKING_MDPS=8000`，呼叫掛在 `App_RunLoopIteration()` 內。GUI 端在 `tools/robot_arm_debug_gui.py` 把 `PREVIEW_CANVAS_WIDTH/HEIGHT` 從 `560×360` 拉到 `760×640`、視窗預設 `1280×820 → 1320×1080`，新增 `STATUS_IMU_PATTERN` regex 與 `_set_imu_monitor_state()` setter，把 `_handle_firmware_line()` 的 IMU 分支放在 obstacle 之前；`_draw_preview()` 改成兩排版面：上排 TOF Safety（左半）+ IMU Attitude（右半），下排 Side view（左半）+ 3D Workspace（右半），所有座標改用 `top_row_top/bottom`、`bottom_row_top/bottom`、`left_col_left/right`、`right_col_left/right` 區塊變數，原本散在 1471/1673/1855 三處的硬寫位置全部對齊到這層抽象。IMU 卡片內含圓形 artificial horizon（sky/ground 雙色填充隨 roll 旋轉、隨 pitch 上下偏移、含 ±15°/±30° pitch ladder 與 12 點鐘參考三角、會繞圓周走的 roll marker、固定中央黃色夾爪 wing 十字）、碟下大字 `pitch ±NN°  roll ±NN°`、底部 gyro magnitude bar 配 `Shake>8000 mdps` threshold 標線、CTA-style 狀態條顯示 state title 與 `device mpu6050  T 26.5 °C`；4 種 state 配色 `level=#52f7ff / tilted=#ffb347 / shaking=#ff6b6b / disabled=#7e8ca3`，`disabled` 時直接畫灰色「OFFLINE」placeholder 取代碟盤。連線時送 `unknown / pending`、斷線時送 `disabled / disconnected`。最後以 `cmake --build --preset Debug` 重建 Appli 通過（ROM 用量 +1.6 KB），GUI 以 `python -m py_compile` 檢查無錯誤、實機啟動後 4 種 IMU state 都能正確 render（已驗證螢幕截圖：pitch +33° roll -27° → TILTED 橘色卡片，horizon 碟盤明顯往左下歪），`STATUS_IMU_PATTERN` 對 4 組 firmware sample line 全部成功匹配。

## LCD Driver Abstraction & UI Scaling (Phase 2 Continued)
- Separated ILI9341 and large panel (labeled as ILI9486) drivers.
- Refactored pp_main.c to use APP_LCD_TYPE macro for switching drivers.
- Identified large panel physically as a 3.2-inch 240x320 display, fixed horizontal clipping by reverting to 240x320 resolution.
- Increased UI font scale from 1U to 2U for readability, resolving spacing and bounding box character limit issues (reduced line max to 20 chars).
