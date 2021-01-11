/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_sdmmc_host.h"
#include "fsl_sdmmc_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SDMMCHOST_TRANSFER_COMPLETE_TIMEOUT (~0U)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*!
 * @brief SDMMCHOST detect card insert status by host controller.
 * @param base host base address.
 * @param userData user can register a application card insert callback through userData.
 */
static void SDMMCHOST_DetectCardInsertByHost(SDIF_Type *base, void *userData);

/*!
 * @brief SDMMCHOST detect card remove status by host controller.
 * @param base host base address.
 * @param userData user can register a application card insert callback through userData.
 */
static void SDMMCHOST_DetectCardRemoveByHost(SDIF_Type *base, void *userData);

/*!
 * @brief SDMMCHOST transfer complete callback.
 * @param base host base address.
 * @param handle host handle.
 * @param status interrupt status.
 * @param userData user data.
 */
static void SDMMCHOST_TransferCompleteCallback(SDIF_Type *base, void *handle, status_t status, void *userData);

/*!
 * @brief SDMMCHOST error recovery.
 * @param base host base address.
 */
static void SDMMCHOST_ErrorRecovery(SDIF_Type *base);
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
static void SDMMCHOST_DetectCardInsertByHost(SDIF_Type *base, void *userData)
{
    sd_detect_card_t *cd = NULL;

    SDMMC_OSAEventSet(((sdmmchost_t *)userData)->hostEvent, SDMMC_OSA_EVENT_CARD_INSERTED);

    if (userData != NULL)
    {
        cd = (sd_detect_card_t *)(((sdmmchost_t *)userData)->cd);
        if (cd != NULL)
        {
            if (cd->callback != NULL)
            {
                cd->callback(true, cd->userData);
            }
        }
    }
}

static void SDMMCHOST_DetectCardRemoveByHost(SDIF_Type *base, void *userData)
{
    sd_detect_card_t *cd = NULL;

    SDMMC_OSAEventSet(((sdmmchost_t *)userData)->hostEvent, SDMMC_OSA_EVENT_CARD_REMOVED);

    if (userData != NULL)
    {
        cd = (sd_detect_card_t *)(((sdmmchost_t *)userData)->cd);
        if (cd != NULL)
        {
            if (cd->callback != NULL)
            {
                cd->callback(false, cd->userData);
            }
        }
    }
}

static void SDMMCHOST_CardInterrupt(SDIF_Type *base, void *userData)
{
    sdio_card_int_t *cardInt = NULL;

    /* application callback */
    if (userData != NULL)
    {
        cardInt = ((sdmmchost_t *)userData)->cardInt;
        if ((cardInt != NULL) && (cardInt->cardInterrupt != NULL))
        {
            cardInt->cardInterrupt(cardInt->userData);
        }
    }
}

status_t SDMMCHOST_CardIntInit(sdmmchost_t *host, void *sdioInt)
{
    host->cardInt                       = sdioInt;
    host->handle.callback.SDIOInterrupt = SDMMCHOST_CardInterrupt;
    SDMMCHOST_EnableCardInt(host, true);

    return kStatus_Success;
}

status_t SDMMCHOST_CardDetectInit(sdmmchost_t *host, void *cd)
{
    SDIF_Type *base        = host->hostController.base;
    sd_detect_card_t *sdCD = (sd_detect_card_t *)cd;
    if (cd == NULL)
    {
        return kStatus_Fail;
    }

    host->cd = cd;

    /* enable card detect interrupt */
    SDIF_EnableInterrupt(base, kSDIF_CardDetect);

    if (SDMMCHOST_CardDetectStatus(host) == kSD_Inserted)
    {
        SDMMC_OSAEventSet(host->hostEvent, SDMMC_OSA_EVENT_CARD_INSERTED);
        /* notify application about the card insertion status */
        if (sdCD->callback)
        {
            sdCD->callback(true, sdCD->userData);
        }
    }
    else
    {
        SDMMC_OSAEventSet(host->hostEvent, SDMMC_OSA_EVENT_CARD_REMOVED);
    }

    return kStatus_Success;
}

