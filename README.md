# LPTIM Peripheral Driver

This module provides an abstraction layer for the **low-power timers** (`LPTIMx`) of STM32 MCUs: a 16-bit timer that keeps
running in the low-power modes and wakes the MCU from them. It offers a time base with a period in microseconds, one-shot
timeouts, PWM / output compare and input capture channels, a pulse counter and the encoder mode. It is a low-level MCAL
module: it owns the timer hardware, the application and the other modules never touch the registers.

The public interface (`LpTim_Port.h`, `LpTim_Types.h`) is **identical for all supported families**. The implementation
for a family lives in its own branch (`Dev/<Family>` for development, `Releases/<Family>` for the released versions), so
a module or an application written against the interface does not change when the MCU family changes.

This README is the landing page and the **specification of the interface** (master). The family branches contain their
own README with everything that is specific to the family: the instances of the device, their settings and the status of
the implementation.

> **Status: design.** No implementation exists yet, the interface below is a proposal. Stories and branches follow.

---

## Intent and purpose

- The general purpose timers (`Tim` module) stop in the low-power modes. The LPTIM is made for the opposite: it is clocked
  by an oscillator that runs in Stop mode (the 32.768 kHz LSE or the LSI, optionally the HSI kernel clock) or by an
  external signal, so it is the **wake-up source and the time base of the low-power application** - for example the
  tick of an RTOS in the tickless idle mode, a periodic sensor sampling, a timeout of a communication or a pulse counter
  that counts when everything else is asleep.
- The user thinks in time, not in registers: the module converts the required period in us to the prescaler (1 ... 128) and
  the auto-reload value, selects the best resolution, and reports the **achieved** period back.
- The LPTIM is clocked from another domain than the bus. Every write of the auto-reload and compare register has to be
  confirmed by the hardware, the counter has to be read twice, the clock source can be changed only when the timer is
  disabled. The module hides all of it behind functions that look the same in all families.
- The module hides the differences between the families: the number of instances, the number of channels, the clock
  sources, the encoder and input capture availability and the wake-up lines differ, the interface stays the same.

What the module is **not**: it does not configure the oscillators (RCC module starts the LSE / LSI / HSI kernel clock
that was selected) and it does not decide how deep the MCU sleeps (power module, future). It is not the RTOS tick itself -
it offers the timeout that the tick implementation of a port uses.

---

## Concept

An instance is identified by `lpTim_PeriphId_t`. The mode of the instance is selected in the configuration:

| Mode | Behaviour |
|---|---|
| `LPTIM_MODE_TIMEBASE` | Counts continuously, an event (and the callback) at every period. Optional toggle / pulse on the output. |
| `LPTIM_MODE_ONE_SHOT` | Counts once for the required time, stops, calls the callback. `LpTim_Set_Timeout` is a short form of it. |
| `LPTIM_MODE_PULSE_COUNTER` | Counts the pulses of an external input (no oscillator is needed), the callback at the compare / auto-reload match. |
| `LPTIM_MODE_ENCODER` | Counts the position of a quadrature encoder on the two inputs, direction change callbacks. |

The time base and the channels are independent: an instance in the `LPTIM_MODE_TIMEBASE` mode can have **channels**
(where the instance has them) working as a PWM output, an output compare or an input capture
(`lpTim_ChannelMode_t`). An instance that has only the single output of the older LPTIM (`LPTIM4` of STM32H5, the `LPTIM1` of
STM32F4 / F7 / G4 / L4) is represented as the channel 1 that offers the PWM and the output compare, but not the input capture.

For every instance the module keeps:

| Item | Meaning |
|---|---|
| **Clock** | The selected source (`lpTim_ClkSource_t`), its frequency (read from the RCC module) and the prescaler found by the module. |
| **Period** | Requested in us, the achieved value is read back by `LpTim_Get_Period`. |
| **State** | Stopped / running (`LpTim_Get_State`), the running state includes the wait for the confirmation of the register writes. |
| **Wake-up capability** | Whether the selected clock source keeps running in Stop mode (the module asks the RCC module to keep it on). |

Rules:

1. `LpTim_Init` configures the instance and leaves it stopped, `LpTim_Set_Start` starts it (continuous or single).
2. A change of the period is written to the preload register and takes effect **at the end of the current period**;
   the call returns after the hardware confirmed the write.
