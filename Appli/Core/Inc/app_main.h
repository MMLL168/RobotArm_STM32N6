#ifndef APP_MAIN_H
#define APP_MAIN_H

void App_ConfigureSystemIsolation(void);
void App_Init(void);
void App_RunLoopIteration(void);
void App_LPUART1_IRQHandler(void);

#endif