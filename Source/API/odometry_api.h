#ifndef SOURCE_API_ODOMETRY_H_
#define SOURCE_API_ODOMETRY_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#ifdef ENABLE_ODOMETRY
#include <stdbool.h>
#include <stdint.h>
#include "cmsis_os2.h"
#include "odometry_config.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

// TODO: Implement internal rpm counting logic

typedef enum eOdomDataState {
    eOdomDataState_First = 0,
    eOdomDataState_Good = eOdomDataState_First,
    eOdomDataState_Stale,
    eOdomDataState_Fault,
    eOdomDataState_Last
} eOdomDataState_t;

typedef struct sOdometryData {
    float x_pos; // in mm
    float y_pos; // in mm
    float heading; // in radians
    float linear_vel; // in mm/s
    float angular_vel; // in rad/s
    float distance; // in mm
    uint32_t timestamp; // in ms
    uint32_t stale_count;
    eOdomDataState_t data_state;
} sOdometryData_t;

typedef bool (*Odometry_API_RequestRPM_Callback)(void);

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool Odometry_API_Init (osEventFlagsId_t *event_flags_id);
bool Odometry_API_Start (void);
bool Odometry_API_Stop (void); 
void Odometry_API_Reset (void);
bool Odometry_API_UpdateRpm (const uint16_t encoder_rpm[], const size_t encoder_count);
bool Odometry_API_RequestRPM (void);
void Odometry_API_SetRequestRpmCallback (Odometry_API_RequestRPM_Callback callback);
bool Odometry_API_GetRPM (const eEncoder_t encoder, int16_t *rpm);
void Odometry_API_SetEventFlag (osEventFlagsId_t *event_flags_id);
osEventFlagsId_t *Odometry_API_GetEventFlag (void);
sOdometryData_t *Odometry_API_GetDataPointer (void);

#endif /* ENABLE_ODOMETRY */
#endif /* SOURCE_API_ODOMETRY_H_ */
