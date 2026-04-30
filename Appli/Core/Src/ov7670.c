/**
 * @file    ov7670.c
 * @brief   OV7670 camera driver
 *
 * Phase 1 – SCCB verification only (no PSSI HAL dependency).
 * Uses raw PSSI registers via CMSIS defines.
 *
 * Pin mapping
 * ─────────────────────────────────────────────────────────
 *  XCLK  : PC9  MCO2 (~16 MHz from HSI/4)
 *  RESET : PD0  active-low
 *  PWDN  : PE0  active-high (drive LOW for normal operation)
 *  SCL   : PH9  I2C1_SCL (shared with MPU6050 / VL53L0X)
 *  SDA   : PC1  I2C1_SDA (shared with MPU6050 / VL53L0X)
 *
 *  PSSI parallel:
 *    PCLK=PG1  HSYNC=PG3  VSYNC=PE6
 *    D0=PD7  D1=PC6  D2=PC8  D3=PE10
 *    D4=PE8  D5=PE4  D6=PF5  D7=PF1
 */

#include "ov7670.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

/* ========================================================================== */
/*  Register addresses                                                         */
/* ========================================================================== */
#define REG_GAIN        0x00U
#define REG_BLUE        0x01U
#define REG_RED         0x02U
#define REG_VREF        0x03U
#define REG_COM1        0x04U
#define REG_COM2        0x09U
#define REG_PID         0x0AU  /* Product ID MSB – reads 0x76 */
#define REG_VER         0x0BU  /* Product ID LSB – reads 0x73 */
#define REG_COM3        0x0CU
#define REG_COM4        0x0DU
#define REG_COM5        0x0EU
#define REG_COM6        0x0FU
#define REG_AECH        0x10U
#define REG_CLKRC       0x11U
#define REG_COM7        0x12U  /* Software reset = bit7 */
#define REG_COM8        0x13U
#define REG_COM9        0x14U
#define REG_COM10       0x15U
#define REG_HSTART      0x17U
#define REG_HSTOP       0x18U
#define REG_VSTRT       0x19U
#define REG_VSTOP       0x1AU
#define REG_MVFP        0x1EU
#define REG_ADCCTR1     0x21U
#define REG_ADCCTR2     0x22U
#define REG_CHLF        0x33U
#define REG_TSLB        0x3AU
#define REG_COM11       0x3BU
#define REG_COM12       0x3CU
#define REG_COM13       0x3DU
#define REG_COM14       0x3EU
#define REG_COM15       0x40U
#define REG_RGB444      0x8CU
#define REG_SCALING_XSC       0x70U
#define REG_SCALING_YSC       0x71U
#define REG_SCALING_DCWCTR    0x72U
#define REG_SCALING_PCLK_DIV  0x73U
#define REG_MTX1        0x4FU
#define REG_MTX2        0x50U
#define REG_MTX3        0x51U
#define REG_MTX4        0x52U
#define REG_MTX5        0x53U
#define REG_MTX6        0x54U
#define REG_MTXS        0x58U
#define REG_SLOP        0x7AU
#define REG_GAM1        0x7BU
#define REG_GAM15       0x89U
#define REG_HREF        0x32U
#define REG_END         0xFFU  /* table sentinel */

/* ========================================================================== */
/*  Init register table – QVGA (320 x 240) RGB565                             */
/* ========================================================================== */
typedef struct { uint8_t reg; uint8_t val; } OV7670_RegEntry;

