/**
 * \author Mr.Nobody
 * \file LpTim_Port.h
 * \ingroup LpTim
 * \brief LpTim module public interface header file
 *
 * This file contains the public interface of the module, identical for all
 * supported families. Include this file to use the module from an application
 * or from another module. Every function is documented at its definition in
 * LpTim.c.
 *
 */

#ifndef LPTIM_LPTIM_PORT_H
#define LPTIM_LPTIM_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================= INCLUDES =================================== */
#include "LpTim_Types.h"                    /* Module types definition        */
/* ============================= TYPEDEFS =================================== */
/* ========================= SYMBOLIC CONSTANTS ============================= */
/* ========================= EXPORTED MACROS ================================ */
/* ========================= EXPORTED VARIABLES ============================= */
/* ======================== EXPORTED FUNCTIONS ============================== */

/* --------------------------- Module management ----------------------------- */

lpTim_ModuleVersion_t LpTim_Get_ModuleVersion  ( void );

lpTim_RequestState_t  LpTim_Init               ( const lpTim_Config_t * const lpTimConfig );
lpTim_RequestState_t  LpTim_Deinit              ( lpTim_PeriphId_t periphId );
void                  LpTim_Task                ( void );
lpTim_RequestState_t  LpTim_Get_DefaultConfig   ( lpTim_Config_t * const lpTimConfig );

/* ------------------------------ Run control --------------------------------- */

lpTim_RequestState_t  LpTim_Set_Start           ( lpTim_PeriphId_t periphId, lpTim_StartMode_t startMode );
lpTim_RequestState_t  LpTim_Set_Stop            ( lpTim_PeriphId_t periphId );
lpTim_RequestState_t  LpTim_Get_State           ( lpTim_PeriphId_t periphId, lpTim_FunctionState_t * const runState );
lpTim_RequestState_t  LpTim_Set_Timeout         ( lpTim_PeriphId_t periphId,
                                                  lpTim_Period_us_t timeout,
                                                  lpTim_Callback_t callback );

/* ------------------------------- Time base ----------------------------------- */

lpTim_RequestState_t  LpTim_Set_Period          ( lpTim_PeriphId_t periphId, lpTim_Period_us_t period );
lpTim_RequestState_t  LpTim_Get_Period          ( lpTim_PeriphId_t periphId, lpTim_Period_us_t * const period );
lpTim_RequestState_t  LpTim_Get_Counter         ( lpTim_PeriphId_t periphId, lpTim_Counter_t * const counter );
lpTim_RequestState_t  LpTim_Get_ClkFrequency    ( lpTim_PeriphId_t periphId, lpTim_Frequency_Hz_t * const clkFreq );
lpTim_RequestState_t  LpTim_Set_Repetition      ( lpTim_PeriphId_t periphId, lpTim_Repetition_t repetition );

/* -------------------------------- Channels ------------------------------------ */

lpTim_RequestState_t  LpTim_Set_ChannelConfig   ( lpTim_PeriphId_t periphId,
                                                  lpTim_ChannelId_t channelId,
                                                  const lpTim_ChannelConfig_t * const channelConfig );
lpTim_RequestState_t  LpTim_Set_PwmDuty         ( lpTim_PeriphId_t periphId,
                                                  lpTim_ChannelId_t channelId,
                                                  lpTim_Duty_permille_t duty );
lpTim_RequestState_t  LpTim_Get_Capture         ( lpTim_PeriphId_t periphId,
                                                  lpTim_ChannelId_t channelId,
                                                  lpTim_Counter_t * const capture );

/* ---------------------------- Trigger and inputs ------------------------------- */

lpTim_RequestState_t  LpTim_Set_Trigger         ( lpTim_PeriphId_t periphId,
                                                   const lpTim_TriggerConfig_t * const triggerConfig );
lpTim_RequestState_t  LpTim_Set_InputFilter     ( lpTim_PeriphId_t periphId,
                                                  lpTim_ChannelId_t channelId,
                                                  lpTim_Filter_t filter );

/* ------------------------- Encoder and pulse counter ---------------------------- */

lpTim_RequestState_t  LpTim_Set_EncoderConfig   ( lpTim_PeriphId_t periphId,
                                                   const lpTim_EncoderConfig_t * const encoderConfig );
lpTim_RequestState_t  LpTim_Get_EncoderDirection( lpTim_PeriphId_t periphId, lpTim_Direction_t * const direction );
lpTim_RequestState_t  LpTim_Get_PulseCount      ( lpTim_PeriphId_t periphId, lpTim_Counter_t * const pulseCount );

/* --------------------------- Interrupts and DMA --------------------------------- */

lpTim_RequestState_t  LpTim_Set_IrqPriority     ( lpTim_PeriphId_t periphId, lpTim_IrqPrio_t irqPrio );
lpTim_RequestState_t  LpTim_Set_IrqActive       ( lpTim_PeriphId_t periphId, lpTim_EventId_t eventId );
lpTim_RequestState_t  LpTim_Set_IrqInactive     ( lpTim_PeriphId_t periphId, lpTim_EventId_t eventId );
lpTim_RequestState_t  LpTim_Set_IrqCallback     ( lpTim_PeriphId_t periphId,
                                                  lpTim_EventId_t eventId,
                                                  lpTim_Callback_t callback );
lpTim_RequestState_t  LpTim_Set_DmaRequestActive  ( lpTim_PeriphId_t periphId, lpTim_EventId_t eventId );
lpTim_RequestState_t  LpTim_Set_DmaRequestInactive( lpTim_PeriphId_t periphId, lpTim_EventId_t eventId );

/* ---------------------------------- Pins ---------------------------------------- */

lpTim_RequestState_t  LpTim_InitOutputGpio      ( lpTim_OutputPin_t pinId );
lpTim_RequestState_t  LpTim_InitInputGpio       ( lpTim_InputPin_t pinId );
lpTim_RequestState_t  LpTim_InitTriggerGpio     ( lpTim_TriggerPin_t pinId );
lpTim_RequestState_t  LpTim_InitExtClkGpio      ( lpTim_ExtClkPin_t pinId );

#ifdef __cplusplus
}
#endif

#endif /* LPTIM_LPTIM_PORT_H */
