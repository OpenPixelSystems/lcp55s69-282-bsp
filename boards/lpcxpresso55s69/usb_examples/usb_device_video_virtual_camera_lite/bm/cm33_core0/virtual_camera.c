/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017, 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_video.h"

#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"

#include "virtual_camera.h"

#include "video_data.h"

#include "fsl_device_registers.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"

#include <stdio.h>
#include <stdlib.h>
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
#include "fsl_sysmpu.h"
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

#if ((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
#include "usb_phy.h"
#endif

#include "pin_mux.h"
#include <stdbool.h>
#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_InitHardware(void);
void USB_DeviceClockInit(void);
void USB_DeviceIsrEnable(void);
#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle);
#endif

static void USB_DeviceVideoPrepareVideoData(void);
static usb_status_t USB_DeviceVideoIsoIn(usb_device_handle deviceHandle,
                                         usb_device_endpoint_callback_message_struct_t *event,
                                         void *arg);
static void USB_DeviceVideoApplicationSetDefault(void);
static usb_status_t USB_DeviceVideoProcessClassVsProbeRequest(usb_device_handle handle,
                                                              usb_setup_struct_t *setup,
                                                              uint32_t *length,
                                                              uint8_t **buffer);
static usb_status_t USB_DeviceVideoProcessClassVsCommitRequest(usb_device_handle handle,
                                                               usb_setup_struct_t *setup,
                                                               uint32_t *length,
                                                               uint8_t **buffer);
static usb_status_t USB_DeviceVideoProcessClassVsStillProbeRequest(usb_device_handle handle,
                                                                   usb_setup_struct_t *setup,
                                                                   uint32_t *length,
                                                                   uint8_t **buffer);
static usb_status_t USB_DeviceVideoProcessClassVsStillCommitRequest(usb_device_handle handle,
                                                                    usb_setup_struct_t *setup,
                                                                    uint32_t *length,
                                                                    uint8_t **buffer);
static usb_status_t USB_DeviceVideoProcessClassVsStillImageTriggerRequest(usb_device_handle handle,
                                                                          usb_setup_struct_t *setup,
                                                                          uint32_t *length,
                                                                          uint8_t **buffer);
static usb_status_t USB_DeviceVideoProcessClassVsRequest(usb_device_handle handle,
                                                         usb_setup_struct_t *setup,
                                                         uint32_t *length,
                                                         uint8_t **buffer);
static usb_status_t USB_DeviceProcessClassVcInterfaceRequest(usb_device_handle handle,
                                                             usb_setup_struct_t *setup,
                                                             uint32_t *length,
                                                             uint8_t **buffer);
static usb_status_t USB_DeviceProcessClassVcCameraTerminalRequest(usb_device_handle handle,
                                                                  usb_setup_struct_t *setup,
                                                                  uint32_t *length,
                                                                  uint8_t **buffer);
static usb_status_t USB_DeviceProcessClassVcInputTerminalRequest(usb_device_handle handle,
                                                                 usb_setup_struct_t *setup,
                                                                 uint32_t *length,
                                                                 uint8_t **buffer);
static usb_status_t USB_DeviceProcessClassVcOutputTerminalRequest(usb_device_handle handle,
                                                                  usb_setup_struct_t *setup,
                                                                  uint32_t *length,
                                                                  uint8_t **buffer);
static usb_status_t USB_DeviceProcessClassVcProcessingUintRequest(usb_device_handle handle,
                                                                  usb_setup_struct_t *setup,
                                                                  uint32_t *length,
                                                                  uint8_t **buffer);
static usb_status_t USB_DeviceProcessClassVcRequest(usb_device_handle handle,
                                                    usb_setup_struct_t *setup,
                                                    uint32_t *length,
                                                    uint8_t **buffer);
static void USB_DeviceApplicationInit(void);
/*******************************************************************************
 * Variables
 ******************************************************************************/

extern const unsigned char g_UsbDeviceVideoMjpegData[];
extern const uint32_t g_UsbDeviceVideoMjpegLength;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static usb_device_video_probe_and_commit_controls_struct_t s_ProbeStruct;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static usb_device_video_probe_and_commit_controls_struct_t s_CommitStruct;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static usb_device_video_still_probe_and_commit_controls_struct_t s_StillProbeStruct;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static usb_device_video_still_probe_and_commit_controls_struct_t s_StillCommitStruct;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint8_t s_ImageBuffer[HS_STREAM_IN_PACKET_SIZE];
usb_video_virtual_camera_struct_t g_UsbDeviceVideoVirtualCamera;

extern uint8_t g_UsbDeviceCurrentConfigure;
extern uint8_t g_UsbDeviceInterface[USB_VIDEO_VIRTUAL_CAMERA_INTERFACE_COUNT];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static uint32_t s_SetupOutBuffer[(sizeof(usb_device_video_probe_and_commit_controls_struct_t) >> 2U) + 1U];

/*******************************************************************************
 * Code
 ******************************************************************************/
#if (defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U))
void USB0_IRQHandler(void)
{
    USB_DeviceLpcIp3511IsrFunction(g_UsbDeviceVideoVirtualCamera.deviceHandle);
}
#endif
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
void USB1_IRQHandler(void)
{
    USB_DeviceLpcIp3511IsrFunction(g_UsbDeviceVideoVirtualCamera.deviceHandle);
}
#endif
void USB_DeviceClockInit(void)
{
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
    /* enable USB IP clock */
    CLOCK_EnableUsbfs0DeviceClock(kCLOCK_UsbfsSrcFro, CLOCK_GetFroHfFreq());
#if defined(FSL_FEATURE_USB_USB_RAM) && (FSL_FEATURE_USB_USB_RAM)
    for (int i = 0; i < FSL_FEATURE_USB_USB_RAM; i++)
    {
        ((uint8_t *)FSL_FEATURE_USB_USB_RAM_BASE_ADDRESS)[i] = 0x00U;
    }
#endif

#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
    /* enable USB IP clock */
    CLOCK_EnableUsbhs0PhyPllClock(kCLOCK_UsbPhySrcExt, BOARD_XTAL0_CLK_HZ);
    CLOCK_EnableUsbhs0DeviceClock(kCLOCK_UsbSrcUnused, 0U);
    USB_EhciPhyInit(CONTROLLER_ID, BOARD_XTAL0_CLK_HZ, NULL);
#if defined(FSL_FEATURE_USBHSD_USB_RAM) && (FSL_FEATURE_USBHSD_USB_RAM)
    for (int i = 0; i < FSL_FEATURE_USBHSD_USB_RAM; i++)
    {
        ((uint8_t *)FSL_FEATURE_USBHSD_USB_RAM_BASE_ADDRESS)[i] = 0x00U;
    }
#endif
#endif
}
void USB_DeviceIsrEnable(void)
{
    uint8_t irqNumber;
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
    uint8_t usbDeviceIP3511Irq[] = USB_IRQS;
    irqNumber                    = usbDeviceIP3511Irq[CONTROLLER_ID - kUSB_ControllerLpcIp3511Fs0];
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
    uint8_t usbDeviceIP3511Irq[] = USBHSD_IRQS;
    irqNumber                    = usbDeviceIP3511Irq[CONTROLLER_ID - kUSB_ControllerLpcIp3511Hs0];
#endif
    /* Install isr, set priority, and enable IRQ. */
    NVIC_SetPriority((IRQn_Type)irqNumber, USB_DEVICE_INTERRUPT_PRIORITY);
    EnableIRQ((IRQn_Type)irqNumber);
}
#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle)
{
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
    USB_DeviceLpcIp3511TaskFunction(deviceHandle);
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
    USB_DeviceLpcIp3511TaskFunction(deviceHandle);
#endif
}
#endif

