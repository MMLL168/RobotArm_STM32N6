#ifndef APP_MAIN_H
#define APP_MAIN_H

#include <stdint.h>

void App_ConfigureSystemIsolation(void);
void App_Init(void);
void App_RunLoopIteration(void);
void App_USART3_IRQHandler(void);
void App_DisplayCameraFrameRgb565(const uint16_t *pixels, uint16_t width, uint16_t height, uint16_t stride_pixels);

#endif
