/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "vl53l0xv2_api.h"

#ifdef USE_VL53L0X

#include "cmsis_os2.h"
#include "vl53l0x_api.h"
#include "i2c_api.h"
#include "debug_api.h"
#include "gpio_driver.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

#define DEBUG_VL53L0X_API

#define VL53L0X_DEFAULT_ADDRESS 0x29

#define MIN_DEFAULT_TIMEOUT 50
#define MIN_LONG_RANGE_TIMEOUT 55
#define MIN_HIGH_ACCURACY_TIMEOUT 300
#define MIN_HIGH_SPEED_TIMEOUT 35

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef enum eVl53l0xRangeProfile {
    eVl53l0xRangeProfile_First = 0,
    eVl53l0xRangeProfile_Default = eVl53l0xRangeProfile_First,
    eVl53l0xRangeProfile_HighAccuracy,
    eVl53l0xRangeProfile_LongRange,
    eVl53l0xRangeProfile_HighSpeed,
    eVl53l0xRangeProfile_Last
} eVl53l0xRangeProfile_t;

typedef enum eVl53l0xState {
    eVl53l0xState_First = 0,
    eVl53l0xState_Off = eVl53l0xState_First,
    eVl53l0xState_Init,
    eVl53l0xState_Standby,
    eVl53l0xState_Measuring,
    eVl53l0xState_Last
} eVl53l0xState_t;

typedef struct sVl53l0xStaticDesc {
    eI2c_t i2c;
    uint8_t i2c_address;
    uint8_t crosstalk_talk_compensation_en;
    FixPoint1616_t crosstalk_talk_distance;
    VL53L0X_DeviceModes device_mode;
    bool has_xshut_pin;
    eGpioPin_t xshut_pin;
    eVl53l0xRangeProfile_t range_profile;
} sVl53l0xStaticDesc_t;

typedef struct sVl53l0xDynamicDesc {
    VL53L0X_Dev_t device;
    eVl53l0xState_t state;
    bool has_calib_default_data;
    uint32_t calib_SpadCount;
	uint8_t calib_isApertureSpads;
    uint8_t calib_VhvSettings;
	uint8_t calib_PhaseCal;
    int32_t offset;
    FixPoint1616_t crosstalk_value;
} sVl53l0xDynamicDesc_t;

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

#ifdef DEBUG_VL53L0X_API 
CREATE_MODULE_NAME (VL53L0XV2_API)
#else
CREATE_MODULE_NAME_EMPTY
#endif

static const FixPoint1616_t g_default_offset_calibration_distance = (uint32_t) DEFAULT_OFFSET_CALIB_DISTANCE_MM * 65536;

/* clang-format off */
static const sVl53l0xStaticDesc_t g_static_vl53l0x_lut[eVl53l0x_Last] = {
    #ifdef USE_VL53L0X_1
    [eVl53l0x_1] = {
        .i2c = eI2c_1,
        .i2c_address = 0x62,
        .crosstalk_talk_compensation_en = 0,
        .crosstalk_talk_distance = DEFAULT_CROSSTALK_CALIB_DISTANCE_MM * 65536,
        .device_mode = VL53L0X_DEVICEMODE_CONTINUOUS_RANGING,
        #ifdef USE_VL53L0_XSHUT1
        .has_xshut_pin = true,
        .xshut_pin = eGpioPin_vl53l0_Xshut_1,
        #else
        .has_xshut_pin = false,
        #endif
        .range_profile = RANGING_PROFILE_VL53L0_1
    },
    #endif

    #ifdef USE_VL53L0X_2
    [eVl53l0x_2] = {
        .i2c = eI2c_1,
        .i2c_address = 0x63,
        .crosstalk_talk_compensation_en = 0,
        .crosstalk_talk_distance = DEFAULT_CROSSTALK_CALIB_DISTANCE_MM * 65536,
        .device_mode = VL53L0X_DEVICEMODE_CONTINUOUS_RANGING,
        #ifdef USE_VL53L0_XSHUT2
        .has_xshut_pin = true,
        .xshut_pin = eGpioPin_vl53l0_Xshut_2,
        #else
        .has_xshut_pin = false,
        #endif
        .range_profile = RANGING_PROFILE_VL53L0_2
    },
    #endif
};
/* clang-format on */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

