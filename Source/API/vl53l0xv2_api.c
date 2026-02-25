/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "vl53l0xv2_api.h"

#if defined(ENABLE_VL53L0X)
#include "cmsis_os2.h"
#include "debug_api.h"
#include "vl53l0x_api.h"
#include "i2c_api.h"
#include "gpio_driver.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

#define BOOT_PERIOD 2

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

#if defined(DEBUG_VL53L0X_API)
CREATE_MODULE_NAME (VL53L0XV2_API)
#else
CREATE_MODULE_NAME_EMPTY
#endif /* DEBUG_VL53L0X_API */

static const FixPoint1616_t g_default_offset_calibration_distance = FIX1616_FROM_INT(DEFAULT_OFFSET_CALIB_DISTANCE_MM);

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
static void VL53L0X_API_GetTimeout (const eVl53l0xRangeProfile_t profile, uint32_t *timeout);
static bool VL53L0X_API_MeasureContinuous (const eVl53l0x_t vl53l0x, VL53L0X_RangingMeasurementData_t *ranging_data, const uint32_t start_tick, const uint32_t timeout);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
 
static bool VL53L0X_API_InitDevice (const eVl53l0x_t vl53l0x) {
    if (!VL53L0XV2_Config_IsCorrectVl53l0x(vl53l0x)) {
        return false;
    }

    if (eVl53l0xState_Off != g_dynamic_vl53l0x_lut[vl53l0x].state) {
        return true;
    }

    if (!I2C_API_Init(g_static_vl53l0x_lut[vl53l0x].i2c)) {
        return false;
    }

    if (g_static_vl53l0x_lut[vl53l0x].has_xshut_pin) {
        if (!GPIO_Driver_WritePin(g_static_vl53l0x_lut[vl53l0x].xshut_pin, false)) {
            return false;
        }
        
        osDelay(BOOT_PERIOD);
        
        if (!GPIO_Driver_WritePin(g_static_vl53l0x_lut[vl53l0x].xshut_pin, true)) {
            return false;
        }
        
        osDelay(BOOT_PERIOD);
    }

    g_dynamic_vl53l0x_lut[vl53l0x].device.I2cDevAddr = VL53L0X_DEFAULT_ADDRESS;

    VL53L0X_Error status = VL53L0X_ERROR_NONE;
    status = VL53L0X_DataInit(&g_dynamic_vl53l0x_lut[vl53l0x].device);

    if (VL53L0X_ERROR_NONE != status) {
        TRACE_ERR("InitDevice: DataInit failed for [%d], status [%d]\n", vl53l0x, status);
        
        return false;
    }

    status = VL53L0X_SetDeviceAddress(&g_dynamic_vl53l0x_lut[vl53l0x].device, (g_static_vl53l0x_lut[vl53l0x].i2c_address << 1));

    if (VL53L0X_ERROR_NONE != status) {
        TRACE_ERR("InitDevice: SetDeviceAddress failed for [%d], status [%d]\n", vl53l0x, status);
        
        return false;
    }

    g_dynamic_vl53l0x_lut[vl53l0x].device.I2cDevAddr = (g_static_vl53l0x_lut[vl53l0x].i2c_address);

    status = VL53L0X_StaticInit(&g_dynamic_vl53l0x_lut[vl53l0x].device);

    if (VL53L0X_ERROR_NONE != status) {
        TRACE_ERR("InitDevice: StaticInit failed for [%d], status [%d]\n", vl53l0x, status);

        return false;
    }

    g_dynamic_vl53l0x_lut[vl53l0x].state = eVl53l0xState_Init;

    return true;
}

