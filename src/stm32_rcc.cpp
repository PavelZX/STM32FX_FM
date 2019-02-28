#include "stm32_rcc.h"
#include "stm32_pwr.h"
#include "stm32_systick.h"
#include "stm32_flash.h"
#include "stm32_gpio.h"

uint32_t STM32_RCC::m_system_core_clock;
uint32_t unused_reg;

#define __MCO1_CLK_ENABLE()   enable_clk_GPIOA()
#define MCO1_GPIO_PORT        gpioa
#define MCO1_PIN              GPIO_PIN_8

#if defined(STM32F4)
#define __MCO2_CLK_ENABLE()   enable_clk_GPIOC()
#define MCO2_GPIO_PORT        gpioc
#define MCO2_PIN              GPIO_PIN_9
#endif // STM32F4

#define HSI_VALUE ((uint32_t)16000000UL)
#define HSE_VALUE ((uint32_t) 8000000UL)
#define F_CPU 		168000000UL

#define HSE_TIMEOUT_VALUE           100
#define HSI_TIMEOUT_VALUE           ((uint32_t)2U)  /* 2 ms */
#define LSI_TIMEOUT_VALUE           ((uint32_t)2U)  /* 2 ms */
#define RCC_DBP_TIMEOUT_VALUE       ((uint32_t)2U)
#define PLL_TIMEOUT_VALUE           ((uint32_t)2U)
#define LSE_TIMEOUT_VALUE           5000
#define CLOCKSWITCH_TIMEOUT_VALUE   ((uint32_t)5000U) /* 5 s    */

#define RCC_HSE_BYPASS (RCC_CR_HSEBYP | RCC_CR_HSEON)
#define RCC_LSE_BYPASS (RCC_BDCR_LSEBYP | RCC_BDCR_LSEON)

#define RCC_PLL_NONE                     ((uint8_t)0x00U)
#define RCC_PLL_OFF                      ((uint8_t)0x01U)
#define RCC_PLL_ON                       ((uint8_t)0x02U)

#define RCC_PLLP_DIV2                    ((uint32_t)0x00000002U)
#define RCC_PLLP_DIV4                    ((uint32_t)0x00000004U)
#define RCC_PLLP_DIV6                    ((uint32_t)0x00000006U)
#define RCC_PLLP_DIV8                    ((uint32_t)0x00000008U)

#if defined(STM32F1)
	#define RCC_PLLSOURCE_HSI_DIV2           0x00000000U
	#define RCC_PLLSOURCE_HSE                RCC_CFGR_PLLSRC
#elif defined(STM32F4)
	#define RCC_PLLSOURCE_HSI                RCC_PLLCFGR_PLLSRC_HSI
	#define RCC_PLLSOURCE_HSE                RCC_PLLCFGR_PLLSRC_HSE
#endif


#define RCC_CLOCKTYPE_SYSCLK             ((uint32_t)0x00000001U)
#define RCC_CLOCKTYPE_HCLK               ((uint32_t)0x00000002U)
#define RCC_CLOCKTYPE_PCLK1              ((uint32_t)0x00000004U)
#define RCC_CLOCKTYPE_PCLK2              ((uint32_t)0x00000008U)

#define RCC_SYSCLKSOURCE_HSI             RCC_CFGR_SW_HSI
#define RCC_SYSCLKSOURCE_HSE             RCC_CFGR_SW_HSE
#define RCC_SYSCLKSOURCE_PLLCLK          RCC_CFGR_SW_PLL
#define RCC_SYSCLKSOURCE_PLLRCLK         ((uint32_t)(RCC_CFGR_SW_0 | RCC_CFGR_SW_1))

#define RCC_SYSCLKSOURCE_STATUS_HSI     RCC_CFGR_SWS_HSI   /*!< HSI used as system clock */
#define RCC_SYSCLKSOURCE_STATUS_HSE     RCC_CFGR_SWS_HSE   /*!< HSE used as system clock */
#define RCC_SYSCLKSOURCE_STATUS_PLLCLK  RCC_CFGR_SWS_PLL   /*!< PLL used as system clock */
#define RCC_SYSCLKSOURCE_STATUS_PLLRCLK ((uint32_t)(RCC_CFGR_SWS_0 | RCC_CFGR_SWS_1))   /*!< PLLR used as system clock */

#define RCC_SYSCLK_DIV1                  RCC_CFGR_HPRE_DIV1
#define RCC_SYSCLK_DIV2                  RCC_CFGR_HPRE_DIV2
#define RCC_SYSCLK_DIV4                  RCC_CFGR_HPRE_DIV4
#define RCC_SYSCLK_DIV8                  RCC_CFGR_HPRE_DIV8
#define RCC_SYSCLK_DIV16                 RCC_CFGR_HPRE_DIV16
#define RCC_SYSCLK_DIV64                 RCC_CFGR_HPRE_DIV64
#define RCC_SYSCLK_DIV128                RCC_CFGR_HPRE_DIV128
#define RCC_SYSCLK_DIV256                RCC_CFGR_HPRE_DIV256
#define RCC_SYSCLK_DIV512                RCC_CFGR_HPRE_DIV512

