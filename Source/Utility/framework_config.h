#ifndef SOURCE_UTILITY_FRAMEWORK_CONFIG_H_
#define SOURCE_UTILITY_FRAMEWORK_CONFIG_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#ifndef PROJECT_CONFIG_H
#include "example_config.h"
#error "PROJECT_CONFIG_H not defined. Add -DPROJECT_CONFIG_H=\"platform_config.h\" to your build."
#endif

#include PROJECT_CONFIG_H
#define SYSTEM_MS_TICS (SYSTEM_CLOCK_HZ / 1000)

/**********************************************************************************************************************
 * Project configuration analysis
 *********************************************************************************************************************/

#if defined(USE_MOTOR) && defined(USE_PWM_LED)
#error "USE_MOTOR and USE_PWM_LED cannot be used together."
#endif

#endif /* SOURCE_UTILITY_FRAMEWORK_CONFIG_H_ */
