#include "app_main.h"
#include "ov7670.h"

#include "i2c.h"
#define APP_LCD_TYPE 9486

#if APP_LCD_TYPE == 9486
#include "ili9486.h"
#define LCD_WIDTH           ILI9486_WIDTH
#define LCD_HEIGHT          ILI9486_HEIGHT
#define LCD_COLOR_BLACK     ILI9486_COLOR_BLACK
#define LCD_COLOR_WHITE     ILI9486_COLOR_WHITE
#define LCD_COLOR_BLUE      ILI9486_COLOR_BLUE
#define LCD_COLOR_GRAY      ILI9486_COLOR_GRAY
#define LCD_COLOR_CYAN      ILI9486_COLOR_CYAN
#define LCD_COLOR_YELLOW    ILI9486_COLOR_YELLOW
#define LCD_COLOR_GREEN     ILI9486_COLOR_GREEN
#define LCD_COLOR_RED       ILI9486_COLOR_RED
#define LCD_COLOR_DARK      ILI9486_COLOR_DARK
#define LCD_COLOR_ORANGE    ILI9486_COLOR_ORANGE
#define LCD_Init            ILI9486_Init
#define LCD_FillScreen      ILI9486_FillScreen
#define LCD_FillRect        ILI9486_FillRect
#define LCD_DrawString      ILI9486_DrawString
#define LCD_DrawRgb565Image ILI9486_DrawRgb565Image
#elif APP_LCD_TYPE == 9341
#include "ili9341.h"
#define LCD_WIDTH           ILI9341_WIDTH
#define LCD_HEIGHT          ILI9341_HEIGHT
#define LCD_COLOR_BLACK     ILI9341_COLOR_BLACK
#define LCD_COLOR_WHITE     ILI9341_COLOR_WHITE
#define LCD_COLOR_BLUE      ILI9341_COLOR_BLUE
#define LCD_COLOR_GRAY      ILI9341_COLOR_GRAY
#define LCD_COLOR_CYAN      ILI9341_COLOR_CYAN
#define LCD_COLOR_YELLOW    ILI9341_COLOR_YELLOW
#define LCD_COLOR_GREEN     ILI9341_COLOR_GREEN
#define LCD_COLOR_RED       ILI9341_COLOR_RED
#define LCD_COLOR_DARK      ILI9341_COLOR_DARK
#define LCD_COLOR_ORANGE    ILI9341_COLOR_ORANGE
#define LCD_Init            ILI9341_Init
#define LCD_FillScreen      ILI9341_FillScreen
#define LCD_FillRect        ILI9341_FillRect
#define LCD_DrawString      ILI9341_DrawString
#define LCD_DrawRgb565Image ILI9341_DrawRgb565Image
#endif
#include "main.h"
#include "robot_arm_kinematics.h"
#include "tim.h"

#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define APP_I2C1_SDA_Pin I2C1_SDA_Pin
#define APP_I2C1_SDA_GPIO_Port I2C1_SDA_GPIO_Port
#define APP_I2C1_SDA_AF GPIO_AF4_I2C1
#define APP_I2C1_SCL_Pin I2CA_SCL_Pin
#define APP_I2C1_SCL_GPIO_Port I2CA_SCL_GPIO_Port
#define APP_I2C1_SCL_AF GPIO_AF4_I2C1
#define APP_LD1_BLUE_Pin GPIO_PIN_8
#define APP_LD1_BLUE_GPIO_Port GPIOG
#define APP_LD2_RED_Pin GPIO_PIN_10
#define APP_LD2_RED_GPIO_Port GPIOG
#define APP_LD3_GREEN_Pin GPIO_PIN_0
#define APP_LD3_GREEN_GPIO_Port GPIOG
#define APP_USER_BUTTON_Pin GPIO_PIN_13
#define APP_USER_BUTTON_GPIO_Port GPIOC
#define APP_VCP_TX_Pin GPIO_PIN_8
#define APP_VCP_TX_GPIO_Port GPIOD
#define APP_VCP_TX_AF GPIO_AF7_USART3
#define APP_VCP_RX_Pin GPIO_PIN_9
#define APP_VCP_RX_GPIO_Port GPIOD
#define APP_VCP_RX_AF GPIO_AF7_USART3

#define MPU6050_I2C_ADDR_AD0_LOW 0x68U
#define MPU6050_I2C_ADDR_AD0_HIGH 0x69U
#define MPU6050_WHO_AM_I_REG 0x75U
#define MPU6050_PWR_MGMT_1_REG 0x6BU
#define MPU6050_CONFIG_REG 0x1AU
#define MPU6050_GYRO_CONFIG_REG 0x1BU
#define MPU6050_ACCEL_CONFIG_REG 0x1CU
#define MPU6050_ACCEL_XOUT_H_REG 0x3BU
#define MPU6050_CLOCK_PLL_XGYRO 0x01U
#define VL53L0X_I2C_ADDR_DEFAULT 0x29U
#define VL53L0X_SYSRANGE_START_REG 0x00U
#define VL53L0X_SYSTEM_INTERRUPT_CONFIG_GPIO_REG 0x0AU
#define VL53L0X_SYSTEM_INTERRUPT_CLEAR_REG 0x0BU
#define VL53L0X_RESULT_INTERRUPT_STATUS_REG 0x13U
#define VL53L0X_RESULT_RANGE_STATUS_REG 0x14U
#define VL53L0X_GPIO_HV_MUX_ACTIVE_HIGH_REG 0x84U
#define VL53L0X_PRIVATE_STANDARD_MODE_REG 0x88U
#define VL53L0X_VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV_REG 0x89U
#define VL53L0X_PRIVATE_INIT_REG 0x80U
#define VL53L0X_PRIVATE_PAGE_SELECT_REG 0xFFU
#define VL53L0X_PRIVATE_STOP_VARIABLE_REG 0x91U
#define VL53L0X_MODEL_ID_REG 0xC0U
#define VL53L0X_MODEL_ID_VALUE 0xEEU
#define VL53L0X_DEVICEERROR_RANGECOMPLETE 11U
#define PHASE1_I2C_TIMEOUT_MS 50U
#define PHASE1_VL53L0X_MEASUREMENT_TIMEOUT_MS 100U
#define PHASE1_MPU6050_ACCEL_LSB_PER_G 16384L
#define PHASE1_MPU6050_GYRO_LSB_PER_DPS 131L
#define PHASE1_VL53L0X_RANGE_JUMP_MM 200U
#define PHASE1_VL53L0X_NEAR_FLOOR_MM 30U
#define APP_SERVO_PWM_FREQUENCY_HZ 50U
#define APP_SERVO_TICK_FREQUENCY_HZ 1000000U
#define APP_SERVO_PERIOD_US (APP_SERVO_TICK_FREQUENCY_HZ / APP_SERVO_PWM_FREQUENCY_HZ)
#define APP_BUTTON_TOGGLE_DEBOUNCE_MS 150U
#define APP_SERVO_DEMO_TRIGGER_MS 50U
#define APP_SERVO_DEMO_STEP_HOLD_MS 700U
#define APP_CLAMP_SERVO_SAFE_PULSE_US 1150U
#define APP_CLAMP_SERVO_OPEN_PULSE_US 800U
#define APP_CLAMP_SERVO_CLOSE_PULSE_US 1500U
#define APP_CLAMP_SERVO_DEFAULT_PULSE_US ((APP_CLAMP_SERVO_OPEN_PULSE_US + APP_CLAMP_SERVO_CLOSE_PULSE_US) / 2U)
#define APP_FORE_AFT_SERVO_AFT_PULSE_US 1000U
#define APP_FORE_AFT_SERVO_FORE_PULSE_US 1600U
#define APP_FORE_AFT_SERVO_CENTER_PULSE_US 1300U
#define APP_FORE_AFT_SERVO_DEFAULT_PULSE_US ((APP_FORE_AFT_SERVO_AFT_PULSE_US + APP_FORE_AFT_SERVO_FORE_PULSE_US) / 2U)
#define APP_GRIPPER_LIFT_SERVO_UP_PULSE_US 1200U
#define APP_GRIPPER_LIFT_SERVO_DOWN_PULSE_US 700U
#define APP_GRIPPER_LIFT_SERVO_DEFAULT_PULSE_US ((APP_GRIPPER_LIFT_SERVO_UP_PULSE_US + APP_GRIPPER_LIFT_SERVO_DOWN_PULSE_US) / 2U)
#define APP_LEFT_RIGHT_SERVO_CENTER_PULSE_US 1400U
#define APP_LEFT_RIGHT_SERVO_LEFT_PULSE_US 1700U
#define APP_LEFT_RIGHT_SERVO_RIGHT_PULSE_US 1100U
#define APP_LEFT_RIGHT_SERVO_DEFAULT_PULSE_US ((APP_LEFT_RIGHT_SERVO_LEFT_PULSE_US + APP_LEFT_RIGHT_SERVO_RIGHT_PULSE_US) / 2U)
#define APP_ROBOT_ARM_PI 3.14159265358979323846f
#define APP_LEFT_RIGHT_SAFE_MIN_DEG (-60.0f)
#define APP_LEFT_RIGHT_SAFE_MAX_DEG 60.0f
#define APP_FORE_AFT_SAFE_MIN_DEG 30.0f
#define APP_FORE_AFT_SAFE_MAX_DEG 90.0f
#define APP_GRIPPER_LIFT_SAFE_MIN_DEG (-60.0f)
#define APP_GRIPPER_LIFT_SAFE_MAX_DEG 0.0f
#define APP_SAFE_WORKSPACE_MIN_Z_MM 0.0f
#define APP_SAFE_WORKSPACE_SAMPLES_PER_JOINT 25U
#define APP_UART_COMMAND_BUFFER_LENGTH 96U
#define APP_UART_RX_RING_BUFFER_LENGTH 256U
#define APP_UART_TX_RING_BUFFER_LENGTH 4096U
#define APP_SERVO_TRAJECTORY_UPDATE_INTERVAL_MS 20U
#define APP_SERVO_TRAJECTORY_MIN_DURATION_MS 180U
#define APP_SERVO_TRAJECTORY_MAX_DURATION_MS 2200U
#define APP_OBSTACLE_MONITOR_INTERVAL_MS 100U
#define APP_VL53L0X_RECOVERY_INTERVAL_MS 1500U
#define APP_OBSTACLE_STOP_DISTANCE_MM 120U
#define APP_OBSTACLE_CLEAR_HYSTERESIS_MM 20U
#define APP_OBSTACLE_CLEAR_DISTANCE_MM (APP_OBSTACLE_STOP_DISTANCE_MM + APP_OBSTACLE_CLEAR_HYSTERESIS_MM)
#define APP_IMU_MONITOR_INTERVAL_MS 100U
#define APP_IMU_LEVEL_DEG 5
#define APP_IMU_TILT_DEG 30
#define APP_IMU_SHAKING_MDPS 8000U
#define APP_SERVO_TRAJECTORY_CLAMP_SPEED_US_PER_S 1800U
#define APP_SERVO_TRAJECTORY_FORE_AFT_SPEED_US_PER_S 900U
#define APP_SERVO_TRAJECTORY_GRIPPER_LIFT_SPEED_US_PER_S 700U
#define APP_SERVO_TRAJECTORY_LEFT_RIGHT_SPEED_US_PER_S 900U
#define APP_CARTESIAN_TRAJECTORY_LINEAR_SPEED_MM_PER_S 120.0f
#define APP_ROBOT_ARM_BASE_HEIGHT_MM 60.0f
#define APP_ROBOT_ARM_SHOULDER_LENGTH_MM 95.0f
#define APP_ROBOT_ARM_FOREARM_LENGTH_MM 90.0f
#define APP_ROBOT_ARM_TOOL_LENGTH_MM 55.0f
#define APP_LCD_UPDATE_INTERVAL_MS 500U
#define APP_LCD_STATUS_HEIGHT 96U
#define APP_LCD_CAMERA_Y APP_LCD_STATUS_HEIGHT
#define APP_LCD_CAMERA_HEIGHT (LCD_HEIGHT - APP_LCD_CAMERA_Y)

typedef enum
{
  APP_CLAMP_SERVO_STATE_SAFE = 0,
  APP_CLAMP_SERVO_STATE_OPEN,
  APP_CLAMP_SERVO_STATE_CLOSE
} App_ClampServoState;

typedef enum
{
  APP_LEFT_RIGHT_SERVO_STATE_CENTER = 0,
  APP_LEFT_RIGHT_SERVO_STATE_LEFT,
  APP_LEFT_RIGHT_SERVO_STATE_RIGHT,
  APP_LEFT_RIGHT_SERVO_STATE_CUSTOM
} App_LeftRightServoState;

typedef enum
{
  APP_SERVO_MOTION_SOURCE_IDLE = 0,
  APP_SERVO_MOTION_SOURCE_HOME,
  APP_SERVO_MOTION_SOURCE_PWM,
  APP_SERVO_MOTION_SOURCE_XYZ,
  APP_SERVO_MOTION_SOURCE_DEMO
} App_ServoMotionSource;

typedef struct
{
  int16_t accel_x;
  int16_t accel_y;
  int16_t accel_z;
  int16_t temperature_raw;
  int16_t gyro_x;
  int16_t gyro_y;
  int16_t gyro_z;
} Phase1_Mpu6050RawSample;

typedef struct
{
  uint16_t range_mm;
  uint8_t range_status_raw;
  uint8_t range_status_code;
} Phase1_Vl53l0xMeasurement;

typedef struct
{
  uint16_t clamp_pulse_us;
  uint16_t fore_aft_pulse_us;
  uint16_t gripper_lift_pulse_us;
  uint16_t left_right_pulse_us;
} App_ServoPulseSet;

typedef struct
{
  bool active;
  App_ServoMotionSource source;
  uint32_t start_tick;
  uint32_t duration_ms;
  uint32_t last_update_tick;
  App_ServoPulseSet start_pulses;
  App_ServoPulseSet target_pulses;
} App_ServoTrajectory;

typedef struct
{
  bool active;
  bool waiting_between_steps;
  uint8_t step_index;
  uint32_t wait_start_tick;
} App_ServoDemoState;

typedef struct
{
  bool active;
  uint32_t start_tick;
  uint32_t duration_ms;
  uint32_t last_update_tick;
  RobotArmVector3 start_position_mm;
  RobotArmVector3 target_position_mm;
  App_ServoPulseSet target_pulses;
  RobotArmElbowMode elbow_mode;
} App_CartesianTrajectory;

static UART_HandleTypeDef huart3;
static uint8_t phase1_mpu6050_address = 0U;
static bool phase1_mpu6050_ready = false;
static bool phase1_vl53l0x_ready = false;
static uint8_t phase1_vl53l0x_stop_variable = 0U;
static bool phase1_vl53l0x_has_last_range = false;
static uint16_t phase1_vl53l0x_last_range_mm = 0U;
static bool app_obstacle_has_last_measurement = false;
static bool app_obstacle_detected = false;
static Phase1_Vl53l0xMeasurement app_obstacle_last_measurement = { 0U, 0U, 0U };
static uint32_t app_last_blink_tick = 0U;
static uint32_t app_last_heartbeat_tick = 0U;
static uint32_t app_last_sensor_tick = 0U;
static uint32_t app_last_lcd_tick = 0U;
static uint32_t app_last_obstacle_monitor_tick = 0U;
static uint32_t app_last_vl53l0x_recovery_tick = 0U;
static uint32_t app_last_imu_monitor_tick = 0U;
static uint32_t app_last_button_toggle_tick = 0U;
static uint32_t app_button_press_tick = 0U;
static bool app_button_pressed = false;
static bool app_last_button_pressed = false;
static App_ClampServoState app_clamp_servo_state = APP_CLAMP_SERVO_STATE_SAFE;
static uint16_t app_clamp_servo_current_pulse_us = APP_CLAMP_SERVO_DEFAULT_PULSE_US;
static uint16_t app_fore_aft_servo_current_pulse_us = APP_FORE_AFT_SERVO_DEFAULT_PULSE_US;
static uint16_t app_gripper_lift_servo_current_pulse_us = APP_GRIPPER_LIFT_SERVO_DEFAULT_PULSE_US;
static App_LeftRightServoState app_left_right_servo_state = APP_LEFT_RIGHT_SERVO_STATE_CENTER;
static uint16_t app_left_right_servo_current_pulse_us = APP_LEFT_RIGHT_SERVO_DEFAULT_PULSE_US;
static App_ServoTrajectory app_servo_trajectory = { 0 };
static App_ServoDemoState app_servo_demo = { 0 };
static App_CartesianTrajectory app_cartesian_trajectory = { 0 };
static uint8_t app_uart_rx_byte = 0U;
static uint8_t app_uart_rx_ring_buffer[APP_UART_RX_RING_BUFFER_LENGTH];
static volatile uint16_t app_uart_rx_ring_read_index = 0U;
static volatile uint16_t app_uart_rx_ring_write_index = 0U;
static volatile bool app_uart_rx_ring_overflow = false;
static uint8_t app_uart_tx_ring_buffer[APP_UART_TX_RING_BUFFER_LENGTH];
static volatile uint16_t app_uart_tx_ring_read_index = 0U;
static volatile uint16_t app_uart_tx_ring_write_index = 0U;
static char app_uart_command_buffer[APP_UART_COMMAND_BUFFER_LENGTH];
static uint16_t app_uart_command_length = 0U;
static bool app_uart_command_overflow = false;
static bool app_lcd_ready = false;
static bool app_camera_frame_seen = false;
static uint32_t app_camera_frame_count = 0U;
static uint16_t app_camera_frame_width = 0U;
static uint16_t app_camera_frame_height = 0U;

static const App_ServoPulseSet app_servo_demo_steps[] = {
  { APP_CLAMP_SERVO_OPEN_PULSE_US, APP_FORE_AFT_SERVO_CENTER_PULSE_US, APP_GRIPPER_LIFT_SERVO_UP_PULSE_US, APP_LEFT_RIGHT_SERVO_RIGHT_PULSE_US },
  { APP_CLAMP_SERVO_OPEN_PULSE_US, APP_FORE_AFT_SERVO_CENTER_PULSE_US, APP_GRIPPER_LIFT_SERVO_UP_PULSE_US, APP_LEFT_RIGHT_SERVO_LEFT_PULSE_US },
  { APP_CLAMP_SERVO_OPEN_PULSE_US, APP_FORE_AFT_SERVO_CENTER_PULSE_US, APP_GRIPPER_LIFT_SERVO_UP_PULSE_US, APP_LEFT_RIGHT_SERVO_CENTER_PULSE_US },
  { APP_CLAMP_SERVO_CLOSE_PULSE_US, APP_FORE_AFT_SERVO_CENTER_PULSE_US, APP_GRIPPER_LIFT_SERVO_UP_PULSE_US, APP_LEFT_RIGHT_SERVO_CENTER_PULSE_US },
  { APP_CLAMP_SERVO_OPEN_PULSE_US, APP_FORE_AFT_SERVO_CENTER_PULSE_US, APP_GRIPPER_LIFT_SERVO_UP_PULSE_US, APP_LEFT_RIGHT_SERVO_CENTER_PULSE_US },
  { APP_CLAMP_SERVO_OPEN_PULSE_US, APP_FORE_AFT_SERVO_AFT_PULSE_US, APP_GRIPPER_LIFT_SERVO_UP_PULSE_US, APP_LEFT_RIGHT_SERVO_CENTER_PULSE_US },
  { APP_CLAMP_SERVO_OPEN_PULSE_US, APP_FORE_AFT_SERVO_FORE_PULSE_US, APP_GRIPPER_LIFT_SERVO_UP_PULSE_US, APP_LEFT_RIGHT_SERVO_CENTER_PULSE_US },
  { APP_CLAMP_SERVO_OPEN_PULSE_US, APP_FORE_AFT_SERVO_CENTER_PULSE_US, APP_GRIPPER_LIFT_SERVO_UP_PULSE_US, APP_LEFT_RIGHT_SERVO_CENTER_PULSE_US },
  { APP_CLAMP_SERVO_OPEN_PULSE_US, APP_FORE_AFT_SERVO_CENTER_PULSE_US, APP_GRIPPER_LIFT_SERVO_DOWN_PULSE_US, APP_LEFT_RIGHT_SERVO_CENTER_PULSE_US },
  { APP_CLAMP_SERVO_OPEN_PULSE_US, APP_FORE_AFT_SERVO_CENTER_PULSE_US, APP_GRIPPER_LIFT_SERVO_UP_PULSE_US, APP_LEFT_RIGHT_SERVO_CENTER_PULSE_US }
};

