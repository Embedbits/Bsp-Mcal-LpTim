/**
 * \author Mr.Nobody
 * \defgroup LpTim LpTim
 * \brief LpTim module
 */

/**
 * \author Mr.Nobody
 * \file LpTim_Types.h
 * \ingroup LpTim
 * \brief LpTim module global types definition
 *
 * This file contains the types definitions used across the module and are
 * available for other modules through Port file.
 *
 */

#ifndef LPTIM_LPTIM_TYPES_H
#define LPTIM_LPTIM_TYPES_H
/* ============================== INCLUDES ================================== */
#include "stdint.h"                         /* Module types definition        */
#include "Gpio_Port.h"                      /* GPIO Port module definitions   */
#include "Nvic_Port.h"                      /* NVIC Port module definitions   */
/* ========================== SYMBOLIC CONSTANTS ============================ */

/** Null pointer definition */
#define LPTIM_NULL_PTR                       ( ( void* ) 0u )

/** Channel identification bit offset in encoded channel pin value */
#define LPTIM_BIT_MASK_CHANNEL_BIT_OFFSET    ( 20u )

/** Peripheral identification bit offset in encoded pin value */
#define LPTIM_BIT_MASK_PERIPH_BIT_OFFSET     ( 15u )

/** Port identification bit offset in encoded pin value */
#define LPTIM_BIT_MASK_PORT_BIT_OFFSET       ( 10u )

/** Pin identification bit offset in encoded pin value */
#define LPTIM_BIT_MASK_PIN_BIT_OFFSET        ( 5u )

/** Alternative function identification bit offset in encoded pin value */
#define LPTIM_BIT_MASK_AF_BIT_OFFSET         ( 0u )

/* ========================== EXPORTED MACROS =============================== */

/** Encode a channel (output or input) pin configuration into a single bit-mask */
#define LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( CHANNEL_ID, PERIPH_ID, PORT_ID, PIN_ID, AF_ID ) \
            ( ( (uint32_t)(CHANNEL_ID) << LPTIM_BIT_MASK_CHANNEL_BIT_OFFSET ) | \
              ( (uint32_t)(PERIPH_ID)  << LPTIM_BIT_MASK_PERIPH_BIT_OFFSET  ) | \
              ( (uint32_t)(PORT_ID)    << LPTIM_BIT_MASK_PORT_BIT_OFFSET    ) | \
              ( (uint32_t)(PIN_ID)     << LPTIM_BIT_MASK_PIN_BIT_OFFSET     ) | \
              ( (uint32_t)(AF_ID)      << LPTIM_BIT_MASK_AF_BIT_OFFSET      )   )

/** Encode an instance-level (trigger / external clock) pin configuration into a single bit-mask */
#define LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( PERIPH_ID, PORT_ID, PIN_ID, AF_ID ) \
            ( ( (uint32_t)(PERIPH_ID) << LPTIM_BIT_MASK_PERIPH_BIT_OFFSET ) | \
              ( (uint32_t)(PORT_ID)   << LPTIM_BIT_MASK_PORT_BIT_OFFSET   ) | \
              ( (uint32_t)(PIN_ID)    << LPTIM_BIT_MASK_PIN_BIT_OFFSET    ) | \
              ( (uint32_t)(AF_ID)     << LPTIM_BIT_MASK_AF_BIT_OFFSET     )   )

/** Pin is not configured by the module (value of the *_PIN_UNUSED items of the channel pin tables) */
#define LPTIM_CHANNEL_PIN_UNUSED \
    LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_CNT, \
                                       LPTIM_PERIPH_CNT, \
                                       GPIO_PORT_CNT, \
                                       GPIO_PIN_ID_CNT, \
                                       GPIO_ALT_FUNC_CNT )

/** Pin is not configured by the module (value of the *_PIN_UNUSED items of the instance pin tables) */
#define LPTIM_PERIPH_PIN_UNUSED \
    LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_CNT, \
                                      GPIO_PORT_CNT, \
                                      GPIO_PIN_ID_CNT, \
                                      GPIO_ALT_FUNC_CNT )

/** Extract the peripheral identification from an encoded pin value */
#define LPTIM_BIT_MASK_DECODE_PERIPH( CODED_VAL ) \
            ( ( (uint32_t)(CODED_VAL) >> LPTIM_BIT_MASK_PERIPH_BIT_OFFSET ) & 0x1Fu )

/** Extract the channel identification from an encoded channel pin value */
#define LPTIM_BIT_MASK_DECODE_CHANNEL( CODED_VAL ) \
            ( ( (uint32_t)(CODED_VAL) >> LPTIM_BIT_MASK_CHANNEL_BIT_OFFSET ) & 0x1Fu )

/** Extract the port identification from an encoded pin value */
#define LPTIM_BIT_MASK_DECODE_PORT( CODED_VAL ) \
            ( ( (uint32_t)(CODED_VAL) >> LPTIM_BIT_MASK_PORT_BIT_OFFSET ) & 0x1Fu )

/** Extract the pin identification from an encoded pin value */
#define LPTIM_BIT_MASK_DECODE_PIN( CODED_VAL ) \
            ( ( (uint32_t)(CODED_VAL) >> LPTIM_BIT_MASK_PIN_BIT_OFFSET ) & 0x1Fu )

/** Extract the alternate function identification from an encoded pin value */
#define LPTIM_BIT_MASK_DECODE_AF( CODED_VAL ) \
            ( ( (uint32_t)(CODED_VAL) >> LPTIM_BIT_MASK_AF_BIT_OFFSET ) & 0x1Fu )

/* ============================== TYPEDEFS ================================== */

/** \brief Type signaling major version of SW module */
typedef uint8_t lpTim_MajorVersion_t;

/** \brief Type signaling minor version of SW module */
typedef uint8_t lpTim_MinorVersion_t;

/** \brief Type signaling patch version of SW module */
typedef uint8_t lpTim_PatchVersion_t;

/** \brief Type signaling actual version of SW module */
typedef struct
{
    lpTim_MajorVersion_t Major; /**< Major version */
    lpTim_MinorVersion_t Minor; /**< Minor version */
    lpTim_PatchVersion_t Patch; /**< Patch version */
}   lpTim_ModuleVersion_t;


/** Function status enumeration */
typedef enum
{
    LPTIM_FUNCTION_INACTIVE = 0u, /**< Function status is inactive */
    LPTIM_FUNCTION_ACTIVE         /**< Function status is active   */
}   lpTim_FunctionState_t;


/** Enumeration used to signal request processing state */
typedef enum
{
    LPTIM_REQUEST_ERROR = 0u, /**< Processing request failed  */
    LPTIM_REQUEST_OK          /**< Processing request succeed */
}   lpTim_RequestState_t;


/** Flag states enumeration */
typedef enum
{
    LPTIM_FLAG_INACTIVE = 0u, /**< Inactive flag state */
    LPTIM_FLAG_ACTIVE         /**< Active flag state   */
}   lpTim_FlagState_t;


/** \brief List of the LPTIM instances available on the device */
typedef enum
{
#if defined(LPTIM1)
    LPTIM_PERIPH_1   = 0u, /**< LPTIM1 instance                    */
#endif
#if defined(LPTIM2)
    LPTIM_PERIPH_2       , /**< LPTIM2 instance                    */
#endif
#if defined(LPTIM3)
    LPTIM_PERIPH_3       , /**< LPTIM3 instance                    */
#endif
#if defined(LPTIM4)
    LPTIM_PERIPH_4       , /**< LPTIM4 instance                    */
#endif
#if defined(LPTIM5)
    LPTIM_PERIPH_5       , /**< LPTIM5 instance                    */
#endif
#if defined(LPTIM6)
    LPTIM_PERIPH_6       , /**< LPTIM6 instance                    */
#endif
    LPTIM_PERIPH_CNT       /**< Count of available LPTIM instances */
}   lpTim_PeriphId_t;


