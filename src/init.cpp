#include "stm32_inc.h"
#include "init.h"
#include "my_func.h"
#ifdef STM32_FATFS_USE
#include "sddriver.h"
#endif

int main();

/* #define VECT_TAB_SRAM */
#define VECT_TAB_OFFSET  0x00 /*!< Vector Table base offset field. 
                                   This value must be a multiple of 0x200. */

// base initialization
void base_init()
{
    /* FPU settings ------------------------------------------------------------*/
    #if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));  /* set CP10 and CP11 Full Access */
    #endif

    //extern uint32_t _RAM_Start;
    //memset((uint8_t*)&_RAM_Start, 0, 1024*50);

    STM32_RCC::deinit();
    STM32_SYSTICK::init();

    #if defined (DATA_IN_ExtSDRAM)
    if (STM32_SDRAM::init() != STM32_RESULT_OK)
        Error_Handler();
    #endif

    /* Configure the Vector Table location add offset address ------------------*/
    #ifdef VECT_TAB_SRAM
    SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM */
    #else
    SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH */
    #endif

    #ifdef INSTRUCTION_CACHE_ENABLE
    STM32_FLASH::enable_instruction_cache();
    #endif
    #ifdef DATA_CACHE_ENABLE
    STM32_FLASH::enable_data_cache();
    #endif
    #ifdef PREFETCH_ENABLE
    STM32_FLASH::enable_prefetch_buffer();
    #endif

    STM32_NVIC::init();
}

void SystemInit()
{
    base_init();

    // system initialization
    //__enable_fault_irq();
    //__enable_irq();
    STM32_RCC::init();
    STM32_SYSTICK::init();

    /* Other IO and peripheral initializations */
    STM32_UART::init_all();
    STM32_SPI::init_all();
#ifdef STM32_RTC_ENABLE
    STM32_RTC::init();
#endif
    #ifdef STM32_FATFS_USE
    sd_driver.init_gpio();
    #endif

    /* Initialize interrupt vectors for a peripheral */
    STM32_NVIC::init_vectors();
}

#define INIT_SP() \
    { \
        __ASM volatile("ldr sp, =_estack\n\r" : : ); \
    } \

void ISR::Reset()
{
    INIT_SP();
    SystemInit();
    main();
}
