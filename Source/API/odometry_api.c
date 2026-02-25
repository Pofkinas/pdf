/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <odometry_api.h>

#if defined(ENABLE_ODOMETRY)
#include <math.h>
#include "debug_api.h"
#include "motor_api.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef struct sEncoderData {
    int16_t rpm;
    uint32_t timestamp; // in ms
} sEncoderData_t;

typedef enum eOdometryState {
    eOdometryState_First = 0,
    eOdometryState_Init = eOdometryState_First,
    eOdometryState_Running,
    eOdometryState_WaitingData,
    eOdometryState_Stopped,
    eOdometryState_Last
} eOdometryState_t;

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

#if defined(DEBUG_ODOMETRY_API)
CREATE_MODULE_NAME (ODOMETRY_API)
#else
CREATE_MODULE_NAME_EMPTY
#endif /* DEBUG_ODOMETRY_API */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
 
static eOdometryState_t g_odometry_state = eOdometryState_Last;

static sOdometryData_t g_odometry_data = {0};
static sEncoderData_t g_encoder_data[eEncoder_Last] = {0};
static sEncoderDesc_t g_static_encoder_lut[eEncoder_Last] = {0};

static osTimerId_t g_odometry_timer_id = NULL;
static osMutexId_t g_odometry_mutex = NULL;
static osEventFlagsId_t g_odometry_event_flag = NULL;

static Odometry_API_RequestRPM_Callback g_request_rpm_callback = NULL;

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
 
static void Odometry_API_TimerCallback (void *arg);
static eOdomDataState_t Odometry_API_GetDataState (uint32_t stale_count);
static bool Odometry_API_UpdateOdometry (const int16_t right_rpm, const int16_t left_rpm, const sOdometryData_t old_data);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

static void Odometry_API_TimerCallback (void *arg) {
    if (eOdometryState_WaitingData == g_odometry_state) {
        if (osOK == osMutexAcquire(g_odometry_mutex, ODOMETRY_MUTEX_TIMEOUT)) {
            g_odometry_data.stale_count++;
            g_odometry_data.data_state = Odometry_API_GetDataState(g_odometry_data.stale_count);
            
            osMutexRelease(g_odometry_mutex);
        }
    }

    Odometry_API_RequestRPM();

    return;
}

static eOdomDataState_t Odometry_API_GetDataState (uint32_t stale_count) {
    if (eOdometryState_Last == g_odometry_state) {
        return eOdomDataState_Fault;
    }
    
    if (stale_count >= ODOMETRY_DATA_FAULT_THRESHOLD) {
        if (NULL != g_odometry_event_flag) {
            osEventFlagsSet(g_odometry_event_flag, ODOMETRY_DATA_FAULT_FLAG);
        }

        return eOdomDataState_Fault;
    } else if (stale_count >= ODOMETRY_DATA_STALE_THRESHOLD) {
        if (NULL != g_odometry_event_flag) {
            osEventFlagsSet(g_odometry_event_flag, ODOMETRY_DATA_STALE_FLAG);
        }
        
        return eOdomDataState_Stale;
    } else {
        return eOdomDataState_Good;
    }
}