#define RCC_HCLK_DIV1                    RCC_CFGR_PPRE1_DIV1
#define RCC_HCLK_DIV2                    RCC_CFGR_PPRE1_DIV2
#define RCC_HCLK_DIV4                    RCC_CFGR_PPRE1_DIV4
#define RCC_HCLK_DIV8                    RCC_CFGR_PPRE1_DIV8
#define RCC_HCLK_DIV16                   RCC_CFGR_PPRE1_DIV16

#define RCC_RTCCLKSOURCE_LSE             ((uint32_t)0x00000100U)
#define RCC_RTCCLKSOURCE_LSI             ((uint32_t)0x00000200U)
#define RCC_RTCCLKSOURCE_HSE_DIV2        ((uint32_t)0x00020300U)
#define RCC_RTCCLKSOURCE_HSE_DIV3        ((uint32_t)0x00030300U)
#define RCC_RTCCLKSOURCE_HSE_DIV4        ((uint32_t)0x00040300U)
#define RCC_RTCCLKSOURCE_HSE_DIV5        ((uint32_t)0x00050300U)
#define RCC_RTCCLKSOURCE_HSE_DIV6        ((uint32_t)0x00060300U)
#define RCC_RTCCLKSOURCE_HSE_DIV7        ((uint32_t)0x00070300U)
#define RCC_RTCCLKSOURCE_HSE_DIV8        ((uint32_t)0x00080300U)
#define RCC_RTCCLKSOURCE_HSE_DIV9        ((uint32_t)0x00090300U)
#define RCC_RTCCLKSOURCE_HSE_DIV10       ((uint32_t)0x000A0300U)
#define RCC_RTCCLKSOURCE_HSE_DIV11       ((uint32_t)0x000B0300U)
#define RCC_RTCCLKSOURCE_HSE_DIV12       ((uint32_t)0x000C0300U)
#define RCC_RTCCLKSOURCE_HSE_DIV13       ((uint32_t)0x000D0300U)
#define RCC_RTCCLKSOURCE_HSE_DIV14       ((uint32_t)0x000E0300U)
#define RCC_RTCCLKSOURCE_HSE_DIV15       ((uint32_t)0x000F0300U)
#define RCC_RTCCLKSOURCE_HSE_DIV16       ((uint32_t)0x00100300U)
#define RCC_RTCCLKSOURCE_HSE_DIV17       ((uint32_t)0x00110300U)
#define RCC_RTCCLKSOURCE_HSE_DIV18       ((uint32_t)0x00120300U)
#define RCC_RTCCLKSOURCE_HSE_DIV19       ((uint32_t)0x00130300U)
#define RCC_RTCCLKSOURCE_HSE_DIV20       ((uint32_t)0x00140300U)
#define RCC_RTCCLKSOURCE_HSE_DIV21       ((uint32_t)0x00150300U)
#define RCC_RTCCLKSOURCE_HSE_DIV22       ((uint32_t)0x00160300U)
#define RCC_RTCCLKSOURCE_HSE_DIV23       ((uint32_t)0x00170300U)
#define RCC_RTCCLKSOURCE_HSE_DIV24       ((uint32_t)0x00180300U)
#define RCC_RTCCLKSOURCE_HSE_DIV25       ((uint32_t)0x00190300U)
#define RCC_RTCCLKSOURCE_HSE_DIV26       ((uint32_t)0x001A0300U)
#define RCC_RTCCLKSOURCE_HSE_DIV27       ((uint32_t)0x001B0300U)
#define RCC_RTCCLKSOURCE_HSE_DIV28       ((uint32_t)0x001C0300U)
#define RCC_RTCCLKSOURCE_HSE_DIV29       ((uint32_t)0x001D0300U)
#define RCC_RTCCLKSOURCE_HSE_DIV30       ((uint32_t)0x001E0300U)
#define RCC_RTCCLKSOURCE_HSE_DIV31       ((uint32_t)0x001F0300U)

const uint8_t APBAHBPrescTable[16] = {0U, 0U, 0U, 0U, 1U, 2U, 3U, 4U, 1U, 2U, 3U, 4U, 6U, 7U, 8U, 9U};
const uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
const uint8_t APBPrescTable[8]  = {0, 0, 0, 0, 1, 2, 3, 4};
#if defined(RCC_CFGR2_PREDIV1SRC)
  const uint8_t aPLLMULFactorTable[14] = {0, 0, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 13};
  const uint8_t aPredivFactorTable[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
#else
  const uint8_t aPLLMULFactorTable[16] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 16};