static const char *const app_servo_demo_step_labels[] = {
  "left_right right",
  "left_right left",
  "left_right center",
  "clamp close",
  "clamp open",
  "fore_aft aft",
  "fore_aft fore",
  "fore_aft center",
  "gripper_lift down",
  "gripper_lift up"
};

static void App_USART3_UART_Init(void);
static void App_GPIO_Init(void);
static bool App_ServoConfigureTimer(TIM_HandleTypeDef *timer_handle, const char *timer_name);
static bool App_ServoConfigureTiming(void);
static bool App_ServoInit(void);
static uint16_t App_Int32ToPulseWidth(int32_t value);
static uint16_t App_ClampPulseToRange(uint16_t pulse_width_us, uint16_t minimum_pulse_us, uint16_t maximum_pulse_us);
static uint16_t App_ClampPulseToMeasuredSafeWindow(const char *servo_name,
                                                   uint16_t pulse_width_us,
                                                   uint16_t minimum_pulse_us,
                                                   uint16_t maximum_pulse_us);
static const char *App_ServoMotionSourceName(App_ServoMotionSource source);
static const char *App_ElbowModeName(RobotArmElbowMode elbow_mode);
static App_ServoPulseSet App_ServoGetCurrentPulseSet(void);
static App_ServoPulseSet App_ServoClampPulseSet(const App_ServoPulseSet *requested_pulses);
static void App_ServoApplyPulseSet(const App_ServoPulseSet *pulse_set);
static uint32_t App_AbsDifferenceU16(uint16_t first_value, uint16_t second_value);
static uint32_t App_ComputeAxisTrajectoryDurationMs(uint16_t start_pulse_us,
                                                    uint16_t target_pulse_us,
                                                    uint32_t speed_us_per_second);
static uint32_t App_ServoTrajectoryComputeDurationMs(const App_ServoPulseSet *start_pulses,
                                                     const App_ServoPulseSet *target_pulses);
static float App_Vector3DistanceMm(const RobotArmVector3 *start_position, const RobotArmVector3 *target_position);
static uint32_t App_CartesianTrajectoryComputeDurationMs(const RobotArmVector3 *start_position,
                                                         const RobotArmVector3 *target_position,
                                                         const App_ServoPulseSet *start_pulses,
                                                         const App_ServoPulseSet *target_pulses);
static bool App_HasActiveMotion(void);
static App_ServoMotionSource App_GetActiveMotionSource(void);
static void App_StopActiveMotionAtCurrentPose(void);
static void App_TryRecoverVl53l0x(uint32_t now);
static void App_UpdateObstacleMonitor(uint32_t now);
static void App_PublishObstacleStatus(const char *state, uint16_t range_mm, const char *device);
static void App_UpdateImuMonitor(uint32_t now);
static void App_PublishImuStatus(const char *state, int pitch_deg, int roll_deg, uint32_t gyro_mag_mdps, int temp_dc);
static float App_MinimumJerkBlend(uint32_t elapsed_ms, uint32_t duration_ms);
static uint16_t App_InterpolatePulse(uint16_t start_pulse_us, uint16_t target_pulse_us, float blend);
static bool App_ServoTrajectoryStart(const App_ServoPulseSet *target_pulses,
                                     App_ServoMotionSource source,
                                     uint32_t *duration_ms_out);
static void App_ServoTrajectoryUpdate(uint32_t now);
static RobotArmVector3 App_InterpolateCartesianTarget(const RobotArmVector3 *start_position,
                                                      const RobotArmVector3 *target_position,
                                                      float blend);
static App_ServoPulseSet App_RobotArmJointAnglesToPulseSet(const RobotArmJointAngles *joint_angles);
static bool App_CartesianTrajectoryStart(const RobotArmVector3 *target_position,
                                         const RobotArmJointAngles *target_joint_solution,
                                         RobotArmElbowMode elbow_mode,
                                         uint32_t *duration_ms_out);
static void App_CartesianTrajectoryUpdate(uint32_t now);
static bool App_ServoDemoQueueStep(uint8_t step_index);
static void App_UpdateServoDemo(uint32_t now);
static void App_ClampServoApplyPulse(uint16_t pulse_width_us);
static void App_ForeAftServoApplyPulse(uint16_t pulse_width_us);
static void App_GripperLiftServoApplyPulse(uint16_t pulse_width_us);
static void App_LeftRightServoApplyPulse(uint16_t pulse_width_us);
static const char *App_LeftRightServoStateName(App_LeftRightServoState state);
static void App_RunServoFullRangeDemo(void);
static float App_DegreesToRadians(float degrees);
static float App_RadiansToDegrees(float radians);
static long App_RoundFloatToLong(float value);
static RobotArmGeometry App_RobotArmGetGeometry(void);
static RobotArmJointLimits App_RobotArmGetSafeJointLimits(void);
static uint16_t App_MapAngleRadiansToPulse(float angle_rad,
                                           float minimum_angle_rad,
                                           float maximum_angle_rad,
                                           uint16_t minimum_pulse_us,
                                           uint16_t maximum_pulse_us);
static float App_MapPulseToAngleRadians(uint16_t pulse_width_us,
                                        uint16_t minimum_pulse_us,
                                        uint16_t maximum_pulse_us,
                                        float minimum_angle_rad,
                                        float maximum_angle_rad);
static RobotArmJointAngles App_RobotArmGetCurrentJointAngles(void);
static bool App_RobotArmEstimateCurrentPose(RobotArmPose *pose);
static RobotArmIkStatus App_RobotArmSolveSafeIk(const RobotArmVector3 *target_position,
                                                RobotArmJointAngles *joint_solution,
                                                RobotArmElbowMode *selected_elbow_mode);
static void App_RobotArmApplyJointAngles(const RobotArmJointAngles *joint_angles);
static bool App_RobotArmMoveToTarget(const RobotArmVector3 *target_position);
static void App_LogRobotArmStatus(void);
static void App_USART3_StartReceiveIT(void);
static void App_ProcessSerialInput(void);
static void App_ClearInheritedInterruptState(void);
static void App_LcdInit(void);
static void App_LcdDrawStaticLayout(void);
static void App_LcdUpdate(uint32_t now);
static void App_LcdDrawDashboard(void);
static void App_LcdDrawCameraPlaceholder(void);
static void App_QueueSerialByteFromISR(uint8_t received_byte);
static bool App_DequeueSerialByte(uint8_t *received_byte_out);
static void App_HandleSerialByte(uint8_t received_byte);
static void App_ProcessSerialCommand(const char *command_line);
static void App_PrintSerialCommandHelp(void);
static bool App_CommandStartsWith(const char *command_line, const char *command_name);
static bool App_ParseNextInt32(const char **cursor, int32_t *value_out);
static bool App_CommandHasTrailingCharacters(const char *cursor);
static void App_LogRobotArmKinematicsModel(void);
static bool Phase0_IsButtonPressed(void);
static void Phase0_SetLedState(GPIO_TypeDef *gpio_port, uint16_t gpio_pin, bool on);
static void Phase0_LogStartup(bool button_pressed);
static bool Phase1_IsDeviceReady(uint8_t address);
static void Phase1_RecoverI2cBus(void);
static bool Phase1_WriteRegister8(uint8_t address, uint8_t register_address, uint8_t value);
static bool Phase1_ReadRegister8(uint8_t address, uint8_t register_address, uint8_t *value);
static bool Phase1_ReadRegisters(uint8_t address, uint8_t register_address, uint8_t *buffer, uint16_t length);
static bool Phase1_ReadRegister16(uint8_t address, uint8_t register_address, uint16_t *value);
static void Phase1_LogMpu6050Probe(uint8_t address);
static bool Phase1_Mpu6050Init(uint8_t address);
static bool Phase1_Mpu6050ReadRawSample(uint8_t address, Phase1_Mpu6050RawSample *sample);
static void Phase1_LogMpu6050Data(void);
static void Phase1_LogVl53l0xProbe(uint8_t address);
static bool Phase1_Vl53l0xPrepareSingleShot(uint8_t address);
static bool Phase1_Vl53l0xInit(uint8_t address);
static bool Phase1_Vl53l0xReadMeasurementInternal(uint8_t address,
                                                  Phase1_Vl53l0xMeasurement *measurement,
                                                  bool log_failures);
static bool Phase1_Vl53l0xReadMeasurement(uint8_t address, Phase1_Vl53l0xMeasurement *measurement);
static const char *Phase1_Vl53l0xDeviceCodeString(uint8_t device_code);
static const char *Phase1_Vl53l0xMeasurementNote(const Phase1_Vl53l0xMeasurement *measurement, bool has_delta, uint16_t delta_mm);
static void Phase1_LogVl53l0xRange(void);
static void Phase1_LogI2CScan(void);
static void Phase1_LogSensorTelemetry(void);

int __io_putchar(int ch)
{
  uint16_t write_index = app_uart_tx_ring_write_index;
  uint16_t next_write_index = (uint16_t)((write_index + 1U) % APP_UART_TX_RING_BUFFER_LENGTH);

  while (next_write_index == app_uart_tx_ring_read_index)
  {
    /* buffer full; UART TX IRQ will drain — yield to it */
    __WFI();
  }

  app_uart_tx_ring_buffer[write_index] = (uint8_t)ch;
  app_uart_tx_ring_write_index = next_write_index;
  __HAL_UART_ENABLE_IT(&huart3, UART_IT_TXE);
  return ch;
}

