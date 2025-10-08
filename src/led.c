/*
 * Copyright 2024-2025 NXP
 * All rights reserved.
 *
 * THIS SOFTWARE IS PROVIDED BY NXP "AS IS" AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL NXP OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.7, External could be made static.
 * The function is defined for external use.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 8.3, Symbol redeclared.
 * Declarations are guarded by preprocessor directives.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 8.4, A compatible declaration shall be
 * visible when an object or function with external linkage is defined.
 * These are symbols weak symbols defined in platform startup files (.s).
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 10.1, Unpermitted operand to operator '||'
 * Variable is of essential boolean type
 *
 */

#include "led.h"

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* AUTOSAR Dio driver */
#include "Dio.h"

/* Basic types for register access */
#include <stdint.h>

/* LED Blink delay in milliseconds */
#define LED_BLINK_DELAY 500U

/*==================================================================================================
*
*                              SOURCE FILE VERSION INFORMATION
==================================================================================================*/

/*==================================================================================================
*                                       FILE VERSION CHECKS
==================================================================================================*/

/*==================================================================================================
*                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*==================================================================================================
*                                       LOCAL MACROS
==================================================================================================*/

/*==================================================================================================
*                                          LOCAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                       LOCAL VARIABLES
==================================================================================================*/

/*==================================================================================================
*                                       GLOBAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                       GLOBAL VARIABLES
==================================================================================================*/

/*==================================================================================================
*                                   LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

/*==================================================================================================
*                                       LOCAL FUNCTIONS
==================================================================================================*/

/*==================================================================================================
*                                       GLOBAL FUNCTIONS
==================================================================================================*/

/*! @brief Initialize LED_RED GPIO pin */
void LED_RED_Init(void)
{
    /* GPIO pin initialization is done by Port driver in device_init().
     * This function is kept for consistency with HAL design. */
}

/*! @brief LED_RED Blink Task */
void LED_RED_Blink_Task(void* pvParameters)
{
    (void)pvParameters; /* Unused parameter */

    /* Initialize LED_RED pin */
    LED_RED_Init();

    /* Main blink loop */
    for(;;)
    {
        /* Toggle LED_RED */
        LED_RED_Toggle();

        /* Delay 500ms */
        vTaskDelay(pdMS_TO_TICKS(LED_BLINK_DELAY));
    }
}

/*! @brief Turn LED_RED on */
void LED_RED_On(void)
{
    Dio_WriteChannel(DioConf_DioChannel_LED_RED, STD_HIGH);
}

/*! @brief Turn LED_RED off */
void LED_RED_Off(void)
{
    Dio_WriteChannel(DioConf_DioChannel_LED_RED, STD_LOW);
}

/*! @brief Toggle LED_RED state */
void LED_RED_Toggle(void)
{
    static Dio_LevelType currentLevel;


    if (currentLevel == STD_HIGH)
    {
        Dio_WriteChannel(DioConf_DioChannel_LED_RED, STD_LOW);
        currentLevel = STD_LOW;
    }
    else
    {
        Dio_WriteChannel(DioConf_DioChannel_LED_RED, STD_HIGH);
        currentLevel = STD_HIGH;
    }
}
