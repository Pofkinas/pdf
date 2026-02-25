/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "cli_cmd_handlers.h"

#ifdef ENABLE_DEFAULT_CMD

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
#include "colour.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

#define CMD_SEPARATOR_LENGTH (sizeof(CMD_SEPARATOR) - 1)

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

#ifdef DEBUG_DEFAULT_CMD
CREATE_MODULE_NAME (CLI_DEFAULT_CMD)
#else
CREATE_MODULE_NAME_EMPTY
#endif /* DEBUG_DEFAULT_CMD */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

#ifdef ENABLE_LED
static eErrorCode_t CLI_APP_Led_Handlers_Common (sMessage_t arguments, sMessage_t *response, const eLedTask_t task);
#endif /* ENABLE_LED */

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

#ifdef ENABLE_LED
static eErrorCode_t CLI_APP_Led_Handlers_Common (sMessage_t arguments, sMessage_t *response, const eLedTask_t task) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return eErrorCode_NULLPTR;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return eErrorCode_NULLPTR;
    }
    
    eLed_t led;
    size_t led_value = 0;

    eErrorCode_t error = eErrorCode_OK;

    error = CMD_API_Helper_FindNextArgUInt(&arguments, &led_value, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);

    if (error != eErrorCode_OK) {
        return error;
    }

    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return eErrorCode_ARGMANY;
    }

    led = led_value;

    if (!LED_Config_IsCorrectLed(led)) {
        snprintf(response->data, response->size, "%d: Incorrect led\n", led);

        return eErrorCode_INVAL;
    }

    sLedCommandDesc_t formated_task = {.task = task, .data = NULL};
    sLedCommon_t *task_data = Heap_API_Calloc(1, sizeof(sLedCommon_t));

    if (task_data == NULL) {
        snprintf(response->data, response->size, "Failed Calloc\n");
        
        return eErrorCode_NOMEM;
    }

    task_data->led = led;
    formated_task.data = task_data;

    if (!LED_APP_Add_Task(&formated_task)) {
        snprintf(response->data, response->size, "Failed task add\n");
        
        Heap_API_Free(task_data);

        return eErrorCode_FAILED;
    }

    snprintf(response->data, response->size, "Operation successful\n");

    return eErrorCode_OK;
}
#endif /* ENABLE_LED */

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

#ifdef ENABLE_LED
eErrorCode_t CLI_APP_Led_Handlers_Set (sMessage_t arguments, sMessage_t *response) {
    eLedTask_t task = eLedTask_Set;

    return CLI_APP_Led_Handlers_Common(arguments, response, task);
}

eErrorCode_t CLI_APP_Led_Handlers_Reset (sMessage_t arguments, sMessage_t *response) {
    eLedTask_t task = eLedTask_Reset;

    return CLI_APP_Led_Handlers_Common(arguments, response, task);
}

eErrorCode_t CLI_APP_Led_Handlers_Toggle (sMessage_t arguments, sMessage_t *response) {
    eLedTask_t task = eLedTask_Toggle;

    return CLI_APP_Led_Handlers_Common(arguments, response, task);
}