void App_ConfigureSystemIsolation(void)
{
  HAL_GPIO_ConfigPinAttributes(APP_I2C1_SDA_GPIO_Port,
                               APP_I2C1_SDA_Pin,
                               GPIO_PIN_SEC | GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(APP_I2C1_SCL_GPIO_Port,
                               APP_I2C1_SCL_Pin,
                               GPIO_PIN_SEC | GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(APP_USER_BUTTON_GPIO_Port,
                               APP_USER_BUTTON_Pin,
                               GPIO_PIN_SEC | GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(APP_VCP_TX_GPIO_Port,
                               APP_VCP_TX_Pin | APP_VCP_RX_Pin,
                               GPIO_PIN_SEC | GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(APP_LD1_BLUE_GPIO_Port,
                               APP_LD1_BLUE_Pin | APP_LD2_RED_Pin | APP_LD3_GREEN_Pin,
                               GPIO_PIN_SEC | GPIO_PIN_NPRIV);
}

void App_Init(void)
{
  App_ClearInheritedInterruptState();
  App_GPIO_Init();
  App_USART3_UART_Init();
  App_LcdInit();

  if (!App_ServoInit())
  {
    Error_Handler();
  }

  OV7670_Init();

  app_button_pressed = Phase0_IsButtonPressed();
  app_last_button_pressed = app_button_pressed;
  app_last_blink_tick = HAL_GetTick();
  app_last_heartbeat_tick = app_last_blink_tick;
  app_last_sensor_tick = app_last_blink_tick;
  app_last_lcd_tick = app_last_blink_tick;
  app_last_obstacle_monitor_tick = app_last_blink_tick;
  app_last_vl53l0x_recovery_tick = app_last_blink_tick;
  app_last_imu_monitor_tick = app_last_blink_tick;
  app_last_button_toggle_tick = 0U;
  app_button_press_tick = 0U;

  Phase0_SetLedState(APP_LD1_BLUE_GPIO_Port, APP_LD1_BLUE_Pin, true);
  Phase0_SetLedState(APP_LD2_RED_GPIO_Port, APP_LD2_RED_Pin, app_button_pressed);
  Phase0_LogStartup(app_button_pressed);
  Phase1_LogI2CScan();
  App_LogRobotArmKinematicsModel();
  App_LcdDrawStaticLayout();
  App_LcdDrawDashboard();
}

static void App_ClearInheritedInterruptState(void)
{
  /* Appli does not own XSPI1/2/3; clear stale state inherited from earlier boot stages. */
  HAL_NVIC_DisableIRQ(XSPI1_IRQn);
  NVIC_ClearPendingIRQ(XSPI1_IRQn);

  HAL_NVIC_DisableIRQ(XSPI2_IRQn);
  NVIC_ClearPendingIRQ(XSPI2_IRQn);

  HAL_NVIC_DisableIRQ(XSPI3_IRQn);
  NVIC_ClearPendingIRQ(XSPI3_IRQn);
}

void App_DisplayCameraFrameRgb565(const uint16_t *pixels, uint16_t width, uint16_t height, uint16_t stride_pixels)
{
  uint16_t draw_width = width;
  uint16_t draw_height = height;
  uint16_t x = 0U;
  uint16_t y = APP_LCD_CAMERA_Y;

  if (!app_lcd_ready || pixels == NULL || width == 0U || height == 0U)
  {
    return;
  }

  if (draw_width > LCD_WIDTH)
  {
    draw_width = LCD_WIDTH;
  }

  if (draw_height > APP_LCD_CAMERA_HEIGHT)
  {
    draw_height = APP_LCD_CAMERA_HEIGHT;
  }

  if (draw_width < LCD_WIDTH)
  {
    x = (uint16_t)((LCD_WIDTH - draw_width) / 2U);
  }

  if (draw_height < APP_LCD_CAMERA_HEIGHT)
  {
    y = (uint16_t)(APP_LCD_CAMERA_Y + ((APP_LCD_CAMERA_HEIGHT - draw_height) / 2U));
  }

  LCD_FillRect(0U, APP_LCD_CAMERA_Y, LCD_WIDTH, APP_LCD_CAMERA_HEIGHT, LCD_COLOR_BLACK);
  LCD_DrawRgb565Image(x, y, draw_width, draw_height, pixels, stride_pixels);
  app_camera_frame_seen = true;
  app_camera_frame_width = width;
  app_camera_frame_height = height;
  ++app_camera_frame_count;
}

static void App_LcdInit(void)
{
  app_lcd_ready = LCD_Init();
  if (app_lcd_ready)
  {
    printf("[lcd] ili9341 init ok spi=SPI5 size=%ux%u\r\n", LCD_WIDTH, LCD_HEIGHT);
  }
  else
  {
    printf("[lcd] ili9341 init failed\r\n");
  }
}

static void App_LcdDrawStaticLayout(void)
{
  if (!app_lcd_ready)
  {
    return;
  }

  LCD_FillScreen(LCD_COLOR_BLACK);
  LCD_FillRect(0U, 0U, LCD_WIDTH, 24U, LCD_COLOR_BLUE);
  LCD_DrawString(4U, 4U, "ROBOT ARM", LCD_COLOR_WHITE, LCD_COLOR_BLUE, 2U);
  LCD_DrawString(140U, 4U, "ILI9486", LCD_COLOR_WHITE, LCD_COLOR_BLUE, 2U);
  LCD_FillRect(0U, (uint16_t)(APP_LCD_CAMERA_Y - 2U), LCD_WIDTH, 2U, LCD_COLOR_GRAY);
  App_LcdDrawCameraPlaceholder();
}

static void App_LcdUpdate(uint32_t now)
{
  if (!app_lcd_ready)
  {
    return;
  }

  if ((now - app_last_lcd_tick) < APP_LCD_UPDATE_INTERVAL_MS)
  {
    return;
  }

  app_last_lcd_tick = now;
  App_LcdDrawDashboard();
}

#define APP_LCD_LINE_FIELD_CHARS 20U
#define APP_LCD_LINE_BUFFER_LEN  (APP_LCD_LINE_FIELD_CHARS + 1U)

static void App_LcdUpdateLine(uint16_t y, char *cache, const char *text, uint16_t color)
{
  char padded[APP_LCD_LINE_BUFFER_LEN];
  size_t len = strlen(text);

  if (len >= APP_LCD_LINE_FIELD_CHARS)
  {
    len = APP_LCD_LINE_FIELD_CHARS;
  }
  memcpy(padded, text, len);
  for (size_t i = len; i < APP_LCD_LINE_FIELD_CHARS; ++i)
  {
    padded[i] = ' ';
  }
  padded[APP_LCD_LINE_FIELD_CHARS] = '\0';

  if (strcmp(padded, cache) == 0)
  {
    return;
  }

  LCD_DrawString(4U, y, padded, color, LCD_COLOR_BLACK, 2U);
  memcpy(cache, padded, APP_LCD_LINE_BUFFER_LEN);
}

static void App_LcdDrawDashboard(void)
{
  static char cache_mpu[APP_LCD_LINE_BUFFER_LEN] = "";
  static char cache_range[APP_LCD_LINE_BUFFER_LEN] = "";
  static char cache_xyz[APP_LCD_LINE_BUFFER_LEN] = "";
  static char cache_pwm[APP_LCD_LINE_BUFFER_LEN] = "";
  static char cache_cam[APP_LCD_LINE_BUFFER_LEN] = "";
  static bool last_obstacle_detected = false;
  static bool first_draw = true;

  char line[48];
  RobotArmPose pose;
  Phase1_Mpu6050RawSample sample;
  bool has_pose = App_RobotArmEstimateCurrentPose(&pose);
  bool has_mpu = phase1_mpu6050_ready && Phase1_Mpu6050ReadRawSample(phase1_mpu6050_address, &sample);

  if (!app_lcd_ready)
  {
    return;
  }

  if (has_mpu)
  {
    long ax_mg = ((long)sample.accel_x * 1000L) / PHASE1_MPU6050_ACCEL_LSB_PER_G;
    long ay_mg = ((long)sample.accel_y * 1000L) / PHASE1_MPU6050_ACCEL_LSB_PER_G;
    long az_mg = ((long)sample.accel_z * 1000L) / PHASE1_MPU6050_ACCEL_LSB_PER_G;
    snprintf(line, sizeof(line), "MPU A %ld %ld %ld", ax_mg, ay_mg, az_mg);
  }
  else
  {
    snprintf(line, sizeof(line), "MPU NO DATA");
  }
  App_LcdUpdateLine(32U, cache_mpu, line, LCD_COLOR_CYAN);

  if (app_obstacle_has_last_measurement)
  {
    snprintf(line,
             sizeof(line),
             "RANGE %uMM %s",
             app_obstacle_last_measurement.range_mm,
             app_obstacle_detected ? "STOP" : "CLEAR");
  }
  else
  {
    snprintf(line, sizeof(line), "RANGE NO DATA");
  }
  /* obstacle color flips between RED/GREEN; force redraw when it toggles */
  if (last_obstacle_detected != app_obstacle_detected || first_draw)
  {
    cache_range[0] = '\0';
    last_obstacle_detected = app_obstacle_detected;
  }
  App_LcdUpdateLine(56U, cache_range, line,
                    app_obstacle_detected ? LCD_COLOR_RED : LCD_COLOR_GREEN);

  if (has_pose)
  {
    snprintf(line,
             sizeof(line),
             "XYZ %ld %ld %ld",
             App_RoundFloatToLong(pose.x_mm),
             App_RoundFloatToLong(pose.y_mm),
             App_RoundFloatToLong(pose.z_mm));
  }
  else
  {
    snprintf(line, sizeof(line), "XYZ NO DATA");
  }
  App_LcdUpdateLine(80U, cache_xyz, line, LCD_COLOR_YELLOW);

  snprintf(line,
           sizeof(line),
           "%u %u %u %u",
           app_clamp_servo_current_pulse_us,
           app_fore_aft_servo_current_pulse_us,
           app_gripper_lift_servo_current_pulse_us,
           app_left_right_servo_current_pulse_us);
  App_LcdUpdateLine(104U, cache_pwm, line, LCD_COLOR_WHITE);

  if (!app_camera_frame_seen)
  {
    if (first_draw)
    {
      App_LcdDrawCameraPlaceholder();
    }
  }
  else
  {
    snprintf(line, sizeof(line), "CAM %ux%u #%lu",
             app_camera_frame_width, app_camera_frame_height,
             (unsigned long)app_camera_frame_count);
    App_LcdUpdateLine(APP_LCD_CAMERA_Y, cache_cam, line, LCD_COLOR_GREEN);
  }

  first_draw = false;
}

static void App_LcdDrawCameraPlaceholder(void)
{
  if (!app_lcd_ready)
  {
    return;
  }

  LCD_FillRect(0U, APP_LCD_CAMERA_Y, LCD_WIDTH, APP_LCD_CAMERA_HEIGHT, LCD_COLOR_DARK);
  LCD_DrawString(18U, (uint16_t)(APP_LCD_CAMERA_Y + 34U), "OV7670 CAMERA", LCD_COLOR_WHITE, LCD_COLOR_DARK, 2U);
  LCD_DrawString(30U, (uint16_t)(APP_LCD_CAMERA_Y + 62U), "WAITING FRAME", LCD_COLOR_ORANGE, LCD_COLOR_DARK, 2U);
  LCD_DrawString(16U, (uint16_t)(APP_LCD_CAMERA_Y + 104U), "WAITING FOR CAM...", LCD_COLOR_GRAY, LCD_COLOR_DARK, 2U);
}

void App_RunLoopIteration(void)
{
  uint32_t now = HAL_GetTick();

  App_TryRecoverVl53l0x(now);
  App_UpdateObstacleMonitor(now);
  App_UpdateImuMonitor(now);

  if (app_cartesian_trajectory.active)
  {
    App_CartesianTrajectoryUpdate(now);
  }
  else
  {
    App_ServoTrajectoryUpdate(now);
  }
  App_UpdateServoDemo(now);
  App_ProcessSerialInput();
  App_LcdUpdate(now);
  app_button_pressed = Phase0_IsButtonPressed();

  if ((now - app_last_blink_tick) >= 500U)
  {
    app_last_blink_tick = now;
    HAL_GPIO_TogglePin(APP_LD3_GREEN_GPIO_Port, APP_LD3_GREEN_Pin);
  }

  Phase0_SetLedState(APP_LD2_RED_GPIO_Port, APP_LD2_RED_Pin, app_button_pressed);

  if (app_button_pressed != app_last_button_pressed)
  {
    app_last_button_pressed = app_button_pressed;

    if (app_button_pressed)
    {
      app_button_press_tick = now;
      printf("[button] pressed\r\n");
    }
    else
    {
      uint32_t held_ms = now - app_button_press_tick;

      printf("[button] released held=%lu ms\r\n", (unsigned long)held_ms);

      if ((now - app_last_button_toggle_tick) >= APP_BUTTON_TOGGLE_DEBOUNCE_MS &&
          held_ms >= APP_SERVO_DEMO_TRIGGER_MS)
      {
        app_last_button_toggle_tick = now;
        App_RunServoFullRangeDemo();
      }
    }
  }

  if ((now - app_last_heartbeat_tick) >= 1000U)
  {
    app_last_heartbeat_tick = now;
       printf("[tick] %lu ms, button=%s, clamp=%u us, fore_aft=%u us, gripper_lift=%u us, left_right=%u us, left_right_state=%s\r\n",
           (unsigned long)now,
           app_button_pressed ? "pressed" : "released",
           app_clamp_servo_current_pulse_us,
           app_fore_aft_servo_current_pulse_us,
           app_gripper_lift_servo_current_pulse_us,
         app_left_right_servo_current_pulse_us,
         App_LeftRightServoStateName(app_left_right_servo_state));
  }

  if ((now - app_last_sensor_tick) >= 1000U)
  {
    app_last_sensor_tick = now;
    Phase1_LogSensorTelemetry();
  }
}

static bool App_ServoInit(void)
{
  if (!App_ServoConfigureTiming())
  {
    return false;
  }

  App_ClampServoApplyPulse(APP_CLAMP_SERVO_DEFAULT_PULSE_US);
  App_ForeAftServoApplyPulse(APP_FORE_AFT_SERVO_DEFAULT_PULSE_US);
  App_GripperLiftServoApplyPulse(APP_GRIPPER_LIFT_SERVO_DEFAULT_PULSE_US);
  app_left_right_servo_state = APP_LEFT_RIGHT_SERVO_STATE_CENTER;
  App_LeftRightServoApplyPulse(APP_LEFT_RIGHT_SERVO_DEFAULT_PULSE_US);

  if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1) != HAL_OK)
  {
    return false;
  }

  if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2) != HAL_OK)
  {
    return false;
  }

  if (HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3) != HAL_OK)
  {
    return false;
  }

  if (HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1) != HAL_OK)
  {
    return false;
  }

    app_clamp_servo_state = APP_CLAMP_SERVO_STATE_SAFE;

  printf("[servo] timers started, %u Hz period=%u us safe=%u us\r\n",
         APP_SERVO_PWM_FREQUENCY_HZ,
         APP_SERVO_PERIOD_US,
         APP_CLAMP_SERVO_SAFE_PULSE_US);
    printf("[servo] default midpoint startup pose clamp=%u fore_aft=%u gripper_lift=%u left_right=%u us\r\n",
      APP_CLAMP_SERVO_DEFAULT_PULSE_US,
      APP_FORE_AFT_SERVO_DEFAULT_PULSE_US,
      APP_GRIPPER_LIFT_SERVO_DEFAULT_PULSE_US,
      APP_LEFT_RIGHT_SERVO_DEFAULT_PULSE_US);
  printf("[servo] button demo: short press runs one full-range sweep across all 4 axes\r\n");
  printf("[servo] hard safety clamps active clamp=%u..%u us fore_aft=%u..%u us gripper_lift=%u..%u us left_right=%u..%u us\r\n",
         APP_CLAMP_SERVO_OPEN_PULSE_US,
         APP_CLAMP_SERVO_CLOSE_PULSE_US,
         APP_FORE_AFT_SERVO_AFT_PULSE_US,
         APP_FORE_AFT_SERVO_FORE_PULSE_US,
         APP_GRIPPER_LIFT_SERVO_DOWN_PULSE_US,
         APP_GRIPPER_LIFT_SERVO_UP_PULSE_US,
         APP_LEFT_RIGHT_SERVO_RIGHT_PULSE_US,
         APP_LEFT_RIGHT_SERVO_LEFT_PULSE_US);
  printf("[servo] left_right CH4 left=%u us right=%u us center=%u us\r\n",
         APP_LEFT_RIGHT_SERVO_LEFT_PULSE_US,
         APP_LEFT_RIGHT_SERVO_RIGHT_PULSE_US,
         APP_LEFT_RIGHT_SERVO_CENTER_PULSE_US);
    printf("[servo] clamp CH1 startup midpoint=%u us\r\n", APP_CLAMP_SERVO_DEFAULT_PULSE_US);
    printf("[servo] fore_aft CH2 startup midpoint=%u us\r\n", APP_FORE_AFT_SERVO_DEFAULT_PULSE_US);
    printf("[servo] gripper_lift CH3 startup midpoint=%u us\r\n", APP_GRIPPER_LIFT_SERVO_DEFAULT_PULSE_US);
    printf("[servo] left_right CH4 startup midpoint=%u us\r\n", APP_LEFT_RIGHT_SERVO_DEFAULT_PULSE_US);

  app_servo_trajectory.active = false;
  app_servo_trajectory.source = APP_SERVO_MOTION_SOURCE_IDLE;
  app_servo_trajectory.start_tick = HAL_GetTick();
  app_servo_trajectory.duration_ms = 0U;
  app_servo_trajectory.last_update_tick = app_servo_trajectory.start_tick;
  app_servo_trajectory.start_pulses = App_ServoGetCurrentPulseSet();
  app_servo_trajectory.target_pulses = app_servo_trajectory.start_pulses;
  app_servo_demo.active = false;
  app_servo_demo.waiting_between_steps = false;
  app_servo_demo.step_index = 0U;
  app_servo_demo.wait_start_tick = 0U;
  app_cartesian_trajectory.active = false;
  app_cartesian_trajectory.start_tick = app_servo_trajectory.start_tick;
  app_cartesian_trajectory.duration_ms = 0U;
  app_cartesian_trajectory.last_update_tick = app_servo_trajectory.start_tick;
  app_cartesian_trajectory.start_position_mm.x_mm = 0.0f;
  app_cartesian_trajectory.start_position_mm.y_mm = 0.0f;
  app_cartesian_trajectory.start_position_mm.z_mm = 0.0f;
  app_cartesian_trajectory.target_position_mm = app_cartesian_trajectory.start_position_mm;
  app_cartesian_trajectory.target_pulses = app_servo_trajectory.start_pulses;
  app_cartesian_trajectory.elbow_mode = ROBOT_ARM_ELBOW_MODE_DOWN;

  return true;
}

static uint16_t App_Int32ToPulseWidth(int32_t value)
{
  if (value < 0)
  {
    return 0U;
  }

  if (value > 0xFFFF)
  {
    return 0xFFFFU;
  }

  return (uint16_t)value;
}

static bool App_ServoConfigureTimer(TIM_HandleTypeDef *timer_handle, const char *timer_name)
{
  uint32_t tim_clock_hz = HAL_RCCEx_GetTIMGFreq();
  uint32_t prescaler_divider;
  uint32_t prescaler_value;

  if (tim_clock_hz < APP_SERVO_TICK_FREQUENCY_HZ)
  {
    return false;
  }

  prescaler_divider = tim_clock_hz / APP_SERVO_TICK_FREQUENCY_HZ;
  if (prescaler_divider == 0U)
  {
    return false;
  }

  prescaler_value = prescaler_divider - 1U;
  if (prescaler_value > 0xFFFFU)
  {
    return false;
  }

  __HAL_TIM_DISABLE(timer_handle);

  timer_handle->Init.Prescaler = prescaler_value;
  timer_handle->Init.Period = APP_SERVO_PERIOD_US - 1U;

  __HAL_TIM_SET_PRESCALER(timer_handle, prescaler_value);
  __HAL_TIM_SET_AUTORELOAD(timer_handle, APP_SERVO_PERIOD_US - 1U);
  __HAL_TIM_SET_COUNTER(timer_handle, 0U);
  timer_handle->Instance->EGR = TIM_EGR_UG;

  printf("[servo] %s timg=%lu Hz prescaler=%lu arr=%u\r\n",
         timer_name,
         (unsigned long)tim_clock_hz,
         (unsigned long)prescaler_value,
         (unsigned int)(APP_SERVO_PERIOD_US - 1U));

  return true;
}

static bool App_ServoConfigureTiming(void)
{
  if (!App_ServoConfigureTimer(&htim1, "TIM1"))
  {
    return false;
  }

  if (!App_ServoConfigureTimer(&htim2, "TIM2"))
  {
    return false;
  }

  if (!App_ServoConfigureTimer(&htim16, "TIM16"))
  {
    return false;
  }

  return true;
}

static uint16_t App_ClampPulseToRange(uint16_t pulse_width_us, uint16_t minimum_pulse_us, uint16_t maximum_pulse_us)
{
  if (pulse_width_us < minimum_pulse_us)
  {
    return minimum_pulse_us;
  }

  if (pulse_width_us > maximum_pulse_us)
  {
    return maximum_pulse_us;
  }

  return pulse_width_us;
}

static uint16_t App_ClampPulseToMeasuredSafeWindow(const char *servo_name,
                                                   uint16_t pulse_width_us,
                                                   uint16_t minimum_pulse_us,
                                                   uint16_t maximum_pulse_us)
{
  uint16_t clamped_pulse_us = App_ClampPulseToRange(pulse_width_us, minimum_pulse_us, maximum_pulse_us);

  if (clamped_pulse_us != pulse_width_us)
  {
    printf("[servo_safety] %s request=%u us clamped=%u us safe_window=%u..%u us\r\n",
           servo_name,
           pulse_width_us,
           clamped_pulse_us,
           minimum_pulse_us,
           maximum_pulse_us);
  }

  return clamped_pulse_us;
}

static const char *App_ServoMotionSourceName(App_ServoMotionSource source)
{
  switch (source)
  {
    case APP_SERVO_MOTION_SOURCE_HOME:
      return "home";

    case APP_SERVO_MOTION_SOURCE_PWM:
      return "pwm";

    case APP_SERVO_MOTION_SOURCE_XYZ:
      return "xyz";

    case APP_SERVO_MOTION_SOURCE_DEMO:
      return "demo";

    case APP_SERVO_MOTION_SOURCE_IDLE:
    default:
      return "idle";
  }
}

static const char *App_ElbowModeName(RobotArmElbowMode elbow_mode)
{
  return elbow_mode == ROBOT_ARM_ELBOW_MODE_UP ? "up" : "down";
}

static App_ServoPulseSet App_ServoGetCurrentPulseSet(void)
{
  App_ServoPulseSet pulse_set = {
    app_clamp_servo_current_pulse_us,
    app_fore_aft_servo_current_pulse_us,
    app_gripper_lift_servo_current_pulse_us,
    app_left_right_servo_current_pulse_us
  };

  return pulse_set;
}

static App_ServoPulseSet App_ServoClampPulseSet(const App_ServoPulseSet *requested_pulses)
{
  App_ServoPulseSet clamped_pulses = App_ServoGetCurrentPulseSet();

  if (requested_pulses == NULL)
  {
    return clamped_pulses;
  }

  clamped_pulses.clamp_pulse_us = App_ClampPulseToMeasuredSafeWindow("clamp",
                                                                      requested_pulses->clamp_pulse_us,
                                                                      APP_CLAMP_SERVO_OPEN_PULSE_US,
                                                                      APP_CLAMP_SERVO_CLOSE_PULSE_US);
  clamped_pulses.fore_aft_pulse_us = App_ClampPulseToMeasuredSafeWindow("fore_aft",
                                                                         requested_pulses->fore_aft_pulse_us,
                                                                         APP_FORE_AFT_SERVO_AFT_PULSE_US,
                                                                         APP_FORE_AFT_SERVO_FORE_PULSE_US);
  clamped_pulses.gripper_lift_pulse_us = App_ClampPulseToMeasuredSafeWindow("gripper_lift",
                                                                             requested_pulses->gripper_lift_pulse_us,
                                                                             APP_GRIPPER_LIFT_SERVO_DOWN_PULSE_US,
                                                                             APP_GRIPPER_LIFT_SERVO_UP_PULSE_US);
  clamped_pulses.left_right_pulse_us = App_ClampPulseToMeasuredSafeWindow("left_right",
                                                                           requested_pulses->left_right_pulse_us,
                                                                           APP_LEFT_RIGHT_SERVO_RIGHT_PULSE_US,
                                                                           APP_LEFT_RIGHT_SERVO_LEFT_PULSE_US);
  return clamped_pulses;
}

static void App_ServoApplyPulseSet(const App_ServoPulseSet *pulse_set)
{
  if (pulse_set == NULL)
  {
    return;
  }

  App_ClampServoApplyPulse(pulse_set->clamp_pulse_us);
  App_ForeAftServoApplyPulse(pulse_set->fore_aft_pulse_us);
  App_GripperLiftServoApplyPulse(pulse_set->gripper_lift_pulse_us);
  App_LeftRightServoApplyPulse(pulse_set->left_right_pulse_us);
}

static uint32_t App_AbsDifferenceU16(uint16_t first_value, uint16_t second_value)
{
  return first_value >= second_value ?
         (uint32_t)(first_value - second_value) :
         (uint32_t)(second_value - first_value);
}

static uint32_t App_ComputeAxisTrajectoryDurationMs(uint16_t start_pulse_us,
                                                    uint16_t target_pulse_us,
                                                    uint32_t speed_us_per_second)
{
  uint32_t delta_pulse_us = App_AbsDifferenceU16(start_pulse_us, target_pulse_us);

  if (delta_pulse_us == 0U || speed_us_per_second == 0U)
  {
    return 0U;
  }

  return ((delta_pulse_us * 1000U) + speed_us_per_second - 1U) / speed_us_per_second;
}

static uint32_t App_ServoTrajectoryComputeDurationMs(const App_ServoPulseSet *start_pulses,
                                                     const App_ServoPulseSet *target_pulses)
{
  uint32_t duration_ms = 0U;
  uint32_t axis_duration_ms;

  if (start_pulses == NULL || target_pulses == NULL)
  {
    return 0U;
  }

  axis_duration_ms = App_ComputeAxisTrajectoryDurationMs(start_pulses->clamp_pulse_us,
                                                         target_pulses->clamp_pulse_us,
                                                         APP_SERVO_TRAJECTORY_CLAMP_SPEED_US_PER_S);
  if (axis_duration_ms > duration_ms)
  {
    duration_ms = axis_duration_ms;
  }

  axis_duration_ms = App_ComputeAxisTrajectoryDurationMs(start_pulses->fore_aft_pulse_us,
                                                         target_pulses->fore_aft_pulse_us,
                                                         APP_SERVO_TRAJECTORY_FORE_AFT_SPEED_US_PER_S);
  if (axis_duration_ms > duration_ms)
  {
    duration_ms = axis_duration_ms;
  }

  axis_duration_ms = App_ComputeAxisTrajectoryDurationMs(start_pulses->gripper_lift_pulse_us,
                                                         target_pulses->gripper_lift_pulse_us,
                                                         APP_SERVO_TRAJECTORY_GRIPPER_LIFT_SPEED_US_PER_S);
  if (axis_duration_ms > duration_ms)
  {
    duration_ms = axis_duration_ms;
  }

  axis_duration_ms = App_ComputeAxisTrajectoryDurationMs(start_pulses->left_right_pulse_us,
                                                         target_pulses->left_right_pulse_us,
                                                         APP_SERVO_TRAJECTORY_LEFT_RIGHT_SPEED_US_PER_S);
  if (axis_duration_ms > duration_ms)
  {
    duration_ms = axis_duration_ms;
  }

  if (duration_ms == 0U)
  {
    return 0U;
  }

  if (duration_ms < APP_SERVO_TRAJECTORY_MIN_DURATION_MS)
  {
    duration_ms = APP_SERVO_TRAJECTORY_MIN_DURATION_MS;
  }
  else if (duration_ms > APP_SERVO_TRAJECTORY_MAX_DURATION_MS)
  {
    duration_ms = APP_SERVO_TRAJECTORY_MAX_DURATION_MS;
  }

  return duration_ms;
}