/**
 * \brief Type used to signal a period expressed in microseconds
 *
 * Use this datatype for the period of an instance, where the following
 * characteristics are sufficient:
 *  - Range of values : 0 us to 4.294 s in microseconds
 *  - Step size        : 1 us (the achieved value is read back, it may differ
 *    by the resolution of the selected clock source and prescaler)
 */
typedef uint32_t lpTim_Period_us_t;


/** \brief 16-bit counter / auto-reload / compare value type of the LPTIM instances */
typedef uint16_t lpTim_Counter_t;


/** \brief Frequency value type, expressed in Hz */
typedef uint32_t lpTim_Frequency_Hz_t;


/**
 * \brief Type used to signal a PWM duty cycle
 *
 * Use this datatype for the duty cycle, where the following characteristics
 * are sufficient:
 *  - Range of values: 0 to 1000 in [0.1 %]
 */
typedef uint16_t lpTim_Duty_permille_t;


/** \brief Repetition counter value type (burst mode, 0 .. 255 additional periods) */
typedef uint8_t lpTim_Repetition_t;


/** \brief LPTIM interrupt priority type (passed through to the Nvic module) */
typedef nvic_IrqPrio_t lpTim_IrqPrio_t;


/** \brief Signature of a callback invoked from the interrupt of an LPTIM instance */
typedef void ( *lpTim_Callback_t )( void );


/** \brief List of the two channels an LPTIM instance can have */
typedef enum
{
    LPTIM_CHANNEL_1   = 0u, /**< Channel 1 (the only channel of an instance with a single output) */
    LPTIM_CHANNEL_2       , /**< Channel 2 (not available on an instance with a single output)    */
    LPTIM_CHANNEL_CNT       /**< Count of available channel identifications                       */
}   lpTim_ChannelId_t;


/** \brief Operating mode of an LPTIM instance */
typedef enum
{
    LPTIM_MODE_TIMEBASE      = 0u, /**< Continuous counting, an update event at every period            */
    LPTIM_MODE_ONE_SHOT          , /**< Counts once for the required time, then stops                   */
    LPTIM_MODE_PULSE_COUNTER     , /**< Counts the pulses of an external input, no oscillator is needed */
    LPTIM_MODE_ENCODER           , /**< Counts the position of a quadrature encoder                     */
    LPTIM_MODE_CNT                 /**< Count of available operating modes                              */
}   lpTim_Mode_t;


/** \brief Start mode of an LPTIM instance */
typedef enum
{
    LPTIM_START_CONTINUOUS = 0u, /**< Counts continuously until stopped */
    LPTIM_START_SINGLE         , /**< Counts once, then stops by itself */
    LPTIM_START_CNT              /**< Count of available start modes    */
}   lpTim_StartMode_t;


/** \brief Operating mode of a single channel of an LPTIM instance */
typedef enum
{
    LPTIM_CHANNEL_PWM            = 0u, /**< PWM output (set/reset on compare match)            */
    LPTIM_CHANNEL_OUTPUT_COMPARE     , /**< Output compare (toggle on compare match)           */
    LPTIM_CHANNEL_INPUT_CAPTURE      , /**< Input capture (counter value latched on the input) */
    LPTIM_CHANNEL_MODE_CNT             /**< Count of available channel modes                   */
}   lpTim_ChannelMode_t;


/** \brief Output / input signal polarity */
typedef enum
{
    LPTIM_POLARITY_HIGH = 0u, /**< Required / active state is high */
    LPTIM_POLARITY_LOW      , /**< Required / active state is low  */
    LPTIM_POLARITY_CNT        /**< Count of available polarities   */
}   lpTim_Polarity_t;


/** \brief Direction of the encoder / pulse counter */
typedef enum
{
    LPTIM_DIRECTION_UP   = 0u, /**< Counting up                   */
    LPTIM_DIRECTION_DOWN     , /**< Counting down                 */
    LPTIM_DIRECTION_CNT        /**< Count of available directions */
}   lpTim_Direction_t;


/** \brief Digital filter length applied to an input / the trigger (CFGR TRIGFLT / INFLT fields) */
typedef enum
{
    LPTIM_FILTER_NONE     = 0u, /**< No filter, every edge is taken into account */
    LPTIM_FILTER_2_CLOCKS     , /**< The edge is confirmed after 2 clock cycles  */
    LPTIM_FILTER_4_CLOCKS     , /**< The edge is confirmed after 4 clock cycles  */
    LPTIM_FILTER_8_CLOCKS     , /**< The edge is confirmed after 8 clock cycles  */
    LPTIM_FILTER_CNT            /**< Count of available filter lengths           */
}   lpTim_Filter_t;


/**
 * \brief Events of an LPTIM instance that can be enabled, polled or dispatched to a callback
 *
 * \note The compare and the capture event of a channel share the same hardware flag (the
 *       channel is either in the output or in the input mode at a given time), the module
 *       exposes both identifications for readability of the caller's code.
 */
typedef enum
{
    LPTIM_EVENT_UPDATE         = 0u, /**< Auto-reload match (update event)                 */
    LPTIM_EVENT_COMPARE_1          , /**< Compare match of the channel 1                   */
    LPTIM_EVENT_COMPARE_2          , /**< Compare match of the channel 2                   */
    LPTIM_EVENT_CAPTURE_1          , /**< Input capture of the channel 1                   */
    LPTIM_EVENT_CAPTURE_2          , /**< Input capture of the channel 2                   */
    LPTIM_EVENT_TRIGGER            , /**< External trigger edge event                      */
    LPTIM_EVENT_DIRECTION_UP       , /**< Encoder / counter direction change, down to up   */
    LPTIM_EVENT_DIRECTION_DOWN     , /**< Encoder / counter direction change, up to down   */
    LPTIM_EVENT_OVERCAPTURE_1      , /**< Over-capture of the channel 1 (not read in time) */
    LPTIM_EVENT_OVERCAPTURE_2      , /**< Over-capture of the channel 2 (not read in time) */
    LPTIM_EVENT_CNT                  /**< Count of available events                        */
}   lpTim_EventId_t;


/** \brief Configuration of a single channel of an LPTIM instance */
typedef struct
{
    lpTim_ChannelMode_t Mode;        /**< Operating mode of the channel                    */
    lpTim_Polarity_t    Polarity;    /**< Polarity of the output / the captured edge       */
    lpTim_Filter_t      InputFilter; /**< Digital filter of the input (input capture mode) */
}   lpTim_ChannelConfig_t;


/** \brief External trigger configuration of an LPTIM instance */
typedef struct
{
    lpTim_FunctionState_t TriggerActive; /**< Whether the external trigger is used        */
    lpTim_Polarity_t      Polarity;      /**< Active edge of the trigger input            */
    lpTim_Filter_t        Filter;        /**< Digital filter applied to the trigger input */
}   lpTim_TriggerConfig_t;


/** \brief Encoder mode configuration of an LPTIM instance */
typedef struct
{
    lpTim_Polarity_t Polarity;    /**< Active edge(s) counted on the two inputs */
    lpTim_Filter_t   InputFilter; /**< Digital filter applied to both inputs    */
}   lpTim_EncoderConfig_t;


