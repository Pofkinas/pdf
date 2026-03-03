/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "vl53l0xv2_api.h"

#ifdef ENABLE_VL53L0X
#include "cmsis_os2.h"
#include "vl53l0x_api.h"
#include "i2c_api.h"
#include "debug_api.h"
#include "gpio_driver.h"
#include "delay.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

#ifdef DEBUG_VL53L0X_API 
CREATE_MODULE_NAME (VL53L0XV2_API)
#else
CREATE_MODULE_NAME_EMPTY
#endif /* DEBUG_VL53L0X_API */

static const FixPoint1616_t g_default_offset_calibration_distance = (uint32_t) DEFAULT_OFFSET_CALIB_DISTANCE_MM * 65536;
static const uint32_t g_timeout = DEFAULT_STOP_TIMEOUT * SYSTEM_MS_TICS;

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static bool g_is_initialized = false;

static sVl53l0xStaticDesc_t g_static_vl53l0x_lut[eVl53l0x_Last] = {0};
static sVl53l0xDynamicDesc_t g_dynamic_vl53l0x_lut[eVl53l0x_Last] = {0};

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

static bool VL53L0X_API_InitDevice (const eVl53l0x_t vl53l0x);
static bool VL53L0X_API_ConfigureDevice (const eVl53l0x_t vl53l0x);
static bool VL53L0X_API_SetRangeProfile (const eVl53l0x_t vl53l0x, const eVl53l0xRangeProfile_t profile);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
 
static bool VL53L0X_API_InitDevice (const eVl53l0x_t vl53l0x) {
    if (!VL53L0XV2_Config_IsCorrectVl53l0x(vl53l0x)) {
        return false;
    }

    if (g_dynamic_vl53l0x_lut[vl53l0x].state != eVl53l0xState_Off) {
        return true;
    }

    if (!I2C_API_Init(g_static_vl53l0x_lut[vl53l0x].i2c)) {
        return false;
    }

    if (g_static_vl53l0x_lut[vl53l0x].has_xshut_pin) {
        if (!GPIO_Driver_WritePin(g_static_vl53l0x_lut[vl53l0x].xshut_pin, true)) {
            return false;
        }
    }

    osDelay(10);

    if (VL53L0X_DataInit(&g_dynamic_vl53l0x_lut[vl53l0x].device) != VL53L0X_ERROR_NONE) {
        TRACE_ERR("InitDevice: DataInit failed for VL53L0X %d\n", vl53l0x);
        
        return false;
    }

    if (VL53L0X_SetDeviceAddress(&g_dynamic_vl53l0x_lut[vl53l0x].device, (g_static_vl53l0x_lut[vl53l0x].i2c_address)) != VL53L0X_ERROR_NONE) {
        TRACE_ERR("InitDevice: SetDeviceAddress failed for VL53L0X %d\n", vl53l0x);
        
        return false;
    }

    g_dynamic_vl53l0x_lut[vl53l0x].device.I2cDevAddr = (g_static_vl53l0x_lut[vl53l0x].i2c_address >> 1);

    if (VL53L0X_StaticInit(&g_dynamic_vl53l0x_lut[vl53l0x].device) != VL53L0X_ERROR_NONE) {
        TRACE_ERR("InitDevice: StaticInit failed for VL53L0X %d\n", vl53l0x);

        return false;
    }

    g_dynamic_vl53l0x_lut[vl53l0x].state = eVl53l0xState_Init;

    return true;
}

