#include "app_main.h"

#include "main.h"
#include "tim.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define APP_I2C1_SDA_Pin GPIO_PIN_1
#define APP_I2C1_SDA_GPIO_Port GPIOC
#define APP_I2C1_SDA_AF GPIO_AF4_I2C1
#define APP_I2C1_SCL_Pin GPIO_PIN_9
#define APP_I2C1_SCL_GPIO_Port GPIOH
#define APP_I2C1_SCL_AF GPIO_AF4_I2C1
#define APP_LD1_BLUE_Pin GPIO_PIN_8
#define APP_LD1_BLUE_GPIO_Port GPIOG
#define APP_LD2_RED_Pin GPIO_PIN_10
#define APP_LD2_RED_GPIO_Port GPIOG
#define APP_LD3_GREEN_Pin GPIO_PIN_0
#define APP_LD3_GREEN_GPIO_Port GPIOG
#define APP_USER_BUTTON_Pin GPIO_PIN_13
#define APP_USER_BUTTON_GPIO_Port GPIOC
#define APP_VCP_TX_Pin GPIO_PIN_5
#define APP_VCP_TX_GPIO_Port GPIOE
#define APP_VCP_TX_AF GPIO_AF3_LPUART1
#define APP_VCP_RX_Pin GPIO_PIN_6
#define APP_VCP_RX_GPIO_Port GPIOE
#define APP_VCP_RX_AF GPIO_AF3_LPUART1

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
#define APP_SERVO_SAFE_PULSE_US 1500U
#define APP_SERVO_OPEN_PULSE_US 700U
#define APP_SERVO_CLOSE_PULSE_US 1600U

typedef enum
{
  APP_SERVO_STATE_SAFE = 0,
  APP_SERVO_STATE_OPEN,
  APP_SERVO_STATE_CLOSE
} App_ServoState;

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

static I2C_HandleTypeDef hi2c1;
static UART_HandleTypeDef hlpuart1;
static uint8_t phase1_mpu6050_address = 0U;
static bool phase1_mpu6050_ready = false;
static bool phase1_vl53l0x_ready = false;
static uint8_t phase1_vl53l0x_stop_variable = 0U;
static bool phase1_vl53l0x_has_last_range = false;
static uint16_t phase1_vl53l0x_last_range_mm = 0U;
static uint32_t app_last_blink_tick = 0U;
static uint32_t app_last_heartbeat_tick = 0U;
static uint32_t app_last_sensor_tick = 0U;
static uint32_t app_last_button_toggle_tick = 0U;
static bool app_button_pressed = false;
static bool app_last_button_pressed = false;
static App_ServoState app_servo_state = APP_SERVO_STATE_SAFE;
static uint16_t app_servo_current_pulse_us = APP_SERVO_SAFE_PULSE_US;

static void App_LPUART1_UART_Init(void);
static void App_I2C1_Init(void);
static void App_GPIO_Init(void);
static bool App_ServoConfigureTiming(void);
static bool App_ServoInit(void);
static void App_ServoApplyPulse(uint16_t pulse_width_us);
static void App_ServoToggleState(void);
static bool Phase0_IsButtonPressed(void);
static void Phase0_SetLedState(GPIO_TypeDef *gpio_port, uint16_t gpio_pin, bool on);
static void Phase0_LogStartup(bool button_pressed);
static bool Phase1_IsDeviceReady(uint8_t address);
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
static bool Phase1_Vl53l0xReadMeasurement(uint8_t address, Phase1_Vl53l0xMeasurement *measurement);
static const char *Phase1_Vl53l0xDeviceCodeString(uint8_t device_code);
static const char *Phase1_Vl53l0xMeasurementNote(const Phase1_Vl53l0xMeasurement *measurement, bool has_delta, uint16_t delta_mm);
static void Phase1_LogVl53l0xRange(void);
static void Phase1_LogI2CScan(void);
static void Phase1_LogSensorTelemetry(void);

