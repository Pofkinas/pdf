/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "timer_driver.h"

#if defined(ENABLE_TIMER)

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

static sTimerDesc_t g_timer_lut[eTimer_Last] = {0};
static bool g_is_all_timers_init = false;
static bool g_is_counter_enable[eTimer_Last] = {false};

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

bool Timer_Driver_InitAllTimers (void) {
    if (g_is_all_timers_init) {
        return true;
    }

    g_is_all_timers_init = true;

    LL_TIM_InitTypeDef timer_init_struct = {0};

    for (eTimer_t timer = eTimer_First; timer < eTimer_Last; timer++) {
        const sTimerDesc_t *desc = Timer_Config_GetTimerDesc(timer);

        if (NULL == desc) {
            g_is_all_timers_init = false;
            return false;
        }

        g_timer_lut[timer] = *desc;

        g_timer_lut[timer].enable_clock_fp(g_timer_lut[timer].clock);

        if (g_timer_lut[timer].enable_interupt) {
            NVIC_SetPriority(g_timer_lut[timer].nvic, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
            NVIC_EnableIRQ(g_timer_lut[timer].nvic);
        }

        timer_init_struct.Prescaler = g_timer_lut[timer].prescaler;
        timer_init_struct.CounterMode = g_timer_lut[timer].counter_mode;
        timer_init_struct.Autoreload = g_timer_lut[timer].auto_reload;
        timer_init_struct.ClockDivision = g_timer_lut[timer].clock_division;
        
        if (ERROR == LL_TIM_Init(g_timer_lut[timer].periph, &timer_init_struct)) {
            g_is_all_timers_init = false;
        }
        
        if (NULL != g_timer_lut[timer].clock_source_fp) {
            g_timer_lut[timer].clock_source_fp(g_timer_lut[timer].periph, g_timer_lut[timer].clock_source);
        }

        if (NULL != g_timer_lut[timer].auto_relead_preload_fp) {
            g_timer_lut[timer].auto_relead_preload_fp(g_timer_lut[timer].periph);
        }

        if (NULL != g_timer_lut[timer].master_slave_mode_fp) {
            g_timer_lut[timer].master_slave_mode_fp(g_timer_lut[timer].periph);
        }

        if (NULL != g_timer_lut[timer].set_slave_mode_fp) {
            g_timer_lut[timer].set_slave_mode_fp(g_timer_lut[timer].periph, g_timer_lut[timer].slave_mode);
        }

        if (NULL != g_timer_lut[timer].set_trigger) {
            g_timer_lut[timer].set_trigger(g_timer_lut[timer].periph, g_timer_lut[timer].triger_sync);
        }
    }

    return g_is_all_timers_init;
}

bool Timer_Driver_Start (const eTimer_t timer) {
    if (!Timer_Config_IsCorrectTimer(timer)) {
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

bool Timer_Driver_Stop (const eTimer_t timer) {
    if (!Timer_Config_IsCorrectTimer(timer)) {
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

uint16_t Timer_Driver_GetResolution (const eTimer_t timer) {
    if (!Timer_Config_IsCorrectTimer(timer)) {
        return 0;
    }

    return LL_TIM_GetAutoReload(g_timer_lut[timer].periph);
}

#endif /* ENABLE_TIMER */
