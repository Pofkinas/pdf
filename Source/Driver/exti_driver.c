/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "exti_driver.h"

#ifdef ENABLE_EXTI
#include "exti_config.h"

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

static sExtiDesc_t g_exti_lut[eExti_Last];
static sExtiDynamic_t g_dynamic_exti_lut[eExti_Last] = {0};

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

static void EXTIx_IRQHandler (const eExti_t exti_device);
void EXTI0_IRQHandler(void);
void EXTI1_IRQHandler(void);
void EXTI2_IRQHandler(void);
void EXTI3_IRQHandler(void);
void EXTI4_IRQHandler(void);
void EXTI9_5_IRQHandler(void);
void EXTI15_10_IRQHandler(void);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

static void EXTIx_IRQHandler (const eExti_t exti_device) {
    if (!Exti_Config_IsCorrectExti(exti_device)) {
        return;
    }
    
    if (LL_EXTI_IsActiveFlag_0_31(g_exti_lut[exti_device].line_0_31)) {
        Exti_Driver_ClearFlag(exti_device);

        g_dynamic_exti_lut[exti_device].callback(g_dynamic_exti_lut[exti_device].callback_context);
    }

    return;
}

void EXTI0_IRQHandler(void) {
    #ifdef EXTI0
    EXTIx_IRQHandler(EXTI0);
    #endif /* EXTI0 */

    return;
}

void EXTI1_IRQHandler(void) {
    #ifdef EXTI1
    EXTIx_IRQHandler(EXTI1);
    #endif /* EXTI1 */

    return;
}

void EXTI2_IRQHandler(void) {
    #ifdef EXTI2
    EXTIx_IRQHandler(EXTI2);
    #endif /* EXTI2 */

    return;
}

void EXTI3_IRQHandler(void) {
    #ifdef EXTI3
    EXTIx_IRQHandler(EXTI3);
    #endif /* EXTI3 */

    return;
}

void EXTI4_IRQHandler(void) {
    #ifdef EXTI4
    EXTIx_IRQHandler(EXTI4);
    #endif /* EXTI4 */

    return;
}

void EXTI9_5_IRQHandler(void) {
    #ifdef EXTI9_5
    EXTIx_IRQHandler(EXTI9_5);
    #endif /* EXTI9_5 */

    return;
}

void EXTI15_10_IRQHandler(void) {
    #ifdef EXTI15_10
    EXTIx_IRQHandler(EXTI15_10);
    #endif /* EXTI15_10 */

    return;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool Exti_Driver_InitDevice (const eExti_t exti_device, exti_callback_t exti_callback, void *callback_context) {
    if (!Exti_Config_IsCorrectExti(exti_device)) {
        return false;
    }

    if (exti_callback == NULL) {
        return false;
    }

    if (g_dynamic_exti_lut[exti_device].is_init) {
        return true;
    }

    const sExtiDesc_t *desc = Exti_Config_GetExtiDesc(exti_device);

    if (desc == NULL) {
        return false;
    }

    g_exti_lut[exti_device] = *desc;

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

bool Exti_Driver_Disable_IT (const eExti_t exti_device) {
    if (!Exti_Config_IsCorrectExti(exti_device)) {
        return false;
    }

    if (!g_dynamic_exti_lut[exti_device].is_init) {
        return false;
    }

    LL_EXTI_DisableIT_0_31(g_exti_lut[exti_device].line_0_31);

    g_dynamic_exti_lut[exti_device].is_exti_enabled = false;

    return true;
}

bool Exti_Driver_Enable_IT (const eExti_t exti_device) {
    if (!Exti_Config_IsCorrectExti(exti_device)) {
        return false;
    }

    if (!g_dynamic_exti_lut[exti_device].is_init) {
        return false;
    }

    LL_EXTI_EnableIT_0_31(g_exti_lut[exti_device].line_0_31);

    g_dynamic_exti_lut[exti_device].is_exti_enabled = true;

    return true;
}

bool Exti_Driver_ClearFlag (const eExti_t exti_device) {
    if (!Exti_Config_IsCorrectExti(exti_device)) {
        return false;
    }

    if (!g_dynamic_exti_lut[exti_device].is_init) {
        return false;
    }

    LL_EXTI_ClearFlag_0_31(g_exti_lut[exti_device].line_0_31);

    return true;
}

#endif /* ENABLE_EXTI */
