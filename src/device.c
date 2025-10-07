/*
 * Copyright 2020-2025 NXP
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
 */

#include "S32K344.h"
#include "device.h"
#include "Mcal.h"
#include "Mcu.h"
#include "Port.h"
#include "ethif_port.h"
#include "OsIf.h"
#include "Platform.h"

#ifndef USING_OS_FREERTOS
#include "Gpt.h"
#endif

void device_init(void)
{
    /* Set RMII configuration for EMAC in DCM module */
    IP_DCM_GPR->DCMRWF1 = (IP_DCM_GPR->DCMRWF1 & ~DCM_GPR_DCMRWF1_MAC_CONF_SEL_MASK) | DCM_GPR_DCMRWF1_MAC_CONF_SEL(2U);

    /* Initialize all pins using the Port driver */
    Port_Init(NULL_PTR);

    /* Setup Clocks */
    /* Initialize Mcu module */
    Mcu_Init(NULL_PTR);

    /* Initialize Mcu clock */
    Mcu_InitClock(McuClockSettingConfig_0);

    while (Mcu_GetPllStatus() != MCU_PLL_LOCKED){};

    /* Use PLL clock */
    Mcu_DistributePllClock();

    Mcu_SetMode(McuModeSettingConf_0);

    /* Initialize Os Interface */
    OsIf_Init(NULL_PTR);

    /* Initialize Platform driver */
    Platform_Init(NULL_PTR);

#ifndef USING_OS_FREERTOS

    /* Initialize PIT driver and start the timer */
    Gpt_Init(NULL_PTR);
    /* Start the Gpt timer */
    Gpt_StartTimer(GptConf_GptChannelConfiguration_GptChannelConfiguration_0, 40000000U);
    /* Enable the Gpt notification*/
    Gpt_EnableNotification(GptConf_GptChannelConfiguration_GptChannelConfiguration_0);

    OsIf_SetTimerFrequency(160000000U,  OSIF_USE_SYSTEM_TIMER);

#endif /* USING_OS_FREERTOS */

    /* Initialize and enable the GMAC module */
    Eth_Init(NULL_PTR);
}
