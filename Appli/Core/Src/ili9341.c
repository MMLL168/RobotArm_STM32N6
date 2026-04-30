#include "ili9341.h"

#include "main.h"
#include "spi.h"

#include <stddef.h>
#include <string.h>

#define ILI9341_CMD_SWRESET 0x01U
#define ILI9341_CMD_SLPOUT  0x11U
#define ILI9341_CMD_DISPON  0x29U
#define ILI9341_CMD_CASET   0x2AU
#define ILI9341_CMD_PASET   0x2BU
#define ILI9341_CMD_RAMWR   0x2CU
#define ILI9341_CMD_MADCTL  0x36U
#define ILI9341_CMD_PIXFMT  0x3AU
#define ILI9341_CMD_INVON   0x21U

#define ILI9341_SPI_TIMEOUT_MS 100U

typedef struct
{
  char character;
  uint8_t columns[5];
} Ili9341Glyph;

static const Ili9341Glyph glyphs[] = {
  { ' ', { 0x00, 0x00, 0x00, 0x00, 0x00 } },
  { '-', { 0x08, 0x08, 0x08, 0x08, 0x08 } },
  { '.', { 0x00, 0x60, 0x60, 0x00, 0x00 } },
  { ':', { 0x00, 0x36, 0x36, 0x00, 0x00 } },
  { '/', { 0x40, 0x30, 0x08, 0x06, 0x01 } },
  { '+', { 0x08, 0x08, 0x3E, 0x08, 0x08 } },
  { '%', { 0x63, 0x13, 0x08, 0x64, 0x63 } },
  { '0', { 0x3E, 0x51, 0x49, 0x45, 0x3E } },
  { '1', { 0x00, 0x42, 0x7F, 0x40, 0x00 } },
  { '2', { 0x42, 0x61, 0x51, 0x49, 0x46 } },
  { '3', { 0x21, 0x41, 0x45, 0x4B, 0x31 } },
  { '4', { 0x18, 0x14, 0x12, 0x7F, 0x10 } },
  { '5', { 0x27, 0x45, 0x45, 0x45, 0x39 } },
  { '6', { 0x3C, 0x4A, 0x49, 0x49, 0x30 } },
  { '7', { 0x01, 0x71, 0x09, 0x05, 0x03 } },
  { '8', { 0x36, 0x49, 0x49, 0x49, 0x36 } },
  { '9', { 0x06, 0x49, 0x49, 0x29, 0x1E } },
  { 'A', { 0x7E, 0x11, 0x11, 0x11, 0x7E } },
  { 'B', { 0x7F, 0x49, 0x49, 0x49, 0x36 } },
  { 'C', { 0x3E, 0x41, 0x41, 0x41, 0x22 } },
  { 'D', { 0x7F, 0x41, 0x41, 0x22, 0x1C } },
  { 'E', { 0x7F, 0x49, 0x49, 0x49, 0x41 } },
  { 'F', { 0x7F, 0x09, 0x09, 0x09, 0x01 } },
  { 'G', { 0x3E, 0x41, 0x49, 0x49, 0x7A } },
  { 'H', { 0x7F, 0x08, 0x08, 0x08, 0x7F } },
  { 'I', { 0x00, 0x41, 0x7F, 0x41, 0x00 } },
  { 'J', { 0x20, 0x40, 0x41, 0x3F, 0x01 } },
  { 'K', { 0x7F, 0x08, 0x14, 0x22, 0x41 } },
  { 'L', { 0x7F, 0x40, 0x40, 0x40, 0x40 } },
  { 'M', { 0x7F, 0x02, 0x0C, 0x02, 0x7F } },
  { 'N', { 0x7F, 0x04, 0x08, 0x10, 0x7F } },
  { 'O', { 0x3E, 0x41, 0x41, 0x41, 0x3E } },
  { 'P', { 0x7F, 0x09, 0x09, 0x09, 0x06 } },
  { 'Q', { 0x3E, 0x41, 0x51, 0x21, 0x5E } },
  { 'R', { 0x7F, 0x09, 0x19, 0x29, 0x46 } },
  { 'S', { 0x46, 0x49, 0x49, 0x49, 0x31 } },
  { 'T', { 0x01, 0x01, 0x7F, 0x01, 0x01 } },
  { 'U', { 0x3F, 0x40, 0x40, 0x40, 0x3F } },
  { 'V', { 0x1F, 0x20, 0x40, 0x20, 0x1F } },
  { 'W', { 0x3F, 0x40, 0x38, 0x40, 0x3F } },
  { 'X', { 0x63, 0x14, 0x08, 0x14, 0x63 } },
  { 'Y', { 0x07, 0x08, 0x70, 0x08, 0x07 } },
  { 'Z', { 0x61, 0x51, 0x49, 0x45, 0x43 } },
  { '_', { 0x40, 0x40, 0x40, 0x40, 0x40 } },
};

