/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "cli_cmd_handlers.h"

#ifdef ENABLE_CLI

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "led_app.h"
#include "motor_app.h"
#include "cmd_api_helper.h"
#include "heap_api.h"
#include "led_api.h"
#include "motor_api.h"
#include "debug_api.h"
#include "error_messages.h"
#include "led_color.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

#define DEBUG_CLI_APP

#define CMD_SEPARATOR ","
#define CMD_SEPARATOR_LENGHT (sizeof(CMD_SEPARATOR) - 1)

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

#ifdef DEBUG_CLI_APP
CREATE_MODULE_NAME (CLI_CMD_HANDLERS)
#else
CREATE_MODULE_NAME_EMPTY
#endif

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

static bool CLI_APP_Led_Handlers_Common (sMessage_t arguments, sMessage_t *response, const eLedTask_t task);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

static bool CLI_APP_Led_Handlers_Common (sMessage_t arguments, sMessage_t *response, const eLedTask_t task) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return false;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return false;
    }
    
    eLed_t led;
    size_t led_value = 0;

    if (CMD_API_Helper_FindNextArgUInt(&arguments, &led_value, CMD_SEPARATOR, CMD_SEPARATOR_LENGHT, response) != eErrorCode_OSOK) {
        return false;
    }

    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return false;
    }

    led = led_value;

    if (!LED_API_IsCorrectLed(led)) {
        snprintf(response->data, response->size, "%d: Incorrect led\n", led);

        return false;
    }

    sLedCommandDesc_t formated_task = {.task = task, .data = NULL};
    sLedCommon_t *task_data = Heap_API_Calloc(1, sizeof(sLedCommon_t));

    if (task_data == NULL) {
        snprintf(response->data, response->size, "Failed Calloc\n");
        
        return false;
    }

    task_data->led = led;
    formated_task.data = task_data;

    if (!LED_APP_Add_Task(&formated_task)) {
        snprintf(response->data, response->size, "Failed task add\n");
        
        Heap_API_Free(task_data);

        return false;
    }

    snprintf(response->data, response->size, "Operation successful\n");

    return true;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

#ifdef USE_LED
bool CLI_APP_Led_Handlers_Set (sMessage_t arguments, sMessage_t *response) {
    eLedTask_t task = eLedTask_Set;

    return CLI_APP_Led_Handlers_Common(arguments, response, task);
}

bool CLI_APP_Led_Handlers_Reset (sMessage_t arguments, sMessage_t *response) {
    eLedTask_t task = eLedTask_Reset;

    return CLI_APP_Led_Handlers_Common(arguments, response, task);
}

bool CLI_APP_Led_Handlers_Toggle (sMessage_t arguments, sMessage_t *response) {
    eLedTask_t task = eLedTask_Toggle;

    return CLI_APP_Led_Handlers_Common(arguments, response, task);
}

bool CLI_APP_Led_Handlers_Blink (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return false;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return false;
    }
    
    eLed_t led;
    size_t led_value = 0;
    size_t blink_time = 0;
    size_t blink_frequency = 0;

    if (CMD_API_Helper_FindNextArgUInt(&arguments, &led_value, CMD_SEPARATOR, CMD_SEPARATOR_LENGHT, response) != eErrorCode_OSOK) {
        return false;
    }

    if (CMD_API_Helper_FindNextArgUInt(&arguments, &blink_time, CMD_SEPARATOR, CMD_SEPARATOR_LENGHT, response) != eErrorCode_OSOK) {
        return false;
    }

    if (CMD_API_Helper_FindNextArgUInt(&arguments, &blink_frequency, CMD_SEPARATOR, CMD_SEPARATOR_LENGHT, response) != eErrorCode_OSOK) {
        return false;
    }
    
    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return false;
    }

    led = led_value;

    if (!LED_API_IsCorrectLed(led)) {
        snprintf(response->data, response->size, "%d: Incorrect led\n", led);

        return false;
    }

    if (!LED_API_IsCorrectBlinkTime(blink_time)) {
        snprintf(response->data, response->size, "%d: Incorrect blink time\n", blink_time);

        return false;
    }

    if (!LED_API_IsCorrectBlinkFrequency(blink_frequency)) {
        snprintf(response->data, response->size, "%d: Incorrect blink frequency\n", blink_frequency);

        return false;
    }

    sLedCommandDesc_t formated_task = {.task = eLedTask_Blink, .data = NULL};
    sLedBlink_t *task_data = Heap_API_Calloc(1, sizeof(sLedBlink_t));

    if (task_data == NULL) {
        snprintf(response->data, response->size, "Failed Calloc\n");
        
        return false;
    }

    task_data->led = led;
    task_data->blink_time = blink_time;
    task_data->blink_frequency = blink_frequency;
    formated_task.data = task_data;

    if (!LED_APP_Add_Task(&formated_task)) {
        snprintf(response->data, response->size, "Failed task add\n");
        
        Heap_API_Free(task_data);

        return false;
    }

    snprintf(response->data, response->size, "Operation successful\n");

    return true;
}
#endif