/* clang-format off */
static sVl53l0xDynamicDesc_t g_dynamic_vl53l0x[eVl53l0x_Last] = {
    #ifdef USE_VL53L0X_1
    [eVl53l0x_1] = {
        .device = {.I2cDevAddr = VL53L0X_DEFAULT_ADDRESS, .comms_type = I2C, .comms_speed_khz = 100},
        .state = eVl53l0xState_Off,
        .has_calib_default_data = true,
        .calib_SpadCount = 5,
        .calib_isApertureSpads = 0,
        .calib_VhvSettings = 28,
        .calib_PhaseCal = 1,
        .offset = 92000,
    },
    #endif

    #ifdef USE_VL53L0X_2
    [eVl53l0x_2] = {
        .device = {.I2cDevAddr = VL53L0X_DEFAULT_ADDRESS, .comms_type = I2C, .comms_speed_khz = 100},
        .state = eVl53l0xState_Off,
        .has_calib_default_data = true,
        .calib_SpadCount = 5,
        .calib_isApertureSpads = 0,
        .calib_VhvSettings = 28,
        .calib_PhaseCal = 1,
        .offset = 92000,
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

static bool VL53L0X_API_InitDevice (const eVl53l0x_t vl53l0x);
static bool VL53L0X_API_ConfigureDevice (const eVl53l0x_t vl53l0x);
static bool VL53L0X_API_SetRangeProfile (const eVl53l0x_t vl53l0x, const eVl53l0xRangeProfile_t profile);
static bool VL53L0X_API_IsCorrectDevice (const eVl53l0x_t vl53l0x);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
 
static bool VL53L0X_API_InitDevice (const eVl53l0x_t vl53l0x) {
    if (!VL53L0X_API_IsCorrectDevice(vl53l0x)) {
        return false;
    }

    if (g_dynamic_vl53l0x[vl53l0x].state != eVl53l0xState_Off) {
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

    if (VL53L0X_DataInit(&g_dynamic_vl53l0x[vl53l0x].device) != VL53L0X_ERROR_NONE) {
        TRACE_ERR("VL53L0X_DataInit failed\n");
        
        return false;
    }

    if (VL53L0X_SetDeviceAddress(&g_dynamic_vl53l0x[vl53l0x].device, (g_static_vl53l0x_lut[vl53l0x].i2c_address)) != VL53L0X_ERROR_NONE) {
        return false;
    }

    g_dynamic_vl53l0x[vl53l0x].device.I2cDevAddr = (g_static_vl53l0x_lut[vl53l0x].i2c_address >> 1);

    if (VL53L0X_StaticInit(&g_dynamic_vl53l0x[vl53l0x].device) != VL53L0X_ERROR_NONE) {
        TRACE_ERR("VL53L0X_StaticInit failed\n");

        return false;
    }

    g_dynamic_vl53l0x[vl53l0x].state = eVl53l0xState_Init;

    return true;
}

static bool VL53L0X_API_ConfigureDevice (const eVl53l0x_t vl53l0x) {
    // TODO: Make calib settings save to flash

    if (!VL53L0X_API_IsCorrectDevice(vl53l0x)) {
        return false;
    }

    if (g_dynamic_vl53l0x[vl53l0x].state != eVl53l0xState_Init) {
        return false;
    }
    
    if (!g_dynamic_vl53l0x[vl53l0x].has_calib_default_data) {
        if (VL53L0X_PerformRefSpadManagement(&g_dynamic_vl53l0x[vl53l0x].device, &g_dynamic_vl53l0x[vl53l0x].calib_SpadCount, &g_dynamic_vl53l0x[vl53l0x].calib_isApertureSpads) != VL53L0X_ERROR_NONE) {
            TRACE_ERR("VL53L0X_PerformRefSpadManagement failed\n");
            
            return false;
        }

        if (VL53L0X_PerformRefCalibration(&g_dynamic_vl53l0x[vl53l0x].device, &g_dynamic_vl53l0x[vl53l0x].calib_VhvSettings, &g_dynamic_vl53l0x[vl53l0x].calib_PhaseCal) != VL53L0X_ERROR_NONE) {
            TRACE_ERR("VL53L0X_PerformRefCalibration failed\n");
            
            return false;
        }

        if (VL53L0X_PerformOffsetCalibration(&g_dynamic_vl53l0x[vl53l0x].device, g_default_offset_calibration_distance, &g_dynamic_vl53l0x[vl53l0x].offset) != VL53L0X_ERROR_NONE) {
            TRACE_ERR("VL53L0X_PerformOffsetCalibration failed\n");
            
            return false;
        }

        if (g_static_vl53l0x_lut[vl53l0x].crosstalk_talk_compensation_en) {
            VL53L0X_PerformXTalkCalibration(&g_dynamic_vl53l0x[vl53l0x].device, g_static_vl53l0x_lut[vl53l0x].crosstalk_talk_distance, &g_dynamic_vl53l0x[vl53l0x].crosstalk_value);
        
        }
    }

    if (VL53L0X_SetReferenceSpads(&g_dynamic_vl53l0x[vl53l0x].device, g_dynamic_vl53l0x[vl53l0x].calib_SpadCount, g_dynamic_vl53l0x[vl53l0x].calib_isApertureSpads) != VL53L0X_ERROR_NONE) {
        TRACE_ERR("VL53L0X_SetReferenceSpads failed\n");
        
        return false;
    }

    if (VL53L0X_SetRefCalibration(&g_dynamic_vl53l0x[vl53l0x].device, g_dynamic_vl53l0x[vl53l0x].calib_VhvSettings, g_dynamic_vl53l0x[vl53l0x].calib_PhaseCal) != VL53L0X_ERROR_NONE) {
        TRACE_ERR("VL53L0X_SetRefCalibration failed\n");
        
        return false;
    }

    if (VL53L0X_SetOffsetCalibrationDataMicroMeter(&g_dynamic_vl53l0x[vl53l0x].device, g_dynamic_vl53l0x[vl53l0x].offset) != VL53L0X_ERROR_NONE) {
        TRACE_ERR("VL53L0X_SetOffsetCalibrationDataMicroMeter failed\n");
        
        return false;
    }

    if (g_static_vl53l0x_lut[vl53l0x].crosstalk_talk_compensation_en) {
        if (VL53L0X_SetXTalkCompensationRateMegaCps(&g_dynamic_vl53l0x[vl53l0x].device, g_dynamic_vl53l0x[vl53l0x].crosstalk_value) != VL53L0X_ERROR_NONE) {
            TRACE_ERR("VL53L0X_SetXTalkCompensationRateMegaCps failed\n");
            
            return false;
        }
    
        if (VL53L0X_SetXTalkCompensationEnable(&g_dynamic_vl53l0x[vl53l0x].device, g_static_vl53l0x_lut[vl53l0x].crosstalk_talk_compensation_en) != VL53L0X_ERROR_NONE) {
            TRACE_ERR("VL53L0X_SetXTalkCompensationEnable failed\n");
            
            return false;
        }
    }

    if (VL53L0X_SetDeviceMode(&g_dynamic_vl53l0x[vl53l0x].device, g_static_vl53l0x_lut[vl53l0x].device_mode) != VL53L0X_ERROR_NONE) {
        TRACE_ERR("VL53L0X_SetDeviceMode failed\n");
        
        return false;
    }

    if (!VL53L0X_API_SetRangeProfile(vl53l0x, g_static_vl53l0x_lut[vl53l0x].range_profile)) {
        TRACE_ERR("VL53L0X_API_SetRangeProfile failed\n");
        
        return false;
    }

    g_dynamic_vl53l0x[vl53l0x].state = eVl53l0xState_Standby;

    return true;
}

static bool VL53L0X_API_SetRangeProfile (const eVl53l0x_t vl53l0x, const eVl53l0xRangeProfile_t profile) {
    if (!VL53L0X_API_IsCorrectDevice(vl53l0x)) {
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

    if (VL53L0X_SetLimitCheckValue(&g_dynamic_vl53l0x[vl53l0x].device, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, (FixPoint1616_t)(signal_rate_multiplyer * 65536)) != VL53L0X_ERROR_NONE) {
        return false;
    }

    if (VL53L0X_SetLimitCheckValue(&g_dynamic_vl53l0x[vl53l0x].device, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, (FixPoint1616_t)(sigma_multiplyer * 65536)) != VL53L0X_ERROR_NONE) {
        return false;
    }
    if (VL53L0X_SetMeasurementTimingBudgetMicroSeconds(&g_dynamic_vl53l0x[vl53l0x].device, measurement_time) != VL53L0X_ERROR_NONE) {
        return false;
    }

    if (profile == eVl53l0xRangeProfile_LongRange) {
        if (VL53L0X_SetVcselPulsePeriod(&g_dynamic_vl53l0x[vl53l0x].device, VL53L0X_VCSEL_PERIOD_PRE_RANGE, 18) != VL53L0X_ERROR_NONE) {
            return false;
        }
        if (VL53L0X_SetVcselPulsePeriod(&g_dynamic_vl53l0x[vl53l0x].device, VL53L0X_VCSEL_PERIOD_FINAL_RANGE, 14) != VL53L0X_ERROR_NONE) {
            return false;
        }
    }

    return true;
}

static bool VL53L0X_API_IsCorrectDevice (const eVl53l0x_t vl53l0x) {
    return (vl53l0x > eVl53l0x_First) && (vl53l0x < eVl53l0x_Last);
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool VL53L0X_API_InitAll (void) {
    if (eVl53l0x_Last == 1) { 
        return false;
    }
    
    if (!GPIO_Driver_InitAllPins()) {
        return false;
    }

    for (eVl53l0x_t vl53l0x = (eVl53l0x_First + 1); vl53l0x < eVl53l0x_Last; vl53l0x++) {
        if (!g_static_vl53l0x_lut[vl53l0x].has_xshut_pin) {
            continue;
        }
        
        if (!GPIO_Driver_WritePin(g_static_vl53l0x_lut[vl53l0x].xshut_pin, false)) {
            return false;
        }
    }

    osDelay(1);

    for (eVl53l0x_t vl53l0x = (eVl53l0x_First + 1); vl53l0x < eVl53l0x_Last; vl53l0x++) {
        if (!VL53L0X_API_InitDevice(vl53l0x)) {
            return false;
        }
    }

    for (eVl53l0x_t vl53l0x = (eVl53l0x_First + 1); vl53l0x < eVl53l0x_Last; vl53l0x++) {
        if (!VL53L0X_API_ConfigureDevice(vl53l0x)) {
            return false;
        }
    }

    return true;
}

bool VL53L0X_API_StartMeasuring (const eVl53l0x_t vl53l0x) {
    if (!VL53L0X_API_IsCorrectDevice(vl53l0x)) {
        return false;
    }

    if (g_dynamic_vl53l0x[vl53l0x].state == eVl53l0xState_Off) {
        return false;
    }

    if (g_dynamic_vl53l0x[vl53l0x].state == eVl53l0xState_Measuring) {
        return true;
    }

    if (VL53L0X_StartMeasurement(&g_dynamic_vl53l0x[vl53l0x].device) != VL53L0X_ERROR_NONE) {
        TRACE_ERR("VL53L0X_StartMeasurement failed\n");
        
        return false;
    }

    g_dynamic_vl53l0x[vl53l0x].state = eVl53l0xState_Measuring;

    return true;
}

bool VL53L0X_API_StopMeasuring (const eVl53l0x_t vl53l0x) {
    if (!VL53L0X_API_IsCorrectDevice(vl53l0x)) {
        return false;
    }

    if (g_dynamic_vl53l0x[vl53l0x].state == eVl53l0xState_Off) {
        return false;
    }

    if (g_dynamic_vl53l0x[vl53l0x].state == eVl53l0xState_Standby) {
        return true;
    }

    uint32_t stop_status = 0;

    VL53L0X_Error error = VL53L0X_StopMeasurement(&g_dynamic_vl53l0x[vl53l0x].device);

    if (error != VL53L0X_ERROR_NONE) {
        TRACE_ERR("VL53L0X_StopMeasurement failed [%d]\n", error);
        
        return false;
    }

    uint32_t timeout = DEFAULT_STOP_TIMEOUT * SYSTEM_MS_TICS;
    uint32_t start_tick = osKernelGetSysTimerCount();

    while ((osKernelGetSysTimerCount() - start_tick) < timeout) {
        error = VL53L0X_GetStopCompletedStatus(&g_dynamic_vl53l0x[vl53l0x].device, &stop_status);

        if (error == VL53L0X_ERROR_NONE && stop_status == 0) {
            break;
        }
    }

    if ((osKernelGetSysTimerCount() - start_tick) >= timeout) {
        TRACE_ERR("VL53L0X_API_Disable timeout [%d]\n", error);
        
        return false;
    }

    g_dynamic_vl53l0x[vl53l0x].state = eVl53l0xState_Standby;

    return true;
}

bool VL53L0X_API_TurnOff (const eVl53l0x_t vl53l0x) {
    if (!VL53L0X_API_IsCorrectDevice(vl53l0x)) {
        return false;
    }

    if (g_dynamic_vl53l0x[vl53l0x].state == eVl53l0xState_Off) {
        return true;
    }

    if (g_dynamic_vl53l0x[vl53l0x].state == eVl53l0xState_Measuring) {
        if (!VL53L0X_API_StopMeasuring(vl53l0x)) {
            TRACE_ERR("Failed to stop sensor before turning off\n");
            return false;
        }
    }

    GPIO_Driver_WritePin(g_static_vl53l0x_lut[vl53l0x].xshut_pin, false);

    g_dynamic_vl53l0x[vl53l0x].state = eVl53l0xState_Off;

    return true;
}

bool VL53L0X_API_TurnOn (const eVl53l0x_t vl53l0x) {
    if (!VL53L0X_API_IsCorrectDevice(vl53l0x)) {
        return false;
    }

    if (g_dynamic_vl53l0x[vl53l0x].state != eVl53l0xState_Off) {
        return true;
    }

    if (!VL53L0X_API_InitDevice(vl53l0x)) {
        return false;
    }

    g_dynamic_vl53l0x[vl53l0x].state = eVl53l0xState_Standby;

    return true;
}

bool VL53L0X_API_GetDistance (const eVl53l0x_t vl53l0x, uint16_t *distance, size_t timeout) {
    if (!VL53L0X_API_IsCorrectDevice(vl53l0x)) {
        return false;
    }

    if (distance == NULL) {
        return false;
    }

    if (g_dynamic_vl53l0x[vl53l0x].state != eVl53l0xState_Measuring) {
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
        error = VL53L0X_GetMeasurementDataReady(&g_dynamic_vl53l0x[vl53l0x].device, &data_status);

        if (error == VL53L0X_ERROR_NONE && data_status != 0) {
            break;
        }

        osDelay(10);
    }

    if ((osKernelGetSysTimerCount() - start_tick) >= timeout) {
        TRACE_ERR("VL53L0X_API_GetDistance timeout [%d]\n", error);
        
        return false;
    }

    if (data_status == 0) {
        return false;
    }

    error = VL53L0X_GetRangingMeasurementData(&g_dynamic_vl53l0x[vl53l0x].device, &ranging_data);

    if (error != VL53L0X_ERROR_NONE) {
        TRACE_ERR("VL53L0X_GetRangingMeasurementData failed [%d]\n", error);
        
        return false;
    }

    error = VL53L0X_ClearInterruptMask(&g_dynamic_vl53l0x[vl53l0x].device, 0);

    if (error != VL53L0X_ERROR_NONE) {
        TRACE_ERR("VL53L0X_ClearInterruptMask failed [%d]\n", error);
        
        return false;
    }

    if (ranging_data.RangeStatus != 0) {
        //TRACE_ERR("RangeStatus: [%d]\n", ranging_data.RangeStatus);

        *distance = 0;
        
        return false;
    }

    *distance = ranging_data.RangeMilliMeter;

    return true;
}

#endif