eErrorCode_t CLI_APP_Led_Handlers_Blink (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return eErrorCode_NULLPTR;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return eErrorCode_NULLPTR;
    }
    
    eLed_t led;
    size_t led_value = 0;
    size_t blink_time = 0;
    size_t blink_frequency = 0;
    eErrorCode_t error = eErrorCode_OK;

    error = CMD_API_Helper_FindNextArgUInt(&arguments, &led_value, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);
    
    if (error != eErrorCode_OK) {
        return error;
    }

    error = CMD_API_Helper_FindNextArgUInt(&arguments, &blink_time, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);
    
    if (error != eErrorCode_OK) {
        return error;
    }

    error = CMD_API_Helper_FindNextArgUInt(&arguments, &blink_frequency, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);
    
    if (error != eErrorCode_OK) {
        return error;
    }
    
    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return eErrorCode_ARGMANY;
    }

    led = led_value;

    if (!LED_Config_IsCorrectLed(led)) {
        snprintf(response->data, response->size, "%d: Incorrect led\n", led);

        return eErrorCode_INVAL;
    }

    if (!LED_API_IsCorrectBlinkTime(blink_time)) {
        snprintf(response->data, response->size, "%d: Incorrect blink time\n", blink_time);

        return eErrorCode_INVAL;
    }

    if (!LED_API_IsCorrectBlinkFrequency(blink_frequency)) {
        snprintf(response->data, response->size, "%d: Incorrect blink frequency\n", blink_frequency);

        return eErrorCode_INVAL;
    }

    sLedCommandDesc_t formated_task = {.task = eLedTask_Blink, .data = NULL};
    sLedBlink_t *task_data = Heap_API_Calloc(1, sizeof(sLedBlink_t));

    if (task_data == NULL) {
        snprintf(response->data, response->size, "Failed Calloc\n");
        
        return eErrorCode_NOMEM;
    }

    task_data->led = led;
    task_data->blink_time = blink_time;
    task_data->blink_frequency = blink_frequency;
    formated_task.data = task_data;

    if (!LED_APP_Add_Task(&formated_task)) {
        snprintf(response->data, response->size, "Failed task add\n");
        
        Heap_API_Free(task_data);

        return eErrorCode_CANCELED;
    }

    snprintf(response->data, response->size, "Operation successful\n");

    return eErrorCode_OK;
}
#endif /* ENABLE_LED */

#ifdef ENABLE_PWM_LED
eErrorCode_t CLI_APP_Pwm_Led_Handlers_Set_Brightness (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return eErrorCode_NULLPTR;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return eErrorCode_NULLPTR;
    }

    eLedPwm_t led;
    size_t led_value = 0;
    size_t duty_cycle = 0;
    eErrorCode_t error = eErrorCode_OK;

    error = CMD_API_Helper_FindNextArgUInt(&arguments, &led_value, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);

    if (error != eErrorCode_OK) {
        return error;
    }
    
    error = CMD_API_Helper_FindNextArgUInt(&arguments, &duty_cycle, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);

    if (error != eErrorCode_OK) {
        return error;
    }

    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return eErrorCode_ARGMANY;
    }

    led = led_value;

    if (!LED_Config_IsCorrectPwmLed(led)) {
        snprintf(response->data, response->size, "%d: Incorrect led\n", led);

        return eErrorCode_INVAL;
    }

    if (!LED_API_IsCorrectDutyCycle(led, duty_cycle)) {
        snprintf(response->data, response->size, "%d: Incorrect duty cycle\n", led);

        return eErrorCode_INVAL;
    }

    sLedCommandDesc_t formated_task = {.task = eLedTask_Set_Brightness, .data = NULL};
    sLedSetBrightness_t *task_data = Heap_API_Calloc(1, sizeof(sLedSetBrightness_t));

    if (task_data == NULL) {
        snprintf(response->data, response->size, "Failed Calloc\n");
        
        return eErrorCode_NOMEM;
    }

    task_data->led = led;
    task_data->duty_cycle = duty_cycle;
    formated_task.data = task_data;

    if (!LED_APP_Add_Task(&formated_task)) {
        snprintf(response->data, response->size, "Failed task add\n");
        
        Heap_API_Free(task_data);

        return eErrorCode_FAILED;
    }

    snprintf(response->data, response->size, "Operation successful\n");

    return eErrorCode_OK;
}

