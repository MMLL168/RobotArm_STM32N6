#ifndef ILI9341_H
#define ILI9341_H

#include <stdbool.h>
#include <stdint.h>

#define ILI9341_WIDTH  240U
#define ILI9341_HEIGHT 320U

#define ILI9341_COLOR_BLACK   0x0000U
#define ILI9341_COLOR_WHITE   0xFFFFU
#define ILI9341_COLOR_RED     0xF800U
#define ILI9341_COLOR_GREEN   0x07E0U
#define ILI9341_COLOR_BLUE    0x001FU
#define ILI9341_COLOR_CYAN    0x07FFU
#define ILI9341_COLOR_YELLOW  0xFFE0U
#define ILI9341_COLOR_MAGENTA 0xF81FU
#define ILI9341_COLOR_GRAY    0x8410U
#define ILI9341_COLOR_DARK    0x18E3U
#define ILI9341_COLOR_ORANGE  0xFD20U

bool ILI9341_Init(void);
void ILI9341_SetBacklight(bool enabled);
void ILI9341_FillScreen(uint16_t color);
void ILI9341_FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
void ILI9341_DrawString(uint16_t x, uint16_t y, const char *text, uint16_t foreground, uint16_t background, uint8_t scale);
void ILI9341_DrawRgb565Image(uint16_t x,
                             uint16_t y,
                             uint16_t width,
                             uint16_t height,
                             const uint16_t *pixels,
                             uint16_t stride_pixels);

#endif
