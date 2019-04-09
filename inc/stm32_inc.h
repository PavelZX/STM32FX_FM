#ifndef STM32_INC_H
#define STM32_INC_H

#if defined(STM32F429xx)
#include "ISRstm32f429xx.h"
#elif defined(STM32F407xx)
#include "ISRstm32f407xx.h"
#elif defined(STM32F100xB)
#include "ISRstm32f10x_md.h"
#endif
#include "stm32_conf.h"
#include "stm32_def.h"

#include "stm32_flash.h"
#include "stm32_rcc.h"
#include "stm32_gpio.h"
#include "stm32_nvic.h"
#include "stm32_pwr.h"
#ifdef STM32_USE_DMA
    #include "stm32_dma.h"
#endif
#ifdef STM32_USE_SDRAM
    #include "stm32_sdram.h"
#endif
#include "stm32_systick.h"
#ifdef STM32_USE_UART
    #include "stm32_uart.h"
#endif
#ifdef STM32_USE_SD
    #include "stm32_sd.h"
#endif
#ifdef STM32_USE_SPI
    #include "stm32_spi.h"
#endif
#ifdef STM32_USE_RTC
    #include "stm32_rtc.h"
#endif
#ifdef STM32_USE_USB
#include "stm32_hcd.h"
#endif
#ifdef STM32_USE_TIM
    #include "stm32_tim.h"
#endif //STM32_USE_TIM
#include "bitbanding.h"

#define STM32_LOCK(HN) \
    { \
        if (HN == STM32_LOCKED) \
            return STM32_RESULT_BUSY; \
        else \
            HN = STM32_LOCKED; \
    }

#define STM32_UNLOCK(HN) \
    HN = STM32_UNLOCKED;

#endif // STM32_INC_H