static void ILI9341_Select(void);
static void ILI9341_Unselect(void);
static void ILI9341_WriteCommand(uint8_t command);
static void ILI9341_WriteData(const uint8_t *data, uint16_t length);
static void ILI9341_WriteDataByte(uint8_t data);
static void ILI9341_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
static const uint8_t *ILI9341_FindGlyph(char character);
static void ILI9341_DrawChar(uint16_t x, uint16_t y, char character, uint16_t foreground, uint16_t background, uint8_t scale);

bool ILI9341_Init(void)
{
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
  ILI9341_SetBacklight(true);

  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
  HAL_Delay(20U);
  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(120U);

  ILI9341_WriteCommand(ILI9341_CMD_SWRESET);
  HAL_Delay(120U);
  ILI9341_WriteCommand(ILI9341_CMD_SLPOUT);
  HAL_Delay(120U);

  ILI9341_WriteCommand(ILI9341_CMD_PIXFMT);
  ILI9341_WriteDataByte(0x55U);

  ILI9341_WriteCommand(ILI9341_CMD_MADCTL);
  ILI9341_WriteDataByte(0x88U);

  ILI9341_WriteCommand(ILI9341_CMD_INVON);
  ILI9341_WriteCommand(ILI9341_CMD_DISPON);
  HAL_Delay(20U);

  ILI9341_FillScreen(ILI9341_COLOR_BLACK);
  return true;
}

void ILI9341_SetBacklight(bool enabled)
{
  HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, enabled ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void ILI9341_FillScreen(uint16_t color)
{
  ILI9341_FillRect(0U, 0U, ILI9341_WIDTH, ILI9341_HEIGHT, color);
}

void ILI9341_FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
  static uint8_t buffer[512];
  uint32_t pixel_count;

  if (x >= ILI9341_WIDTH || y >= ILI9341_HEIGHT || width == 0U || height == 0U)
  {
    return;
  }

  if ((uint32_t)x + width > ILI9341_WIDTH)
  {
    width = (uint16_t)(ILI9341_WIDTH - x);
  }

  if ((uint32_t)y + height > ILI9341_HEIGHT)
  {
    height = (uint16_t)(ILI9341_HEIGHT - y);
  }

  pixel_count = (uint32_t)width * height;

  for (size_t index = 0U; index < sizeof(buffer); index += 2U)
  {
    buffer[index] = (uint8_t)(color >> 8);
    buffer[index + 1U] = (uint8_t)color;
  }

  ILI9341_SetAddressWindow(x, y, (uint16_t)(x + width - 1U), (uint16_t)(y + height - 1U));
  HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
  ILI9341_Select();
  while (pixel_count > 0U)
  {
    uint16_t chunk_pixels = pixel_count > (sizeof(buffer) / 2U) ? (uint16_t)(sizeof(buffer) / 2U) : (uint16_t)pixel_count;
    HAL_SPI_Transmit(&hspi5, buffer, (uint16_t)(chunk_pixels * 2U), ILI9341_SPI_TIMEOUT_MS);
    pixel_count -= chunk_pixels;
  }
  ILI9341_Unselect();
}

void ILI9341_DrawString(uint16_t x, uint16_t y, const char *text, uint16_t foreground, uint16_t background, uint8_t scale)
{
  uint16_t cursor_x = x;

  if (text == NULL)
  {
    return;
  }

  while (*text != '\0')
  {
    if (*text == '\n')
    {
      cursor_x = x;
      y = (uint16_t)(y + (uint16_t)(8U * scale));
    }
    else
    {
      ILI9341_DrawChar(cursor_x, y, *text, foreground, background, scale);
      cursor_x = (uint16_t)(cursor_x + (uint16_t)(6U * scale));
    }
    ++text;
  }
}

void ILI9341_DrawRgb565Image(uint16_t x,
                             uint16_t y,
                             uint16_t width,
                             uint16_t height,
                             const uint16_t *pixels,
                             uint16_t stride_pixels)
{
  uint8_t line_buffer[ILI9341_WIDTH * 2U];

  if (pixels == NULL || x >= ILI9341_WIDTH || y >= ILI9341_HEIGHT || width == 0U || height == 0U)
  {
    return;
  }

  if (width > ILI9341_WIDTH)
  {
    width = ILI9341_WIDTH;
  }

  if ((uint32_t)x + width > ILI9341_WIDTH)
  {
    width = (uint16_t)(ILI9341_WIDTH - x);
  }

  if ((uint32_t)y + height > ILI9341_HEIGHT)
  {
    height = (uint16_t)(ILI9341_HEIGHT - y);
  }

  if (stride_pixels < width)
  {
    stride_pixels = width;
  }

  ILI9341_SetAddressWindow(x, y, (uint16_t)(x + width - 1U), (uint16_t)(y + height - 1U));
  HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
  ILI9341_Select();
  for (uint16_t row = 0U; row < height; ++row)
  {
    const uint16_t *source = &pixels[(uint32_t)row * stride_pixels];
    for (uint16_t column = 0U; column < width; ++column)
    {
      uint16_t color = source[column];
      line_buffer[(uint32_t)column * 2U] = (uint8_t)(color >> 8);
      line_buffer[(uint32_t)column * 2U + 1U] = (uint8_t)color;
    }
    HAL_SPI_Transmit(&hspi5, line_buffer, (uint16_t)(width * 2U), ILI9341_SPI_TIMEOUT_MS);
  }
  ILI9341_Unselect();
}

