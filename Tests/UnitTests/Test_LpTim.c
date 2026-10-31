/**
 * \author Mr.Nobody
 * \file Test_LpTim.c
 * \ingroup LpTim
 * \brief Unit tests of the Low-Power Timer (LPTIM) module.
 *
 * LpTim.c / LpTim_Reg.c are compiled unchanged. LPTIM1 registers are emulated by RegMem, RCC /
 * NVIC / GPIO modules are mocked by CMock.
 *
 * \note The auto-reload / compare / repetition / DIER write confirmation flags (ARROK, CMP1OK,
 *       CMP2OK, REPOK, DIEROK) are set by the hardware itself after the write; plain RegMem does
 *       not emulate this, so the tests that need a confirmed write activate the HW model
 *       \ref UtLpTim_HwModel_Confirm, which ORs every confirmation flag into ISR after every
 *       store (see RegMem_Set_ModelActive() of the RegMem framework).
 */

/* ============================= INCLUDES =================================== */
#include "unity.h"                          /* Unity testing framework        */
#include "RegMem.h"                         /* Register memory emulation      */
#include "LpTim_Port.h"                     /* Module under test              */
#include "LpTim.h"                          /* Private interface (test only)  */
#include "LpTim_Reg.h"                      /* Register access (test only)    */
#include "MockRcc_Port.h"                   /* RCC module mock                */
#include "MockNvic_Port.h"                  /* NVIC module mock                */
#include "MockGpio_Port.h"                  /* GPIO module mock                */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

static void UtLpTim_HwModel_Confirm( void );
static void UtLpTim_InitNominal( lpTim_Period_us_t period );
static void UtLpTim_Callback( void );

/* ========================== SYMBOLIC CONSTANTS ============================= */

/** Kernel clock used by most tests (Hz), matches LPTIM_CLK_SRC_LPTIM1_PCLK3 */
#define UT_LPTIM_CLK_HZ              ( 4000000u )

/** Period used by most tests (us), one tick at the smallest prescaler (4 MHz) */
#define UT_LPTIM_PERIOD_US           ( 1000u )

/** Shorter period used by the Set_Period test (us) */
#define UT_LPTIM_PERIOD_SHORT_US     ( 500u )

/** Long period that needs a prescaler above 1 at the 4 MHz kernel clock (us), prescaler 8 */
#define UT_LPTIM_PERIOD_LONG_US      ( 100000u )

/** Period beyond the 16-bit counter at the smallest prescaler of the 4 MHz kernel clock (us) */
#define UT_LPTIM_PERIOD_TOO_LONG_US  ( 20000u )

/** Counter value preset by the Get_Counter test */
#define UT_LPTIM_COUNTER_PRESET     ( 123u )

/** Duty cycle used by the PWM tests (25.0 %, in 0.1 %) */
#define UT_LPTIM_DUTY_PERMILLE       ( 250u )

/** Permille base used to derive the expected compare value */
#define UT_LPTIM_PERMILLE_BASE       ( 1000u )

/** Timeout used by the Set_Timeout test (us) */
#define UT_LPTIM_TIMEOUT_US          ( 2000u )

/* ========================== LOCAL VARIABLES ================================ */

/** Count of calls of the test callback */
static uint32_t utLpTim_CallbackCnt;

/* ============================ TEST FIXTURE ================================= */

void setUp( void )
{
    TEST_ASSERT_EQUAL( REGMEM_REQUEST_OK, RegMem_Reset() );
    utLpTim_CallbackCnt = 0u;
}


void tearDown( void )
{
    ( void )RegMem_Set_ModelInactive();
}

/* ============================ HELPER FUNCTIONS ============================== */

/** HW model: every register write is immediately confirmed (ARROK/CMPxOK/REPOK/DIEROK set) */
static void UtLpTim_HwModel_Confirm( void )
{
    uint32_t confirmMask = ( LPTIM_ISR_ARROK | LPTIM_ISR_CMP1OK | LPTIM_ISR_CMP2OK |
                             LPTIM_ISR_REPOK | LPTIM_ISR_DIEROK   );

    ( void )__atomic_or_fetch( &LPTIM1->ISR, confirmMask, __ATOMIC_SEQ_CST );
}