/** \brief List of the kernel clock sources an LPTIM instance can be clocked from */
typedef enum
{
#if defined(LPTIM1)
    LPTIM_CLK_SRC_LPTIM1_PCLK3 = 0u, /**< LPTIM1 clocked by the APB3 (PCLK3) bus clock                   */
    LPTIM_CLK_SRC_LPTIM1_PLL2P     , /**< LPTIM1 clocked by the PLL2 P output                            */
#if defined(RCC_CR_PLL3ON)
    LPTIM_CLK_SRC_LPTIM1_PLL3R     , /**< LPTIM1 clocked by the PLL3 R output                            */
#endif
    LPTIM_CLK_SRC_LPTIM1_LSE       , /**< LPTIM1 clocked by the LSE oscillator (wake-up capable)         */
    LPTIM_CLK_SRC_LPTIM1_LSI       , /**< LPTIM1 clocked by the LSI oscillator (wake-up capable)         */
    LPTIM_CLK_SRC_LPTIM1_LPCLK     , /**< LPTIM1 clocked by the low-power clock domain (wake-up capable) */
#endif /* LPTIM1 */
#if defined(LPTIM2)
    LPTIM_CLK_SRC_LPTIM2_PCLK1     , /**< LPTIM2 clocked by the APB1 (PCLK1) bus clock                   */
    LPTIM_CLK_SRC_LPTIM2_PLL2P     , /**< LPTIM2 clocked by the PLL2 P output                            */
#if defined(RCC_CR_PLL3ON)
    LPTIM_CLK_SRC_LPTIM2_PLL3R     , /**< LPTIM2 clocked by the PLL3 R output                            */
#endif
    LPTIM_CLK_SRC_LPTIM2_LSE       , /**< LPTIM2 clocked by the LSE oscillator (wake-up capable)         */
    LPTIM_CLK_SRC_LPTIM2_LSI       , /**< LPTIM2 clocked by the LSI oscillator (wake-up capable)         */
    LPTIM_CLK_SRC_LPTIM2_LPCLK     , /**< LPTIM2 clocked by the low-power clock domain (wake-up capable) */
#endif /* LPTIM2 */
#if defined(LPTIM3)
    LPTIM_CLK_SRC_LPTIM3_PCLK3     , /**< LPTIM3 clocked by the APB3 (PCLK3) bus clock                   */
    LPTIM_CLK_SRC_LPTIM3_PLL2P     , /**< LPTIM3 clocked by the PLL2 P output                            */
    LPTIM_CLK_SRC_LPTIM3_PLL3R     , /**< LPTIM3 clocked by the PLL3 R output                            */
    LPTIM_CLK_SRC_LPTIM3_LSE       , /**< LPTIM3 clocked by the LSE oscillator (wake-up capable)         */
    LPTIM_CLK_SRC_LPTIM3_LSI       , /**< LPTIM3 clocked by the LSI oscillator (wake-up capable)         */
    LPTIM_CLK_SRC_LPTIM3_LPCLK     , /**< LPTIM3 clocked by the low-power clock domain (wake-up capable) */
#endif /* LPTIM3 */
#if defined(LPTIM4)
    LPTIM_CLK_SRC_LPTIM4_PCLK3     , /**< LPTIM4 clocked by the APB3 (PCLK3) bus clock                   */
    LPTIM_CLK_SRC_LPTIM4_PLL2P     , /**< LPTIM4 clocked by the PLL2 P output                            */
    LPTIM_CLK_SRC_LPTIM4_PLL3R     , /**< LPTIM4 clocked by the PLL3 R output                            */
    LPTIM_CLK_SRC_LPTIM4_LSE       , /**< LPTIM4 clocked by the LSE oscillator (wake-up capable)         */
    LPTIM_CLK_SRC_LPTIM4_LSI       , /**< LPTIM4 clocked by the LSI oscillator (wake-up capable)         */
    LPTIM_CLK_SRC_LPTIM4_LPCLK     , /**< LPTIM4 clocked by the low-power clock domain (wake-up capable) */
#endif /* LPTIM4 */
#if defined(LPTIM5)
    LPTIM_CLK_SRC_LPTIM5_PCLK3     , /**< LPTIM5 clocked by the APB3 (PCLK3) bus clock                   */
    LPTIM_CLK_SRC_LPTIM5_PLL2P     , /**< LPTIM5 clocked by the PLL2 P output                            */
    LPTIM_CLK_SRC_LPTIM5_PLL3R     , /**< LPTIM5 clocked by the PLL3 R output                            */
    LPTIM_CLK_SRC_LPTIM5_LSE       , /**< LPTIM5 clocked by the LSE oscillator (wake-up capable)         */
    LPTIM_CLK_SRC_LPTIM5_LSI       , /**< LPTIM5 clocked by the LSI oscillator (wake-up capable)         */
    LPTIM_CLK_SRC_LPTIM5_LPCLK     , /**< LPTIM5 clocked by the low-power clock domain (wake-up capable) */
#endif /* LPTIM5 */
#if defined(LPTIM6)
    LPTIM_CLK_SRC_LPTIM6_PCLK3     , /**< LPTIM6 clocked by the APB3 (PCLK3) bus clock                   */
    LPTIM_CLK_SRC_LPTIM6_PLL2P     , /**< LPTIM6 clocked by the PLL2 P output                            */
    LPTIM_CLK_SRC_LPTIM6_PLL3R     , /**< LPTIM6 clocked by the PLL3 R output                            */
    LPTIM_CLK_SRC_LPTIM6_LSE       , /**< LPTIM6 clocked by the LSE oscillator (wake-up capable)         */
    LPTIM_CLK_SRC_LPTIM6_LSI       , /**< LPTIM6 clocked by the LSI oscillator (wake-up capable)         */
    LPTIM_CLK_SRC_LPTIM6_LPCLK     , /**< LPTIM6 clocked by the low-power clock domain (wake-up capable) */
#endif /* LPTIM6 */
    LPTIM_CLK_SRC_CNT                /**< Count of available clock source identifications                */
}   lpTim_ClkSource_t;


/** \brief Configuration of an LPTIM instance, passed to \ref LpTim_Init */
typedef struct
{
    lpTim_PeriphId_t      PeriphId;  /**< Instance to configure                                         */
    lpTim_ClkSource_t     ClkSrc;    /**< Selected clock source of the instance                         */
    lpTim_Mode_t          Mode;      /**< Operating mode of the instance                                */
    lpTim_Period_us_t     Period;    /**< Requested period in us (time base / one-shot modes)           */
    lpTim_StartMode_t     StartMode; /**< Start mode applied by \ref LpTim_Set_Start                    */
    lpTim_TriggerConfig_t Trigger;   /**< External trigger configuration                                */
    lpTim_Callback_t      Callback;  /**< Callback of the update / timeout event, may be LPTIM_NULL_PTR */
    lpTim_IrqPrio_t       IrqPrio;   /**< Interrupt priority of the instance                            */
}   lpTim_Config_t;

/* --------------------------- Pin configuration lists ----------------------- */

/**
 * \brief List of channel output (PWM / output compare) pins of the LPTIM instances
 *        (STM32CubeMX database, pins existing only on some
 *        STM32H5 lines are guarded by the CMSIS device line)
 */
typedef enum
{
#if defined(STM32H503xx)
    LPTIM_OUT_PIN_LPTIM1_CH1_PA5  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_1, GPIO_PORT_A, GPIO_PIN_ID_5,  GPIO_ALT_FUNC_4 ),   /**< LPTIM1 CH1 pin PA5 */
#endif
#if defined(STM32H503xx)
    LPTIM_OUT_PIN_LPTIM1_CH1_PA13 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_1, GPIO_PORT_A, GPIO_PIN_ID_13, GPIO_ALT_FUNC_2 ),   /**< LPTIM1 CH1 pin PA13 */
#endif
    LPTIM_OUT_PIN_LPTIM1_CH1_PB2  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_1, GPIO_PORT_B, GPIO_PIN_ID_2,  GPIO_ALT_FUNC_5 ),   /**< LPTIM1 CH1 pin PB2 */
