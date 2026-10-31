/**
 * \author Mr.Nobody
 * \file LpTim_Reg.c
 * \ingroup LpTim
 * \brief LpTim module Reg component source file
 *
 * This component contains the functionality for register operations of the
 * multi-instance LPTIM peripheral: every accessor takes the instance
 * identification in addition to the register identification, the address is
 * built from the instance base address and the byte offset of the register
 * inside the CMSIS LPTIM_TypeDef structure.
 *
 */

/* ============================= INCLUDES =================================== */
#include "LpTim_Reg.h"                      /* Module Reg component header    */
#include "stddef.h"                         /* offsetof() definition          */
/* ========================== PRIVATE DECLARATIONS =========================== */

/** \brief Count of read-back iterations of a wait on a register bit */
#define LPTIM_REG_WAIT_TIMEOUT_RAW     ( 10000u )

/** \brief Register value without any bit set */
#define LPTIM_REG_BITS_CLEAR           ( 0u )

/* =========================== LOCAL VARIABLES ============================== */

/** \brief Base address of every available LPTIM instance, indexed by lpTim_PeriphId_t */
static LPTIM_TypeDef * const lpTim_InstanceBase[ LPTIM_PERIPH_CNT ] =
{
#if defined(LPTIM1)
    LPTIM1, /**< LPTIM_PERIPH_1 base address */
#endif
#if defined(LPTIM2)
    LPTIM2, /**< LPTIM_PERIPH_2 base address */
#endif
#if defined(LPTIM3)
    LPTIM3, /**< LPTIM_PERIPH_3 base address */
#endif
#if defined(LPTIM4)
    LPTIM4, /**< LPTIM_PERIPH_4 base address */
#endif
#if defined(LPTIM5)
    LPTIM5, /**< LPTIM_PERIPH_5 base address */
#endif
#if defined(LPTIM6)
    LPTIM6, /**< LPTIM_PERIPH_6 base address */
#endif
};

_Static_assert( ( sizeof( lpTim_InstanceBase ) / sizeof( LPTIM_TypeDef * ) ) == LPTIM_PERIPH_CNT,
                 "LpTim_Reg: lpTim_InstanceBase has incorrect size." );


/** \brief Byte offset of every used LPTIM register inside LPTIM_TypeDef, indexed by lpTim_RegId_t */
static const uint32_t lpTim_RegOffset[ LPTIM_REG_CNT ] =
{
    offsetof( LPTIM_TypeDef, ISR   ), /**< LPTIM_REG_ISR   */
    offsetof( LPTIM_TypeDef, ICR   ), /**< LPTIM_REG_ICR   */
    offsetof( LPTIM_TypeDef, DIER  ), /**< LPTIM_REG_DIER  */
    offsetof( LPTIM_TypeDef, CFGR  ), /**< LPTIM_REG_CFGR  */
    offsetof( LPTIM_TypeDef, CR    ), /**< LPTIM_REG_CR    */
    offsetof( LPTIM_TypeDef, CCR1  ), /**< LPTIM_REG_CCR1  */
    offsetof( LPTIM_TypeDef, ARR   ), /**< LPTIM_REG_ARR   */
    offsetof( LPTIM_TypeDef, CNT   ), /**< LPTIM_REG_CNTR  */
    offsetof( LPTIM_TypeDef, CFGR2 ), /**< LPTIM_REG_CFGR2 */
    offsetof( LPTIM_TypeDef, RCR   ), /**< LPTIM_REG_RCR   */
    offsetof( LPTIM_TypeDef, CCMR1 ), /**< LPTIM_REG_CCMR1 */
    offsetof( LPTIM_TypeDef, CCR2  ), /**< LPTIM_REG_CCR2  */
};

_Static_assert( ( sizeof( lpTim_RegOffset ) / sizeof( uint32_t ) ) == LPTIM_REG_CNT,
                 "LpTim_Reg: lpTim_RegOffset has incorrect size." );

/* ========================== PRIVATE FUNCTIONS ============================== */

/**
 * \brief Builds the register address of an instance / register pair
 *
 * \param periphId [in] : Instance identification.
 * \param regId    [in] : Register identification.
 *
 * \return Address of the register, or LPTIM_NULL_PTR for an invalid parameter.
 */
static volatile uint32_t * LpTim_Reg_GetAddr( lpTim_PeriphId_t periphId, lpTim_RegId_t regId )
{
    volatile uint32_t *regAddr  = LPTIM_NULL_PTR;

    if( ( LPTIM_PERIPH_CNT > periphId ) &&
        ( LPTIM_REG_CNT    > regId    )    )
    {
        regAddr = (volatile uint32_t *)( (uint8_t *)lpTim_InstanceBase[periphId] + lpTim_RegOffset[regId] );
    }
    else
    {
        /* Invalid instance or register ID, no address is built */
        regAddr = LPTIM_NULL_PTR;
    }

    return ( regAddr );
}

/* ========================= EXPORTED FUNCTIONS =============================== */

/**
 * \brief Sets the register bit(s) with the given mask.
 *
 * \param periphId [in] : LPTIM instance ID
 * \param regId    [in] : LPTIM register ID
 * \param bitMask  [in] : Mask to apply to the register
 */
void LpTim_Set_RegBit( lpTim_PeriphId_t periphId, lpTim_RegId_t regId, uint32_t bitMask )
{
    volatile uint32_t *regAddr = LpTim_Reg_GetAddr( periphId, regId );

    if( LPTIM_NULL_PTR != regAddr )
    {
        *regAddr |= bitMask;
    }
    else
    {
        /* Invalid instance or register ID, no action required */
    }
}


/**
 * \brief Resets the register bit(s) with the given mask.
 *
 * \param periphId [in] : LPTIM instance ID
 * \param regId    [in] : LPTIM register ID
 * \param bitMask  [in] : Mask to apply to the register
 */