eErrorCode_t CLI_APP_Pwm_Led_Handlers_Pulse (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return eErrorCode_NULLPTR;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return eErrorCode_NULLPTR;
    }

    eLedPwm_t led;
    size_t led_value = 0;
    size_t pulse_time = 0;
    size_t pulse_frequency = 0;
    eErrorCode_t error = eErrorCode_OK;

    error = CMD_API_Helper_FindNextArgUInt(&arguments, &led_value, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);

    if (error != eErrorCode_OK) {
        return error;
    }
    
    error = CMD_API_Helper_FindNextArgUInt(&arguments, &pulse_time, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);

    if (error != eErrorCode_OK) {
        return error;
    }

    error = CMD_API_Helper_FindNextArgUInt(&arguments, &pulse_frequency, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);

    if (error != eErrorCode_OK) {
        return error;
    }

    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return eErrorCode_ARGMANY;
    }

    led = led_value;

    if (!LED_Config_IsCorrectPwmLed(led)) {
        snprintf(response->data, response->size, "%d: Incorrect led\n", led);

        return eErrorCode_INVAL;
    }

    if (!LED_API_IsCorrectPulseTime(pulse_time)) {
        snprintf(response->data, response->size, "%d: Incorrect pulse time\n", led);

        return eErrorCode_INVAL;
    }

    if (!LED_API_IsCorrectPulseFrequency(pulse_frequency)) {
        snprintf(response->data, response->size, "%d: Incorrect pulse frequency\n", led);

        return eErrorCode_INVAL;
    }

    sLedCommandDesc_t formated_task = {.task = eLedTask_Pulse, .data = NULL};
    sLedPulse_t *task_data = Heap_API_Calloc(1, sizeof(sLedPulse_t));

    if (task_data == NULL) {
        snprintf(response->data, response->size, "Failed Calloc\n");
        
        return eErrorCode_NOMEM;
    }

    task_data->led = led;
    task_data->pulse_time = pulse_time;
    task_data->pulse_frequency = pulse_frequency;
    formated_task.data = task_data;

    if (!LED_APP_Add_Task(&formated_task)) {
        snprintf(response->data, response->size, "Failed task add\n");
        
        Heap_API_Free(task_data);

        return eErrorCode_FAILED;
    }

    snprintf(response->data, response->size, "Operation successful\n");

    return eErrorCode_OK;
}
#endif /* ENABLE_PWM_LED */

#ifdef ENABLE_MOTOR
eErrorCode_t CLI_APP_Motors_Handlers_Stop (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return eErrorCode_NULLPTR;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return eErrorCode_NULLPTR;
    }

    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return eErrorCode_ARGMANY;
    }

    sMotorCommandDesc_t formated_task = {.task = eMotorTask_Stop, .data = NULL};

    if (!Motor_APP_Add_Task(&formated_task)) {
        snprintf(response->data, response->size, "Failed task add\n");

        return eErrorCode_FAILED;
    }

    snprintf(response->data, response->size, "Operation successful\n");

    return eErrorCode_OK;
}

eErrorCode_t CLI_APP_Motors_Handlers_Set (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return eErrorCode_NULLPTR;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return eErrorCode_NULLPTR;
    }

    eMotorDirection_t direction;
    eMotorControl_t mode;
    size_t speed = 0;
    size_t direction_value = 0;
    size_t mode_value = 0;
    eErrorCode_t error = eErrorCode_OK;

    error = CMD_API_Helper_FindNextArgUInt(&arguments, &speed, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);

    if (error != eErrorCode_OK) {
        return error;
    }

    error = CMD_API_Helper_FindNextArgUInt(&arguments, &direction_value, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);

    if (error != eErrorCode_OK) {
        return error;
    }

    error = CMD_API_Helper_FindNextArgUInt(&arguments, &mode_value, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);

    if (error != eErrorCode_OK) {
        return error;
    }

    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return eErrorCode_ARGMANY;
    }

    direction = direction_value;
    mode = mode_value;

    if (!Motor_API_IsCorrectSpeed(speed)) {
        snprintf(response->data, response->size, "%d: Incorect speed\n", speed);

        return eErrorCode_INVAL;
    }

    if (!Motor_Config_IsCorrectDirection(direction)) {
        snprintf(response->data, response->size, "%d: Incorect motor direction\n", direction);

        return eErrorCode_INVAL;
    }

    if (!Motor_API_IsCorrectMode(mode)) {
        snprintf(response->data, response->size, "%d: Incorect motor mode\n", mode);

        return eErrorCode_INVAL;
    }

    sMotorCommandDesc_t formated_task = {.task = eMotorTask_Set, .data = NULL};
    sMotorSet_t *task_data = Heap_API_Calloc(1, sizeof(sMotorSet_t));

    if (task_data == NULL) {
        snprintf(response->data, response->size, "Failed Calloc\n");
        
        return eErrorCode_NOMEM;
    }

    task_data->speed = speed;
    task_data->direction = direction;
    task_data->mode = mode;
    formated_task.data = task_data;

    if (!Motor_APP_Add_Task(&formated_task)) {
        snprintf(response->data, response->size, "Failed task add\n");
        
        Heap_API_Free(task_data);

        return eErrorCode_FAILED;
    }

    snprintf(response->data, response->size, "Operation successful\n");

    return eErrorCode_OK;
}
#endif /* ENABLE_MOTOR */

