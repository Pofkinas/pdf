/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "led_api.h"

#if defined(ENABLE_LED) || defined(ENABLE_PWM_LED)
#include "cmsis_os2.h"
#include "gpio_driver.h"
#include "pwm_driver.h"
#include "timer_driver.h"
#include "debug_api.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

#define BLINK_MUTEX_TIMEOUT 0U
#define PULSE_MUTEX_TIMEOUT 0U

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef struct sLedBlinkDesc {
    eLed_t led;
    osTimerId_t blink_timer;
    osMutexId_t blink_mutex;
    bool is_running;
    uint16_t total_blinks;
    uint16_t blink_count;
} sLedBlinkDesc_t; 

typedef struct sLedPulseDesc {
    eLedPwm_t led;
    osTimerId_t pulse_timer;
    osMutexId_t pulse_mutex;
    bool is_running;
    bool count_dir_up;
    uint16_t timer_resolution;
    uint16_t duty_cycle_change;
    uint16_t total_pulses;
    uint16_t total_changes_per_pulse;
    uint16_t current_duty_cycle;
    uint16_t pulse_count;
    uint16_t change_count;
} sLedPulseDesc_t;

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

#ifdef DEBUG_LED_API
CREATE_MODULE_NAME (LED_API)
#else
CREATE_MODULE_NAME_EMPTY
#endif /* DEBUG_LED_API */

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

#ifdef ENABLE_LED
static void LED_API_Blink_Timer_Callback (void *arg);
#endif /* ENABLE_LED */

#ifdef ENABLE_PWM_LED
static void LED_API_Pulse_Timer_Callback (void *arg);
#endif /* ENABLE_PWM_LED */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

#ifdef ENABLE_LED
static bool g_is_led_initialized = false;
#endif

#ifdef ENABLE_PWM_LED
static bool g_is_pwm_initialized = false;
#endif

osTimerId_t g_blink_timer = NULL;
uint16_t g_blink_count = 0;

#ifdef ENABLE_LED
static sLedBlinkDesc_t g_led_blink_lut[eLed_Last] = {0};
static sLedDesc_t g_led_desc_lut[eLed_Last] = {0};
#endif /* ENABLE_LED */

#ifdef ENABLE_PWM_LED
static sLedPulseDesc_t g_led_pulse_lut[eLedPwm_Last] = {0};
static sLedPwmDesc_t g_pwm_led_desc_lut[eLedPwm_Last] = {0};
#endif /* ENABLE_PWM_LED */

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

#ifdef ENABLE_LED
static void LED_API_Blink_Timer_Callback (void *arg) {
    sLedBlinkDesc_t *led_blink_desc = (sLedBlinkDesc_t*) arg;

    if (!led_blink_desc->is_running) {
        if (osMutexAcquire(led_blink_desc->blink_mutex, BLINK_MUTEX_TIMEOUT) != osOK) {
            return;
        }
    }

    led_blink_desc->is_running = true;

    osMutexRelease(led_blink_desc->blink_mutex);

    LED_API_Toggle(led_blink_desc->led);

    led_blink_desc->blink_count++;

    if (led_blink_desc->blink_count >= led_blink_desc->total_blinks){
        osTimerStop(led_blink_desc->blink_timer);
        
        LED_API_TurnOff(led_blink_desc->led);
        
        led_blink_desc->is_running = false;
    }

    return;
}
#endif /* ENABLE_LED */

