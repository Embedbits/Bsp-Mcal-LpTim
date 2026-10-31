/**
 * \author Mr.Nobody
 * \file LpTim_Reg.h
 * \ingroup LpTim
 * \brief LpTim module Reg component header file
 *
 * This component contains the functionality for register operations. It is
 * private to the module (the register access is not shared with other modules).
 * The LPTIM instances are multi-instance: every accessor takes the instance
 * identification and addresses the register of that one instance only.
 *
 */

#ifndef LPTIM_REG_LPTIM_REG_H
#define LPTIM_REG_LPTIM_REG_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================= INCLUDES =================================== */
#include "LpTim_Types.h"                    /* Module types definition        */
/* ============================= TYPEDEFS =================================== */

/** \brief List of the LPTIM registers used by the module (one set per instance) */
typedef enum
{
    LPTIM_REG_ISR   = 0u, /**< Interrupt and status register */
    LPTIM_REG_ICR       , /**< Interrupt clear register      */
    LPTIM_REG_DIER      , /**< Interrupt enable register     */
    LPTIM_REG_CFGR      , /**< Configuration register        */
    LPTIM_REG_CR        , /**< Control register              */
    LPTIM_REG_CCR1      , /**< Capture/compare register 1    */
    LPTIM_REG_ARR       , /**< Auto-reload register          */
    LPTIM_REG_CNTR      , /**< Counter register              */
    LPTIM_REG_CFGR2     , /**< Configuration register 2      */
    LPTIM_REG_RCR       , /**< Repetition register           */
    LPTIM_REG_CCMR1     , /**< Capture/compare mode register */
    LPTIM_REG_CCR2      , /**< Capture/compare register 2    */
    LPTIM_REG_CNT         /**< Count of available registers  */
}   lpTim_RegId_t;

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ========================= EXPORTED MACROS ================================ */

/* ========================= EXPORTED VARIABLES ============================= */

/* ======================== EXPORTED FUNCTIONS ============================== */

/* ---------------------------- Generic accessors --------------------------- */

void                 LpTim_Set_RegBit         ( lpTim_PeriphId_t periphId, lpTim_RegId_t regId, uint32_t bitMask );
void                 LpTim_Reset_RegBit       ( lpTim_PeriphId_t periphId, lpTim_RegId_t regId, uint32_t bitMask );
uint32_t             LpTim_Get_RegBit         ( lpTim_PeriphId_t periphId, lpTim_RegId_t regId, uint32_t bitMask );

void                 LpTim_Set_RegVal         ( lpTim_PeriphId_t periphId,
                                                lpTim_RegId_t regId,
                                                uint32_t regMask,
                                                uint32_t regValue );
uint32_t             LpTim_Get_RegVal         ( lpTim_PeriphId_t periphId, lpTim_RegId_t regId, uint32_t regMask );

/* ----------------------------- Register protocol -------------------------- */

lpTim_RequestState_t LpTim_Reg_Wait_RegBitSet ( lpTim_PeriphId_t periphId, lpTim_RegId_t regId, uint32_t bitMask );
lpTim_RequestState_t LpTim_Reg_Wait_RegBitClr ( lpTim_PeriphId_t periphId, lpTim_RegId_t regId, uint32_t bitMask );

#ifdef __cplusplus
}
#endif

#endif /* LPTIM_REG_LPTIM_REG_H */