static float App_Vector3DistanceMm(const RobotArmVector3 *start_position, const RobotArmVector3 *target_position)
{
  float delta_x;
  float delta_y;
  float delta_z;

  if (start_position == NULL || target_position == NULL)
  {
    return 0.0f;
  }

  delta_x = target_position->x_mm - start_position->x_mm;
  delta_y = target_position->y_mm - start_position->y_mm;
  delta_z = target_position->z_mm - start_position->z_mm;
  return sqrtf((delta_x * delta_x) + (delta_y * delta_y) + (delta_z * delta_z));
}

static uint32_t App_CartesianTrajectoryComputeDurationMs(const RobotArmVector3 *start_position,
                                                         const RobotArmVector3 *target_position,
                                                         const App_ServoPulseSet *start_pulses,
                                                         const App_ServoPulseSet *target_pulses)
{
  uint32_t duration_ms = App_ServoTrajectoryComputeDurationMs(start_pulses, target_pulses);
  float distance_mm = App_Vector3DistanceMm(start_position, target_position);
  uint32_t cartesian_duration_ms;

  if (distance_mm <= 0.0f)
  {
    return duration_ms;
  }

  cartesian_duration_ms = (uint32_t)(((distance_mm * 1000.0f) / APP_CARTESIAN_TRAJECTORY_LINEAR_SPEED_MM_PER_S) + 0.5f);
  if (cartesian_duration_ms > duration_ms)
  {
    duration_ms = cartesian_duration_ms;
  }

  if (duration_ms == 0U)
  {
    return 0U;
  }

  if (duration_ms < APP_SERVO_TRAJECTORY_MIN_DURATION_MS)
  {
    duration_ms = APP_SERVO_TRAJECTORY_MIN_DURATION_MS;
  }
  else if (duration_ms > APP_SERVO_TRAJECTORY_MAX_DURATION_MS)
  {
    duration_ms = APP_SERVO_TRAJECTORY_MAX_DURATION_MS;
  }

  return duration_ms;
}

static bool App_HasActiveMotion(void)
{
  return app_cartesian_trajectory.active || app_servo_trajectory.active || app_servo_demo.active;
}

static App_ServoMotionSource App_GetActiveMotionSource(void)
{
  if (app_cartesian_trajectory.active)
  {
    return APP_SERVO_MOTION_SOURCE_XYZ;
  }

  if (app_servo_trajectory.active)
  {
    return app_servo_trajectory.source;
  }

  if (app_servo_demo.active)
  {
    return APP_SERVO_MOTION_SOURCE_DEMO;
  }

  return APP_SERVO_MOTION_SOURCE_IDLE;
}

static void App_StopActiveMotionAtCurrentPose(void)
{
  RobotArmPose current_pose;
  App_ServoPulseSet current_pulses = App_ServoGetCurrentPulseSet();
  uint32_t now = HAL_GetTick();

  app_servo_demo.active = false;
  app_servo_demo.waiting_between_steps = false;
  app_servo_demo.step_index = 0U;
  app_servo_demo.wait_start_tick = 0U;

  app_servo_trajectory.active = false;
  app_servo_trajectory.source = APP_SERVO_MOTION_SOURCE_IDLE;
  app_servo_trajectory.start_tick = now;
  app_servo_trajectory.duration_ms = 0U;
  app_servo_trajectory.last_update_tick = now;
  app_servo_trajectory.start_pulses = current_pulses;
  app_servo_trajectory.target_pulses = current_pulses;

  app_cartesian_trajectory.active = false;
  app_cartesian_trajectory.start_tick = now;
  app_cartesian_trajectory.duration_ms = 0U;
  app_cartesian_trajectory.last_update_tick = now;
  app_cartesian_trajectory.target_pulses = current_pulses;
  app_cartesian_trajectory.elbow_mode = ROBOT_ARM_ELBOW_MODE_DOWN;
  if (App_RobotArmEstimateCurrentPose(&current_pose))
  {
    app_cartesian_trajectory.start_position_mm.x_mm = current_pose.x_mm;
    app_cartesian_trajectory.start_position_mm.y_mm = current_pose.y_mm;
    app_cartesian_trajectory.start_position_mm.z_mm = current_pose.z_mm;
    app_cartesian_trajectory.target_position_mm = app_cartesian_trajectory.start_position_mm;
  }
  else
  {
    app_cartesian_trajectory.start_position_mm.x_mm = 0.0f;
    app_cartesian_trajectory.start_position_mm.y_mm = 0.0f;
    app_cartesian_trajectory.start_position_mm.z_mm = 0.0f;
    app_cartesian_trajectory.target_position_mm = app_cartesian_trajectory.start_position_mm;
  }
}

static void App_TryRecoverVl53l0x(uint32_t now)
{
  if (phase1_vl53l0x_ready)
  {
    return;
  }

  if ((now - app_last_vl53l0x_recovery_tick) < APP_VL53L0X_RECOVERY_INTERVAL_MS)
  {
    return;
  }

  app_last_vl53l0x_recovery_tick = now;
  if (!Phase1_IsDeviceReady(VL53L0X_I2C_ADDR_DEFAULT))
  {
    return;
  }

  if (!Phase1_Vl53l0xInit(VL53L0X_I2C_ADDR_DEFAULT))
  {
    printf("[vl53l0x] recovery init failed\r\n");
    return;
  }

  printf("[vl53l0x] recovery init ok, single-shot ranging ready\r\n");
  Phase1_LogVl53l0xRange();
  App_LogRobotArmStatus();
}

static void App_PublishObstacleStatus(const char *state, uint16_t range_mm, const char *device)
{
  printf("[status] obstacle=%s range=%u mm threshold=%u mm clear=%u mm device=%s\r\n",
         state,
         range_mm,
         APP_OBSTACLE_STOP_DISTANCE_MM,
         APP_OBSTACLE_CLEAR_DISTANCE_MM,
         device);
}

static void App_UpdateObstacleMonitor(uint32_t now)
{
  Phase1_Vl53l0xMeasurement measurement;
  const char *device_status;
  App_ServoMotionSource interrupted_motion;
  bool obstacle_now;
  bool clear_now;
  bool was_detected;
  bool motion_active;
  const char *publish_state;

  if (phase1_vl53l0x_ready && app_obstacle_detected && App_HasActiveMotion())
  {
    interrupted_motion = App_GetActiveMotionSource();
    App_StopActiveMotionAtCurrentPose();
    printf("[safety] obstacle detected range=%u mm threshold=%u mm clear=%u mm device=%s motion=%s action=stop\r\n",
           app_obstacle_last_measurement.range_mm,
           APP_OBSTACLE_STOP_DISTANCE_MM,
           APP_OBSTACLE_CLEAR_DISTANCE_MM,
           app_obstacle_has_last_measurement ?
             Phase1_Vl53l0xDeviceCodeString(app_obstacle_last_measurement.range_status_code) :
             "latched",
           App_ServoMotionSourceName(interrupted_motion));
    App_LogRobotArmStatus();
  }

  if ((now - app_last_obstacle_monitor_tick) < APP_OBSTACLE_MONITOR_INTERVAL_MS)
  {
    return;
  }
  app_last_obstacle_monitor_tick = now;

  if (!phase1_vl53l0x_ready)
  {
    App_PublishObstacleStatus("disabled", 0U, "sensor-not-ready");
    return;
  }

  if (!Phase1_Vl53l0xReadMeasurementInternal(VL53L0X_I2C_ADDR_DEFAULT, &measurement, false))
  {
    phase1_vl53l0x_ready = false;
    App_PublishObstacleStatus("disabled", 0U, "read-failed");
    return;
  }

  if (measurement.range_status_code != VL53L0X_DEVICEERROR_RANGECOMPLETE)
  {
    /* Sporadic device-error reads (TCC, MSRC, sigma-threshold...) report a
       spec-defined ~20 mm clip that is not a real distance. Keep the last
       valid range visible to the GUI instead of flickering. */
    if (app_obstacle_has_last_measurement)
    {
      App_PublishObstacleStatus(
          app_obstacle_detected ? "detected" : "clear",
          app_obstacle_last_measurement.range_mm,
          Phase1_Vl53l0xDeviceCodeString(app_obstacle_last_measurement.range_status_code));
    }
    else
    {
      App_PublishObstacleStatus("unknown",
                                0U,
                                Phase1_Vl53l0xDeviceCodeString(measurement.range_status_code));
    }
    return;
  }

  app_obstacle_last_measurement = measurement;
  app_obstacle_has_last_measurement = true;
  was_detected = app_obstacle_detected;
  motion_active = App_HasActiveMotion();
  obstacle_now = measurement.range_mm <= APP_OBSTACLE_STOP_DISTANCE_MM;
  clear_now = measurement.range_mm > APP_OBSTACLE_CLEAR_DISTANCE_MM;
  device_status = Phase1_Vl53l0xDeviceCodeString(measurement.range_status_code);

  if (obstacle_now)
  {
    app_obstacle_detected = true;
    if (motion_active)
    {
      interrupted_motion = App_GetActiveMotionSource();
      App_StopActiveMotionAtCurrentPose();
      printf("[safety] obstacle detected range=%u mm threshold=%u mm clear=%u mm device=%s motion=%s action=stop\r\n",
             measurement.range_mm,
             APP_OBSTACLE_STOP_DISTANCE_MM,
             APP_OBSTACLE_CLEAR_DISTANCE_MM,
             device_status,
             App_ServoMotionSourceName(interrupted_motion));
      App_LogRobotArmStatus();
    }
    else if (!was_detected)
    {
      printf("[safety] obstacle detected range=%u mm threshold=%u mm clear=%u mm device=%s\r\n",
             measurement.range_mm,
             APP_OBSTACLE_STOP_DISTANCE_MM,
             APP_OBSTACLE_CLEAR_DISTANCE_MM,
             device_status);
      App_LogRobotArmStatus();
    }
  }
  else if (was_detected && clear_now)
  {
    app_obstacle_detected = false;
    printf("[safety] obstacle cleared range=%u mm threshold=%u mm clear=%u mm device=%s\r\n",
           measurement.range_mm,
           APP_OBSTACLE_STOP_DISTANCE_MM,
           APP_OBSTACLE_CLEAR_DISTANCE_MM,
           device_status);
    App_LogRobotArmStatus();
  }

  publish_state = app_obstacle_detected ? "detected" : (clear_now ? "clear" : "unknown");
  App_PublishObstacleStatus(publish_state, measurement.range_mm, device_status);
}

static void App_PublishImuStatus(const char *state, int pitch_deg, int roll_deg, uint32_t gyro_mag_mdps, int temp_dc)
{
  printf("[status] imu=%s pitch=%d roll=%d gyro_mag=%lu temp_dc=%d\r\n",
         state,
         pitch_deg,
         roll_deg,
         (unsigned long)gyro_mag_mdps,
         temp_dc);
}

static void App_UpdateImuMonitor(uint32_t now)
{
  Phase1_Mpu6050RawSample sample;
  long ax_mg;
  long ay_mg;
  long az_mg;
  long gx_mdps;
  long gy_mdps;
  long gz_mdps;
  long temp_centi_c;
  int temp_dc;
  float ax_f;
  float ay_f;
  float az_f;
  float pitch_rad;
  float roll_rad;
  int pitch_deg;
  int roll_deg;
  int pitch_abs;
  int roll_abs;
  uint64_t gyro_sq;
  uint32_t gyro_mag;
  const char *state;

  if ((now - app_last_imu_monitor_tick) < APP_IMU_MONITOR_INTERVAL_MS)
  {
    return;
  }
  app_last_imu_monitor_tick = now;

  if (!phase1_mpu6050_ready)
  {
    App_PublishImuStatus("disabled", 0, 0, 0U, 0);
    return;
  }

  if (!Phase1_Mpu6050ReadRawSample(phase1_mpu6050_address, &sample))
  {
    App_PublishImuStatus("disabled", 0, 0, 0U, 0);
    return;
  }

  ax_mg = ((long)sample.accel_x * 1000L) / PHASE1_MPU6050_ACCEL_LSB_PER_G;
  ay_mg = ((long)sample.accel_y * 1000L) / PHASE1_MPU6050_ACCEL_LSB_PER_G;
  az_mg = ((long)sample.accel_z * 1000L) / PHASE1_MPU6050_ACCEL_LSB_PER_G;
  gx_mdps = ((long)sample.gyro_x * 1000L) / PHASE1_MPU6050_GYRO_LSB_PER_DPS;
  gy_mdps = ((long)sample.gyro_y * 1000L) / PHASE1_MPU6050_GYRO_LSB_PER_DPS;
  gz_mdps = ((long)sample.gyro_z * 1000L) / PHASE1_MPU6050_GYRO_LSB_PER_DPS;
  temp_centi_c = (((long)sample.temperature_raw) * 100L) / 340L + 3653L;
  temp_dc = (int)(temp_centi_c >= 0 ? (temp_centi_c + 5L) / 10L : (temp_centi_c - 5L) / 10L);

  ax_f = (float)ax_mg;
  ay_f = (float)ay_mg;
  az_f = (float)az_mg;
  pitch_rad = atan2f(-ax_f, sqrtf((ay_f * ay_f) + (az_f * az_f)));
  roll_rad = atan2f(ay_f, az_f);
  pitch_deg = (int)(pitch_rad * (180.0f / 3.14159265f));
  roll_deg = (int)(roll_rad * (180.0f / 3.14159265f));

  gyro_sq = (uint64_t)((long long)gx_mdps * (long long)gx_mdps +
                       (long long)gy_mdps * (long long)gy_mdps +
                       (long long)gz_mdps * (long long)gz_mdps);
  gyro_mag = (uint32_t)sqrtf((float)gyro_sq);

  pitch_abs = pitch_deg < 0 ? -pitch_deg : pitch_deg;
  roll_abs = roll_deg < 0 ? -roll_deg : roll_deg;

  if (gyro_mag > APP_IMU_SHAKING_MDPS)
  {
    state = "shaking";
  }
  else if (pitch_abs > APP_IMU_TILT_DEG || roll_abs > APP_IMU_TILT_DEG)
  {
    state = "tilted";
  }
  else if (pitch_abs <= APP_IMU_LEVEL_DEG && roll_abs <= APP_IMU_LEVEL_DEG)
  {
    state = "level";
  }
  else
  {
    state = "tilted";
  }

  App_PublishImuStatus(state, pitch_deg, roll_deg, gyro_mag, temp_dc);
}

static float App_MinimumJerkBlend(uint32_t elapsed_ms, uint32_t duration_ms)
{
  float normalized_time;
  float normalized_time_squared;
  float normalized_time_cubed;

  if (duration_ms == 0U || elapsed_ms >= duration_ms)
  {
    return 1.0f;
  }

  normalized_time = (float)elapsed_ms / (float)duration_ms;
  normalized_time_squared = normalized_time * normalized_time;
  normalized_time_cubed = normalized_time_squared * normalized_time;

  return (10.0f * normalized_time_cubed)
         - (15.0f * normalized_time_cubed * normalized_time)
         + (6.0f * normalized_time_cubed * normalized_time_squared);
}

static uint16_t App_InterpolatePulse(uint16_t start_pulse_us, uint16_t target_pulse_us, float blend)
{
  float interpolated_pulse_us = (float)start_pulse_us
                                + ((float)((int32_t)target_pulse_us - (int32_t)start_pulse_us) * blend);

  if (interpolated_pulse_us <= 0.0f)
  {
    return 0U;
  }

  if (interpolated_pulse_us >= 65535.0f)
  {
    return 0xFFFFU;
  }

  return (uint16_t)(interpolated_pulse_us + 0.5f);
}

static RobotArmVector3 App_InterpolateCartesianTarget(const RobotArmVector3 *start_position,
                                                      const RobotArmVector3 *target_position,
                                                      float blend)
{
  RobotArmVector3 interpolated_position = { 0.0f, 0.0f, 0.0f };

  if (start_position == NULL || target_position == NULL)
  {
    return interpolated_position;
  }

  interpolated_position.x_mm = start_position->x_mm + ((target_position->x_mm - start_position->x_mm) * blend);
  interpolated_position.y_mm = start_position->y_mm + ((target_position->y_mm - start_position->y_mm) * blend);
  interpolated_position.z_mm = start_position->z_mm + ((target_position->z_mm - start_position->z_mm) * blend);
  return interpolated_position;
}

static bool App_ServoTrajectoryStart(const App_ServoPulseSet *target_pulses,
                                     App_ServoMotionSource source,
                                     uint32_t *duration_ms_out)
{
  App_ServoPulseSet current_pulses;
  App_ServoPulseSet clamped_target_pulses;
  uint32_t duration_ms;
  uint32_t now = HAL_GetTick();

  if (target_pulses == NULL)
  {
    return false;
  }

  current_pulses = App_ServoGetCurrentPulseSet();
  clamped_target_pulses = App_ServoClampPulseSet(target_pulses);
  duration_ms = App_ServoTrajectoryComputeDurationMs(&current_pulses, &clamped_target_pulses);

  if (source != APP_SERVO_MOTION_SOURCE_DEMO)
  {
    app_servo_demo.active = false;
    app_servo_demo.waiting_between_steps = false;
  }

  app_cartesian_trajectory.active = false;

  app_servo_trajectory.active = (duration_ms > 0U);
  app_servo_trajectory.source = app_servo_trajectory.active ? source : APP_SERVO_MOTION_SOURCE_IDLE;
  app_servo_trajectory.start_tick = now;
  app_servo_trajectory.duration_ms = duration_ms;
  app_servo_trajectory.last_update_tick = now;
  app_servo_trajectory.start_pulses = current_pulses;
  app_servo_trajectory.target_pulses = clamped_target_pulses;

  if (duration_ms_out != NULL)
  {
    *duration_ms_out = duration_ms;
  }

  if (duration_ms == 0U)
  {
    App_ServoApplyPulseSet(&clamped_target_pulses);
  }

  return true;
}