/* Prepare next transfer payload */
static void USB_DeviceVideoPrepareVideoData(void)
{
    usb_device_video_mjpeg_payload_header_struct_t *payloadHeader;
    uint32_t maxPacketSize;
    uint32_t temp32dwFrameInterval;

    g_UsbDeviceVideoVirtualCamera.currentTime += 10000U;

    payloadHeader = (usb_device_video_mjpeg_payload_header_struct_t *)&g_UsbDeviceVideoVirtualCamera.imageBuffer[0];

    payloadHeader->bHeaderLength                = sizeof(usb_device_video_mjpeg_payload_header_struct_t);
    payloadHeader->headerInfoUnion.bmheaderInfo = 0U;
    payloadHeader->headerInfoUnion.headerInfoBits.frameIdentifier = g_UsbDeviceVideoVirtualCamera.currentFrameId;
    g_UsbDeviceVideoVirtualCamera.imageBufferLength = sizeof(usb_device_video_mjpeg_payload_header_struct_t);

    if (g_UsbDeviceVideoVirtualCamera.stillImageTransmission)
    {
        payloadHeader->headerInfoUnion.headerInfoBits.stillImage = 1U;
        maxPacketSize =
            USB_LONG_FROM_LITTLE_ENDIAN_DATA(g_UsbDeviceVideoVirtualCamera.stillCommitStruct->dwMaxPayloadTransferSize);
    }
    else
    {
        maxPacketSize =
            USB_LONG_FROM_LITTLE_ENDIAN_DATA(g_UsbDeviceVideoVirtualCamera.commitStruct->dwMaxPayloadTransferSize);
    }

    if (g_UsbDeviceVideoVirtualCamera.waitForNewInterval)
    {
        temp32dwFrameInterval =
            USB_LONG_FROM_LITTLE_ENDIAN_DATA(g_UsbDeviceVideoVirtualCamera.commitStruct->dwFrameInterval);
        if (g_UsbDeviceVideoVirtualCamera.currentTime < temp32dwFrameInterval)
        {
            return;
        }
        else
        {
            g_UsbDeviceVideoVirtualCamera.currentTime                = 0U;
            g_UsbDeviceVideoVirtualCamera.waitForNewInterval         = 0U;
            payloadHeader->headerInfoUnion.headerInfoBits.endOfFrame = 1U;
            g_UsbDeviceVideoVirtualCamera.stillImageTransmission     = 0U;
            g_UsbDeviceVideoVirtualCamera.currentFrameId ^= 1U;
            if (USB_DEVICE_VIDEO_STILL_IMAGE_TRIGGER_TRANSMIT_STILL_IMAGE ==
                g_UsbDeviceVideoVirtualCamera.stillImageTriggerControl)
            {
                g_UsbDeviceVideoVirtualCamera.stillImageTriggerControl =
                    USB_DEVICE_VIDEO_STILL_IMAGE_TRIGGER_NORMAL_OPERATION;
                g_UsbDeviceVideoVirtualCamera.stillImageTransmission = 1U;
            }
            return;
        }
    }

    for (; g_UsbDeviceVideoVirtualCamera.imageBufferLength < maxPacketSize;
         g_UsbDeviceVideoVirtualCamera.imageBufferLength++)
    {
        g_UsbDeviceVideoVirtualCamera.imageBuffer[g_UsbDeviceVideoVirtualCamera.imageBufferLength] =
            g_UsbDeviceVideoMjpegData[g_UsbDeviceVideoVirtualCamera.imageIndex];
        g_UsbDeviceVideoVirtualCamera.imageIndex++;

        if ((0xFFU == g_UsbDeviceVideoMjpegData[g_UsbDeviceVideoVirtualCamera.imageIndex - 2]) &&
            (0xD9U == g_UsbDeviceVideoMjpegData[g_UsbDeviceVideoVirtualCamera.imageIndex - 1U]))
        {
            temp32dwFrameInterval =
                USB_LONG_FROM_LITTLE_ENDIAN_DATA(g_UsbDeviceVideoVirtualCamera.commitStruct->dwFrameInterval);
            if (g_UsbDeviceVideoVirtualCamera.imageIndex >= g_UsbDeviceVideoMjpegLength)
            {
                g_UsbDeviceVideoVirtualCamera.imageIndex = 0U;
            }
            if (g_UsbDeviceVideoVirtualCamera.currentTime < temp32dwFrameInterval)
            {
                g_UsbDeviceVideoVirtualCamera.waitForNewInterval = 1U;
            }
            else
            {
                g_UsbDeviceVideoVirtualCamera.currentTime                = 0U;
                payloadHeader->headerInfoUnion.headerInfoBits.endOfFrame = 1U;
                g_UsbDeviceVideoVirtualCamera.stillImageTransmission     = 0U;
                g_UsbDeviceVideoVirtualCamera.currentFrameId ^= 1U;
                if (USB_DEVICE_VIDEO_STILL_IMAGE_TRIGGER_TRANSMIT_STILL_IMAGE ==
                    g_UsbDeviceVideoVirtualCamera.stillImageTriggerControl)
                {
                    g_UsbDeviceVideoVirtualCamera.stillImageTriggerControl =
                        USB_DEVICE_VIDEO_STILL_IMAGE_TRIGGER_NORMAL_OPERATION;
                    g_UsbDeviceVideoVirtualCamera.stillImageTransmission = 1U;
                }
            }
            g_UsbDeviceVideoVirtualCamera.imageBufferLength++;
            break;
        }
    }
}

/* USB device video ISO IN endpoint callback */
static usb_status_t USB_DeviceVideoIsoIn(usb_device_handle deviceHandle,
                                         usb_device_endpoint_callback_message_struct_t *event,
                                         void *arg)
{
    if (g_UsbDeviceVideoVirtualCamera.attach)
    {
        USB_DeviceVideoPrepareVideoData();
        return USB_DeviceSendRequest(deviceHandle, USB_VIDEO_VIRTUAL_CAMERA_STREAM_ENDPOINT_IN,
                                     g_UsbDeviceVideoVirtualCamera.imageBuffer,
                                     g_UsbDeviceVideoVirtualCamera.imageBufferLength);
    }

    return kStatus_USB_Error;
}

