/*==================================================================================================
*   Project              : RTD AUTOSAR 4.7
*   Platform             : CORTEXM
*   Peripheral           : 
*   Dependencies         : 
*
*   Autosar Version      : 4.7.0
*   Autosar Revision     : ASR_REL_4_7_REV_0000
*   Autosar Conf.Variant :
*   SW Version           : 6.0.0
*   Build Version        : S32K3_RTD_6_0_0_D2506_ASR_REL_4_7_REV_0000_20250610
*
*   (c) Copyright 2020 - 2025 NXP
*   All Rights Reserved.
*
*   NXP Confidential and Proprietary. This software is owned or controlled by NXP
*   and may only be used strictly in accordance with the applicable license terms.
*   By expressly accepting such terms or by downloading, installing, activating
*   and/or otherwise using the software, you are agreeing that you have read,
*   and that you agree to comply with and are bound by, such license terms.
*   If you do not agree to be bound by the applicable license terms, then
*   you may not retain, install, activate or otherwise use the software.
==================================================================================================*/

/**
*   @file
*
*   @addtogroup ETHIF Ethernet Interface
*   @{
*/

#ifdef __cplusplus
extern "C"{
#endif

/*==================================================================================================
*                                        INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
==================================================================================================*/
#include "ComStackTypes.h"
#include "EthIf.h"

/*==================================================================================================
*                              SOURCE FILE VERSION INFORMATION
==================================================================================================*/

#define ETHIF_VENDOR_ID_C                      43
#define ETHIF_AR_RELEASE_MAJOR_VERSION_C       4
#define ETHIF_AR_RELEASE_MINOR_VERSION_C       7
#define ETHIF_AR_RELEASE_REVISION_VERSION_C    0
#define ETHIF_SW_MAJOR_VERSION_C               6
#define ETHIF_SW_MINOR_VERSION_C               0
#define ETHIF_SW_PATCH_VERSION_C               0

/*==================================================================================================
*                                     FILE VERSION CHECKS
==================================================================================================*/
/* Check if current file and ETHIF header file are of the same vendor */
#if (ETHIF_VENDOR_ID_C != ETHIF_VENDOR_ID)
    #error "EthIf.c and EthIf.h have different vendor ids"
#endif
/* Check if current file and ETHIF header file are of the same Autosar version */
#if ((ETHIF_AR_RELEASE_MAJOR_VERSION_C    != ETHIF_AR_RELEASE_MAJOR_VERSION) || \
     (ETHIF_AR_RELEASE_MINOR_VERSION_C    != ETHIF_AR_RELEASE_MINOR_VERSION) || \
     (ETHIF_AR_RELEASE_REVISION_VERSION_C != ETHIF_AR_RELEASE_REVISION_VERSION))
    #error "AutoSar Version Numbers of EthIf.c and EthIf.h are different"
#endif
/* Check if current file and ETHIF header file are of the same Software version */
#if ((ETHIF_SW_MAJOR_VERSION_C != ETHIF_SW_MAJOR_VERSION) || \
     (ETHIF_SW_MINOR_VERSION_C != ETHIF_SW_MINOR_VERSION) || \
     (ETHIF_SW_PATCH_VERSION_C != ETHIF_SW_PATCH_VERSION))
    #error "Software Version Numbers of EthIf.c and EthIf.h are different"
#endif
/*==================================================================================================
*                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/


/*==================================================================================================
*                                       LOCAL MACROS
==================================================================================================*/


/*==================================================================================================
*                                      LOCAL CONSTANTS
==================================================================================================*/


/*==================================================================================================
*                                      LOCAL VARIABLES
==================================================================================================*/


/*==================================================================================================
*                                      GLOBAL CONSTANTS
==================================================================================================*/


/*==================================================================================================
*                                      GLOBAL VARIABLES
==================================================================================================*/
volatile uint32 EthIf_RxIndications[10]    = {0};
volatile uint32 EthIf_TxConfirmations[10]  = {0};
volatile boolean EthIf_ModeIndications[10] = {0};
volatile uint16 EthIf_ChecksumValue[10]    = {0};
Std_ReturnType (*EthIf_GetIngressTimestampCallback)(uint8, const Eth_DataType*, Eth_TimeStampQualType*, Eth_TimeStampType*) = &EthIf_GetIngressTimestamp;