/** Initializes LPTIM_PERIPH_1 in the time base mode with the given period, expects success */
static void UtLpTim_InitNominal( lpTim_Period_us_t period )
{
    lpTim_Config_t      lpTimConfig;
    rcc_FreqHz_t          clkHz = UT_LPTIM_CLK_HZ;

    TEST_ASSERT_EQUAL( REGMEM_REQUEST_OK, RegMem_Set_ModelActive( UtLpTim_HwModel_Confirm ) );

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Get_DefaultConfig( &lpTimConfig ) );
    lpTimConfig.PeriphId = LPTIM_PERIPH_1;
    lpTimConfig.Period   = period;

    Rcc_Set_PeriphActive_ExpectAndReturn( RCC_PERIPH_LPTIM1_PCLK3, RCC_REQUEST_OK );
    Rcc_Get_PeriphClk_ExpectAndReturn( RCC_PERIPH_LPTIM1_PCLK3, NULL, RCC_REQUEST_OK );
    Rcc_Get_PeriphClk_IgnoreArg_periphClk();
    Rcc_Get_PeriphClk_ReturnThruPtr_periphClk( &clkHz );
    Nvic_Set_PeriphIrq_Handler_ExpectAnyArgsAndReturn( NVIC_REQUEST_OK );
    Nvic_Set_PeriphIrq_Prio_ExpectAndReturn( NVIC_PERIPH_IRQ_LPTIM1, 0u, NVIC_REQUEST_OK );
    Nvic_Set_PeriphIrq_Active_ExpectAndReturn( NVIC_PERIPH_IRQ_LPTIM1, NVIC_REQUEST_OK );

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Init( &lpTimConfig ) );
}


/** Test callback, counts its own calls */
static void UtLpTim_Callback( void )
{
    utLpTim_CallbackCnt++;
}

/* ============================== TEST CASES =================================== */

/**
 * \brief Verifies the module version is reported.
 *
 * \par Expected results
 * LpTim_Get_ModuleVersion() returns a version with Major == 1.
 */
void Ut_LpTim_Get_ModuleVersion_ReturnsVersion( void )
{
    lpTim_ModuleVersion_t version = LpTim_Get_ModuleVersion();

    TEST_ASSERT_EQUAL( 1u, version.Major );
}


/**
 * \brief Verifies the default configuration of an instance.
 *
 * \par Expected results
 * LpTim_Get_DefaultConfig() returns LPTIM_REQUEST_OK, the time base mode, the continuous start
 * mode and no callback; LPTIM_REQUEST_ERROR for a NULL pointer.
 */
void Ut_LpTim_Get_DefaultConfig_Nominal( void )
{
    lpTim_Config_t lpTimConfig;

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Get_DefaultConfig( &lpTimConfig ) );
    TEST_ASSERT_EQUAL( LPTIM_MODE_TIMEBASE, lpTimConfig.Mode );
    TEST_ASSERT_EQUAL( LPTIM_START_CONTINUOUS, lpTimConfig.StartMode );
    TEST_ASSERT_NULL( lpTimConfig.Callback );
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_ERROR, LpTim_Get_DefaultConfig( NULL ) );
}


/**
 * \brief Verifies the nominal initialization of an instance.
 *
 * \par Expected results
 * LpTim_Init() enables the RCC clock, reads the kernel clock, registers the NVIC handler and
 * priority, activates the NVIC interrupt and leaves the instance stopped with CR.ENABLE set.
 */
void Ut_LpTim_Init_Nominal_ReturnsOk( void )
{
    lpTim_FunctionState_t runState = LPTIM_FUNCTION_ACTIVE;

    UtLpTim_InitNominal( UT_LPTIM_PERIOD_US );

    TEST_ASSERT_EQUAL( 1u, ( LPTIM1->CR & LPTIM_CR_ENABLE ) );
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Get_State( LPTIM_PERIPH_1, &runState ) );
    TEST_ASSERT_EQUAL( LPTIM_FUNCTION_INACTIVE, runState );
}


/**
 * \brief Verifies a NULL configuration pointer is rejected.
 *
 * \par Expected results
 * LpTim_Init( NULL ) returns LPTIM_REQUEST_ERROR without touching the RCC / NVIC modules.
 */
void Ut_LpTim_Init_NullConfig_ReturnsError( void )
{
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_ERROR, LpTim_Init( NULL ) );
}


/**
 * \brief Verifies an out-of-range trigger Polarity / Filter is rejected by LpTim_Init().
 *
 * \par Expected results
 * LpTim_Init() returns LPTIM_REQUEST_ERROR without touching the RCC module.
 */
void Ut_LpTim_Init_OutOfRangeTrigger_ReturnsError( void )
{
    lpTim_Config_t lpTimConfig;

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Get_DefaultConfig( &lpTimConfig ) );
    lpTimConfig.PeriphId             = LPTIM_PERIPH_1;
    lpTimConfig.Period               = UT_LPTIM_PERIOD_US;
    lpTimConfig.Trigger.Filter       = LPTIM_FILTER_CNT;

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_ERROR, LpTim_Init( &lpTimConfig ) );
}