static const OV7670_RegEntry s_init_regs[] =
{
  { REG_COM7,  0x80 },  /* Software reset */
  { REG_END,   0x00 },  /* Delay sentinel – driver inserts HAL_Delay(30) */
  { REG_CLKRC, 0x01 },  /* PLL x2, prescale /2 → 16 MHz PCLK from 16 MHz XCLK */
  { REG_COM7,  0x04 },  /* QVGA, RGB */
  { REG_COM3,  0x04 },  /* DCW enable */
  { REG_COM14, 0x19 },  /* Manual scaling, PCLK/2 */
  { REG_SCALING_XSC,      0x3A },
  { REG_SCALING_YSC,      0x35 },
  { REG_SCALING_DCWCTR,   0x11 },
  { REG_SCALING_PCLK_DIV, 0xF1 },
  { 0xA2,      0x02 },
  /* RGB565 output */
  { REG_RGB444, 0x00 },
  { REG_COM15,  0xD0 },  /* range 0x00-0xFF, RGB565 */
  /* Window QVGA */
  { REG_HSTART, 0x16 },  { REG_HSTOP, 0x04 },  { REG_HREF, 0x24 },
  { REG_VSTRT,  0x02 },  { REG_VSTOP, 0x7A },  { REG_VREF, 0x0A },
  /* Auto controls */
  { REG_COM8,  0xE7 },
  { REG_GAIN,  0x00 },
  { REG_AECH,  0x00 },
  /* Colour matrix */
  { REG_MTX1, 0x80 }, { REG_MTX2, 0x80 }, { REG_MTX3, 0x00 },
  { REG_MTX4, 0x22 }, { REG_MTX5, 0x5E }, { REG_MTX6, 0x80 },
  { REG_MTXS, 0x9E },
  /* Gamma (abbreviated) */
  { REG_SLOP,  0x20 },
  { REG_GAM1,  0x1C }, { 0x7C, 0x28 }, { 0x7D, 0x3C },
  { 0x7E, 0x55 }, { 0x7F, 0x68 }, { 0x80, 0x76 },
  { 0x81, 0x80 }, { 0x82, 0x88 }, { 0x83, 0x8F },
  { 0x84, 0x96 }, { 0x85, 0xA3 }, { 0x86, 0xAF },
  { 0x87, 0xC4 }, { 0x88, 0xD7 }, { REG_GAM15, 0xE8 },
  /* Misc */
  { REG_COM5,    0x61 }, { REG_COM6, 0x4B },
  { 0x16,        0x02 },
  { REG_MVFP,    0x07 },  /* No flip – change to 0x30 to flip both axes */
  { REG_ADCCTR1, 0x02 }, { REG_ADCCTR2, 0x91 },
  { 0x29,  0x07 }, { REG_CHLF, 0x0B },
  { 0x35,  0x0B }, { 0x36, 0x1D }, { 0x37, 0x71 }, { 0x38, 0x2A },
  { REG_TSLB, 0x0F },
  { REG_COM11, 0xC2 }, { REG_COM12, 0x78 },
  { 0x4D, 0x40 }, { 0x4E, 0x20 },
  { 0x74, 0x19 },
  { 0x8D, 0x4F }, { 0x8E, 0x00 }, { 0x8F, 0x00 }, { 0x90, 0x00 },
  { 0x91, 0x00 }, { 0x96, 0x00 }, { 0x9A, 0x00 },
  { 0xB0, 0x84 }, { 0xB1, 0x0C }, { 0xB2, 0x0E },
  { 0xB3, 0x82 }, { 0xB8, 0x0A },
  { REG_END, 0x00 }  /* End of table */
};

/* ========================================================================== */
/*  Private state                                                              */
/* ========================================================================== */
static bool s_ov7670_ready = false;