#if defined(STM32H503xx)
    LPTIM_OUT_PIN_LPTIM1_CH1_PB3  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_1, GPIO_PORT_B, GPIO_PIN_ID_3,  GPIO_ALT_FUNC_2 ),   /**< LPTIM1 CH1 pin PB3 */
#endif
#if defined(STM32H503xx)
    LPTIM_OUT_PIN_LPTIM1_CH1_PB15 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_1, GPIO_PORT_B, GPIO_PIN_ID_15, GPIO_ALT_FUNC_2 ),   /**< LPTIM1 CH1 pin PB15 */
#endif
#if defined(STM32H503xx)
    LPTIM_OUT_PIN_LPTIM1_CH1_PC12 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_1, GPIO_PORT_C, GPIO_PIN_ID_12, GPIO_ALT_FUNC_2 ),   /**< LPTIM1 CH1 pin PC12 */
#endif
#if defined(STM32H523xx) || \
    defined(STM32H533xx) || \
    defined(STM32H543xx) || \
    defined(STM32H553xx) || \
    defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_OUT_PIN_LPTIM1_CH1_PD13 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_1, GPIO_PORT_D, GPIO_PIN_ID_13, GPIO_ALT_FUNC_1 ),   /**< LPTIM1 CH1 pin PD13 */
#endif
#if defined(STM32H523xx) || \
    defined(STM32H533xx) || \
    defined(STM32H543xx) || \
    defined(STM32H553xx) || \
    defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_OUT_PIN_LPTIM1_CH1_PG13 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_1, GPIO_PORT_G, GPIO_PIN_ID_13, GPIO_ALT_FUNC_1 ),   /**< LPTIM1 CH1 pin PG13 */
#endif
#if defined(STM32H503xx)
    LPTIM_OUT_PIN_LPTIM1_CH2_PB2  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_1, GPIO_PORT_B, GPIO_PIN_ID_2,  GPIO_ALT_FUNC_2 ),   /**< LPTIM1 CH2 pin PB2 */
#endif
    LPTIM_OUT_PIN_LPTIM1_CH2_PB4  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_1, GPIO_PORT_B, GPIO_PIN_ID_4,  GPIO_ALT_FUNC_4 ),   /**< LPTIM1 CH2 pin PB4 */
#if defined(STM32H503xx)
    LPTIM_OUT_PIN_LPTIM1_CH2_PB8  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_1, GPIO_PORT_B, GPIO_PIN_ID_8,  GPIO_ALT_FUNC_2 ),   /**< LPTIM1 CH2 pin PB8 */
#endif
#if defined(STM32H503xx)
    LPTIM_OUT_PIN_LPTIM1_CH2_PC0  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_1, GPIO_PORT_C, GPIO_PIN_ID_0,  GPIO_ALT_FUNC_2 ),   /**< LPTIM1 CH2 pin PC0 */
#endif
#if defined(STM32H523xx) || \
    defined(STM32H533xx) || \
    defined(STM32H543xx) || \
    defined(STM32H553xx) || \
    defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_OUT_PIN_LPTIM1_CH2_PG14 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_1, GPIO_PORT_G, GPIO_PIN_ID_14, GPIO_ALT_FUNC_4 ),   /**< LPTIM1 CH2 pin PG14 */
#endif
#if defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_OUT_PIN_LPTIM1_CH2_PI11 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_1, GPIO_PORT_I, GPIO_PIN_ID_11, GPIO_ALT_FUNC_4 ),   /**< LPTIM1 CH2 pin PI11 */
#endif
    LPTIM_OUT_PIN_LPTIM2_CH1_PA4  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_2, GPIO_PORT_A, GPIO_PIN_ID_4,  GPIO_ALT_FUNC_3 ),   /**< LPTIM2 CH1 pin PA4 */
#if defined(STM32H503xx)
    LPTIM_OUT_PIN_LPTIM2_CH1_PA14 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_2, GPIO_PORT_A, GPIO_PIN_ID_14, GPIO_ALT_FUNC_3 ),   /**< LPTIM2 CH1 pin PA14 */
#endif
#if defined(STM32H503xx)
    LPTIM_OUT_PIN_LPTIM2_CH1_PA15 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_2, GPIO_PORT_A, GPIO_PIN_ID_15, GPIO_ALT_FUNC_3 ),   /**< LPTIM2 CH1 pin PA15 */
#endif
#if defined(STM32H503xx)
    LPTIM_OUT_PIN_LPTIM2_CH1_PB8  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_2, GPIO_PORT_B, GPIO_PIN_ID_8,  GPIO_ALT_FUNC_14 ),  /**< LPTIM2 CH1 pin PB8 */
#endif
    LPTIM_OUT_PIN_LPTIM2_CH1_PB13 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_2, GPIO_PORT_B, GPIO_PIN_ID_13, GPIO_ALT_FUNC_3 ),   /**< LPTIM2 CH1 pin PB13 */
#if defined(STM32H503xx)
    LPTIM_OUT_PIN_LPTIM2_CH1_PC5  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_2, GPIO_PORT_C, GPIO_PIN_ID_5,  GPIO_ALT_FUNC_14 ),  /**< LPTIM2 CH1 pin PC5 */
#endif
#if defined(STM32H523xx) || \
    defined(STM32H533xx) || \
    defined(STM32H543xx) || \
    defined(STM32H553xx) || \
    defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_OUT_PIN_LPTIM2_CH1_PD13 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_2, GPIO_PORT_D, GPIO_PIN_ID_13, GPIO_ALT_FUNC_3 ),   /**< LPTIM2 CH1 pin PD13 */
#endif
#if defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_OUT_PIN_LPTIM2_CH1_PG12 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_2, GPIO_PORT_G, GPIO_PIN_ID_12, GPIO_ALT_FUNC_3 ),   /**< LPTIM2 CH1 pin PG12 */
#endif
#if defined(STM32H503xx)
    LPTIM_OUT_PIN_LPTIM2_CH2_PA5  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_2, GPIO_PORT_A, GPIO_PIN_ID_5,  GPIO_ALT_FUNC_3 ),   /**< LPTIM2 CH2 pin PA5 */
#endif
#if defined(STM32H503xx)
    LPTIM_OUT_PIN_LPTIM2_CH2_PA11 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_2, GPIO_PORT_A, GPIO_PIN_ID_11, GPIO_ALT_FUNC_14 ),  /**< LPTIM2 CH2 pin PA11 */
#endif
#if defined(STM32H503xx)
    LPTIM_OUT_PIN_LPTIM2_CH2_PB10 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_2, GPIO_PORT_B, GPIO_PIN_ID_10, GPIO_ALT_FUNC_14 ),  /**< LPTIM2 CH2 pin PB10 */
#endif
#if defined(STM32H503xx) || \
    defined(STM32H523xx) || \
    defined(STM32H533xx) || \
    defined(STM32H543xx) || \
    defined(STM32H553xx)
    LPTIM_OUT_PIN_LPTIM2_CH2_PC12 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_2, GPIO_PORT_C, GPIO_PIN_ID_12, GPIO_ALT_FUNC_3 ),   /**< LPTIM2 CH2 pin PC12 */
#endif
#if defined(STM32H523xx) || \
    defined(STM32H533xx) || \
    defined(STM32H543xx) || \
    defined(STM32H553xx) || \
    defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_OUT_PIN_LPTIM2_CH2_PD10 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_2, GPIO_PORT_D, GPIO_PIN_ID_10, GPIO_ALT_FUNC_3 ),   /**< LPTIM2 CH2 pin PD10 */
