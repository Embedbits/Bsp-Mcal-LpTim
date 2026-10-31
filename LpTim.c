/**
 * \author Mr.Nobody
 * \file LpTim.c
 * \ingroup LpTim
 * \brief LpTim module implementation for STM32H5
 *
 * The data handling modes (POLL / ISR / DMA) share the same configuration
 * structure and callbacks as required by the project convention: an event is
 * polled by calling \ref LpTim_Get_Capture / \ref LpTim_Get_Counter / \ref
 * LpTim_Task without enabling it, interrupt-driven by \ref LpTim_Set_IrqActive
 * (the callback registered by \ref LpTim_Set_IrqCallback is then called from
 * the instance's interrupt) and DMA-driven by \ref LpTim_Set_DmaRequestActive
 * (the Gpdma module's own API configures the channel, source and destination
 * of the transfer). The module does not need a separate source file per mode:
 * unlike a byte stream (Usart, Spi, ...), the LPTIM has no data buffer of its
 * own to poll / push / pull, only events, so the three modes differ only in
 * which register bit is set, which this single file covers.
 *
 */

/* ============================= INCLUDES =================================== */
#include "LpTim.h"                          /* Module private header          */
#include "Rcc_Port.h"                       /* Rcc Port module definitions    */
/* ========================== PRIVATE DECLARATIONS =========================== */

static lpTim_RequestState_t LpTim_CalcPrescaler( lpTim_PeriphId_t periphId,
                                                 lpTim_Period_us_t period,
                                                 uint32_t * const prescReg,
                                                 lpTim_Counter_t * const arrValue );
static lpTim_RequestState_t LpTim_CalcArrFixedPrescaler( lpTim_PeriphId_t periphId,
                                                         lpTim_Period_us_t period,
                                                         lpTim_Counter_t * const arrValue );
static lpTim_RequestState_t LpTim_Reg_Set_ArrConfirmed( lpTim_PeriphId_t periphId, lpTim_Counter_t arrValue );
static lpTim_RequestState_t LpTim_Reg_Set_CcrConfirmed( lpTim_PeriphId_t periphId,
                                                        lpTim_ChannelId_t channelId,
                                                        lpTim_Counter_t ccrValue );
static lpTim_FunctionState_t LpTim_ClkSrc_IsWakeupCapable( lpTim_ClkSource_t clkSrc );
static uint32_t              LpTim_Event_DierBit( lpTim_EventId_t eventId );
static uint32_t              LpTim_Event_IsrBit( lpTim_EventId_t eventId );
static uint32_t              LpTim_Event_DmaBit( lpTim_EventId_t eventId );
static void                  LpTim_Isr_Common( lpTim_PeriphId_t periphId );

#if defined(LPTIM1)
static void LpTim_Isr_Lptim1( void );
#endif
#if defined(LPTIM2)
static void LpTim_Isr_Lptim2( void );
#endif
#if defined(LPTIM3)
static void LpTim_Isr_Lptim3( void );
#endif
#if defined(LPTIM4)
static void LpTim_Isr_Lptim4( void );
#endif
#if defined(LPTIM5)
static void LpTim_Isr_Lptim5( void );
#endif
#if defined(LPTIM6)
static void LpTim_Isr_Lptim6( void );
#endif

/* ========================== SYMBOLIC CONSTANTS ============================ */

/** \brief Module version */
#define LPTIM_MODULE_VERSION_MAJOR     ( 1u )
#define LPTIM_MODULE_VERSION_MINOR     ( 0u )
#define LPTIM_MODULE_VERSION_PATCH     ( 0u )

/** \brief Count of selectable prescaler steps (CFGR.PRESC = 0 .. 7, divider 1 .. 128) */
#define LPTIM_PRESCALER_STEP_CNT       ( 8u )

/** \brief Largest value the 16-bit counter / auto-reload register can hold */
#define LPTIM_COUNTER_MAX              ( 0xFFFFu )

/** \brief Mask selecting every bit of a 32-bit register (unrestricted read) */
#define LPTIM_REG_MASK_ALL             ( 0xFFFFFFFFu )

/* =============================== MACROS =================================== */

/* ========================== EXPORTED VARIABLES ============================ */

/* =========================== LOCAL VARIABLES =============================== */

/** \brief Run-time state of every available LPTIM instance, indexed by lpTim_PeriphId_t */
static lpTim_State_t lpTim_State[ LPTIM_PERIPH_CNT ];

/** \brief Rcc peripheral identification matching every lpTim_ClkSource_t item, same order and guards */
static const rcc_PeriphId_t lpTim_ClkSrcToRcc[ LPTIM_CLK_SRC_CNT ] =
{
#if defined(LPTIM1)
    RCC_PERIPH_LPTIM1_PCLK3, /**< LPTIM_CLK_SRC_LPTIM1_PCLK3 */
    RCC_PERIPH_LPTIM1_PLL2P, /**< LPTIM_CLK_SRC_LPTIM1_PLL2P */
#if defined(RCC_CR_PLL3ON)
    RCC_PERIPH_LPTIM1_PLL3R, /**< LPTIM_CLK_SRC_LPTIM1_PLL3R */
#endif
    RCC_PERIPH_LPTIM1_LSE  , /**< LPTIM_CLK_SRC_LPTIM1_LSE   */
    RCC_PERIPH_LPTIM1_LSI  , /**< LPTIM_CLK_SRC_LPTIM1_LSI   */
    RCC_PERIPH_LPTIM1_LPCLK, /**< LPTIM_CLK_SRC_LPTIM1_LPCLK */
#endif
#if defined(LPTIM2)
    RCC_PERIPH_LPTIM2_PCLK1, /**< LPTIM_CLK_SRC_LPTIM2_PCLK1 */
    RCC_PERIPH_LPTIM2_PLL2P, /**< LPTIM_CLK_SRC_LPTIM2_PLL2P */
#if defined(RCC_CR_PLL3ON)
    RCC_PERIPH_LPTIM2_PLL3R, /**< LPTIM_CLK_SRC_LPTIM2_PLL3R */
#endif
    RCC_PERIPH_LPTIM2_LSE  , /**< LPTIM_CLK_SRC_LPTIM2_LSE   */
    RCC_PERIPH_LPTIM2_LSI  , /**< LPTIM_CLK_SRC_LPTIM2_LSI   */
    RCC_PERIPH_LPTIM2_LPCLK, /**< LPTIM_CLK_SRC_LPTIM2_LPCLK */
#endif
#if defined(LPTIM3)
    RCC_PERIPH_LPTIM3_PCLK3, /**< LPTIM_CLK_SRC_LPTIM3_PCLK3 */
    RCC_PERIPH_LPTIM3_PLL2P, /**< LPTIM_CLK_SRC_LPTIM3_PLL2P */
    RCC_PERIPH_LPTIM3_PLL3R, /**< LPTIM_CLK_SRC_LPTIM3_PLL3R */
    RCC_PERIPH_LPTIM3_LSE  , /**< LPTIM_CLK_SRC_LPTIM3_LSE   */
    RCC_PERIPH_LPTIM3_LSI  , /**< LPTIM_CLK_SRC_LPTIM3_LSI   */
    RCC_PERIPH_LPTIM3_LPCLK, /**< LPTIM_CLK_SRC_LPTIM3_LPCLK */
#endif
#if defined(LPTIM4)
    RCC_PERIPH_LPTIM4_PCLK3, /**< LPTIM_CLK_SRC_LPTIM4_PCLK3 */
    RCC_PERIPH_LPTIM4_PLL2P, /**< LPTIM_CLK_SRC_LPTIM4_PLL2P */
    RCC_PERIPH_LPTIM4_PLL3R, /**< LPTIM_CLK_SRC_LPTIM4_PLL3R */
    RCC_PERIPH_LPTIM4_LSE  , /**< LPTIM_CLK_SRC_LPTIM4_LSE   */
    RCC_PERIPH_LPTIM4_LSI  , /**< LPTIM_CLK_SRC_LPTIM4_LSI   */
    RCC_PERIPH_LPTIM4_LPCLK, /**< LPTIM_CLK_SRC_LPTIM4_LPCLK */
#endif
#if defined(LPTIM5)
    RCC_PERIPH_LPTIM5_PCLK3, /**< LPTIM_CLK_SRC_LPTIM5_PCLK3 */
    RCC_PERIPH_LPTIM5_PLL2P, /**< LPTIM_CLK_SRC_LPTIM5_PLL2P */
    RCC_PERIPH_LPTIM5_PLL3R, /**< LPTIM_CLK_SRC_LPTIM5_PLL3R */
    RCC_PERIPH_LPTIM5_LSE  , /**< LPTIM_CLK_SRC_LPTIM5_LSE   */
    RCC_PERIPH_LPTIM5_LSI  , /**< LPTIM_CLK_SRC_LPTIM5_LSI   */
    RCC_PERIPH_LPTIM5_LPCLK, /**< LPTIM_CLK_SRC_LPTIM5_LPCLK */
#endif
#if defined(LPTIM6)
    RCC_PERIPH_LPTIM6_PCLK3, /**< LPTIM_CLK_SRC_LPTIM6_PCLK3 */
    RCC_PERIPH_LPTIM6_PLL2P, /**< LPTIM_CLK_SRC_LPTIM6_PLL2P */
    RCC_PERIPH_LPTIM6_PLL3R, /**< LPTIM_CLK_SRC_LPTIM6_PLL3R */
    RCC_PERIPH_LPTIM6_LSE  , /**< LPTIM_CLK_SRC_LPTIM6_LSE   */
    RCC_PERIPH_LPTIM6_LSI  , /**< LPTIM_CLK_SRC_LPTIM6_LSI   */
    RCC_PERIPH_LPTIM6_LPCLK, /**< LPTIM_CLK_SRC_LPTIM6_LPCLK */
#endif
};

