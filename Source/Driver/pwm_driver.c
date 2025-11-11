/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "pwm_driver.h"

#ifdef ENABLE_PWM
#include "timer_driver.h"

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

static sPwmOcDesc_t g_oc_pwm_lut[ePwm_Last] = {0};
static bool g_is_all_device_init = false;
static bool g_is_device_enabled[ePwm_Last] = {false};

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

bool PWM_Driver_InitAllDevices (void) {
    if (g_is_all_device_init) {
        return true;
    }

    g_is_all_device_init = true;
    
    LL_TIM_OC_InitTypeDef channel_oc_init_struct = {0};

    for (ePwm_t device = ePwm_First; device < ePwm_Last; device++) {
        const sPwmOcDesc_t *desc = PWM_Config_GetPwmOcDesc(device);
        
        if (desc == NULL) {
            g_is_all_device_init = false;
            return false;
        }

        g_oc_pwm_lut[device] = *desc;

        channel_oc_init_struct.OCMode = g_oc_pwm_lut[device].mode;
        channel_oc_init_struct.OCState = g_oc_pwm_lut[device].oc_state;
        channel_oc_init_struct.OCNState = g_oc_pwm_lut[device].ocn_state;
        channel_oc_init_struct.CompareValue = g_oc_pwm_lut[device].compare_value;
        channel_oc_init_struct.OCPolarity = g_oc_pwm_lut[device].oc_polarity;
        channel_oc_init_struct.OCNPolarity = g_oc_pwm_lut[device].ocn_polarity;
        channel_oc_init_struct.OCIdleState = g_oc_pwm_lut[device].oc_idle;
        channel_oc_init_struct.OCNIdleState = g_oc_pwm_lut[device].ocn_idle;

        if (LL_TIM_OC_Init(g_oc_pwm_lut[device].periph, g_oc_pwm_lut[device].channel, &channel_oc_init_struct) == ERROR) {
            g_is_all_device_init = false;
        }
    
        if (g_oc_pwm_lut[device].fast_mode_fp != NULL) {
            g_oc_pwm_lut[device].fast_mode_fp(g_oc_pwm_lut[device].periph, g_oc_pwm_lut[device].channel);
        }
    
        if (g_oc_pwm_lut[device].compare_preload_fp != NULL) {
            g_oc_pwm_lut[device].compare_preload_fp(g_oc_pwm_lut[device].periph, g_oc_pwm_lut[device].channel);
        }

        if (g_oc_pwm_lut[device].is_dma_request_enabled) {
            g_oc_pwm_lut[device].dma_request_fp(g_oc_pwm_lut[device].periph);
        }
    }

    return g_is_all_device_init;
}

bool PWM_Driver_Enable_Device (const ePwm_t device) {
    if (!PWM_Config_IsCorrectPwm(device)) {
        return false;
    }

    if (!g_is_all_device_init) {
        return false;
    }

    if (g_is_device_enabled[device]) {
        return true;
    }

    LL_TIM_CC_EnableChannel(g_oc_pwm_lut[device].periph, g_oc_pwm_lut[device].channel);

    if (!Timer_Driver_Start(g_oc_pwm_lut[device].timer)) {
        return false;
    }

    g_is_device_enabled[device] = true;

    return true;
}

bool PWM_Driver_Disable_Device (const ePwm_t device) {
    if (!PWM_Config_IsCorrectPwm(device)) {
        return false;
    }

    if (!g_is_all_device_init) {
        return false;
    }

    if (!g_is_device_enabled[device]) {
        return true;
    }

    g_oc_pwm_lut[device].compare_value_fp(g_oc_pwm_lut[device].periph, 0);

    if (!Timer_Driver_Stop(g_oc_pwm_lut[device].timer)) {
        return false;
    }

    g_is_device_enabled[device] = false;

    return true;
}

bool PWM_Driver_Change_Duty_Cycle (const ePwm_t device, const size_t value) {
    if (!PWM_Config_IsCorrectPwm(device)) {
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

    if (!LL_TIM_IsEnabledCounter(g_oc_pwm_lut[device].periph)) {
        return false;
    }

    g_oc_pwm_lut[device].compare_value_fp(g_oc_pwm_lut[device].periph, value);

    return true;
}

uint32_t PWM_Driver_GetRegAddr (const ePwm_t device) {
    if (!PWM_Config_IsCorrectPwm(device)) {
        return 0;
    }

    switch (g_oc_pwm_lut[device].channel) {
        case LL_TIM_CHANNEL_CH1: {
            return (uint32_t) &g_oc_pwm_lut[device].periph->CCR1;
        }
        case LL_TIM_CHANNEL_CH2: {
            return (uint32_t) &g_oc_pwm_lut[device].periph->CCR2;
        }
        case LL_TIM_CHANNEL_CH3: {
            return (uint32_t) &g_oc_pwm_lut[device].periph->CCR3;
        }
        case LL_TIM_CHANNEL_CH4: {
            return (uint32_t) &g_oc_pwm_lut[device].periph->CCR4;
        }
        default: {
            return 0;
        }
    }
}

uint16_t PWM_Driver_GetDeviceTimerResolution (const ePwm_t device) {
    if (!PWM_Config_IsCorrectPwm(device)) {
        return 0;
    }

    if (!g_is_all_device_init) {
        return 0;
    }

    return Timer_Driver_GetResolution(g_oc_pwm_lut[device].timer);
}

uint32_t PWM_Driver_GetCompareValue (const ePwm_t device) {
    if (!PWM_Config_IsCorrectPwm(device)) {
        return 0;
    }

    if (!g_is_all_device_init) {
        return 0;
    }

    if (!g_is_device_enabled[device]) {
        return 0;
    }

    if (!LL_TIM_IsEnabledCounter(g_oc_pwm_lut[device].periph)) {
        return 0;
    }

    uint32_t value = g_oc_pwm_lut[device].get_ccr_fp(g_oc_pwm_lut[device].periph);

    return value;
}

#endif /* ENABLE_PWM */