static void App_ServoTrajectoryUpdate(uint32_t now)
{
  App_ServoPulseSet interpolated_pulses;
  uint32_t elapsed_ms;
  float blend;

  if (!app_servo_trajectory.active)
  {
    return;
  }

  elapsed_ms = now - app_servo_trajectory.start_tick;
  if (elapsed_ms >= app_servo_trajectory.duration_ms)
  {
    App_ServoApplyPulseSet(&app_servo_trajectory.target_pulses);
    app_servo_trajectory.active = false;
    app_servo_trajectory.source = APP_SERVO_MOTION_SOURCE_IDLE;
    app_servo_trajectory.last_update_tick = now;
    return;
  }

  if ((now - app_servo_trajectory.last_update_tick) < APP_SERVO_TRAJECTORY_UPDATE_INTERVAL_MS)
  {
    return;
  }

  blend = App_MinimumJerkBlend(elapsed_ms, app_servo_trajectory.duration_ms);
  interpolated_pulses.clamp_pulse_us = App_InterpolatePulse(app_servo_trajectory.start_pulses.clamp_pulse_us,
                                                            app_servo_trajectory.target_pulses.clamp_pulse_us,
                                                            blend);
  interpolated_pulses.fore_aft_pulse_us = App_InterpolatePulse(app_servo_trajectory.start_pulses.fore_aft_pulse_us,
                                                               app_servo_trajectory.target_pulses.fore_aft_pulse_us,
                                                               blend);
  interpolated_pulses.gripper_lift_pulse_us = App_InterpolatePulse(app_servo_trajectory.start_pulses.gripper_lift_pulse_us,
                                                                   app_servo_trajectory.target_pulses.gripper_lift_pulse_us,
                                                                   blend);
  interpolated_pulses.left_right_pulse_us = App_InterpolatePulse(app_servo_trajectory.start_pulses.left_right_pulse_us,
                                                                 app_servo_trajectory.target_pulses.left_right_pulse_us,
                                                                 blend);
  App_ServoApplyPulseSet(&interpolated_pulses);
  app_servo_trajectory.last_update_tick = now;
}

static bool App_CartesianTrajectoryStart(const RobotArmVector3 *target_position,
                                         const RobotArmJointAngles *target_joint_solution,
                                         RobotArmElbowMode elbow_mode,
                                         uint32_t *duration_ms_out)
{
  RobotArmPose current_pose;
  RobotArmVector3 start_position_mm;
  App_ServoPulseSet current_pulses;
  App_ServoPulseSet target_pulses;
  uint32_t duration_ms;
  uint32_t now = HAL_GetTick();

  if (target_position == NULL || target_joint_solution == NULL)
  {
    return false;
  }

  if (!App_RobotArmEstimateCurrentPose(&current_pose))
  {
    return false;
  }

  current_pulses = App_ServoGetCurrentPulseSet();
  target_pulses = App_RobotArmJointAnglesToPulseSet(target_joint_solution);
  start_position_mm.x_mm = current_pose.x_mm;
  start_position_mm.y_mm = current_pose.y_mm;
  start_position_mm.z_mm = current_pose.z_mm;
  duration_ms = App_CartesianTrajectoryComputeDurationMs(&start_position_mm,
                                                         target_position,
                                                         &current_pulses,
                                                         &target_pulses);

  app_servo_demo.active = false;
  app_servo_demo.waiting_between_steps = false;
  app_servo_trajectory.active = false;
  app_servo_trajectory.source = APP_SERVO_MOTION_SOURCE_IDLE;

  app_cartesian_trajectory.start_position_mm = start_position_mm;
  app_cartesian_trajectory.target_position_mm = *target_position;
  app_cartesian_trajectory.target_pulses = target_pulses;
  app_cartesian_trajectory.elbow_mode = elbow_mode;
  app_cartesian_trajectory.start_tick = now;
  app_cartesian_trajectory.duration_ms = duration_ms;
  app_cartesian_trajectory.last_update_tick = now;
  app_cartesian_trajectory.active = (duration_ms > 0U);

  if (duration_ms_out != NULL)
  {
    *duration_ms_out = duration_ms;
  }

  if (duration_ms == 0U)
  {
    App_ServoApplyPulseSet(&target_pulses);
  }

  return true;
}

static void App_CartesianTrajectoryUpdate(uint32_t now)
{
  RobotArmGeometry geometry = App_RobotArmGetGeometry();
  RobotArmJointLimits joint_limits = App_RobotArmGetSafeJointLimits();
  RobotArmJointAngles joint_solution;
  RobotArmIkStatus status;
  RobotArmVector3 waypoint;
  App_ServoPulseSet waypoint_pulses;
  uint32_t elapsed_ms;
  float blend;

  if (!app_cartesian_trajectory.active)
  {
    return;
  }

  elapsed_ms = now - app_cartesian_trajectory.start_tick;
  if (elapsed_ms >= app_cartesian_trajectory.duration_ms)
  {
    App_ServoApplyPulseSet(&app_cartesian_trajectory.target_pulses);
    app_cartesian_trajectory.active = false;
    app_cartesian_trajectory.last_update_tick = now;
    return;
  }

  if ((now - app_cartesian_trajectory.last_update_tick) < APP_SERVO_TRAJECTORY_UPDATE_INTERVAL_MS)
  {
    return;
  }

  blend = App_MinimumJerkBlend(elapsed_ms, app_cartesian_trajectory.duration_ms);
  waypoint = App_InterpolateCartesianTarget(&app_cartesian_trajectory.start_position_mm,
                                            &app_cartesian_trajectory.target_position_mm,
                                            blend);
  status = RobotArmKinematics_InversePosition(&geometry,
                                              &waypoint,
                                              app_cartesian_trajectory.elbow_mode,
                                              &joint_solution);
  if (status != ROBOT_ARM_IK_STATUS_OK ||
      !RobotArmKinematics_AreJointAnglesWithinLimits(&joint_solution, &joint_limits))
  {
    if (status == ROBOT_ARM_IK_STATUS_OK)
    {
      status = ROBOT_ARM_IK_STATUS_OUT_OF_REACH;
    }

    printf("[traj_xyz] cartesian waypoint x=%ld y=%ld z=%ld mm fallback_to_joint_space status=%s\r\n",
           App_RoundFloatToLong(waypoint.x_mm),
           App_RoundFloatToLong(waypoint.y_mm),
           App_RoundFloatToLong(waypoint.z_mm),
           RobotArmKinematics_IkStatusString(status));
    (void)App_ServoTrajectoryStart(&app_cartesian_trajectory.target_pulses, APP_SERVO_MOTION_SOURCE_XYZ, NULL);
    return;
  }

  waypoint_pulses = App_RobotArmJointAnglesToPulseSet(&joint_solution);
  App_ServoApplyPulseSet(&waypoint_pulses);
  app_cartesian_trajectory.last_update_tick = now;
}

static void App_ClampServoApplyPulse(uint16_t pulse_width_us)
{
  pulse_width_us = App_ClampPulseToMeasuredSafeWindow("clamp",
                                                      pulse_width_us,
                                                      APP_CLAMP_SERVO_OPEN_PULSE_US,
                                                      APP_CLAMP_SERVO_CLOSE_PULSE_US);
  if (pulse_width_us == APP_CLAMP_SERVO_OPEN_PULSE_US)
  {
    app_clamp_servo_state = APP_CLAMP_SERVO_STATE_OPEN;
  }
  else if (pulse_width_us == APP_CLAMP_SERVO_CLOSE_PULSE_US)
  {
    app_clamp_servo_state = APP_CLAMP_SERVO_STATE_CLOSE;
  }
  else
  {
    app_clamp_servo_state = APP_CLAMP_SERVO_STATE_SAFE;
  }

  app_clamp_servo_current_pulse_us = pulse_width_us;
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pulse_width_us);
}

static void App_ForeAftServoApplyPulse(uint16_t pulse_width_us)
{
  pulse_width_us = App_ClampPulseToMeasuredSafeWindow("fore_aft",
                                                      pulse_width_us,
                                                      APP_FORE_AFT_SERVO_AFT_PULSE_US,
                                                      APP_FORE_AFT_SERVO_FORE_PULSE_US);
  app_fore_aft_servo_current_pulse_us = pulse_width_us;
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, pulse_width_us);
}

static void App_GripperLiftServoApplyPulse(uint16_t pulse_width_us)
{
  pulse_width_us = App_ClampPulseToMeasuredSafeWindow("gripper_lift",
                                                      pulse_width_us,
                                                      APP_GRIPPER_LIFT_SERVO_DOWN_PULSE_US,
                                                      APP_GRIPPER_LIFT_SERVO_UP_PULSE_US);
  app_gripper_lift_servo_current_pulse_us = pulse_width_us;
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, pulse_width_us);
}

static void App_LeftRightServoApplyPulse(uint16_t pulse_width_us)
{
  pulse_width_us = App_ClampPulseToMeasuredSafeWindow("left_right",
                                                      pulse_width_us,
                                                      APP_LEFT_RIGHT_SERVO_RIGHT_PULSE_US,
                                                      APP_LEFT_RIGHT_SERVO_LEFT_PULSE_US);
  if (pulse_width_us == APP_LEFT_RIGHT_SERVO_LEFT_PULSE_US)
  {
    app_left_right_servo_state = APP_LEFT_RIGHT_SERVO_STATE_LEFT;
  }
  else if (pulse_width_us == APP_LEFT_RIGHT_SERVO_RIGHT_PULSE_US)
  {
    app_left_right_servo_state = APP_LEFT_RIGHT_SERVO_STATE_RIGHT;
  }
  else if (pulse_width_us == APP_LEFT_RIGHT_SERVO_CENTER_PULSE_US)
  {
    app_left_right_servo_state = APP_LEFT_RIGHT_SERVO_STATE_CENTER;
  }
  else
  {
    app_left_right_servo_state = APP_LEFT_RIGHT_SERVO_STATE_CUSTOM;
  }

  app_left_right_servo_current_pulse_us = pulse_width_us;
  __HAL_TIM_SET_COMPARE(&htim16, TIM_CHANNEL_1, pulse_width_us);
}

static const char *App_LeftRightServoStateName(App_LeftRightServoState state)
{
  switch (state)
  {
    case APP_LEFT_RIGHT_SERVO_STATE_LEFT:
      return "left";

    case APP_LEFT_RIGHT_SERVO_STATE_RIGHT:
      return "right";

    case APP_LEFT_RIGHT_SERVO_STATE_CUSTOM:
      return "custom";

    case APP_LEFT_RIGHT_SERVO_STATE_CENTER:
    default:
      return "center";
  }
}

static bool App_ServoDemoQueueStep(uint8_t step_index)
{
  uint32_t duration_ms;
  size_t step_count = sizeof(app_servo_demo_steps) / sizeof(app_servo_demo_steps[0]);

  if ((size_t)step_index >= step_count)
  {
    return false;
  }

  if (!App_ServoTrajectoryStart(&app_servo_demo_steps[step_index], APP_SERVO_MOTION_SOURCE_DEMO, &duration_ms))
  {
    return false;
  }

  printf("[servo_demo] step %u/%u %s duration=%lu ms\r\n",
         (unsigned int)(step_index + 1U),
         (unsigned int)step_count,
         app_servo_demo_step_labels[step_index],
         (unsigned long)duration_ms);
  return true;
}

static void App_RunServoFullRangeDemo(void)
{
  app_servo_demo.active = true;
  app_servo_demo.waiting_between_steps = false;
  app_servo_demo.step_index = 0U;
  app_servo_demo.wait_start_tick = 0U;

  printf("[servo_demo] start full-range sweep left_right-first minimum-jerk\r\n");
  (void)App_ServoDemoQueueStep(0U);
}

static void App_UpdateServoDemo(uint32_t now)
{
  size_t step_count = sizeof(app_servo_demo_steps) / sizeof(app_servo_demo_steps[0]);

  if (!app_servo_demo.active)
  {
    return;
  }

  if (app_servo_trajectory.active)
  {
    return;
  }

  if (!app_servo_demo.waiting_between_steps)
  {
    app_servo_demo.waiting_between_steps = true;
    app_servo_demo.wait_start_tick = now;
    return;
  }

  if ((now - app_servo_demo.wait_start_tick) < APP_SERVO_DEMO_STEP_HOLD_MS)
  {
    return;
  }

  app_servo_demo.waiting_between_steps = false;
  ++app_servo_demo.step_index;
  if ((size_t)app_servo_demo.step_index >= step_count)
  {
    app_servo_demo.active = false;
    printf("[servo_demo] complete\r\n");
    return;
  }

  (void)App_ServoDemoQueueStep(app_servo_demo.step_index);
}

static float App_RadiansToDegrees(float radians)
{
  return radians * (180.0f / APP_ROBOT_ARM_PI);
}

static long App_RoundFloatToLong(float value)
{
  return (long)(value >= 0.0f ? (value + 0.5f) : (value - 0.5f));
}

static RobotArmGeometry App_RobotArmGetGeometry(void)
{
  RobotArmGeometry geometry = {
    APP_ROBOT_ARM_BASE_HEIGHT_MM,
    APP_ROBOT_ARM_SHOULDER_LENGTH_MM,
    APP_ROBOT_ARM_FOREARM_LENGTH_MM,
    APP_ROBOT_ARM_TOOL_LENGTH_MM
  };

  return geometry;
}

static RobotArmJointLimits App_RobotArmGetSafeJointLimits(void)
{
  RobotArmJointLimits joint_limits = {
    { App_DegreesToRadians(APP_LEFT_RIGHT_SAFE_MIN_DEG), App_DegreesToRadians(APP_LEFT_RIGHT_SAFE_MAX_DEG) },
    { App_DegreesToRadians(APP_FORE_AFT_SAFE_MIN_DEG), App_DegreesToRadians(APP_FORE_AFT_SAFE_MAX_DEG) },
    { App_DegreesToRadians(APP_GRIPPER_LIFT_SAFE_MIN_DEG), App_DegreesToRadians(APP_GRIPPER_LIFT_SAFE_MAX_DEG) }
  };

  return joint_limits;
}

static uint16_t App_MapAngleRadiansToPulse(float angle_rad,
                                           float minimum_angle_rad,
                                           float maximum_angle_rad,
                                           uint16_t minimum_pulse_us,
                                           uint16_t maximum_pulse_us)
{
  float ratio;
  float pulse_width_us;

  if (maximum_angle_rad <= minimum_angle_rad)
  {
    return minimum_pulse_us;
  }

  ratio = (angle_rad - minimum_angle_rad) / (maximum_angle_rad - minimum_angle_rad);
  if (ratio < 0.0f)
  {
    ratio = 0.0f;
  }
  else if (ratio > 1.0f)
  {
    ratio = 1.0f;
  }

  pulse_width_us = (float)minimum_pulse_us + (ratio * (float)(maximum_pulse_us - minimum_pulse_us));
  return (uint16_t)(pulse_width_us + 0.5f);
}

static float App_MapPulseToAngleRadians(uint16_t pulse_width_us,
                                        uint16_t minimum_pulse_us,
                                        uint16_t maximum_pulse_us,
                                        float minimum_angle_rad,
                                        float maximum_angle_rad)
{
  float ratio;

  if (maximum_pulse_us <= minimum_pulse_us)
  {
    return minimum_angle_rad;
  }

  pulse_width_us = App_ClampPulseToRange(pulse_width_us, minimum_pulse_us, maximum_pulse_us);
  ratio = (float)(pulse_width_us - minimum_pulse_us) / (float)(maximum_pulse_us - minimum_pulse_us);
  return minimum_angle_rad + (ratio * (maximum_angle_rad - minimum_angle_rad));
}

static RobotArmJointAngles App_RobotArmGetCurrentJointAngles(void)
{
  RobotArmJointAngles joint_angles;
  RobotArmJointLimits joint_limits = App_RobotArmGetSafeJointLimits();

  joint_angles.base_yaw_rad = App_MapPulseToAngleRadians(app_left_right_servo_current_pulse_us,
                                                         APP_LEFT_RIGHT_SERVO_RIGHT_PULSE_US,
                                                         APP_LEFT_RIGHT_SERVO_LEFT_PULSE_US,
                                                         joint_limits.base_yaw.minimum_rad,
                                                         joint_limits.base_yaw.maximum_rad);
  joint_angles.shoulder_pitch_rad = App_MapPulseToAngleRadians(app_fore_aft_servo_current_pulse_us,
                                                               APP_FORE_AFT_SERVO_AFT_PULSE_US,
                                                               APP_FORE_AFT_SERVO_FORE_PULSE_US,
                                                               joint_limits.shoulder_pitch.minimum_rad,
                                                               joint_limits.shoulder_pitch.maximum_rad);
  joint_angles.gripper_lift_pitch_rad = App_MapPulseToAngleRadians(app_gripper_lift_servo_current_pulse_us,
                                                                   APP_GRIPPER_LIFT_SERVO_DOWN_PULSE_US,
                                                                   APP_GRIPPER_LIFT_SERVO_UP_PULSE_US,
                                                                   joint_limits.gripper_lift_pitch.minimum_rad,
                                                                   joint_limits.gripper_lift_pitch.maximum_rad);

  return joint_angles;
}

static bool App_RobotArmEstimateCurrentPose(RobotArmPose *pose)
{
  RobotArmGeometry geometry = App_RobotArmGetGeometry();
  RobotArmJointAngles joint_angles = App_RobotArmGetCurrentJointAngles();

  return RobotArmKinematics_Forward(&geometry, &joint_angles, pose);
}

static RobotArmIkStatus App_RobotArmSolveSafeIk(const RobotArmVector3 *target_position,
                                                RobotArmJointAngles *joint_solution,
                                                RobotArmElbowMode *selected_elbow_mode)
{
  RobotArmGeometry geometry = App_RobotArmGetGeometry();
  RobotArmJointLimits joint_limits = App_RobotArmGetSafeJointLimits();
  RobotArmJointAngles candidate_solution;
  RobotArmIkStatus down_status;
  RobotArmIkStatus up_status;

  if (target_position == NULL || joint_solution == NULL)
  {
    return ROBOT_ARM_IK_STATUS_INVALID_ARGUMENT;
  }

  if (target_position->z_mm < APP_SAFE_WORKSPACE_MIN_Z_MM)
  {
    return ROBOT_ARM_IK_STATUS_OUT_OF_REACH;
  }

  down_status = RobotArmKinematics_InversePosition(&geometry,
                                                   target_position,
                                                   ROBOT_ARM_ELBOW_MODE_DOWN,
                                                   &candidate_solution);
  if (down_status == ROBOT_ARM_IK_STATUS_OK &&
      RobotArmKinematics_AreJointAnglesWithinLimits(&candidate_solution, &joint_limits))
  {
    *joint_solution = candidate_solution;
    if (selected_elbow_mode != NULL)
    {
      *selected_elbow_mode = ROBOT_ARM_ELBOW_MODE_DOWN;
    }
    return ROBOT_ARM_IK_STATUS_OK;
  }

  up_status = RobotArmKinematics_InversePosition(&geometry,
                                                 target_position,
                                                 ROBOT_ARM_ELBOW_MODE_UP,
                                                 &candidate_solution);
  if (up_status == ROBOT_ARM_IK_STATUS_OK &&
      RobotArmKinematics_AreJointAnglesWithinLimits(&candidate_solution, &joint_limits))
  {
    *joint_solution = candidate_solution;
    if (selected_elbow_mode != NULL)
    {
      *selected_elbow_mode = ROBOT_ARM_ELBOW_MODE_UP;
    }
    return ROBOT_ARM_IK_STATUS_OK;
  }

  if (down_status == ROBOT_ARM_IK_STATUS_OK || up_status == ROBOT_ARM_IK_STATUS_OK)
  {
    return ROBOT_ARM_IK_STATUS_OUT_OF_REACH;
  }

  return down_status != ROBOT_ARM_IK_STATUS_OK ? down_status : up_status;
}