#ifdef USE_PWM_LED
bool CLI_APP_Pwm_Led_Handlers_Set_Brightness (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return false;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return false;
    }

    eLedPwm_t led;
    size_t led_value = 0;
    size_t duty_cycle = 0;

    if (CMD_API_Helper_FindNextArgUInt(&arguments, &led_value, CMD_SEPARATOR, CMD_SEPARATOR_LENGHT, response) != eErrorCode_OSOK) {
        return false;
    }
    
    if (CMD_API_Helper_FindNextArgUInt(&arguments, &duty_cycle, CMD_SEPARATOR, CMD_SEPARATOR_LENGHT, response) != eErrorCode_OSOK) {
        return false;
    }

    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return false;
    }

    led = led_value;

    if (!LED_API_IsCorrectPwmLed(led)) {
        snprintf(response->data, response->size, "%d: Incorrect led\n", led);

        return false;
    }

    if (!LED_API_IsCorrectDutyCycle(led, duty_cycle)) {
        snprintf(response->data, response->size, "%d: Incorrect duty cycle\n", led);

        return false;
    }

    sLedCommandDesc_t formated_task = {.task = eLedTask_Set_Brightness, .data = NULL};
    sLedSetBrightness_t *task_data = Heap_API_Calloc(1, sizeof(sLedSetBrightness_t));

    if (task_data == NULL) {
        snprintf(response->data, response->size, "Failed Calloc\n");
        
        return false;
    }

    task_data->led = led;
    task_data->duty_cycle = duty_cycle;
    formated_task.data = task_data;

    if (!LED_APP_Add_Task(&formated_task)) {
        snprintf(response->data, response->size, "Failed task add\n");
        
        Heap_API_Free(task_data);

        return false;
    }

    snprintf(response->data, response->size, "Operation successful\n");

    return true;
}

bool CLI_APP_Pwm_Led_Handlers_Pulse (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return false;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return false;
    }

    eLedPwm_t led;
    size_t led_value = 0;
    size_t pulse_time = 0;
    size_t pulse_frequency = 0;

    if (CMD_API_Helper_FindNextArgUInt(&arguments, &led_value, CMD_SEPARATOR, CMD_SEPARATOR_LENGHT, response) != eErrorCode_OSOK) {
        return false;
    }
    
    if (CMD_API_Helper_FindNextArgUInt(&arguments, &pulse_time, CMD_SEPARATOR, CMD_SEPARATOR_LENGHT, response) != eErrorCode_OSOK) {
        return false;
    }

    if (CMD_API_Helper_FindNextArgUInt(&arguments, &pulse_frequency, CMD_SEPARATOR, CMD_SEPARATOR_LENGHT, response) != eErrorCode_OSOK) {
        return false;
    }

    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return false;
    }

    led = led_value;

    if (!LED_API_IsCorrectPwmLed(led)) {
        snprintf(response->data, response->size, "%d: Incorrect led\n", led);

        return false;
    }

    if (!LED_API_IsCorrectPulseTime(pulse_time)) {
        snprintf(response->data, response->size, "%d: Incorrect pulse time\n", led);

        return false;
    }

    if (!LED_API_IsCorrectPulseFrequency(pulse_frequency)) {
        snprintf(response->data, response->size, "%d: Incorrect pulse frequency\n", led);

        return false;
    }

    sLedCommandDesc_t formated_task = {.task = eLedTask_Pulse, .data = NULL};
    sLedPulse_t *task_data = Heap_API_Calloc(1, sizeof(sLedPulse_t));

    if (task_data == NULL) {
        snprintf(response->data, response->size, "Failed Calloc\n");
        
        return false;
    }

    task_data->led = led;
    task_data->pulse_time = pulse_time;
    task_data->pulse_frequency = pulse_frequency;
    formated_task.data = task_data;

    if (!LED_APP_Add_Task(&formated_task)) {
        snprintf(response->data, response->size, "Failed task add\n");
        
        Heap_API_Free(task_data);

        return false;
    }

    snprintf(response->data, response->size, "Operation successful\n");

    return true;
}
#endif

#ifdef USE_MOTORS
bool CLI_APP_Motors_Handlers_Stop (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return false;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return false;
    }

    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return false;
    }

    sMotorCommandDesc_t formated_task = {.task = eMotorTask_Stop, .data = NULL};

    if (!Motor_APP_Add_Task(&formated_task)) {
        snprintf(response->data, response->size, "Failed task add\n");

        return false;
    }

    snprintf(response->data, response->size, "Operation successful\n");

    return true;
}