#ifdef ENABLE_PWM_LED
static void LED_API_Pulse_Timer_Callback (void *arg) {
   sLedPulseDesc_t *led_pulse_desc = (sLedPulseDesc_t*) arg;

   if (!led_pulse_desc->is_running) {
       if (osMutexAcquire(led_pulse_desc->pulse_mutex, PULSE_MUTEX_TIMEOUT) != osOK) {
           return;
       }
   }

   led_pulse_desc->is_running = true;

   osMutexRelease(led_pulse_desc->pulse_mutex);

   PWM_Driver_Change_Duty_Cycle(g_pwm_led_desc_lut[led_pulse_desc->led].pwm_device, led_pulse_desc->current_duty_cycle);

   if (led_pulse_desc->change_count >= led_pulse_desc->total_changes_per_pulse) {
       if (led_pulse_desc->count_dir_up) {
           led_pulse_desc->count_dir_up = false;
           led_pulse_desc->change_count = 0;
       } else {
           led_pulse_desc->count_dir_up = true;
           led_pulse_desc->change_count = 0;
           led_pulse_desc->pulse_count ++;
       }
   }

   if (led_pulse_desc->count_dir_up) {
       led_pulse_desc->current_duty_cycle += led_pulse_desc->duty_cycle_change;
   } else {
       led_pulse_desc->current_duty_cycle -= led_pulse_desc->duty_cycle_change;
   }

   led_pulse_desc->change_count++;

   if (led_pulse_desc->pulse_count >= led_pulse_desc->total_pulses) {
       osTimerStop(led_pulse_desc->pulse_timer);

       PWM_Driver_Change_Duty_Cycle(g_pwm_led_desc_lut[led_pulse_desc->led].pwm_device, 0);

       led_pulse_desc->is_running = false;
   }

   return;
}
#endif /* ENABLE_PWM_LED */

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool LED_API_Init (void) {
    if (g_is_led_initialized || g_is_pwm_initialized) {
        return true;
    }

    if (!GPIO_Driver_InitAllPins()) {
        TRACE_ERR("Init: Failed to initialize GPIO pins\n");
        
        return false;
    }

    #ifdef ENABLE_PWM_LED
    if (!PWM_Driver_InitAllDevices()) {
        TRACE_ERR("Init: Failed to initialize PWM devices\n");

        return false;
    }

    g_is_pwm_initialized = true;
    #endif /* ENABLE_PWM_LED */

    g_is_led_initialized = true;

    #ifdef ENABLE_LED
    for (eLed_t led = eLed_First; led < eLed_Last; led++) {
        const sLedDesc_t *desc = LED_Config_GetLedDesc(led);

        if (desc == NULL) {
            TRACE_ERR("Init: Failed to get LED %d description\n", led);
            
            return false;
        }

        g_led_desc_lut[led] = *desc;
        
        g_led_blink_lut[led].blink_timer = osTimerNew(LED_API_Blink_Timer_Callback, osTimerPeriodic, &g_led_blink_lut[led], &g_led_desc_lut[led].blink_timer_attributes);
        
        if (g_led_blink_lut[led].blink_timer == NULL) {
            TRACE_ERR("Init: Failed to create blink timer for LED %d\n", led);
            
            return false;
        }

        g_led_blink_lut[led].blink_mutex = osMutexNew(&g_led_desc_lut[led].blink_mutex_attributes);

        if (g_led_blink_lut[led].blink_mutex == NULL) {
            TRACE_ERR("Init: Failed to create blink mutex for LED %d\n", led);
            
            return false;
        }

        g_led_blink_lut[led].led = led;
    }
    #endif /* ENABLE_LED */

    #ifdef ENABLE_PWM_LED
    for (eLedPwm_t led = eLedPwm_First; led < eLedPwm_Last; led++) {
        const sLedPwmDesc_t *desc = LED_Config_GetPwmLedDesc(led);

        if (desc == NULL) {
            TRACE_ERR("Init: Failed to get PWM LED %d description\n", led);
            
            return false;
        }

        g_pwm_led_desc_lut[led] = *desc;

        g_led_pulse_lut[led].pulse_timer = osTimerNew(LED_API_Pulse_Timer_Callback, osTimerPeriodic, &g_led_pulse_lut[led], &g_pwm_led_desc_lut[led].pulse_timer_attributes);
        
        if (g_led_pulse_lut[led].pulse_timer == NULL) {
            TRACE_ERR("Init: Failed to create pulse timer for PWM LED %d\n", led);
            
            return false;
        }

        g_led_pulse_lut[led].pulse_mutex = osMutexNew(&g_pwm_led_desc_lut[led].pulse_mutex_attributes);

        if (g_led_pulse_lut[led].pulse_mutex == NULL) {
            TRACE_ERR("Init: Failed to create pulse mutex for PWM LED %d\n", led);
            
            return false;
        }

        if (!PWM_Driver_Enable_Device(g_pwm_led_desc_lut[led].pwm_device)) {
            TRACE_ERR("Init: Failed to enable PWM device for LED %d\n", led);
            
            return false;
        }

        g_led_pulse_lut[led].led = led;
        g_led_pulse_lut[led].timer_resolution = PWM_Driver_GetDeviceTimerResolution(g_pwm_led_desc_lut[led].pwm_device);
    }
    #endif /* ENABLE_PWM_LED */

    return true;
}