_Static_assert( ( sizeof( lpTim_ClkSrcToRcc ) / sizeof( rcc_PeriphId_t ) ) == LPTIM_CLK_SRC_CNT,
                 "LpTim: lpTim_ClkSrcToRcc has incorrect size." );


/** \brief NVIC interrupt identification matching every lpTim_PeriphId_t item, same order and guards */
static const nvic_PeriphIrqList_t lpTim_PeriphIrq[ LPTIM_PERIPH_CNT ] =
{
#if defined(LPTIM1)
    NVIC_PERIPH_IRQ_LPTIM1, /**< LPTIM_PERIPH_1 interrupt */
#endif
#if defined(LPTIM2)
    NVIC_PERIPH_IRQ_LPTIM2, /**< LPTIM_PERIPH_2 interrupt */
#endif
#if defined(LPTIM3)
    NVIC_PERIPH_IRQ_LPTIM3, /**< LPTIM_PERIPH_3 interrupt */
#endif
#if defined(LPTIM4)
    NVIC_PERIPH_IRQ_LPTIM4, /**< LPTIM_PERIPH_4 interrupt */
#endif
#if defined(LPTIM5)
    NVIC_PERIPH_IRQ_LPTIM5, /**< LPTIM_PERIPH_5 interrupt */
#endif
#if defined(LPTIM6)
    NVIC_PERIPH_IRQ_LPTIM6, /**< LPTIM_PERIPH_6 interrupt */
#endif
};

_Static_assert( ( sizeof( lpTim_PeriphIrq ) / sizeof( nvic_PeriphIrqList_t ) ) == LPTIM_PERIPH_CNT,
                 "LpTim: lpTim_PeriphIrq has incorrect size." );


/** \brief Interrupt handler of every available LPTIM instance, indexed by lpTim_PeriphId_t */
static const nvic_IsrCallback_t lpTim_IsrHandler[ LPTIM_PERIPH_CNT ] =
{
#if defined(LPTIM1)
    LpTim_Isr_Lptim1, /**< LPTIM_PERIPH_1 interrupt handler */
#endif
#if defined(LPTIM2)
    LpTim_Isr_Lptim2, /**< LPTIM_PERIPH_2 interrupt handler */
#endif
#if defined(LPTIM3)
    LpTim_Isr_Lptim3, /**< LPTIM_PERIPH_3 interrupt handler */
#endif
#if defined(LPTIM4)
    LpTim_Isr_Lptim4, /**< LPTIM_PERIPH_4 interrupt handler */
#endif
#if defined(LPTIM5)
    LpTim_Isr_Lptim5, /**< LPTIM_PERIPH_5 interrupt handler */
#endif
#if defined(LPTIM6)
    LpTim_Isr_Lptim6, /**< LPTIM_PERIPH_6 interrupt handler */
#endif
};

_Static_assert( ( sizeof( lpTim_IsrHandler ) / sizeof( nvic_IsrCallback_t ) ) == LPTIM_PERIPH_CNT,
                 "LpTim: lpTim_IsrHandler has incorrect size." );

/* ========================== PRIVATE FUNCTIONS ============================== */

/**
 * \brief Computes the smallest prescaler and the resulting ARR value for a requested period
 *
 * \param periphId  [in]  : Instance (its cached kernel clock frequency is used).
 * \param period    [in]  : Requested period in us.
 * \param prescReg  [out] : CFGR.PRESC field value (0 .. 7) achieving the smallest resolution step.
 * \param arrValue  [out] : Auto-reload value (ARR = ticks - 1) at the chosen prescaler.
 *
 * \return LPTIM_REQUEST_OK if a prescaler/ARR pair was found, LPTIM_REQUEST_ERROR if the
 *         requested period does not fit in the 16-bit counter at any prescaler.
 */
static lpTim_RequestState_t LpTim_CalcPrescaler( lpTim_PeriphId_t periphId,
                                                 lpTim_Period_us_t period,
                                                 uint32_t * const prescReg,
                                                 lpTim_Counter_t * const arrValue )
{
    lpTim_RequestState_t retState  = LPTIM_REQUEST_ERROR;
    uint64_t               ticks     = 0u;
    uint32_t                presc     = 0u;
    uint8_t                 found     = 0u;

    for( presc = 0u;
         ( LPTIM_PRESCALER_STEP_CNT > presc ) &&
         ( 0u == found )                        ;
         presc++ )
    {
        ticks = ( (uint64_t)period * lpTim_State[periphId].ClkFreqHz ) /
                ( (uint64_t)LPTIM_US_PER_SECOND * ( (uint64_t)1u << presc ) );

        if( ( LPTIM_COUNTER_MAX >= ticks ) &&
            ( 0u < ticks )                    )
        {
            found = 1u;
        }
        else
        {
            /* This prescaler does not fit, the next (coarser) one is tried */
        }
    }

    if( 0u != found )
    {
        *prescReg = ( presc - 1u );
        *arrValue = (lpTim_Counter_t)( ticks - 1u );
        retState  = LPTIM_REQUEST_OK;
    }
    else
    {
        *prescReg = 0u;
        *arrValue = 0u;
        retState  = LPTIM_REQUEST_ERROR;
    }

    return ( retState );
}


/**
 * \brief Computes the ARR value of a requested period at the prescaler the instance already runs with
 *
 * The prescaler (CFGR.PRESC) is fixed by \ref LpTim_Init, so the auto-reload value has to be
 * computed for that prescaler, not for the smallest one that would fit the new period.
 *
 * \param periphId [in]  : Instance (its cached kernel clock frequency and CFGR.PRESC are used).
 * \param period   [in]  : Requested period in us.
 * \param arrValue [out] : Auto-reload value (ARR = ticks - 1) at the current prescaler.
 *
 * \return LPTIM_REQUEST_OK if the period fits the 16-bit counter, LPTIM_REQUEST_ERROR otherwise
 *         (too short for one tick or too long for the counter).
 */
static lpTim_RequestState_t LpTim_CalcArrFixedPrescaler( lpTim_PeriphId_t periphId,
                                                         lpTim_Period_us_t period,
                                                         lpTim_Counter_t * const arrValue )
{
    lpTim_RequestState_t retState = LPTIM_REQUEST_ERROR;
    uint32_t               prescReg = 0u;
    uint64_t               ticks    = 0u;

    prescReg = LpTim_Get_RegVal( periphId, LPTIM_REG_CFGR, LPTIM_CFGR_PRESC ) >> LPTIM_CFGR_PRESC_Pos;

    ticks = ( (uint64_t)period * lpTim_State[periphId].ClkFreqHz ) /
            ( (uint64_t)LPTIM_US_PER_SECOND * ( (uint64_t)1u << prescReg ) );

    if( ( LPTIM_COUNTER_MAX >= ticks ) &&
        ( 0u < ticks )                    )
    {
        *arrValue = (lpTim_Counter_t)( ticks - 1u );
        retState  = LPTIM_REQUEST_OK;
    }
    else
    {
        *arrValue = 0u;
        retState  = LPTIM_REQUEST_ERROR;
    }

    return ( retState );
}


/**
 * \brief Writes the ARR register and waits for the hardware confirmation of the write
 *
 * \param periphId [in] : Instance to configure.
 * \param arrValue [in] : Requested auto-reload value.
 *
 * \return LPTIM_REQUEST_OK once the write is confirmed, LPTIM_REQUEST_ERROR on timeout.
 */
static lpTim_RequestState_t LpTim_Reg_Set_ArrConfirmed( lpTim_PeriphId_t periphId, lpTim_Counter_t arrValue )
{
    lpTim_RequestState_t retState = LPTIM_REQUEST_ERROR;

    LpTim_Reset_RegBit( periphId, LPTIM_REG_ICR, LPTIM_ISR_ARROK );
    LpTim_Set_RegVal( periphId, LPTIM_REG_ARR, LPTIM_COUNTER_MAX, (uint32_t)arrValue );
    retState = LpTim_Reg_Wait_RegBitSet( periphId, LPTIM_REG_ISR, LPTIM_ISR_ARROK );
    LpTim_Set_RegBit( periphId, LPTIM_REG_ICR, LPTIM_ISR_ARROK );

    return ( retState );
}


/**
 * \brief Writes a CCRx register and waits for the hardware confirmation of the write
 *
 * \param periphId  [in] : Instance to configure.
 * \param channelId [in] : Channel whose compare register is written.
 * \param ccrValue  [in] : Requested compare value.
 *
 * \return LPTIM_REQUEST_OK once the write is confirmed, LPTIM_REQUEST_ERROR on timeout
 *         or for an invalid channel.
 */
static lpTim_RequestState_t LpTim_Reg_Set_CcrConfirmed( lpTim_PeriphId_t periphId,
                                                        lpTim_ChannelId_t channelId,
                                                        lpTim_Counter_t ccrValue )
{
    lpTim_RequestState_t retState = LPTIM_REQUEST_ERROR;
    lpTim_RegId_t          ccrReg   = LPTIM_REG_CNT;
    uint32_t                okFlag   = 0u;

    if( LPTIM_CHANNEL_1 == channelId )
    {
        ccrReg = LPTIM_REG_CCR1;
        okFlag = LPTIM_ISR_CMP1OK;
    }
    else if( LPTIM_CHANNEL_2 == channelId )
    {
        ccrReg = LPTIM_REG_CCR2;
        okFlag = LPTIM_ISR_CMP2OK;
    }
    else
    {
        /* Invalid channel, nothing is written */
    }

    if( LPTIM_REG_CNT != ccrReg )
    {
        LpTim_Reset_RegBit( periphId, LPTIM_REG_ICR, okFlag );
        LpTim_Set_RegVal( periphId, ccrReg, LPTIM_COUNTER_MAX, (uint32_t)ccrValue );
        retState = LpTim_Reg_Wait_RegBitSet( periphId, LPTIM_REG_ISR, okFlag );
        LpTim_Set_RegBit( periphId, LPTIM_REG_ICR, okFlag );
    }
    else
    {
        retState = LPTIM_REQUEST_ERROR;
    }

    return ( retState );
}