// TODO: Make calib settings save to flash
static bool VL53L0X_API_ConfigureDevice (const eVl53l0x_t vl53l0x) {
    if (!VL53L0XV2_Config_IsCorrectVl53l0x(vl53l0x)) {
        return false;
    }

    if (eVl53l0xState_Init != g_dynamic_vl53l0x_lut[vl53l0x].state) {
        return false;
    }

    VL53L0X_Error status = VL53L0X_ERROR_NONE;
    
    if (!g_dynamic_vl53l0x_lut[vl53l0x].has_calib_data) {
        status = VL53L0X_PerformRefSpadManagement(&g_dynamic_vl53l0x_lut[vl53l0x].device, &g_dynamic_vl53l0x_lut[vl53l0x].calib_SpadCount, &g_dynamic_vl53l0x_lut[vl53l0x].calib_isApertureSpads);

        if (VL53L0X_ERROR_NONE != status) {
            TRACE_ERR("ConfigureDevice: PerformRefSpadManagement failed for [%d], status [%d]\n", vl53l0x, status);
            
            return false;
        }

        status = VL53L0X_PerformRefCalibration(&g_dynamic_vl53l0x_lut[vl53l0x].device, &g_dynamic_vl53l0x_lut[vl53l0x].calib_VhvSettings, &g_dynamic_vl53l0x_lut[vl53l0x].calib_PhaseCal);

        if (VL53L0X_ERROR_NONE != status) {
            TRACE_ERR("ConfigureDevice: PerformRefCalibration failed for [%d], status [%d]\n", vl53l0x, status);
            
            return false;
        }

        status = VL53L0X_PerformOffsetCalibration(&g_dynamic_vl53l0x_lut[vl53l0x].device, g_default_offset_calibration_distance, &g_dynamic_vl53l0x_lut[vl53l0x].offset);

        if (VL53L0X_ERROR_NONE != status) {
            TRACE_ERR("ConfigureDevice: PerformOffsetCalibration failed for [%d], status [%d]\n", vl53l0x, status);
            
            return false;
        } 

        if (g_static_vl53l0x_lut[vl53l0x].crosstalk_compensation_en) {
            status = VL53L0X_PerformXTalkCalibration(&g_dynamic_vl53l0x_lut[vl53l0x].device, g_static_vl53l0x_lut[vl53l0x].crosstalk_distance, &g_dynamic_vl53l0x_lut[vl53l0x].crosstalk_value); 

            if (VL53L0X_ERROR_NONE != status) {
                TRACE_ERR("ConfigureDevice: PerformXTalkCalibration failed for [%d], status [%d]\n", vl53l0x, status);
                
                return false;
            }
        }

        TRACE_INFO("ConfigureDevice: Calib for [%d]: spad=%u, aperture=%u, Vhv=%u, PhaseCal=%u, offset=%u\n", vl53l0x, g_dynamic_vl53l0x_lut[vl53l0x].calib_SpadCount, g_dynamic_vl53l0x_lut[vl53l0x].calib_isApertureSpads, g_dynamic_vl53l0x_lut[vl53l0x].calib_VhvSettings, g_dynamic_vl53l0x_lut[vl53l0x].calib_PhaseCal, g_dynamic_vl53l0x_lut[vl53l0x].offset);

        g_dynamic_vl53l0x_lut[vl53l0x].has_calib_data = true;
    }

    status = VL53L0X_SetReferenceSpads(&g_dynamic_vl53l0x_lut[vl53l0x].device, g_dynamic_vl53l0x_lut[vl53l0x].calib_SpadCount, g_dynamic_vl53l0x_lut[vl53l0x].calib_isApertureSpads); 

    if (VL53L0X_ERROR_NONE != status) {
        TRACE_ERR("ConfigureDevice: SetReferenceSpads failed for [%d], status [%d]\n", vl53l0x, status);
        
        return false;
    }

    status = VL53L0X_SetRefCalibration(&g_dynamic_vl53l0x_lut[vl53l0x].device, g_dynamic_vl53l0x_lut[vl53l0x].calib_VhvSettings, g_dynamic_vl53l0x_lut[vl53l0x].calib_PhaseCal); 

    if (VL53L0X_ERROR_NONE != status) {
        TRACE_ERR("ConfigureDevice: SetRefCalibration failed for [%d], status [%d]\n", vl53l0x, status);
        
        return false;
    }

    status = VL53L0X_SetOffsetCalibrationDataMicroMeter(&g_dynamic_vl53l0x_lut[vl53l0x].device, g_dynamic_vl53l0x_lut[vl53l0x].offset);

    if (VL53L0X_ERROR_NONE != status) {
        TRACE_ERR("ConfigureDevice: SetOffsetCalibrationDataMicroMeter failed for [%d], status [%d]\n", vl53l0x, status);
        
        return false;
    }

    if (g_static_vl53l0x_lut[vl53l0x].crosstalk_compensation_en) {
        status = VL53L0X_SetXTalkCompensationRateMegaCps(&g_dynamic_vl53l0x_lut[vl53l0x].device, g_dynamic_vl53l0x_lut[vl53l0x].crosstalk_value); 

        if (VL53L0X_ERROR_NONE != status) {
            TRACE_ERR("ConfigureDevice: SetXTalkCompensationRateMegaCps failed for [%d], status [%d]\n", vl53l0x, status);
            
            return false;
        }

        status = VL53L0X_SetXTalkCompensationEnable(&g_dynamic_vl53l0x_lut[vl53l0x].device, g_static_vl53l0x_lut[vl53l0x].crosstalk_compensation_en);
    
        if (VL53L0X_ERROR_NONE != status) {
            TRACE_ERR("ConfigureDevice: SetXTalkCompensationEnable failed for [%d], status [%d]\n", vl53l0x, status);
            
            return false;
        }
    }

    status = VL53L0X_SetDeviceMode(&g_dynamic_vl53l0x_lut[vl53l0x].device, g_static_vl53l0x_lut[vl53l0x].device_mode);

    if (VL53L0X_ERROR_NONE != status) {
        TRACE_ERR("ConfigureDevice: SetDeviceMode failed for [%d], status [%d]\n", vl53l0x, status);
        
        return false;
    }

    if (!VL53L0X_API_SetRangeProfile(vl53l0x, g_static_vl53l0x_lut[vl53l0x].range_profile)) {
        TRACE_ERR("ConfigureDevice: SetRangeProfile failed for [%d], status [%d]\n", vl53l0x, status);
        
        return false;
    }

    g_dynamic_vl53l0x_lut[vl53l0x].state = eVl53l0xState_SwStandby;

    return true;
}