#ifdef ENABLE_PID_CONTROL
CLI_APP_Motors_Handlers_SetTargetRPM (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return eErrorCode_NULLPTR;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return eErrorCode_NULLPTR;
    }

    eMotor_t motor;
    eMotorControl_t mode;
    size_t motor_value = 0;
    size_t mode_value = 0;
    float target_rpm;

    eErrorCode_t error = eErrorCode_OK;

    error = CMD_API_Helper_FindNextArgUInt(&arguments, &motor_value, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);

    if (error != eErrorCode_OK) {
        return error;
    }

    error = CMD_API_Helper_FindNextArgUInt(&arguments, &mode_value, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);

    if (error != eErrorCode_OK) {
        return error;
    }

    error = CMD_API_Helper_FindNextArgFloat(&arguments, &target_rpm, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);
    
    if (error != eErrorCode_OK) {
        return error;
    }

    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return eErrorCode_ARGMANY;
    }

    if (!Motor_Config_IsCorrectMotor(motor_value)) {
        snprintf(response->data, response->size, "%d: Incorrect motor\n", motor_value);

        return eErrorCode_INVAL;
    }

    if (!Motor_API_IsCorrectRPM(target_rpm)) {
        snprintf(response->data, response->size, "%d: Incorrect target RPM\n", target_rpm);

        return eErrorCode_INVAL;
    }

    if (!Motor_Config_IsCorrectMode(mode_value)) {
        snprintf(response->data, response->size, "%d: Incorect motor mode\n", mode_value);

        return eErrorCode_INVAL;
    }

    sMotorCommandDesc_t formated_task = {.task = eMotorTask_SetRpm, .data = NULL};
    sMotorSetRpm_t *task_data = Heap_API_Calloc(1, sizeof(sMotorSetRpm_t));

    if (task_data == NULL) {
        snprintf(response->data, response->size, "Failed Calloc\n");
        
        return eErrorCode_NOMEM;
    }

    task_data->motor = motor_value;
    task_data->target_rpm = target_rpm;
    task_data->mode = mode_value;
    formated_task.data = task_data;

    if (!Motor_APP_Add_Task(&formated_task)) {
        snprintf(response->data, response->size, "Failed task add\n");
        
        Heap_API_Free(task_data);

        return eErrorCode_FAILED;
    }

    snprintf(response->data, response->size, "Operation successful\n");

    return eErrorCode_OK;
}

eErrorCode_t Custom_CLI_APP_Handlers_SetPID (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return eErrorCode_NULLPTR;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return eErrorCode_NULLPTR;
    }

    sPID_t pid_params = {0};
    sMotor_t motor;
    size_t motor_value = 0;
    eErrorCode_t error = eErrorCode_OK;

    error = CMD_API_Helper_FindNextArgUInt(&arguments, &motor_value, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);
    
    if (error != eErrorCode_OK) {
        return error;
    }

    error = CMD_API_Helper_FindNextArgFloat(&arguments, &pid_params.kp, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);

    if (error != eErrorCode_OK) {
        return error;
    }

    error = CMD_API_Helper_FindNextArgFloat(&arguments, &pid_params.ki, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);

    if (error != eErrorCode_OK) {
        return error;
    }

    error = CMD_API_Helper_FindNextArgFloat(&arguments, &pid_params.kd, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);

    if (error != eErrorCode_OK) {
        return error;
    }

    error = CMD_API_Helper_FindNextArgFloat(&arguments, &pid_params.integral_limit, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);

    if (error != eErrorCode_OK) {
        return error;
    }

    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return eErrorCode_ARGMANY;
    }

    if (!Motor_Config_IsCorrectMotor(motor_value)) {
        snprintf(response->data, response->size, "%d: Incorrect motor\n", motor_value);

        return eErrorCode_INVAL;
    }

    motor = motor_value;

    if (!Motor_API_SetPID(motor, &pid_params)) {
        snprintf(response->data, response->size, "Failed to set PID parameters\n");

        return eErrorCode_FAILED;
    }

    TRACE_INFO("Set PID for [%d]: Kp: %ld.%04u, Ki: %ld.%04u, Kd: %ld.%04u, I limit: %ld.%04u\n", motor, FLOAT_INTEGER_PART(pid_params.kp), FLOAT_FRACTIONAL_PART(pid_params.kp, 4), FLOAT_INTEGER_PART(pid_params.ki), FLOAT_FRACTIONAL_PART(pid_params.ki, 4), FLOAT_INTEGER_PART(pid_params.kd), FLOAT_FRACTIONAL_PART(pid_params.kd, 4), FLOAT_INTEGER_PART(pid_params.integral_limit), FLOAT_FRACTIONAL_PART(pid_params.integral_limit, 4));

    snprintf(response->data, response->size, "Operation successful\n");

    return eErrorCode_OK;
}
#endif /* ENABLE_PID_CONTROL */