int __io_putchar(int ch)
{
  uint8_t data = (uint8_t)ch;
  HAL_UART_Transmit(&hlpuart1, &data, 1U, HAL_MAX_DELAY);
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
  App_GPIO_Init();
  App_LPUART1_UART_Init();
  App_I2C1_Init();

  if (!App_ServoInit())
  {
    Error_Handler();
  }

  app_button_pressed = Phase0_IsButtonPressed();
  app_last_button_pressed = app_button_pressed;
  app_last_blink_tick = HAL_GetTick();
  app_last_heartbeat_tick = app_last_blink_tick;
  app_last_sensor_tick = app_last_blink_tick;
  app_last_button_toggle_tick = 0U;

  Phase0_SetLedState(APP_LD1_BLUE_GPIO_Port, APP_LD1_BLUE_Pin, true);
  Phase0_SetLedState(APP_LD2_RED_GPIO_Port, APP_LD2_RED_Pin, app_button_pressed);
  Phase0_LogStartup(app_button_pressed);
  Phase1_LogI2CScan();
}

void App_RunLoopIteration(void)
{
  uint32_t now = HAL_GetTick();

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
      printf("[button] pressed\r\n");
      if ((now - app_last_button_toggle_tick) >= APP_BUTTON_TOGGLE_DEBOUNCE_MS)
      {
        app_last_button_toggle_tick = now;
        App_ServoToggleState();
      }
    }
    else
    {
      printf("[button] released\r\n");
    }
  }

  if ((now - app_last_heartbeat_tick) >= 1000U)
  {
    app_last_heartbeat_tick = now;
    printf("[tick] %lu ms, button=%s, servo=%u us, state=%s\r\n",
           (unsigned long)now,
           app_button_pressed ? "pressed" : "released",
           app_servo_current_pulse_us,
           app_servo_state == APP_SERVO_STATE_CLOSE ? "close" :
           (app_servo_state == APP_SERVO_STATE_OPEN ? "open" : "safe"));
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

  App_ServoApplyPulse(APP_SERVO_OPEN_PULSE_US);

  if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1) != HAL_OK)
  {
    return false;
  }

  app_servo_state = APP_SERVO_STATE_OPEN;

  printf("[servo] TIM1 CH1 started, %u Hz period=%u us safe=%u us\r\n",
         APP_SERVO_PWM_FREQUENCY_HZ,
         APP_SERVO_PERIOD_US,
         APP_SERVO_SAFE_PULSE_US);
    printf("[servo] action mode: press button to toggle release/clamp open=%u us close=%u us\r\n",
         APP_SERVO_OPEN_PULSE_US,
         APP_SERVO_CLOSE_PULSE_US);
    printf("[servo] startup state=open pulse=%u us\r\n", APP_SERVO_OPEN_PULSE_US);

  return true;
}

static bool App_ServoConfigureTiming(void)
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

  __HAL_TIM_DISABLE(&htim1);

  htim1.Init.Prescaler = prescaler_value;
  htim1.Init.Period = APP_SERVO_PERIOD_US - 1U;

  __HAL_TIM_SET_PRESCALER(&htim1, prescaler_value);
  __HAL_TIM_SET_AUTORELOAD(&htim1, APP_SERVO_PERIOD_US - 1U);
  __HAL_TIM_SET_COUNTER(&htim1, 0U);
  htim1.Instance->EGR = TIM_EGR_UG;

  printf("[servo] timg=%lu Hz prescaler=%lu arr=%u\r\n",
         (unsigned long)tim_clock_hz,
         (unsigned long)prescaler_value,
         (unsigned int)(APP_SERVO_PERIOD_US - 1U));

  return true;
}

static void App_ServoApplyPulse(uint16_t pulse_width_us)
{
  app_servo_current_pulse_us = pulse_width_us;
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pulse_width_us);
}