/* ========================================================================== */
/*  Private: GPIO init                                                         */
/* ========================================================================== */
static void OV7670_GpioInit(void)
{
  GPIO_InitTypeDef g = {0};

  /* Enable all needed GPIO clocks */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();

  /* ── RESET (PD0) – output, start LOW (hold in reset) ─────────────────── */
  HAL_GPIO_WritePin(OV7670_RESET_GPIO_Port, OV7670_RESET_Pin, GPIO_PIN_RESET);
  g.Pin   = OV7670_RESET_Pin;
  g.Mode  = GPIO_MODE_OUTPUT_PP;
  g.Pull  = GPIO_NOPULL;
  g.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OV7670_RESET_GPIO_Port, &g);

  /* ── PWDN (PE0) – output, LOW = normal operation ─────────────────────── */
  HAL_GPIO_WritePin(OV7670_PWDN_GPIO_Port, OV7670_PWDN_Pin, GPIO_PIN_RESET);
  g.Pin = OV7670_PWDN_Pin;
  HAL_GPIO_Init(OV7670_PWDN_GPIO_Port, &g);

  /* ── PSSI data / sync pins – AF9 ─────────────────────────────────────── */
  g.Mode      = GPIO_MODE_AF_PP;
  g.Pull      = GPIO_NOPULL;
  g.Speed     = GPIO_SPEED_FREQ_HIGH;
  g.Alternate = GPIO_AF9_PSSI;

  /* PG1 = PCLK, PG3 = HSYNC */
  g.Pin = GPIO_PIN_1 | GPIO_PIN_3;
  HAL_GPIO_Init(GPIOG, &g);

  /* PE6=VSYNC, PE4=D5, PE8=D4, PE10=D3 */
  g.Pin = GPIO_PIN_6 | GPIO_PIN_4 | GPIO_PIN_8 | GPIO_PIN_10;
  HAL_GPIO_Init(GPIOE, &g);

  /* PD7 = D0 */
  g.Pin = GPIO_PIN_7;
  HAL_GPIO_Init(GPIOD, &g);

  /* PC6=D1, PC8=D2 */
  g.Pin = GPIO_PIN_6 | GPIO_PIN_8;
  HAL_GPIO_Init(GPIOC, &g);

  /* PF5=D6, PF1=D7 */
  g.Pin = GPIO_PIN_5 | GPIO_PIN_1;
  HAL_GPIO_Init(GPIOF, &g);
}

/* ========================================================================== */
/*  Private: XCLK via MCO2 on PC9                                             */
/* ========================================================================== */
static void OV7670_XclkInit(void)
{
  /* HSI = 64 MHz / 4 = 16 MHz – within OV7670 XCLK 10-48 MHz range */
  HAL_RCC_MCOConfig(RCC_MCO2, RCC_MCO2SOURCE_HSI, RCC_MCODIV_4);
  printf("[cam] XCLK MCO2 PC9 ~16 MHz (HSI/4)\r\n");
}

/* ========================================================================== */
/*  Private: PSSI raw register init (no HAL PSSI needed)                      */
/* ========================================================================== */
static void OV7670_PssiRawInit(void)
{
  /* Enable PSSI peripheral clock */
  __HAL_RCC_DCMI_PSSI_CLK_ENABLE();

  /* Reset peripheral */
  __HAL_RCC_DCMI_PSSI_FORCE_RESET();
  HAL_Delay(1);
  __HAL_RCC_DCMI_PSSI_RELEASE_RESET();

  /* Configure PSSI_CR:
   *   DERDYCFG = 011  (DE+RDY both used as HSYNC+VSYNC)
   *   EDM      = 00   (8-bit data width)
   *   CKPOL    = 0    (data captured on falling edge of PCLK)
   *   DEPOL    = 0    (DE active high = HSYNC active high)
   *   RDYPOL   = 0    (RDY active high = VSYNC active high)
   *   OUTEN    = 0    (input / receive mode)
   *   DMAEN    = 0    (polling mode for now)
   */
  uint32_t cr = (3UL << 18U);  /* DERDYCFG = 011 */
  PSSI->CR = cr;

  /* Enable */
  PSSI->CR = cr | (1UL << 14U);  /* ENABLE = 1 */

  printf("[cam] PSSI raw init ok (polling 8-bit)\r\n");
}

/* ========================================================================== */
/*  Public: SCCB read/write                                                    */
/* ========================================================================== */

bool OV7670_WriteReg(uint8_t reg, uint8_t value)
{
  uint8_t buf[2] = { reg, value };
  return (HAL_I2C_Master_Transmit(OV7670_I2C_HANDLE,
                                  OV7670_SCCB_ADDR_W,
                                  buf, 2U,
                                  OV7670_SCCB_TIMEOUT) == HAL_OK);
}