/*==================================================================================================
*                                   LOCAL FUNCTION PROTOTYPES
==================================================================================================*/


/*==================================================================================================
*                                       LOCAL FUNCTIONS
==================================================================================================*/


/*==================================================================================================
*                                       GLOBAL FUNCTIONS
==================================================================================================*/

/*================================================================================================*/
/**
* @brief          This function handles the received Ethernet frame.
* @details        Function should parse the received frame and pass the gathered
*                 information to the appropriate upper layer module.
* @note           The passed data buffer is no longer valid after the function
*                 is exited.
* @warning        This is only an empty stub function provided only to be able
*                 to compile and link the Eth module.
* @param[in]      CtrlIdx Index of the controller which received the frame.
* @param[in]      FrameType The received frame Ethertype (from the frame header)
* @param[in]      IsBroadcast The value TRUE indicates that the received frame
*                 was sent to broadcast address (ff-ff-ff-ff-ff-ff)
* @param[in]      PhysAddrPtr Pointer to received frame source MAC address
*                 (6 bytes).
* @param[in]      DataPtr Data buffer containing the received Ethernet frame 
*                 payload.
* @param[in]      LenByte Length of the data in the buffer DataPtr.
*
*/
void EthIf_RxIndication(\
                        uint8 CtrlIdx,\
                        Eth_FrameType FrameType, \
                        boolean IsBroadcast, \
                        const uint8* PhysAddrPtr, \
                        const Eth_DataType* DataPtr,\
                        uint16 LenByte)
{
    /* This is an empty stub function */
    Std_ReturnType Status;
    Eth_TimeStampQualType TimeQual = ETH_UNCERTAIN;
    Eth_TimeStampType TimeStamp = {0U};
    ++EthIf_RxIndications[CtrlIdx];
    EthIf_ChecksumValue[CtrlIdx] = *((uint16 *)(&DataPtr[10U]));
    (void)FrameType;
    (void)IsBroadcast;
    (void)PhysAddrPtr;
    (void)DataPtr;
    (void)LenByte;
    Status = (*EthIf_GetIngressTimestampCallback)(CtrlIdx, DataPtr, &TimeQual, &TimeStamp);
    (void)Status;
}
/*================================================================================================*/
/**
* @brief          This function confirms that transmission of an Ethernet frame
*                 was finished.
* @details        Function should notify the appropriate upper layer module that
*                 the data transmission was successfully finished.
* @warning        This is only an empty stub function provided only to be able
*                 to compile and link the Eth module.
* @param[in]      CtrlIdx Index of the controller which transmitted the frame.
* @param[in]      BufIdx Index of the transmitted data buffer.

*/
void EthIf_TxConfirmation(uint8 CtrlIdx, \
                          Eth_BufIdxType BufIdx, \
                          Std_ReturnType Result)
{
    /* This is an empty stub function */ 
    ++EthIf_TxConfirmations[CtrlIdx];
    (void)BufIdx;
    (void)Result;
}

/*================================================================================================*/
/**
* @brief          This function indicate that driver mode has been changed
* @details        Called asynchronously when mode has been read out. Triggered by previous 
*                 Eth_SetControllerMode call. Can directly be called within the trigger functions.
* @warning        This is only an empty stub function provided only to be able
*                 to compile and link the Eth module.
* @param[in]      CtrlIdx Index of the controller which mode has been changed.
* @param[in]      CtrlMode New mode of correspond Eth driver.

*/
void EthIf_CtrlModeIndication( \
                                uint8 CtrlIdx, \
                                Eth_ModeType CtrlMode \
                             )
{
    /* This is an empty stub function */ 
    EthIf_ModeIndications[CtrlIdx] = TRUE;
    (void)CtrlMode;
}