static bool VL53L0X_API_ConfigureDevice (const eVl53l0x_t vl53l0x) {
    // TODO: Make calib settings save to flash

    if (!VL53L0XV2_Config_IsCorrectVl53l0x(vl53l0x)) {
        return false;
    }

    if (g_dynamic_vl53l0x_lut[vl53l0x].state != eVl53l0xState_Init) {
        return false;
    }
    
    if (!g_dynamic_vl53l0x_lut[vl53l0x].has_calib_default_data) {
        if (VL53L0X_PerformRefSpadManagement(&g_dynamic_vl53l0x_lut[vl53l0x].device, &g_dynamic_vl53l0x_lut[vl53l0x].calib_SpadCount, &g_dynamic_vl53l0x_lut[vl53l0x].calib_isApertureSpads) != VL53L0X_ERROR_NONE) {
            TRACE_ERR("ConfigureDevice: PerformRefSpadManagement failed for VL53L0X %d\n", vl53l0x);
            
            return false;
        }

        if (VL53L0X_PerformRefCalibration(&g_dynamic_vl53l0x_lut[vl53l0x].device, &g_dynamic_vl53l0x_lut[vl53l0x].calib_VhvSettings, &g_dynamic_vl53l0x_lut[vl53l0x].calib_PhaseCal) != VL53L0X_ERROR_NONE) {
            TRACE_ERR("ConfigureDevice: PerformRefCalibration failed for VL53L0X %d\n", vl53l0x);
            
            return false;
        }

        if (VL53L0X_PerformOffsetCalibration(&g_dynamic_vl53l0x_lut[vl53l0x].device, g_default_offset_calibration_distance, &g_dynamic_vl53l0x_lut[vl53l0x].offset) != VL53L0X_ERROR_NONE) {
            TRACE_ERR("ConfigureDevice: PerformOffsetCalibration failed for VL53L0X %d\n", vl53l0x);
            
            return false;
        }

        if (g_static_vl53l0x_lut[vl53l0x].crosstalk_talk_compensation_en) {
            VL53L0X_PerformXTalkCalibration(&g_dynamic_vl53l0x_lut[vl53l0x].device, g_static_vl53l0x_lut[vl53l0x].crosstalk_talk_distance, &g_dynamic_vl53l0x_lut[vl53l0x].crosstalk_value);
        
        }
    }

    if (VL53L0X_SetReferenceSpads(&g_dynamic_vl53l0x_lut[vl53l0x].device, g_dynamic_vl53l0x_lut[vl53l0x].calib_SpadCount, g_dynamic_vl53l0x_lut[vl53l0x].calib_isApertureSpads) != VL53L0X_ERROR_NONE) {
        TRACE_ERR("ConfigureDevice: SetReferenceSpads failed for VL53L0X %d\n", vl53l0x);
        
        return false;
    }

    if (VL53L0X_SetRefCalibration(&g_dynamic_vl53l0x_lut[vl53l0x].device, g_dynamic_vl53l0x_lut[vl53l0x].calib_VhvSettings, g_dynamic_vl53l0x_lut[vl53l0x].calib_PhaseCal) != VL53L0X_ERROR_NONE) {
        TRACE_ERR("ConfigureDevice: SetRefCalibration failed for VL53L0X %d\n", vl53l0x);
        
        return false;
    }

    if (VL53L0X_SetOffsetCalibrationDataMicroMeter(&g_dynamic_vl53l0x_lut[vl53l0x].device, g_dynamic_vl53l0x_lut[vl53l0x].offset) != VL53L0X_ERROR_NONE) {
        TRACE_ERR("ConfigureDevice: SetOffsetCalibrationDataMicroMeter failed for VL53L0X %d\n", vl53l0x);
        
        return false;
    }

    if (g_static_vl53l0x_lut[vl53l0x].crosstalk_talk_compensation_en) {
        if (VL53L0X_SetXTalkCompensationRateMegaCps(&g_dynamic_vl53l0x_lut[vl53l0x].device, g_dynamic_vl53l0x_lut[vl53l0x].crosstalk_value) != VL53L0X_ERROR_NONE) {
            TRACE_ERR("ConfigureDevice: SetXTalkCompensationRateMegaCps failed for VL53L0X %d\n", vl53l0x);
            
            return false;
        }
    
        if (VL53L0X_SetXTalkCompensationEnable(&g_dynamic_vl53l0x_lut[vl53l0x].device, g_static_vl53l0x_lut[vl53l0x].crosstalk_talk_compensation_en) != VL53L0X_ERROR_NONE) {
            TRACE_ERR("ConfigureDevice: SetXTalkCompensationEnable failed for VL53L0X %d\n", vl53l0x);
            
            return false;
        }
    }

    if (VL53L0X_SetDeviceMode(&g_dynamic_vl53l0x_lut[vl53l0x].device, g_static_vl53l0x_lut[vl53l0x].device_mode) != VL53L0X_ERROR_NONE) {
        TRACE_ERR("ConfigureDevice: SetDeviceMode failed for VL53L0X %d\n", vl53l0x);
        
        return false;
    }

    if (!VL53L0X_API_SetRangeProfile(vl53l0x, g_static_vl53l0x_lut[vl53l0x].range_profile)) {
        TRACE_ERR("ConfigureDevice: SetRangeProfile failed for VL53L0X %d\n", vl53l0x);
        
        return false;
    }

    g_dynamic_vl53l0x_lut[vl53l0x].state = eVl53l0xState_Standby;

    return true;
}