static App_ServoPulseSet App_RobotArmJointAnglesToPulseSet(const RobotArmJointAngles *joint_angles)
{
  RobotArmJointLimits joint_limits = App_RobotArmGetSafeJointLimits();
  App_ServoPulseSet target_pulses = App_ServoGetCurrentPulseSet();

  if (joint_angles == NULL)
  {
    return target_pulses;
  }

  target_pulses.left_right_pulse_us = App_MapAngleRadiansToPulse(joint_angles->base_yaw_rad,
                                                                 joint_limits.base_yaw.minimum_rad,
                                                                 joint_limits.base_yaw.maximum_rad,
                                                                 APP_LEFT_RIGHT_SERVO_RIGHT_PULSE_US,
                                                                 APP_LEFT_RIGHT_SERVO_LEFT_PULSE_US);
  target_pulses.fore_aft_pulse_us = App_MapAngleRadiansToPulse(joint_angles->shoulder_pitch_rad,
                                                               joint_limits.shoulder_pitch.minimum_rad,
                                                               joint_limits.shoulder_pitch.maximum_rad,
                                                               APP_FORE_AFT_SERVO_AFT_PULSE_US,
                                                               APP_FORE_AFT_SERVO_FORE_PULSE_US);
  target_pulses.gripper_lift_pulse_us = App_MapAngleRadiansToPulse(joint_angles->gripper_lift_pitch_rad,
                                                                   joint_limits.gripper_lift_pitch.minimum_rad,
                                                                   joint_limits.gripper_lift_pitch.maximum_rad,
                                                                   APP_GRIPPER_LIFT_SERVO_DOWN_PULSE_US,
                                                                   APP_GRIPPER_LIFT_SERVO_UP_PULSE_US);
  return target_pulses;
}

static void App_RobotArmApplyJointAngles(const RobotArmJointAngles *joint_angles)
{
  App_ServoPulseSet target_pulses;

  if (joint_angles == NULL)
  {
    return;
  }

  target_pulses = App_RobotArmJointAnglesToPulseSet(joint_angles);

  (void)App_ServoTrajectoryStart(&target_pulses, APP_SERVO_MOTION_SOURCE_XYZ, NULL);
}

static bool App_RobotArmMoveToTarget(const RobotArmVector3 *target_position)
{
  RobotArmGeometry geometry = App_RobotArmGetGeometry();
  RobotArmJointAngles joint_solution;
  RobotArmPose solved_pose;
  RobotArmIkStatus status;
  RobotArmElbowMode elbow_mode = ROBOT_ARM_ELBOW_MODE_DOWN;
  App_ServoPulseSet target_pulses;
  uint32_t duration_ms = 0U;
  bool cartesian_started;

  if (target_position == NULL)
  {
    return false;
  }

  status = App_RobotArmSolveSafeIk(target_position, &joint_solution, &elbow_mode);
  if (status != ROBOT_ARM_IK_STATUS_OK)
  {
    printf("[cmd_xyz] target=%ld,%ld,%ld mm rejected status=%s\r\n",
           (long)target_position->x_mm,
           (long)target_position->y_mm,
           (long)target_position->z_mm,
           RobotArmKinematics_IkStatusString(status));
    return false;
  }

  if (!RobotArmKinematics_Forward(&geometry, &joint_solution, &solved_pose))
  {
    printf("[cmd_xyz] forward verification failed\r\n");
    return false;
  }

  target_pulses = App_RobotArmJointAnglesToPulseSet(&joint_solution);
  cartesian_started = App_CartesianTrajectoryStart(target_position, &joint_solution, elbow_mode, &duration_ms);
  if (cartesian_started)
  {
    printf("[cmd_xyz] cartesian queued start=%ld,%ld,%ld target=%ld,%ld,%ld mm elbow=%s q_deg yaw=%ld shoulder=%ld gripper=%ld target_pulses fore_aft=%u gripper_lift=%u left_right=%u duration=%lu ms\r\n",
           App_RoundFloatToLong(app_cartesian_trajectory.start_position_mm.x_mm),
           App_RoundFloatToLong(app_cartesian_trajectory.start_position_mm.y_mm),
           App_RoundFloatToLong(app_cartesian_trajectory.start_position_mm.z_mm),
           (long)target_position->x_mm,
           (long)target_position->y_mm,
           (long)target_position->z_mm,
           App_ElbowModeName(elbow_mode),
           App_RoundFloatToLong(App_RadiansToDegrees(joint_solution.base_yaw_rad)),
           App_RoundFloatToLong(App_RadiansToDegrees(joint_solution.shoulder_pitch_rad)),
           App_RoundFloatToLong(App_RadiansToDegrees(joint_solution.gripper_lift_pitch_rad)),
           app_cartesian_trajectory.target_pulses.fore_aft_pulse_us,
           app_cartesian_trajectory.target_pulses.gripper_lift_pulse_us,
           app_cartesian_trajectory.target_pulses.left_right_pulse_us,
           (unsigned long)duration_ms);
  }
  else
  {
    App_RobotArmApplyJointAngles(&joint_solution);
    duration_ms = app_servo_trajectory.duration_ms;
    printf("[cmd_xyz] current pose estimate unavailable, fallback_to_joint_space target=%ld,%ld,%ld mm elbow=%s q_deg yaw=%ld shoulder=%ld gripper=%ld target_pulses fore_aft=%u gripper_lift=%u left_right=%u duration=%lu ms\r\n",
           (long)target_position->x_mm,
           (long)target_position->y_mm,
           (long)target_position->z_mm,
           App_ElbowModeName(elbow_mode),
           App_RoundFloatToLong(App_RadiansToDegrees(joint_solution.base_yaw_rad)),
           App_RoundFloatToLong(App_RadiansToDegrees(joint_solution.shoulder_pitch_rad)),
           App_RoundFloatToLong(App_RadiansToDegrees(joint_solution.gripper_lift_pitch_rad)),
           target_pulses.fore_aft_pulse_us,
           target_pulses.gripper_lift_pulse_us,
           target_pulses.left_right_pulse_us,
           (unsigned long)duration_ms);
  }

  printf("[cmd_xyz] estimated_pose x=%ld y=%ld z=%ld mm\r\n",
         App_RoundFloatToLong(solved_pose.x_mm),
         App_RoundFloatToLong(solved_pose.y_mm),
         App_RoundFloatToLong(solved_pose.z_mm));
  App_LogRobotArmStatus();
  return true;
}

typedef struct
{
  long x_mm;
  long y_mm;
  long z_mm;
  uint16_t clamp_pulse_us;
  uint16_t fore_aft_pulse_us;
  uint16_t gripper_lift_pulse_us;
  uint16_t left_right_pulse_us;
  App_ServoMotionSource motion_source;
  bool obstacle_detected;
  bool vl53l0x_ready;
  bool valid;
} App_StatusSnapshot;

static App_StatusSnapshot app_last_logged_status = { 0 };
static uint32_t app_last_status_log_tick = 0U;

#define APP_STATUS_LOG_HEARTBEAT_MS 5000U

static bool App_StatusSnapshotEqual(const App_StatusSnapshot *a, const App_StatusSnapshot *b)
{
  return a->valid && b->valid &&
         a->x_mm == b->x_mm && a->y_mm == b->y_mm && a->z_mm == b->z_mm &&
         a->clamp_pulse_us == b->clamp_pulse_us &&
         a->fore_aft_pulse_us == b->fore_aft_pulse_us &&
         a->gripper_lift_pulse_us == b->gripper_lift_pulse_us &&
         a->left_right_pulse_us == b->left_right_pulse_us &&
         a->motion_source == b->motion_source &&
         a->obstacle_detected == b->obstacle_detected &&
         a->vl53l0x_ready == b->vl53l0x_ready;
}

static void App_LogRobotArmStatus(void)
{
  RobotArmPose current_pose;
  App_ServoMotionSource active_motion = APP_SERVO_MOTION_SOURCE_IDLE;
  App_ServoPulseSet target_pulses = App_ServoGetCurrentPulseSet();
  uint32_t remaining_ms = 0U;
  uint32_t now = HAL_GetTick();
  App_StatusSnapshot snap;
  const char *obstacle_state;
  uint16_t obstacle_range_mm;

  if (!App_RobotArmEstimateCurrentPose(&current_pose))
  {
    printf("[status] pose estimation failed\r\n");
    return;
  }

  if (app_cartesian_trajectory.active)
  {
    uint32_t elapsed_ms = now - app_cartesian_trajectory.start_tick;
    active_motion = APP_SERVO_MOTION_SOURCE_XYZ;
    target_pulses = app_cartesian_trajectory.target_pulses;
    remaining_ms = app_cartesian_trajectory.duration_ms > elapsed_ms ?
                   (app_cartesian_trajectory.duration_ms - elapsed_ms) : 0U;
  }
  else if (app_servo_trajectory.active)
  {
    uint32_t elapsed_ms = now - app_servo_trajectory.start_tick;
    active_motion = app_servo_trajectory.source;
    target_pulses = app_servo_trajectory.target_pulses;
    remaining_ms = app_servo_trajectory.duration_ms > elapsed_ms ?
                   (app_servo_trajectory.duration_ms - elapsed_ms) : 0U;
  }

  snap.x_mm = App_RoundFloatToLong(current_pose.x_mm);
  snap.y_mm = App_RoundFloatToLong(current_pose.y_mm);
  snap.z_mm = App_RoundFloatToLong(current_pose.z_mm);
  snap.clamp_pulse_us = app_clamp_servo_current_pulse_us;
  snap.fore_aft_pulse_us = app_fore_aft_servo_current_pulse_us;
  snap.gripper_lift_pulse_us = app_gripper_lift_servo_current_pulse_us;
  snap.left_right_pulse_us = app_left_right_servo_current_pulse_us;
  snap.motion_source = active_motion;
  snap.obstacle_detected = app_obstacle_detected;
  snap.vl53l0x_ready = phase1_vl53l0x_ready;
  snap.valid = true;

  if (App_StatusSnapshotEqual(&snap, &app_last_logged_status) &&
      (now - app_last_status_log_tick) < APP_STATUS_LOG_HEARTBEAT_MS)
  {
    return;
  }

  if (!phase1_vl53l0x_ready)
  {
    obstacle_state = "off";
    obstacle_range_mm = 0U;
  }
  else if (!app_obstacle_has_last_measurement)
  {
    obstacle_state = "no-data";
    obstacle_range_mm = 0U;
  }
  else
  {
    obstacle_state = app_obstacle_detected ? "STOP" :
                     (app_obstacle_last_measurement.range_status_code == VL53L0X_DEVICEERROR_RANGECOMPLETE ? "clear" : "unknown");
    obstacle_range_mm = app_obstacle_last_measurement.range_mm;
  }

  printf("[status] xyz=%ld,%ld,%ld pulse=%u,%u,%u,%u motion=%s remain=%lu obstacle=%s/%umm\r\n",
         snap.x_mm, snap.y_mm, snap.z_mm,
         snap.clamp_pulse_us, snap.fore_aft_pulse_us,
         snap.gripper_lift_pulse_us, snap.left_right_pulse_us,
         App_ServoMotionSourceName(active_motion),
         (unsigned long)remaining_ms,
         obstacle_state, obstacle_range_mm);

  app_last_logged_status = snap;
  app_last_status_log_tick = now;
}

static bool App_CommandStartsWith(const char *command_line, const char *command_name)
{
  size_t index = 0U;

  if (command_line == NULL || command_name == NULL)
  {
    return false;
  }

  while (command_name[index] != '\0')
  {
    if (toupper((unsigned char)command_line[index]) != (unsigned char)command_name[index])
    {
      return false;
    }

    ++index;
  }

  return command_line[index] == '\0' || command_line[index] == ' ' || command_line[index] == '\t';
}

static bool App_ParseNextInt32(const char **cursor, int32_t *value_out)
{
  char *end_pointer;
  long parsed_value;

  if (cursor == NULL || *cursor == NULL || value_out == NULL)
  {
    return false;
  }

  while (**cursor == ' ' || **cursor == '\t' || **cursor == ',')
  {
    ++(*cursor);
  }

  if (**cursor == '\0')
  {
    return false;
  }

  parsed_value = strtol(*cursor, &end_pointer, 10);
  if (end_pointer == *cursor)
  {
    return false;
  }

  *value_out = (int32_t)parsed_value;
  *cursor = end_pointer;
  return true;
}

static bool App_CommandHasTrailingCharacters(const char *cursor)
{
  if (cursor == NULL)
  {
    return false;
  }

  while (*cursor == ' ' || *cursor == '\t' || *cursor == ',')
  {
    ++cursor;
  }

  return *cursor != '\0';
}

static void App_PrintSerialCommandHelp(void)
{
  printf("[cmd] HELP\r\n");
  printf("[cmd] STATUS\r\n");
  printf("[cmd] HOME\r\n");
  printf("[cmd] XYZ <x_mm> <y_mm> <z_mm>\r\n");
  printf("[cmd] PWM <clamp_us> <fore_aft_us> <gripper_lift_us> <left_right_us>\r\n");
}

static void App_ProcessSerialCommand(const char *command_line)
{
  const char *cursor = command_line;

  if (command_line == NULL)
  {
    return;
  }

  while (*cursor == ' ' || *cursor == '\t')
  {
    ++cursor;
  }

  if (*cursor == '\0')
  {
    return;
  }

  if (App_CommandStartsWith(cursor, "HELP"))
  {
    App_PrintSerialCommandHelp();
    return;
  }

  if (App_CommandStartsWith(cursor, "STATUS"))
  {
    App_LogRobotArmStatus();
    return;
  }

  if (App_CommandStartsWith(cursor, "HOME"))
  {
    App_ServoPulseSet home_pulses = {
      APP_CLAMP_SERVO_DEFAULT_PULSE_US,
      APP_FORE_AFT_SERVO_DEFAULT_PULSE_US,
      APP_GRIPPER_LIFT_SERVO_DEFAULT_PULSE_US,
      APP_LEFT_RIGHT_SERVO_DEFAULT_PULSE_US
    };
    uint32_t duration_ms = 0U;

    (void)App_ServoTrajectoryStart(&home_pulses, APP_SERVO_MOTION_SOURCE_HOME, &duration_ms);
    printf("[cmd_home] queued default midpoint pose duration=%lu ms target clamp=%u fore_aft=%u gripper_lift=%u left_right=%u\r\n",
           (unsigned long)duration_ms,
           app_servo_trajectory.target_pulses.clamp_pulse_us,
           app_servo_trajectory.target_pulses.fore_aft_pulse_us,
           app_servo_trajectory.target_pulses.gripper_lift_pulse_us,
           app_servo_trajectory.target_pulses.left_right_pulse_us);
    App_LogRobotArmStatus();
    return;
  }

  if (App_CommandStartsWith(cursor, "XYZ"))
  {
    RobotArmVector3 target_position;
    int32_t x_mm;
    int32_t y_mm;
    int32_t z_mm;

    cursor += strlen("XYZ");
    if (!App_ParseNextInt32(&cursor, &x_mm) ||
        !App_ParseNextInt32(&cursor, &y_mm) ||
        !App_ParseNextInt32(&cursor, &z_mm) ||
        App_CommandHasTrailingCharacters(cursor))
    {
      printf("[cmd_xyz] usage: XYZ <x_mm> <y_mm> <z_mm>\r\n");
      return;
    }

    target_position.x_mm = (float)x_mm;
    target_position.y_mm = (float)y_mm;
    target_position.z_mm = (float)z_mm;
    App_RobotArmMoveToTarget(&target_position);
    return;
  }

  if (App_CommandStartsWith(cursor, "PWM"))
  {
    App_ServoPulseSet requested_pulses;
    uint32_t duration_ms = 0U;
    int32_t clamp_pulse_us;
    int32_t fore_aft_pulse_us;
    int32_t gripper_lift_pulse_us;
    int32_t left_right_pulse_us;

    cursor += strlen("PWM");
    if (!App_ParseNextInt32(&cursor, &clamp_pulse_us) ||
        !App_ParseNextInt32(&cursor, &fore_aft_pulse_us) ||
        !App_ParseNextInt32(&cursor, &gripper_lift_pulse_us) ||
        !App_ParseNextInt32(&cursor, &left_right_pulse_us) ||
        App_CommandHasTrailingCharacters(cursor))
    {
      printf("[cmd_pwm] usage: PWM <clamp_us> <fore_aft_us> <gripper_lift_us> <left_right_us>\r\n");
      return;
    }

          requested_pulses.clamp_pulse_us = App_Int32ToPulseWidth(clamp_pulse_us);
          requested_pulses.fore_aft_pulse_us = App_Int32ToPulseWidth(fore_aft_pulse_us);
          requested_pulses.gripper_lift_pulse_us = App_Int32ToPulseWidth(gripper_lift_pulse_us);
          requested_pulses.left_right_pulse_us = App_Int32ToPulseWidth(left_right_pulse_us);
          (void)App_ServoTrajectoryStart(&requested_pulses, APP_SERVO_MOTION_SOURCE_PWM, &duration_ms);
          printf("[cmd_pwm] queued request clamp=%ld fore_aft=%ld gripper_lift=%ld left_right=%ld us target clamp=%u fore_aft=%u gripper_lift=%u left_right=%u duration=%lu ms\r\n",
           (long)clamp_pulse_us,
           (long)fore_aft_pulse_us,
           (long)gripper_lift_pulse_us,
            (long)left_right_pulse_us,
            app_servo_trajectory.target_pulses.clamp_pulse_us,
            app_servo_trajectory.target_pulses.fore_aft_pulse_us,
            app_servo_trajectory.target_pulses.gripper_lift_pulse_us,
            app_servo_trajectory.target_pulses.left_right_pulse_us,
            (unsigned long)duration_ms);
    App_LogRobotArmStatus();
    return;
  }

  printf("[cmd] unknown command: %s\r\n", cursor);
  App_PrintSerialCommandHelp();
}

static void App_QueueSerialByteFromISR(uint8_t received_byte)
{
  uint16_t next_write_index = app_uart_rx_ring_write_index + 1U;

  if (next_write_index >= APP_UART_RX_RING_BUFFER_LENGTH)
  {
    next_write_index = 0U;
  }

  if (next_write_index == app_uart_rx_ring_read_index)
  {
    app_uart_rx_ring_overflow = true;
    return;
  }

  app_uart_rx_ring_buffer[app_uart_rx_ring_write_index] = received_byte;
  app_uart_rx_ring_write_index = next_write_index;
}

static bool App_DequeueSerialByte(uint8_t *received_byte_out)
{
  uint16_t read_index;

  if (received_byte_out == NULL)
  {
    return false;
  }

  read_index = app_uart_rx_ring_read_index;
  if (read_index == app_uart_rx_ring_write_index)
  {
    return false;
  }

  *received_byte_out = app_uart_rx_ring_buffer[read_index];
  ++read_index;
  if (read_index >= APP_UART_RX_RING_BUFFER_LENGTH)
  {
    read_index = 0U;
  }

  app_uart_rx_ring_read_index = read_index;
  return true;
}