/*================================================================================================*/
/**
* @brief          This function indicates that an ingress timestamp was captured
* @details        Returns an ingress timestamp value out of the Switch.
*                 If the HW resolution is lower than the Eth_TimeStampType resolution resp. range,
*                 than the remaining bits will be filled with 0.
* @warning        This is only an empty stub function provided only to be able
*                 to compile and link the EthSwt module.
* @param[in]      CtrlIdx       Index of the controller which mode has been changed.
* @param[in]      MgmtInfoPtr   Management information
* @param[in]      timeStampPtr  Current timestamp
* @param[out]     DataPtr       Ethernet data pointer
*/
void EthIf_SwitchIngressTimeStampIndication(uint8 CtrlIdx,
                                            Eth_DataType* DataPtr,
                                            EthSwt_MgmtInfoType* MgmtInfoPtr,
                                            Eth_TimeStampType* timeStampPtr
                                           )
{
    /* This is an empty stub function */
    (void)CtrlIdx;
    (void)DataPtr;
    (void)MgmtInfoPtr;
    (void)timeStampPtr;
}

/**
* @brief          This function indicates that an egress timestamp was captured
* @details        Returns an egress timestamp value out of the Switch.
*                 If the HW resolution is lower than the Eth_TimeStampType resolution resp. range,
*                 than the remaining bits will be filled with 0.
* @warning        This is only an empty stub function provided only to be able
*                 to compile and link the EthSwt module.
* @param[in]      CtrlIdx       Index of the controller which mode has been changed.
* @param[intout]  DataPtr       Ethernet data pointer
* @param[out]     MgmtInfoPtr   Management information
* @param[out]     timeStampPtr  Current timestamp
*/
void EthIf_SwitchEgressTimeStampIndication(uint8 CtrlIdx,
                                           Eth_DataType* DataPtr,
                                           EthSwt_MgmtInfoType* MgmtInfoPtr,
                                           Eth_TimeStampType* timeStampPtr
                                          )
{
    /* This is an empty stub function */
    (void)CtrlIdx;
    (void)DataPtr;
    (void)MgmtInfoPtr;
    (void)timeStampPtr;
}

/**
* @brief          This function indicates that mgmt information was received
* @details        Ingress Switch management info indication redirected call to upper layers who registered for the call.
* @warning        This is only an empty stub function provided only to be able
*                 to compile and link the EthSwt module.
* @param[in]      CtrlIdx       Index of the controller which mode has been changed.
* @param[intout]  DataPtr       Ethernet data pointer
* @param[out]     MgmtInfoPtr   Management information
*/
void EthIf_SwitchMgmtInfoIndication(uint8 CtrlIdx,
                                    Eth_DataType* DataPtr,
                                    EthSwt_MgmtInfoType* MgmtInfoPtr
                                   )
{
    /* This is an empty stub function */ 
    (void)CtrlIdx;
    (void)DataPtr;
    (void)MgmtInfoPtr;
}

/**
* @brief          This function indicates that a transceiver's mode was changed
* @details        Called asynchronously when a mode change has been read out. If the function is triggered by previous call of
*                 EthTrcv_SetTransceiverMode it can directly be called within the trigger function.
* @warning        This is only an empty stub function provided only to be able
*                 to compile and link the EthSwt module.
* @param[in]      TrcvIdx       Index of the Ethernet transceiver within the context of the Ethernet Interface
* @param[in]      TrcvMode      Notified Ethernet transceiver mode
*/
void EthIf_TrcvModeIndication(uint8 TrcvIdx,
                              EthTrcv_ModeType TrcvMode
                             )
{
    /* This is an empty stub function */ 
    (void)TrcvIdx;
    (void)TrcvMode;
}