#if defined(RCC_CFGR2_PREDIV1)
  const uint8_t aPredivFactorTable[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
#else
  const uint8_t aPredivFactorTable[2] = {1, 2};
#endif /*RCC_CFGR2_PREDIV1*/
#endif

#define PLLI2S_TIMEOUT_VALUE       ((uint32_t)2)  /* Timeout value fixed to 2 ms  */
#define PLLSAI_TIMEOUT_VALUE       ((uint32_t)2)  /* Timeout value fixed to 2 ms  */

void STM32_RCC::init()
{
    enable_clk_PWR();
    #if defined(STM32F4)
    STM32_PWR::set_voltage_scaling_config(PWR_REGULATOR_VOLTAGE_SCALE1);
    #endif
    if (config_osc() != STM32_RESULT_OK)
        Error_Handler();
    #if defined(STM32F1)
    #if defined(FLASH_ACR_LATENCY)
    if (config_clock(FLASH_ACR_LATENCY) != STM32_RESULT_OK)
        Error_Handler();
    #else // FLASH_ACR_LATENCY
    if (config_clock(0) != STM32_RESULT_OK)
        Error_Handler();
    #endif // FLASH_ACR_LATENCY
    #else
    if (config_clock(FLASH_ACR_LATENCY_5WS) != STM32_RESULT_OK)
        Error_Handler();
    #endif

    #ifdef STM32_RTC_ENABLE
    RCC_Periph_Clock_Source sources;
    sources.selector = RCC_PERIPHCLK_RTC;
    sources.RTCClockSelection = STM32_RTC_CLOCK_SOURCE;
    if (periph_CLK_config(&sources) != STM32_RESULT_OK)
        Error_Handler();
    #endif

    #ifdef STM32_USE_CSE
    enable_CSS();
    #endif

    STM32_SYSTICK::update_freq();
}

uint32_t STM32_RCC::deinit()
{
    m_system_core_clock = HSI_VALUE;
    #if defined (STM32F4)
    RCC->CR |= RCC_CR_HSION | RCC_CR_HSITRIM_4;
    #elif defined(STM32F1)
    RCC->CR |= RCC_CR_HSION;
    #endif
    // wait till HSI is ready
    WAIT_TIMEOUT(!HSI_ready(), HSI_TIMEOUT_VALUE);
    #if defined(STM32F1)
    set_calibration_value_HSI(STM32_HSI_CALIBRATION);
    #endif

    // Reset CFGR register
    RCC->CFGR = 0;
    WAIT_TIMEOUT((get_source_SYSCLK() != RESET), HSI_TIMEOUT_VALUE);

    m_system_core_clock = HSI_VALUE;
    STM32_SYSTICK::update_freq();

    RCC->CR &= ~(RCC_CR_PLLON
			#if defined (STM32F4)
			| RCC_CR_PLLI2SON);
			#else
			);
			#endif
    WAIT_TIMEOUT((PLL_ready() != RESET), PLL_TIMEOUT_VALUE);
    RCC->CFGR = 0;

    RCC->CR &= ~(RCC_CR_HSEON | RCC_CR_CSSON);
    WAIT_TIMEOUT((HSE_ready() != RESET), HSE_TIMEOUT_VALUE);

    #if defined (STM32F4)
    RCC->PLLCFGR = RCC_PLLCFGR_PLLM_4 | RCC_PLLCFGR_PLLN_6 | RCC_PLLCFGR_PLLN_7 | RCC_PLLCFGR_PLLQ_2;
    
    RCC->PLLI2SCFGR = RCC_PLLI2SCFGR_PLLI2SN_6 | RCC_PLLI2SCFGR_PLLI2SN_7 | RCC_PLLI2SCFGR_PLLI2SR_1;
    #endif // ST<32F4

    CLEAR_BIT(RCC->CR, RCC_CR_HSEBYP);
    SET_BIT(RCC->CSR, RCC_CSR_RMVF);
    RCC->CIR = 0;
    
    STM32_SYSTICK::update_freq();
    m_system_core_clock = update_system_core_clock();

    return STM32_RESULT_OK;
}

void STM32_RCC::config_HSE(uint32_t state)
{
    switch (state)
    {
    case RCC_CR_HSEON:
        on_HSE();
        break;
    case RCC_HSE_BYPASS:
        on_HSE_BYPASS();
        break;
    default:
        off_HSE();
        break;
    }
}

void STM32_RCC::config_LSE(uint32_t state)
{
    switch (state)
    {
    case RCC_BDCR_LSEON:
        on_LSE();
        break;
    case RCC_LSE_BYPASS:
        on_LSE_BYPASS();
        break;
    default:
        off_LSE();
        break;
    }
}