/* Set to default state */
static void USB_DeviceVideoApplicationSetDefault(void)
{
    g_UsbDeviceVideoVirtualCamera.speed                = USB_SPEED_FULL;
    g_UsbDeviceVideoVirtualCamera.attach               = 0U;
    g_UsbDeviceVideoVirtualCamera.currentMaxPacketSize = HS_STREAM_IN_PACKET_SIZE;
    g_UsbDeviceVideoVirtualCamera.imageBuffer          = s_ImageBuffer;
    g_UsbDeviceVideoVirtualCamera.probeStruct          = &s_ProbeStruct;
    g_UsbDeviceVideoVirtualCamera.commitStruct         = &s_CommitStruct;
    g_UsbDeviceVideoVirtualCamera.stillProbeStruct     = &s_StillProbeStruct;
    g_UsbDeviceVideoVirtualCamera.stillCommitStruct    = &s_StillCommitStruct;

    g_UsbDeviceVideoVirtualCamera.probeStruct->bFormatIndex = USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FORMAT_INDEX;
    g_UsbDeviceVideoVirtualCamera.probeStruct->bFrameIndex  = USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_INDEX;
    USB_LONG_TO_LITTLE_ENDIAN_DATA(USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_DEFAULT_INTERVAL,
                                   g_UsbDeviceVideoVirtualCamera.probeStruct->dwFrameInterval);
    USB_LONG_TO_LITTLE_ENDIAN_DATA(g_UsbDeviceVideoVirtualCamera.currentMaxPacketSize,
                                   g_UsbDeviceVideoVirtualCamera.probeStruct->dwMaxPayloadTransferSize);
    USB_LONG_TO_LITTLE_ENDIAN_DATA(USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_MAX_FRAME_SIZE,
                                   g_UsbDeviceVideoVirtualCamera.probeStruct->dwMaxVideoFrameSize);

    g_UsbDeviceVideoVirtualCamera.commitStruct->bFormatIndex = USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FORMAT_INDEX;
    g_UsbDeviceVideoVirtualCamera.commitStruct->bFrameIndex  = USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_INDEX;
    USB_LONG_TO_LITTLE_ENDIAN_DATA(USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_DEFAULT_INTERVAL,
                                   g_UsbDeviceVideoVirtualCamera.commitStruct->dwFrameInterval);
    USB_LONG_TO_LITTLE_ENDIAN_DATA(g_UsbDeviceVideoVirtualCamera.currentMaxPacketSize,
                                   g_UsbDeviceVideoVirtualCamera.commitStruct->dwMaxPayloadTransferSize);
    USB_LONG_TO_LITTLE_ENDIAN_DATA(USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_MAX_FRAME_SIZE,
                                   g_UsbDeviceVideoVirtualCamera.commitStruct->dwMaxVideoFrameSize);

    g_UsbDeviceVideoVirtualCamera.probeInfo    = 0x03U;
    g_UsbDeviceVideoVirtualCamera.probeLength  = 26U;
    g_UsbDeviceVideoVirtualCamera.commitInfo   = 0x03U;
    g_UsbDeviceVideoVirtualCamera.commitLength = 26U;

    g_UsbDeviceVideoVirtualCamera.stillProbeStruct->bFormatIndex      = USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FORMAT_INDEX;
    g_UsbDeviceVideoVirtualCamera.stillProbeStruct->bFrameIndex       = USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_INDEX;
    g_UsbDeviceVideoVirtualCamera.stillProbeStruct->bCompressionIndex = 0x01U;
    USB_LONG_TO_LITTLE_ENDIAN_DATA(g_UsbDeviceVideoVirtualCamera.currentMaxPacketSize,
                                   g_UsbDeviceVideoVirtualCamera.stillProbeStruct->dwMaxPayloadTransferSize);
    USB_LONG_TO_LITTLE_ENDIAN_DATA(USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_MAX_FRAME_SIZE,
                                   g_UsbDeviceVideoVirtualCamera.stillProbeStruct->dwMaxVideoFrameSize);

    g_UsbDeviceVideoVirtualCamera.stillCommitStruct->bFormatIndex      = USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FORMAT_INDEX;
    g_UsbDeviceVideoVirtualCamera.stillCommitStruct->bFrameIndex       = USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_INDEX;
    g_UsbDeviceVideoVirtualCamera.stillCommitStruct->bCompressionIndex = 0x01U;
    USB_LONG_TO_LITTLE_ENDIAN_DATA(g_UsbDeviceVideoVirtualCamera.currentMaxPacketSize,
                                   g_UsbDeviceVideoVirtualCamera.stillCommitStruct->dwMaxPayloadTransferSize);
    USB_LONG_TO_LITTLE_ENDIAN_DATA(USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_MAX_FRAME_SIZE,
                                   g_UsbDeviceVideoVirtualCamera.stillCommitStruct->dwMaxVideoFrameSize);

    g_UsbDeviceVideoVirtualCamera.stillProbeInfo    = 0x03U;
    g_UsbDeviceVideoVirtualCamera.stillProbeLength  = sizeof(s_StillProbeStruct);
    g_UsbDeviceVideoVirtualCamera.stillCommitInfo   = 0x03U;
    g_UsbDeviceVideoVirtualCamera.stillCommitLength = sizeof(s_StillCommitStruct);

    g_UsbDeviceVideoVirtualCamera.currentTime                            = 0U;
    g_UsbDeviceVideoVirtualCamera.currentFrameId                         = 0U;
    g_UsbDeviceVideoVirtualCamera.currentStreamInterfaceAlternateSetting = 0U;
    g_UsbDeviceVideoVirtualCamera.imageBufferLength                      = 0U;
    g_UsbDeviceVideoVirtualCamera.imageIndex                             = 0U;
    g_UsbDeviceVideoVirtualCamera.waitForNewInterval                     = 0U;
    g_UsbDeviceVideoVirtualCamera.stillImageTransmission                 = 0U;
    g_UsbDeviceVideoVirtualCamera.stillImageTriggerControl = USB_DEVICE_VIDEO_STILL_IMAGE_TRIGGER_NORMAL_OPERATION;
}