eErrorCode_t CLI_APP_Led_Handlers_RgbToHsv (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return eErrorCode_NULLPTR;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return eErrorCode_NULLPTR;
    }

    size_t red = 0;
    size_t green = 0;
    size_t blue = 0;
    eErrorCode_t error = eErrorCode_OK;

    error = CMD_API_Helper_FindNextArgUInt(&arguments, &red, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);

    if (error != eErrorCode_OK) {
        return error;
    }

    error = CMD_API_Helper_FindNextArgUInt(&arguments, &green, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);

    if (error != eErrorCode_OK) {
        return error;
    }

    error = CMD_API_Helper_FindNextArgUInt(&arguments, &blue, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);

    if (error != eErrorCode_OK) {
        return error;
    }
    
    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return eErrorCode_ARGMANY;
    }

    if ((red > 255) || (green > 255) || (blue > 255)) {
        snprintf(response->data, response->size, "Invalid RGB values\n");

        return eErrorCode_INVAL;
    }

    sLedColorRgb_t rgb = {0};
    sLedColorHsv_t hsv = {0};
    rgb.color = (red << 16) | (green << 8) | blue;

    LED_RgbToHsv(rgb, &hsv);

    TRACE_INFO("hue: %d, sat: %d, val: %d\n", hsv.hue, hsv.saturation, hsv.value);

    snprintf(response->data, response->size, "Operation successful\n");

    return eErrorCode_OK;
}

eErrorCode_t CLI_APP_Led_Handlers_HsvToRgb (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return eErrorCode_NULLPTR;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return eErrorCode_NULLPTR;
    }

    size_t hue = 0;
    size_t saturation = 0;
    size_t value = 0;
    eErrorCode_t error = eErrorCode_OK;

    error = CMD_API_Helper_FindNextArgUInt(&arguments, &hue, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);

    if (error != eErrorCode_OK) {
        return error;
    }

    error = CMD_API_Helper_FindNextArgUInt(&arguments, &saturation, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);

    if (error != eErrorCode_OK) {
        return error;
    }

    error = CMD_API_Helper_FindNextArgUInt(&arguments, &value, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);

    if (error != eErrorCode_OK) {
        return error;
    }
    
    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return eErrorCode_ARGMANY;
    }

    if ((hue > 255) || (saturation > 255) || (value > 255)) {
        snprintf(response->data, response->size, "Invalid RGB values\n");

        return eErrorCode_INVAL;
    }

    sLedColorHsv_t hsv = {0};
    sLedColorRgb_t rgb = {0};

    hsv.hue = hue;
    hsv.saturation = saturation;
    hsv.value = value;

    LED_HsvToRgb(hsv, &rgb);

    TRACE_INFO("red: %d, green: %d, blue: %d\n", (rgb.color >> 16) & 0xFF, (rgb.color >> 8) & 0xFF, rgb.color & 0xFF);
    
    snprintf(response->data, response->size, "Operation successful\n");

    return eErrorCode_OK;
}

#endif /* ENABLE_DEFAULT_CMD */