static bool VL53L0X_API_SetRangeProfile (const eVl53l0x_t vl53l0x, const eVl53l0xRangeProfile_t profile) {
    if (!VL53L0XV2_Config_IsCorrectVl53l0x(vl53l0x)) {
        return false;
    }
    
    if ((profile < eVl53l0xRangeProfile_First) || (profile >= eVl53l0xRangeProfile_Last)) {
        return false;
    }

    float signal_rate = 0.0f;
    float sigma = 0.0f;
    uint32_t measurement_time = 0;

    switch (profile) {
        case eVl53l0xRangeProfile_Default: {
            signal_rate = 0.25f;
            sigma = 18;
            measurement_time = 33000;
        } break;
        case eVl53l0xRangeProfile_HighAccuracy: {
            signal_rate = 0.25f;
            sigma = 18;
            measurement_time = 200000;
        } break;
        case eVl53l0xRangeProfile_LongRange: {
            signal_rate = 0.1f;
            sigma = 60;
            measurement_time = 33000;
        } break;
        case eVl53l0xRangeProfile_HighSpeed: {
            signal_rate = 0.25f;
            sigma = 32;
            measurement_time = 20000;
        } break;
        case eVl53l0xRangeProfile_Custom: {
            signal_rate = CUSTOM_RANGE_PROFILE_SIGNAL_RATE;
            sigma = CUSTOM_RANGE_PROFILE_SIGMA;
            measurement_time = CUSTOM_RANGE_PROFILE_MEASUREMENT_TIME;
        } break;
        default: {
            return false;
        }
    }

    VL53L0X_Error status = VL53L0X_ERROR_NONE;

    status = VL53L0X_SetLimitCheckValue(&g_dynamic_vl53l0x_lut[vl53l0x].device, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, FIX1616_FROM_FLOAT(signal_rate));

    if (VL53L0X_ERROR_NONE != status) {
        TRACE_ERR("SetRangeProfile: SetLimitCheckValue failed for [%d], status [%d]\n", vl53l0x, status);
        
        return false;
    }

    status = VL53L0X_SetLimitCheckValue(&g_dynamic_vl53l0x_lut[vl53l0x].device, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, FIX1616_FROM_FLOAT(sigma));

    if (VL53L0X_ERROR_NONE != status) {
        TRACE_ERR("SetRangeProfile: SetLimitCheckValue failed for [%d], status [%d]\n", vl53l0x, status);
        
        return false;
    }

    status = VL53L0X_SetMeasurementTimingBudgetMicroSeconds(&g_dynamic_vl53l0x_lut[vl53l0x].device, measurement_time); 

    if (VL53L0X_ERROR_NONE != status) {
        TRACE_ERR("SetRangeProfile: SetMeasurementTimingBudgetMicroSeconds failed for [%d], status [%d]\n", vl53l0x, status);
        
        return false;
    }

    if (eVl53l0xRangeProfile_LongRange == profile) {
        status = VL53L0X_SetVcselPulsePeriod(&g_dynamic_vl53l0x_lut[vl53l0x].device, VL53L0X_VCSEL_PERIOD_PRE_RANGE, 18);

        if (VL53L0X_ERROR_NONE != status) {
            TRACE_ERR("SetRangeProfile: SetVcselPulsePeriod failed for [%d], status [%d]\n", vl53l0x, status);
            
            return false;
        }

        status = VL53L0X_SetVcselPulsePeriod(&g_dynamic_vl53l0x_lut[vl53l0x].device, VL53L0X_VCSEL_PERIOD_FINAL_RANGE, 14);
        
        if (VL53L0X_ERROR_NONE != status) {
            TRACE_ERR("SetRangeProfile: SetVcselPulsePeriod failed for [%d], status [%d]\n", vl53l0x, status);
            
            return false;
        }
    }

    return true;
}