void LpTim_Reset_RegBit( lpTim_PeriphId_t periphId, lpTim_RegId_t regId, uint32_t bitMask )
{
    volatile uint32_t *regAddr = LpTim_Reg_GetAddr( periphId, regId );

    if( LPTIM_NULL_PTR != regAddr )
    {
        *regAddr &= ~bitMask;
    }
    else
    {
        /* Invalid instance or register ID, no action required */
    }
}


/**
 * \brief Gets the register bit(s) value with the given mask.
 *
 * \param periphId [in] : LPTIM instance ID
 * \param regId    [in] : LPTIM register ID
 * \param bitMask  [in] : Mask to apply to the register
 *
 * \return Register bit value with applied mask, LPTIM_REG_BITS_CLEAR for an invalid parameter.
 */
uint32_t LpTim_Get_RegBit( lpTim_PeriphId_t periphId, lpTim_RegId_t regId, uint32_t bitMask )
{
    volatile uint32_t *regAddr = LpTim_Reg_GetAddr( periphId, regId );
    uint32_t            retValue = LPTIM_REG_BITS_CLEAR;

    if( LPTIM_NULL_PTR != regAddr )
    {
        retValue = ( *regAddr & bitMask );
    }
    else
    {
        /* Invalid instance or register ID, no bit is set */
        retValue = LPTIM_REG_BITS_CLEAR;
    }

    return ( retValue );
}


/**
 * \brief Updates the register value with the given mask and value.
 *
 * \param periphId [in] : LPTIM instance ID
 * \param regId    [in] : LPTIM register ID
 * \param regMask  [in] : Mask to apply to the register
 * \param regValue [in] : Value to set in the register
 */
void LpTim_Set_RegVal( lpTim_PeriphId_t periphId, lpTim_RegId_t regId, uint32_t regMask, uint32_t regValue )
{
    volatile uint32_t *regAddr = LpTim_Reg_GetAddr( periphId, regId );

    if( LPTIM_NULL_PTR != regAddr )
    {
        *regAddr = ( ( *regAddr & ~regMask ) | ( regValue & regMask ) );
    }
    else
    {
        /* Invalid instance or register ID, no action required */
    }
}


/**
 * \brief Gets the register value with the given mask.
 *
 * \param periphId [in] : LPTIM instance ID
 * \param regId    [in] : LPTIM register ID
 * \param regMask  [in] : Mask to apply to the register
 *
 * \return Register value with applied mask, LPTIM_REG_BITS_CLEAR for an invalid parameter.
 */
uint32_t LpTim_Get_RegVal( lpTim_PeriphId_t periphId, lpTim_RegId_t regId, uint32_t regMask )
{
    volatile uint32_t *regAddr = LpTim_Reg_GetAddr( periphId, regId );
    uint32_t            retValue = LPTIM_REG_BITS_CLEAR;

    if( LPTIM_NULL_PTR != regAddr )
    {
        retValue = ( *regAddr & regMask );
    }
    else
    {
        /* Invalid instance or register ID, no bit is set */
        retValue = LPTIM_REG_BITS_CLEAR;
    }

    return ( retValue );
}


/**
 * \brief Waits (bounded) until every bit of the mask is set in the register.
 *
 * \param periphId [in] : LPTIM instance ID
 * \param regId    [in] : LPTIM register ID
 * \param bitMask  [in] : Mask of the bit(s) to wait for
 *
 * \return LPTIM_REQUEST_OK once every bit of the mask is set, LPTIM_REQUEST_ERROR on timeout.
 */
lpTim_RequestState_t LpTim_Reg_Wait_RegBitSet( lpTim_PeriphId_t periphId, lpTim_RegId_t regId, uint32_t bitMask )
{
    lpTim_RequestState_t retState = LPTIM_REQUEST_ERROR;
    uint32_t              bitValue = LPTIM_REG_BITS_CLEAR;

    for( uint32_t idx = 0u;
         ( LPTIM_REG_WAIT_TIMEOUT_RAW > idx ) &&
         ( bitMask                    != bitValue ) ;
         idx++ )
    {
        bitValue = LpTim_Get_RegBit( periphId, regId, bitMask );
    }

    if( bitMask == bitValue )
    {
        retState = LPTIM_REQUEST_OK;
    }
    else
    {
        retState = LPTIM_REQUEST_ERROR;
    }

    return ( retState );
}


/**
 * \brief Waits (bounded) until every bit of the mask is cleared in the register.
 *
 * \param periphId [in] : LPTIM instance ID
 * \param regId    [in] : LPTIM register ID
 * \param bitMask  [in] : Mask of the bit(s) to wait for
 *
 * \return LPTIM_REQUEST_OK once every bit of the mask is cleared, LPTIM_REQUEST_ERROR on timeout.
 */
lpTim_RequestState_t LpTim_Reg_Wait_RegBitClr( lpTim_PeriphId_t periphId, lpTim_RegId_t regId, uint32_t bitMask )
{
    lpTim_RequestState_t retState = LPTIM_REQUEST_ERROR;
    uint32_t              bitValue = bitMask;

    for( uint32_t idx = 0u;
         ( LPTIM_REG_WAIT_TIMEOUT_RAW > idx ) &&
         ( LPTIM_REG_BITS_CLEAR       != bitValue ) ;
         idx++ )
    {
        bitValue = LpTim_Get_RegBit( periphId, regId, bitMask );
    }

    if( LPTIM_REG_BITS_CLEAR == bitValue )
    {
        retState = LPTIM_REQUEST_OK;
    }
    else
    {
        retState = LPTIM_REQUEST_ERROR;
    }

    return ( retState );
}