static bool Odometry_API_UpdateOdometry (const int16_t right_rpm, const int16_t left_rpm, const sOdometryData_t old_data) {
    if (eOdometryState_WaitingData != g_odometry_state) {
        return false;
    }

    uint32_t current_time = osKernelGetTickCount();
    
    if (0 == old_data.timestamp) {
        if (osOK != osMutexAcquire(g_odometry_mutex, ODOMETRY_MUTEX_TIMEOUT)) {
            return false;
        }

        g_odometry_data.timestamp = current_time;
        g_odometry_state = eOdometryState_Running;
        
        osMutexRelease(g_odometry_mutex);

        if (NULL != g_odometry_event_flag) {
            osEventFlagsSet(g_odometry_event_flag, ODOMETRY_NEW_DATA_READY_FLAG);
        }
        
        return true;
    }
    
    uint32_t dt_ms = current_time - old_data.timestamp;
    float dt_s = dt_ms / 1000.0f;

    float right_vel = right_rpm * WHEEL_CIRCUMFERENCE_MM / 60.0f;
    float left_vel = left_rpm * WHEEL_CIRCUMFERENCE_MM / 60.0f;
    
    float linear_vel = (right_vel + left_vel) / 2.0f;
    float angular_vel = (right_vel - left_vel) / WHEEL_BASE_MM;
    
    float new_heading = old_data.heading + (angular_vel * dt_s);
    float delta_distance = linear_vel * dt_s;
    float new_x = old_data.x_pos + (delta_distance * cos(new_heading));
    float new_y = old_data.y_pos + (delta_distance * sin(new_heading));
    
    if (osOK != osMutexAcquire(g_odometry_mutex, ODOMETRY_MUTEX_TIMEOUT)) {
        return false;
    }

    g_odometry_data.x_pos = new_x;
    g_odometry_data.y_pos = new_y;
    g_odometry_data.heading = new_heading;
    g_odometry_data.linear_vel = linear_vel;
    g_odometry_data.angular_vel = angular_vel;
    g_odometry_data.distance += fabs(delta_distance);
    g_odometry_data.timestamp = current_time;
    g_odometry_data.stale_count = 0;
    g_odometry_data.data_state = Odometry_API_GetDataState(g_odometry_data.stale_count);

    g_odometry_state = eOdometryState_Running;

    osMutexRelease(g_odometry_mutex);

    if (NULL != g_odometry_event_flag) {
        osEventFlagsSet(g_odometry_event_flag, ODOMETRY_NEW_DATA_READY_FLAG);
    }

    TRACE_INFO("RPM: R: %d, L: %d | Pos: (%ld, %ld), Head: %ld deg, LinVel: %ld mm/s, AngVel: %ld deg/s\n", right_rpm, left_rpm, (int32_t)(new_x + 0.5f), (int32_t)(new_y + 0.5f), (int32_t)(Math_Utils_RadiansToDegrees(new_heading) + 0.5f), (int32_t)(linear_vel + 0.5f), (int32_t)(Math_Utils_RadiansToDegrees(angular_vel) + 0.5f));
    
    return true;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool Odometry_API_Init (osEventFlagsId_t *event_flags_id) {
    if (eOdometryState_Last != g_odometry_state) {
        return true;
    }

    for (eEncoder_t encoder = eEncoder_First; encoder < eEncoder_Last; encoder++) {
        const sEncoderDesc_t *encoder_desc = Odometry_Config_GetEncoderDesc(encoder);
        
        if (NULL == encoder_desc) {
            return false;
        }

        g_static_encoder_lut[encoder] = *encoder_desc;
    }

    g_odometry_timer_id = osTimerNew(Odometry_API_TimerCallback, osTimerPeriodic, NULL, &g_odometry_timer_attributes);

    if (NULL == g_odometry_timer_id) {
        return false;
    }

    g_odometry_mutex = osMutexNew(&g_odometry_mutex_attributes);

    if (NULL == g_odometry_mutex) {
        return false;
    }

    if (NULL != event_flags_id) {
        g_odometry_event_flag = *event_flags_id;
    }
    
    g_odometry_state = eOdometryState_Init;

    return true;
}

bool Odometry_API_Start (void) {
    if (eOdometryState_Last == g_odometry_state) {
        return false;
    }

    if (eOdometryState_Running == g_odometry_state) {
        return true;
    }

    if (osOK != osMutexAcquire(g_odometry_mutex, ODOMETRY_MUTEX_TIMEOUT)) {
        return false;
    }

    if (osOK != osTimerStart(g_odometry_timer_id, ODOMETRY_UPDATE_RATE_MS)) {
        return false;
    }

    g_odometry_state = eOdometryState_Running;

    osMutexRelease(g_odometry_mutex);

    return true;
}

bool Odometry_API_Stop (void) {
    if ((eOdometryState_Running != g_odometry_state) && (eOdometryState_WaitingData != g_odometry_state)) {
        return true;
    }

    if (osTimerIsRunning(g_odometry_timer_id)) {
        if (osOK != osTimerStop(g_odometry_timer_id)) {
            return false;
        }
    }

    if (osOK != osMutexAcquire(g_odometry_mutex, ODOMETRY_MUTEX_TIMEOUT)) {
        return false;
    }

    g_odometry_state = eOdometryState_Stopped;

    osMutexRelease(g_odometry_mutex);

    return true;
}

void Odometry_API_Reset (void) {
    if (eOdometryState_Last == g_odometry_state) {
        return;
    }

    if (osOK != osMutexAcquire(g_odometry_mutex, ODOMETRY_MUTEX_TIMEOUT)) {
        return;
    }

    memset(&g_odometry_data, 0, sizeof(sOdometryData_t));

    for (eEncoder_t encoder = eEncoder_First; encoder < eEncoder_Last; encoder++) {
        g_encoder_data[encoder].rpm = 0;
        g_encoder_data[encoder].timestamp = 0;
    }

    if (eOdometryState_Stopped == g_odometry_state) {
        g_odometry_state = eOdometryState_Init;
    }

    osMutexRelease(g_odometry_mutex);

    return;
}

bool Odometry_API_UpdateRpm (const uint16_t encoder_rpm[], const size_t encoder_count) {
    if (NULL == encoder_rpm) {
        return false;
    }

    /* TODO: Parse encoder IDs from arguments binary data.
     * arguments.size indicates how many encoder IDs are requested.
     * arguments.data first arg before separator is some size_t representing Encoder ID combination,
     * eg. 0b00000001 for ENCODER_RIGHT_1, 0b00000010 for ENCODER_LEFT_1, etc.
     * */

    if (eEncoder_Last != encoder_count) {
        return false;
    }

    int32_t right_rpm_sum = 0;
    int32_t left_rpm_sum = 0;
    uint8_t right_encoder_count = 0;
    uint8_t left_encoder_count = 0;

    uint32_t current_time = osKernelGetTickCount();
    eMotorRotation_t current_rotation = eMotorRotation_Last;

    if (osOK != osMutexAcquire(g_odometry_mutex, ODOMETRY_MUTEX_TIMEOUT)) {
        return false;
    }

    for (eEncoder_t encoder = eEncoder_First; encoder < eEncoder_Last; encoder++) {
        int16_t signed_rpm = (int16_t)encoder_rpm[encoder];
        
        if (Motor_API_GetMotorRotation(g_static_encoder_lut[encoder].motor, &current_rotation)) {
            if (g_static_encoder_lut[encoder].positive_dir != current_rotation) {
                signed_rpm = -signed_rpm;
            }
        }

        g_encoder_data[encoder].rpm = signed_rpm;
        g_encoder_data[encoder].timestamp = current_time;

        switch (g_static_encoder_lut[encoder].side) {
            case eEncoderSide_Right: {
                right_rpm_sum += signed_rpm;
                right_encoder_count++;
            } break;
            case eEncoderSide_Left: {
                left_rpm_sum += signed_rpm;
                left_encoder_count++;
            } break;
            default: {
                continue;
            }
        }
    }
    
    osMutexRelease(g_odometry_mutex);

    int16_t right_rpm_avg = (right_encoder_count > 0) ? (int16_t)(right_rpm_sum / right_encoder_count) : 0;
    int16_t left_rpm_avg = (left_encoder_count > 0) ? (int16_t)(left_rpm_sum / left_encoder_count) : 0;

    return Odometry_API_UpdateOdometry(right_rpm_avg, left_rpm_avg, g_odometry_data);
}

bool Odometry_API_RequestRpm (void) {
    if (eOdometryState_Last == g_odometry_state) {
        return false;
    }

    if (osOK != osMutexAcquire(g_odometry_mutex, ODOMETRY_MUTEX_TIMEOUT)) {
        return false;
    }

    g_odometry_state = eOdometryState_WaitingData;

    osMutexRelease(g_odometry_mutex);

    /* TODO: Make specific encoder RPM requests.
     * params.size indicates how many encoder IDs are requested.
     * params.data first arg before separator is some size_t representing Encoder ID combination,
     * eg. 0b00000001 for ENCODER_RIGHT_1, 0b00000010 for ENCODER_LEFT_1, etc.
     * */

    if (NULL == g_request_rpm_callback) {
        TRACE_ERR("Odometry RPM request callback not set\n");
        
        return false;
    }

    return g_request_rpm_callback();
}

void Odometry_API_SetRequestRpmCallback (Odometry_API_RequestRpm_Callback callback) {
    if (NULL == callback) {
        TRACE_ERR("Invalid Odometry RPM request callback\n");
        
        return;
    }
    
    g_request_rpm_callback = callback;

    return;
}

bool Odometry_API_GetRpm (const eEncoder_t encoder, int16_t *rpm) {
    if ((encoder < eEncoder_First) || (encoder >= eEncoder_Last)) {
        return false;
    }

    if (NULL == rpm) {
        return false;
    }
    
    if (eOdometryState_Last == g_odometry_state) {
        return false;
    }

    if (osOK != osMutexAcquire(g_odometry_mutex, ODOMETRY_MUTEX_TIMEOUT)) {
        return false;
    }

    *rpm = g_encoder_data[encoder].rpm;

    osMutexRelease(g_odometry_mutex);

    return true;
}

void Odometry_API_SetEventFlag (osEventFlagsId_t *event_flags_id) {
    if (NULL == event_flags_id) {
        TRACE_ERR("Invalid event flags ID pointer\n");
        
        return;
    }
    
    g_odometry_event_flag = *event_flags_id;

    return;
}

osEventFlagsId_t *Odometry_API_GetEventFlag (void) {
    return &g_odometry_event_flag;
}

sOdometryData_t *Odometry_API_GetDataPointer (void) {
    return &g_odometry_data;
}

#endif /* ENABLE_ODOMETRY */
