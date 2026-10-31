/**
 * \author Mr.Nobody
 * \file ItTest_LpTim.c
 * \ingroup LpTim
 * \brief Integration tests of the Low-Power Timer (LPTIM) module on target.
 *
 * LpTim module runs on the MCU together with real RCC and NVIC modules and hardware. Tests
 * verify behavior which cannot be verified by unit tests (emulated registers): the instance
 * actually enables, the auto-reload / compare write confirmations arrive from the real
 * hardware, the achieved period is close to the requested one and the run / stop state follows
 * the real control register.
 *
 * Used resources:
 * - LPTIM1 (bus clock PCLK3 kernel clock) - present on every STM32H5 line, no GPIO pin is
 *   needed for the time base tests (no channel / trigger / external clock is configured)
 *
 * \note Waiting is bounded by number of loop iterations (no reference timer), limits are set
 *       with large margin for the default system clock.
 */

/* ============================= INCLUDES =================================== */
#include "unity.h"                          /* Unity testing framework        */
#include "IntegrationTesting.h"             /* Integration testing on target  */
#include "LpTim_Port.h"                     /* Module under test              */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

static lpTim_RequestState_t ItLpTim_Init_Module( lpTim_Period_us_t period );

/* ========================= SYMBOLIC CONSTANTS ============================= */

/** Period used by the tests (us), one tick at the smallest prescaler on the bus clock */
#define IT_LPTIM_PERIOD_US                  ( 1000u )

/** Shorter period used by the period round-trip test (us) */
#define IT_LPTIM_PERIOD_SHORT_US            ( 500u )

/** Acceptable rounding margin of the achieved period (us) */
#define IT_LPTIM_PERIOD_MARGIN_US           ( 50u )

/* ========================== LOCAL VARIABLES =============================== */

/* ============================= TEST SETUP ================================= */

void setUp( void )
{
    /* No action required */
}


void tearDown( void )
{
    /* Peripheral state is cleared by system reset after the test case */
}

/* ============================== TEST CASES ================================= */

/**
 * \brief Verifies that LpTim_Init() enables LPTIM1 on real hardware (RM0481 - LPTIM_CR.ENABLE).
 *
 * \par Expected results
 * LpTim_Init() returns LPTIM_REQUEST_OK and the instance reports LPTIM_FUNCTION_INACTIVE
 * (configured, not started) through LpTim_Get_State().
 */
void It_LpTim_Init_PclkSrc_InstanceConfigured( void )
{
    lpTim_FunctionState_t runState = LPTIM_FUNCTION_ACTIVE;

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, ItLpTim_Init_Module( IT_LPTIM_PERIOD_US ) );
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Get_State( LPTIM_PERIPH_1, &runState ) );
    TEST_ASSERT_EQUAL( LPTIM_FUNCTION_INACTIVE, runState );

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Deinit( LPTIM_PERIPH_1 ) );
}


/**
 * \brief Verifies LpTim_Set_Start() / LpTim_Set_Stop() on real hardware.
 *
 * \par Expected results
 * The run state follows the requested state (LPTIM_CR.CNTSTRT / LPTIM_CR.ENABLE of the real
 * instance).
 */
void It_LpTim_SetStart_SetStop_RunStateFollows( void )
{
    lpTim_FunctionState_t runState = LPTIM_FUNCTION_INACTIVE;

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, ItLpTim_Init_Module( IT_LPTIM_PERIOD_US ) );

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Set_Start( LPTIM_PERIPH_1, LPTIM_START_CONTINUOUS ) );
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Get_State( LPTIM_PERIPH_1, &runState ) );
    TEST_ASSERT_EQUAL( LPTIM_FUNCTION_ACTIVE, runState );

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Set_Stop( LPTIM_PERIPH_1 ) );
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Get_State( LPTIM_PERIPH_1, &runState ) );
    TEST_ASSERT_EQUAL( LPTIM_FUNCTION_INACTIVE, runState );
}


/**
 * \brief Verifies the achieved period of LpTim_Set_Period() / LpTim_Get_Period() on real
 *        hardware, confirmed by the real LPTIM_ISR.ARROK flag.
 *
 * \par Expected results
 * LpTim_Set_Period() returns LPTIM_REQUEST_OK (the write is confirmed by the real hardware
 * within the bounded wait) and LpTim_Get_Period() reads back a period close to the requested
 * one.
 */
void It_LpTim_SetPeriod_GetPeriod_RoundTrips( void )
{
    lpTim_Period_us_t readPeriod = 0u;

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, ItLpTim_Init_Module( IT_LPTIM_PERIOD_US ) );

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Set_Period( LPTIM_PERIPH_1, IT_LPTIM_PERIOD_SHORT_US ) );
    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Get_Period( LPTIM_PERIPH_1, &readPeriod ) );
    TEST_ASSERT_UINT32_WITHIN( IT_LPTIM_PERIOD_MARGIN_US, IT_LPTIM_PERIOD_SHORT_US, readPeriod );

    TEST_ASSERT_EQUAL( LPTIM_REQUEST_OK, LpTim_Deinit( LPTIM_PERIPH_1 ) );
}

/* ========================== LOCAL FUNCTIONS =============================== */

/**
 * \brief Initializes LPTIM1 in the time base mode (bus clock, given period).
 *
 * \param period [in]: Requested period in us.
 * \return Result of LpTim_Init()
 */
static lpTim_RequestState_t ItLpTim_Init_Module( lpTim_Period_us_t period )
{
    lpTim_Config_t lpTimConfig;

    (void)LpTim_Get_DefaultConfig( &lpTimConfig );
    lpTimConfig.PeriphId = LPTIM_PERIPH_1;
    lpTimConfig.ClkSrc   = LPTIM_CLK_SRC_LPTIM1_PCLK3;
    lpTimConfig.Mode     = LPTIM_MODE_TIMEBASE;
    lpTimConfig.Period   = period;

    return ( LpTim_Init( &lpTimConfig ) );
}