/* Device callback */
usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_Success;
    uint8_t *temp8     = (uint8_t *)param;

    switch (event)
    {
        case kUSB_DeviceEventBusReset:
        {
            USB_DeviceControlPipeInit(g_UsbDeviceVideoVirtualCamera.deviceHandle);
            USB_DeviceVideoApplicationSetDefault();
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
            /* Get USB speed to configure the device, including max packet size and interval of the endpoints. */
            if (kStatus_USB_Success == USB_DeviceGetStatus(g_UsbDeviceVideoVirtualCamera.deviceHandle,
                                                           kUSB_DeviceStatusSpeed,
                                                           &g_UsbDeviceVideoVirtualCamera.speed))
            {
                USB_DeviceSetSpeed(g_UsbDeviceVideoVirtualCamera.speed);
            }

            if (USB_SPEED_HIGH == g_UsbDeviceVideoVirtualCamera.speed)
            {
                g_UsbDeviceVideoVirtualCamera.currentMaxPacketSize = HS_STREAM_IN_PACKET_SIZE;
            }
#endif
        }
        break;
        case kUSB_DeviceEventSetConfiguration:
            if (USB_VIDEO_VIRTUAL_CAMERA_CONFIGURE_INDEX == (*temp8))
            {
                g_UsbDeviceVideoVirtualCamera.attach = 1U;
            }
            break;
        case kUSB_DeviceEventSetInterface:
            if ((g_UsbDeviceVideoVirtualCamera.attach) && param)
            {
                uint8_t interface        = (*temp8) & 0xFFU;
                uint8_t alternateSetting = g_UsbDeviceInterface[interface];

                if (g_UsbDeviceVideoVirtualCamera.currentStreamInterfaceAlternateSetting != alternateSetting)
                {
                    if (g_UsbDeviceVideoVirtualCamera.currentStreamInterfaceAlternateSetting)
                    {
                        USB_DeviceDeinitEndpoint(g_UsbDeviceVideoVirtualCamera.deviceHandle,
                                                 USB_VIDEO_VIRTUAL_CAMERA_STREAM_ENDPOINT_IN |
                                                     (USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT));
                    }
                    else
                    {
                        usb_device_endpoint_init_struct_t epInitStruct;
                        usb_device_endpoint_callback_struct_t epCallback;

                        epCallback.callbackFn    = USB_DeviceVideoIsoIn;
                        epCallback.callbackParam = handle;

                        epInitStruct.zlt             = 0U;
                        epInitStruct.transferType    = USB_ENDPOINT_ISOCHRONOUS;
                        epInitStruct.endpointAddress = USB_VIDEO_VIRTUAL_CAMERA_STREAM_ENDPOINT_IN |
                                                       (USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT);
                        if (USB_SPEED_HIGH == g_UsbDeviceVideoVirtualCamera.speed)
                        {
                            epInitStruct.maxPacketSize = HS_STREAM_IN_PACKET_SIZE;
                            epInitStruct.interval      = HS_STREAM_IN_INTERVAL;
                        }
                        else
                        {
                            epInitStruct.maxPacketSize = FS_STREAM_IN_PACKET_SIZE;
                            epInitStruct.interval      = FS_STREAM_IN_INTERVAL;
                        }

                        USB_DeviceInitEndpoint(g_UsbDeviceVideoVirtualCamera.deviceHandle, &epInitStruct, &epCallback);
                        USB_DeviceVideoPrepareVideoData();
                        error = USB_DeviceSendRequest(
                            g_UsbDeviceVideoVirtualCamera.deviceHandle, USB_VIDEO_VIRTUAL_CAMERA_STREAM_ENDPOINT_IN,
                            g_UsbDeviceVideoVirtualCamera.imageBuffer, g_UsbDeviceVideoVirtualCamera.imageBufferLength);
                    }
                    g_UsbDeviceVideoVirtualCamera.currentStreamInterfaceAlternateSetting = alternateSetting;
                }
            }
            break;
        default:
            break;
    }

    return error;
}

/* Get the setup buffer */
usb_status_t USB_DeviceGetSetupBuffer(usb_device_handle handle, usb_setup_struct_t **setupBuffer)
{
    static uint32_t setup[2];
    if (NULL == setupBuffer)
    {
        return kStatus_USB_InvalidParameter;
    }
    *setupBuffer = (usb_setup_struct_t *)&setup;
    return kStatus_USB_Success;
}

/* Configure remote wakeup(Enable or disbale) */
usb_status_t USB_DeviceConfigureRemoteWakeup(usb_device_handle handle, uint8_t enable)
{
    return kStatus_USB_Success;
}
/* Configure endpoint status(Idle or stall) */
usb_status_t USB_DeviceConfigureEndpointStatus(usb_device_handle handle, uint8_t ep, uint8_t status)
{
    if (status)
    {
        if ((USB_VIDEO_VIRTUAL_CAMERA_STREAM_ENDPOINT_IN == (ep & USB_ENDPOINT_NUMBER_MASK)) &&
            (ep & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK))
        {
            return USB_DeviceStallEndpoint(handle, ep);
        }
        else if ((USB_VIDEO_VIRTUAL_CAMERA_CONTROL_ENDPOINT == (ep & USB_ENDPOINT_NUMBER_MASK)) &&
                 (ep & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK))
        {
            return USB_DeviceStallEndpoint(handle, ep);
        }
        else
        {
        }
    }
    else
    {
        if ((USB_VIDEO_VIRTUAL_CAMERA_STREAM_ENDPOINT_IN == (ep & USB_ENDPOINT_NUMBER_MASK)) &&
            (ep & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK))
        {
            return USB_DeviceUnstallEndpoint(handle, ep);
        }
        else if ((USB_VIDEO_VIRTUAL_CAMERA_CONTROL_ENDPOINT == (ep & USB_ENDPOINT_NUMBER_MASK)) &&
                 (ep & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK))
        {
            return USB_DeviceUnstallEndpoint(handle, ep);
        }
        else
        {
        }
    }
    return kStatus_USB_InvalidRequest;
}

/* Get class-specific request buffer */
usb_status_t USB_DeviceGetClassReceiveBuffer(usb_device_handle handle,
                                             usb_setup_struct_t *setup,
                                             uint32_t *length,
                                             uint8_t **buffer)
{
    if ((NULL == buffer) || ((*length) > sizeof(s_SetupOutBuffer)))
    {
        return kStatus_USB_InvalidRequest;
    }
    *buffer = (uint8_t *)&s_SetupOutBuffer[0];
    return kStatus_USB_Success;
}