#ifdef ENABLE_LED
bool LED_API_TurnOn (const eLed_t led) {
    if (!g_is_led_initialized) {
        TRACE_ERR("TurnOn: LED not initialized\n");
        
        return false;
    }
    
    if (!LED_Config_IsCorrectLed(led)) {
        TRACE_ERR("TurnOn: Incorrect LED type %d\n", led);
        
        return false;
    }

    return GPIO_Driver_WritePin(g_led_desc_lut[led].led_pin, !g_led_desc_lut[led].is_inverted);
}

bool LED_API_TurnOff (const eLed_t led) {
    if (!g_is_led_initialized) {
        TRACE_ERR("TurnOff: LED not initialized\n");
        
        return false;
    }
    
    if (!LED_Config_IsCorrectLed(led)) {
        TRACE_ERR("TurnOff: Incorrect LED type %d\n", led);
        
        return false;
    }
    
    return GPIO_Driver_WritePin(g_led_desc_lut[led].led_pin, g_led_desc_lut[led].is_inverted);
}

bool LED_API_Toggle (const eLed_t led) {
    if (!g_is_led_initialized) {
        TRACE_ERR("Toggle: LED not initialized\n");
        
        return false;
    }
    
    if (!LED_Config_IsCorrectLed(led)) {
        TRACE_ERR("Toggle: Incorrect LED type %d\n", led);
        
        return false;
    }

    return GPIO_Driver_TogglePin(g_led_desc_lut[led].led_pin);
}

bool LED_API_Blink (const eLed_t led, const uint8_t blink_time, const uint16_t blink_frequency) {
    if (!g_is_led_initialized) {
        TRACE_ERR("Blink: LED not initialized\n");
        
        return false;
    }

    if (!LED_Config_IsCorrectLed(led)) {
        TRACE_ERR("Blink: Incorrect LED type %d\n", led);
        
        return false;
    }

    if (!LED_API_IsCorrectBlinkTime(blink_time)) {
        TRACE_ERR("Blink: Incorrect blink time %d\n", blink_time);
        
        return false;
    }

    if (!LED_API_IsCorrectBlinkFrequency(blink_frequency)) {
        TRACE_ERR("Blink: Incorrect blink frequency %d\n", blink_frequency);
        
        return false;
    }

    if (g_led_blink_lut[led].is_running) {
        return true;
    }

    if (osMutexAcquire(g_led_blink_lut[led].blink_mutex, BLINK_MUTEX_TIMEOUT) != osOK) {
        TRACE_ERR("Blink: Failed to acquire blink mutex for LED %d\n", led);
        
        return false;
    }

    g_led_blink_lut[led].total_blinks = (blink_time * 1000 / blink_frequency) * 2;
    g_led_blink_lut[led].blink_count = 0;

    osTimerStart(g_led_blink_lut[led].blink_timer, (blink_frequency / 2));

    osMutexRelease(g_led_blink_lut[led].blink_mutex);

    return true;
}
#endif /* ENABLE_LED */

