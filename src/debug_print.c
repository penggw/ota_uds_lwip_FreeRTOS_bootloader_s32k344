/**
 * @file     debug_print.c
 * @brief    Debug print driver implementation using MCAL UART (LPUART6)
 * @details  Provides debug print functionality using Uart_AsyncSend API
 */

/*==================================================================================================
*                                        INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
==================================================================================================*/
#include "debug_print.h"
#include "CDD_Uart.h"

/* Extern declaration for predefined UART configuration */
extern const Uart_ConfigType* const Uart_pxPBcfgVariantPredefined[UART_MAX_PARTITIONS];

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/*==================================================================================================
*                                       DEFINES AND MACROS
==================================================================================================*/

/** @brief UART Channel used for debug output (maps to LPUART6) */
#define DEBUG_PRINT_UART_CHANNEL     (0U)

/** @brief Internal buffer size for formatted strings */
#define DEBUG_PRINT_BUFFER_SIZE      (DEBUG_PRINT_MAX_MESSAGE_LENGTH)

/*==================================================================================================
                                       LOCAL VARIABLES
==================================================================================================*/

/** @brief Internal buffer for formatting strings */
static char debug_print_buffer[DEBUG_PRINT_BUFFER_SIZE];

/** @brief Track if UART is initialized */
static boolean debug_print_initialized = FALSE;

/*==================================================================================================
                                   LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

/**
 * @brief Wait for UART transmission to complete
 *
 * @param timeout_us    Timeout in microseconds
 * @return boolean     TRUE if transmission completed, FALSE on timeout
 */
static boolean debug_print_wait_transmit_complete(uint32 timeout_us);

/**
 * @brief Newlib system call to redirect printf output to debug UART
 *
 * This function implements the _write system call used by newlib's C library
 * for functions like printf, puts, etc. All standard output is redirected
 * to the debug UART through the debug_print_char function.
 *
 * @param file    File descriptor (1=stdout, 2=stderr) - ignored
 * @param ptr     Pointer to buffer containing characters to write
 * @param len     Number of characters to write
 *
 * @return int    Number of characters written, or -1 on error
 */
int _write(int file, char *ptr, int len);

/*==================================================================================================
                                       GLOBAL FUNCTIONS
==================================================================================================*/

void debug_print_init(void)
{
    const Uart_ConfigType *uart_config;

    /* Check if already initialized */
    if (debug_print_initialized == TRUE)
    {
        return;
    }

    /* Get UART configuration from predefined configurations */
    //uart_config = Uart_pxPBcfgVariantPredefined[0];

    /* Initialize UART driver */
    //Uart_Init(uart_config);
    Uart_Init(NULL_PTR);

    /* Set initialized flag */
    debug_print_initialized = TRUE;
}

void debug_print(const char *format, ...)
{
    va_list args;
    int32_t formatted_length;
    Std_ReturnType uart_result;

    /* Check if initialized */
    if (debug_print_initialized == FALSE)
    {
        return;
    }

    /* Format the string */
    va_start(args, format);
    formatted_length = vsnprintf(debug_print_buffer, DEBUG_PRINT_BUFFER_SIZE, format, args);
    va_end(args);

    /* Check if formatting was successful */
    if (formatted_length < 0 || (uint32)formatted_length >= DEBUG_PRINT_BUFFER_SIZE)
    {
        /* Formatting error or buffer overflow, print error message */
        debug_print_string("DEBUG_PRINT: Format error\r\n");
        return;
    }

    /* Send the formatted string asynchronously */
    uart_result = Uart_AsyncSend(DEBUG_PRINT_UART_CHANNEL,
                                (const uint8 *)debug_print_buffer,
                                (uint32)formatted_length);

    if (uart_result == E_OK)
    {
        /* Wait for transmission to complete */
        if (debug_print_wait_transmit_complete(100000U) == FALSE)
        {
            /* Transmission timeout */
            debug_print_string("DEBUG_PRINT: TX timeout\r\n");
        }
    }
    else
    {
        /* UART send failed */
        debug_print_string("DEBUG_PRINT: UART send failed\r\n");
    }
}

