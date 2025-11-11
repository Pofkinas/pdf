/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "vl53l0xv2_config.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

/* clang-format off */
static const sVl53l0xStaticDesc_t g_static_vl53l0x_lut[eVl53l0x_Last] = {
    [eVl53l0x_1] = {
        .i2c = I2C_1,
        .i2c_address = VL53L0X_1_I2C_ADDRESS,
        .crosstalk_talk_compensation_en = 0,
        .crosstalk_talk_distance = DEFAULT_CROSSTALK_CALIB_DISTANCE_MM * 65536,
        .device_mode = VL53L0X_DEVICEMODE_CONTINUOUS_RANGING,
        .has_xshut_pin = true,
        .xshut_pin = eGpio_Vl53l0_Xshut_1,
        .range_profile = eVl53l0xRangeProfile_LongRange
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

/* clang-format off */
static sVl53l0xDynamicDesc_t g_dynamic_vl53l0x_lut[eVl53l0x_Last] = {
    [eVl53l0x_1] = {
        .device = {.I2cDevAddr = VL53L0X_DEFAULT_ADDRESS, .comms_type = I2C, .comms_speed_khz = I2C_1_CLOCK_SPEED / 1000},
        .state = eVl53l0xState_Off,
        .has_calib_default_data = true,
        .calib_SpadCount = 5,
        .calib_isApertureSpads = 0,
        .calib_VhvSettings = 28,
        .calib_PhaseCal = 1,
        .offset = 92000,
    }
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

bool VL53L0XV2_Config_IsCorrectVl53l0x (const eVl53l0x_t vl53l0x_device) {
    return (vl53l0x_device >= eVl53l0x_First) && (vl53l0x_device < eVl53l0x_Last);
}

const sVl53l0xStaticDesc_t *VL53L0XV2_Config_GetVl53l0xStaticDesc (const eVl53l0x_t vl53l0x_device) {
    if (!VL53L0XV2_Config_IsCorrectVl53l0x(vl53l0x_device)) {
        return NULL;
    }

    return &g_static_vl53l0x_lut[vl53l0x_device];
}

const sVl53l0xDynamicDesc_t *VL53L0XV2_Config_GetVl53l0xDynamicDesc (const eVl53l0x_t vl53l0x_device) {
    if (!VL53L0XV2_Config_IsCorrectVl53l0x(vl53l0x_device)) {
        return NULL;
    }

    return &g_dynamic_vl53l0x_lut[vl53l0x_device];
}