/**
 * \brief Tells whether a clock source keeps the instance running (and able to wake up) in Stop mode
 *
 * \param clkSrc [in] : Clock source to check.
 *
 * \return LPTIM_FUNCTION_ACTIVE for LSE / LSI / the low-power clock domain, LPTIM_FUNCTION_INACTIVE
 *         for a bus clock or a PLL output (both stop with the system clock in Stop mode).
 */
static lpTim_FunctionState_t LpTim_ClkSrc_IsWakeupCapable( lpTim_ClkSource_t clkSrc )
{
    lpTim_FunctionState_t retState = LPTIM_FUNCTION_INACTIVE;
    rcc_PeriphId_t          rccId    = RCC_PERIPH_ID_CNT;

    if( LPTIM_CLK_SRC_CNT > clkSrc )
    {
        rccId = lpTim_ClkSrcToRcc[clkSrc];
    }
    else
    {
        /* Invalid clock source, treated as not wake-up capable */
    }

    if( ( RCC_PERIPH_LPTIM1_LSE   == rccId ) ||
        ( RCC_PERIPH_LPTIM1_LSI   == rccId ) ||
        ( RCC_PERIPH_LPTIM1_LPCLK == rccId )    )
    {
        retState = LPTIM_FUNCTION_ACTIVE;
    }
    else
    {
        /* The source does not keep the kernel clock running in Stop mode */
        retState = LPTIM_FUNCTION_INACTIVE;
    }

    return ( retState );
}


/**
 * \brief DIER bit mask of an event (interrupt enable)
 */
static uint32_t LpTim_Event_DierBit( lpTim_EventId_t eventId )
{
    static const uint32_t dierBit[ LPTIM_EVENT_CNT ] =
    {
        LPTIM_DIER_UEIE   , /**< LPTIM_EVENT_UPDATE          */
        LPTIM_DIER_CC1IE  , /**< LPTIM_EVENT_COMPARE_1       */
        LPTIM_DIER_CC2IE  , /**< LPTIM_EVENT_COMPARE_2       */
        LPTIM_DIER_CC1IE  , /**< LPTIM_EVENT_CAPTURE_1       */
        LPTIM_DIER_CC2IE  , /**< LPTIM_EVENT_CAPTURE_2       */
        LPTIM_DIER_EXTTRIGIE, /**< LPTIM_EVENT_TRIGGER        */
        LPTIM_DIER_UPIE   , /**< LPTIM_EVENT_DIRECTION_UP    */
        LPTIM_DIER_DOWNIE , /**< LPTIM_EVENT_DIRECTION_DOWN  */
        LPTIM_DIER_CC1OIE , /**< LPTIM_EVENT_OVERCAPTURE_1   */
        LPTIM_DIER_CC2OIE   /**< LPTIM_EVENT_OVERCAPTURE_2   */
    };
    uint32_t retValue = 0u;

    if( LPTIM_EVENT_CNT > eventId )
    {
        retValue = dierBit[eventId];
    }
    else
    {
        retValue = 0u;
    }

    return ( retValue );
}


/**
 * \brief ISR status bit mask of an event (flag and clear bit, same position in ISR and ICR)
 */
static uint32_t LpTim_Event_IsrBit( lpTim_EventId_t eventId )
{
    static const uint32_t isrBit[ LPTIM_EVENT_CNT ] =
    {
        LPTIM_ISR_UE    , /**< LPTIM_EVENT_UPDATE          */
        LPTIM_ISR_CC1IF , /**< LPTIM_EVENT_COMPARE_1       */
        LPTIM_ISR_CC2IF , /**< LPTIM_EVENT_COMPARE_2       */
        LPTIM_ISR_CC1IF , /**< LPTIM_EVENT_CAPTURE_1       */
        LPTIM_ISR_CC2IF , /**< LPTIM_EVENT_CAPTURE_2       */
        LPTIM_ISR_EXTTRIG, /**< LPTIM_EVENT_TRIGGER        */
        LPTIM_ISR_UP    , /**< LPTIM_EVENT_DIRECTION_UP    */
        LPTIM_ISR_DOWN  , /**< LPTIM_EVENT_DIRECTION_DOWN  */
        LPTIM_ISR_CC1OF , /**< LPTIM_EVENT_OVERCAPTURE_1   */
        LPTIM_ISR_CC2OF   /**< LPTIM_EVENT_OVERCAPTURE_2   */
    };
    uint32_t retValue = 0u;

    if( LPTIM_EVENT_CNT > eventId )
    {
        retValue = isrBit[eventId];
    }
    else
    {
        retValue = 0u;
    }

    return ( retValue );
}


/**
 * \brief DIER DMA-request bit mask of an event, 0u for an event without a DMA request
 */
static uint32_t LpTim_Event_DmaBit( lpTim_EventId_t eventId )
{
    uint32_t retValue = 0u;

    if( LPTIM_EVENT_UPDATE == eventId )
    {
        retValue = LPTIM_DIER_UEDE;
    }
    else if( LPTIM_EVENT_CAPTURE_1 == eventId )
    {
        retValue = LPTIM_DIER_CC1DE;
    }
    else if( LPTIM_EVENT_CAPTURE_2 == eventId )
    {
        retValue = LPTIM_DIER_CC2DE;
    }
    else
    {
        /* The event has no DMA request */
        retValue = 0u;
    }

    return ( retValue );
}


/**
 * \brief Common interrupt dispatch of an instance: calls the callback of every pending and
 *        enabled event, then clears its flag.
 *
 * \param periphId [in] : Instance whose interrupt is serviced.
 */
static void LpTim_Isr_Common( lpTim_PeriphId_t periphId )
{
    uint32_t statusReg = LpTim_Get_RegVal( periphId, LPTIM_REG_ISR, LPTIM_REG_MASK_ALL );
    uint32_t enableReg = LpTim_Get_RegVal( periphId, LPTIM_REG_DIER, LPTIM_REG_MASK_ALL );

    for( uint32_t eventId = 0u; LPTIM_EVENT_CNT > eventId; eventId++ )
    {
        uint32_t isrBit  = LpTim_Event_IsrBit( (lpTim_EventId_t)eventId );
        uint32_t dierBit = LpTim_Event_DierBit( (lpTim_EventId_t)eventId );

        if( ( 0u != ( statusReg & isrBit  ) ) &&
            ( 0u != ( enableReg & dierBit ) )    )
        {
            if( LPTIM_NULL_PTR != lpTim_State[periphId].EventCbk[eventId] )
            {
                lpTim_State[periphId].EventCbk[eventId]();
            }
            else
            {
                /* No callback registered for this event */
            }

            LpTim_Set_RegBit( periphId, LPTIM_REG_ICR, isrBit );
        }
        else
        {
            /* Event not pending or not enabled, no action required */
        }
    }
}


#if defined(LPTIM1)
static void LpTim_Isr_Lptim1( void )
{
    LpTim_Isr_Common( LPTIM_PERIPH_1 );
}
#endif
#if defined(LPTIM2)
static void LpTim_Isr_Lptim2( void )
{
    LpTim_Isr_Common( LPTIM_PERIPH_2 );
}
#endif
#if defined(LPTIM3)
static void LpTim_Isr_Lptim3( void )
{
    LpTim_Isr_Common( LPTIM_PERIPH_3 );
}
#endif
#if defined(LPTIM4)
static void LpTim_Isr_Lptim4( void )
{
    LpTim_Isr_Common( LPTIM_PERIPH_4 );
}
#endif
#if defined(LPTIM5)
static void LpTim_Isr_Lptim5( void )
{
    LpTim_Isr_Common( LPTIM_PERIPH_5 );
}
#endif
#if defined(LPTIM6)
static void LpTim_Isr_Lptim6( void )
{
    LpTim_Isr_Common( LPTIM_PERIPH_6 );
}
#endif

/* ========================= EXPORTED FUNCTIONS =============================== */

/* --------------------------- Module management ----------------------------- */

/**
 * \brief Reads the version of the module
 *
 * \return Version of the module (major, minor, patch).
 */
lpTim_ModuleVersion_t LpTim_Get_ModuleVersion( void )
{
    lpTim_ModuleVersion_t moduleVersion;

    moduleVersion.Major = LPTIM_MODULE_VERSION_MAJOR;
    moduleVersion.Minor = LPTIM_MODULE_VERSION_MINOR;
    moduleVersion.Patch = LPTIM_MODULE_VERSION_PATCH;

    return ( moduleVersion );
}