static bool VL53L0X_API_SetRangeProfile (const eVl53l0x_t vl53l0x, const eVl53l0xRangeProfile_t profile) {
    if (!VL53L0XV2_Config_IsCorrectVl53l0x(vl53l0x)) {
        return false;
    }
    
    if ((profile < eVl53l0xRangeProfile_First) || (profile >= eVl53l0xRangeProfile_Last)) {
        return false;
    }

    uint8_t signal_rate_multiplyer = 0;
    uint8_t sigma_multiplyer = 0;
    uint32_t measurement_time = 0;

    switch (profile) {
        case eVl53l0xRangeProfile_Default: {
            signal_rate_multiplyer = 0.25;
            sigma_multiplyer = 18;
            measurement_time = 33000;
            return true;
        }
        case eVl53l0xRangeProfile_HighAccuracy: {
            signal_rate_multiplyer = 0.25;
            sigma_multiplyer = 18;
            measurement_time = 200000;
        } break;
        case eVl53l0xRangeProfile_LongRange: {
            signal_rate_multiplyer = 0.1;
            sigma_multiplyer = 60;
            measurement_time = 33000;
        } break;
        case eVl53l0xRangeProfile_HighSpeed: {
            signal_rate_multiplyer = 0.25;
            sigma_multiplyer = 32;
            measurement_time = 20000;
        } break;
        default: {
            return false;
        }
    }

    if (VL53L0X_SetLimitCheckValue(&g_dynamic_vl53l0x_lut[vl53l0x].device, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, (FixPoint1616_t)(signal_rate_multiplyer * 65536)) != VL53L0X_ERROR_NONE) {
        return false;
    }

    if (VL53L0X_SetLimitCheckValue(&g_dynamic_vl53l0x_lut[vl53l0x].device, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, (FixPoint1616_t)(sigma_multiplyer * 65536)) != VL53L0X_ERROR_NONE) {
        return false;
    }
    if (VL53L0X_SetMeasurementTimingBudgetMicroSeconds(&g_dynamic_vl53l0x_lut[vl53l0x].device, measurement_time) != VL53L0X_ERROR_NONE) {
        return false;
    }

    if (profile == eVl53l0xRangeProfile_LongRange) {
        if (VL53L0X_SetVcselPulsePeriod(&g_dynamic_vl53l0x_lut[vl53l0x].device, VL53L0X_VCSEL_PERIOD_PRE_RANGE, 18) != VL53L0X_ERROR_NONE) {
            return false;
        }
        if (VL53L0X_SetVcselPulsePeriod(&g_dynamic_vl53l0x_lut[vl53l0x].device, VL53L0X_VCSEL_PERIOD_FINAL_RANGE, 14) != VL53L0X_ERROR_NONE) {
            return false;
        }
    }

    return true;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool VL53L0X_API_InitAll (void) {
    if (g_is_initialized) {
        return true;
    }
    
    if (!GPIO_Driver_InitAllPins()) {
        return false;
    }

    for (eVl53l0x_t vl53l0x = eVl53l0x_First; vl53l0x < eVl53l0x_Last; vl53l0x++) {
        const sVl53l0xStaticDesc_t *static_desc = VL53L0XV2_Config_GetVl53l0xStaticDesc(vl53l0x);
        const sVl53l0xDynamicDesc_t *dynamic_desc = VL53L0XV2_Config_GetVl53l0xDynamicDesc(vl53l0x);

        if (static_desc == NULL || dynamic_desc == NULL) {
            TRACE_ERR("InitAll: Failed to get VL53L0X %d description\n", vl53l0x);
            
            return false;
        }

        g_static_vl53l0x_lut[vl53l0x] = *static_desc;
        g_dynamic_vl53l0x_lut[vl53l0x] = *dynamic_desc;
        
        if (!g_static_vl53l0x_lut[vl53l0x].has_xshut_pin) {
            continue;
        }
        
        if (!GPIO_Driver_WritePin(g_static_vl53l0x_lut[vl53l0x].xshut_pin, false)) {
            return false;
        }
    }

    osDelay(1);

    for (eVl53l0x_t vl53l0x = eVl53l0x_First; vl53l0x < eVl53l0x_Last; vl53l0x++) {
        if (!VL53L0X_API_InitDevice(vl53l0x)) {
            return false;
        }
    }

    for (eVl53l0x_t vl53l0x = eVl53l0x_First; vl53l0x < eVl53l0x_Last; vl53l0x++) {
        if (!VL53L0X_API_ConfigureDevice(vl53l0x)) {
            return false;
        }
    }

    return true;
}

bool VL53L0X_API_StartMeasuring (const eVl53l0x_t vl53l0x) {
    if (!VL53L0XV2_Config_IsCorrectVl53l0x(vl53l0x)) {
        return false;
    }

    if (g_dynamic_vl53l0x_lut[vl53l0x].state == eVl53l0xState_Off) {
        return false;
    }

    if (g_dynamic_vl53l0x_lut[vl53l0x].state == eVl53l0xState_Measuring) {
        return true;
    }

    if (VL53L0X_StartMeasurement(&g_dynamic_vl53l0x_lut[vl53l0x].device) != VL53L0X_ERROR_NONE) {
        TRACE_ERR("StartMeasuring: StartMeasurement failed for VL53L0X %d\n", vl53l0x);
        
        return false;
    }

    g_dynamic_vl53l0x_lut[vl53l0x].state = eVl53l0xState_Measuring;

    return true;
}

bool VL53L0X_API_StopMeasuring (const eVl53l0x_t vl53l0x) {
    if (!VL53L0XV2_Config_IsCorrectVl53l0x(vl53l0x)) {
        return false;
    }

    if (g_dynamic_vl53l0x_lut[vl53l0x].state == eVl53l0xState_Off) {
        return false;
    }

    if (g_dynamic_vl53l0x_lut[vl53l0x].state == eVl53l0xState_Standby) {
        return true;
    }

    uint32_t stop_status = 0;

    VL53L0X_Error error = VL53L0X_StopMeasurement(&g_dynamic_vl53l0x_lut[vl53l0x].device);

    if (error != VL53L0X_ERROR_NONE) {
        TRACE_ERR("StopMeasuring: StopMeasurement failed for VL53L0X %d [%d]\n", vl53l0x, error);
        
        return false;
    }

    uint32_t start_tick = osKernelGetSysTimerCount();

    while ((osKernelGetSysTimerCount() - start_tick) < g_timeout) {
        error = VL53L0X_GetStopCompletedStatus(&g_dynamic_vl53l0x_lut[vl53l0x].device, &stop_status);

        if (error == VL53L0X_ERROR_NONE && stop_status == 0) {
            break;
        }
    }

    if ((osKernelGetSysTimerCount() - start_tick) >= g_timeout) {
        TRACE_ERR("StopMeasuring: Timeout for VL53L0X %d [%d]\n", vl53l0x, error);
        
        return false;
    }

    g_dynamic_vl53l0x_lut[vl53l0x].state = eVl53l0xState_Standby;

    return true;
}

bool VL53L0X_API_TurnOff (const eVl53l0x_t vl53l0x) {
    if (!VL53L0XV2_Config_IsCorrectVl53l0x(vl53l0x)) {
        return false;
    }

    if (g_dynamic_vl53l0x_lut[vl53l0x].state == eVl53l0xState_Off) {
        return true;
    }

    if (g_dynamic_vl53l0x_lut[vl53l0x].state == eVl53l0xState_Measuring) {
        if (!VL53L0X_API_StopMeasuring(vl53l0x)) {
            TRACE_ERR("TurnOff: Failed to stop sensor before turning off VL53L0X %d\n", vl53l0x);
            
            return false;
        }
    }

    GPIO_Driver_WritePin(g_static_vl53l0x_lut[vl53l0x].xshut_pin, false);

    g_dynamic_vl53l0x_lut[vl53l0x].state = eVl53l0xState_Off;

    return true;
}

bool VL53L0X_API_TurnOn (const eVl53l0x_t vl53l0x) {
    if (!VL53L0XV2_Config_IsCorrectVl53l0x(vl53l0x)) {
        return false;
    }

    if (g_dynamic_vl53l0x_lut[vl53l0x].state != eVl53l0xState_Off) {
        return true;
    }

    if (!VL53L0X_API_InitDevice(vl53l0x)) {
        return false;
    }

    g_dynamic_vl53l0x_lut[vl53l0x].state = eVl53l0xState_Standby;

    return true;
}

bool VL53L0X_API_GetDistance (const eVl53l0x_t vl53l0x, uint16_t *distance, size_t timeout) {
    if (!VL53L0XV2_Config_IsCorrectVl53l0x(vl53l0x)) {
        return false;
    }

    if (distance == NULL) {
        return false;
    }

    if (g_dynamic_vl53l0x_lut[vl53l0x].state != eVl53l0xState_Measuring) {
        return false;
    }

    VL53L0X_Error error = 0;
    uint8_t data_status = 0;
    VL53L0X_RangingMeasurementData_t ranging_data = {0};

    switch (g_static_vl53l0x_lut[vl53l0x].range_profile) {
        case eVl53l0xRangeProfile_Default: {
            if (timeout < MIN_DEFAULT_TIMEOUT) {
                timeout = MIN_DEFAULT_TIMEOUT;
            }
        } break;
        case eVl53l0xRangeProfile_HighAccuracy:{
            if (timeout < MIN_HIGH_ACCURACY_TIMEOUT) {
                timeout = MIN_HIGH_ACCURACY_TIMEOUT;
            }
        } break;
        case eVl53l0xRangeProfile_LongRange: {
            if (timeout < MIN_LONG_RANGE_TIMEOUT) {
                timeout = MIN_LONG_RANGE_TIMEOUT;
            }
        } break;
        case eVl53l0xRangeProfile_HighSpeed: {
            if (timeout < MIN_HIGH_SPEED_TIMEOUT) {
                timeout = MIN_HIGH_SPEED_TIMEOUT;
            }
        } break;
        default: {
            timeout = MIN_DEFAULT_TIMEOUT;
        } break;
    }

    timeout *= SYSTEM_MS_TICS;

    uint32_t start_tick = osKernelGetSysTimerCount();

    while ((osKernelGetSysTimerCount() - start_tick) < timeout) {
        error = VL53L0X_GetMeasurementDataReady(&g_dynamic_vl53l0x_lut[vl53l0x].device, &data_status);

        if (error == VL53L0X_ERROR_NONE && data_status != 0) {
            break;
        }

        osDelay(10);
    }

    if ((osKernelGetSysTimerCount() - start_tick) >= timeout) {
        TRACE_ERR("GetDistance: Timeout for VL53L0X %d [%d]\n", vl53l0x, error);
        
        return false;
    }

    if (data_status == 0) {
        return false;
    }

    error = VL53L0X_GetRangingMeasurementData(&g_dynamic_vl53l0x_lut[vl53l0x].device, &ranging_data);

    if (error != VL53L0X_ERROR_NONE) {
        TRACE_ERR("GetDistance: GetRangingMeasurementData failed for VL53L0X %d [%d]\n", vl53l0x, error);
        
        return false;
    }

    error = VL53L0X_ClearInterruptMask(&g_dynamic_vl53l0x_lut[vl53l0x].device, 0);

    if (error != VL53L0X_ERROR_NONE) {
        TRACE_ERR("GetDistance: ClearInterruptMask failed for VL53L0X %d [%d]\n", vl53l0x, error);
        
        return false;
    }

    if (ranging_data.RangeStatus != 0) {
        #ifdef DEBUG_VL53L0X_RANGE_STATUS
        TRACE_ERR("GetDistance: RangeStatus: [%d]\n", ranging_data.RangeStatus);
        #endif

        *distance = 0;
        
        return false;
    }

    *distance = ranging_data.RangeMilliMeter;

    return true;
}

#endif /* ENABLE_VL53L0X */