#ifdef ENABLE_PWM_LED
bool LED_API_Set_Brightness (const eLedPwm_t led, const uint8_t brightness) {
    if (!g_is_pwm_initialized) {
        TRACE_ERR("Set_Brightness: PWM LED not initialized\n");
        
        return false;
    }

    if (!LED_Config_IsCorrectPwmLed(led)) {
        TRACE_ERR("Set_Brightness: Incorrect PWM LED type %d\n", led);
        
        return false;
    }

    if (!LED_API_IsCorrectDutyCycle(led, brightness)) {
        TRACE_ERR("Set_Brightness: Incorrect brightness %d for LED %d\n", brightness, led);
        
        return false;
    }

    return PWM_Driver_Change_Duty_Cycle(g_pwm_led_desc_lut[led].pwm_device, brightness);
}

bool LED_API_Pulse (const eLedPwm_t led, const uint8_t pulsing_time, const uint16_t pulse_frequency) {
    if (!g_is_pwm_initialized) {
        TRACE_ERR("Pulse: PWM LED not initialized\n");
        
        return false;
    }

    if (!LED_Config_IsCorrectPwmLed(led)) {
        TRACE_ERR("Pulse: Incorrect PWM LED type %d\n", led);
        
        return false;
    }

    if (!LED_API_IsCorrectPulseTime(pulsing_time)) {
        TRACE_ERR("Pulse: Incorrect pulse time %d\n", pulsing_time);
        
        return false;
    }

    if (!LED_API_IsCorrectPulseFrequency(pulse_frequency)) {
        TRACE_ERR("Pulse: Incorrect pulse frequency %d\n", pulse_frequency);
        
        return false;
    }

    if (g_led_pulse_lut[led].is_running) {
        return false;
    }

    if (osMutexAcquire(g_led_pulse_lut[led].pulse_mutex, PULSE_MUTEX_TIMEOUT) != osOK) {
        TRACE_ERR("Pulse: Failed to acquire pulse mutex for LED %d\n", led);
        
        return false;
    }

    g_led_pulse_lut[led].total_pulses = (pulsing_time * 1000 / pulse_frequency);
    g_led_pulse_lut[led].pulse_count = 0;

    g_led_pulse_lut[led].total_changes_per_pulse = pulse_frequency / 2; 
    g_led_pulse_lut[led].duty_cycle_change = g_led_pulse_lut[led].timer_resolution / g_led_pulse_lut[led].total_changes_per_pulse;
    
    g_led_pulse_lut[led].change_count = 0;
    g_led_pulse_lut[led].current_duty_cycle = g_led_pulse_lut[led].duty_cycle_change;
    g_led_pulse_lut[led].count_dir_up = true;

    osTimerStart(g_led_pulse_lut[led].pulse_timer, PULSE_TIMER_FREQUENCY);

    osMutexRelease(g_led_pulse_lut[led].pulse_mutex);

    return true;
}
#endif /* ENABLE_PWM_LED */

#ifdef ENABLE_LED
bool LED_API_IsCorrectBlinkTime (const uint8_t blink_time) {
    return (blink_time <= MAX_BLINK_TIME) && (blink_time > 0);
}

bool LED_API_IsCorrectBlinkFrequency (const uint16_t blink_frequency) {
    return (blink_frequency <= MAX_BLINK_FREQUENCY) && (blink_frequency >= MIN_BLINK_FREQUENCY);
}
#endif /* ENABLE_LED */

#ifdef ENABLE_PWM_LED
bool LED_API_IsCorrectDutyCycle (const eLedPwm_t led, const uint8_t duty_cycle) {
    return (duty_cycle >= 0) && (duty_cycle <= g_led_pulse_lut[led].timer_resolution);
}

bool LED_API_IsCorrectPulseTime (const uint8_t pulse_time) {
    return (pulse_time <= MAX_PULSING_TIME) && (pulse_time > 0);
}

bool LED_API_IsCorrectPulseFrequency (const uint16_t pulse_frequency) {
    return (pulse_frequency <= MAX_PULSE_FREQUENCY) && (pulse_frequency > MIN_PULSE_FREQUENCY);
}
#endif /* ENABLE_PWM_LED */

#endif /* ENABLE_LED || ENABLE_PWM_LED */