uint32_t SDMMCHOST_CardDetectStatus(sdmmchost_t *host)
{
    SDIF_Type *base        = host->hostController.base;
    sd_detect_card_t *sdCD = (sd_detect_card_t *)host->cd;

#if defined(FSL_FEATURE_SDIF_ONE_INSTANCE_SUPPORT_TWO_CARD) && FSL_FEATURE_SDIF_ONE_INSTANCE_SUPPORT_TWO_CARD
    if (((host->hostPort == 0U) &&
         SDIF_DetectCardInsert(base, sdCD->type == kSD_DetectCardByHostDATA3 ? true : false)) ||
        ((host->hostPort == 1U) &&
         SDIF_DetectCard1Insert(base, sdCD->type == kSD_DetectCardByHostDATA3 ? true : false)))
#else
    if ((host->hostPort == 0U) && SDIF_DetectCardInsert(base, sdCD->type == kSD_DetectCardByHostDATA3 ? true : false))
#endif
    {
        return kSD_Inserted;
    }

    return kSD_Removed;
}

status_t SDMMCHOST_PollingCardDetectStatus(sdmmchost_t *host, uint32_t waitCardStatus, uint32_t timeout)
{
    assert(host != NULL);
    assert(host->cd != NULL);

    sd_detect_card_t *cd = host->cd;
    uint32_t event       = 0U;

    SDMMC_OSAEventGet(host->hostEvent, SDMMC_OSA_EVENT_CARD_INSERTED | SDMMC_OSA_EVENT_CARD_REMOVED, &event);
    if ((((event & SDMMC_OSA_EVENT_CARD_INSERTED) == SDMMC_OSA_EVENT_CARD_INSERTED) &&
         (waitCardStatus == kSD_Inserted)) ||
        (((event & SDMMC_OSA_EVENT_CARD_REMOVED) == SDMMC_OSA_EVENT_CARD_REMOVED) && (waitCardStatus == kSD_Removed)))
    {
        return kStatus_Success;
    }

    /* Wait card inserted. */
    do
    {
        if (SDMMC_OSAEventWait(host->hostEvent, SDMMC_OSA_EVENT_CARD_INSERTED | SDMMC_OSA_EVENT_CARD_REMOVED, timeout,
                               &event) != kStatus_Success)
        {
            return kStatus_Fail;
        }
        else
        {
            if ((waitCardStatus == kSD_Inserted) &&
                ((event & SDMMC_OSA_EVENT_CARD_INSERTED) == SDMMC_OSA_EVENT_CARD_INSERTED))
            {
                SDMMC_OSADelay(cd->cdDebounce_ms);
                if (SDMMCHOST_CardDetectStatus(host) == kSD_Inserted)
                {
                    break;
                }
            }

            if (((event & SDMMC_OSA_EVENT_CARD_REMOVED) == SDMMC_OSA_EVENT_CARD_REMOVED) &&
                (waitCardStatus == kSD_Removed))
            {
                break;
            }
        }
    } while (1U);

    return kStatus_Success;
}

status_t SDMMCHOST_WaitCardDetectStatus(SDMMCHOST_TYPE *hostBase,
                                        const sdmmchost_detect_card_t *cd,
                                        bool waitCardStatus)
{
    assert(cd != NULL);

    while (SDIF_DetectCardInsert(hostBase, false) != waitCardStatus)
    {
    }

    return kStatus_Success;
}

static void SDMMCHOST_TransferCompleteCallback(SDIF_Type *base, void *handle, status_t status, void *userData)
{
    uint32_t eventStatus = 0U;

    if (status == kStatus_SDIF_DataTransferFail)
    {
        eventStatus = SDMMC_OSA_EVENT_TRANSFER_DATA_FAIL;
    }
    else if (status == kStatus_SDIF_DataTransferSuccess)
    {
        eventStatus = SDMMC_OSA_EVENT_TRANSFER_DATA_SUCCESS;
    }
    else if (status == kStatus_SDIF_SendCmdFail)
    {
        eventStatus = SDMMC_OSA_EVENT_TRANSFER_CMD_FAIL;
    }
    else
    {
        eventStatus = SDMMC_OSA_EVENT_TRANSFER_CMD_SUCCESS;
    }

    SDMMC_OSAEventSet(((sdmmchost_t *)userData)->hostEvent, eventStatus);
}