#endif
#if defined(STM32H523xx) || \
    defined(STM32H533xx) || \
    defined(STM32H543xx) || \
    defined(STM32H553xx) || \
    defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_OUT_PIN_LPTIM2_CH2_PE0  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_2, GPIO_PORT_E, GPIO_PIN_ID_0,  GPIO_ALT_FUNC_3 ),   /**< LPTIM2 CH2 pin PE0 */
#endif
#if defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_OUT_PIN_LPTIM2_CH2_PF13 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_2, GPIO_PORT_F, GPIO_PIN_ID_13, GPIO_ALT_FUNC_3 ),   /**< LPTIM2 CH2 pin PF13 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_OUT_PIN_LPTIM3_CH1_PB0  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_3, GPIO_PORT_B, GPIO_PIN_ID_0,  GPIO_ALT_FUNC_14 ),  /**< LPTIM3 CH1 pin PB0 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_OUT_PIN_LPTIM3_CH1_PB10 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_3, GPIO_PORT_B, GPIO_PIN_ID_10, GPIO_ALT_FUNC_2 ),   /**< LPTIM3 CH1 pin PB10 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_OUT_PIN_LPTIM3_CH1_PC3  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_3, GPIO_PORT_C, GPIO_PIN_ID_3,  GPIO_ALT_FUNC_3 ),   /**< LPTIM3 CH1 pin PC3 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_OUT_PIN_LPTIM3_CH1_PF5  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_3, GPIO_PORT_F, GPIO_PIN_ID_5,  GPIO_ALT_FUNC_2 ),   /**< LPTIM3 CH1 pin PF5 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_OUT_PIN_LPTIM3_CH2_PB1  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_3, GPIO_PORT_B, GPIO_PIN_ID_1,  GPIO_ALT_FUNC_14 ),  /**< LPTIM3 CH2 pin PB1 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_OUT_PIN_LPTIM3_CH2_PF2  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_3, GPIO_PORT_F, GPIO_PIN_ID_2,  GPIO_ALT_FUNC_2 ),   /**< LPTIM3 CH2 pin PF2 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_OUT_PIN_LPTIM4_CH1_PD7  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_4, GPIO_PORT_D, GPIO_PIN_ID_7,  GPIO_ALT_FUNC_14 ),  /**< LPTIM4 CH1 pin PD7 */
#endif
#if defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_OUT_PIN_LPTIM4_CH1_PG8  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_4, GPIO_PORT_G, GPIO_PIN_ID_8,  GPIO_ALT_FUNC_14 ),  /**< LPTIM4 CH1 pin PG8 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_OUT_PIN_LPTIM5_CH1_PF0  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_5, GPIO_PORT_F, GPIO_PIN_ID_0,  GPIO_ALT_FUNC_13 ),  /**< LPTIM5 CH1 pin PF0 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_OUT_PIN_LPTIM5_CH1_PF6  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_5, GPIO_PORT_F, GPIO_PIN_ID_6,  GPIO_ALT_FUNC_13 ),  /**< LPTIM5 CH1 pin PF6 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_OUT_PIN_LPTIM5_CH1_PG12 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_5, GPIO_PORT_G, GPIO_PIN_ID_12, GPIO_ALT_FUNC_14 ),  /**< LPTIM5 CH1 pin PG12 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_OUT_PIN_LPTIM5_CH2_PF1  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_5, GPIO_PORT_F, GPIO_PIN_ID_1,  GPIO_ALT_FUNC_13 ),  /**< LPTIM5 CH2 pin PF1 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_OUT_PIN_LPTIM5_CH2_PF7  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_5, GPIO_PORT_F, GPIO_PIN_ID_7,  GPIO_ALT_FUNC_13 ),  /**< LPTIM5 CH2 pin PF7 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_OUT_PIN_LPTIM5_CH2_PG13 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_5, GPIO_PORT_G, GPIO_PIN_ID_13, GPIO_ALT_FUNC_13 ),  /**< LPTIM5 CH2 pin PG13 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_OUT_PIN_LPTIM6_CH1_PF11 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_6, GPIO_PORT_F, GPIO_PIN_ID_11, GPIO_ALT_FUNC_14 ),  /**< LPTIM6 CH1 pin PF11 */
#endif
#if defined(STM32H5E5xx) || \
    defined(STM32H5F5xx)
    LPTIM_OUT_PIN_LPTIM6_CH1_PK2  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_6, GPIO_PORT_K, GPIO_PIN_ID_2,  GPIO_ALT_FUNC_13 ),  /**< LPTIM6 CH1 pin PK2 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_OUT_PIN_LPTIM6_CH2_PF12 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_6, GPIO_PORT_F, GPIO_PIN_ID_12, GPIO_ALT_FUNC_14 ),  /**< LPTIM6 CH2 pin PF12 */
#endif
    LPTIM_OUT_PIN_UNUSED          = LPTIM_CHANNEL_PIN_UNUSED                                                                                              /**< Unused */
}   lpTim_OutputPin_t;


/**
 * \brief List of channel input (capture / encoder / pulse counter) pins of the LPTIM instances
 *        (STM32CubeMX database, pins existing only on some
 *        STM32H5 lines are guarded by the CMSIS device line)
 */
typedef enum
{
    LPTIM_IN_PIN_LPTIM1_CH1_PA1  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_1, GPIO_PORT_A, GPIO_PIN_ID_1,  GPIO_ALT_FUNC_5 ),   /**< LPTIM1 CH1 pin PA1 */
#if defined(STM32H503xx)
    LPTIM_IN_PIN_LPTIM1_CH1_PB0  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_1, GPIO_PORT_B, GPIO_PIN_ID_0,  GPIO_ALT_FUNC_4 ),   /**< LPTIM1 CH1 pin PB0 */
#endif
#if defined(STM32H503xx)
    LPTIM_IN_PIN_LPTIM1_CH1_PB5  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_1, GPIO_PORT_B, GPIO_PIN_ID_5,  GPIO_ALT_FUNC_14 ),  /**< LPTIM1 CH1 pin PB5 */
#endif
#if defined(STM32H523xx) || \
    defined(STM32H533xx) || \
    defined(STM32H543xx) || \
    defined(STM32H553xx) || \
    defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_IN_PIN_LPTIM1_CH1_PD12 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_1, GPIO_PORT_D, GPIO_PIN_ID_12, GPIO_ALT_FUNC_1 ),   /**< LPTIM1 CH1 pin PD12 */
#endif
#if defined(STM32H523xx) || \
    defined(STM32H533xx) || \
    defined(STM32H543xx) || \
    defined(STM32H553xx) || \
    defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_IN_PIN_LPTIM1_CH1_PG12 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_1, GPIO_PORT_G, GPIO_PIN_ID_12, GPIO_ALT_FUNC_1 ),   /**< LPTIM1 CH1 pin PG12 */
#endif
#if defined(STM32H503xx)
    LPTIM_IN_PIN_LPTIM1_CH2_PA0  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_1, GPIO_PORT_A, GPIO_PIN_ID_0,  GPIO_ALT_FUNC_5 ),   /**< LPTIM1 CH2 pin PA0 */
#endif
    LPTIM_IN_PIN_LPTIM1_CH2_PA2  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_1, GPIO_PORT_A, GPIO_PIN_ID_2,  GPIO_ALT_FUNC_5 ),   /**< LPTIM1 CH2 pin PA2 */
#if defined(STM32H503xx)
    LPTIM_IN_PIN_LPTIM1_CH2_PA15 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_1, GPIO_PORT_A, GPIO_PIN_ID_15, GPIO_ALT_FUNC_2 ),   /**< LPTIM1 CH2 pin PA15 */