/**
* @brief          This function indicates that a switch port mode was changed
* @details        The EthIf shall determine the expected notifications based on the EthSwtPort configuration.
*                 In case the EthSwtPort references an EthTrcv the EthIf expects a notification from the EthTrcv via API EthIf_TrcvModeIndication().
*                 Otherwise the EthIf expects a notification from the EthSwt via API EthIf_SwitchPortModeIndication().
* @warning        This is only an empty stub function provided only to be able
*                 to compile and link the EthSwt module.
* @param[in]      SwitchIdx     Index of the switch within the context of the Ethernet Switch Driver
* @param[in]      SwitchPortIdx Index of the port at the addressed switch.
* @param[in]      PortMode      Notified Ethernet Switch port mode.
*/
void EthIf_SwitchPortModeIndication(uint8 SwitchIdx,
                                    uint8 SwitchPortIdx,
                                    Eth_ModeType PortMode
                                   )
{
    /* This is an empty stub function */
    (void)SwitchIdx;
    (void)SwitchPortIdx;
    (void)PortMode;
}

/**
* @brief          This function extracts Ingress Timestamps from received frames.
* @details        Function should be called after the received frame is parsed 
*                 and pass the gathered information to the appropriate upper layer module.
* @warning        This is only an empty stub function provided only to be able
*                 to compile and link the Eth module.
* @param[in]      CtrlIdx Index of the controller which received the frame.
* @param[in]      DataPtr Data buffer containing the received Ethernet frame 
*                 payload.
* @param[in]      TimeQualPtr Pointer to timestamp quality indicator
* @param[in]      TimeStampPtr Pointer to timestamp structure to be populated
*
*/
Std_ReturnType EthIf_GetIngressTimestamp( \
                                         uint8 CtrlIdx, \
                                         const Eth_DataType *DataPtr, \
                                         Eth_TimeStampQualType* TimeQualPtr, \
                                         Eth_TimeStampType* TimeStampPtr \
                                        )
{
    (void)CtrlIdx;
    (void)DataPtr;
    (void)TimeQualPtr;
    (void)TimeStampPtr;
    return E_OK;
}

/**
 * @brief   EhtIf_SwitchMacSecUpdateSecY callback.
 *
 * @details Callback to notify that EhtIf_SwitchMacSecUpdateSecY finished.
 * 
 * @warning        This is only an empty stub function provided only to be able
 *                 to compile and link the Eth module.
 * 
 * @param[in] CtrlIdx Index of the Ethernet controller within the context of the Ethernet Interface.
 */
void EthIf_MacSecUpdateSecYNotification(uint8 CtrlIdx)
{
    (void)CtrlIdx;
}

/**
 * @brief   EthIf_MacSecAddTxSa callback.
 *
 * @details Callback to notify that EthIf_MacSecAddTxSa finished.
 *
 * @warning        This is only an empty stub function provided only to be able
 *                 to compile and link the Eth module.
 * 
 * @param[in] CtrlIdx Index of the Ethernet controller within the context of the Ethernet Interface.
 */
void EthIf_MacSecAddTxSaNotification(uint8 CtrlIdx)
{
    (void)CtrlIdx;
}

/**
 * @brief   EthIf_MacSecAddRxSa callback.
 *
 * @details Callback to notify that EthIf_MacSecAddRxSa finished.
 *
 * @warning        This is only an empty stub function provided only to be able
 *                 to compile and link the Eth module.
 * 
 * @param[in] CtrlIdx Index of the Ethernet controller within the context of the Ethernet Interface.
 */
void EthIf_MacSecAddRxSaNotification(uint8 CtrlIdx)
{
    (void)CtrlIdx;
}

/**
 * @brief   EthIf_MacSecGetMacSecStats callback.
 *
 * @details Callback to notify that EthIf_MacSecGetMacSecStats finished and provide the requested statistics.
 *
 * @warning        This is only an empty stub function provided only to be able
 *                 to compile and link the Eth module.
 * 
 * @param[in] CtrlIdx         Index of the Ethernet controller within the context of the Ethernet Interface.
 * @param[in] MacSecStatsPtr  Pointer to a structure including the MACsec statistics of an MKA participant
 */
Std_ReturnType EthIf_MacSecGetMacSecStatsNotification(uint8 CtrlIdx, const Mka_Stats_SecYType* MacSecStatsPtr)
{
    (void)CtrlIdx;
    (void)MacSecStatsPtr;
    return E_OK;
}

#ifdef __cplusplus
}
#endif

/** @} */