status_t SDMMCHOST_TransferFunction(sdmmchost_t *host, sdmmchost_transfer_t *content)
{
    status_t error = kStatus_Success;
    uint32_t event = 0U;
    sdif_dma_config_t dmaConfig;

    /* clear redundant transfer event flag */
    SDMMC_OSAEventClear(host->hostEvent, SDMMC_OSA_EVENT_TRANSFER_CMD_SUCCESS | SDMMC_OSA_EVENT_TRANSFER_CMD_FAIL |
                                             SDMMC_OSA_EVENT_TRANSFER_DATA_SUCCESS |
                                             SDMMC_OSA_EVENT_TRANSFER_DATA_FAIL);

    /* user DMA mode transfer data */
    if (content->data != NULL)
    {
        memset(&dmaConfig, 0, sizeof(dmaConfig));

        dmaConfig.enableFixBurstLen     = false;
        dmaConfig.mode                  = kSDIF_DualDMAMode;
        dmaConfig.dmaDesBufferStartAddr = host->dmaDesBuffer;
        dmaConfig.dmaDesBufferLen       = host->dmaDesBufferWordsNum;
        dmaConfig.dmaDesSkipLen         = 0U;
    }

    do
    {
        error = SDIF_TransferNonBlocking(host->hostController.base, &host->handle, &dmaConfig, content);
    } while (error == kStatus_SDIF_SyncCmdTimeout);

    /* wait command event */
    if ((kStatus_Fail ==
         SDMMC_OSAEventWait(host->hostEvent,
                            SDMMC_OSA_EVENT_TRANSFER_CMD_SUCCESS | SDMMC_OSA_EVENT_TRANSFER_CMD_FAIL |
                                SDMMC_OSA_EVENT_TRANSFER_DATA_SUCCESS | SDMMC_OSA_EVENT_TRANSFER_DATA_FAIL,
                            SDMMCHOST_TRANSFER_COMPLETE_TIMEOUT, &event)) ||
        ((event & SDMMC_OSA_EVENT_TRANSFER_CMD_FAIL) != 0U))
    {
        error = kStatus_Fail;
    }
    else
    {
        if (content->data != NULL)
        {
            if ((event & SDMMC_OSA_EVENT_TRANSFER_DATA_SUCCESS) == 0U)
            {
                if (((event & SDMMC_OSA_EVENT_TRANSFER_DATA_FAIL) != 0U) ||
                    (kStatus_Fail ==
                         SDMMC_OSAEventWait(host->hostEvent,
                                            SDMMC_OSA_EVENT_TRANSFER_DATA_SUCCESS | SDMMC_OSA_EVENT_TRANSFER_DATA_FAIL,
                                            SDMMCHOST_TRANSFER_COMPLETE_TIMEOUT, &event) ||
                     ((event & SDMMC_OSA_EVENT_TRANSFER_DATA_FAIL) != 0U)))
                {
                    error = kStatus_Fail;
                }
            }
        }
    }

    if (error != kStatus_Success)
    {
        error = kStatus_Fail;
        /* host error recovery */
        SDMMCHOST_ErrorRecovery(host->hostController.base);
    }

    return error;
}

static void SDMMCHOST_ErrorRecovery(SDIF_Type *base)
{
    /* reserved for future */
}

void SDMMCHOST_SetCardPower(sdmmchost_t *host, bool enable)
{
    if (host->hostPort == 0U)
    {
        SDIF_EnableCardPower(host->hostController.base, enable);
    }
#if defined(FSL_FEATURE_SDIF_ONE_INSTANCE_SUPPORT_TWO_CARD) && FSL_FEATURE_SDIF_ONE_INSTANCE_SUPPORT_TWO_CARD
    else
    {
        SDIF_EnableCard1Power(host->hostController.base, enable);
    }
#endif

    if (enable)
    {
        /* perform SDIF host controller reset only when DATA BUSY is assert */
        if ((SDIF_GetControllerStatus(host->hostController.base) & SDIF_STATUS_DATA_BUSY_MASK) != 0U)
        {
            SDIF_Reset(host->hostController.base, kSDIF_ResetAll, 100U);
        }
    }
}

void SDMMCHOST_PowerOffCard(SDMMCHOST_TYPE *base, const sdmmchost_pwr_card_t *pwr)
{
    if (pwr != NULL)
    {
        pwr->powerOff();
        SDMMC_OSADelay(pwr->powerOffDelay_ms);
    }
    else
    {
        /* disable the card power */
        SDIF_EnableCardPower(base, false);
        /* Delay several milliseconds to make card stable. */
        SDMMC_OSADelay(500U);
    }
}