#endif
#if defined(STM32H503xx)
    LPTIM_IN_PIN_LPTIM1_CH2_PB1  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_1, GPIO_PORT_B, GPIO_PIN_ID_1,  GPIO_ALT_FUNC_5 ),   /**< LPTIM1 CH2 pin PB1 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_IN_PIN_LPTIM1_CH2_PE1  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_1, GPIO_PORT_E, GPIO_PIN_ID_1,  GPIO_ALT_FUNC_1 ),   /**< LPTIM1 CH2 pin PE1 */
#endif
#if defined(STM32H523xx) || \
    defined(STM32H533xx) || \
    defined(STM32H543xx) || \
    defined(STM32H553xx) || \
    defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_IN_PIN_LPTIM1_CH2_PE2  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_1, GPIO_PORT_E, GPIO_PIN_ID_2,  GPIO_ALT_FUNC_1 ),   /**< LPTIM1 CH2 pin PE2 */
#endif
#if defined(STM32H523xx) || \
    defined(STM32H533xx) || \
    defined(STM32H543xx) || \
    defined(STM32H553xx) || \
    defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_IN_PIN_LPTIM1_CH2_PG11 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_1, GPIO_PORT_G, GPIO_PIN_ID_11, GPIO_ALT_FUNC_1 ),   /**< LPTIM1 CH2 pin PG11 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_IN_PIN_LPTIM1_CH2_PH2  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_1, GPIO_PORT_H, GPIO_PIN_ID_2,  GPIO_ALT_FUNC_1 ),   /**< LPTIM1 CH2 pin PH2 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_IN_PIN_LPTIM1_CH2_PH13 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_1, GPIO_PORT_H, GPIO_PIN_ID_13, GPIO_ALT_FUNC_1 ),   /**< LPTIM1 CH2 pin PH13 */
#endif
#if defined(STM32H503xx)
    LPTIM_IN_PIN_LPTIM2_CH1_PA8  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_2, GPIO_PORT_A, GPIO_PIN_ID_8,  GPIO_ALT_FUNC_3 ),   /**< LPTIM2 CH1 pin PA8 */
#endif
    LPTIM_IN_PIN_LPTIM2_CH1_PB10 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_2, GPIO_PORT_B, GPIO_PIN_ID_10, GPIO_ALT_FUNC_3 ),   /**< LPTIM2 CH1 pin PB10 */
#if defined(STM32H503xx)
    LPTIM_IN_PIN_LPTIM2_CH1_PC1  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_2, GPIO_PORT_C, GPIO_PIN_ID_1,  GPIO_ALT_FUNC_14 ),  /**< LPTIM2 CH1 pin PC1 */
#endif
#if defined(STM32H523xx) || \
    defined(STM32H533xx) || \
    defined(STM32H543xx) || \
    defined(STM32H553xx) || \
    defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_IN_PIN_LPTIM2_CH1_PD12 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_2, GPIO_PORT_D, GPIO_PIN_ID_12, GPIO_ALT_FUNC_3 ),   /**< LPTIM2 CH1 pin PD12 */
#endif
#if defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_IN_PIN_LPTIM2_CH1_PF14 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_2, GPIO_PORT_F, GPIO_PIN_ID_14, GPIO_ALT_FUNC_3 ),   /**< LPTIM2 CH1 pin PF14 */
#endif
    LPTIM_IN_PIN_LPTIM2_CH2_PA10 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_2, GPIO_PORT_A, GPIO_PIN_ID_10, GPIO_ALT_FUNC_4 ),   /**< LPTIM2 CH2 pin PA10 */
#if defined(STM32H503xx)
    LPTIM_IN_PIN_LPTIM2_CH2_PB15 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_2, GPIO_PORT_B, GPIO_PIN_ID_15, GPIO_ALT_FUNC_3 ),   /**< LPTIM2 CH2 pin PB15 */
#endif
#if defined(STM32H503xx)
    LPTIM_IN_PIN_LPTIM2_CH2_PC3  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_2, GPIO_PORT_C, GPIO_PIN_ID_3,  GPIO_ALT_FUNC_14 ),  /**< LPTIM2 CH2 pin PC3 */
#endif
#if defined(STM32H523xx) || \
    defined(STM32H533xx) || \
    defined(STM32H543xx) || \
    defined(STM32H553xx) || \
    defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_IN_PIN_LPTIM2_CH2_PD11 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_2, GPIO_PORT_D, GPIO_PIN_ID_11, GPIO_ALT_FUNC_3 ),   /**< LPTIM2 CH2 pin PD11 */
#endif
#if defined(STM32H5E5xx) || \
    defined(STM32H5F5xx)
    LPTIM_IN_PIN_LPTIM2_CH2_PJ4  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_2, GPIO_PORT_J, GPIO_PIN_ID_4,  GPIO_ALT_FUNC_3 ),   /**< LPTIM2 CH2 pin PJ4 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_IN_PIN_LPTIM3_CH1_PB13 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_3, GPIO_PORT_B, GPIO_PIN_ID_13, GPIO_ALT_FUNC_2 ),   /**< LPTIM3 CH1 pin PB13 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_IN_PIN_LPTIM3_CH1_PC11 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_3, GPIO_PORT_C, GPIO_PIN_ID_11, GPIO_ALT_FUNC_2 ),   /**< LPTIM3 CH1 pin PC11 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_IN_PIN_LPTIM3_CH1_PF3  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_3, GPIO_PORT_F, GPIO_PIN_ID_3,  GPIO_ALT_FUNC_2 ),   /**< LPTIM3 CH1 pin PF3 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_IN_PIN_LPTIM3_CH1_PF5  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_3, GPIO_PORT_F, GPIO_PIN_ID_5,  GPIO_ALT_FUNC_14 ),  /**< LPTIM3 CH1 pin PF5 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_IN_PIN_LPTIM3_CH2_PA15 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_3, GPIO_PORT_A, GPIO_PIN_ID_15, GPIO_ALT_FUNC_2 ),   /**< LPTIM3 CH2 pin PA15 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_IN_PIN_LPTIM3_CH2_PF2  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_3, GPIO_PORT_F, GPIO_PIN_ID_2,  GPIO_ALT_FUNC_3 ),   /**< LPTIM3 CH2 pin PF2 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_IN_PIN_LPTIM4_CH1_PD13 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_4, GPIO_PORT_D, GPIO_PIN_ID_13, GPIO_ALT_FUNC_14 ),  /**< LPTIM4 CH1 pin PD13 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_IN_PIN_LPTIM4_CH1_PG0  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_4, GPIO_PORT_G, GPIO_PIN_ID_0,  GPIO_ALT_FUNC_14 ),  /**< LPTIM4 CH1 pin PG0 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_IN_PIN_LPTIM5_CH1_PF2  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_5, GPIO_PORT_F, GPIO_PIN_ID_2,  GPIO_ALT_FUNC_13 ),  /**< LPTIM5 CH1 pin PF2 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_IN_PIN_LPTIM5_CH1_PF8  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_5, GPIO_PORT_F, GPIO_PIN_ID_8,  GPIO_ALT_FUNC_13 ),  /**< LPTIM5 CH1 pin PF8 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_IN_PIN_LPTIM5_CH1_PG14 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_5, GPIO_PORT_G, GPIO_PIN_ID_14, GPIO_ALT_FUNC_13 ),  /**< LPTIM5 CH1 pin PG14 */