#if defined(STM32F4)
void STM32_RCC::set_prescaler_RTC(uint32_t value)
{
    if ((value & RCC_BDCR_RTCSEL) == RCC_BDCR_RTCSEL)
        MODIFY_REG(RCC->CFGR, RCC_CFGR_RTCPRE, (value & 0xFFFFCFFU));
    else
        BIT_BAND_PER(RCC->CFGR, RCC_CFGR_RTCPRE) = DISABLE;
}
#endif

void STM32_RCC::set_config_RTC(uint32_t value)
{
		#ifdef STM32F4
    set_prescaler_RTC(value);
		#endif
		RCC->BDCR |= value & RCC_BDCR_RTCSEL;
}

uint32_t STM32_RCC::get_PCLK1_freq()
{
    return (get_HCLK_freq() >> APBAHBPrescTable[(RCC->CFGR & RCC_CFGR_PPRE1) >> POSITION_VAL(RCC_CFGR_PPRE1)]);
}

uint32_t STM32_RCC::config_osc()
{
    #ifdef STM32_USE_HSE
    config_HSE(STM32_HSE_STATE);
    if (STM32_HSE_STATE != 0)
    {
        WAIT_TIMEOUT(get_flag(RCC_FLAG_HSERDY) == RESET, HSE_TIMEOUT_VALUE);
    }
    else
    {
        WAIT_TIMEOUT(get_flag(RCC_FLAG_HSERDY) != RESET, HSE_TIMEOUT_VALUE);
    }
    #elif defined(STM32_USE_HSI)
    if (STM32_HSI_STATE != 0)
    {
        enable_HSI();
        WAIT_TIMEOUT(get_flag(RCC_FLAG_HSIRDY) == RESET, HSI_TIMEOUT_VALUE);
        set_calibration_value_HSI(STM32_HSI_CALIBRATION);
    }
    else
    {
        disable_HSI();
        WAIT_TIMEOUT(get_flag(RCC_FLAG_HSIRDY) != RESET, HSI_TIMEOUT_VALUE);
    }
    #endif

    #ifdef STM32_USE_LSI
    if (STM32_LSI_STATE != 0)
    {
        enable_LSI();
        WAIT_TIMEOUT(get_flag(RCC_FLAG_LSIRDY) == RESET, LSI_TIMEOUT_VALUE);
    }
    else
    {
        disable_LSI();
        WAIT_TIMEOUT(get_flag(RCC_FLAG_LSIRDY) != RESET, LSI_TIMEOUT_VALUE);
    }
    #elif defined(STM32_USE_LSE)
    enable_clk_PWR();
    STM32_PWR::enable_backup_access();
    WAIT_TIMEOUT(STM32_PWR::is_backup_acces_RO(), RCC_DBP_TIMEOUT_VALUE);
    config_LSE(STM32_LSE_STATE);

    if (STM32_LSE_STATE != 0)
    {
        WAIT_TIMEOUT(get_flag(RCC_FLAG_LSERDY) == RESET, LSE_TIMEOUT_VALUE);
    }
    else
    {
        WAIT_TIMEOUT(get_flag(RCC_FLAG_LSERDY) != RESET, LSE_TIMEOUT_VALUE);
    }
    #endif

    #ifdef STM32_USE_PLL
    if (get_source_SYSCLK() != RCC_CFGR_SWS_PLL)
    {
        disable_PLL();
        WAIT_TIMEOUT(get_flag(RCC_FLAG_PLLRDY) != RESET, PLL_TIMEOUT_VALUE);
        if (STM32_PLL_STATE == RCC_PLL_ON)
        {
						#if defined(STM32F4)
            set_config_PLL_source((STM32_PLL_SOURCE | STM32_PLLM |
                                  (STM32_PLLN << RCC_PLLCFGR_PLLN_Pos) |
                                  (((STM32_PLLP >> 1U) - 1U) << RCC_PLLCFGR_PLLP_Pos) |
                                  (STM32_PLLQ << RCC_PLLCFGR_PLLQ_Pos)));
						#elif defined(STM32F1)
                        set_config_PLL_source(STM32_PLL_SOURCE | STM32_PLLM);
						#endif
            enable_PLL();
            WAIT_TIMEOUT(get_flag(RCC_FLAG_PLLRDY) == RESET, PLL_TIMEOUT_VALUE);
        }
    }
    else
        return STM32_RESULT_FAIL;
    #endif

    return STM32_RESULT_OK;
}

