/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "pwm_driver.h"

#if defined(USE_MOTOR) || defined(USE_WS2812B) || defined(USE_PWM_LED)

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static sPwmOcChannelDesc_t g_pwm_lut[ePwmDevice_Last];
static bool g_is_all_device_init = false;

/* clang-format off */
static bool g_is_device_enabled[ePwmDevice_Last] = {
    #ifdef USE_PULSE_LED
    [ePwmDevice_PulseLed] = false,
    #endif

    #ifdef USE_MOTOR_A
    [ePwmDevice_MotorA_A1] = false,
    [ePwmDevice_MotorA_A2] = false,
    #endif

    #ifdef USE_MOTOR_B
    [ePwmDevice_MotorB_A1] = false,
    [ePwmDevice_MotorB_A2] = false,
    #endif

    #ifdef USE_WS2812B_1
    [ePwmDevice_Ws2812b_1] = false,
    #endif

    #ifdef USE_WS2812B_2
    [ePwmDevice_Ws2812b_2] = false,
    #endif
};
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

void PWM_Driver_DefinePerips (const sPwmOcChannelDesc_t *pwm_lut) {
    if (pwm_lut == NULL) {
        return;
    }

    g_pwm_lut = pwm_lut;

    g_is_all_device_init = false;

    return;
}

bool PWM_Driver_InitAllDevices (void) {
    if (g_is_all_device_init) {
        return true;
    }

    if (ePwmDevice_Last == 1) {
        return false;
    }

    g_is_all_device_init = true;
    
    LL_TIM_OC_InitTypeDef channel_oc_init_struct = {0};

    for (ePwmDevice_t device = (ePwmDevice_First + 1); device < ePwmDevice_Last; device++) {
        channel_oc_init_struct.OCMode = g_pwm_lut[device].mode;
        channel_oc_init_struct.OCState = g_pwm_lut[device].oc_state;
        channel_oc_init_struct.OCNState = g_pwm_lut[device].ocn_state;
        channel_oc_init_struct.CompareValue = g_pwm_lut[device].compare_value;
        channel_oc_init_struct.OCPolarity = g_pwm_lut[device].oc_polarity;
        channel_oc_init_struct.OCNPolarity = g_pwm_lut[device].ocn_polarity;
        channel_oc_init_struct.OCIdleState = g_pwm_lut[device].oc_idle;
        channel_oc_init_struct.OCNIdleState = g_pwm_lut[device].ocn_idle;
    
        if (LL_TIM_OC_Init(g_pwm_lut[device].periph, g_pwm_lut[device].channel, &channel_oc_init_struct) == ERROR) {
            g_is_all_device_init = false;
        }
    
        if (g_pwm_lut[device].fast_mode_fp != NULL) {
            g_pwm_lut[device].fast_mode_fp(g_pwm_lut[device].periph, g_pwm_lut[device].channel);
        }
    
        if (g_pwm_lut[device].compare_preload_fp != NULL) {
            g_pwm_lut[device].compare_preload_fp(g_pwm_lut[device].periph, g_pwm_lut[device].channel);
        }

        if (g_pwm_lut[device].is_dma_request_enabled) {
            g_pwm_lut[device].dma_request_fp(g_pwm_lut[device].periph);
        }
    }

    return g_is_all_device_init;
}

bool PWM_Driver_Enable_Device (const ePwmDevice_t device) {
    if ((device <= ePwmDevice_First) || (device >= ePwmDevice_Last)) {
        return false;
    }

    if (!g_is_all_device_init) {
        return false;
    }

    if (g_is_device_enabled[device]) {
        return true;
    }

    LL_TIM_CC_EnableChannel(g_pwm_lut[device].periph, g_pwm_lut[device].channel);

    if (!Timer_Driver_Start(g_pwm_lut[device].timer)) {
        return false;
    }

    g_is_device_enabled[device] = true;

    return true;
}

bool PWM_Driver_Disable_Device (const ePwmDevice_t device) {
    if ((device <= ePwmDevice_First) || (device >= ePwmDevice_Last)) {
        return false;
    }

    if (!g_is_all_device_init) {
        return false;
    }

    if (!g_is_device_enabled[device]) {
        return true;
    }

    g_pwm_lut[device].compare_value_fp(g_pwm_lut[device].periph, 0);

    if (!Timer_Driver_Stop(g_pwm_lut[device].timer)) {
        return false;
    }

    g_is_device_enabled[device] = false;

    return true;
}

bool PWM_Driver_Change_Duty_Cycle (const ePwmDevice_t device, const size_t value) {
    if ((device <= ePwmDevice_First) || (device >= ePwmDevice_Last)) {
        return false;
    }

    if (!g_is_all_device_init) {
        return false;
    }

    if (!g_is_device_enabled[device]) {
        return false;
    }

    if (value > UINT16_MAX) {
        return false;
    }

    if (!LL_TIM_IsEnabledCounter(g_pwm_lut[device].periph)) {
        return false;
    }

    g_pwm_lut[device].compare_value_fp(g_pwm_lut[device].periph, value);

    return true;
}

uint32_t PWM_Driver_GetRegAddr (const ePwmDevice_t device) {
    if ((device <= ePwmDevice_First) || (device >= ePwmDevice_Last)) {
        return 0;
    }

    switch (g_pwm_lut[device].channel) {
        case LL_TIM_CHANNEL_CH1: {
            return (uint32_t) &g_pwm_lut[device].periph->CCR1;
        }
        case LL_TIM_CHANNEL_CH2: {
            return (uint32_t) &g_pwm_lut[device].periph->CCR2;
        }
        case LL_TIM_CHANNEL_CH3: {
            return (uint32_t) &g_pwm_lut[device].periph->CCR3;
        }
        case LL_TIM_CHANNEL_CH4: {
            return (uint32_t) &g_pwm_lut[device].periph->CCR4;
        }
        default: {
            return 0;
        }
    }
}

uint16_t PWM_Driver_GetDeviceTimerResolution (const ePwmDevice_t device) {
    if ((device <= ePwmDevice_First) || (device >= ePwmDevice_Last)) {
        return 0;
    }

    if (!g_is_all_device_init) {
        return 0;
    }

    return Timer_Driver_GetResolution(g_pwm_lut[device].timer);
}

uint32_t PWM_Driver_GetCompareValue (const ePwmDevice_t device) {
    if ((device <= ePwmDevice_First) || (device >= ePwmDevice_Last)) {
        return 0;
    }

    if (!g_is_all_device_init) {
        return 0;
    }

    if (!g_is_device_enabled[device]) {
        return 0;
    }

    if (!LL_TIM_IsEnabledCounter(g_pwm_lut[device].periph)) {
        return 0;
    }

    uint32_t value = g_pwm_lut[device].get_ccr_fp(g_pwm_lut[device].periph);

    return value;
}

#endif