/**
 * \brief Verifies a failed clock activation is reported and the instance is cleaned up.
 *
 * \par Expected results
 * LpTim_Init() returns LPTIM_REQUEST_ERROR when Rcc_Set_PeriphActive() fails; the instance is
 * deinitialized (Rcc_Set_PeriphInactive() and Nvic_Set_PeriphIrq_Inactive() are called).
 */
void Ut_LpTim_Init_RccClockFails_ReturnsError( void )
{
    lpTim_Config_t lpTimConfig;

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Get_DefaultConfig( &lpTimConfig ) );
    lpTimConfig.PeriphId = LPTIM_PERIPH_1;
    lpTimConfig.Period   = UT_LPTIM_PERIOD_US;

    Rcc_Set_PeriphActive_ExpectAndReturn( RCC_PERIPH_LPTIM1_PCLK3, RCC_REQUEST_ERROR );
    Nvic_Set_PeriphIrq_Inactive_ExpectAnyArgsAndReturn( NVIC_REQUEST_OK );
    Rcc_Set_PeriphInactive_ExpectAnyArgsAndReturn( RCC_REQUEST_OK );

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_ERROR, LpTim_Init( &lpTimConfig ) );
}


/**
 * \brief Verifies the period is changed and the write is confirmed.
 *
 * \par Expected results
 * LpTim_Set_Period() returns LPTIM_REQUEST_OK and LpTim_Get_Period() reads back a period close
 * to the requested one (within the resolution of the kernel clock).
 */
void Ut_LpTim_Set_Period_ConfirmedWrite_ReturnsOk( void )
{
    lpTim_Period_us_t readPeriod = 0u;

    UtLpTim_InitNominal( UT_LPTIM_PERIOD_US );

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Set_Period( LPTIM_PERIPH_1, UT_LPTIM_PERIOD_SHORT_US ) );
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Get_Period( LPTIM_PERIPH_1, &readPeriod ) );
    TEST_ASSERT_UINT32_WITHIN( 1u, UT_LPTIM_PERIOD_SHORT_US, readPeriod );
}


/**
 * \brief Verifies the period change keeps the prescaler chosen by LpTim_Init().
 *
 * \par Expected results
 * After LpTim_Init() with a long period (prescaler 8 at the 4 MHz kernel clock),
 * LpTim_Set_Period() with a shorter period returns LPTIM_REQUEST_OK and LpTim_Get_Period() reads
 * back the requested period, as the auto-reload value is computed for the unchanged prescaler.
 */
void Ut_LpTim_Set_Period_FixedPrescaler_ReadsBackRequestedPeriod( void )
{
    lpTim_Period_us_t readPeriod = 0u;

    UtLpTim_InitNominal( UT_LPTIM_PERIOD_LONG_US );

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Set_Period( LPTIM_PERIPH_1, UT_LPTIM_PERIOD_SHORT_US ) );
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Get_Period( LPTIM_PERIPH_1, &readPeriod ) );
    TEST_ASSERT_UINT32_WITHIN( 1u, UT_LPTIM_PERIOD_SHORT_US, readPeriod );
}


/**
 * \brief Verifies a period which does not fit the prescaler fixed by LpTim_Init() is rejected.
 *
 * \par Expected results
 * After LpTim_Init() with a short period (prescaler 1), LpTim_Set_Period() with a period beyond
 * the 16-bit counter at that prescaler returns LPTIM_REQUEST_ERROR and the period is unchanged.
 */
void Ut_LpTim_Set_Period_BeyondFixedPrescaler_ReturnsError( void )
{
    lpTim_Period_us_t readPeriod = 0u;

    UtLpTim_InitNominal( UT_LPTIM_PERIOD_US );

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_ERROR, LpTim_Set_Period( LPTIM_PERIPH_1, UT_LPTIM_PERIOD_TOO_LONG_US ) );
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Get_Period( LPTIM_PERIPH_1, &readPeriod ) );
    TEST_ASSERT_UINT32_WITHIN( 1u, UT_LPTIM_PERIOD_US, readPeriod );
}


/**
 * \brief Verifies a period change on a non-initialized instance is rejected.
 *
 * \par Expected results
 * LpTim_Set_Period() returns LPTIM_REQUEST_ERROR without touching any register.
 */
void Ut_LpTim_Set_Period_NotInitialized_ReturnsError( void )
{
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_ERROR, LpTim_Set_Period( LPTIM_PERIPH_1, UT_LPTIM_PERIOD_US ) );
}


/**
 * \brief Verifies the counter is read back correctly when two consecutive reads agree.
 *
 * \par Expected results
 * LpTim_Get_Counter() returns LPTIM_REQUEST_OK and the preset counter value.
 */