3. The clock source, the mode and the trigger are changed only on a stopped instance, otherwise the call returns an
   error and nothing changes.
4. The callbacks are called from the interrupt of the instance (the module owns the interrupt and clears the flags).
   An event that is not enabled is not reported.
5. `LpTim_Task` is not needed for the correctness; it checks the confirmation of the register writes that were left
   unconfirmed when the call had to return earlier.

---

## Supported families

Every family gets its own story, its own tasks and its own branches `Dev/<Family>` and `Releases/<Family>` cut from this
master.

| Family | Branch | Instances | Status |
|---|---|---|---|
| STM32H5 | `Dev/STM32H5` | `LPTIM1` ... `LPTIM6` (STM32H503: `LPTIM1`; STM32H523 / H533 / H543 / H553: `LPTIM1`, `LPTIM2`). Two channels, encoder mode, input capture and DMA requests on `LPTIM1` - `LPTIM3`, `LPTIM5`, `LPTIM6`; `LPTIM4` has no channels, no encoder mode and no DMA | planned (first) |
| STM32U5 | `Dev/STM32U5` | `LPTIM1` ... `LPTIM4` | planned |
| STM32H7 | `Dev/STM32H7` | `LPTIM1` ... `LPTIM5` | planned |
| STM32G4 | `Dev/STM32G4` | `LPTIM1` | planned |
| STM32L4 | `Dev/STM32L4` | `LPTIM1`, `LPTIM2` | planned |
| STM32F7 | `Dev/STM32F7` | `LPTIM1` | planned |
| STM32F4 | `Dev/STM32F4` | `LPTIM1` on a few lines only | planned |
| STM32U3 | `Dev/STM32U3` | device dependent | planned |

A function without a meaning in a family stays in the interface, returns `LPTIM_REQUEST_ERROR` and suppresses the unused
parameters (for example a channel the instance does not have, the encoder mode of an instance without it, the input
capture on the families that have only the single output of the older LPTIM).

---

## Public interface in short (proposal)

| Group | Functions |
|---|---|
| Module management | `LpTim_Get_ModuleVersion`, `LpTim_Init`, `LpTim_Deinit`, `LpTim_Task`, `LpTim_Get_DefaultConfig` |
| Run control | `LpTim_Set_Start`, `LpTim_Set_Stop`, `LpTim_Get_State`, `LpTim_Set_Timeout` |
| Time base | `LpTim_Set_Period`, `LpTim_Get_Period`, `LpTim_Get_Counter`, `LpTim_Get_ClkFrequency`, `LpTim_Set_Repetition` |
| Channels | `LpTim_Set_ChannelConfig`, `LpTim_Set_PwmDuty`, `LpTim_Get_Capture` |
| Trigger and inputs | `LpTim_Set_Trigger`, `LpTim_Set_InputFilter` |
| Encoder and pulse counter | `LpTim_Set_EncoderConfig`, `LpTim_Get_EncoderDirection`, `LpTim_Get_PulseCount` |
| Interrupts and DMA | `LpTim_Set_IrqPriority`, `LpTim_Set_IrqActive`, `LpTim_Set_IrqInactive`, `LpTim_Set_IrqCallback`, `LpTim_Set_DmaRequestActive`, `LpTim_Set_DmaRequestInactive` |
| Pins | `LpTim_InitOutputGpio`, `LpTim_InitInputGpio`, `LpTim_InitTriggerGpio`, `LpTim_InitExtClkGpio` |