#endif
#if defined(STM32H5E5xx) || \
    defined(STM32H5F5xx)
    LPTIM_IN_PIN_LPTIM5_CH1_PJ9  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_5, GPIO_PORT_J, GPIO_PIN_ID_9,  GPIO_ALT_FUNC_13 ),  /**< LPTIM5 CH1 pin PJ9 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_IN_PIN_LPTIM5_CH2_PF3  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_5, GPIO_PORT_F, GPIO_PIN_ID_3,  GPIO_ALT_FUNC_13 ),  /**< LPTIM5 CH2 pin PF3 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_IN_PIN_LPTIM5_CH2_PF9  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_5, GPIO_PORT_F, GPIO_PIN_ID_9,  GPIO_ALT_FUNC_13 ),  /**< LPTIM5 CH2 pin PF9 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_IN_PIN_LPTIM6_CH1_PF13 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_6, GPIO_PORT_F, GPIO_PIN_ID_13, GPIO_ALT_FUNC_14 ),  /**< LPTIM6 CH1 pin PF13 */
#endif
#if defined(STM32H5E5xx) || \
    defined(STM32H5F5xx)
    LPTIM_IN_PIN_LPTIM6_CH1_PK0  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_1, LPTIM_PERIPH_6, GPIO_PORT_K, GPIO_PIN_ID_0,  GPIO_ALT_FUNC_13 ),  /**< LPTIM6 CH1 pin PK0 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_IN_PIN_LPTIM6_CH2_PF14 = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_6, GPIO_PORT_F, GPIO_PIN_ID_14, GPIO_ALT_FUNC_14 ),  /**< LPTIM6 CH2 pin PF14 */
#endif
#if defined(STM32H5E5xx) || \
    defined(STM32H5F5xx)
    LPTIM_IN_PIN_LPTIM6_CH2_PK1  = LPTIM_CHANNEL_PIN_BIT_MASK_ENCODE( LPTIM_CHANNEL_2, LPTIM_PERIPH_6, GPIO_PORT_K, GPIO_PIN_ID_1,  GPIO_ALT_FUNC_13 ),  /**< LPTIM6 CH2 pin PK1 */
#endif
    LPTIM_IN_PIN_UNUSED          = LPTIM_CHANNEL_PIN_UNUSED                                                                                              /**< Unused */
}   lpTim_InputPin_t;


/**
 * \brief List of external trigger input pins of the LPTIM instances
 *        (STM32CubeMX database, pins existing only on some
 *        STM32H5 lines are guarded by the CMSIS device line)
 */
typedef enum
{
#if defined(STM32H503xx)
    LPTIM_TRIG_PIN_LPTIM1_PA0  = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_1, GPIO_PORT_A, GPIO_PIN_ID_0,  GPIO_ALT_FUNC_4 ),   /**< LPTIM1 pin PA0 */
#endif
#if defined(STM32H503xx)
    LPTIM_TRIG_PIN_LPTIM1_PA5  = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_1, GPIO_PORT_A, GPIO_PIN_ID_5,  GPIO_ALT_FUNC_2 ),   /**< LPTIM1 pin PA5 */
#endif
#if defined(STM32H503xx)
    LPTIM_TRIG_PIN_LPTIM1_PA14 = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_1, GPIO_PORT_A, GPIO_PIN_ID_14, GPIO_ALT_FUNC_4 ),   /**< LPTIM1 pin PA14 */
#endif
#if defined(STM32H503xx)
    LPTIM_TRIG_PIN_LPTIM1_PB14 = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_1, GPIO_PORT_B, GPIO_PIN_ID_14, GPIO_ALT_FUNC_2 ),   /**< LPTIM1 pin PB14 */
#endif
#if defined(STM32H523xx) || \
    defined(STM32H533xx) || \
    defined(STM32H543xx) || \
    defined(STM32H553xx) || \
    defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_TRIG_PIN_LPTIM1_PE0  = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_1, GPIO_PORT_E, GPIO_PIN_ID_0,  GPIO_ALT_FUNC_1 ),   /**< LPTIM1 pin PE0 */
#endif
#if defined(STM32H523xx) || \
    defined(STM32H533xx) || \
    defined(STM32H543xx) || \
    defined(STM32H553xx) || \
    defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_TRIG_PIN_LPTIM1_PG14 = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_1, GPIO_PORT_G, GPIO_PIN_ID_14, GPIO_ALT_FUNC_1 ),   /**< LPTIM1 pin PG14 */
#endif
#if defined(STM32H5E5xx) || \
    defined(STM32H5F5xx)
    LPTIM_TRIG_PIN_LPTIM1_PJ11 = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_1, GPIO_PORT_J, GPIO_PIN_ID_11, GPIO_ALT_FUNC_4 ),   /**< LPTIM1 pin PJ11 */
#endif
#if defined(STM32H503xx)
    LPTIM_TRIG_PIN_LPTIM2_PA0  = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_2, GPIO_PORT_A, GPIO_PIN_ID_0,  GPIO_ALT_FUNC_3 ),   /**< LPTIM2 pin PA0 */
#endif
#if defined(STM32H503xx)
    LPTIM_TRIG_PIN_LPTIM2_PA7  = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_2, GPIO_PORT_A, GPIO_PIN_ID_7,  GPIO_ALT_FUNC_3 ),   /**< LPTIM2 pin PA7 */
#endif
#if defined(STM32H503xx)
    LPTIM_TRIG_PIN_LPTIM2_PB1  = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_2, GPIO_PORT_B, GPIO_PIN_ID_1,  GPIO_ALT_FUNC_3 ),   /**< LPTIM2 pin PB1 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_TRIG_PIN_LPTIM2_PB11 = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_2, GPIO_PORT_B, GPIO_PIN_ID_11, GPIO_ALT_FUNC_3 ),   /**< LPTIM2 pin PB11 */
#endif
    LPTIM_TRIG_PIN_LPTIM2_PC4  = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_2, GPIO_PORT_C, GPIO_PIN_ID_4,  GPIO_ALT_FUNC_3 ),   /**< LPTIM2 pin PC4 */
#if defined(STM32H523xx) || \
    defined(STM32H533xx) || \
    defined(STM32H543xx) || \
    defined(STM32H553xx) || \
    defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_TRIG_PIN_LPTIM2_PE0  = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_2, GPIO_PORT_E, GPIO_PIN_ID_0,  GPIO_ALT_FUNC_4 ),   /**< LPTIM2 pin PE0 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_TRIG_PIN_LPTIM3_PB14 = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_3, GPIO_PORT_B, GPIO_PIN_ID_14, GPIO_ALT_FUNC_14 ),  /**< LPTIM3 pin PB14 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_TRIG_PIN_LPTIM3_PC10 = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_3, GPIO_PORT_C, GPIO_PIN_ID_10, GPIO_ALT_FUNC_2 ),   /**< LPTIM3 pin PC10 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_TRIG_PIN_LPTIM3_PF4  = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_3, GPIO_PORT_F, GPIO_PIN_ID_4,  GPIO_ALT_FUNC_2 ),   /**< LPTIM3 pin PF4 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_TRIG_PIN_LPTIM4_PD2  = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_4, GPIO_PORT_D, GPIO_PIN_ID_2,  GPIO_ALT_FUNC_14 ),  /**< LPTIM4 pin PD2 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_TRIG_PIN_LPTIM4_PG4  = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_4, GPIO_PORT_G, GPIO_PIN_ID_4,  GPIO_ALT_FUNC_14 ),  /**< LPTIM4 pin PG4 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_TRIG_PIN_LPTIM5_PB2  = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_5, GPIO_PORT_B, GPIO_PIN_ID_2,  GPIO_ALT_FUNC_13 ),  /**< LPTIM5 pin PB2 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_TRIG_PIN_LPTIM5_PG3  = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_5, GPIO_PORT_G, GPIO_PIN_ID_3,  GPIO_ALT_FUNC_13 ),  /**< LPTIM5 pin PG3 */