/**
 * \brief Configures an LPTIM instance and leaves it stopped
 *
 * \note The kernel clock prescaler chosen for the requested period is kept fixed for the
 *       lifetime of the instance: \ref LpTim_Set_Period can only move the auto-reload value
 *       within the resolution range it defines (changing the prescaler requires disabling
 *       the instance, which the hardware also resets, so it is only done here).
 *
 * \param lpTimConfig [in] : Configuration of the instance.
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
lpTim_RequestState_t LpTim_Init( const lpTim_Config_t * const lpTimConfig )
{
    lpTim_RequestState_t retState  = LPTIM_REQUEST_ERROR;
    lpTim_PeriphId_t       periphId  = LPTIM_PERIPH_CNT;
    rcc_PeriphId_t          rccId     = RCC_PERIPH_ID_CNT;
    rcc_FreqHz_t            clkFreqHz = 0u;
    rcc_RequestState_t      rccState  = RCC_REQUEST_ERROR;
    uint32_t                 prescReg  = 0u;
    lpTim_Counter_t         arrValue  = 0u;
    uint32_t                 cfgrValue = 0u;
    uint32_t                 enableBit = 0u;

    if( ( LPTIM_NULL_PTR    != lpTimConfig                    ) &&
        ( LPTIM_PERIPH_CNT  > lpTimConfig->PeriphId            ) &&
        ( LPTIM_CLK_SRC_CNT > lpTimConfig->ClkSrc              ) &&
        ( LPTIM_MODE_CNT    > lpTimConfig->Mode                ) &&
        ( LPTIM_START_CNT   > lpTimConfig->StartMode           ) &&
        ( LPTIM_POLARITY_CNT > lpTimConfig->Trigger.Polarity   ) &&
        ( LPTIM_FILTER_CNT  > lpTimConfig->Trigger.Filter      ) &&
        ( LPTIM_FUNCTION_ACTIVE != lpTim_State[lpTimConfig->PeriphId].Used )    )
    {
        periphId = lpTimConfig->PeriphId;
        rccId    = lpTim_ClkSrcToRcc[lpTimConfig->ClkSrc];
        rccState = Rcc_Set_PeriphActive( rccId );
    }
    else
    {
        rccState = RCC_REQUEST_ERROR;
    }

    if( RCC_REQUEST_OK == rccState )
    {
        rccState = Rcc_Get_PeriphClk( rccId, &clkFreqHz );
    }
    else
    {
        /* Instance already used or an invalid parameter, nothing else is done */
    }

    if( RCC_REQUEST_OK == rccState )
    {
        lpTim_State[periphId].ClkFreqHz = (lpTim_Frequency_Hz_t)clkFreqHz;
        retState = LpTim_CalcPrescaler( periphId, lpTimConfig->Period, &prescReg, &arrValue );
    }
    else
    {
        retState = LPTIM_REQUEST_ERROR;
    }

    if( LPTIM_REQUEST_OK == retState )
    {
        LpTim_Reset_RegBit( periphId, LPTIM_REG_CR, LPTIM_CR_ENABLE );

        cfgrValue = ( prescReg << LPTIM_CFGR_PRESC_Pos ) | LPTIM_CFGR_PRELOAD;

        if( LPTIM_MODE_PULSE_COUNTER == lpTimConfig->Mode )
        {
            cfgrValue |= LPTIM_CFGR_CKSEL;
        }
        else
        {
            /* Internal kernel clock, CKSEL stays 0 */
        }

        if( LPTIM_MODE_ENCODER == lpTimConfig->Mode )
        {
            cfgrValue |= LPTIM_CFGR_ENC;
        }
        else
        {
            /* Not the encoder mode, ENC stays 0 */
        }

        if( LPTIM_FUNCTION_ACTIVE == lpTimConfig->Trigger.TriggerActive )
        {
            cfgrValue |= LPTIM_CFGR_TRGFLT & ( (uint32_t)lpTimConfig->Trigger.Filter << LPTIM_CFGR_TRGFLT_Pos );

            if( LPTIM_POLARITY_HIGH == lpTimConfig->Trigger.Polarity )
            {
                cfgrValue |= LPTIM_CFGR_TRIGEN_0;
            }
            else
            {
                cfgrValue |= LPTIM_CFGR_TRIGEN_1;
            }
        }
        else
        {
            /* External trigger disabled, TRIGEN stays 0 */
        }

        LpTim_Set_RegVal( periphId, LPTIM_REG_CFGR, LPTIM_REG_MASK_ALL, cfgrValue );
        LpTim_Set_RegBit( periphId, LPTIM_REG_CR, LPTIM_CR_ENABLE );
        retState = LpTim_Reg_Set_ArrConfirmed( periphId, arrValue );
    }
    else
    {
        /* Prescaler / ARR do not fit the requested period, nothing is configured */
    }

    if( LPTIM_REQUEST_OK == retState )
    {
        enableBit = LpTim_Get_RegBit( periphId, LPTIM_REG_CR, LPTIM_CR_ENABLE );

        if( LPTIM_CR_ENABLE == enableBit )
        {
            ( void )Nvic_Set_PeriphIrq_Handler( lpTim_PeriphIrq[periphId], lpTim_IsrHandler[periphId] );
            ( void )Nvic_Set_PeriphIrq_Prio( lpTim_PeriphIrq[periphId], lpTimConfig->IrqPrio );
            ( void )Nvic_Set_PeriphIrq_Active( lpTim_PeriphIrq[periphId] );

            lpTim_State[periphId].Used      = LPTIM_FUNCTION_ACTIVE;
            lpTim_State[periphId].Running   = LPTIM_FUNCTION_INACTIVE;
            lpTim_State[periphId].Mode      = lpTimConfig->Mode;
            lpTim_State[periphId].StartMode = lpTimConfig->StartMode;
            lpTim_State[periphId].ClkSrc    = lpTimConfig->ClkSrc;
            lpTim_State[periphId].WakeupCapable = LpTim_ClkSrc_IsWakeupCapable( lpTimConfig->ClkSrc );
            lpTim_State[periphId].Direction = LPTIM_DIRECTION_UP;
            lpTim_State[periphId].EventCbk[LPTIM_EVENT_UPDATE] = lpTimConfig->Callback;

            for( uint32_t channelId = 0u; LPTIM_CHANNEL_CNT > channelId; channelId++ )
            {
                lpTim_State[periphId].ChannelMode[channelId] = LPTIM_CHANNEL_MODE_CNT;
            }

            retState = LPTIM_REQUEST_OK;
        }
        else
        {
            retState = LPTIM_REQUEST_ERROR;
        }
    }
    else
    {
        /* The write confirmation timed out, nothing else is done */
    }

    if( ( LPTIM_REQUEST_OK   != retState  ) &&
        ( LPTIM_PERIPH_CNT   > periphId   )    )
    {
        ( void )LpTim_Deinit( periphId );
    }
    else
    {
        /* Initialization succeeded, no clean-up required */
    }

    return ( retState );
}


/**
 * \brief Stops and deinitializes an LPTIM instance
 *
 * \param periphId [in] : Instance to deinitialize.
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
lpTim_RequestState_t LpTim_Deinit( lpTim_PeriphId_t periphId )
{
    lpTim_RequestState_t retState = LPTIM_REQUEST_ERROR;

    if( LPTIM_PERIPH_CNT > periphId )
    {
        LpTim_Set_RegVal( periphId, LPTIM_REG_DIER, LPTIM_REG_MASK_ALL, 0u );
        LpTim_Reset_RegBit( periphId, LPTIM_REG_CR, LPTIM_CR_ENABLE );
        ( void )Nvic_Set_PeriphIrq_Inactive( lpTim_PeriphIrq[periphId] );
        ( void )Rcc_Set_PeriphInactive( lpTim_ClkSrcToRcc[lpTim_State[periphId].ClkSrc] );

        lpTim_State[periphId].Used    = LPTIM_FUNCTION_INACTIVE;
        lpTim_State[periphId].Running = LPTIM_FUNCTION_INACTIVE;

        for( uint32_t eventId = 0u; LPTIM_EVENT_CNT > eventId; eventId++ )
        {
            lpTim_State[periphId].EventCbk[eventId] = LPTIM_NULL_PTR;
        }

        retState = LPTIM_REQUEST_OK;
    }
    else
    {
        retState = LPTIM_REQUEST_ERROR;
    }

    return ( retState );
}


/**
 * \brief Periodic task of the module
 *
 * Updates the cached direction of every initialized instance from the sticky
 * LPTIM_ISR_UP / LPTIM_ISR_DOWN flags, so \ref LpTim_Get_EncoderDirection reflects
 * the hardware even in the POLL data handling mode (no interrupt enabled).
 */
void LpTim_Task( void )
{
    for( uint32_t periphIdx = 0u; LPTIM_PERIPH_CNT > periphIdx; periphIdx++ )
    {
        lpTim_PeriphId_t periphId = (lpTim_PeriphId_t)periphIdx;

        if( LPTIM_FUNCTION_ACTIVE == lpTim_State[periphId].Used )
        {
            uint32_t statusReg = LpTim_Get_RegVal( periphId, LPTIM_REG_ISR, ( LPTIM_ISR_UP | LPTIM_ISR_DOWN ) );

            if( 0u != ( statusReg & LPTIM_ISR_UP ) )
            {
                lpTim_State[periphId].Direction = LPTIM_DIRECTION_UP;
                LpTim_Set_RegBit( periphId, LPTIM_REG_ICR, LPTIM_ISR_UP );
            }
            else
            {
                /* No new "up" direction event */
            }

            if( 0u != ( statusReg & LPTIM_ISR_DOWN ) )
            {
                lpTim_State[periphId].Direction = LPTIM_DIRECTION_DOWN;
                LpTim_Set_RegBit( periphId, LPTIM_REG_ICR, LPTIM_ISR_DOWN );
            }
            else
            {
                /* No new "down" direction event */
            }
        }
        else
        {
            /* Instance not initialized, nothing to update */
        }
    }
}


/**
 * \brief Reads the default configuration of an instance
 *
 * \param lpTimConfig [out] : Default configuration (time base mode, the first available clock
 *        source, no trigger, continuous start mode, no callback, the lowest interrupt priority).
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
lpTim_RequestState_t LpTim_Get_DefaultConfig( lpTim_Config_t * const lpTimConfig )
{
    lpTim_RequestState_t retState = LPTIM_REQUEST_ERROR;

    if( LPTIM_NULL_PTR != lpTimConfig )
    {
        lpTimConfig->PeriphId             = LPTIM_PERIPH_1;
        lpTimConfig->ClkSrc                = (lpTim_ClkSource_t)0u;
        lpTimConfig->Mode                  = LPTIM_MODE_TIMEBASE;
        lpTimConfig->Period                = 0u;
        lpTimConfig->StartMode             = LPTIM_START_CONTINUOUS;
        lpTimConfig->Trigger.TriggerActive = LPTIM_FUNCTION_INACTIVE;
        lpTimConfig->Trigger.Polarity      = LPTIM_POLARITY_HIGH;
        lpTimConfig->Trigger.Filter        = LPTIM_FILTER_NONE;
        lpTimConfig->Callback              = LPTIM_NULL_PTR;
        lpTimConfig->IrqPrio               = (lpTim_IrqPrio_t)0u;

        retState = LPTIM_REQUEST_OK;
    }
    else
    {
        retState = LPTIM_REQUEST_ERROR;
    }

    return ( retState );
}

/* ------------------------------ Run control --------------------------------- */