```c
lpTim_ModuleVersion_t LpTim_Get_ModuleVersion  ( void );

lpTim_RequestState_t  LpTim_Init               ( const lpTim_Config_t * const lpTimConfig );
lpTim_RequestState_t  LpTim_Deinit             ( lpTim_PeriphId_t periphId );
void                  LpTim_Task               ( void );
lpTim_RequestState_t  LpTim_Get_DefaultConfig  ( lpTim_Config_t * const lpTimConfig );

lpTim_RequestState_t  LpTim_Set_Start          ( lpTim_PeriphId_t periphId, lpTim_StartMode_t startMode );
lpTim_RequestState_t  LpTim_Set_Stop           ( lpTim_PeriphId_t periphId );
lpTim_RequestState_t  LpTim_Get_State          ( lpTim_PeriphId_t periphId, lpTim_FunctionState_t * const runState );
lpTim_RequestState_t  LpTim_Set_Timeout        ( lpTim_PeriphId_t periphId, lpTim_Period_us_t timeout, lpTim_Callback_t * const callback );

lpTim_RequestState_t  LpTim_Set_Period         ( lpTim_PeriphId_t periphId, lpTim_Period_us_t period );
lpTim_RequestState_t  LpTim_Get_Period         ( lpTim_PeriphId_t periphId, lpTim_Period_us_t * const period );
lpTim_RequestState_t  LpTim_Get_Counter        ( lpTim_PeriphId_t periphId, lpTim_Counter_t * const counter );
lpTim_RequestState_t  LpTim_Get_ClkFrequency   ( lpTim_PeriphId_t periphId, lpTim_Frequency_Hz_t * const clkFreq );
lpTim_RequestState_t  LpTim_Set_Repetition     ( lpTim_PeriphId_t periphId, lpTim_Repetition_t repetition );

lpTim_RequestState_t  LpTim_Set_ChannelConfig  ( lpTim_PeriphId_t periphId, lpTim_ChannelId_t channelId, const lpTim_ChannelConfig_t * const channelConfig );
lpTim_RequestState_t  LpTim_Set_PwmDuty        ( lpTim_PeriphId_t periphId, lpTim_ChannelId_t channelId, lpTim_Duty_permille_t duty );
lpTim_RequestState_t  LpTim_Get_Capture        ( lpTim_PeriphId_t periphId, lpTim_ChannelId_t channelId, lpTim_Counter_t * const capture );

lpTim_RequestState_t  LpTim_Set_Trigger        ( lpTim_PeriphId_t periphId, const lpTim_TriggerConfig_t * const triggerConfig );
lpTim_RequestState_t  LpTim_Set_InputFilter    ( lpTim_PeriphId_t periphId, lpTim_InputId_t inputId, lpTim_Filter_t filter );

lpTim_RequestState_t  LpTim_Set_EncoderConfig  ( lpTim_PeriphId_t periphId, const lpTim_EncoderConfig_t * const encoderConfig );
lpTim_RequestState_t  LpTim_Get_EncoderDirection( lpTim_PeriphId_t periphId, lpTim_Direction_t * const direction );
lpTim_RequestState_t  LpTim_Get_PulseCount     ( lpTim_PeriphId_t periphId, lpTim_Counter_t * const pulseCount );

lpTim_RequestState_t  LpTim_Set_IrqPriority    ( lpTim_PeriphId_t periphId, lpTim_IrqPrio_t irqPrio );
lpTim_RequestState_t  LpTim_Set_IrqActive      ( lpTim_PeriphId_t periphId, lpTim_EventId_t eventId );
lpTim_RequestState_t  LpTim_Set_IrqInactive    ( lpTim_PeriphId_t periphId, lpTim_EventId_t eventId );
lpTim_RequestState_t  LpTim_Set_IrqCallback    ( lpTim_PeriphId_t periphId, lpTim_EventId_t eventId, lpTim_Callback_t * const callback );
```

Every function returns `LPTIM_REQUEST_OK` / `LPTIM_REQUEST_ERROR` and checks its parameters and the state of the module
before it touches a register.

| Type | Meaning |
|---|---|
| `lpTim_PeriphId_t` | `LPTIM_PERIPH_1` ... (only the instances of the device are defined), `LPTIM_PERIPH_CNT` |
| `lpTim_Config_t` | Instance, clock source, mode, period (us), start mode, trigger, callbacks, interrupt priority |
| `lpTim_ClkSource_t` | `LPTIM_CLK_SRC_<instance>_<source>` - the sources the instance offers (bus clock, LSI, LSE, HSI, PLL output, external input); the others are not in the list |
| `lpTim_Mode_t` | `LPTIM_MODE_TIMEBASE`, `LPTIM_MODE_ONE_SHOT`, `LPTIM_MODE_PULSE_COUNTER`, `LPTIM_MODE_ENCODER` |
| `lpTim_StartMode_t` | `LPTIM_START_CONTINUOUS`, `LPTIM_START_SINGLE` |
| `lpTim_ChannelMode_t` | `LPTIM_CHANNEL_PWM`, `LPTIM_CHANNEL_OUTPUT_COMPARE`, `LPTIM_CHANNEL_INPUT_CAPTURE` |
| `lpTim_EventId_t` | `LPTIM_EVENT_UPDATE` (auto-reload match), `LPTIM_EVENT_COMPARE_1/2`, `LPTIM_EVENT_CAPTURE_1/2`, `LPTIM_EVENT_TRIGGER`, `LPTIM_EVENT_DIRECTION_UP/DOWN`, `LPTIM_EVENT_OVERCAPTURE` |
| `lpTim_Period_us_t`, `lpTim_Counter_t`, `lpTim_Duty_permille_t` | Period in us, 16-bit counter value, duty in 0.1 % |