/* Precess class-specific VS probe request */
static usb_status_t USB_DeviceVideoProcessClassVsProbeRequest(usb_device_handle handle,
                                                              usb_setup_struct_t *setup,
                                                              uint32_t *length,
                                                              uint8_t **buffer)
{
    usb_device_video_probe_and_commit_controls_struct_t *probe;
    usb_status_t error = kStatus_USB_InvalidRequest;
    uint32_t temp32;

    if ((NULL == buffer) || (NULL == length))
    {
        return error;
    }

    probe = (usb_device_video_probe_and_commit_controls_struct_t *)(*buffer);

    switch (setup->bRequest)
    {
        case USB_DEVICE_VIDEO_REQUEST_CODE_SET_CUR:
            if ((*buffer == NULL) || (*length == 0U))
            {
                return error;
            }

            temp32 = USB_LONG_FROM_LITTLE_ENDIAN_DATA(probe->dwFrameInterval);
            if ((temp32 >= USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_MIN_INTERVAL) &&
                (temp32 <= USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_MAX_INTERVAL))
            {
                USB_LONG_TO_LITTLE_ENDIAN_DATA(temp32, g_UsbDeviceVideoVirtualCamera.probeStruct->dwFrameInterval);
            }

            temp32 = USB_LONG_FROM_LITTLE_ENDIAN_DATA(probe->dwMaxPayloadTransferSize);
            if ((temp32) && (temp32 < g_UsbDeviceVideoVirtualCamera.currentMaxPacketSize))
            {
                USB_LONG_TO_LITTLE_ENDIAN_DATA(temp32,
                                               g_UsbDeviceVideoVirtualCamera.probeStruct->dwMaxPayloadTransferSize);
            }

            g_UsbDeviceVideoVirtualCamera.probeStruct->bFormatIndex = probe->bFormatIndex;
            g_UsbDeviceVideoVirtualCamera.probeStruct->bFrameIndex  = probe->bFrameIndex;

            error = kStatus_USB_Success;
            break;
        case USB_DEVICE_VIDEO_REQUEST_CODE_GET_CUR:
            *buffer = (uint8_t *)g_UsbDeviceVideoVirtualCamera.probeStruct;
            *length = g_UsbDeviceVideoVirtualCamera.probeLength;
            error   = kStatus_USB_Success;
            break;
        case USB_DEVICE_VIDEO_REQUEST_CODE_GET_MIN:
            break;
        case USB_DEVICE_VIDEO_REQUEST_CODE_GET_MAX:
            break;
        case USB_DEVICE_VIDEO_REQUEST_CODE_GET_RES:
            break;
        case USB_DEVICE_VIDEO_REQUEST_CODE_GET_LEN:
            *buffer = &g_UsbDeviceVideoVirtualCamera.probeLength;
            *length = sizeof(g_UsbDeviceVideoVirtualCamera.probeLength);
            error   = kStatus_USB_Success;
            break;
        case USB_DEVICE_VIDEO_REQUEST_CODE_GET_INFO:
            *buffer = &g_UsbDeviceVideoVirtualCamera.probeInfo;
            *length = sizeof(g_UsbDeviceVideoVirtualCamera.probeInfo);
            error   = kStatus_USB_Success;
            break;
        case USB_DEVICE_VIDEO_REQUEST_CODE_GET_DEF:
            break;
        default:
            break;
    }
    return error;
}

/* Precess class-specific VS commit request */
static usb_status_t USB_DeviceVideoProcessClassVsCommitRequest(usb_device_handle handle,
                                                               usb_setup_struct_t *setup,
                                                               uint32_t *length,
                                                               uint8_t **buffer)
{
    usb_device_video_probe_and_commit_controls_struct_t *commit;
    usb_status_t error = kStatus_USB_InvalidRequest;
    uint32_t temp32;

    if ((NULL == buffer) || (NULL == length))
    {
        return error;
    }

    commit = (usb_device_video_probe_and_commit_controls_struct_t *)(*buffer);
    switch (setup->bRequest)
    {
        case USB_DEVICE_VIDEO_REQUEST_CODE_SET_CUR:
            if ((*buffer == NULL) || (*length == 0U))
            {
                return error;
            }

            temp32 = USB_LONG_FROM_LITTLE_ENDIAN_DATA(commit->dwFrameInterval);
            if ((temp32 >= USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_MIN_INTERVAL) &&
                (temp32 <= USB_VIDEO_VIRTUAL_CAMERA_MJPEG_FRAME_MAX_INTERVAL))
            {
                USB_LONG_TO_LITTLE_ENDIAN_DATA(temp32, g_UsbDeviceVideoVirtualCamera.commitStruct->dwFrameInterval);
            }

            temp32 = USB_LONG_FROM_LITTLE_ENDIAN_DATA(commit->dwMaxPayloadTransferSize);
            if ((temp32) && (temp32 < g_UsbDeviceVideoVirtualCamera.currentMaxPacketSize))
            {
                USB_LONG_TO_LITTLE_ENDIAN_DATA(temp32,
                                               g_UsbDeviceVideoVirtualCamera.commitStruct->dwMaxPayloadTransferSize);
            }

            g_UsbDeviceVideoVirtualCamera.commitStruct->bFormatIndex = commit->bFormatIndex;
            g_UsbDeviceVideoVirtualCamera.commitStruct->bFrameIndex  = commit->bFrameIndex;
            error                                                    = kStatus_USB_Success;
            break;
        case USB_DEVICE_VIDEO_REQUEST_CODE_GET_CUR:
            *buffer = (uint8_t *)g_UsbDeviceVideoVirtualCamera.commitStruct;
            *length = g_UsbDeviceVideoVirtualCamera.commitLength;
            error   = kStatus_USB_Success;
            break;
        case USB_DEVICE_VIDEO_REQUEST_CODE_GET_LEN:
            *buffer = &g_UsbDeviceVideoVirtualCamera.commitLength;
            *length = sizeof(g_UsbDeviceVideoVirtualCamera.commitLength);
            error   = kStatus_USB_Success;
            break;
        case USB_DEVICE_VIDEO_REQUEST_CODE_GET_INFO:
            *buffer = &g_UsbDeviceVideoVirtualCamera.commitInfo;
            *length = sizeof(g_UsbDeviceVideoVirtualCamera.commitInfo);
            error   = kStatus_USB_Success;
            break;
        default:
            break;
    }
    return error;
}

