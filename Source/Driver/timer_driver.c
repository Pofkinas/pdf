/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

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

static sTimerDesc_t g_timer_lut[eTimerDriver_Last];
static bool g_is_all_timers_init = false;

/* clang-format off */
static bool g_is_counter_enable[eTimerDriver_Last] = {
    [eTimerDriver_TIM10] = false,

    #if defined(USE_MOTOR_A) || defined(USE_MOTOR_B) || defined(USE_PULSE_LED)
    [eTimerDriver_TIM3] = false,
    #endif

    #if defined(USE_WS2812B_1) || defined(USE_WS2812B_2)
    [eTimerDriver_TIM5] = false,
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

void Timer_Driver_DefineTimers (const sTimerDesc_t *timer_lut) {
    if (timer_lut == NULL) {
        return;
    }

    g_timer_lut = timer_lut;

    g_is_all_timers_init = false;

    return;
}

bool Timer_Driver_InitAllTimers (void) {
    if (g_is_all_timers_init) {
        return true;
    }

    if (eTimerDriver_Last == 1) {
        return false;
    }

    g_is_all_timers_init = true;

    LL_TIM_InitTypeDef timer_init_struct = {0};

    for (eTimerDriver_t timer = (eTimerDriver_First + 1); timer < eTimerDriver_Last; timer++) {
        g_timer_lut[timer].enable_clock_fp(g_timer_lut[timer].clock);

        if (g_timer_lut[timer].enable_interupt) {
            NVIC_SetPriority(g_timer_lut[timer].nvic, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
            NVIC_EnableIRQ(g_timer_lut[timer].nvic);
        }

        timer_init_struct.Prescaler = g_timer_lut[timer].prescaler;
        timer_init_struct.CounterMode = g_timer_lut[timer].counter_mode;
        timer_init_struct.Autoreload = g_timer_lut[timer].auto_reload;
        timer_init_struct.ClockDivision = g_timer_lut[timer].clock_division;
        
        if (LL_TIM_Init(g_timer_lut[timer].periph, &timer_init_struct) == ERROR) {
            g_is_all_timers_init = false;
        }
        
        if (g_timer_lut[timer].clock_source_fp != NULL) {
            g_timer_lut[timer].clock_source_fp(g_timer_lut[timer].periph, g_timer_lut[timer].clock_source);
        }

        if (g_timer_lut[timer].auto_relead_preload_fp != NULL) {
            g_timer_lut[timer].auto_relead_preload_fp(g_timer_lut[timer].periph);
        }

        if (g_timer_lut[timer].master_slave_mode_fp != NULL) {
            g_timer_lut[timer].master_slave_mode_fp(g_timer_lut[timer].periph);
        }

        if (g_timer_lut[timer].set_slave_mode_fp != NULL) {
            g_timer_lut[timer].set_slave_mode_fp(g_timer_lut[timer].periph, g_timer_lut[timer].slave_mode);
        }

        if (g_timer_lut[timer].set_trigger != NULL) {
            g_timer_lut[timer].set_trigger(g_timer_lut[timer].periph, g_timer_lut[timer].triger_sync);
        }
    }

    return g_is_all_timers_init;
}

bool Timer_Driver_Start (const eTimerDriver_t timer) {
    if ((timer <= eTimerDriver_First) || (timer >= eTimerDriver_Last)) {
        return false;
    }

    if (!g_is_all_timers_init) {
        return false;
    }

    if (!g_is_counter_enable[timer]) {
        LL_TIM_EnableCounter(g_timer_lut[timer].periph);

        g_is_counter_enable[timer] = true;
    }

    return true;
}

bool Timer_Driver_Stop (const eTimerDriver_t timer) {
    if ((timer <= eTimerDriver_First) || (timer >= eTimerDriver_Last)) {
        return false;
    }

    if (!g_is_all_timers_init) {
        return false;
    }

    if (g_is_counter_enable[timer]) {
        LL_TIM_DisableCounter(g_timer_lut[timer].periph);

        LL_TIM_SetCounter(g_timer_lut[timer].periph, 0);

        g_is_counter_enable[timer] = false;
    }

    return true;
}

uint16_t Timer_Driver_GetResolution (const eTimerDriver_t timer) {
    if ((timer <= eTimerDriver_First) || (timer >= eTimerDriver_Last)) {
        return 0;
    }

    return LL_TIM_GetAutoReload(g_timer_lut[timer].periph);
}