#endif
#if defined(STM32H5E5xx) || \
    defined(STM32H5F5xx)
    LPTIM_TRIG_PIN_LPTIM5_PJ8  = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_5, GPIO_PORT_J, GPIO_PIN_ID_8,  GPIO_ALT_FUNC_13 ),  /**< LPTIM5 pin PJ8 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_TRIG_PIN_LPTIM6_PB3  = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_6, GPIO_PORT_B, GPIO_PIN_ID_3,  GPIO_ALT_FUNC_14 ),  /**< LPTIM6 pin PB3 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_TRIG_PIN_LPTIM6_PG2  = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_6, GPIO_PORT_G, GPIO_PIN_ID_2,  GPIO_ALT_FUNC_14 ),  /**< LPTIM6 pin PG2 */
#endif
    LPTIM_TRIG_PIN_UNUSED      = LPTIM_PERIPH_PIN_UNUSED                                                                             /**< Unused */
}   lpTim_TriggerPin_t;


/**
 * \brief List of external clock input (shares the CH1 input pin) pins of the LPTIM instances
 *        (STM32CubeMX database, pins existing only on some
 *        STM32H5 lines are guarded by the CMSIS device line)
 */
typedef enum
{
    LPTIM_EXTCLK_PIN_LPTIM1_PA1  = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_1, GPIO_PORT_A, GPIO_PIN_ID_1,  GPIO_ALT_FUNC_5 ),   /**< LPTIM1 pin PA1 */
#if defined(STM32H503xx)
    LPTIM_EXTCLK_PIN_LPTIM1_PB0  = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_1, GPIO_PORT_B, GPIO_PIN_ID_0,  GPIO_ALT_FUNC_4 ),   /**< LPTIM1 pin PB0 */
#endif
#if defined(STM32H503xx)
    LPTIM_EXTCLK_PIN_LPTIM1_PB5  = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_1, GPIO_PORT_B, GPIO_PIN_ID_5,  GPIO_ALT_FUNC_14 ),  /**< LPTIM1 pin PB5 */
#endif
#if defined(STM32H523xx) || \
    defined(STM32H533xx) || \
    defined(STM32H543xx) || \
    defined(STM32H553xx) || \
    defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_EXTCLK_PIN_LPTIM1_PD12 = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_1, GPIO_PORT_D, GPIO_PIN_ID_12, GPIO_ALT_FUNC_1 ),   /**< LPTIM1 pin PD12 */
#endif
#if defined(STM32H523xx) || \
    defined(STM32H533xx) || \
    defined(STM32H543xx) || \
    defined(STM32H553xx) || \
    defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_EXTCLK_PIN_LPTIM1_PG12 = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_1, GPIO_PORT_G, GPIO_PIN_ID_12, GPIO_ALT_FUNC_1 ),   /**< LPTIM1 pin PG12 */
#endif
#if defined(STM32H503xx)
    LPTIM_EXTCLK_PIN_LPTIM2_PA8  = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_2, GPIO_PORT_A, GPIO_PIN_ID_8,  GPIO_ALT_FUNC_3 ),   /**< LPTIM2 pin PA8 */
#endif
    LPTIM_EXTCLK_PIN_LPTIM2_PB10 = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_2, GPIO_PORT_B, GPIO_PIN_ID_10, GPIO_ALT_FUNC_3 ),   /**< LPTIM2 pin PB10 */
#if defined(STM32H503xx)
    LPTIM_EXTCLK_PIN_LPTIM2_PC1  = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_2, GPIO_PORT_C, GPIO_PIN_ID_1,  GPIO_ALT_FUNC_14 ),  /**< LPTIM2 pin PC1 */
#endif
#if defined(STM32H523xx) || \
    defined(STM32H533xx) || \
    defined(STM32H543xx) || \
    defined(STM32H553xx) || \
    defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_EXTCLK_PIN_LPTIM2_PD12 = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_2, GPIO_PORT_D, GPIO_PIN_ID_12, GPIO_ALT_FUNC_3 ),   /**< LPTIM2 pin PD12 */
#endif
#if defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_EXTCLK_PIN_LPTIM2_PF14 = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_2, GPIO_PORT_F, GPIO_PIN_ID_14, GPIO_ALT_FUNC_3 ),   /**< LPTIM2 pin PF14 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_EXTCLK_PIN_LPTIM3_PB13 = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_3, GPIO_PORT_B, GPIO_PIN_ID_13, GPIO_ALT_FUNC_2 ),   /**< LPTIM3 pin PB13 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_EXTCLK_PIN_LPTIM3_PC11 = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_3, GPIO_PORT_C, GPIO_PIN_ID_11, GPIO_ALT_FUNC_2 ),   /**< LPTIM3 pin PC11 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_EXTCLK_PIN_LPTIM3_PF3  = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_3, GPIO_PORT_F, GPIO_PIN_ID_3,  GPIO_ALT_FUNC_2 ),   /**< LPTIM3 pin PF3 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_EXTCLK_PIN_LPTIM3_PF5  = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_3, GPIO_PORT_F, GPIO_PIN_ID_5,  GPIO_ALT_FUNC_14 ),  /**< LPTIM3 pin PF5 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_EXTCLK_PIN_LPTIM4_PD13 = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_4, GPIO_PORT_D, GPIO_PIN_ID_13, GPIO_ALT_FUNC_14 ),  /**< LPTIM4 pin PD13 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_EXTCLK_PIN_LPTIM4_PG0  = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_4, GPIO_PORT_G, GPIO_PIN_ID_0,  GPIO_ALT_FUNC_14 ),  /**< LPTIM4 pin PG0 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_EXTCLK_PIN_LPTIM5_PF2  = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_5, GPIO_PORT_F, GPIO_PIN_ID_2,  GPIO_ALT_FUNC_13 ),  /**< LPTIM5 pin PF2 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_EXTCLK_PIN_LPTIM5_PF8  = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_5, GPIO_PORT_F, GPIO_PIN_ID_8,  GPIO_ALT_FUNC_13 ),  /**< LPTIM5 pin PF8 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_EXTCLK_PIN_LPTIM5_PG14 = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_5, GPIO_PORT_G, GPIO_PIN_ID_14, GPIO_ALT_FUNC_13 ),  /**< LPTIM5 pin PG14 */
#endif
#if defined(STM32H5E5xx) || \
    defined(STM32H5F5xx)
    LPTIM_EXTCLK_PIN_LPTIM5_PJ9  = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_5, GPIO_PORT_J, GPIO_PIN_ID_9,  GPIO_ALT_FUNC_13 ),  /**< LPTIM5 pin PJ9 */
#endif
#if defined(STM32H562xx) || \
    defined(STM32H563xx) || \
    defined(STM32H573xx) || \
    defined(STM32H5E4xx) || \
    defined(STM32H5E5xx) || \
    defined(STM32H5F4xx) || \
    defined(STM32H5F5xx)
    LPTIM_EXTCLK_PIN_LPTIM6_PF13 = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_6, GPIO_PORT_F, GPIO_PIN_ID_13, GPIO_ALT_FUNC_14 ),  /**< LPTIM6 pin PF13 */
#endif
#if defined(STM32H5E5xx) || \
    defined(STM32H5F5xx)
    LPTIM_EXTCLK_PIN_LPTIM6_PK0  = LPTIM_PERIPH_PIN_BIT_MASK_ENCODE( LPTIM_PERIPH_6, GPIO_PORT_K, GPIO_PIN_ID_0,  GPIO_ALT_FUNC_13 ),  /**< LPTIM6 pin PK0 */
#endif
    LPTIM_EXTCLK_PIN_UNUSED      = LPTIM_PERIPH_PIN_UNUSED                                                                             /**< Unused */
}   lpTim_ExtClkPin_t;

#endif /* LPTIM_LPTIM_TYPES_H */