/* Precess class-specific VS STILL probe request */
static usb_status_t USB_DeviceVideoProcessClassVsStillProbeRequest(usb_device_handle handle,
                                                                   usb_setup_struct_t *setup,
                                                                   uint32_t *length,
                                                                   uint8_t **buffer)
{
    usb_device_video_still_probe_and_commit_controls_struct_t *still_probe;
    usb_status_t error = kStatus_USB_InvalidRequest;
    uint32_t temp32;

    if ((NULL == buffer) || (NULL == length))
    {
        return error;
    }

    still_probe = (usb_device_video_still_probe_and_commit_controls_struct_t *)(*buffer);

    switch (setup->bRequest)
    {
        case USB_DEVICE_VIDEO_REQUEST_CODE_SET_CUR:
            if ((*buffer == NULL) || (*length == 0U))
            {
                return error;
            }

            temp32 = USB_LONG_FROM_LITTLE_ENDIAN_DATA(still_probe->dwMaxPayloadTransferSize);
            if ((temp32) && (temp32 < g_UsbDeviceVideoVirtualCamera.currentMaxPacketSize))
            {
                USB_LONG_TO_LITTLE_ENDIAN_DATA(
                    temp32, g_UsbDeviceVideoVirtualCamera.stillProbeStruct->dwMaxPayloadTransferSize);
            }

            g_UsbDeviceVideoVirtualCamera.stillProbeStruct->bFormatIndex = still_probe->bFormatIndex;
            g_UsbDeviceVideoVirtualCamera.stillProbeStruct->bFrameIndex  = still_probe->bFrameIndex;

            error = kStatus_USB_Success;
            break;
        case USB_DEVICE_VIDEO_REQUEST_CODE_GET_CUR:
            *buffer = (uint8_t *)g_UsbDeviceVideoVirtualCamera.stillProbeStruct;
            *length = g_UsbDeviceVideoVirtualCamera.stillProbeLength;
            error   = kStatus_USB_Success;
            break;
        case USB_DEVICE_VIDEO_REQUEST_CODE_GET_MIN:
            break;
        case USB_DEVICE_VIDEO_REQUEST_CODE_GET_MAX:
            break;
        case USB_DEVICE_VIDEO_REQUEST_CODE_GET_RES:
            break;
        case USB_DEVICE_VIDEO_REQUEST_CODE_GET_LEN:
            *buffer = &g_UsbDeviceVideoVirtualCamera.stillProbeLength;
            *length = sizeof(g_UsbDeviceVideoVirtualCamera.stillProbeLength);
            error   = kStatus_USB_Success;
            break;
        case USB_DEVICE_VIDEO_REQUEST_CODE_GET_INFO:
            *buffer = &g_UsbDeviceVideoVirtualCamera.stillProbeInfo;
            *length = sizeof(g_UsbDeviceVideoVirtualCamera.stillProbeInfo);
            error   = kStatus_USB_Success;
            break;
        case USB_DEVICE_VIDEO_REQUEST_CODE_GET_DEF:
            break;
        default:
            break;
    }
    return error;
}

/* Precess class-specific VS STILL commit request */
static usb_status_t USB_DeviceVideoProcessClassVsStillCommitRequest(usb_device_handle handle,
                                                                    usb_setup_struct_t *setup,
                                                                    uint32_t *length,
                                                                    uint8_t **buffer)
{
    usb_device_video_still_probe_and_commit_controls_struct_t *still_commit;
    usb_status_t error = kStatus_USB_InvalidRequest;
    uint32_t temp32;

    if ((NULL == buffer) || (NULL == length))
    {
        return error;
    }

    still_commit = (usb_device_video_still_probe_and_commit_controls_struct_t *)(*buffer);

    switch (setup->bRequest)
    {
        case USB_DEVICE_VIDEO_REQUEST_CODE_SET_CUR:
            if ((*buffer == NULL) || (*length == 0U))
            {
                return error;
            }

            temp32 = USB_LONG_FROM_LITTLE_ENDIAN_DATA(still_commit->dwMaxPayloadTransferSize);
            if ((temp32) && (temp32 < g_UsbDeviceVideoVirtualCamera.currentMaxPacketSize))
            {
                USB_LONG_TO_LITTLE_ENDIAN_DATA(
                    temp32, g_UsbDeviceVideoVirtualCamera.stillCommitStruct->dwMaxPayloadTransferSize);
            }

            g_UsbDeviceVideoVirtualCamera.stillCommitStruct->bFormatIndex = still_commit->bFormatIndex;
            g_UsbDeviceVideoVirtualCamera.stillCommitStruct->bFrameIndex  = still_commit->bFrameIndex;

            error = kStatus_USB_Success;
            break;
        case USB_DEVICE_VIDEO_REQUEST_CODE_GET_CUR:
            *buffer = (uint8_t *)g_UsbDeviceVideoVirtualCamera.stillCommitStruct;
            *length = g_UsbDeviceVideoVirtualCamera.stillCommitLength;
            error   = kStatus_USB_Success;
            break;
        case USB_DEVICE_VIDEO_REQUEST_CODE_GET_LEN:
            *buffer = &g_UsbDeviceVideoVirtualCamera.stillCommitLength;
            *length = sizeof(g_UsbDeviceVideoVirtualCamera.stillCommitLength);
            error   = kStatus_USB_Success;
            break;
        case USB_DEVICE_VIDEO_REQUEST_CODE_GET_INFO:
            *buffer = &g_UsbDeviceVideoVirtualCamera.stillCommitInfo;
            *length = sizeof(g_UsbDeviceVideoVirtualCamera.stillCommitInfo);
            error   = kStatus_USB_Success;
            break;
        default:
            break;
    }
    return error;
}

/* Precess class-specific VS STILL image trigger request */
static usb_status_t USB_DeviceVideoProcessClassVsStillImageTriggerRequest(usb_device_handle handle,
                                                                          usb_setup_struct_t *setup,
                                                                          uint32_t *length,
                                                                          uint8_t **buffer)
{
    usb_status_t error = kStatus_USB_InvalidRequest;

    if ((NULL == buffer) || (NULL == length))
    {
        return error;
    }

    switch (setup->bRequest)
    {
        case USB_DEVICE_VIDEO_REQUEST_CODE_SET_CUR:
            if ((*buffer == NULL) || (*length == 0U))
            {
                return error;
            }
            g_UsbDeviceVideoVirtualCamera.stillImageTriggerControl = *(*buffer);
            error                                                  = kStatus_USB_Success;
            break;
        case USB_DEVICE_VIDEO_REQUEST_CODE_GET_CUR:
            break;
        case USB_DEVICE_VIDEO_REQUEST_CODE_GET_INFO:
            break;
        default:
            break;
    }
    return error;
}

/* Precess class-specific VS request */
static usb_status_t USB_DeviceVideoProcessClassVsRequest(usb_device_handle handle,
                                                         usb_setup_struct_t *setup,
                                                         uint32_t *length,
                                                         uint8_t **buffer)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    uint8_t cs         = (setup->wValue >> 0x08U) & 0xFFU;

    switch (cs)
    {
        case USB_DEVICE_VIDEO_VS_PROBE_CONTROL:
            error = USB_DeviceVideoProcessClassVsProbeRequest(handle, setup, length, buffer);
            break;
        case USB_DEVICE_VIDEO_VS_COMMIT_CONTROL:
            error = USB_DeviceVideoProcessClassVsCommitRequest(handle, setup, length, buffer);
            break;
        case USB_DEVICE_VIDEO_VS_STILL_PROBE_CONTROL:
            error = USB_DeviceVideoProcessClassVsStillProbeRequest(handle, setup, length, buffer);
            break;
        case USB_DEVICE_VIDEO_VS_STILL_COMMIT_CONTROL:
            error = USB_DeviceVideoProcessClassVsStillCommitRequest(handle, setup, length, buffer);
            break;
        case USB_DEVICE_VIDEO_VS_STILL_IMAGE_TRIGGER_CONTROL:
            error = USB_DeviceVideoProcessClassVsStillImageTriggerRequest(handle, setup, length, buffer);
            break;
        case USB_DEVICE_VIDEO_VS_STREAM_ERROR_CODE_CONTROL:
            break;
        case USB_DEVICE_VIDEO_VS_GENERATE_KEY_FRAME_CONTROL:
            break;
        case USB_DEVICE_VIDEO_VS_UPDATE_FRAME_SEGMENT_CONTROL:
            break;
        case USB_DEVICE_VIDEO_VS_SYNCH_DELAY_CONTROL:
            break;
        default:
            break;
    }
    return error;
}