uint32_t STM32_RCC::config_clock(uint32_t flash_latency)
{
    #if defined(FLASH_ACR_LATENCY)
    if (flash_latency > STM32_FLASH::get_latency())
    {
        STM32_FLASH::set_latency(flash_latency);
        if (STM32_FLASH::get_latency() != flash_latency)
            return STM32_RESULT_FAIL;
    }
    else
    (void)(flash_latency);
    #endif

    if ((STM32_CLOCK_TYPE & RCC_CLOCKTYPE_HCLK) == RCC_CLOCKTYPE_HCLK)
    {
        MODIFY_REG(RCC->CFGR, RCC_CFGR_HPRE, STM32_CLOCK_AHB_DIV);
    }

    if ((STM32_CLOCK_TYPE & RCC_CLOCKTYPE_SYSCLK) == RCC_CLOCKTYPE_SYSCLK)
    {
        if (STM32_CLOCK_SYSCLK_SOURCE == RCC_CFGR_SW_HSE)
        {
            if (get_flag(RCC_FLAG_HSERDY) == RESET)
                return STM32_RESULT_FAIL;
        }
        else if ((STM32_CLOCK_SYSCLK_SOURCE == RCC_CFGR_SW_PLL) ||
                 (STM32_CLOCK_SYSCLK_SOURCE == (RCC_CFGR_SW_HSE | RCC_CFGR_SW_PLL)))
        {
            if (get_flag(RCC_FLAG_PLLRDY) == RESET)
                return STM32_RESULT_FAIL;
        }
        else if (get_flag(RCC_FLAG_HSIRDY) == RESET)
            return STM32_RESULT_FAIL;

        set_config_SYSCLK(STM32_CLOCK_SYSCLK_SOURCE);
        switch (STM32_CLOCK_SYSCLK_SOURCE)
        {
        case RCC_SYSCLKSOURCE_HSE:
            WAIT_TIMEOUT(get_source_SYSCLK() != RCC_SYSCLKSOURCE_STATUS_HSE, CLOCKSWITCH_TIMEOUT_VALUE);
            break;
        case RCC_SYSCLKSOURCE_PLLCLK:
            WAIT_TIMEOUT(get_source_SYSCLK() != RCC_SYSCLKSOURCE_STATUS_PLLCLK, CLOCKSWITCH_TIMEOUT_VALUE);
            break;
        default:
            WAIT_TIMEOUT(get_source_SYSCLK() != RCC_SYSCLKSOURCE_STATUS_HSI, CLOCKSWITCH_TIMEOUT_VALUE);
            break;
        }
    }

    #if defined(FLASH_ACR_LATENCY)
    if (flash_latency < STM32_FLASH::get_latency())
    {
        STM32_FLASH::set_latency(flash_latency);
        if (STM32_FLASH::get_latency() != flash_latency)
            return STM32_RESULT_FAIL;
    }
    #endif

    if ((STM32_CLOCK_TYPE & RCC_CLOCKTYPE_PCLK1) == RCC_CLOCKTYPE_PCLK1)
    {
        MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE1, STM32_CLOCK_APB1_DIV);
    }

    if ((STM32_CLOCK_TYPE & RCC_CLOCKTYPE_PCLK2) == RCC_CLOCKTYPE_PCLK2)
    {
        MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE2, STM32_CLOCK_APB2_DIV);
    }

    update_clock();
    STM32_SYSTICK::init();

    return STM32_RESULT_OK;
}

bool STM32_RCC::get_flag(uint32_t value)
{
    __IO uint32_t reg;
    switch (value >> 5U)
    {
    case 0x01U:
        reg = RCC->CR;
        break;
    case 0x02U:
        reg = RCC->BDCR;
        break;
    case 0x03U:
        reg = RCC->CSR;
        break;
    default:
        reg = RCC->CIR;
        break;
    }
    return (reg & (0x01U << (value & RCC_FLAG_MASK)));
}

uint32_t STM32_RCC::get_PCLK2_freq()
{
    return (get_HCLK_freq() >> APBAHBPrescTable[(RCC->CFGR & RCC_CFGR_PPRE2) >> POSITION_VAL(RCC_CFGR_PPRE2)]);
}