/**
 * \brief Starts a configured and stopped LPTIM instance
 *
 * \param periphId  [in] : Instance to start.
 * \param startMode [in] : Continuous or single (one-shot) counting.
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
lpTim_RequestState_t LpTim_Set_Start( lpTim_PeriphId_t periphId, lpTim_StartMode_t startMode )
{
    lpTim_RequestState_t retState = LPTIM_REQUEST_ERROR;

    if( ( LPTIM_PERIPH_CNT > periphId                                 ) &&
        ( LPTIM_START_CNT  > startMode                                ) &&
        ( LPTIM_FUNCTION_ACTIVE == lpTim_State[periphId].Used          ) &&
        ( LPTIM_FUNCTION_ACTIVE != lpTim_State[periphId].Running        )    )
    {
        if( LPTIM_START_CONTINUOUS == startMode )
        {
            LpTim_Set_RegBit( periphId, LPTIM_REG_CR, LPTIM_CR_CNTSTRT );
        }
        else
        {
            LpTim_Set_RegBit( periphId, LPTIM_REG_CR, LPTIM_CR_SNGSTRT );
        }

        lpTim_State[periphId].StartMode = startMode;
        lpTim_State[periphId].Running   = LPTIM_FUNCTION_ACTIVE;
        retState                        = LPTIM_REQUEST_OK;
    }
    else
    {
        retState = LPTIM_REQUEST_ERROR;
    }

    return ( retState );
}


/**
 * \brief Stops a running LPTIM instance
 *
 * \note The LPTIM hardware only knows a full disable (which resets every register of the
 *       instance), there is no pause/resume of the counter: a subsequent \ref LpTim_Set_Start
 *       requires the instance to be configured again with \ref LpTim_Init.
 *
 * \param periphId [in] : Instance to stop.
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
lpTim_RequestState_t LpTim_Set_Stop( lpTim_PeriphId_t periphId )
{
    lpTim_RequestState_t retState = LPTIM_REQUEST_ERROR;

    if( ( LPTIM_PERIPH_CNT      > periphId                     ) &&
        ( LPTIM_FUNCTION_ACTIVE == lpTim_State[periphId].Used   )    )
    {
        LpTim_Reset_RegBit( periphId, LPTIM_REG_CR, LPTIM_CR_ENABLE );
        lpTim_State[periphId].Running = LPTIM_FUNCTION_INACTIVE;
        retState                      = LPTIM_REQUEST_OK;
    }
    else
    {
        retState = LPTIM_REQUEST_ERROR;
    }

    return ( retState );
}


/**
 * \brief Reads the run state of an LPTIM instance
 *
 * \param periphId [in]  : Instance to query.
 * \param runState [out] : LPTIM_FUNCTION_ACTIVE while the instance is running.
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
lpTim_RequestState_t LpTim_Get_State( lpTim_PeriphId_t periphId, lpTim_FunctionState_t * const runState )
{
    lpTim_RequestState_t retState = LPTIM_REQUEST_ERROR;

    if( ( LPTIM_PERIPH_CNT      > periphId                     ) &&
        ( LPTIM_NULL_PTR        != runState                     ) &&
        ( LPTIM_FUNCTION_ACTIVE == lpTim_State[periphId].Used    )    )
    {
        *runState = lpTim_State[periphId].Running;
        retState  = LPTIM_REQUEST_OK;
    }
    else
    {
        retState = LPTIM_REQUEST_ERROR;
    }

    return ( retState );
}


/**
 * \brief Configures, in one call, a one-shot timeout and starts the instance
 *
 * \param periphId [in] : Instance already initialized.
 * \param timeout  [in] : Requested timeout in us.
 * \param callback [in] : Callback called from the interrupt at the timeout.
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
lpTim_RequestState_t LpTim_Set_Timeout( lpTim_PeriphId_t periphId,
                                        lpTim_Period_us_t timeout,
                                        lpTim_Callback_t callback )
{
    lpTim_RequestState_t retState = LPTIM_REQUEST_ERROR;

    retState = LpTim_Set_Period( periphId, timeout );

    if( LPTIM_REQUEST_OK == retState )
    {
        retState = LpTim_Set_IrqCallback( periphId, LPTIM_EVENT_UPDATE, callback );
    }
    else
    {
        /* Period does not fit the instance's resolution, nothing else is done */
    }

    if( LPTIM_REQUEST_OK == retState )
    {
        retState = LpTim_Set_IrqActive( periphId, LPTIM_EVENT_UPDATE );
    }
    else
    {
        /* Callback registration failed, nothing else is done */
    }

    if( LPTIM_REQUEST_OK == retState )
    {
        retState = LpTim_Set_Start( periphId, LPTIM_START_SINGLE );
    }
    else
    {
        /* Event activation failed, the instance is not started */
    }

    return ( retState );
}

/* ------------------------------- Time base ----------------------------------- */

/**
 * \brief Changes the period of an instance
 *
 * \param periphId [in] : Instance to configure.
 * \param period   [in] : Requested period in us.
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
lpTim_RequestState_t LpTim_Set_Period( lpTim_PeriphId_t periphId, lpTim_Period_us_t period )
{
    lpTim_RequestState_t retState = LPTIM_REQUEST_ERROR;
    lpTim_Counter_t        arrValue = 0u;

    if( ( LPTIM_PERIPH_CNT      > periphId                     ) &&
        ( LPTIM_FUNCTION_ACTIVE == lpTim_State[periphId].Used   )    )
    {
        retState = LpTim_CalcArrFixedPrescaler( periphId, period, &arrValue );
    }
    else
    {
        retState = LPTIM_REQUEST_ERROR;
    }

    if( LPTIM_REQUEST_OK == retState )
    {
        retState = LpTim_Reg_Set_ArrConfirmed( periphId, arrValue );
    }
    else
    {
        /* Instance not initialized, nothing is written */
    }

    return ( retState );
}


/**
 * \brief Reads the achieved period of an instance
 *
 * \param periphId [in]  : Instance to query.
 * \param period   [out] : Achieved period in us.
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
lpTim_RequestState_t LpTim_Get_Period( lpTim_PeriphId_t periphId, lpTim_Period_us_t * const period )
{
    lpTim_RequestState_t retState  = LPTIM_REQUEST_ERROR;
    uint32_t                arrValue  = 0u;
    uint32_t                prescReg  = 0u;
    uint64_t                periodUs  = 0u;

    if( ( LPTIM_PERIPH_CNT > periphId ) &&
        ( LPTIM_NULL_PTR   != period  ) &&
        ( LPTIM_FUNCTION_ACTIVE == lpTim_State[periphId].Used )    )
    {
        arrValue = LpTim_Get_RegVal( periphId, LPTIM_REG_ARR, LPTIM_COUNTER_MAX );
        prescReg = LpTim_Get_RegVal( periphId, LPTIM_REG_CFGR, LPTIM_CFGR_PRESC ) >> LPTIM_CFGR_PRESC_Pos;

        periodUs = ( ( (uint64_t)arrValue + 1u ) * ( (uint64_t)1u << prescReg ) *
                     (uint64_t)LPTIM_US_PER_SECOND ) / lpTim_State[periphId].ClkFreqHz;

        *period  = (lpTim_Period_us_t)periodUs;
        retState = LPTIM_REQUEST_OK;
    }
    else
    {
        retState = LPTIM_REQUEST_ERROR;
    }

    return ( retState );
}


/**
 * \brief Reads the current counter value of an instance
 *
 * The counter is read until two consecutive reads agree (asynchronous clock domain).
 *
 * \param periphId [in]  : Instance to query.
 * \param counter  [out] : Current counter value.
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
lpTim_RequestState_t LpTim_Get_Counter( lpTim_PeriphId_t periphId, lpTim_Counter_t * const counter )
{
    lpTim_RequestState_t retState  = LPTIM_REQUEST_ERROR;
    uint32_t                previous  = 0u;
    uint32_t                current   = 1u;
    uint8_t                  agreed    = 0u;

    if( ( LPTIM_PERIPH_CNT > periphId ) &&
        ( LPTIM_NULL_PTR   != counter ) &&
        ( LPTIM_FUNCTION_ACTIVE == lpTim_State[periphId].Used )    )
    {
        previous = LpTim_Get_RegVal( periphId, LPTIM_REG_CNTR, LPTIM_COUNTER_MAX );

        for( uint32_t idx = 0u;
             ( LPTIM_REG_WAIT_LIMIT > idx ) &&
             ( 0u == agreed )                  ;
             idx++ )
        {
            current = LpTim_Get_RegVal( periphId, LPTIM_REG_CNTR, LPTIM_COUNTER_MAX );

            if( current == previous )
            {
                agreed = 1u;
            }
            else
            {
                previous = current;
            }
        }

        *counter = (lpTim_Counter_t)current;
        retState = LPTIM_REQUEST_OK;
    }
    else
    {
        retState = LPTIM_REQUEST_ERROR;
    }

    return ( retState );
}


/**
 * \brief Reads the kernel clock frequency used by an instance
 *
 * \param periphId [in]  : Instance to query.
 * \param clkFreq  [out] : Kernel clock frequency in Hz (before the instance's own prescaler).
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
lpTim_RequestState_t LpTim_Get_ClkFrequency( lpTim_PeriphId_t periphId, lpTim_Frequency_Hz_t * const clkFreq )
{
    lpTim_RequestState_t retState = LPTIM_REQUEST_ERROR;

    if( ( LPTIM_PERIPH_CNT > periphId   ) &&
        ( LPTIM_NULL_PTR   != clkFreq   ) &&
        ( LPTIM_FUNCTION_ACTIVE == lpTim_State[periphId].Used )    )
    {
        *clkFreq = lpTim_State[periphId].ClkFreqHz;
        retState = LPTIM_REQUEST_OK;
    }
    else
    {
        retState = LPTIM_REQUEST_ERROR;
    }

    return ( retState );
}


/**
 * \brief Configures the repetition counter (burst of additional periods) of an instance
 *
 * \param periphId   [in] : Instance to configure.
 * \param repetition [in] : Number of additional periods before the update event.
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
lpTim_RequestState_t LpTim_Set_Repetition( lpTim_PeriphId_t periphId, lpTim_Repetition_t repetition )
{
    lpTim_RequestState_t retState = LPTIM_REQUEST_ERROR;

    if( ( LPTIM_PERIPH_CNT      > periphId                     ) &&
        ( LPTIM_FUNCTION_ACTIVE == lpTim_State[periphId].Used   )    )
    {
        LpTim_Reset_RegBit( periphId, LPTIM_REG_ICR, LPTIM_ISR_REPOK );
        LpTim_Set_RegVal( periphId, LPTIM_REG_RCR, LPTIM_RCR_REP, (uint32_t)repetition );
        retState = LpTim_Reg_Wait_RegBitSet( periphId, LPTIM_REG_ISR, LPTIM_ISR_REPOK );
        LpTim_Set_RegBit( periphId, LPTIM_REG_ICR, LPTIM_ISR_REPOK );
    }
    else
    {
        retState = LPTIM_REQUEST_ERROR;
    }

    return ( retState );
}

/* -------------------------------- Channels ------------------------------------ */