static void VL53L0X_API_GetTimeout (const eVl53l0xRangeProfile_t profile, uint32_t *timeout) {
    if (NULL == timeout) {
        return;
    }
    
    switch (profile) {
        case eVl53l0xRangeProfile_Default: {
            if (*timeout < MIN_DEFAULT_TIMEOUT) {
                *timeout = MIN_DEFAULT_TIMEOUT;
            }
        } break;
        case eVl53l0xRangeProfile_HighAccuracy:{
            if (*timeout < MIN_HIGH_ACCURACY_TIMEOUT) {
                *timeout = MIN_HIGH_ACCURACY_TIMEOUT;
            }
        } break;
        case eVl53l0xRangeProfile_LongRange: {
            if (*timeout < MIN_LONG_RANGE_TIMEOUT) {
                *timeout = MIN_LONG_RANGE_TIMEOUT;
            }
        } break;
        case eVl53l0xRangeProfile_HighSpeed: {
            if (*timeout < MIN_HIGH_SPEED_TIMEOUT) {
                *timeout = MIN_HIGH_SPEED_TIMEOUT;
            }
        } break;
        case eVl53l0xRangeProfile_Custom: {
            if (*timeout < MIN_CUSTOM_TIMEOUT) {
                *timeout = MIN_CUSTOM_TIMEOUT;
            }
        } break;
        default: {
            if (*timeout < MIN_DEFAULT_TIMEOUT) {
                *timeout = MIN_DEFAULT_TIMEOUT;
            }
        } break;
    }
}

