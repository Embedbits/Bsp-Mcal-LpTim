/**
 * \author Mr.Nobody
 * \file LpTim.h
 * \ingroup LpTim
 * \brief LpTim module private header file
 *
 * This file contains the functionality private to the module (not shared
 * with the application or other modules through the Port file).
 *
 */

#ifndef LPTIM_LPTIM_H
#define LPTIM_LPTIM_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================= INCLUDES =================================== */
#include "LpTim_Port.h"                     /* Module public interface        */
#include "LpTim_Reg.h"                      /* Module register component      */
/* ============================= TYPEDEFS =================================== */

/** \brief Run-time state kept by the module for a single LPTIM instance */
typedef struct
{
    lpTim_FunctionState_t Used;                           /**< LPTIM_FUNCTION_ACTIVE once \ref LpTim_Init succeeded  */
    lpTim_FunctionState_t Running;                        /**< LPTIM_FUNCTION_ACTIVE while the instance is running   */
    lpTim_Mode_t          Mode;                           /**< Configured operating mode                             */
    lpTim_StartMode_t     StartMode;                      /**< Configured start mode                                 */
    lpTim_ClkSource_t     ClkSrc;                         /**< Configured clock source                               */
    lpTim_FunctionState_t WakeupCapable;                  /**< LPTIM_FUNCTION_ACTIVE if ClkSrc runs in Stop mode     */
    lpTim_Period_us_t     Period;                         /**< Last requested period in us                           */
    lpTim_Frequency_Hz_t  ClkFreqHz;                      /**< Kernel clock frequency of the configured clock source */
    lpTim_Direction_t     Direction;                      /**< Last direction reported by \ref LpTim_Task            */
    lpTim_Callback_t      EventCbk[LPTIM_EVENT_CNT];      /**< Callback of every event (UPDATE included)             */
    lpTim_ChannelMode_t   ChannelMode[LPTIM_CHANNEL_CNT]; /**< Configured mode of every channel                      */
}   lpTim_State_t;

/* ========================= SYMBOLIC CONSTANTS ============================= */

/** Bound of the wait loop on a register write confirmation flag */
#define LPTIM_REG_WAIT_LIMIT                 ( 10000u )

/** Width, in bits, of the 16-bit counter / auto-reload / compare registers */
#define LPTIM_COUNTER_BIT_WIDTH              ( 16u )

/** Smallest prescaler divider value (CFGR.PRESC = 0) */
#define LPTIM_PRESCALER_MIN                  ( 1u )

/** Largest prescaler divider value (CFGR.PRESC = 7) */
#define LPTIM_PRESCALER_MAX                  ( 128u )

/** Number of microseconds in one second, used for the period / frequency conversion */
#define LPTIM_US_PER_SECOND                  ( 1000000u )

/** Permille base used for the PWM duty cycle conversion */
#define LPTIM_DUTY_PERMILLE_MAX              ( 1000u )

/* ========================= EXPORTED MACROS ================================ */
/* ========================= EXPORTED VARIABLES ============================= */
/* ======================== EXPORTED FUNCTIONS ============================== */

#ifdef __cplusplus
}
#endif

#endif /* LPTIM_LPTIM_H */