bool CLI_APP_Motors_Handlers_Set (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return false;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return false;
    }

    eMotorDirection_t direction;
    eMotorControl_t mode;
    size_t speed = 0;
    size_t direction_value = 0;
    size_t mode_value = 0;
    eErrorCode_t error = eErrorCode_OSOK;

    if (CMD_API_Helper_FindNextArgUInt(&arguments, &speed, CMD_SEPARATOR, CMD_SEPARATOR_LENGHT, response) != eErrorCode_OSOK) {
        return false;
    }

    error = CMD_API_Helper_FindNextArgUInt(&arguments, &direction_value, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);

    if (error != eErrorCode_OSOK) {
        return error;
    }

    error = CMD_API_Helper_FindNextArgUInt(&arguments, &mode_value, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);

    if (error != eErrorCode_OSOK) {
        return error;
    }

    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return false;
    }

    direction = direction_value;
    mode = mode_value;

    if (!Motor_API_IsCorrectSpeed(speed)) {
        snprintf(response->data, response->size, "%d: Incorect speed\n", speed);

        return false;
    }

    if (!Motor_Config_IsCorrectDirection(direction)) {
        snprintf(response->data, response->size, "%d: Incorect motor direction\n", direction);

        return eErrorCode_INVAL;
    }

    if (!Motor_API_IsCorrectMode(mode)) {
        snprintf(response->data, response->size, "%d: Incorect motor mode\n", mode);

        return false;
    }

    sMotorCommandDesc_t formated_task = {.task = eMotorTask_Set, .data = NULL};
    sMotorSet_t *task_data = Heap_API_Calloc(1, sizeof(sMotorSet_t));

    if (task_data == NULL) {
        snprintf(response->data, response->size, "Failed Calloc\n");
        
        return false;
    }

    task_data->speed = speed;
    task_data->direction = direction;
    task_data->mode = mode;
    formated_task.data = task_data;

    if (!Motor_APP_Add_Task(&formated_task)) {
        snprintf(response->data, response->size, "Failed task add\n");
        
        Heap_API_Free(task_data);

        return false;
    }

    snprintf(response->data, response->size, "Operation successful\n");

    return true;
}
#endif

bool CLI_APP_Led_Handlers_RgbToHsv (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return false;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return false;
    }

    size_t red = 0;
    size_t green = 0;
    size_t blue = 0;

    if (CMD_API_Helper_FindNextArgUInt(&arguments, &red, CMD_SEPARATOR, CMD_SEPARATOR_LENGHT, response) != eErrorCode_OSOK) {
        return false;
    }

    if (CMD_API_Helper_FindNextArgUInt(&arguments, &green, CMD_SEPARATOR, CMD_SEPARATOR_LENGHT, response) != eErrorCode_OSOK) {
        return false;
    }

    if (CMD_API_Helper_FindNextArgUInt(&arguments, &blue, CMD_SEPARATOR, CMD_SEPARATOR_LENGHT, response) != eErrorCode_OSOK) {
        return false;
    }
    
    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return false;
    }

    if ((red > 255) || (green > 255) || (blue > 255)) {
        snprintf(response->data, response->size, "Invalid RGB values\n");

        return false;
    }

    sLedColorRgb_t rgb = {0};
    sLedColorHsv_t hsv = {0};
    rgb.color = (red << 16) | (green << 8) | blue;

    LED_RgbToHsv(rgb, &hsv);

    TRACE_INFO("hue: %d, sat: %d, val: %d\n", hsv.hue, hsv.saturation, hsv.value);

    snprintf(response->data, response->size, "Operation successful\n");

    return true;
}

bool CLI_APP_Led_Handlers_HsvToRgb (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return false;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return false;
    }

    size_t hue = 0;
    size_t saturation = 0;
    size_t value = 0;

    if (CMD_API_Helper_FindNextArgUInt(&arguments, &hue, CMD_SEPARATOR, CMD_SEPARATOR_LENGHT, response) != eErrorCode_OSOK) {
        return false;
    }

    if (CMD_API_Helper_FindNextArgUInt(&arguments, &saturation, CMD_SEPARATOR, CMD_SEPARATOR_LENGHT, response) != eErrorCode_OSOK) {
        return false;
    }

    if (CMD_API_Helper_FindNextArgUInt(&arguments, &value, CMD_SEPARATOR, CMD_SEPARATOR_LENGHT, response) != eErrorCode_OSOK) {
        return false;
    }
    
    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return false;
    }

    if ((hue > 255) || (saturation > 255) || (value > 255)) {
        snprintf(response->data, response->size, "Invalid RGB values\n");

        return false;
    }

    sLedColorHsv_t hsv = {0};
    sLedColorRgb_t rgb = {0};

    hsv.hue = hue;
    hsv.saturation = saturation;
    hsv.value = value;

    LED_HsvToRgb(hsv, &rgb);

    TRACE_INFO("red: %d, green: %d, blue: %d\n", (rgb.color >> 16) & 0xFF, (rgb.color >> 8) & 0xFF, rgb.color & 0xFF);
    
    snprintf(response->data, response->size, "Operation successful\n");

    return true;
}

#endif