void SDMMCHOST_PowerOnCard(SDMMCHOST_TYPE *base, const sdmmchost_pwr_card_t *pwr)
{
    /* use user define the power on function  */
    if (pwr != NULL)
    {
        pwr->powerOn();
        SDMMC_OSADelay(pwr->powerOnDelay_ms);
    }
    else
    {
        /* Enable the card power */
        SDIF_EnableCardPower(base, true);
        /* Delay several milliseconds to make card stable. */
        SDMMC_OSADelay(500U);
    }

    /* perform SDIF host controller reset only when DATA BUSY is assert */
    if ((SDIF_GetControllerStatus(base) & SDIF_STATUS_DATA_BUSY_MASK) != 0U)
    {
        SDIF_Reset(base, kSDIF_ResetAll, 100U);
    }
}

status_t SDMMCHOST_Init(sdmmchost_t *host)
{
    assert(host != NULL);
    assert(host->hostEvent != NULL);

    sdif_transfer_callback_t sdifCallback = {0};
    sdif_host_t *sdifHost                 = &(host->hostController);

    /* sdmmc osa init */
    SDMMC_OSAInit();

    /* Initialize SDIF. */
    sdifHost->config.endianMode            = kSDMMCHOST_EndianModeLittle;
    sdifHost->config.responseTimeout       = 0xFFU;
    sdifHost->config.cardDetDebounce_Clock = 0xFFFFFFU;
    sdifHost->config.dataTimeout           = 0xFFFFFFU;
    SDIF_Init(sdifHost->base, &(sdifHost->config));

    /* Create handle for SDIF driver */
    sdifCallback.TransferComplete = SDMMCHOST_TransferCompleteCallback;
    sdifCallback.cardInserted     = SDMMCHOST_DetectCardInsertByHost;
    sdifCallback.cardRemoved      = SDMMCHOST_DetectCardRemoveByHost;
    SDIF_TransferCreateHandle(sdifHost->base, &host->handle, &sdifCallback, host);

    /* Create transfer event. */
    if (kStatus_Success != SDMMC_OSAEventCreate(host->hostEvent))
    {
        return kStatus_Fail;
    }

    return kStatus_Success;
}

void SDMMCHOST_Reset(sdmmchost_t *host)
{
    /* make sure host controller release all the bus line. */
    SDIF_Reset(host->hostController.base, kSDIF_ResetAll, 100U);
}

void SDMMCHOST_SetCardBusWidth(sdmmchost_t *host, uint32_t dataBusWidth)
{
    if (host->hostPort == 0U)
    {
        SDIF_SetCardBusWidth(host->hostController.base,
                             dataBusWidth == kSDMMC_BusWdith1Bit ?
                                 kSDIF_Bus1BitWidth :
                                 dataBusWidth == kSDMMC_BusWdith4Bit ? kSDIF_Bus4BitWidth : kSDIF_Bus8BitWidth);
    }
#if defined(FSL_FEATURE_SDIF_ONE_INSTANCE_SUPPORT_TWO_CARD) && FSL_FEATURE_SDIF_ONE_INSTANCE_SUPPORT_TWO_CARD
    else
    {
        SDIF_SetCard1BusWidth(host->hostController.base,
                              dataBusWidth == kSDMMC_BusWdith1Bit ?
                                  kSDIF_Bus1BitWidth :
                                  dataBusWidth == kSDMMC_BusWdith4Bit ? kSDIF_Bus4BitWidth : kSDIF_Bus8BitWidth);
    }
#endif
}

void SDMMCHOST_Deinit(sdmmchost_t *host)
{
    sdif_host_t *sdifHost = &host->hostController;
    SDIF_Deinit(sdifHost->base);
    SDMMC_OSAEventDestroy(host->hostEvent);
}

status_t SDMMCHOST_StartBoot(sdmmchost_t *host,
                             sdmmchost_boot_config_t *hostConfig,
                             sdmmchost_cmd_t *cmd,
                             uint8_t *buffer)
{
    /* not support */
    return kStatus_Success;
}

status_t SDMMCHOST_ReadBootData(sdmmchost_t *host, sdmmchost_boot_config_t *hostConfig, uint8_t *buffer)
{
    /* not support */
    return kStatus_Success;
}