---

## Usage

### Wake-up timeout from Stop mode

```c
#include "LpTim_Port.h"

static void App_WakeUp( void )
{
    /* called from the interrupt, the MCU is awake */
}

lpTim_Config_t lpTimConfig;

(void)LpTim_Get_DefaultConfig( &lpTimConfig );

lpTimConfig.PeriphId = LPTIM_PERIPH_1;
lpTimConfig.ClkSrc   = LPTIM_CLK_SRC_LPTIM1_LSE;     /* keeps running in Stop mode */
lpTimConfig.Mode     = LPTIM_MODE_ONE_SHOT;

(void)LpTim_Init( &lpTimConfig );
(void)LpTim_Set_Timeout( LPTIM_PERIPH_1, 2000000u, App_WakeUp );    /* 2 s */

/* enter the Stop mode: the MCU wakes up after 2 s */
```

### PWM output with a changing duty

```c
lpTim_ChannelConfig_t pwmChannel;

(void)LpTim_Get_DefaultConfig( &lpTimConfig );
lpTimConfig.PeriphId = LPTIM_PERIPH_2;
lpTimConfig.ClkSrc   = LPTIM_CLK_SRC_LPTIM2_LSI;
lpTimConfig.Mode     = LPTIM_MODE_TIMEBASE;
lpTimConfig.Period   = 10000u;                       /* 100 Hz */
(void)LpTim_Init( &lpTimConfig );

pwmChannel.Mode      = LPTIM_CHANNEL_PWM;
pwmChannel.Polarity  = LPTIM_POLARITY_HIGH;
pwmChannel.OutputPin = LPTIM_OUT_PIN_LPTIM2_CH1_PA4;      /* a pin of lpTim_OutputPin_t, only the pins of the device exist */
(void)LpTim_Set_ChannelConfig( LPTIM_PERIPH_2, LPTIM_CHANNEL_1, &pwmChannel );

(void)LpTim_Set_Start( LPTIM_PERIPH_2, LPTIM_START_CONTINUOUS );
(void)LpTim_Set_PwmDuty( LPTIM_PERIPH_2, LPTIM_CHANNEL_1, 250u );   /* 25.0 % */
```

### Pulse counter

```c
lpTim_Counter_t pulseCount = 0u;

lpTimConfig.PeriphId = LPTIM_PERIPH_3;
lpTimConfig.ClkSrc   = LPTIM_CLK_SRC_LPTIM3_EXTERNAL;   /* the pulses on the input are the clock */
lpTimConfig.Mode     = LPTIM_MODE_PULSE_COUNTER;
(void)LpTim_Init( &lpTimConfig );
(void)LpTim_Set_Start( LPTIM_PERIPH_3, LPTIM_START_CONTINUOUS );

(void)LpTim_Get_PulseCount( LPTIM_PERIPH_3, &pulseCount );
```

---

## Behaviour and limits

- **Resolution and range.** They follow from the kernel clock: with the 32.768 kHz LSE and no prescaler the resolution
  is 30.5 us, with the prescaler of 128 the longest period of the 16-bit counter is 256 s. A period that does not fit is
  rejected; the achieved period differs from the requested one by the resolution.
- **Confirmation of writes.** The auto-reload and compare registers are updated through the clock domain of the timer;
  a second write before the confirmation returns an error (busy). The module waits for the confirmation with a bound
  and reports a timeout as an error.
- **Counter read.** The counter is read until two consecutive reads agree (asynchronous clock), so `LpTim_Get_Counter`
  takes a few kernel clock cycles.
- **Stop mode.** Only the sources that run in Stop mode wake the MCU. The module asks the RCC module to keep the
  selected oscillator (and the HSI kernel clock request) on; the application must not select a source that stops.