/* Precess class-specific VC interface request */
static usb_status_t USB_DeviceProcessClassVcInterfaceRequest(usb_device_handle handle,
                                                             usb_setup_struct_t *setup,
                                                             uint32_t *length,
                                                             uint8_t **buffer)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    uint8_t cs         = (setup->wValue >> 0x08U) & 0xFFU;

    if (USB_DEVICE_VIDEO_VC_VIDEO_POWER_MODE_CONTROL == cs)
    {
    }
    else if (USB_DEVICE_VIDEO_VC_REQUEST_ERROR_CODE_CONTROL == cs)
    {
    }
    else
    {
    }
    return error;
}

/* Precess class-specific VC camera terminal request */
static usb_status_t USB_DeviceProcessClassVcCameraTerminalRequest(usb_device_handle handle,
                                                                  usb_setup_struct_t *setup,
                                                                  uint32_t *length,
                                                                  uint8_t **buffer)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    uint8_t cs         = (setup->wValue >> 0x08U) & 0xFFU;

    switch (cs)
    {
        case USB_DEVICE_VIDEO_CT_SCANNING_MODE_CONTROL:
            break;
        case USB_DEVICE_VIDEO_CT_AE_MODE_CONTROL:
            break;
        case USB_DEVICE_VIDEO_CT_AE_PRIORITY_CONTROL:
            break;
        case USB_DEVICE_VIDEO_CT_EXPOSURE_TIME_ABSOLUTE_CONTROL:
            break;
        case USB_DEVICE_VIDEO_CT_EXPOSURE_TIME_RELATIVE_CONTROL:
            break;
        case USB_DEVICE_VIDEO_CT_FOCUS_ABSOLUTE_CONTROL:
            break;
        case USB_DEVICE_VIDEO_CT_FOCUS_RELATIVE_CONTROL:
            break;
        case USB_DEVICE_VIDEO_CT_FOCUS_AUTO_CONTROL:
            break;
        case USB_DEVICE_VIDEO_CT_IRIS_ABSOLUTE_CONTROL:
            break;
        case USB_DEVICE_VIDEO_CT_IRIS_RELATIVE_CONTROL:
            break;
        case USB_DEVICE_VIDEO_CT_ZOOM_ABSOLUTE_CONTROL:
            break;
        case USB_DEVICE_VIDEO_CT_ZOOM_RELATIVE_CONTROL:
            break;
        case USB_DEVICE_VIDEO_CT_PANTILT_ABSOLUTE_CONTROL:
            break;
        case USB_DEVICE_VIDEO_CT_PANTILT_RELATIVE_CONTROL:
            break;
        case USB_DEVICE_VIDEO_CT_ROLL_ABSOLUTE_CONTROL:
            break;
        case USB_DEVICE_VIDEO_CT_ROLL_RELATIVE_CONTROL:
            break;
        case USB_DEVICE_VIDEO_CT_PRIVACY_CONTROL:
            break;
        case USB_DEVICE_VIDEO_CT_FOCUS_SIMPLE_CONTROL:
            break;
        case USB_DEVICE_VIDEO_CT_WINDOW_CONTROL:
            break;
        case USB_DEVICE_VIDEO_CT_REGION_OF_INTEREST_CONTROL:
            break;
        default:
            break;
    }
    return error;
}

/* Precess class-specific VC input terminal request */
static usb_status_t USB_DeviceProcessClassVcInputTerminalRequest(usb_device_handle handle,
                                                                 usb_setup_struct_t *setup,
                                                                 uint32_t *length,
                                                                 uint8_t **buffer)
{
    usb_status_t error = kStatus_USB_InvalidRequest;

    if (USB_DEVICE_VIDEO_ITT_CAMERA == USB_VIDEO_VIRTUAL_CAMERA_VC_INPUT_TERMINAL_TYPE)
    {
        error = USB_DeviceProcessClassVcCameraTerminalRequest(handle, setup, length, buffer);
    }

    return error;
}

/* Precess class-specific VC onput terminal request */
static usb_status_t USB_DeviceProcessClassVcOutputTerminalRequest(usb_device_handle handle,
                                                                  usb_setup_struct_t *setup,
                                                                  uint32_t *length,
                                                                  uint8_t **buffer)
{
    usb_status_t error = kStatus_USB_InvalidRequest;

    return error;
}

/* Precess class-specific VC processing unit request */
static usb_status_t USB_DeviceProcessClassVcProcessingUintRequest(usb_device_handle handle,
                                                                  usb_setup_struct_t *setup,
                                                                  uint32_t *length,
                                                                  uint8_t **buffer)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    uint8_t cs         = (setup->wValue >> 0x08U) & 0xFFU;

    switch (cs)
    {
        case USB_DEVICE_VIDEO_PU_BACKLIGHT_COMPENSATION_CONTROL:
            break;
        case USB_DEVICE_VIDEO_PU_BRIGHTNESS_CONTROL:
            break;
        case USB_DEVICE_VIDEO_PU_CONTRAST_CONTROL:
            break;
        case USB_DEVICE_VIDEO_PU_GAIN_CONTROL:
            break;
        case USB_DEVICE_VIDEO_PU_POWER_LINE_FREQUENCY_CONTROL:
            break;
        case USB_DEVICE_VIDEO_PU_HUE_CONTROL:
            break;
        case USB_DEVICE_VIDEO_PU_SATURATION_CONTROL:
            break;
        case USB_DEVICE_VIDEO_PU_SHARPNESS_CONTROL:
            break;
        case USB_DEVICE_VIDEO_PU_GAMMA_CONTROL:
            break;
        case USB_DEVICE_VIDEO_PU_WHITE_BALANCE_TEMPERATURE_CONTROL:
            break;
        case USB_DEVICE_VIDEO_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL:
            break;
        case USB_DEVICE_VIDEO_PU_WHITE_BALANCE_COMPONENT_CONTROL:
            break;
        case USB_DEVICE_VIDEO_PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL:
            break;
        case USB_DEVICE_VIDEO_PU_DIGITAL_MULTIPLIER_CONTROL:
            break;
        case USB_DEVICE_VIDEO_PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL:
            break;
        case USB_DEVICE_VIDEO_PU_HUE_AUTO_CONTROL:
            break;
        case USB_DEVICE_VIDEO_PU_ANALOG_VIDEO_STANDARD_CONTROL:
            break;
        case USB_DEVICE_VIDEO_PU_ANALOG_LOCK_STATUS_CONTROL:
            break;
        case USB_DEVICE_VIDEO_PU_CONTRAST_AUTO_CONTROL:
            break;
        default:
            break;
    }

    return error;
}