/**
 * \brief Configures a channel of an instance (PWM, output compare or input capture)
 *
 * \note The capture/compare mode register (CCMR1) is only modified while the instance is
 *       stopped; the call is rejected with an error while the instance is running.
 *
 * \param periphId      [in] : Instance of the channel.
 * \param channelId     [in] : Channel to configure.
 * \param channelConfig [in] : Configuration of the channel.
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
lpTim_RequestState_t LpTim_Set_ChannelConfig( lpTim_PeriphId_t periphId,
                                              lpTim_ChannelId_t channelId,
                                              const lpTim_ChannelConfig_t * const channelConfig )
{
    lpTim_RequestState_t retState  = LPTIM_REQUEST_ERROR;
    uint32_t                selBit    = 0u;
    uint32_t                polField  = 0u;
    uint32_t                filtField = 0u;

    if( ( LPTIM_PERIPH_CNT      > periphId                    ) &&
        ( LPTIM_CHANNEL_CNT     > channelId                    ) &&
        ( LPTIM_NULL_PTR        != channelConfig               ) &&
        ( LPTIM_CHANNEL_MODE_CNT > channelConfig->Mode          ) &&
        ( LPTIM_POLARITY_CNT    > channelConfig->Polarity       ) &&
        ( LPTIM_FILTER_CNT      > channelConfig->InputFilter    ) &&
        ( LPTIM_FUNCTION_ACTIVE == lpTim_State[periphId].Used   ) &&
        ( LPTIM_FUNCTION_ACTIVE != lpTim_State[periphId].Running )    )
    {
        if( LPTIM_CHANNEL_INPUT_CAPTURE == channelConfig->Mode )
        {
            selBit = 1u;
        }
        else
        {
            selBit = 0u;
        }

        if( LPTIM_POLARITY_LOW == channelConfig->Polarity )
        {
            polField = 1u;
        }
        else
        {
            polField = 0u;
        }

        filtField = (uint32_t)channelConfig->InputFilter;

        if( LPTIM_CHANNEL_1 == channelId )
        {
            LpTim_Set_RegVal( periphId, LPTIM_REG_CCMR1, LPTIM_CCMR1_CC1SEL, selBit << LPTIM_CCMR1_CC1SEL_Pos );
            LpTim_Set_RegVal( periphId, LPTIM_REG_CCMR1, LPTIM_CCMR1_CC1P,   polField << LPTIM_CCMR1_CC1P_Pos  );
            LpTim_Set_RegVal( periphId, LPTIM_REG_CCMR1, LPTIM_CCMR1_IC1F,   filtField << LPTIM_CCMR1_IC1F_Pos );
            LpTim_Set_RegBit( periphId, LPTIM_REG_CCMR1, LPTIM_CCMR1_CC1E );
        }
        else
        {
            LpTim_Set_RegVal( periphId, LPTIM_REG_CCMR1, LPTIM_CCMR1_CC2SEL, selBit << LPTIM_CCMR1_CC2SEL_Pos );
            LpTim_Set_RegVal( periphId, LPTIM_REG_CCMR1, LPTIM_CCMR1_CC2P,   polField << LPTIM_CCMR1_CC2P_Pos  );
            LpTim_Set_RegVal( periphId, LPTIM_REG_CCMR1, LPTIM_CCMR1_IC2F,   filtField << LPTIM_CCMR1_IC2F_Pos );
            LpTim_Set_RegBit( periphId, LPTIM_REG_CCMR1, LPTIM_CCMR1_CC2E );
        }

        lpTim_State[periphId].ChannelMode[channelId] = channelConfig->Mode;
        retState                                     = LPTIM_REQUEST_OK;
    }
    else
    {
        retState = LPTIM_REQUEST_ERROR;
    }

    return ( retState );
}


/**
 * \brief Changes the duty cycle of a PWM channel
 *
 * \param periphId  [in] : Instance of the channel.
 * \param channelId [in] : Channel configured in the LPTIM_CHANNEL_PWM mode.
 * \param duty      [in] : Requested duty cycle in 0.1 %.
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
lpTim_RequestState_t LpTim_Set_PwmDuty( lpTim_PeriphId_t periphId,
                                        lpTim_ChannelId_t channelId,
                                        lpTim_Duty_permille_t duty )
{
    lpTim_RequestState_t retState  = LPTIM_REQUEST_ERROR;
    uint32_t                arrValue  = 0u;
    uint64_t                ccrValue  = 0u;

    if( ( LPTIM_PERIPH_CNT          > periphId                              ) &&
        ( LPTIM_CHANNEL_CNT         > channelId                             ) &&
        ( LPTIM_DUTY_PERMILLE_MAX   >= duty                                 ) &&
        ( LPTIM_FUNCTION_ACTIVE     == lpTim_State[periphId].Used            ) &&
        ( LPTIM_CHANNEL_PWM         == lpTim_State[periphId].ChannelMode[channelId] )    )
    {
        arrValue = LpTim_Get_RegVal( periphId, LPTIM_REG_ARR, LPTIM_COUNTER_MAX );
        ccrValue = ( (uint64_t)duty * ( (uint64_t)arrValue + 1u ) ) / LPTIM_DUTY_PERMILLE_MAX;
        retState = LpTim_Reg_Set_CcrConfirmed( periphId, channelId, (lpTim_Counter_t)ccrValue );
    }
    else
    {
        retState = LPTIM_REQUEST_ERROR;
    }

    return ( retState );
}


/**
 * \brief Reads the last captured value of an input capture channel
 *
 * \param periphId  [in]  : Instance of the channel.
 * \param channelId [in]  : Channel configured in the LPTIM_CHANNEL_INPUT_CAPTURE mode.
 * \param capture   [out] : Last captured counter value.
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
lpTim_RequestState_t LpTim_Get_Capture( lpTim_PeriphId_t periphId,
                                        lpTim_ChannelId_t channelId,
                                        lpTim_Counter_t * const capture )
{
    lpTim_RequestState_t retState = LPTIM_REQUEST_ERROR;
    lpTim_RegId_t          ccrReg  = LPTIM_REG_CNT;

    if( LPTIM_CHANNEL_1 == channelId )
    {
        ccrReg = LPTIM_REG_CCR1;
    }
    else
    {
        ccrReg = LPTIM_REG_CCR2;
    }

    if( ( LPTIM_PERIPH_CNT          > periphId                                ) &&
        ( LPTIM_CHANNEL_CNT         > channelId                               ) &&
        ( LPTIM_NULL_PTR            != capture                                ) &&
        ( LPTIM_FUNCTION_ACTIVE     == lpTim_State[periphId].Used              ) &&
        ( LPTIM_CHANNEL_INPUT_CAPTURE == lpTim_State[periphId].ChannelMode[channelId] )    )
    {
        *capture = (lpTim_Counter_t)LpTim_Get_RegVal( periphId, ccrReg, LPTIM_COUNTER_MAX );
        retState = LPTIM_REQUEST_OK;
    }
    else
    {
        retState = LPTIM_REQUEST_ERROR;
    }

    return ( retState );
}

/* ---------------------------- Trigger and inputs ------------------------------- */

