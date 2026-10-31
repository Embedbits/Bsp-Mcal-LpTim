#======================== Unit tests configuration ============================#
# Unit tests of LpTim module. Registered in module CMakeLists.txt by
# UnitTesting_AddPath() and created by UnitTesting_Generate() in "UnitTest"
# build (see EmBi_Platform/CMake/UnitTesting/README.md).

get_filename_component(LPTIM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
get_filename_component(MCAL_DIR  "${LPTIM_DIR}/.." ABSOLUTE)

# LPTIM1 registers are emulated by RegMem. The auto-reload / compare / repetition / DIER
# write confirmation flags (ARROK, CMP1OK, CMP2OK, REPOK, DIEROK) are set by the hardware
# itself after the write, which plain RegMem does not emulate: the HW model of RegMem sets
# them after every store (see UtLpTim_HwModel in the test source).
UnitTesting_Add_Test(
    NAME            LpTim
    TEST_SOURCE     Test_LpTim.c
    UUT_SOURCES     ${LPTIM_DIR}/LpTim.c
                    ${LPTIM_DIR}/LpTim_Reg.c
    MOCK_HEADERS    ${MCAL_DIR}/Rcc/Rcc_Port.h
                    ${MCAL_DIR}/Nvic/Nvic_Port.h
                    ${MCAL_DIR}/Gpio/Gpio_Port.h
    INCLUDE_DIRS    ${LPTIM_DIR}
                    ${MCAL_DIR}/Rcc
                    ${MCAL_DIR}/Nvic
                    ${MCAL_DIR}/Gpio
    LINK_LIBS       ut_RegMem                           # Registers in host memory
                    ut_CmsisHost                        # CMSIS core intrinsics on host
    FORCE_INCLUDES  ${UNIT_TESTING_CMSIS_HOST_HEADER}   # Replaces cmsis_gcc.h
    HOST_LIBS       Ral_Lib                             # ST LL compiled for host
    COVERAGE_EXCLUDE ".*/Ral/.*"
    RUN_SERIAL
)