- **Callbacks** run in the interrupt of the instance; keep them short.
- **Not part of the first version:** the autonomous mode (peripheral works without the CPU through the autonomous
  clock request), the repetition-counter-based burst control beyond `LpTim_Set_Repetition`, the XOR of the inputs,
  the interrupt-driven DMA bursts.

---

## Dependencies

- **Rcc** - peripheral clock, kernel clock source selection, frequency of the clock, request of the oscillator in Stop mode
- **Nvic** - interrupts of the instances (and the wake-up lines through the EXTI module where needed)
- **Gpio** - output, input, trigger and external clock pins
- **Gpdma** - DMA requests of the update and capture events (where the instance has them)
- **RAL (Register Abstraction Layer)** - used internally to access the registers

All mandatory RCC, NVIC and GPIO configurations are handled internally.

---

## Testing

- **Unit tests** (host, Unity + CMock + RegMem): the calculation of the prescaler and the auto-reload from the period for
  every clock source (the limits, the rounding), the rules of the rejected calls on a running instance, the confirmation
  of the register writes (also the timeout), the PWM duty calculation, the interrupt dispatch to the callbacks.
- **Integration tests** (target, probe-rs): the achieved period measured by a second timer, the wake-up from Stop mode by
  the one-shot timeout, the PWM on the output, the pulse counter fed by another timer.

---

## Interface conventions

The module follows the common rules of the MCAL layer, so that it behaves like the other modules:

- **Lists instead of free members.** Everything that the hardware restricts (the pins, the kernel clock sources, the DMA
  channels, the triggers and the other selectable sources) is a `typedef enum` list with exactly the items that are valid
  for the device line; a pin is an item `LPTIM_<SIGNAL>_PIN_<INSTANCE>_P<port><pin>` and there is an `*_UNUSED` item. A choice
  that is not in the list cannot be written.
- **Own peripheral only.** The module calls only the registers / LL functions of its own peripheral. The clocks, the
  interrupts, the pins, the DMA and the EXTI lines go through the API of the Rcc, Nvic, Gpio, Gpdma and Exti modules; a
  missing function is added to that module, it is not bypassed.
- **Init verifies itself.** `LpTim_Init` checks the result at the end; when it is not as required, the module is
  deinitialized and an error is returned. The waiting for the hardware is a loop on the register state with a timeout,
  never a blind delay.
- **Transfer modes.** Where the module moves data, the POLL (served by `LpTim_Task`), ISR and DMA modes have the same
  configuration structure, the same request and the same callbacks (error, half transfer, transfer complete).
- **Flags and states are enums of the module** (`lpTim_FunctionState_t`, `lpTim_FlagState_t`), the limits and the constants are
  named, there are no magic numbers in the interface.
- **Unsupported functions stay.** A function without a meaning in a family keeps its signature, suppresses the unused
  parameters and returns `LPTIM_REQUEST_ERROR`.
- **Documentation.** Every function and type has a Doxygen description, the main header has the `\defgroup`.

---

## Repository layout and branches

The master holds this README and the license. The family branches (`Dev/<Family>`, `Releases/<Family>`) hold:

| Path | Content |
|---|---|
| `README.md` | README of the family: the instances of the device, specifics, errata, status |
| `LpTim_Port.h`, `LpTim_Types.h` | Public interface (identical in all families) |
| `LpTim*.c`, `LpTim.h` | Implementation of the family |
| `CMakeLists.txt` | Library `LpTim_Lib` of the EmBi platform |
| `Tests/UnitTests` | Unit tests (Unity + CMock + RegMem) |
| `Tests/IntegrationTests` | Integration tests on the target (probe-rs) |

A change of the public interface is made in all families.

---

## CMake integration

```cmake
target_link_libraries( User_Lib PRIVATE LpTim_Lib )
```

1. Link `LpTim_Lib` to your CMake library.
2. Include `LpTim_Port.h` in your project.
3. Configure the module as needed for your hardware.

---

## License

This project is licensed under the **Creative Commons Attribution-NonCommercial 4.0 International (CC BY-NC 4.0)**.

You are free to use, modify, and share this work for **non-commercial purposes**, provided appropriate credit is given.

See [LICENSE.md](LICENSE.md) for full terms or visit [creativecommons.org/licenses/by-nc/4.0](https://creativecommons.org/licenses/by-nc/4.0/).

---

## Authors

- **Mr.Nobody** - [embedbits.com](https://embedbits.com)

Contributions are welcome! Please open a pull request.