static void App_ServoToggleState(void)
{
  if (app_servo_state == APP_SERVO_STATE_CLOSE)
  {
    app_servo_state = APP_SERVO_STATE_OPEN;
    App_ServoApplyPulse(APP_SERVO_OPEN_PULSE_US);
    printf("[servo] toggle open=%u us\r\n", APP_SERVO_OPEN_PULSE_US);
  }
  else
  {
    app_servo_state = APP_SERVO_STATE_CLOSE;
    App_ServoApplyPulse(APP_SERVO_CLOSE_PULSE_US);
    printf("[servo] toggle close=%u us\r\n", APP_SERVO_CLOSE_PULSE_US);
  }
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  if (hi2c->Instance == I2C1)
  {
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
    PeriphClkInitStruct.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();

    GPIO_InitStruct.Pin = APP_I2C1_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = APP_I2C1_SDA_AF;
    HAL_GPIO_Init(APP_I2C1_SDA_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = APP_I2C1_SCL_Pin;
    GPIO_InitStruct.Alternate = APP_I2C1_SCL_AF;
    HAL_GPIO_Init(APP_I2C1_SCL_GPIO_Port, &GPIO_InitStruct);
  }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
  if (hi2c->Instance == I2C1)
  {
    HAL_GPIO_DeInit(APP_I2C1_SDA_GPIO_Port, APP_I2C1_SDA_Pin);
    HAL_GPIO_DeInit(APP_I2C1_SCL_GPIO_Port, APP_I2C1_SCL_Pin);
    __HAL_RCC_I2C1_CLK_DISABLE();
  }
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  if (huart->Instance == LPUART1)
  {
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_LPUART1_CLK_ENABLE();

    GPIO_InitStruct.Pin = APP_VCP_TX_Pin | APP_VCP_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = APP_VCP_TX_AF;
    HAL_GPIO_Init(APP_VCP_TX_GPIO_Port, &GPIO_InitStruct);
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
  if (huart->Instance == LPUART1)
  {
    HAL_GPIO_DeInit(APP_VCP_TX_GPIO_Port, APP_VCP_TX_Pin | APP_VCP_RX_Pin);
    __HAL_RCC_LPUART1_CLK_DISABLE();
  }
}

static void App_LPUART1_UART_Init(void)
{
  hlpuart1.Instance = LPUART1;
  hlpuart1.Init.BaudRate = 115200;
  hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
  hlpuart1.Init.StopBits = UART_STOPBITS_1;
  hlpuart1.Init.Parity = UART_PARITY_NONE;
  hlpuart1.Init.Mode = UART_MODE_TX_RX;
  hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hlpuart1.Init.OverSampling = UART_OVERSAMPLING_8;
  hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  hlpuart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

  if (HAL_UART_Init(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_UARTEx_SetTxFifoThreshold(&hlpuart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_UARTEx_SetRxFifoThreshold(&hlpuart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_UARTEx_DisableFifoMode(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }
}

static void App_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x30C0EDFF;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
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
  printf("[phase0] uart: LPUART1 115200 8N1 on PE5/PE6\r\n");
  printf("[phase0] led: LD3 heartbeat, LD2 mirrors button\r\n");
  printf("[phase0] button: initial state is %s\r\n", button_pressed ? "pressed" : "released");
}

static bool Phase1_IsDeviceReady(uint8_t address)
{
  return HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(address << 1), 2U, 10U) == HAL_OK;
}

static bool Phase1_WriteRegister8(uint8_t address, uint8_t register_address, uint8_t value)
{
  return HAL_I2C_Mem_Write(&hi2c1,
                           (uint16_t)(address << 1),
                           register_address,
                           I2C_MEMADD_SIZE_8BIT,
                           &value,
                           1U,
                           PHASE1_I2C_TIMEOUT_MS) == HAL_OK;
}

static bool Phase1_ReadRegister8(uint8_t address, uint8_t register_address, uint8_t *value)
{
  return Phase1_ReadRegisters(address, register_address, value, 1U);
}

static bool Phase1_ReadRegisters(uint8_t address, uint8_t register_address, uint8_t *buffer, uint16_t length)
{
  return HAL_I2C_Mem_Read(&hi2c1,
                          (uint16_t)(address << 1),
                          register_address,
                          I2C_MEMADD_SIZE_8BIT,
                          buffer,
                          length,
                          PHASE1_I2C_TIMEOUT_MS) == HAL_OK;
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

  if (!Phase1_ReadRegister8(address, VL53L0X_MODEL_ID_REG, &value) || value != VL53L0X_MODEL_ID_VALUE)
  {
    return false;
  }

  if (!Phase1_ReadRegister8(address, VL53L0X_VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV_REG, &value))
  {
    return false;
  }

  if (!Phase1_WriteRegister8(address, VL53L0X_VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV_REG, (uint8_t)(value | 0x01U)))
  {
    return false;
  }

  if (!Phase1_WriteRegister8(address, VL53L0X_PRIVATE_STANDARD_MODE_REG, 0x00U) ||
      !Phase1_WriteRegister8(address, VL53L0X_PRIVATE_INIT_REG, 0x01U) ||
      !Phase1_WriteRegister8(address, VL53L0X_PRIVATE_PAGE_SELECT_REG, 0x01U) ||
      !Phase1_WriteRegister8(address, 0x00U, 0x00U))
  {
    return false;
  }

  if (!Phase1_ReadRegister8(address, VL53L0X_PRIVATE_STOP_VARIABLE_REG, &phase1_vl53l0x_stop_variable))
  {
    return false;
  }

  if (!Phase1_WriteRegister8(address, 0x00U, 0x01U) ||
      !Phase1_WriteRegister8(address, VL53L0X_PRIVATE_PAGE_SELECT_REG, 0x00U) ||
      !Phase1_WriteRegister8(address, VL53L0X_PRIVATE_INIT_REG, 0x00U) ||
      !Phase1_WriteRegister8(address, VL53L0X_SYSTEM_INTERRUPT_CONFIG_GPIO_REG, 0x04U))
  {
    return false;
  }

  if (!Phase1_ReadRegister8(address, VL53L0X_GPIO_HV_MUX_ACTIVE_HIGH_REG, &value))
  {
    return false;
  }

  if (!Phase1_WriteRegister8(address, VL53L0X_GPIO_HV_MUX_ACTIVE_HIGH_REG, (uint8_t)(value & ~0x10U)) ||
      !Phase1_WriteRegister8(address, VL53L0X_SYSTEM_INTERRUPT_CLEAR_REG, 0x01U))
  {
    return false;
  }

  phase1_vl53l0x_ready = true;
  return true;
}

static bool Phase1_Vl53l0xReadMeasurement(uint8_t address, Phase1_Vl53l0xMeasurement *measurement)
{
  uint8_t status = 0U;
  uint32_t start_tick = HAL_GetTick();

  if (!Phase1_Vl53l0xPrepareSingleShot(address) ||
      !Phase1_WriteRegister8(address, VL53L0X_SYSRANGE_START_REG, 0x01U))
  {
    return false;
  }

  while (true)
  {
    if (!Phase1_ReadRegister8(address, VL53L0X_SYSRANGE_START_REG, &status))
    {
      return false;
    }

    if ((status & 0x01U) == 0U)
    {
      break;
    }

    if ((HAL_GetTick() - start_tick) >= PHASE1_VL53L0X_MEASUREMENT_TIMEOUT_MS)
    {
      return false;
    }
  }

  start_tick = HAL_GetTick();
  while (true)
  {
    if (!Phase1_ReadRegister8(address, VL53L0X_RESULT_INTERRUPT_STATUS_REG, &status))
    {
      return false;
    }

    if ((status & 0x07U) != 0U)
    {
      break;
    }

    if ((HAL_GetTick() - start_tick) >= PHASE1_VL53L0X_MEASUREMENT_TIMEOUT_MS)
    {
      return false;
    }
  }

  if (!Phase1_ReadRegister8(address, VL53L0X_RESULT_RANGE_STATUS_REG, &measurement->range_status_raw) ||
      !Phase1_ReadRegister16(address, (uint8_t)(VL53L0X_RESULT_RANGE_STATUS_REG + 10U), &measurement->range_mm) ||
      !Phase1_WriteRegister8(address, VL53L0X_SYSTEM_INTERRUPT_CLEAR_REG, 0x01U))
  {
    return false;
  }

  measurement->range_status_code = (uint8_t)((measurement->range_status_raw >> 3) & 0x0FU);

  return true;
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
    printf("[vl53l0x] single-shot read failed\r\n");
    return;
  }

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