#include "main.h"
#include "app_main.h"

/*
 * User-owned IRQ forwarding stubs live outside CubeMX-generated files so
 * interrupt glue and defensive handlers survive regenerate.
 */
static void App_DisableUnexpectedXspiIrq(IRQn_Type irqn)
{
  HAL_NVIC_DisableIRQ(irqn);
  NVIC_ClearPendingIRQ(irqn);
  __DSB();
  __ISB();
}

void LPUART1_IRQHandler(void)
{
  App_LPUART1_IRQHandler();
}

void XSPI1_IRQHandler(void)
{
  App_DisableUnexpectedXspiIrq(XSPI1_IRQn);
}

void XSPI2_IRQHandler(void)
{
  App_DisableUnexpectedXspiIrq(XSPI2_IRQn);
}

void XSPI3_IRQHandler(void)
{
  App_DisableUnexpectedXspiIrq(XSPI3_IRQn);
}