bool OV7670_ReadReg(uint8_t reg, uint8_t *value)
{
  if (!value) { return false; }
  /* SCCB read = write register address, then repeated-start read */
  if (HAL_I2C_Master_Transmit(OV7670_I2C_HANDLE,
                               OV7670_SCCB_ADDR_W,
                               &reg, 1U,
                               OV7670_SCCB_TIMEOUT) != HAL_OK)
  {
    return false;
  }
  return (HAL_I2C_Master_Receive(OV7670_I2C_HANDLE,
                                 OV7670_SCCB_ADDR_R,
                                 value, 1U,
                                 OV7670_SCCB_TIMEOUT) == HAL_OK);
}

/* ========================================================================== */
/*  Public: OV7670_Init                                                        */
/* ========================================================================== */

bool OV7670_Init(void)
{
  s_ov7670_ready = false;

  /* 1. GPIO */
  OV7670_GpioInit();

  /* 2. XCLK – must be running before SCCB */
  OV7670_XclkInit();
  HAL_Delay(10);

  /* 3. Toggle RESET: hold LOW then release HIGH */
  HAL_GPIO_WritePin(OV7670_RESET_GPIO_Port, OV7670_RESET_Pin, GPIO_PIN_RESET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(OV7670_RESET_GPIO_Port, OV7670_RESET_Pin, GPIO_PIN_SET);
  HAL_Delay(30);

  /* 4. Verify product ID via SCCB */
  uint8_t pid = 0, ver = 0;
  if (!OV7670_ReadReg(REG_PID, &pid) || !OV7670_ReadReg(REG_VER, &ver))
  {
    printf("[cam] OV7670 SCCB no ACK - check wiring SCL=PH9 SDA=PC1\r\n");
    return false;
  }
  printf("[cam] OV7670 PID=0x%02X VER=0x%02X%s\r\n",
         pid, ver, (pid == 0x76U) ? " ok" : " unexpected");

  /* 5. Write register init table */
  uint32_t written = 0U;
  for (uint32_t i = 0U; ; i++)
  {
    uint8_t r = s_init_regs[i].reg;
    uint8_t v = s_init_regs[i].val;

    if (r == REG_END)
    {
      if (written == 0U) { break; }  /* actual end sentinel */
      /* delay sentinel (REG_END used as pseudo-NOP in first entry) */
      HAL_Delay(30);
      continue;
    }
    if (!OV7670_WriteReg(r, v))
    {
      printf("[cam] reg write failed reg=0x%02X val=0x%02X\r\n", r, v);
      return false;
    }
    written++;
  }
  printf("[cam] OV7670 %lu registers written\r\n", written);

  /* 6. PSSI peripheral (raw register) */
  OV7670_PssiRawInit();

  s_ov7670_ready = true;
  printf("[cam] OV7670 ready QVGA RGB565 %ux%u\r\n",
         OV7670_FRAME_WIDTH, OV7670_FRAME_HEIGHT);
  return true;
}

bool OV7670_IsReady(void)
{
  return s_ov7670_ready;
}

/* ========================================================================== */
/*  Public: OV7670_CaptureFrame (polling, blocking)                           */
/* ========================================================================== */
bool OV7670_CaptureFrame(uint8_t *frame_buf, uint32_t buf_len)
{
  if (!s_ov7670_ready || !frame_buf || buf_len < OV7670_FRAME_BYTES)
  {
    return false;
  }

  uint32_t total  = OV7670_FRAME_BYTES;
  uint32_t i      = 0U;
  uint32_t t_start = HAL_GetTick();

  /* Wait for VSYNC rising edge (start of frame) */
  while (!(PSSI->SR & (1UL << 3U)))  /* RTT4B bit – PSSI ready-to-transmit */
  {
    if ((HAL_GetTick() - t_start) > 1000U)
    {
      printf("[cam] VSYNC timeout\r\n");
      return false;
    }
  }

  /* Read bytes from PSSI data register */
  while (i < total)
  {
    if (PSSI->SR & (1UL << 2U))  /* RTT4B = data available */
    {
      frame_buf[i++] = (uint8_t)(PSSI->DR & 0xFFU);
    }
    if ((HAL_GetTick() - t_start) > 3000U)
    {
      printf("[cam] frame capture timeout at byte %lu/%lu\r\n", i, total);
      return false;
    }
  }

  return true;
}