static void App_HandleSerialByte(uint8_t received_byte)
{
  if (received_byte == '\r' || received_byte == '\n')
  {
    if (app_uart_command_overflow)
    {
      printf("[cmd] input line too long, command discarded\r\n");
      app_uart_command_length = 0U;
      app_uart_command_overflow = false;
      return;
    }

    if (app_uart_command_length == 0U)
    {
      return;
    }

    app_uart_command_buffer[app_uart_command_length] = '\0';
    App_ProcessSerialCommand(app_uart_command_buffer);
    app_uart_command_length = 0U;
    return;
  }

  if (received_byte == 0x08U || received_byte == 0x7FU)
  {
    if (app_uart_command_length > 0U)
    {
      --app_uart_command_length;
    }
    return;
  }

  if (app_uart_command_overflow)
  {
    return;
  }

  if (received_byte < 32U || received_byte > 126U)
  {
    return;
  }

  if (app_uart_command_length >= (APP_UART_COMMAND_BUFFER_LENGTH - 1U))
  {
    app_uart_command_overflow = true;
    return;
  }

  app_uart_command_buffer[app_uart_command_length] = (char)received_byte;
  ++app_uart_command_length;
}

static void App_ProcessSerialInput(void)
{
  uint8_t received_byte;

  while (App_DequeueSerialByte(&received_byte))
  {
    App_HandleSerialByte(received_byte);
  }

  if (app_uart_rx_ring_overflow)
  {
    app_uart_rx_ring_overflow = false;
    printf("[cmd] uart rx ring overflow, input bytes discarded\r\n");
  }
}

static void App_USART3_StartReceiveIT(void)
{
  HAL_StatusTypeDef status = HAL_UART_Receive_IT(&huart3, &app_uart_rx_byte, 1U);

  if (status != HAL_OK && status != HAL_BUSY)
  {
    Error_Handler();
  }
}

void App_USART3_IRQHandler(void)
{
  /* Drain TX ringbuffer while FIFO has space and we have data. */
  if (__HAL_UART_GET_IT_SOURCE(&huart3, UART_IT_TXE) != RESET)
  {
    while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TXE) != RESET &&
           app_uart_tx_ring_read_index != app_uart_tx_ring_write_index)
    {
      huart3.Instance->TDR = app_uart_tx_ring_buffer[app_uart_tx_ring_read_index];
      app_uart_tx_ring_read_index =
        (uint16_t)((app_uart_tx_ring_read_index + 1U) % APP_UART_TX_RING_BUFFER_LENGTH);
    }

    if (app_uart_tx_ring_read_index == app_uart_tx_ring_write_index)
    {
      __HAL_UART_DISABLE_IT(&huart3, UART_IT_TXE);
    }
  }

  HAL_UART_IRQHandler(&huart3);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart == NULL || huart->Instance != USART3)
  {
    return;
  }

  App_QueueSerialByteFromISR(app_uart_rx_byte);
  App_USART3_StartReceiveIT();
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart == NULL || huart->Instance != USART3)
  {
    return;
  }

  __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_OREF | UART_CLEAR_NEF | UART_CLEAR_PEF | UART_CLEAR_FEF);
  App_USART3_StartReceiveIT();
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  if (huart->Instance == USART3)
  {
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_USART3_CLK_ENABLE();

    GPIO_InitStruct.Pin = APP_VCP_TX_Pin | APP_VCP_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = APP_VCP_TX_AF;
    HAL_GPIO_Init(APP_VCP_TX_GPIO_Port, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(USART3_IRQn, 0U, 0U);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART3)
  {
    HAL_GPIO_DeInit(APP_VCP_TX_GPIO_Port, APP_VCP_TX_Pin | APP_VCP_RX_Pin);
    HAL_NVIC_DisableIRQ(USART3_IRQn);
    __HAL_RCC_USART3_CLK_DISABLE();
  }
}

static void App_USART3_UART_Init(void)
{
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_8;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
  {
    Error_Handler();
  }

  app_uart_rx_ring_read_index = 0U;
  app_uart_rx_ring_write_index = 0U;
  app_uart_rx_ring_overflow = false;
  App_USART3_StartReceiveIT();
}