void debug_print_string(const char *str)
{
    Std_ReturnType uart_result;
    uint32 str_length;

    /* Check if initialized */
    if (debug_print_initialized == FALSE)
    {
        return;
    }

    /* Check input parameter */
    if (str == NULL_PTR)
    {
        return;
    }

    /* Get string length */
    str_length = (uint32)strlen(str);

    /* Send string asynchronously */
    uart_result = Uart_AsyncSend(DEBUG_PRINT_UART_CHANNEL,
                                (const uint8 *)str,
                                str_length);

    if (uart_result == E_OK)
    {
        /* Wait for transmission to complete */
        if (debug_print_wait_transmit_complete(100000U) == FALSE)
        {
            /* Transmission timeout */
            debug_print_string("DEBUG_PRINT: TX timeout\r\n");
        }
    }
}

void debug_print_hex(const uint8_t *buffer, uint32_t length, const char *prefix)
{
    uint32_t i;
    char hex_buffer[64]; /* Buffer for one line of hex output */
    uint32_t buffer_index = 0;

    /* Check if initialized */
    if (debug_print_initialized == FALSE)
    {
        return;
    }

    /* Print prefix if provided */
    if (prefix != NULL_PTR)
    {
        debug_print_string(prefix);
    }

    /* Print hex data in lines of 16 bytes */
    for (i = 0; i < length; i++)
    {
        /* Format hex value */
        int32_t chars_written = snprintf(&hex_buffer[buffer_index],
                                        sizeof(hex_buffer) - buffer_index,
                                        "%02X ", buffer[i]);
        if (chars_written < 0)
        {
            break; /* Error in formatting */
        }
        buffer_index += (uint32_t)chars_written;

        /* Print line when buffer is full or every 16 bytes */
        if ((buffer_index >= sizeof(hex_buffer) - 6) || /* Leave space for newline */
            ((i % 16) == 15) || (i == length - 1))
        {
            /* Add newline and print */
            uint32_t copy_index = buffer_index;
            hex_buffer[copy_index++] = '\r';
            hex_buffer[copy_index++] = '\n';
            hex_buffer[copy_index] = '\0';

            debug_print_string(hex_buffer);
            buffer_index = 0;
        }
    }

    /* Add final newline if not already added */
    if (length > 0 && (length % 16) != 0)
    {
        debug_print_string("\r\n");
    }
}

void debug_print_char(char character)
{
    Std_ReturnType uart_result;

    /* Check if initialized */
    if (debug_print_initialized == FALSE)
    {
        return;
    }

    /* Send single character asynchronously */
    uart_result = Uart_AsyncSend(DEBUG_PRINT_UART_CHANNEL,
                                (const uint8 *)&character,
                                1U);

    if (uart_result == E_OK)
    {
        /* Wait for transmission to complete */
        if (debug_print_wait_transmit_complete(10000U) == FALSE)
        {
            /* Transmission timeout - silently fail for single char */
        }
    }
}

void debug_print_newline(void)
{
    debug_print_string("\r\n");
}

/*==================================================================================================
                                       LOCAL FUNCTIONS
==================================================================================================*/

static boolean debug_print_wait_transmit_complete(uint32 timeout_us)
{
    Uart_StatusType status;
    uint32 bytes_remaining;
    uint32 wait_count = 0;
    uint32 max_wait_cycles = timeout_us / 10; /* 10us per cycle */

    /* Wait for transmission to complete */
    do
    {
        status = Uart_GetStatus(DEBUG_PRINT_UART_CHANNEL, &bytes_remaining, UART_SEND);

        if (status == UART_STATUS_NO_ERROR)
        {
            /* Transmission completed successfully */
            return TRUE;
        }
        else if (status == UART_STATUS_OPERATION_ONGOING)
        {
            /* Still transmitting, continue waiting */
            wait_count++;
            /* Small delay */
            for (volatile uint32 delay = 0; delay < 100; delay++)
            {
                /* Busy wait */
            }
        }
        else
        {
            /* Error occurred */
            return FALSE;
        }
    } while (wait_count < max_wait_cycles);

    /* Timeout */
    return FALSE;
}

int _write(int file, char *ptr, int len)
{
    int i;
    (void)file; /* Suppress unused parameter warning */

    /* Check if debug print is initialized */
    if (debug_print_initialized == FALSE)
    {
        return -1; /* Error: not initialized */
    }

    /* Check input parameters */
    if (ptr == NULL_PTR || len < 0)
    {
        return -1; /* Error: invalid parameters */
    }

    /* Send each character individually through debug UART */
    for (i = 0; i < len; i++)
    {
        debug_print_char(ptr[i]);
    }

    /* Return number of characters written */
    return len;
}