static void ILI9341_Select(void)
{
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
}

static void ILI9341_Unselect(void)
{
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

static void ILI9341_WriteCommand(uint8_t command)
{
  HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);
  ILI9341_Select();
  HAL_SPI_Transmit(&hspi5, &command, 1U, ILI9341_SPI_TIMEOUT_MS);
  ILI9341_Unselect();
}

static void ILI9341_WriteData(const uint8_t *data, uint16_t length)
{
  HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
  ILI9341_Select();
  HAL_SPI_Transmit(&hspi5, (uint8_t *)data, length, ILI9341_SPI_TIMEOUT_MS);
  ILI9341_Unselect();
}

static void ILI9341_WriteDataByte(uint8_t data)
{
  ILI9341_WriteData(&data, 1U);
}

static void ILI9341_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  uint8_t data[4];

  ILI9341_WriteCommand(ILI9341_CMD_CASET);
  data[0] = (uint8_t)(x0 >> 8);
  data[1] = (uint8_t)x0;
  data[2] = (uint8_t)(x1 >> 8);
  data[3] = (uint8_t)x1;
  ILI9341_WriteData(data, sizeof(data));

  ILI9341_WriteCommand(ILI9341_CMD_PASET);
  data[0] = (uint8_t)(y0 >> 8);
  data[1] = (uint8_t)y0;
  data[2] = (uint8_t)(y1 >> 8);
  data[3] = (uint8_t)y1;
  ILI9341_WriteData(data, sizeof(data));

  ILI9341_WriteCommand(ILI9341_CMD_RAMWR);
}

static const uint8_t *ILI9341_FindGlyph(char character)
{
  if (character >= 'a' && character <= 'z')
  {
    character = (char)(character - 'a' + 'A');
  }

  for (size_t index = 0U; index < sizeof(glyphs) / sizeof(glyphs[0]); ++index)
  {
    if (glyphs[index].character == character)
    {
      return glyphs[index].columns;
    }
  }

  return glyphs[0].columns;
}

static void ILI9341_DrawChar(uint16_t x, uint16_t y, char character, uint16_t foreground, uint16_t background, uint8_t scale)
{
  const uint8_t *glyph = ILI9341_FindGlyph(character);
  static uint8_t char_buffer[1536]; /* max scale 4: 24x32 pixels * 2 = 1536 bytes */
  uint16_t char_width;
  uint16_t char_height;
  uint8_t fg_high;
  uint8_t fg_low;
  uint8_t bg_high;
  uint8_t bg_low;

  if (scale == 0U)
  {
    scale = 1U;
  }
  if (scale > 4U)
  {
    scale = 4U;
  }

  char_width = (uint16_t)(6U * scale);
  char_height = (uint16_t)(8U * scale);

  if (x >= ILI9341_WIDTH || y >= ILI9341_HEIGHT)
  {
    return;
  }
  if ((uint32_t)x + char_width > ILI9341_WIDTH)
  {
    char_width = (uint16_t)(ILI9341_WIDTH - x);
  }
  if ((uint32_t)y + char_height > ILI9341_HEIGHT)
  {
    char_height = (uint16_t)(ILI9341_HEIGHT - y);
  }

  fg_high = (uint8_t)(foreground >> 8);
  fg_low = (uint8_t)foreground;
  bg_high = (uint8_t)(background >> 8);
  bg_low = (uint8_t)background;

  ILI9341_SetAddressWindow(x, y, (uint16_t)(x + char_width - 1U), (uint16_t)(y + char_height - 1U));
  HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
  ILI9341_Select();

  for (uint16_t output_row = 0U; output_row < char_height; ++output_row)
  {
    uint8_t source_row = (uint8_t)(output_row / scale);
    for (uint16_t output_col = 0U; output_col < char_width; ++output_col)
    {
      uint8_t source_col = (uint8_t)(output_col / scale);
      uint8_t bits = source_col < 5U ? glyph[source_col] : 0U;
      bool pixel_on = (bits & (1U << source_row)) != 0U;
      uint32_t index = (output_row * char_width + output_col) * 2U;
      char_buffer[index] = pixel_on ? fg_high : bg_high;
      char_buffer[index + 1U] = pixel_on ? fg_low : bg_low;
    }
  }
  HAL_SPI_Transmit(&hspi5, char_buffer, (uint16_t)(char_width * char_height * 2U), ILI9341_SPI_TIMEOUT_MS);

  ILI9341_Unselect();
}