static bool VL53L0X_API_MeasureContinuous (const eVl53l0x_t vl53l0x, VL53L0X_RangingMeasurementData_t *ranging_data, const uint32_t start_tick, const uint32_t timeout) {
    if (!VL53L0XV2_Config_IsCorrectVl53l0x(vl53l0x)) {
        return false;
    }
    
    if (NULL == ranging_data) {
        return false;
    }

    VL53L0X_Error error = 0;
    uint8_t data_status = 0;

    uint32_t elapsed_ms = osKernelGetTickCount() - start_tick;
        
    while (elapsed_ms < timeout) {
        error = VL53L0X_GetMeasurementDataReady(&g_dynamic_vl53l0x_lut[vl53l0x].device, &data_status);

        if ((VL53L0X_ERROR_NONE == error) && (0 != data_status)) {
            break;
        }

        osDelay(NEXT_MEASUREMENT_POLL_DELAY);

        elapsed_ms = osKernelGetTickCount() - start_tick;
    }

    if (elapsed_ms > timeout) {
        TRACE_ERR("MeasureContinuous: Timeout for [%d], error [%d], status [%d], %lums elapsed\n", vl53l0x, error, data_status, elapsed_ms);
        
        return false;
    }

    if (0 == data_status) {
        TRACE_ERR("MeasureContinuous: Sensor [%d] data_status 0\n", vl53l0x);
        
        return false;
    }

    error = VL53L0X_GetRangingMeasurementData(&g_dynamic_vl53l0x_lut[vl53l0x].device, ranging_data);

    if (VL53L0X_ERROR_NONE != error) {
        TRACE_ERR("MeasureContinuous: GetRangingMeasurementData failed for [%d], error [%d]\n", vl53l0x, error);
        
        return false;
    }

    error = VL53L0X_ClearInterruptMask(&g_dynamic_vl53l0x_lut[vl53l0x].device, 0);

    if (VL53L0X_ERROR_NONE != error) {
        TRACE_ERR("MeasureContinuous: ClearInterruptMask failed for [%d], error [%d]\n", vl53l0x, error);
        
        return false;
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

        if ((NULL == static_desc) || (NULL == dynamic_desc)) {
            TRACE_ERR("InitAll: Failed to get [%d] description\n", vl53l0x);
            
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

    osDelay(BOOT_PERIOD);

    for (eVl53l0x_t vl53l0x = eVl53l0x_First; vl53l0x < eVl53l0x_Last; vl53l0x++) {
        if (!VL53L0X_API_InitDevice(vl53l0x)) {
            return false;
        }

        if (!VL53L0X_API_ConfigureDevice(vl53l0x)) {
            return false;
        }
    }

    g_is_initialized = true;

    return true;
}

bool VL53L0X_API_StartMeasuring (const eVl53l0x_t vl53l0x) {
    if (!VL53L0XV2_Config_IsCorrectVl53l0x(vl53l0x)) {
        return false;
    }

    if (eVl53l0xState_Off == g_dynamic_vl53l0x_lut[vl53l0x].state) {
        return false;
    }

    if (eVl53l0xState_Measuring == g_dynamic_vl53l0x_lut[vl53l0x].state) {
        return true;
    }

    if (VL53L0X_DEVICEMODE_CONTINUOUS_RANGING == g_static_vl53l0x_lut[vl53l0x].device_mode) {
        VL53L0X_Error status = VL53L0X_StartMeasurement(&g_dynamic_vl53l0x_lut[vl53l0x].device);
    
        if (VL53L0X_ERROR_NONE != status) {
            TRACE_ERR("StartMeasuring: StartMeasurement failed for [%d], status [%d]\n", vl53l0x, status);
            
            return false;
        }
    }

    g_dynamic_vl53l0x_lut[vl53l0x].state = eVl53l0xState_Measuring;

    return true;
}

bool VL53L0X_API_StopMeasuring (const eVl53l0x_t vl53l0x) {
    if (!VL53L0XV2_Config_IsCorrectVl53l0x(vl53l0x)) {
        return false;
    }

    if (eVl53l0xState_Off == g_dynamic_vl53l0x_lut[vl53l0x].state) {
        return false;
    }

    if (eVl53l0xState_SwStandby == g_dynamic_vl53l0x_lut[vl53l0x].state) {
        return true;
    }

    if (VL53L0X_DEVICEMODE_CONTINUOUS_RANGING == g_static_vl53l0x_lut[vl53l0x].device_mode) {
        uint32_t stop_status = 0;
        VL53L0X_Error error = VL53L0X_StopMeasurement(&g_dynamic_vl53l0x_lut[vl53l0x].device);

        if (VL53L0X_ERROR_NONE != error) {
            TRACE_ERR("StopMeasuring: StopMeasurement failed for [%d] [%d]\n", vl53l0x, error);
            
            return false;
        }

        uint32_t start_tick = osKernelGetTickCount();

        while ((osKernelGetTickCount() - start_tick) < DEFAULT_STOP_TIMEOUT_MS) {
            error = VL53L0X_GetStopCompletedStatus(&g_dynamic_vl53l0x_lut[vl53l0x].device, &stop_status);

            if ((VL53L0X_ERROR_NONE == error) && (0 == stop_status)) {
                break;
            }
        }

        if ((osKernelGetTickCount() - start_tick) > DEFAULT_STOP_TIMEOUT_MS) {
            TRACE_ERR("StopMeasuring: Timeout for [%d]; error [%d]\n", vl53l0x, error);
            
            return false;
        }
    }

    g_dynamic_vl53l0x_lut[vl53l0x].state = eVl53l0xState_SwStandby;

    return true;
}

// TODO: Analyze issue where after device power off->on cycle something breaks and device doesn't work properly
bool VL53L0X_API_TurnOff (const eVl53l0x_t vl53l0x) {
    if (!VL53L0XV2_Config_IsCorrectVl53l0x(vl53l0x)) {
        return false;
    }

    if (eVl53l0xState_Off == g_dynamic_vl53l0x_lut[vl53l0x].state) {
        return true;
    }

    if (eVl53l0xState_Measuring == g_dynamic_vl53l0x_lut[vl53l0x].state) {
        if (!VL53L0X_API_StopMeasuring(vl53l0x)) {
            TRACE_ERR("TurnOff: Failed to stop sensor [%d] before turning off\n", vl53l0x);
            
            return false;
        }
    }

    if (!GPIO_Driver_WritePin(g_static_vl53l0x_lut[vl53l0x].xshut_pin, false)) {
        return false;
    }

    memset(&g_dynamic_vl53l0x_lut[vl53l0x].device.Data, 0, sizeof(VL53L0X_DevData_t));
    g_dynamic_vl53l0x_lut[vl53l0x].state = eVl53l0xState_Off;

    return true;
}

bool VL53L0X_API_TurnOn (const eVl53l0x_t vl53l0x) {
    if (!VL53L0XV2_Config_IsCorrectVl53l0x(vl53l0x)) {
        return false;
    }

    if (eVl53l0xState_Off != g_dynamic_vl53l0x_lut[vl53l0x].state) {
        return true;
    }

    if (!VL53L0X_API_InitDevice(vl53l0x)) {
        return false;
    }

    if (!VL53L0X_API_ConfigureDevice(vl53l0x)) {
        return false;
    }

    g_dynamic_vl53l0x_lut[vl53l0x].state = eVl53l0xState_SwStandby;

    osDelay(BOOT_PERIOD);

    return true;
}

bool VL53L0X_API_GetDistance (const eVl53l0x_t vl53l0x, uint16_t *distance, uint32_t timeout) {
    if (!VL53L0XV2_Config_IsCorrectVl53l0x(vl53l0x)) {
        return false;
    }

    if (NULL == distance) {
        return false;
    }

    if (eVl53l0xState_Measuring != g_dynamic_vl53l0x_lut[vl53l0x].state) {
        return false;
    }

    VL53L0X_API_GetTimeout(g_static_vl53l0x_lut[vl53l0x].range_profile, &timeout);

    VL53L0X_RangingMeasurementData_t ranging_data = {0};
    uint32_t start_tick = osKernelGetTickCount();

    switch (g_static_vl53l0x_lut[vl53l0x].device_mode) {
        case VL53L0X_DEVICEMODE_SINGLE_RANGING: {
            VL53L0X_Error error = VL53L0X_PerformSingleRangingMeasurement(&g_dynamic_vl53l0x_lut[vl53l0x].device, &ranging_data);

            if (VL53L0X_ERROR_NONE != error) {
                TRACE_ERR("GetDistance: PerformSingleRangingMeasurement failed for [%d], error [%d]\n", vl53l0x, error);
                
                return false;
            }

            uint32_t elapsed_ms = osKernelGetTickCount() - start_tick;

            if (elapsed_ms > timeout) {
                TRACE_ERR("GetDistance: Timeout for [%d], error [%d], %lums elapsed\n", vl53l0x, error, elapsed_ms);
                
                return false;
            }
        } break;
        case VL53L0X_DEVICEMODE_CONTINUOUS_RANGING: {
            if (!VL53L0X_API_MeasureContinuous(vl53l0x, &ranging_data, start_tick, timeout)) {
                return false;
            }
        } break;
        default: {
            TRACE_ERR("GetDistance: Unknown device mode for [%d]\n", vl53l0x);

            return false;
        } break;
    }

    #if defined(DEBUG_VL53L0X_DETAILS)
    if ((0 != ranging_data.RangeStatus) || (ranging_data.RangeMilliMeter <= 10U)) {
        // Convert 16.16 fixpoint MCPS to milli-MCPS with rounding.
        int32_t sr_mmcps = (int32_t)((((int64_t)ranging_data.SignalRateRtnMegaCps * 1000) + 0x8000) >> 16);
        int32_t amb_mmcps = (int32_t)((((int64_t)ranging_data.AmbientRateRtnMegaCps * 1000) + 0x8000) >> 16);
        int32_t sigma_est_mmm = (int32_t)((((int64_t)g_dynamic_vl53l0x_lut[vl53l0x].device.Data.SigmaEstimate * 1000) + 0x8000) >> 16);

        // EffectiveSpadRtnCount is Q8.8, real value is /256.
        uint16_t eff_spad_int = (uint16_t)(ranging_data.EffectiveSpadRtnCount >> 8);
        uint16_t eff_spad_frac_2dp = (uint16_t)(((uint16_t)(ranging_data.EffectiveSpadRtnCount & 0xFFU) * 100U + 128U) / 256U);

        TRACE_INFO("Meas[%d]: d=%u status=%d dmax=%u SR=%ld.%03d amb=%ld.%03d sigmaEst=%d.%03d effSpad=%d.%02d\n", vl53l0x, ranging_data.RangeMilliMeter, ranging_data.RangeStatus, ranging_data.RangeDMaxMilliMeter, (int)(sr_mmcps / 1000), (int)(sr_mmcps < 0 ? -(sr_mmcps % 1000) : (sr_mmcps % 1000)), (int)(amb_mmcps / 1000), (int)(amb_mmcps < 0 ? -(amb_mmcps % 1000) : (amb_mmcps % 1000)), (int)(sigma_est_mmm / 1000), (int)(sigma_est_mmm < 0 ? -(sigma_est_mmm % 1000) : (sigma_est_mmm % 1000)), eff_spad_int, eff_spad_frac_2dp);
    }
    #endif

    if (0 != ranging_data.RangeStatus) {
        #if defined(DEBUG_VL53L0X_RANGE_STATUS)
        TRACE_ERR("GetDistance: sensor [%d] RangeStatus [%d]\n", vl53l0x, ranging_data.RangeStatus);
        #endif

        *distance = 0;
        
        return false;
    }

    *distance = ranging_data.RangeMilliMeter;

    return true;
}

#endif /* ENABLE_VL53L0X */