/**
 * \brief Configures the external trigger (start condition) of an instance
 *
 * \note The trigger source is the instance's external trigger pin (lpTim_TriggerPin_t);
 *       selecting an internal trigger source (another instance's output) is not part of
 *       this version, the CFGR.TRIGSEL field is kept at its reset value (external pin).
 *
 * \note The configuration register (CFGR) is only modified while the instance is stopped;
 *       the call is rejected with an error while the instance is running.
 *
 * \param periphId      [in] : Instance to configure.
 * \param triggerConfig [in] : Trigger configuration.
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
lpTim_RequestState_t LpTim_Set_Trigger( lpTim_PeriphId_t periphId,
                                         const lpTim_TriggerConfig_t * const triggerConfig )
{
    lpTim_RequestState_t retState  = LPTIM_REQUEST_ERROR;
    uint32_t                cfgrValue = 0u;

    if( ( LPTIM_PERIPH_CNT      > periphId                           ) &&
        ( LPTIM_NULL_PTR        != triggerConfig                     ) &&
        ( LPTIM_POLARITY_CNT    > triggerConfig->Polarity             ) &&
        ( LPTIM_FILTER_CNT      > triggerConfig->Filter               ) &&
        ( LPTIM_FUNCTION_ACTIVE == lpTim_State[periphId].Used         ) &&
        ( LPTIM_FUNCTION_ACTIVE != lpTim_State[periphId].Running       )    )
    {
        if( LPTIM_FUNCTION_ACTIVE == triggerConfig->TriggerActive )
        {
            if( LPTIM_POLARITY_HIGH == triggerConfig->Polarity )
            {
                cfgrValue = LPTIM_CFGR_TRIGEN_0;
            }
            else
            {
                cfgrValue = LPTIM_CFGR_TRIGEN_1;
            }
        }
        else
        {
            cfgrValue = 0u;
        }

        cfgrValue |= ( (uint32_t)triggerConfig->Filter << LPTIM_CFGR_TRGFLT_Pos ) & LPTIM_CFGR_TRGFLT;

        LpTim_Set_RegVal( periphId, LPTIM_REG_CFGR, ( LPTIM_CFGR_TRIGEN | LPTIM_CFGR_TRGFLT ), cfgrValue );
        retState = LPTIM_REQUEST_OK;
    }
    else
    {
        retState = LPTIM_REQUEST_ERROR;
    }

    return ( retState );
}


/**
 * \brief Configures the digital filter of a channel input
 *
 * \note The capture/compare mode register (CCMR1) is only modified while the instance is
 *       stopped; the call is rejected with an error while the instance is running.
 *
 * \param periphId  [in] : Instance of the channel.
 * \param channelId [in] : Channel whose input filter is configured.
 * \param filter    [in] : Requested filter length.
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
lpTim_RequestState_t LpTim_Set_InputFilter( lpTim_PeriphId_t periphId,
                                            lpTim_ChannelId_t channelId,
                                            lpTim_Filter_t filter )
{
    lpTim_RequestState_t retState = LPTIM_REQUEST_ERROR;

    if( ( LPTIM_PERIPH_CNT  > periphId                        ) &&
        ( LPTIM_CHANNEL_CNT > channelId                       ) &&
        ( LPTIM_FILTER_CNT  > filter                           ) &&
        ( LPTIM_FUNCTION_ACTIVE == lpTim_State[periphId].Used  ) &&
        ( LPTIM_FUNCTION_ACTIVE != lpTim_State[periphId].Running )    )
    {
        if( LPTIM_CHANNEL_1 == channelId )
        {
            LpTim_Set_RegVal( periphId, LPTIM_REG_CCMR1, LPTIM_CCMR1_IC1F, (uint32_t)filter << LPTIM_CCMR1_IC1F_Pos );
        }
        else
        {
            LpTim_Set_RegVal( periphId, LPTIM_REG_CCMR1, LPTIM_CCMR1_IC2F, (uint32_t)filter << LPTIM_CCMR1_IC2F_Pos );
        }

        retState = LPTIM_REQUEST_OK;
    }
    else
    {
        retState = LPTIM_REQUEST_ERROR;
    }

    return ( retState );
}

/* ------------------------- Encoder and pulse counter ---------------------------- */

/**
 * \brief Configures the encoder mode of an instance
 *
 * \note The capture/compare mode register (CCMR1) is only modified while the instance is
 *       stopped; the call is rejected with an error while the instance is running.
 *
 * \param periphId      [in] : Instance to configure.
 * \param encoderConfig [in] : Encoder configuration.
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
lpTim_RequestState_t LpTim_Set_EncoderConfig( lpTim_PeriphId_t periphId,
                                               const lpTim_EncoderConfig_t * const encoderConfig )
{
    lpTim_RequestState_t retState = LPTIM_REQUEST_ERROR;
    uint32_t                polField = 0u;

    if( ( LPTIM_NULL_PTR    != encoderConfig                      ) &&
        ( LPTIM_PERIPH_CNT  > periphId                            ) &&
        ( LPTIM_POLARITY_CNT > encoderConfig->Polarity             ) &&
        ( LPTIM_FILTER_CNT  > encoderConfig->InputFilter           ) &&
        ( LPTIM_MODE_ENCODER == lpTim_State[periphId].Mode         ) &&
        ( LPTIM_FUNCTION_ACTIVE == lpTim_State[periphId].Used      ) &&
        ( LPTIM_FUNCTION_ACTIVE != lpTim_State[periphId].Running    )    )
    {
        if( LPTIM_POLARITY_LOW == encoderConfig->Polarity )
        {
            polField = 1u;
        }
        else
        {
            polField = 0u;
        }

        LpTim_Set_RegVal( periphId, LPTIM_REG_CCMR1, LPTIM_CCMR1_CC1P, polField << LPTIM_CCMR1_CC1P_Pos );
        LpTim_Set_RegVal( periphId, LPTIM_REG_CCMR1, LPTIM_CCMR1_CC2P, polField << LPTIM_CCMR1_CC2P_Pos );
        LpTim_Set_RegVal( periphId,
                          LPTIM_REG_CCMR1,
                          LPTIM_CCMR1_IC1F,
                          (uint32_t)encoderConfig->InputFilter << LPTIM_CCMR1_IC1F_Pos );
        LpTim_Set_RegVal( periphId,
                          LPTIM_REG_CCMR1,
                          LPTIM_CCMR1_IC2F,
                          (uint32_t)encoderConfig->InputFilter << LPTIM_CCMR1_IC2F_Pos );
        LpTim_Set_RegBit( periphId, LPTIM_REG_CCMR1, ( LPTIM_CCMR1_CC1E | LPTIM_CCMR1_CC2E ) );

        retState = LPTIM_REQUEST_OK;
    }
    else
    {
        retState = LPTIM_REQUEST_ERROR;
    }

    return ( retState );
}


/**
 * \brief Reads the current direction of the encoder / pulse counter
 *
 * \param periphId  [in]  : Instance to query.
 * \param direction [out] : Current counting direction, updated by \ref LpTim_Task or by the
 *        LPTIM_EVENT_DIRECTION_UP / LPTIM_EVENT_DIRECTION_DOWN interrupt.
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
lpTim_RequestState_t LpTim_Get_EncoderDirection( lpTim_PeriphId_t periphId, lpTim_Direction_t * const direction )
{
    lpTim_RequestState_t retState = LPTIM_REQUEST_ERROR;

    if( ( LPTIM_PERIPH_CNT > periphId   ) &&
        ( LPTIM_NULL_PTR   != direction ) &&
        ( LPTIM_FUNCTION_ACTIVE == lpTim_State[periphId].Used )    )
    {
        *direction = lpTim_State[periphId].Direction;
        retState   = LPTIM_REQUEST_OK;
    }
    else
    {
        retState = LPTIM_REQUEST_ERROR;
    }

    return ( retState );
}


/**
 * \brief Reads the pulse count of an instance in the pulse counter mode
 *
 * \param periphId   [in]  : Instance to query.
 * \param pulseCount [out] : Current counter value.
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
lpTim_RequestState_t LpTim_Get_PulseCount( lpTim_PeriphId_t periphId, lpTim_Counter_t * const pulseCount )
{
    return ( LpTim_Get_Counter( periphId, pulseCount ) );
}

/* --------------------------- Interrupts and DMA --------------------------------- */