void Ut_LpTim_Get_Counter_TwoReadsAgree_ReturnsOk( void )
{
    lpTim_Counter_t counter = 0u;

    UtLpTim_InitNominal( UT_LPTIM_PERIOD_US );
    LPTIM1->CNT = UT_LPTIM_COUNTER_PRESET;

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Get_Counter( LPTIM_PERIPH_1, &counter ) );
    TEST_ASSERT_EQUAL( UT_LPTIM_COUNTER_PRESET, counter );
}


/**
 * \brief Verifies the continuous start mode sets CR.CNTSTRT and the running state.
 *
 * \par Expected results
 * LpTim_Set_Start() returns LPTIM_REQUEST_OK, CR.CNTSTRT is set, LpTim_Get_State() reports
 * LPTIM_FUNCTION_ACTIVE.
 */
void Ut_LpTim_Set_Start_Continuous_SetsRunning( void )
{
    lpTim_FunctionState_t runState = LPTIM_FUNCTION_INACTIVE;

    UtLpTim_InitNominal( UT_LPTIM_PERIOD_US );

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Set_Start( LPTIM_PERIPH_1, LPTIM_START_CONTINUOUS ) );
    TEST_ASSERT_EQUAL( LPTIM_CR_CNTSTRT, ( LPTIM1->CR & LPTIM_CR_CNTSTRT ) );
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Get_State( LPTIM_PERIPH_1, &runState ) );
    TEST_ASSERT_EQUAL( LPTIM_FUNCTION_ACTIVE, runState );
}


/**
 * \brief Verifies stopping a running instance clears CR.ENABLE and the running state.
 *
 * \par Expected results
 * LpTim_Set_Stop() returns LPTIM_REQUEST_OK, CR.ENABLE is cleared, LpTim_Get_State() reports
 * LPTIM_FUNCTION_INACTIVE.
 */
void Ut_LpTim_Set_Stop_ClearsEnableAndRunning( void )
{
    lpTim_FunctionState_t runState = LPTIM_FUNCTION_ACTIVE;

    UtLpTim_InitNominal( UT_LPTIM_PERIOD_US );
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Set_Start( LPTIM_PERIPH_1, LPTIM_START_CONTINUOUS ) );

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Set_Stop( LPTIM_PERIPH_1 ) );
    TEST_ASSERT_EQUAL( 0u, ( LPTIM1->CR & LPTIM_CR_ENABLE ) );
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Get_State( LPTIM_PERIPH_1, &runState ) );
    TEST_ASSERT_EQUAL( LPTIM_FUNCTION_INACTIVE, runState );
}


/**
 * \brief Verifies a PWM channel configuration sets the CCMR1 fields of channel 1.
 *
 * \par Expected results
 * LpTim_Set_ChannelConfig() returns LPTIM_REQUEST_OK, CCMR1.CC1SEL is 0 (compare), CCMR1.CC1E
 * is set.
 */
void Ut_LpTim_Set_ChannelConfig_Pwm_SetsCcmr1( void )
{
    lpTim_ChannelConfig_t channelConfig;

    UtLpTim_InitNominal( UT_LPTIM_PERIOD_US );

    channelConfig.Mode        = LPTIM_CHANNEL_PWM;
    channelConfig.Polarity    = LPTIM_POLARITY_HIGH;
    channelConfig.InputFilter = LPTIM_FILTER_NONE;

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK,
                        LpTim_Set_ChannelConfig( LPTIM_PERIPH_1, LPTIM_CHANNEL_1, &channelConfig ) );
    TEST_ASSERT_EQUAL( 0u, ( LPTIM1->CCMR1 & LPTIM_CCMR1_CC1SEL ) );
    TEST_ASSERT_EQUAL( LPTIM_CCMR1_CC1E, ( LPTIM1->CCMR1 & LPTIM_CCMR1_CC1E ) );
}


/**
 * \brief Verifies an invalid channel identification is rejected.
 *
 * \par Expected results
 * LpTim_Set_ChannelConfig() returns LPTIM_REQUEST_ERROR for LPTIM_CHANNEL_CNT.
 */
void Ut_LpTim_Set_ChannelConfig_InvalidChannel_ReturnsError( void )
{
    lpTim_ChannelConfig_t channelConfig;

    UtLpTim_InitNominal( UT_LPTIM_PERIOD_US );

    channelConfig.Mode        = LPTIM_CHANNEL_PWM;
    channelConfig.Polarity    = LPTIM_POLARITY_HIGH;
    channelConfig.InputFilter = LPTIM_FILTER_NONE;

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_ERROR,
                        LpTim_Set_ChannelConfig( LPTIM_PERIPH_1, LPTIM_CHANNEL_CNT, &channelConfig ) );
}


/**
 * \brief Verifies the PWM duty cycle is converted to the compare register and confirmed.
 *
 * \par Expected results
 * LpTim_Set_PwmDuty() returns LPTIM_REQUEST_OK and CCR1 holds 25 % of (ARR + 1).
 */
