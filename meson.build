# DEVICE INCLUDES #
cmsis_includes = include_directories('./CMSIS/Include/')
device_includes = include_directories('./devices/LPC55S69/')
cmsis_driver_includes = include_directories('./devices/LPC55S69/cmsis_drivers/')
driver_includes = include_directories('./devices/LPC55S69/drivers')
sdk_device_includes = [cmsis_includes, device_includes,
		       cmsis_driver_includes, driver_includes]
# DEVICE SRCS #
device_srcs = files(['./devices/LPC55S69/system_LPC55S69_cm33_core0.c'])
driver_srcs = files([
		     #'./devices/LPC55S69/drivers/fsl_anactrl.c',
		     #'./devices/LPC55S69/drivers/fsl_casper.c',
		     './devices/LPC55S69/drivers/fsl_clock.c',
		     #'./devices/LPC55S69/drivers/fsl_cmp.c',
		     './devices/LPC55S69/drivers/fsl_common.c',
		     #'./devices/LPC55S69/drivers/fsl_crc.c',
		     #'./devices/LPC55S69/drivers/fsl_ctimer.c',
		     #'./devices/LPC55S69/drivers/fsl_dma.c',
		     './devices/LPC55S69/drivers/fsl_flexcomm.c',
		     #'./devices/LPC55S69/drivers/fsl_gint.c',
		     './devices/LPC55S69/drivers/fsl_gpio.c',
		     #'./devices/LPC55S69/drivers/fsl_hashcrypt.c',
		     #'./devices/LPC55S69/drivers/fsl_i2c.c',
		     #'./devices/LPC55S69/drivers/fsl_i2c_dma.c',
		     #'./devices/LPC55S69/drivers/fsl_i2c_freertos.c',
		     #'./devices/LPC55S69/drivers/fsl_i2s.c',
		     #'./devices/LPC55S69/drivers/fsl_i2s_dma.c',
		     #'./devices/LPC55S69/drivers/fsl_iap.c',
		     #'./devices/LPC55S69/drivers/fsl_inputmux.c',
		     #'./devices/LPC55S69/drivers/fsl_lpadc.c',
		     #'./devices/LPC55S69/drivers/fsl_mrt.c',
		     #'./devices/LPC55S69/drivers/fsl_ostimer.c',
		     #'./devices/LPC55S69/drivers/fsl_pint.c',
		     #'./devices/LPC55S69/drivers/fsl_plu.c',
		     './devices/LPC55S69/drivers/fsl_power.c',
		     #'./devices/LPC55S69/drivers/fsl_powerquad_basic.c',
		     #'./devices/LPC55S69/drivers/fsl_powerquad_cmsis.c',
		     #'./devices/LPC55S69/drivers/fsl_powerquad_data.c',
		     #'./devices/LPC55S69/drivers/fsl_powerquad_filter.c',
		     #'./devices/LPC55S69/drivers/fsl_powerquad_math.c',
		     #'./devices/LPC55S69/drivers/fsl_powerquad_matrix.c',
		     #'./devices/LPC55S69/drivers/fsl_powerquad_transform.c',
		     #'./devices/LPC55S69/drivers/fsl_prince.c',
		     #'./devices/LPC55S69/drivers/fsl_puf.c',
		     './devices/LPC55S69/drivers/fsl_reset.c',
		     #'./devices/LPC55S69/drivers/fsl_rng.c',
		     #'./devices/LPC55S69/drivers/fsl_rtc.c',
		     #'./devices/LPC55S69/drivers/fsl_sctimer.c',
		     #'./devices/LPC55S69/drivers/fsl_sdif.c',
		     #'./devices/LPC55S69/drivers/fsl_spi.c',
		     #'./devices/LPC55S69/drivers/fsl_spi_dma.c',
		     #'./devices/LPC55S69/drivers/fsl_spi_freertos.c',
		     #'./devices/LPC55S69/drivers/fsl_sysctl.c',
		     './devices/LPC55S69/drivers/fsl_usart.c',
		     #'./devices/LPC55S69/drivers/fsl_usart_dma.c',
		     #'./devices/LPC55S69/drivers/fsl_usart_freertos.c',
		     #'./devices/LPC55S69/drivers/fsl_utick.c',
		     #'./devices/LPC55S69/drivers/fsl_wwdt.c'
		   ])

sdk_device_srcs = [device_srcs, driver_srcs]

# MIDDLEWARE INCLUDES #
usb_includes = include_directories('./middleware/usb/include/')
usb_host_includes = include_directories('./middleware/usb/host/')
usb_host_class_includes = include_directories('./middleware/usb/host/class/')
usb_phy_includes = include_directories('./middleware/usb/phy/')

fatfs_includes = include_directories('./middleware/fatfs/source/')
fatfs_usb_includes = include_directories('./middleware/fatfs/source/fsl_usb_disk/')
fatfs_ram_includes = include_directories('./middleware/fatfs/source/fsl_ram_disk/')

sdk_mw_includes = [usb_includes, usb_host_includes, usb_host_class_includes,
		   usb_phy_includes, fatfs_includes, fatfs_usb_includes,
		   fatfs_ram_includes]

# MIDDLEWARE SRCS #
usb_host_srcs = files([
		       './middleware/usb/host/usb_host_devices.c',
		       './middleware/usb/host/usb_host_framework.c',
		       './middleware/usb/host/usb_host_hci.c',
		       './middleware/usb/host/usb_host_ip3516hs.c',
		       './middleware/usb/host/usb_host_ohci.c'])
usb_host_class_srcs = files([
#			     './middleware/usb/host/class/usb_host_audio.c',
#			     './middleware/usb/host/class/usb_host_cdc.c',
#			     './middleware/usb/host/class/usb_host_cdc_rndis.c',
#			     './middleware/usb/host/class/usb_host_hid.c',
			     './middleware/usb/host/class/usb_host_hub_app.c',
			     './middleware/usb/host/class/usb_host_hub.c',
			     './middleware/usb/host/class/usb_host_msd.c',
			     './middleware/usb/host/class/usb_host_msd_ufi.c',
#			     './middleware/usb/host/class/usb_host_phdc.c',
#			     './middleware/usb/host/class/usb_host_printer.c',
#			     './middleware/usb/host/class/usb_host_video.c',
			   ])
usb_phy_srcs = files(['./middleware/usb/phy/usb_phy.c'])

fatfs_srcs = files([
		    './middleware/fatfs/source/diskio.c',
		    './middleware/fatfs/source/ff.c',
		    './middleware/fatfs/source/ffsystem.c',
		    './middleware/fatfs/source/ffunicode.c',
		    './middleware/fatfs/source/fsl_ram_disk/fsl_ram_disk.c',
		    './middleware/fatfs/source/fsl_usb_disk/fsl_usb_disk_bm.c'
		  ])

sdk_mw_srcs = [usb_host_srcs, usb_host_class_srcs, usb_phy_srcs, fatfs_srcs]

# COMPONENT INCLUDES #
osa_includes = include_directories('./components/osa')
list_includes = include_directories('./components/lists/')

# COMPONENT SOURCES #
osa_srcs = files(['./components/osa/fsl_os_abstraction_bm.c'])
list_srcs = files(['./components/lists/generic_list.c'])

sdk_component_includes = [osa_includes, list_includes]
sdk_component_srcs = [osa_srcs, list_srcs]

# TOP LEVEL INCLUDES #
sdk_includes = [sdk_device_includes, sdk_mw_includes, sdk_component_includes]
sdk_srcs = [sdk_device_srcs, sdk_mw_srcs, sdk_component_srcs]