void STM32_RCC::config_MCO(uint32_t RCC_MCOx, uint32_t RCC_MCOSource, uint32_t RCC_MCODiv)
{
    if (RCC_MCOx == RCC_MCO1)
    {
        __MCO1_CLK_ENABLE();
				#if defined(STM3F1)
				UNUSED(RCC_MCODiv);
				MCO1_GPIO_PORT.set_config(MCO1_PIN, GPIO_MODE_AF_PP, 0, GPIO_SPEED_FREQ_HIGH, GPIO_NOPULL);
        MODIFY_REG(RCC->CFGR, RCC_CFGR_MCO, RCC_MCOSource);
				#elif defined(STM32F4)
        MCO1_GPIO_PORT.set_config(MCO1_PIN, GPIO_MODE_AF_PP, GPIO_AF0_MCO, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_NOPULL);
        MODIFY_REG(RCC->CFGR, (RCC_CFGR_MCO1 | RCC_CFGR_MCO1PRE), (RCC_MCOSource | RCC_MCODiv));
				#endif
    }
    #ifdef RCC_CFGR_MCO2
    else
    {
        __MCO2_CLK_ENABLE();
        MCO2_GPIO_PORT.set_config(MCO2_PIN, GPIO_MODE_AF_PP, GPIO_AF0_MCO, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_NOPULL);
        MODIFY_REG(RCC->CFGR, (RCC_CFGR_MCO2 | RCC_CFGR_MCO2PRE), (RCC_MCOSource | RCC_MCODiv));
    }
    #endif
}

void STM32_RCC::NMI_IRQ_Handler()
{
    if (get_IT(RCC_IT_CSS))
    {
        ///TODO: CSSCallback
        clear_IT(RCC_IT_CSS);
    }
}

#if defined(STM32F4)
uint32_t STM32_RCC::periph_CLK_config(RCC_Periph_Clock_Source *sources)
{
    uint32_t tmpreg1 = 0U;

    /*----------------------- SAI/I2S Configuration (PLLI2S) -------------------*/
    if (((sources->selector & RCC_PERIPHCLK_I2S) == RCC_PERIPHCLK_I2S) ||
#ifdef RCC_PERIPHCLK_SAI_PLLI2S
        ((sources->selector & RCC_PERIPHCLK_SAI_PLLI2S) == RCC_PERIPHCLK_SAI_PLLI2S))
#else
        ((sources->selector & RCC_PERIPHCLK_PLLI2S) == RCC_PERIPHCLK_PLLI2S))
#endif
    {
        disable_PLLI2S();
        /* Wait till PLLI2S is disabled */
        WAIT_TIMEOUT(get_flag(RCC_FLAG_PLLI2SRDY) != RESET, PLLI2S_TIMEOUT_VALUE);
#ifdef RCC_PERIPHCLK_SAI_PLLI2S
        /*---------------------------- SAI configuration -------------------------*/
        /* In Case of SAI Clock Configuration through PLLI2S, PLLI2SQ and PLLI2S_DIVQ must
        be added only for SAI configuration */
        if ((sources->selector & RCC_PERIPHCLK_SAI_PLLI2S) == RCC_PERIPHCLK_SAI_PLLI2S)
        {
            /* Read PLLI2SR value from PLLI2SCFGR register (this value is not need for SAI configuration) */
            tmpreg1 = ((RCC->PLLI2SCFGR & RCC_PLLI2SCFGR_PLLI2SR) >> POSITION_VAL(RCC_PLLI2SCFGR_PLLI2SR));
            /* Configure the PLLI2S division factors */
            /* PLLI2S_VCO Input  = PLL_SOURCE/PLLM */
            /* PLLI2S_VCO Output = PLLI2S_VCO Input * PLLI2SN */
            /* SAI_CLK(first level) = PLLI2S_VCO Output/PLLI2SQ */
            PLLI2S_SAI_config(sources->PLLI2SN, sources->PLLI2SQ , tmpreg1);
            /* SAI_CLK_x = SAI_CLK(first level)/PLLI2SDIVQ */
            PLLI2S_SAI_configQ(sources->PLLI2SDivQ);
        }
#else
        PLLSAI_config(sources->PLLI2SN, sources->PLLI2SR);
#endif

        /* Enable the PLLI2S */
        enable_PLLI2S();
        /* Wait till PLLI2S is ready */
        WAIT_TIMEOUT(get_flag(RCC_FLAG_PLLI2SRDY) == RESET, PLLI2S_TIMEOUT_VALUE);
    }

    /*----------------------- SAI/LTDC Configuration (PLLSAI) ------------------*/