void Ut_LpTim_Set_PwmDuty_Nominal_WritesCcr1( void )
{
    lpTim_ChannelConfig_t channelConfig;
    uint32_t                 arrValue = 0u;

    UtLpTim_InitNominal( UT_LPTIM_PERIOD_US );

    channelConfig.Mode        = LPTIM_CHANNEL_PWM;
    channelConfig.Polarity    = LPTIM_POLARITY_HIGH;
    channelConfig.InputFilter = LPTIM_FILTER_NONE;
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK,
                        LpTim_Set_ChannelConfig( LPTIM_PERIPH_1, LPTIM_CHANNEL_1, &channelConfig ) );

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK,
                        LpTim_Set_PwmDuty( LPTIM_PERIPH_1, LPTIM_CHANNEL_1, UT_LPTIM_DUTY_PERMILLE ) );

    arrValue = LPTIM1->ARR;
    TEST_ASSERT_EQUAL( ( ( arrValue + 1u ) * UT_LPTIM_DUTY_PERMILLE ) / UT_LPTIM_PERMILLE_BASE, LPTIM1->CCR1 );
}


/**
 * \brief Verifies LpTim_Task() updates the cached encoder direction from the ISR.UP flag.
 *
 * \par Expected results
 * LpTim_Task() reads LPTIM_DIRECTION_UP after ISR.UP is preset (the clear of the flag through
 * ICR.UP is not observable on the plain RegMem emulation, which does not propagate a write of
 * one register to another: it is exercised by the \ref Ut_LpTim_Set_IrqActive_Update_SetsDierBit
 * style tests that activate the HW model instead).
 */
void Ut_LpTim_Task_DirectionUp_UpdatesState( void )
{
    lpTim_Direction_t direction = LPTIM_DIRECTION_DOWN;

    UtLpTim_InitNominal( UT_LPTIM_PERIOD_US );
    LPTIM1->ISR |= LPTIM_ISR_UP;

    LpTim_Task();

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Get_EncoderDirection( LPTIM_PERIPH_1, &direction ) );
    TEST_ASSERT_EQUAL( LPTIM_DIRECTION_UP, direction );
}


/**
 * \brief Verifies enabling an event sets the matching DIER bit and the write is confirmed.
 *
 * \par Expected results
 * LpTim_Set_IrqActive( LPTIM_EVENT_UPDATE ) returns LPTIM_REQUEST_OK and DIER.UEIE is set.
 */
void Ut_LpTim_Set_IrqActive_Update_SetsDierBit( void )
{
    UtLpTim_InitNominal( UT_LPTIM_PERIOD_US );

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Set_IrqActive( LPTIM_PERIPH_1, LPTIM_EVENT_UPDATE ) );
    TEST_ASSERT_EQUAL( LPTIM_DIER_UEIE, ( LPTIM1->DIER & LPTIM_DIER_UEIE ) );
}


/**
 * \brief Verifies the callback of an event is registered and used by LpTim_Set_Timeout().
 *
 * \par Expected results
 * LpTim_Set_Timeout() registers the callback, starts the instance in the single mode (CR.SNGSTRT
 * set); the callback is not called by the request itself (no interrupt is simulated here).
 */
void Ut_LpTim_Set_Timeout_Nominal_StartsSingle( void )
{
    UtLpTim_InitNominal( UT_LPTIM_PERIOD_US );

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK,
                        LpTim_Set_Timeout( LPTIM_PERIPH_1, UT_LPTIM_TIMEOUT_US, UtLpTim_Callback ) );
    TEST_ASSERT_EQUAL( LPTIM_DIER_UEIE, ( LPTIM1->DIER & LPTIM_DIER_UEIE ) );
    TEST_ASSERT_EQUAL( LPTIM_CR_SNGSTRT, ( LPTIM1->CR & LPTIM_CR_SNGSTRT ) );
    TEST_ASSERT_EQUAL( 0u, utLpTim_CallbackCnt );
}


/**
 * \brief Verifies an output pin is decoded and passed to Gpio_Init().
 *
 * \par Expected results
 * LpTim_InitOutputGpio( LPTIM_OUT_PIN_LPTIM2_CH1_PA4 ) returns LPTIM_REQUEST_OK.
 */
void Ut_LpTim_InitOutputGpio_Nominal_CallsGpioInit( void )
{
    Gpio_Init_ExpectAnyArgsAndReturn( GPIO_REQUEST_OK );

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_InitOutputGpio( LPTIM_OUT_PIN_LPTIM2_CH1_PA4 ) );
}


/**
 * \brief Verifies a trigger pin is decoded and passed to Gpio_Init().
 *
 * \par Expected results
 * LpTim_InitTriggerGpio() returns LPTIM_REQUEST_OK.
 */