/**
 * \brief Configures the NVIC interrupt priority of an instance
 *
 * \param periphId [in] : Instance to configure.
 * \param irqPrio  [in] : Requested interrupt priority.
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
lpTim_RequestState_t LpTim_Set_IrqPriority( lpTim_PeriphId_t periphId, lpTim_IrqPrio_t irqPrio )
{
    lpTim_RequestState_t retState = LPTIM_REQUEST_ERROR;
    nvic_RequestState_t    nvicState = NVIC_REQUEST_ERROR;

    if( ( LPTIM_PERIPH_CNT      > periphId                     ) &&
        ( LPTIM_FUNCTION_ACTIVE == lpTim_State[periphId].Used   )    )
    {
        nvicState = Nvic_Set_PeriphIrq_Prio( lpTim_PeriphIrq[periphId], irqPrio );
    }
    else
    {
        nvicState = NVIC_REQUEST_ERROR;
    }

    if( NVIC_REQUEST_OK == nvicState )
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
 * \brief Enables an event of an instance (interrupt and / or polled flag)
 *
 * \param periphId [in] : Instance to configure.
 * \param eventId  [in] : Event to enable.
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
lpTim_RequestState_t LpTim_Set_IrqActive( lpTim_PeriphId_t periphId, lpTim_EventId_t eventId )
{
    lpTim_RequestState_t retState = LPTIM_REQUEST_ERROR;

    if( ( LPTIM_PERIPH_CNT > periphId ) &&
        ( LPTIM_EVENT_CNT  > eventId  ) &&
        ( LPTIM_FUNCTION_ACTIVE == lpTim_State[periphId].Used )    )
    {
        LpTim_Reset_RegBit( periphId, LPTIM_REG_ICR, LPTIM_ISR_DIEROK );
        LpTim_Set_RegBit( periphId, LPTIM_REG_DIER, LpTim_Event_DierBit( eventId ) );
        retState = LpTim_Reg_Wait_RegBitSet( periphId, LPTIM_REG_ISR, LPTIM_ISR_DIEROK );
        LpTim_Set_RegBit( periphId, LPTIM_REG_ICR, LPTIM_ISR_DIEROK );
    }
    else
    {
        retState = LPTIM_REQUEST_ERROR;
    }

    return ( retState );
}


/**
 * \brief Disables an event of an instance
 *
 * \param periphId [in] : Instance to configure.
 * \param eventId  [in] : Event to disable.
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
lpTim_RequestState_t LpTim_Set_IrqInactive( lpTim_PeriphId_t periphId, lpTim_EventId_t eventId )
{
    lpTim_RequestState_t retState = LPTIM_REQUEST_ERROR;

    if( ( LPTIM_PERIPH_CNT > periphId ) &&
        ( LPTIM_EVENT_CNT  > eventId  ) &&
        ( LPTIM_FUNCTION_ACTIVE == lpTim_State[periphId].Used )    )
    {
        LpTim_Reset_RegBit( periphId, LPTIM_REG_ICR, LPTIM_ISR_DIEROK );
        LpTim_Reset_RegBit( periphId, LPTIM_REG_DIER, LpTim_Event_DierBit( eventId ) );
        retState = LpTim_Reg_Wait_RegBitSet( periphId, LPTIM_REG_ISR, LPTIM_ISR_DIEROK );
        LpTim_Set_RegBit( periphId, LPTIM_REG_ICR, LPTIM_ISR_DIEROK );
    }
    else
    {
        retState = LPTIM_REQUEST_ERROR;
    }

    return ( retState );
}


/**
 * \brief Registers the callback called from the interrupt of an event
 *
 * \param periphId [in] : Instance to configure.
 * \param eventId  [in] : Event whose callback is registered.
 * \param callback [in] : Callback to register, may be LPTIM_NULL_PTR to unregister.
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
lpTim_RequestState_t LpTim_Set_IrqCallback( lpTim_PeriphId_t periphId,
                                            lpTim_EventId_t eventId,
                                            lpTim_Callback_t callback )
{
    lpTim_RequestState_t retState = LPTIM_REQUEST_ERROR;

    if( ( LPTIM_PERIPH_CNT > periphId ) &&
        ( LPTIM_EVENT_CNT  > eventId  ) &&
        ( LPTIM_FUNCTION_ACTIVE == lpTim_State[periphId].Used )    )
    {
        lpTim_State[periphId].EventCbk[eventId] = callback;
        retState                                 = LPTIM_REQUEST_OK;
    }
    else
    {
        retState = LPTIM_REQUEST_ERROR;
    }

    return ( retState );
}


/**
 * \brief Enables the DMA request of an event of an instance
 *
 * \param periphId [in] : Instance to configure.
 * \param eventId  [in] : Event whose DMA request is enabled (update or capture only).
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
lpTim_RequestState_t LpTim_Set_DmaRequestActive( lpTim_PeriphId_t periphId, lpTim_EventId_t eventId )
{
    lpTim_RequestState_t retState = LPTIM_REQUEST_ERROR;
    uint32_t                dmaBit   = 0u;

    dmaBit = LpTim_Event_DmaBit( eventId );

    if( ( LPTIM_PERIPH_CNT > periphId ) &&
        ( 0u               != dmaBit  ) &&
        ( LPTIM_FUNCTION_ACTIVE == lpTim_State[periphId].Used )    )
    {
        LpTim_Set_RegBit( periphId, LPTIM_REG_DIER, dmaBit );
        retState = LPTIM_REQUEST_OK;
    }
    else
    {
        retState = LPTIM_REQUEST_ERROR;
    }

    return ( retState );
}


/**
 * \brief Disables the DMA request of an event of an instance
 *
 * \param periphId [in] : Instance to configure.
 * \param eventId  [in] : Event whose DMA request is disabled.
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
lpTim_RequestState_t LpTim_Set_DmaRequestInactive( lpTim_PeriphId_t periphId, lpTim_EventId_t eventId )
{
    lpTim_RequestState_t retState = LPTIM_REQUEST_ERROR;
    uint32_t                dmaBit   = 0u;

    dmaBit = LpTim_Event_DmaBit( eventId );

    if( ( LPTIM_PERIPH_CNT > periphId ) &&
        ( 0u               != dmaBit  ) &&
        ( LPTIM_FUNCTION_ACTIVE == lpTim_State[periphId].Used )    )
    {
        LpTim_Reset_RegBit( periphId, LPTIM_REG_DIER, dmaBit );
        retState = LPTIM_REQUEST_OK;
    }
    else
    {
        retState = LPTIM_REQUEST_ERROR;
    }

    return ( retState );
}

/* ---------------------------------- Pins ---------------------------------------- */

/**
 * \brief Configures the GPIO pin of a channel output (PWM / output compare)
 *
 * \param pinId [in] : Pin to configure (one of the device's lpTim_OutputPin_t items).
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
lpTim_RequestState_t LpTim_InitOutputGpio( lpTim_OutputPin_t pinId )
{
    lpTim_RequestState_t retState      = LPTIM_REQUEST_ERROR;
    gpio_RequestState_t    gpioInitState = GPIO_REQUEST_ERROR;
    gpio_Config_t           pinConfig     = { 0u };

    pinConfig.PortId         = (gpio_PortId_t)LPTIM_BIT_MASK_DECODE_PORT( pinId );
    pinConfig.PinId          = (gpio_PinId_t)LPTIM_BIT_MASK_DECODE_PIN( pinId );
    pinConfig.PinMode        = GPIO_PIN_MODE_ALTERNATE;
    pinConfig.PinPull        = GPIO_PIN_PULL_NONE;
    pinConfig.PinSpeed       = GPIO_PIN_SPEED_MEDIUM;
    pinConfig.PinOutType     = GPIO_PIN_OUTPUT_PUSHPULL;
    pinConfig.PinAltFunction = (gpio_AltFunction_t)LPTIM_BIT_MASK_DECODE_AF( pinId );

    gpioInitState = Gpio_Init( &pinConfig );

    if( GPIO_REQUEST_ERROR != gpioInitState )
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
 * \brief Configures the GPIO pin of a channel input (input capture / encoder / pulse counter)
 *
 * \param pinId [in] : Pin to configure (one of the device's lpTim_InputPin_t items).
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
lpTim_RequestState_t LpTim_InitInputGpio( lpTim_InputPin_t pinId )
{
    lpTim_RequestState_t retState      = LPTIM_REQUEST_ERROR;
    gpio_RequestState_t    gpioInitState = GPIO_REQUEST_ERROR;
    gpio_Config_t           pinConfig     = { 0u };

    pinConfig.PortId         = (gpio_PortId_t)LPTIM_BIT_MASK_DECODE_PORT( pinId );
    pinConfig.PinId          = (gpio_PinId_t)LPTIM_BIT_MASK_DECODE_PIN( pinId );
    pinConfig.PinMode        = GPIO_PIN_MODE_ALTERNATE;
    pinConfig.PinPull        = GPIO_PIN_PULL_NONE;
    pinConfig.PinSpeed       = GPIO_PIN_SPEED_MEDIUM;
    pinConfig.PinOutType     = GPIO_PIN_OUTPUT_PUSHPULL;
    pinConfig.PinAltFunction = (gpio_AltFunction_t)LPTIM_BIT_MASK_DECODE_AF( pinId );

    gpioInitState = Gpio_Init( &pinConfig );

    if( GPIO_REQUEST_ERROR != gpioInitState )
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
 * \brief Configures the GPIO pin of the external trigger input
 *
 * \param pinId [in] : Pin to configure (one of the device's lpTim_TriggerPin_t items).
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
lpTim_RequestState_t LpTim_InitTriggerGpio( lpTim_TriggerPin_t pinId )
{
    lpTim_RequestState_t retState      = LPTIM_REQUEST_ERROR;
    gpio_RequestState_t    gpioInitState = GPIO_REQUEST_ERROR;
    gpio_Config_t           pinConfig     = { 0u };

    pinConfig.PortId         = (gpio_PortId_t)LPTIM_BIT_MASK_DECODE_PORT( pinId );
    pinConfig.PinId          = (gpio_PinId_t)LPTIM_BIT_MASK_DECODE_PIN( pinId );
    pinConfig.PinMode        = GPIO_PIN_MODE_ALTERNATE;
    pinConfig.PinPull        = GPIO_PIN_PULL_NONE;
    pinConfig.PinSpeed       = GPIO_PIN_SPEED_MEDIUM;
    pinConfig.PinOutType     = GPIO_PIN_OUTPUT_PUSHPULL;
    pinConfig.PinAltFunction = (gpio_AltFunction_t)LPTIM_BIT_MASK_DECODE_AF( pinId );

    gpioInitState = Gpio_Init( &pinConfig );

    if( GPIO_REQUEST_ERROR != gpioInitState )
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
 * \brief Configures the GPIO pin of the external clock input
 *
 * \param pinId [in] : Pin to configure (one of the device's lpTim_ExtClkPin_t items).
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
lpTim_RequestState_t LpTim_InitExtClkGpio( lpTim_ExtClkPin_t pinId )
{
    lpTim_RequestState_t retState      = LPTIM_REQUEST_ERROR;
    gpio_RequestState_t    gpioInitState = GPIO_REQUEST_ERROR;
    gpio_Config_t           pinConfig     = { 0u };

    pinConfig.PortId         = (gpio_PortId_t)LPTIM_BIT_MASK_DECODE_PORT( pinId );
    pinConfig.PinId          = (gpio_PinId_t)LPTIM_BIT_MASK_DECODE_PIN( pinId );
    pinConfig.PinMode        = GPIO_PIN_MODE_ALTERNATE;
    pinConfig.PinPull        = GPIO_PIN_PULL_NONE;
    pinConfig.PinSpeed       = GPIO_PIN_SPEED_MEDIUM;
    pinConfig.PinOutType     = GPIO_PIN_OUTPUT_PUSHPULL;
    pinConfig.PinAltFunction = (gpio_AltFunction_t)LPTIM_BIT_MASK_DECODE_AF( pinId );

    gpioInitState = Gpio_Init( &pinConfig );

    if( GPIO_REQUEST_ERROR != gpioInitState )
    {
        retState = LPTIM_REQUEST_OK;
    }
    else
    {
        retState = LPTIM_REQUEST_ERROR;
    }

    return ( retState );
}