#ifdef RCC_PERIPHCLK_SAI_PLLSAI
    if (((sources->selector & RCC_PERIPHCLK_SAI_PLLSAI) == RCC_PERIPHCLK_SAI_PLLSAI) ||
        ((sources->selector & RCC_PERIPHCLK_LTDC) == RCC_PERIPHCLK_LTDC))
    {
        /* Disable PLLSAI Clock */
        disable_PLLSAI();
        /* Wait till PLLSAI is disabled */
        WAIT_TIMEOUT(get_PLLSAI_flag() != RESET, PLLSAI_TIMEOUT_VALUE);

        /*---------------------------- SAI configuration -------------------------*/
        /* In Case of SAI Clock Configuration through PLLSAI, PLLSAIQ and PLLSAI_DIVQ must
        be added only for SAI configuration */
        if ((sources->selector & RCC_PERIPHCLK_SAI_PLLSAI) == RCC_PERIPHCLK_SAI_PLLSAI)
        {
            /* Read PLLSAIR value from PLLSAICFGR register (this value is not need for SAI configuration) */
            tmpreg1 = ((RCC->PLLSAICFGR & RCC_PLLSAICFGR_PLLSAIR) >> POSITION_VAL(RCC_PLLSAICFGR_PLLSAIR));
            /* PLLSAI_VCO Input  = PLL_SOURCE/PLLM */
            /* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN */
            /* SAI_CLK(first level) = PLLSAI_VCO Output/PLLSAIQ */
            PLLSAI_config(sources->PLLSAIN , sources->PLLSAIQ, tmpreg1);
            /* SAI_CLK_x = SAI_CLK(first level)/PLLSAIDIVQ */
            PLLSAI_configQ(sources->PLLSAIDivQ);
        }

        /*---------------------------- LTDC configuration ------------------------*/
        if (((sources->selector) & RCC_PERIPHCLK_LTDC) == (RCC_PERIPHCLK_LTDC))
        {
            /* Read PLLSAIR value from PLLSAICFGR register (this value is not need for SAI configuration) */
            tmpreg1 = ((RCC->PLLSAICFGR & RCC_PLLSAICFGR_PLLSAIQ) >> POSITION_VAL(RCC_PLLSAICFGR_PLLSAIQ));
            /* PLLSAI_VCO Input  = PLL_SOURCE/PLLM */
            /* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN */
            /* LTDC_CLK(first level) = PLLSAI_VCO Output/PLLSAIR */
            PLLSAI_config(sources->PLLSAIN , tmpreg1, sources->PLLSAIR);
            /* LTDC_CLK = LTDC_CLK(first level)/PLLSAIDIVR */
            PLLSAI_configR(sources->PLLSAIDivR);
        }
        /* Enable PLLSAI Clock */
        enable_PLLSAI();
        /* Wait till PLLSAI is ready */
        WAIT_TIMEOUT(get_PLLSAI_flag() == RESET, PLLSAI_TIMEOUT_VALUE);
    }
#endif

    /*---------------------------- RTC configuration ---------------------------*/
    if ((sources->selector & RCC_PERIPHCLK_RTC) == RCC_PERIPHCLK_RTC)
    {
        /* Enable Power Clock*/
        enable_clk_PWR();

        /* Enable write access to Backup domain */
        STM32_PWR::enable_backup_access();
        WAIT_TIMEOUT(STM32_PWR::is_backup_acces_RO(), RCC_DBP_TIMEOUT_VALUE);

        /* Reset the Backup domain only if the RTC Clock source selection is modified from reset value */
        tmpreg1 = (RCC->BDCR & RCC_BDCR_RTCSEL);
        if ((tmpreg1 != 0x00000000U) &&
            (tmpreg1 != (sources->RTCClockSelection & RCC_BDCR_RTCSEL)))
        {
            /* Store the content of BDCR register before the reset of Backup Domain */
            tmpreg1 = (RCC->BDCR & ~(RCC_BDCR_RTCSEL));
            /* RTC Clock selection can be changed only if the Backup Domain is reset */
            force_reset_backup();
            release_reset_backup();
            /* Restore the Content of BDCR register */
            RCC->BDCR = tmpreg1;

            /* Wait for LSE reactivation if LSE was enable prior to Backup Domain reset */
            if ((RCC->BDCR & RCC_BDCR_LSEON) == RCC_BDCR_LSEON)
            {
                /* Wait till LSE is ready */
                WAIT_TIMEOUT(get_flag(RCC_FLAG_LSERDY) == RESET, LSE_TIMEOUT_VALUE);
            }
        }
        set_config_RTC(sources->RTCClockSelection);
    }

#if defined(STM32F401xC) || defined(STM32F401xE) || defined(STM32F411xE)
 || defined(STM32F427xx) || defined(STM32F437xx) || defined(STM32F429xx) || defined(STM32F439xx) || defined(STM32F469xx) || defined(STM32F479xx)
    /*---------------------------- TIM configuration ---------------------------*/
    if (((sources->selector) & RCC_PERIPHCLK_TIM) == RCC_PERIPHCLK_TIM)
        set_TIM_prescaler(sources->TIMPresSelection);
#endif

    return STM32_RESULT_OK;
}
#endif // STM32F4