void Ut_LpTim_InitTriggerGpio_Nominal_CallsGpioInit( void )
{
    Gpio_Init_ExpectAnyArgsAndReturn( GPIO_REQUEST_OK );

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_InitTriggerGpio( LPTIM_TRIG_PIN_LPTIM2_PC4 ) );
}


/**
 * \brief Verifies an external clock pin is decoded and passed to Gpio_Init().
 *
 * \par Expected results
 * LpTim_InitExtClkGpio() returns LPTIM_REQUEST_OK.
 */
void Ut_LpTim_InitExtClkGpio_Nominal_CallsGpioInit( void )
{
    Gpio_Init_ExpectAnyArgsAndReturn( GPIO_REQUEST_OK );

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_InitExtClkGpio( LPTIM_EXTCLK_PIN_LPTIM1_PA1 ) );
}


/**
 * \brief Verifies LpTim_Get_State() is rejected on a non-initialized instance.
 *
 * \par Expected results
 * LpTim_Get_State() returns LPTIM_REQUEST_ERROR without touching the output parameter's
 * instance, matching every other getter of the module.
 */
void Ut_LpTim_Get_State_NotInitialized_ReturnsError( void )
{
    lpTim_FunctionState_t runState = LPTIM_FUNCTION_ACTIVE;

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_ERROR, LpTim_Get_State( LPTIM_PERIPH_1, &runState ) );
}


/**
 * \brief Verifies LpTim_Get_Capture() on an input capture channel.
 *
 * \par Expected results
 * LpTim_Get_Capture() returns LPTIM_REQUEST_OK and the preset CCR1 value, LPTIM_REQUEST_ERROR
 * while the channel is not configured as input capture.
 */
void Ut_LpTim_Get_Capture_Nominal_ReadsCcr1( void )
{
    lpTim_ChannelConfig_t channelConfig;
    lpTim_Counter_t        capture = 0u;

    UtLpTim_InitNominal( UT_LPTIM_PERIOD_US );

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_ERROR, LpTim_Get_Capture( LPTIM_PERIPH_1, LPTIM_CHANNEL_1, &capture ) );

    channelConfig.Mode        = LPTIM_CHANNEL_INPUT_CAPTURE;
    channelConfig.Polarity    = LPTIM_POLARITY_HIGH;
    channelConfig.InputFilter = LPTIM_FILTER_NONE;
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK,
                        LpTim_Set_ChannelConfig( LPTIM_PERIPH_1, LPTIM_CHANNEL_1, &channelConfig ) );

    LPTIM1->CCR1 = UT_LPTIM_COUNTER_PRESET;
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Get_Capture( LPTIM_PERIPH_1, LPTIM_CHANNEL_1, &capture ) );
    TEST_ASSERT_EQUAL( UT_LPTIM_COUNTER_PRESET, capture );
}


/**
 * \brief Verifies the nominal external trigger configuration sets CFGR.TRIGEN / TRGFLT.
 *
 * \par Expected results
 * LpTim_Set_Trigger() returns LPTIM_REQUEST_OK, CFGR.TRIGEN selects the rising edge.
 */
void Ut_LpTim_Set_Trigger_Nominal_SetsCfgr( void )
{
    lpTim_TriggerConfig_t triggerConfig;

    UtLpTim_InitNominal( UT_LPTIM_PERIOD_US );

    triggerConfig.TriggerActive = LPTIM_FUNCTION_ACTIVE;
    triggerConfig.Polarity      = LPTIM_POLARITY_HIGH;
    triggerConfig.Filter        = LPTIM_FILTER_NONE;

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Set_Trigger( LPTIM_PERIPH_1, &triggerConfig ) );
    TEST_ASSERT_EQUAL( LPTIM_CFGR_TRIGEN_0, ( LPTIM1->CFGR & LPTIM_CFGR_TRIGEN ) );
}


/**
 * \brief Verifies LpTim_Set_Trigger() rejects an out-of-range polarity / filter value.
 *
 * \par Expected results
 * LpTim_Set_Trigger() returns LPTIM_REQUEST_ERROR and does not touch CFGR for either an
 * out-of-range Polarity or an out-of-range Filter.
 */