static void App_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  HAL_GPIO_WritePin(APP_LD1_BLUE_GPIO_Port, APP_LD1_BLUE_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(APP_LD2_RED_GPIO_Port, APP_LD2_RED_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(APP_LD3_GREEN_GPIO_Port, APP_LD3_GREEN_Pin, GPIO_PIN_SET);

  GPIO_InitStruct.Pin = APP_LD1_BLUE_Pin | APP_LD2_RED_Pin | APP_LD3_GREEN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(APP_LD1_BLUE_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = APP_USER_BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(APP_USER_BUTTON_GPIO_Port, &GPIO_InitStruct);
}

static bool Phase0_IsButtonPressed(void)
{
  return HAL_GPIO_ReadPin(APP_USER_BUTTON_GPIO_Port, APP_USER_BUTTON_Pin) == GPIO_PIN_SET;
}

static void Phase0_SetLedState(GPIO_TypeDef *gpio_port, uint16_t gpio_pin, bool on)
{
  HAL_GPIO_WritePin(gpio_port, gpio_pin, on ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

static void Phase0_LogStartup(bool button_pressed)
{
  printf("\r\n[phase0] minimal system ready\r\n");
  printf("[phase0] uart: USART3 115200 8N1 on PE5/PE6\r\n");
  printf("[phase0] led: LD3 heartbeat, LD2 mirrors button\r\n");
  printf("[phase0] button: initial state is %s\r\n", button_pressed ? "pressed" : "released");
  printf("[phase0] button: short press runs one full-range sweep across all 4 axes\r\n");
  printf("[phase0] uart commands: HELP | STATUS | HOME | XYZ x y z | PWM clamp fore gripper left\r\n");
}

static float App_DegreesToRadians(float degrees)
{
  return degrees * (APP_ROBOT_ARM_PI / 180.0f);
}

static void App_LogRobotArmKinematicsModel(void)
{
  const RobotArmGeometry sample_geometry = App_RobotArmGetGeometry();
  const RobotArmJointAngles sample_joint_angles = {
    0.35f,
    0.60f,
    -0.40f
  };
  const RobotArmVector3 cartesian_step = {
    5.0f,
    0.0f,
    3.0f
  };
  const RobotArmJointLimits safe_joint_limits = App_RobotArmGetSafeJointLimits();
  RobotArmPose sample_pose;
  RobotArmJacobian sample_jacobian;
  RobotArmWorkspaceBounds safe_workspace;
  RobotArmJointAngles recovered_joint_angles;
  RobotArmJointAngles joint_step;
  RobotArmVector3 sample_target;
  RobotArmIkStatus analytic_status;
  RobotArmIkStatus differential_status;
  bool safe_workspace_ok;

  printf("[kinematics] frame: origin=base, +X forward, +Y left, +Z up\r\n");
  printf("[kinematics] joints: q0=left_right yaw, q1=fore_aft pitch, q2=gripper_lift pitch, clamp excluded from xyz solve\r\n");

  if (!RobotArmKinematics_Forward(&sample_geometry, &sample_joint_angles, &sample_pose) ||
      !RobotArmKinematics_ComputeJacobian(&sample_geometry, &sample_joint_angles, &sample_jacobian))
  {
    printf("[kinematics] model self-check failed during forward/jacobian setup\r\n");
    return;
  }

  sample_target.x_mm = sample_pose.x_mm;
  sample_target.y_mm = sample_pose.y_mm;
  sample_target.z_mm = sample_pose.z_mm;

  analytic_status = RobotArmKinematics_InversePosition(&sample_geometry,
                                                       &sample_target,
                                                       ROBOT_ARM_ELBOW_MODE_DOWN,
                                                       &recovered_joint_angles);
  differential_status = RobotArmKinematics_SolveDifferentialIk(&sample_geometry,
                                                               &sample_joint_angles,
                                                               &cartesian_step,
                                                               0.10f,
                                                               &joint_step);
  safe_workspace_ok = RobotArmKinematics_EstimateWorkspaceBounds(&sample_geometry,
                                                                 &safe_joint_limits,
                                                                 APP_SAFE_WORKSPACE_MIN_Z_MM,
                                                                 APP_SAFE_WORKSPACE_SAMPLES_PER_JOINT,
                                                                 &safe_workspace);

  printf("[kinematics] sample geometry base=%u upper=%u forearm=%u tool=%u mm analytic_ik=%s differential_ik=%s\r\n",
         (unsigned int)sample_geometry.base_height_mm,
         (unsigned int)sample_geometry.shoulder_length_mm,
         (unsigned int)sample_geometry.forearm_length_mm,
         (unsigned int)sample_geometry.tool_length_mm,
         RobotArmKinematics_IkStatusString(analytic_status),
         RobotArmKinematics_IkStatusString(differential_status));
  printf("[kinematics] hard safety limit: never exceed measured no-collision pwm windows clamp=%u..%u us left_right=%u..%u us fore_aft=%u..%u us gripper_lift=%u..%u us\r\n",
         APP_CLAMP_SERVO_OPEN_PULSE_US,
         APP_CLAMP_SERVO_CLOSE_PULSE_US,
         APP_LEFT_RIGHT_SERVO_RIGHT_PULSE_US,
         APP_LEFT_RIGHT_SERVO_LEFT_PULSE_US,
         APP_FORE_AFT_SERVO_AFT_PULSE_US,
         APP_FORE_AFT_SERVO_FORE_PULSE_US,
         APP_GRIPPER_LIFT_SERVO_DOWN_PULSE_US,
         APP_GRIPPER_LIFT_SERVO_UP_PULSE_US);
  printf("[kinematics] conservative safe angle estimate left_right=%.0f..%.0f deg fore_aft=%.0f..%.0f deg gripper_lift=%.0f..%.0f deg\r\n",
         APP_LEFT_RIGHT_SAFE_MIN_DEG,
         APP_LEFT_RIGHT_SAFE_MAX_DEG,
         APP_FORE_AFT_SAFE_MIN_DEG,
         APP_FORE_AFT_SAFE_MAX_DEG,
         APP_GRIPPER_LIFT_SAFE_MIN_DEG,
         APP_GRIPPER_LIFT_SAFE_MAX_DEG);

  if (!safe_workspace_ok)
  {
    printf("[kinematics] safe workspace estimate failed\r\n");
    return;
  }

  printf("[kinematics] conservative safe workspace above base x=%.0f..%.0f y=%.0f..%.0f z=%.0f..%.0f mm horiz_radius=%.0f..%.0f mm\r\n",
         safe_workspace.minimum_position.x_mm,
         safe_workspace.maximum_position.x_mm,
         safe_workspace.minimum_position.y_mm,
         safe_workspace.maximum_position.y_mm,
         safe_workspace.minimum_position.z_mm,
         safe_workspace.maximum_position.z_mm,
         safe_workspace.minimum_horizontal_radius_mm,
         safe_workspace.maximum_horizontal_radius_mm);
}

static bool Phase1_IsDeviceReady(uint8_t address)
{
  return HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(address << 1), 1U, 2U) == HAL_OK;
}

static void Phase1_RecoverI2cBus(void)
{
  GPIO_InitTypeDef gpio_init = {0};
  uint32_t i;

  /* Reset I2C peripheral and release pin AF so we can drive them manually. */
  HAL_I2C_DeInit(&hi2c1);
  __HAL_RCC_I2C1_FORCE_RESET();
  HAL_Delay(2U);
  __HAL_RCC_I2C1_RELEASE_RESET();

  /* Ensure clocks are on (may already be, but harmless). */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();

  /* Pre-set both lines high before switching mode to avoid a spurious START. */
  HAL_GPIO_WritePin(APP_I2C1_SCL_GPIO_Port, APP_I2C1_SCL_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(APP_I2C1_SDA_GPIO_Port, APP_I2C1_SDA_Pin, GPIO_PIN_SET);

  gpio_init.Mode  = GPIO_MODE_OUTPUT_OD;
  gpio_init.Pull  = GPIO_NOPULL;
  gpio_init.Speed = GPIO_SPEED_FREQ_LOW;

  gpio_init.Pin = APP_I2C1_SCL_Pin;
  HAL_GPIO_Init(APP_I2C1_SCL_GPIO_Port, &gpio_init);

  gpio_init.Pin = APP_I2C1_SDA_Pin;
  HAL_GPIO_Init(APP_I2C1_SDA_GPIO_Port, &gpio_init);

  /*
   * Clock out any slave that is stuck mid-transaction (holding SDA low).
   * Toggle SCL up to 9 times; stop early once SDA is released.
   */
  for (i = 0U; i < 9U; ++i)
  {
    if (HAL_GPIO_ReadPin(APP_I2C1_SDA_GPIO_Port, APP_I2C1_SDA_Pin) == GPIO_PIN_SET)
    {
      break;
    }

    HAL_GPIO_WritePin(APP_I2C1_SCL_GPIO_Port, APP_I2C1_SCL_Pin, GPIO_PIN_RESET);
    HAL_Delay(1U);
    HAL_GPIO_WritePin(APP_I2C1_SCL_GPIO_Port, APP_I2C1_SCL_Pin, GPIO_PIN_SET);
    HAL_Delay(1U);
  }

  /* Generate a STOP condition: SDA low -> high while SCL is high. */
  HAL_GPIO_WritePin(APP_I2C1_SDA_GPIO_Port, APP_I2C1_SDA_Pin, GPIO_PIN_RESET);
  HAL_Delay(1U);
  HAL_GPIO_WritePin(APP_I2C1_SCL_GPIO_Port, APP_I2C1_SCL_Pin, GPIO_PIN_SET);
  HAL_Delay(1U);
  HAL_GPIO_WritePin(APP_I2C1_SDA_GPIO_Port, APP_I2C1_SDA_Pin, GPIO_PIN_SET);
  HAL_Delay(1U);

  /* Restore SCL and SDA to I2C alternate-function open-drain. */
  gpio_init.Mode = GPIO_MODE_AF_OD;

  gpio_init.Pin       = APP_I2C1_SCL_Pin;
  gpio_init.Alternate = APP_I2C1_SCL_AF;
  HAL_GPIO_Init(APP_I2C1_SCL_GPIO_Port, &gpio_init);

  gpio_init.Pin       = APP_I2C1_SDA_Pin;
  gpio_init.Alternate = APP_I2C1_SDA_AF;
  HAL_GPIO_Init(APP_I2C1_SDA_GPIO_Port, &gpio_init);

  /* Reinitialize the I2C peripheral with the CubeMX-generated settings. */
  MX_I2C1_Init();
}

static bool Phase1_WriteRegister8(uint8_t address, uint8_t register_address, uint8_t value)
{
  if (HAL_I2C_Mem_Write(&hi2c1,
                         (uint16_t)(address << 1),
                         register_address,
                         I2C_MEMADD_SIZE_8BIT,
                         &value,
                         1U,
                         PHASE1_I2C_TIMEOUT_MS) == HAL_OK)
  {
    return true;
  }

  Phase1_RecoverI2cBus();
  return false;
}

static bool Phase1_ReadRegister8(uint8_t address, uint8_t register_address, uint8_t *value)
{
  return Phase1_ReadRegisters(address, register_address, value, 1U);
}

static bool Phase1_ReadRegisters(uint8_t address, uint8_t register_address, uint8_t *buffer, uint16_t length)
{
  if (HAL_I2C_Mem_Read(&hi2c1,
                        (uint16_t)(address << 1),
                        register_address,
                        I2C_MEMADD_SIZE_8BIT,
                        buffer,
                        length,
                        PHASE1_I2C_TIMEOUT_MS) == HAL_OK)
  {
    return true;
  }

  Phase1_RecoverI2cBus();
  return false;
}

static bool Phase1_ReadRegister16(uint8_t address, uint8_t register_address, uint16_t *value)
{
  uint8_t buffer[2];

  if (!Phase1_ReadRegisters(address, register_address, buffer, sizeof(buffer)))
  {
    return false;
  }

  *value = (uint16_t)(((uint16_t)buffer[0] << 8) | buffer[1]);
  return true;
}

static void Phase1_LogMpu6050Probe(uint8_t address)
{
  uint8_t who_am_i = 0U;
  bool recognized = false;

  if (!Phase1_ReadRegister8(address, MPU6050_WHO_AM_I_REG, &who_am_i))
  {
    printf("[mpu6050] 0x%02X WHO_AM_I read failed\r\n", address);
    return;
  }

  recognized = (who_am_i == MPU6050_I2C_ADDR_AD0_LOW) || (who_am_i == MPU6050_I2C_ADDR_AD0_HIGH);
  printf("[mpu6050] 0x%02X WHO_AM_I=0x%02X%s\r\n",
         address,
         who_am_i,
         recognized ? " ok" : " unexpected");
}

static bool Phase1_Mpu6050Init(uint8_t address)
{
  if (!Phase1_WriteRegister8(address, MPU6050_PWR_MGMT_1_REG, MPU6050_CLOCK_PLL_XGYRO))
  {
    return false;
  }

  if (!Phase1_WriteRegister8(address, MPU6050_CONFIG_REG, 0x03U))
  {
    return false;
  }

  if (!Phase1_WriteRegister8(address, MPU6050_GYRO_CONFIG_REG, 0x00U))
  {
    return false;
  }

  if (!Phase1_WriteRegister8(address, MPU6050_ACCEL_CONFIG_REG, 0x00U))
  {
    return false;
  }

  phase1_mpu6050_address = address;
  phase1_mpu6050_ready = true;
  return true;
}

static bool Phase1_Mpu6050ReadRawSample(uint8_t address, Phase1_Mpu6050RawSample *sample)
{
  uint8_t buffer[14];

  if (!Phase1_ReadRegisters(address, MPU6050_ACCEL_XOUT_H_REG, buffer, sizeof(buffer)))
  {
    return false;
  }

  sample->accel_x = (int16_t)(((uint16_t)buffer[0] << 8) | buffer[1]);
  sample->accel_y = (int16_t)(((uint16_t)buffer[2] << 8) | buffer[3]);
  sample->accel_z = (int16_t)(((uint16_t)buffer[4] << 8) | buffer[5]);
  sample->temperature_raw = (int16_t)(((uint16_t)buffer[6] << 8) | buffer[7]);
  sample->gyro_x = (int16_t)(((uint16_t)buffer[8] << 8) | buffer[9]);
  sample->gyro_y = (int16_t)(((uint16_t)buffer[10] << 8) | buffer[11]);
  sample->gyro_z = (int16_t)(((uint16_t)buffer[12] << 8) | buffer[13]);
  return true;
}

static void Phase1_LogMpu6050Data(void)
{
  Phase1_Mpu6050RawSample sample;
  long accel_mg_x;
  long accel_mg_y;
  long accel_mg_z;
  long gyro_mdps_x;
  long gyro_mdps_y;
  long gyro_mdps_z;
  long temperature_centi_c;
  long temperature_fraction;

  if (!phase1_mpu6050_ready)
  {
    return;
  }

  if (!Phase1_Mpu6050ReadRawSample(phase1_mpu6050_address, &sample))
  {
    printf("[mpu6050] raw sample read failed\r\n");
    return;
  }

  accel_mg_x = ((long)sample.accel_x * 1000L) / PHASE1_MPU6050_ACCEL_LSB_PER_G;
  accel_mg_y = ((long)sample.accel_y * 1000L) / PHASE1_MPU6050_ACCEL_LSB_PER_G;
  accel_mg_z = ((long)sample.accel_z * 1000L) / PHASE1_MPU6050_ACCEL_LSB_PER_G;
  gyro_mdps_x = ((long)sample.gyro_x * 1000L) / PHASE1_MPU6050_GYRO_LSB_PER_DPS;
  gyro_mdps_y = ((long)sample.gyro_y * 1000L) / PHASE1_MPU6050_GYRO_LSB_PER_DPS;
  gyro_mdps_z = ((long)sample.gyro_z * 1000L) / PHASE1_MPU6050_GYRO_LSB_PER_DPS;
  temperature_centi_c = (((long)sample.temperature_raw) * 100L) / 340L + 3653L;
  temperature_fraction = temperature_centi_c >= 0 ? (temperature_centi_c % 100L) : (-temperature_centi_c % 100L);

  printf("[mpu6050] acc=%ld,%ld,%ld mg temp=%ld.%02ldC gyro=%ld,%ld,%ld mdps\r\n",
         accel_mg_x,
         accel_mg_y,
         accel_mg_z,
         temperature_centi_c / 100L,
         temperature_fraction,
         gyro_mdps_x,
         gyro_mdps_y,
         gyro_mdps_z);
}

static void Phase1_LogVl53l0xProbe(uint8_t address)
{
  uint8_t model_id = 0U;

  if (!Phase1_ReadRegister8(address, VL53L0X_MODEL_ID_REG, &model_id))
  {
    printf("[vl53l0x] 0x%02X MODEL_ID read failed\r\n", address);
    return;
  }

  printf("[vl53l0x] 0x%02X MODEL_ID=0x%02X%s\r\n",
         address,
         model_id,
         model_id == VL53L0X_MODEL_ID_VALUE ? " ok" : " unexpected");
}

static bool Phase1_Vl53l0xPrepareSingleShot(uint8_t address)
{
  return Phase1_WriteRegister8(address, VL53L0X_PRIVATE_INIT_REG, 0x01U) &&
         Phase1_WriteRegister8(address, VL53L0X_PRIVATE_PAGE_SELECT_REG, 0x01U) &&
         Phase1_WriteRegister8(address, 0x00U, 0x00U) &&
         Phase1_WriteRegister8(address, VL53L0X_PRIVATE_STOP_VARIABLE_REG, phase1_vl53l0x_stop_variable) &&
         Phase1_WriteRegister8(address, 0x00U, 0x01U) &&
         Phase1_WriteRegister8(address, VL53L0X_PRIVATE_PAGE_SELECT_REG, 0x00U) &&
         Phase1_WriteRegister8(address, VL53L0X_PRIVATE_INIT_REG, 0x00U);
}

static bool Phase1_Vl53l0xInit(uint8_t address)
{
  uint8_t value = 0U;

  if (!Phase1_ReadRegister8(address, VL53L0X_MODEL_ID_REG, &value))
  {
    printf("[vl53l0x] init failed step=model-id-read reg=0x%02X i2c_err=0x%08lX\r\n",
           VL53L0X_MODEL_ID_REG,
           (unsigned long)HAL_I2C_GetError(&hi2c1));
    return false;
  }

  if (value != VL53L0X_MODEL_ID_VALUE)
  {
    printf("[vl53l0x] init failed step=model-id-validate reg=0x%02X expected=0x%02X actual=0x%02X\r\n",
           VL53L0X_MODEL_ID_REG,
           VL53L0X_MODEL_ID_VALUE,
           value);
    return false;
  }

  if (!Phase1_ReadRegister8(address, VL53L0X_VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV_REG, &value))
  {
    printf("[vl53l0x] init failed step=extsup-read reg=0x%02X i2c_err=0x%08lX\r\n",
           VL53L0X_VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV_REG,
           (unsigned long)HAL_I2C_GetError(&hi2c1));
    return false;
  }

  if (!Phase1_WriteRegister8(address, VL53L0X_VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV_REG, (uint8_t)(value | 0x01U)))
  {
    printf("[vl53l0x] init failed step=extsup-write reg=0x%02X value=0x%02X i2c_err=0x%08lX\r\n",
           VL53L0X_VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV_REG,
           (uint8_t)(value | 0x01U),
           (unsigned long)HAL_I2C_GetError(&hi2c1));
    return false;
  }

  if (!Phase1_WriteRegister8(address, VL53L0X_PRIVATE_STANDARD_MODE_REG, 0x00U))
  {
    printf("[vl53l0x] init failed step=private-standard-mode reg=0x%02X value=0x00 i2c_err=0x%08lX\r\n",
           VL53L0X_PRIVATE_STANDARD_MODE_REG,
           (unsigned long)HAL_I2C_GetError(&hi2c1));
    return false;
  }

  if (!Phase1_WriteRegister8(address, VL53L0X_PRIVATE_INIT_REG, 0x01U))
  {
    printf("[vl53l0x] init failed step=private-init-enter reg=0x%02X value=0x01 i2c_err=0x%08lX\r\n",
           VL53L0X_PRIVATE_INIT_REG,
           (unsigned long)HAL_I2C_GetError(&hi2c1));
    return false;
  }

  if (!Phase1_WriteRegister8(address, VL53L0X_PRIVATE_PAGE_SELECT_REG, 0x01U))
  {
    printf("[vl53l0x] init failed step=page-select-1 reg=0x%02X value=0x01 i2c_err=0x%08lX\r\n",
           VL53L0X_PRIVATE_PAGE_SELECT_REG,
           (unsigned long)HAL_I2C_GetError(&hi2c1));
    return false;
  }

  if (!Phase1_WriteRegister8(address, 0x00U, 0x00U))
  {
    printf("[vl53l0x] init failed step=page-1-select-zero reg=0x00 value=0x00 i2c_err=0x%08lX\r\n",
           (unsigned long)HAL_I2C_GetError(&hi2c1));
    return false;
  }

  if (!Phase1_ReadRegister8(address, VL53L0X_PRIVATE_STOP_VARIABLE_REG, &phase1_vl53l0x_stop_variable))
  {
    printf("[vl53l0x] init failed step=stop-variable-read reg=0x%02X i2c_err=0x%08lX\r\n",
           VL53L0X_PRIVATE_STOP_VARIABLE_REG,
           (unsigned long)HAL_I2C_GetError(&hi2c1));
    return false;
  }

  if (!Phase1_WriteRegister8(address, 0x00U, 0x01U))
  {
    printf("[vl53l0x] init failed step=page-1-select-one reg=0x00 value=0x01 i2c_err=0x%08lX\r\n",
           (unsigned long)HAL_I2C_GetError(&hi2c1));
    return false;
  }

  if (!Phase1_WriteRegister8(address, VL53L0X_PRIVATE_PAGE_SELECT_REG, 0x00U))
  {
    printf("[vl53l0x] init failed step=page-select-0 reg=0x%02X value=0x00 i2c_err=0x%08lX\r\n",
           VL53L0X_PRIVATE_PAGE_SELECT_REG,
           (unsigned long)HAL_I2C_GetError(&hi2c1));
    return false;
  }

  if (!Phase1_WriteRegister8(address, VL53L0X_PRIVATE_INIT_REG, 0x00U))
  {
    printf("[vl53l0x] init failed step=private-init-exit reg=0x%02X value=0x00 i2c_err=0x%08lX\r\n",
           VL53L0X_PRIVATE_INIT_REG,
           (unsigned long)HAL_I2C_GetError(&hi2c1));
    return false;
  }

  if (!Phase1_WriteRegister8(address, VL53L0X_SYSTEM_INTERRUPT_CONFIG_GPIO_REG, 0x04U))
  {
    printf("[vl53l0x] init failed step=irq-config reg=0x%02X value=0x04 i2c_err=0x%08lX\r\n",
           VL53L0X_SYSTEM_INTERRUPT_CONFIG_GPIO_REG,
           (unsigned long)HAL_I2C_GetError(&hi2c1));
    return false;
  }

  if (!Phase1_ReadRegister8(address, VL53L0X_GPIO_HV_MUX_ACTIVE_HIGH_REG, &value))
  {
    printf("[vl53l0x] init failed step=gpio-hv-read reg=0x%02X i2c_err=0x%08lX\r\n",
           VL53L0X_GPIO_HV_MUX_ACTIVE_HIGH_REG,
           (unsigned long)HAL_I2C_GetError(&hi2c1));
    return false;
  }

  if (!Phase1_WriteRegister8(address, VL53L0X_GPIO_HV_MUX_ACTIVE_HIGH_REG, (uint8_t)(value & ~0x10U)))
  {
    printf("[vl53l0x] init failed step=gpio-hv-write reg=0x%02X value=0x%02X i2c_err=0x%08lX\r\n",
           VL53L0X_GPIO_HV_MUX_ACTIVE_HIGH_REG,
           (uint8_t)(value & ~0x10U),
           (unsigned long)HAL_I2C_GetError(&hi2c1));
    return false;
  }

  if (!Phase1_WriteRegister8(address, VL53L0X_SYSTEM_INTERRUPT_CLEAR_REG, 0x01U))
  {
    printf("[vl53l0x] init failed step=irq-clear reg=0x%02X value=0x01 i2c_err=0x%08lX\r\n",
           VL53L0X_SYSTEM_INTERRUPT_CLEAR_REG,
           (unsigned long)HAL_I2C_GetError(&hi2c1));
    return false;
  }

  phase1_vl53l0x_ready = true;
  return true;
}

static bool Phase1_Vl53l0xReadMeasurementInternal(uint8_t address,
                                                  Phase1_Vl53l0xMeasurement *measurement,
                                                  bool log_failures)
{
  uint8_t status = 0U;
  uint32_t start_tick = HAL_GetTick();

  if (!Phase1_Vl53l0xPrepareSingleShot(address))
  {
    if (log_failures)
    {
      printf("[vl53l0x] prepare single-shot failed i2c_err=0x%08lX stop=0x%02X\r\n",
             (unsigned long)hi2c1.ErrorCode,
             phase1_vl53l0x_stop_variable);
    }
    return false;
  }

  if (!Phase1_WriteRegister8(address, VL53L0X_SYSRANGE_START_REG, 0x01U))
  {
    if (log_failures)
    {
      printf("[vl53l0x] start single-shot failed i2c_err=0x%08lX\r\n",
             (unsigned long)hi2c1.ErrorCode);
    }
    return false;
  }

  while (true)
  {
    if (!Phase1_ReadRegister8(address, VL53L0X_SYSRANGE_START_REG, &status))
    {
      if (log_failures)
      {
        printf("[vl53l0x] poll start failed i2c_err=0x%08lX\r\n",
               (unsigned long)hi2c1.ErrorCode);
      }
      return false;
    }

    if ((status & 0x01U) == 0U)
    {
      break;
    }

    if ((HAL_GetTick() - start_tick) >= PHASE1_VL53L0X_MEASUREMENT_TIMEOUT_MS)
    {
      if (log_failures)
      {
        printf("[vl53l0x] start timeout sysrange=0x%02X elapsed=%lu ms\r\n",
               status,
               (unsigned long)(HAL_GetTick() - start_tick));
      }
      return false;
    }
  }

  start_tick = HAL_GetTick();
  while (true)
  {
    if (!Phase1_ReadRegister8(address, VL53L0X_RESULT_INTERRUPT_STATUS_REG, &status))
    {
      if (log_failures)
      {
        printf("[vl53l0x] poll interrupt failed i2c_err=0x%08lX\r\n",
               (unsigned long)hi2c1.ErrorCode);
      }
      return false;
    }

    if ((status & 0x07U) != 0U)
    {
      break;
    }

    if ((HAL_GetTick() - start_tick) >= PHASE1_VL53L0X_MEASUREMENT_TIMEOUT_MS)
    {
      if (log_failures)
      {
        printf("[vl53l0x] interrupt timeout irq=0x%02X elapsed=%lu ms\r\n",
               status,
               (unsigned long)(HAL_GetTick() - start_tick));
      }
      return false;
    }
  }

  if (!Phase1_ReadRegister8(address, VL53L0X_RESULT_RANGE_STATUS_REG, &measurement->range_status_raw) ||
      !Phase1_ReadRegister16(address, (uint8_t)(VL53L0X_RESULT_RANGE_STATUS_REG + 10U), &measurement->range_mm) ||
      !Phase1_WriteRegister8(address, VL53L0X_SYSTEM_INTERRUPT_CLEAR_REG, 0x01U))
  {
    if (log_failures)
    {
      printf("[vl53l0x] readout failed i2c_err=0x%08lX\r\n",
             (unsigned long)hi2c1.ErrorCode);
    }
    return false;
  }

  measurement->range_status_code = (uint8_t)((measurement->range_status_raw >> 3) & 0x0FU);

  return true;
}

static bool Phase1_Vl53l0xReadMeasurement(uint8_t address, Phase1_Vl53l0xMeasurement *measurement)
{
  return Phase1_Vl53l0xReadMeasurementInternal(address, measurement, true);
}

static const char *Phase1_Vl53l0xDeviceCodeString(uint8_t device_code)
{
  switch (device_code)
  {
    case 0U:
      return "no-update";
    case 1U:
      return "vcsel-continuity";
    case 2U:
      return "vcsel-watchdog";
    case 3U:
      return "no-vhv";
    case 4U:
      return "msrc-no-target";
    case 5U:
      return "snr-check";
    case 6U:
      return "range-phase-check";
    case 7U:
      return "sigma-threshold";
    case 8U:
      return "tcc";
    case 9U:
      return "phase-consistency";
    case 10U:
      return "min-clip";
    case VL53L0X_DEVICEERROR_RANGECOMPLETE:
      return "range-complete";
    case 12U:
      return "algo-underflow";
    case 13U:
      return "algo-overflow";
    case 14U:
      return "range-ignore-threshold";
    default:
      return "unknown";
  }
}

static const char *Phase1_Vl53l0xMeasurementNote(const Phase1_Vl53l0xMeasurement *measurement, bool has_delta, uint16_t delta_mm)
{
  if (measurement->range_status_code != VL53L0X_DEVICEERROR_RANGECOMPLETE)
  {
    return "device-error";
  }

  if (measurement->range_mm <= PHASE1_VL53L0X_NEAR_FLOOR_MM)
  {
    return has_delta && delta_mm >= PHASE1_VL53L0X_RANGE_JUMP_MM ? "near-or-jump" : "near-floor";
  }

  if (has_delta && delta_mm >= PHASE1_VL53L0X_RANGE_JUMP_MM)
  {
    return "jump";
  }

  return "ok";
}

static void Phase1_LogVl53l0xRange(void)
{
  Phase1_Vl53l0xMeasurement measurement;
  uint16_t delta_mm = 0U;
  bool has_delta = false;
  const char *note;
  const char *device_status;

  if (!phase1_vl53l0x_ready)
  {
    return;
  }

  if (!Phase1_Vl53l0xReadMeasurement(VL53L0X_I2C_ADDR_DEFAULT, &measurement))
  {
    phase1_vl53l0x_ready = false;
    return;
  }

  app_obstacle_last_measurement = measurement;
  app_obstacle_has_last_measurement = true;

  if (phase1_vl53l0x_has_last_range)
  {
    has_delta = true;
    delta_mm = measurement.range_mm >= phase1_vl53l0x_last_range_mm
      ? (uint16_t)(measurement.range_mm - phase1_vl53l0x_last_range_mm)
      : (uint16_t)(phase1_vl53l0x_last_range_mm - measurement.range_mm);
  }

  note = Phase1_Vl53l0xMeasurementNote(&measurement, has_delta, delta_mm);
  device_status = Phase1_Vl53l0xDeviceCodeString(measurement.range_status_code);

  printf("[vl53l0x] range=%u mm status_raw=0x%02X device_code=%u device=%s",
         measurement.range_mm,
         measurement.range_status_raw,
         measurement.range_status_code,
         device_status);

  if (has_delta)
  {
    printf(" delta=%u", delta_mm);
  }

  printf(" note=%s\r\n", note);

  phase1_vl53l0x_last_range_mm = measurement.range_mm;
  phase1_vl53l0x_has_last_range = true;
}

static void Phase1_LogSensorTelemetry(void)
{
  Phase1_LogMpu6050Data();
  Phase1_LogVl53l0xRange();
}

static void Phase1_LogI2CScan(void)
{
  uint8_t device_count = 0U;
  bool mpu6050_low_found = false;
  bool mpu6050_high_found = false;
  bool vl53l0x_found = false;

  phase1_mpu6050_address = 0U;
  phase1_mpu6050_ready = false;
  phase1_vl53l0x_ready = false;
  phase1_vl53l0x_has_last_range = false;
  phase1_vl53l0x_last_range_mm = 0U;
  app_obstacle_has_last_measurement = false;
  app_obstacle_detected = false;
  app_last_vl53l0x_recovery_tick = HAL_GetTick();
  app_obstacle_last_measurement.range_mm = 0U;
  app_obstacle_last_measurement.range_status_raw = 0U;
  app_obstacle_last_measurement.range_status_code = 0U;

  printf("[i2c] scanning 7-bit addresses on I2C1...\r\n");

  for (uint8_t address = 1U; address < 0x7FU; ++address)
  {
    if (Phase1_IsDeviceReady(address))
    {
      ++device_count;
      printf("[i2c] found device at 0x%02X\r\n", address);

      if (address == MPU6050_I2C_ADDR_AD0_LOW)
      {
        mpu6050_low_found = true;
      }
      else if (address == MPU6050_I2C_ADDR_AD0_HIGH)
      {
        mpu6050_high_found = true;
      }
      else if (address == VL53L0X_I2C_ADDR_DEFAULT)
      {
        vl53l0x_found = true;
      }
    }
  }

  if (device_count == 0U)
  {
    printf("[i2c] no devices acknowledged\r\n");
  }

  if (mpu6050_low_found)
  {
    Phase1_LogMpu6050Probe(MPU6050_I2C_ADDR_AD0_LOW);
  }

  if (mpu6050_high_found)
  {
    Phase1_LogMpu6050Probe(MPU6050_I2C_ADDR_AD0_HIGH);
  }

  if (!mpu6050_low_found && !mpu6050_high_found)
  {
    printf("[mpu6050] not found at 0x68 or 0x69\r\n");
  }
  else
  {
    uint8_t mpu6050_address = mpu6050_low_found ? MPU6050_I2C_ADDR_AD0_LOW : MPU6050_I2C_ADDR_AD0_HIGH;

    if (Phase1_Mpu6050Init(mpu6050_address))
    {
      printf("[mpu6050] init ok at 0x%02X, accel=+/-2g gyro=+/-250dps\r\n", mpu6050_address);
    }
    else
    {
      printf("[mpu6050] init failed at 0x%02X\r\n", mpu6050_address);
    }
  }

  if (vl53l0x_found)
  {
    Phase1_LogVl53l0xProbe(VL53L0X_I2C_ADDR_DEFAULT);

    if (Phase1_Vl53l0xInit(VL53L0X_I2C_ADDR_DEFAULT))
    {
      printf("[vl53l0x] init ok, single-shot ranging ready\r\n");
    }
    else
    {
      printf("[vl53l0x] init failed\r\n");
    }
  }
  else
  {
    printf("[vl53l0x] not found at 0x29\r\n");
  }

  Phase1_LogSensorTelemetry();
}
