#ifndef OV7670_H
#define OV7670_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32n6xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

/* --------------------------------------------------------------------------
 * Hardware mapping
 * -------------------------------------------------------------------------- */
/* SCCB / I2C – shared with MPU6050 and VL53L0X on I2C1 */
extern I2C_HandleTypeDef hi2c1;
#define OV7670_I2C_HANDLE     (&hi2c1)
#define OV7670_SCCB_ADDR_W    0x42U   /* 8-bit write address */
#define OV7670_SCCB_ADDR_R    0x43U   /* 8-bit read  address */
#define OV7670_SCCB_TIMEOUT   50U     /* ms */

/* RESET  – PD0 */
#define OV7670_RESET_GPIO_Port  GPIOD
#define OV7670_RESET_Pin        GPIO_PIN_0

/* PWDN   – PE0 (or tie to GND) */
#define OV7670_PWDN_GPIO_Port   GPIOE
#define OV7670_PWDN_Pin         GPIO_PIN_0

/* XCLK   – PC9 via MCO2 at 24 MHz */
/* Data pins (PSSI / DCMI parallel interface) */
/*   D0 = PD7   D1 = PC6   D2 = PC8   D3 = PE10
     D4 = PE8   D5 = PE4   D6 = PF5   D7 = PF1
     PCLK = PG1  HSYNC = PG3  VSYNC = PE6            */

/* --------------------------------------------------------------------------
 * Frame-buffer settings
 * QVGA RGB565 = 320 x 240 x 2 bytes = 153 600 bytes
 * -------------------------------------------------------------------------- */
#define OV7670_FRAME_WIDTH   320U
#define OV7670_FRAME_HEIGHT  240U
#define OV7670_BYTES_PER_PIX 2U   /* RGB565 */
#define OV7670_FRAME_BYTES   (OV7670_FRAME_WIDTH * OV7670_FRAME_HEIGHT * OV7670_BYTES_PER_PIX)

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */
bool OV7670_Init(void);
bool OV7670_CaptureFrame(uint8_t *frame_buf, uint32_t buf_len);
bool OV7670_IsReady(void);
bool OV7670_WriteReg(uint8_t reg, uint8_t value);
bool OV7670_ReadReg(uint8_t reg, uint8_t *value);

#ifdef __cplusplus
}
#endif
#endif /* OV7670_H */
