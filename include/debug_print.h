/**
 * @file     debug_print.h
 * @brief    Debug print driver for S32K344 using LPUART6
 * @details  Provides debug print functionality with printf-like capabilities
 */

#ifndef DEBUG_PRINT_H
#define DEBUG_PRINT_H

#ifdef __cplusplus
extern "C"
{
#endif

/*==================================================================================================
*                                        INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
==================================================================================================*/
#include <stdarg.h>
#include <stdint.h>
#include "Std_Types.h"

/*==================================================================================================
*                                       DEFINES AND MACROS
==================================================================================================*/

/** @brief Maximum length of a single debug print message */
#define DEBUG_PRINT_MAX_MESSAGE_LENGTH  (256U)

/*==================================================================================================
*                                              ENUMS
==================================================================================================*/

/*==================================================================================================
*                                  STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

/*==================================================================================================
*                                  GLOBAL VARIABLE DECLARATIONS
==================================================================================================*/

/*==================================================================================================
*                                       FUNCTION PROTOTYPES
==================================================================================================*/

/**
 * @brief Initialize the debug print functionality
 *
 * This function initializes LPUART6 for debug output with default settings.
 * Must be called before any debug print functions.
 *
 * @return void
 */
void debug_print_init(void);

/**
 * @brief Print formatted string to debug output (printf-like)
 *
 * @param format   Format string (like printf)
 * @param ...      Variable arguments
 *
 * @return void
 */
void debug_print(const char *format, ...);

/**
 * @brief Print string to debug output
 *
 * @param str   Null-terminated string to print
 *
 * @return void
 */
void debug_print_string(const char *str);

/**
 * @brief Print buffer content in hexadecimal format
 *
 * @param buffer    Pointer to buffer
 * @param length    Length of buffer in bytes
 * @param prefix    Optional prefix string (can be NULL)
 *
 * @return void
 */
void debug_print_hex(const uint8_t *buffer, uint32_t length, const char *prefix);

/**
 * @brief Print a single character to debug output
 *
 * @param character    Character to print
 *
 * @return void
 */
void debug_print_char(char character);

/**
 * @brief Print newline to debug output
 *
 * @return void
 */
void debug_print_newline(void);

#ifdef __cplusplus
}
#endif

#endif /* DEBUG_PRINT_H */