uint32_t STM32_RCC::update_system_core_clock()
{
    uint32_t tmp = 0,
            pllvco = 0,
            pllp = 2,
            pllsource = 0,
						prediv = 0,
            pllm = 2;
    uint32_t sysclockfreq = 0U;

    /* Get SYSCLK source -------------------------------------------------------*/
    tmp = RCC->CFGR & RCC_CFGR_SWS;

    switch (tmp)
    {
    case RCC_CFGR_SWS_HSI:  /* HSI used as system clock source */
        sysclockfreq = HSI_VALUE;
        break;
    case RCC_CFGR_SWS_HSE:  /* HSE used as system clock source */
        sysclockfreq = HSE_VALUE;
        break;
    case RCC_CFGR_SWS_PLL:  /* PLL used as system clock source */
        #if defined(STM32F1)
        pllsource = get_source_PLL_OSC() >> 22;
        pllm = aPLLMULFactorTable[(RCC->CFGR & RCC_CFGR_PLLMULL >> RCC_CFGR_PLLMULL_Pos)];
        if (get_source_PLL_OSC() != RCC_PLLSOURCE_HSI_DIV2)
        {
            #if defined(RCC_CFGR2_PREDIV1)
            prediv = aPredivFactorTable[(uint32_t)(RCC->CFGR2 & RCC_CFGR2_PREDIV1) >> RCC_CFGR2_PREDIV1_Pos];
            #else
            prediv = aPredivFactorTable[(uint32_t)(RCC->CFGR & RCC_CFGR_PLLXTPRE) >> RCC_CFGR_PLLXTPRE_Pos];
            #endif /*RCC_CFGR2_PREDIV1*/

            #if defined(RCC_CFGR2_PREDIV1SRC)
            if(HAL_IS_BIT_SET(RCC->CFGR2, RCC_CFGR2_PREDIV1SRC))
            {
                    /* PLL2 selected as Prediv1 source */
                    /* PLLCLK = PLL2CLK / PREDIV1 * PLLMUL with PLL2CLK = HSE/PREDIV2 * PLL2MUL */
                    prediv2 = ((RCC->CFGR2 & RCC_CFGR2_PREDIV2) >> RCC_CFGR2_PREDIV2_Pos) + 1;
                    pll2mul = ((RCC->CFGR2 & RCC_CFGR2_PLL2MUL) >> RCC_CFGR2_PLL2MUL_Pos) + 2;
                    pllclk = (uint32_t)(((uint64_t)HSE_VALUE * (uint64_t)pll2mul * (uint64_t)pllmul) / ((uint64_t)prediv2 * (uint64_t)prediv));
            }
            else
            {
                    /* HSE used as PLL clock source : PLLCLK = HSE/PREDIV1 * PLLMUL */
                    pllclk = (uint32_t)((HSE_VALUE * pllmul) / prediv);
            }

            /* If PLLMUL was set to 13 means that it was to cover the case PLLMUL 6.5 (avoid using float) */
            /* In this case need to divide pllclk by 2 */
            if (pllmul == aPLLMULFactorTable[(uint32_t)(RCC_CFGR_PLLMULL6_5) >> RCC_CFGR_PLLMULL_Pos])
                    pllclk = pllclk / 2;
            #else
            /* HSE used as PLL clock source : PLLCLK = HSE/PREDIV1 * PLLMUL */
            sysclockfreq = (uint32_t)((HSE_VALUE  * pllm) / prediv);
            #endif /*RCC_CFGR2_PREDIV1SRC*/
        }
        else
        {
                /* HSI used as PLL clock source : PLLCLK = HSI/2 * PLLMUL */
                sysclockfreq = (uint32_t)((HSI_VALUE >> 1) * pllm);
        }
        #elif defined(STM32F4)
        pllsource = get_source_PLL_OSC() >> 22;
        pllm = RCC->PLLCFGR & RCC_PLLCFGR_PLLM;
        if (pllsource != 0)
        {
            /* HSE used as PLL clock source */
            pllvco = (HSE_VALUE / pllm) * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> 6);
        }
        else
        {
            /* HSI used as PLL clock source */
            pllvco = (HSI_VALUE / pllm) * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> 6);
        }
        pllp = (((RCC->PLLCFGR & RCC_PLLCFGR_PLLP) >>16) + 1 ) *2;
        sysclockfreq = pllvco/pllp;
				#endif
        break;
    default:
        sysclockfreq = HSI_VALUE;
        break;
    }
    /* Compute HCLK frequency --------------------------------------------------*/
    /* Get HCLK prescaler */
    tmp = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> 4)];
    /* HCLK frequency */
    sysclockfreq >>= tmp;

    return sysclockfreq;
}

__attribute__((weak)) void ISR::RCC_IRQ()
{
    if (STM32_RCC::get_IT(RCC_IT_CSS))
    {
        STM32_RCC::clear_IT(RCC_IT_CSS);
    }
}

void STM32_RCC::update_clock()
{
    m_system_core_clock = update_system_core_clock() >> APBAHBPrescTable[(RCC->CFGR & RCC_CFGR_HPRE)>> RCC_CFGR_HPRE_Pos];
}
