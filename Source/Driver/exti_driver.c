/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "exti_driver.h"

#ifdef USE_EXTI

#include <stdint.h>

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef struct sExtiDynamic {
    bool is_init;
    bool is_exti_enabled;
    void (*callback) (void *context);
    void *callback_context;
} sExtiDynamic_t;

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static sExtiDesc_t g_exti_lut[eExtiDriver_Last];

/* clang-format off */
static sExtiDynamic_t g_dynamic_exti_lut[eExtiDriver_Last] = {
    #ifdef USE_START_BUTTON
    [eExtiDriver_StartButton] = {
        .is_init = false,
        .is_exti_enabled = false,
        .callback = NULL,
        .callback_context = NULL,
    },
    #endif
    
    #ifdef USE_TCRT5000_RIGHT
    [eExtiDriver_Tcrt5000_Right] = {
        .is_init = false,
        .is_exti_enabled = false,
        .callback = NULL,
        .callback_context = NULL,
    },
    #endif

    #ifdef USE_TCRT5000_LEFT
    [eExtiDriver_Tcrt5000_Left] = {
        .is_init = false,
        .is_exti_enabled = false,
        .callback = NULL,
        .callback_context = NULL,
    },
    #endif
};
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

static void EXTIx_IRQHandler (const IRQn_Type interupt);
void EXTI0_IRQHandler(void);
void EXTI1_IRQHandler(void);
void EXTI9_5_IRQHandler(void);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

static void EXTIx_IRQHandler (const IRQn_Type interupt) {
    for (eExtiDriver_t exti_device = (eExtiDriver_First + 1); exti_device < eExtiDriver_Last; exti_device++) {
        if (g_exti_lut[exti_device].nvic != interupt) {
            continue;
        }

        if (LL_EXTI_IsActiveFlag_0_31(g_exti_lut[exti_device].line_0_31)) {
            Exti_Driver_ClearFlag(exti_device);

            g_dynamic_exti_lut[exti_device].callback(g_dynamic_exti_lut[exti_device].callback_context);
        }
    }

    return;
}

void EXTI0_IRQHandler(void) {
    #ifdef USE_EXTI0
    EXTIx_IRQHandler(EXTI0_IRQn);
    #endif
}

void EXTI1_IRQHandler(void) {
    #ifdef USE_EXTI1
    EXTIx_IRQHandler(EXTI1_IRQn);
    #endif
}

void EXTI2_IRQHandler(void) {
    #ifdef USE_EXTI2
    EXTIx_IRQHandler(EXTI2_IRQn);
    #endif
}

void EXTI3_IRQHandler(void) {
    #ifdef USE_EXTI3
    EXTIx_IRQHandler(EXTI3_IRQn);
    #endif
}

void EXTI4_IRQHandler(void) {
    #ifdef USE_EXTI4
    EXTIx_IRQHandler(EXTI4_IRQn);
    #endif
}

void EXTI9_5_IRQHandler(void) {
    #ifdef USE_EXTI9_5
    EXTIx_IRQHandler(EXTI9_5_IRQn);
    #endif
}

void EXTI15_10_IRQHandler(void) {
    #ifdef USE_EXTI15_10
    EXTIx_IRQHandler(EXTI15_10_IRQn);
    #endif
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

void Exti_Driver_DefinePerips (const sExtiStaticDesc_t *exti_lut) {
    if (exti_lut == NULL) {
        return;
    }

    g_exti_lut = exti_lut;

    return;
}

bool Exti_Driver_InitDevice (const eExtiDriver_t exti_device, exti_callback_t exti_callback, void *callback_context) {
    if (exti_callback == NULL) {
        return false;
    }

    if ((exti_device <= eExtiDriver_First) || (exti_device >= eExtiDriver_Last)) {
        return false;
    }

    if (g_dynamic_exti_lut[exti_device].is_init) {
        return true;
    }

    LL_EXTI_InitTypeDef exti_init_struct = {0};

    LL_SYSCFG_SetEXTISource(g_exti_lut[exti_device].system_port, g_exti_lut[exti_device].system_line);

    exti_init_struct.Line_0_31 = g_exti_lut[exti_device].line_0_31;
    exti_init_struct.LineCommand = g_exti_lut[exti_device].command;
    exti_init_struct.Mode = g_exti_lut[exti_device].mode;
    exti_init_struct.Trigger = g_exti_lut[exti_device].trigger;

    if (LL_EXTI_Init(&exti_init_struct) == ERROR) {
        return false;
    }

    NVIC_SetPriority(g_exti_lut[exti_device].nvic, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));

    NVIC_EnableIRQ(g_exti_lut[exti_device].nvic);

    g_dynamic_exti_lut[exti_device].is_init = true;
    g_dynamic_exti_lut[exti_device].callback = exti_callback;
    g_dynamic_exti_lut[exti_device].callback_context = callback_context;

    return true;
}

bool Exti_Driver_Disable_IT (const eExtiDriver_t exti_device) {
    if ((exti_device <= eExtiDriver_First) || (exti_device >= eExtiDriver_Last)) {
        return false;
    }

    LL_EXTI_DisableIT_0_31(g_exti_lut[exti_device].line_0_31);

    g_dynamic_exti_lut[exti_device].is_exti_enabled = false;

    return true;
}

bool Exti_Driver_Enable_IT (const eExtiDriver_t exti_device) {
    if ((exti_device <= eExtiDriver_First) || (exti_device >= eExtiDriver_Last)) {
        return false;
    }

    LL_EXTI_EnableIT_0_31(g_exti_lut[exti_device].line_0_31);

    g_dynamic_exti_lut[exti_device].is_exti_enabled = true;

    return true;
}

bool Exti_Driver_ClearFlag (const eExtiDriver_t exti_device) {
    if ((exti_device <= eExtiDriver_First) || (exti_device >= eExtiDriver_Last)) {
        return false;
    }

    LL_EXTI_ClearFlag_0_31(g_exti_lut[exti_device].line_0_31);

    return true;
}

#endif