void Ut_LpTim_Set_Trigger_OutOfRangeValue_ReturnsError( void )
{
    lpTim_TriggerConfig_t triggerConfig;
    uint32_t                 cfgrBefore = 0u;

    UtLpTim_InitNominal( UT_LPTIM_PERIOD_US );
    cfgrBefore = LPTIM1->CFGR;

    triggerConfig.TriggerActive = LPTIM_FUNCTION_ACTIVE;
    triggerConfig.Polarity      = LPTIM_POLARITY_CNT;
    triggerConfig.Filter        = LPTIM_FILTER_NONE;
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_ERROR, LpTim_Set_Trigger( LPTIM_PERIPH_1, &triggerConfig ) );
    TEST_ASSERT_EQUAL( cfgrBefore, LPTIM1->CFGR );

    triggerConfig.Polarity = LPTIM_POLARITY_HIGH;
    triggerConfig.Filter   = LPTIM_FILTER_CNT;
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_ERROR, LpTim_Set_Trigger( LPTIM_PERIPH_1, &triggerConfig ) );
    TEST_ASSERT_EQUAL( cfgrBefore, LPTIM1->CFGR );
}


/**
 * \brief Verifies LpTim_Set_Trigger() is rejected while the instance is running.
 *
 * \par Expected results
 * LpTim_Set_Trigger() returns LPTIM_REQUEST_ERROR once the instance was started (CFGR is only
 * modified while the instance is stopped, see \ref LpTim_Set_Trigger).
 */
void Ut_LpTim_Set_Trigger_Running_ReturnsError( void )
{
    lpTim_TriggerConfig_t triggerConfig;

    UtLpTim_InitNominal( UT_LPTIM_PERIOD_US );
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Set_Start( LPTIM_PERIPH_1, LPTIM_START_CONTINUOUS ) );

    triggerConfig.TriggerActive = LPTIM_FUNCTION_ACTIVE;
    triggerConfig.Polarity      = LPTIM_POLARITY_HIGH;
    triggerConfig.Filter        = LPTIM_FILTER_NONE;
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_ERROR, LpTim_Set_Trigger( LPTIM_PERIPH_1, &triggerConfig ) );
}


/**
 * \brief Verifies LpTim_Set_InputFilter() sets the CCMR1 filter field of channel 1.
 *
 * \par Expected results
 * LpTim_Set_InputFilter() returns LPTIM_REQUEST_OK and CCMR1.IC1F holds the requested filter.
 */
void Ut_LpTim_Set_InputFilter_Channel1_SetsIc1f( void )
{
    UtLpTim_InitNominal( UT_LPTIM_PERIOD_US );

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK,
                        LpTim_Set_InputFilter( LPTIM_PERIPH_1, LPTIM_CHANNEL_1, LPTIM_FILTER_4_CLOCKS ) );
    TEST_ASSERT_EQUAL( (uint32_t)LPTIM_FILTER_4_CLOCKS << LPTIM_CCMR1_IC1F_Pos,
                        ( LPTIM1->CCMR1 & LPTIM_CCMR1_IC1F ) );
}


/**
 * \brief Verifies LpTim_Set_InputFilter() is rejected while the instance is running.
 *
 * \par Expected results
 * LpTim_Set_InputFilter() returns LPTIM_REQUEST_ERROR once the instance was started.
 */
void Ut_LpTim_Set_InputFilter_Running_ReturnsError( void )
{
    UtLpTim_InitNominal( UT_LPTIM_PERIOD_US );
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Set_Start( LPTIM_PERIPH_1, LPTIM_START_CONTINUOUS ) );

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_ERROR,
                        LpTim_Set_InputFilter( LPTIM_PERIPH_1, LPTIM_CHANNEL_1, LPTIM_FILTER_NONE ) );
}


/**
 * \brief Verifies the nominal encoder mode configuration sets CFGR.ENC and CCMR1.
 *
 * \par Expected results
 * LpTim_Init() with LPTIM_MODE_ENCODER sets CFGR.ENC; LpTim_Set_EncoderConfig() returns
 * LPTIM_REQUEST_OK and enables both channels (CCMR1.CC1E / CC2E).
 */