/* Precess class-specific VC request */
static usb_status_t USB_DeviceProcessClassVcRequest(usb_device_handle handle,
                                                    usb_setup_struct_t *setup,
                                                    uint32_t *length,
                                                    uint8_t **buffer)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    uint8_t entityId   = (uint8_t)(setup->wIndex >> 0x08U);
    if (0U == entityId)
    {
        error = USB_DeviceProcessClassVcInterfaceRequest(handle, setup, length, buffer);
    }
    else if (USB_VIDEO_VIRTUAL_CAMERA_VC_INPUT_TERMINAL_ID == entityId)
    {
        error = USB_DeviceProcessClassVcInputTerminalRequest(handle, setup, length, buffer);
    }
    else if (USB_VIDEO_VIRTUAL_CAMERA_VC_OUTPUT_TERMINAL_ID == entityId)
    {
        error = USB_DeviceProcessClassVcOutputTerminalRequest(handle, setup, length, buffer);
    }
    else if (USB_VIDEO_VIRTUAL_CAMERA_VC_PROCESSING_UNIT_ID == entityId)
    {
        error = USB_DeviceProcessClassVcProcessingUintRequest(handle, setup, length, buffer);
    }
    else
    {
    }
    return error;
}

/* Precess class-specific request */
usb_status_t USB_DeviceProcessClassRequest(usb_device_handle handle,
                                           usb_setup_struct_t *setup,
                                           uint32_t *length,
                                           uint8_t **buffer)
{
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint8_t interface_index = (uint8_t)setup->wIndex;

    switch (setup->bmRequestType)
    {
        case USB_DEVICE_VIDEO_GET_REQUEST_INTERFACE:
            if (USB_VIDEO_VIRTUAL_CAMERA_CONTROL_INTERFACE_INDEX == interface_index)
            {
                error = USB_DeviceProcessClassVcRequest(handle, setup, length, buffer);
            }
            else if (USB_VIDEO_VIRTUAL_CAMERA_STREAM_INTERFACE_INDEX == interface_index)
            {
                error = USB_DeviceVideoProcessClassVsRequest(handle, setup, length, buffer);
            }
            else
            {
            }
            break;
        case USB_DEVICE_VIDEO_SET_REQUEST_INTERFACE:
            if (USB_VIDEO_VIRTUAL_CAMERA_CONTROL_INTERFACE_INDEX == interface_index)
            {
                error = USB_DeviceProcessClassVcRequest(handle, setup, length, buffer);
            }
            else if (USB_VIDEO_VIRTUAL_CAMERA_STREAM_INTERFACE_INDEX == interface_index)
            {
                error = USB_DeviceVideoProcessClassVsRequest(handle, setup, length, buffer);
            }
            else
            {
            }
            break;
        default:
            break;
    }

    return error;
}

static void USB_DeviceApplicationInit(void)
{
    USB_DeviceClockInit();
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

    USB_DeviceVideoApplicationSetDefault();

    if (kStatus_USB_Success !=
        USB_DeviceInit(CONTROLLER_ID, USB_DeviceCallback, &g_UsbDeviceVideoVirtualCamera.deviceHandle))
    {
        usb_echo("USB device video virtual camera failed\r\n");
        return;
    }
    else
    {
        usb_echo("USB device video virtual camera demo\r\n");
    }

    USB_DeviceIsrEnable();

    /*Add one delay here to make the DP pull down long enough to allow host to detect the previous disconnection.*/
    SDK_DelayAtLeastUs(5000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    USB_DeviceRun(g_UsbDeviceVideoVirtualCamera.deviceHandle);
}

#if defined(__CC_ARM) || (defined(__ARMCC_VERSION)) || defined(__GNUC__)
int main(void)
#else
void main(void)
#endif
{
    /* set BOD VBAT level to 1.65V */
    POWER_SetBodVbatLevel(kPOWER_BodVbatLevel1650mv, kPOWER_BodHystLevel50mv, false);
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);
    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    NVIC_ClearPendingIRQ(USB0_IRQn);
    NVIC_ClearPendingIRQ(USB0_NEEDCLK_IRQn);
    NVIC_ClearPendingIRQ(USB1_IRQn);
    NVIC_ClearPendingIRQ(USB1_NEEDCLK_IRQn);

    POWER_DisablePD(kPDRUNCFG_PD_USB0_PHY); /*< Turn on USB0 Phy */
    POWER_DisablePD(kPDRUNCFG_PD_USB1_PHY); /*< Turn on USB1 Phy */

    /* reset the IP to make sure it's in reset state. */
    RESET_PeripheralReset(kUSB0D_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB0HSL_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB0HMR_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1H_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1D_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1RAM_RST_SHIFT_RSTn);

#if (defined USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS)
    CLOCK_EnableClock(kCLOCK_Usbh1);
    /* Put PHY powerdown under software control */
    *((uint32_t *)(USBHSH_BASE + 0x50)) = USBHSH_PORTMODE_SW_PDCOM_MASK;
    /* According to reference mannual, device mode setting has to be set by access usb host register */
    *((uint32_t *)(USBHSH_BASE + 0x50)) |= USBHSH_PORTMODE_DEV_ENABLE_MASK;
    /* enable usb1 host clock */
    CLOCK_DisableClock(kCLOCK_Usbh1);
#endif
#if (defined USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS)
    POWER_DisablePD(kPDRUNCFG_PD_USB0_PHY); /*< Turn on USB Phy */
    CLOCK_SetClkDiv(kCLOCK_DivUsb0Clk, 1, false);
    CLOCK_AttachClk(kFRO_HF_to_USB0_CLK);
    /* enable usb0 host clock */
    CLOCK_EnableClock(kCLOCK_Usbhsl0);
    /*According to reference mannual, device mode setting has to be set by access usb host register */
    *((uint32_t *)(USBFSH_BASE + 0x5C)) |= USBFSH_PORTMODE_DEV_ENABLE_MASK;
    /* disable usb0 host clock */
    CLOCK_DisableClock(kCLOCK_Usbhsl0);
#endif

    USB_DeviceApplicationInit();

    while (1U)
    {
#if USB_DEVICE_CONFIG_USE_TASK
        USB_DeviceTaskFn(g_UsbDeviceVideoVirtualCamera.deviceHandle);
#endif
    }
}