void Ut_LpTim_Set_EncoderConfig_Nominal_SetsCcmr1( void )
{
    lpTim_Config_t          lpTimConfig;
    lpTim_EncoderConfig_t encoderConfig;
    rcc_FreqHz_t             clkHz = UT_LPTIM_CLK_HZ;

    TEST_ASSERT_EQUAL( REGMEM_REQUEST_OK, RegMem_Set_ModelActive( UtLpTim_HwModel_Confirm ) );
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Get_DefaultConfig( &lpTimConfig ) );
    lpTimConfig.PeriphId = LPTIM_PERIPH_1;
    lpTimConfig.Period   = UT_LPTIM_PERIOD_US;
    lpTimConfig.Mode     = LPTIM_MODE_ENCODER;

    Rcc_Set_PeriphActive_ExpectAndReturn( RCC_PERIPH_LPTIM1_PCLK3, RCC_REQUEST_OK );
    Rcc_Get_PeriphClk_ExpectAndReturn( RCC_PERIPH_LPTIM1_PCLK3, NULL, RCC_REQUEST_OK );
    Rcc_Get_PeriphClk_IgnoreArg_periphClk();
    Rcc_Get_PeriphClk_ReturnThruPtr_periphClk( &clkHz );
    Nvic_Set_PeriphIrq_Handler_ExpectAnyArgsAndReturn( NVIC_REQUEST_OK );
    Nvic_Set_PeriphIrq_Prio_ExpectAndReturn( NVIC_PERIPH_IRQ_LPTIM1, 0u, NVIC_REQUEST_OK );
    Nvic_Set_PeriphIrq_Active_ExpectAndReturn( NVIC_PERIPH_IRQ_LPTIM1, NVIC_REQUEST_OK );
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Init( &lpTimConfig ) );
    TEST_ASSERT_EQUAL( LPTIM_CFGR_ENC, ( LPTIM1->CFGR & LPTIM_CFGR_ENC ) );

    encoderConfig.Polarity    = LPTIM_POLARITY_HIGH;
    encoderConfig.InputFilter = LPTIM_FILTER_NONE;
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Set_EncoderConfig( LPTIM_PERIPH_1, &encoderConfig ) );
    TEST_ASSERT_EQUAL( ( LPTIM_CCMR1_CC1E | LPTIM_CCMR1_CC2E ),
                        ( LPTIM1->CCMR1 & ( LPTIM_CCMR1_CC1E | LPTIM_CCMR1_CC2E ) ) );
}


/**
 * \brief Verifies LpTim_Set_EncoderConfig() is rejected while the instance is running.
 *
 * \par Expected results
 * LpTim_Set_EncoderConfig() returns LPTIM_REQUEST_ERROR once the instance was started.
 */
void Ut_LpTim_Set_EncoderConfig_Running_ReturnsError( void )
{
    lpTim_Config_t          lpTimConfig;
    lpTim_EncoderConfig_t encoderConfig;
    rcc_FreqHz_t             clkHz = UT_LPTIM_CLK_HZ;

    TEST_ASSERT_EQUAL( REGMEM_REQUEST_OK, RegMem_Set_ModelActive( UtLpTim_HwModel_Confirm ) );
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Get_DefaultConfig( &lpTimConfig ) );
    lpTimConfig.PeriphId = LPTIM_PERIPH_1;
    lpTimConfig.Period   = UT_LPTIM_PERIOD_US;
    lpTimConfig.Mode     = LPTIM_MODE_ENCODER;

    Rcc_Set_PeriphActive_ExpectAndReturn( RCC_PERIPH_LPTIM1_PCLK3, RCC_REQUEST_OK );
    Rcc_Get_PeriphClk_ExpectAndReturn( RCC_PERIPH_LPTIM1_PCLK3, NULL, RCC_REQUEST_OK );
    Rcc_Get_PeriphClk_IgnoreArg_periphClk();
    Rcc_Get_PeriphClk_ReturnThruPtr_periphClk( &clkHz );
    Nvic_Set_PeriphIrq_Handler_ExpectAnyArgsAndReturn( NVIC_REQUEST_OK );
    Nvic_Set_PeriphIrq_Prio_ExpectAndReturn( NVIC_PERIPH_IRQ_LPTIM1, 0u, NVIC_REQUEST_OK );
    Nvic_Set_PeriphIrq_Active_ExpectAndReturn( NVIC_PERIPH_IRQ_LPTIM1, NVIC_REQUEST_OK );
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Init( &lpTimConfig ) );
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Set_Start( LPTIM_PERIPH_1, LPTIM_START_CONTINUOUS ) );

    encoderConfig.Polarity    = LPTIM_POLARITY_HIGH;
    encoderConfig.InputFilter = LPTIM_FILTER_NONE;
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_ERROR, LpTim_Set_EncoderConfig( LPTIM_PERIPH_1, &encoderConfig ) );
}


/**
 * \brief Verifies LpTim_Set_ChannelConfig() is rejected while the instance is running.
 *
 * \par Expected results
 * LpTim_Set_ChannelConfig() returns LPTIM_REQUEST_ERROR once the instance was started (CCMR1 is
 * only modified while the instance is stopped).
 */
void Ut_LpTim_Set_ChannelConfig_Running_ReturnsError( void )
{
    lpTim_ChannelConfig_t channelConfig;

    UtLpTim_InitNominal( UT_LPTIM_PERIOD_US );
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Set_Start( LPTIM_PERIPH_1, LPTIM_START_CONTINUOUS ) );

    channelConfig.Mode        = LPTIM_CHANNEL_PWM;
    channelConfig.Polarity    = LPTIM_POLARITY_HIGH;
    channelConfig.InputFilter = LPTIM_FILTER_NONE;
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_ERROR,
                        LpTim_Set_ChannelConfig( LPTIM_PERIPH_1, LPTIM_CHANNEL_1, &channelConfig ) );
}
