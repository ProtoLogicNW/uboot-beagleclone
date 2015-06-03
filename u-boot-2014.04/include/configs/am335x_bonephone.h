/*
 * am335x_bonelogic.h
 *
 */

#ifndef __CONFIG_AM335X_BONEPHONE_H
#define __CONFIG_AM335X_BONEPHONE_H

#include <configs/ti_am335x_common.h>

#undef CONFIG_DRIVER_TI_CPSW

#undef CONFIG_ENV_IS_NOWHERE

#define MACH_TYPE_TIAM335EVM		3589	/* Until the next sync */
#define CONFIG_MACH_TYPE		MACH_TYPE_TIAM335EVM
#define CONFIG_BOARD_LATE_INIT

/* Clock Defines */
#define V_OSCK				24000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

/* Custom script for NOR */
#define CONFIG_SYS_LDSCRIPT		"board/ti/bonephone/u-boot.lds"

/* Enhance our eMMC support / experience. */
#if 0
#define CONFIG_CMD_GPT
#define CONFIG_EFI_PARTITION
#define CONFIG_PARTITION_UUIDS
#define CONFIG_CMD_PART
#endif

#define CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG

/* 32kB to match 256kBit i2c eeprom */
#define CONFIG_ENV_SIZE			(32 << 10)
#define CONFIG_CMD_EEPROM
#define CONFIG_ENV_EEPROM_IS_ON_I2C
#define CONFIG_I2C_ENV_EEPROM_BUS 0
#define CONFIG_SYS_I2C_EEPROM_ADDR 0x50
#define CONFIG_SYS_DEF_EEPROM_ADDR CONFIG_SYS_I2C_EEPROM_ADDR
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 2
#define CONFIG_SYS_I2C_MULTI_EEPROMS

#define CONFIG_ENV_IS_IN_EEPROM
#define CONFIG_ENV_OFFSET	0x00 /* beginning of the EEPROM */


#ifndef CONFIG_SPL_BUILD

//Backlight PWM ... remove this
#define PWM_TICKS	0x1388
#define PWM_DUTY	0x200

/*#define CONFIG_VIDEO*/
#if defined(CONFIG_VIDEO)
#define CONFIG_VIDEO_DA8XX
#define CONFIG_CFB_CONSOLE
#define CONFIG_VGA_AS_SINGLE_DEVICE
#define CONFIG_SPLASH_SCREEN
#define CONFIG_SPLASH_SCREEN_ALIGN
#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_BMP_RLE8
#define CONFIG_VIDEO_BMP_LOGO
#define CONFIG_CMD_BMP
#define DA8XX_LCD_CNTL_BASE	LCD_CNTL_BASE
#define CONFIG_SYS_CONSOLE_BG_COL	0xff
#define CONFIG_SYS_CONSOLE_FG_COL	0x00
#endif

/* 16MB BOOT FLASH config
S1 0x00000 - 0x1FFFF (128k) MLO (~76k currently)
S2 0x20000 - 0xBFFFF (512k) u-boot.img 
S3 0xA0000 - 0xAFFFF (64k) dtb (~32k currently)
S4 not used 
S5 0x0B0000 - 0x4AFFFF (4096k) zImage
S6 0x4B0000 - 0xFFFFFF (11584k) rootfs
*/
#define CONFIG_EXTRA_ENV_SETTINGS \
	"s1=0\0"\
	"z1=0x1FFFF\0"\
	"s2=0x20000\0"\
	"z2=0x7FFFF\0"\
	"s3=0xA0000\0"\
	"z3=0x0FFFF\0"\
	"s4=0x00000\0"\
	"z4=0x00000\0"\
	"s5=0x0B0000\0"\
	"z5=0x3FFFFF\0"\
	"s6=0x4B0000\0"\
	"z6=0xB4FFFF\0"\
	"bootdbg=earlyprintk=serial,ttyO0,115200\0" \
	"loadaddr=0x80200000\0" \
	"loadimage=sf read 0x80300000 ${s5} ${z5};\0" \
	"loaddtb=sf read 0x815f0000 ${s3} ${z3};\0" \
	"loadinitrd=sf read 0x82000000 ${s4} ${z4};\0" \
	"loadrootfs=sf read 0x82000000 ${s6} ${z6};\0" \
	"dtb=am335x-beaglephone.dtb\0"\
	"sfs=beaglephone_boot.sfs\0"\
	"bootargs_spi=console=ttyO0,115200n8 root=/dev/ram0 rw ramdisk_size=65536 initrd=0x82000000,32M rootfstype=squashfs earlyprintk=serial,ttyO0,115200 consoleblank=0\0"\
	"bootargs_sfs=console=ttyO0,115200n8 root=/dev/ram0 rw initrd=0x82000000,4M init=/linuxrc earlyprintk=serial,ttyO0,115200 consoleblank=0 panic=10\0"\
	"bootargs_mmc=console=ttyO0,115200n8 root=/dev/mmcblk0p2 rw rootwait earlyprintk=serial,ttyO0,115200 consoleblank=0\0"\
	"boot_spi=echo Booting SPI; sf probe 0;run loadimage;run loaddtb;run loadrootfs; setenv bootargs ${bootargs_spi}; bootz 0x80300000 - 0x815f0000;\0"\
	"boot_sfs=sf probe 0;run loadimage;run loaddtb;run loadinitrd; setenv bootargs ${bootargs_sfs}; bootz 0x80300000 - 0x815f0000;\0"\
	"boot_mmc=run boot_mmc1;\0"\
	"boot_mmc1=mmc dev 0; mmc rescan; fatload mmc :1 0x815f0000 ${dtb}; run boot_mmc1;\0"\
	"boot_mmc2=echo Booting MMC; fatload mmc :1 0x80300000 zImage; setenv bootargs ${bootargs_mmc}; bootz 0x80300000 - 0x815f0000;\0"\
	"beaglephone_boot=mmc dev 0; if fatload mmc :1 0x815f0000 ${dtb}; then run boot_mmc2; else run boot_spi; fi;\0"\
	"updateprep=mmc dev 0; mmc rescan; sf probe 0;\0"\
	"updtboot=run updateprep;fatload mmc 0 ${loadaddr} MLO.byteswap;sf update ${loadaddr} ${s1} ${filesize};fatload mmc 0 ${loadaddr} u-boot.img; sf update ${loadaddr} ${s2} ${filesize};\0"\
	"updtimg=run updateprep;fatload mmc 0 ${loadaddr} zImage; sf update ${loadaddr} ${s5} ${filesize};fatload mmc 0 ${loadaddr} ${dtb}; sf update ${loadaddr} ${s3} ${filesize};\0"\
	"updtfs=run updateprep;fatload mmc 0 ${loadaddr} ${sfs}; sf update ${loadaddr} ${s6} ${filesize};\0"\
	"updtall=run updtboot; run updtimg; run updtfs;\0"\
	"nfsburn=nfs 0x80200000 169.254.99.129:/srv/nfs/spi.img; sf probe 0; sf update ${loadaddr} 0 ${filesize};\0"\
	"serverip=169.254.99.129;\0"\
	"ipaddr=169.254.99.130;\0"\
	DFUARGS
#define CONFIG_BOOTCOMMAND  "run beaglephone_boot;" 

#endif

/* NS16550 Configuration */
#define CONFIG_SYS_NS16550_COM1		0x44e09000	/* Base EVM has UART0 */
#define CONFIG_SYS_NS16550_COM2		0x48022000	/* UART1 */
#define CONFIG_SYS_NS16550_COM3		0x48024000	/* UART2 */
#define CONFIG_SYS_NS16550_COM4		0x481a6000	/* UART3 */
#define CONFIG_SYS_NS16550_COM5		0x481a8000	/* UART4 */
#define CONFIG_SYS_NS16550_COM6		0x481aa000	/* UART5 */
#define CONFIG_BAUDRATE			115200

/* PMIC support */
#define CONFIG_POWER_TPS65217
#define CONFIG_POWER_TPS65910

/* SPL */
#ifndef CONFIG_NOR_BOOT
#define CONFIG_SPL_POWER_SUPPORT
#define CONFIG_SPL_YMODEM_SUPPORT

/* Bootcount using the RTC block */
#define CONFIG_BOOTCOUNT_LIMIT
#define CONFIG_BOOTCOUNT_AM33XX

/* USB gadget RNDIS */
#define CONFIG_SPL_MUSB_NEW_SUPPORT

/* General network SPL, both CPSW and USB gadget RNDIS */
#if 0
#define CONFIG_SPL_NET_SUPPORT
#define CONFIG_SPL_ENV_SUPPORT
#define CONFIG_SPL_NET_VCI_STRING	"BeagleClone U-Boot SPL"
#endif

/* SPI flash. */
#define CONFIG_SPL_SPI_SUPPORT
#define CONFIG_SPL_SPI_FLASH_SUPPORT
#define CONFIG_SPL_SPI_LOAD
#define CONFIG_SPL_SPI_BUS		0
#define CONFIG_SPL_SPI_CS		0
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x20000

#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/am33xx/u-boot-spl.lds"

#endif

/*
 * For NOR boot, we must set this to the start of where NOR is mapped
 * in memory.
 */
#ifdef CONFIG_NOR_BOOT
#define CONFIG_SYS_TEXT_BASE		0x08000000
#endif

/*
 * USB configuration.  We enable MUSB support, both for host and for
 * gadget.  We set USB0 as peripheral and USB1 as host, based on the
 * board schematic and physical port wired to each.  Then for host we
 * add mass storage support and for gadget we add both RNDIS ethernet
 * and DFU.
 */
#define CONFIG_USB_MUSB_DSPS
#define CONFIG_ARCH_MISC_INIT
#define CONFIG_MUSB_GADGET
#define CONFIG_MUSB_PIO_ONLY
#define CONFIG_MUSB_DISABLE_BULK_COMBINE_SPLIT
#define CONFIG_USB_GADGET
#define CONFIG_USBDOWNLOAD_GADGET
#define CONFIG_USB_GADGET_DUALSPEED
#define CONFIG_USB_GADGET_VBUS_DRAW	2
#define CONFIG_MUSB_HOST
#define CONFIG_AM335X_USB0
#define CONFIG_AM335X_USB0_MODE	MUSB_PERIPHERAL
#define CONFIG_AM335X_USB1
#define CONFIG_AM335X_USB1_MODE MUSB_HOST

#ifdef CONFIG_MUSB_HOST
#define CONFIG_CMD_USB
#define CONFIG_USB_STORAGE
#endif

#ifdef CONFIG_MUSB_GADGET
#define CONFIG_USB_ETHER
#define CONFIG_USB_ETH_RNDIS
#define CONFIG_USBNET_HOST_ADDR	"de:ad:be:af:00:00"

/* USB TI's IDs */
#define CONFIG_G_DNL_VENDOR_NUM 0x0403
#define CONFIG_G_DNL_PRODUCT_NUM 0xBD00
#define CONFIG_G_DNL_MANUFACTURER "Texas Instruments"
#endif /* CONFIG_MUSB_GADGET */

#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_USBETH_SUPPORT)
/* disable host part of MUSB in SPL */
#undef CONFIG_MUSB_HOST
/* disable EFI partitions and partition UUID support */
#undef CONFIG_PARTITION_UUIDS
#undef CONFIG_EFI_PARTITION
/*
 * Disable CPSW SPL support so we fit within the 101KiB limit.
 */
#undef CONFIG_SPL_ETH_SUPPORT
#endif

/* USB Device Firmware Update support */
#define CONFIG_DFU_FUNCTION
#define CONFIG_DFU_MMC
#define CONFIG_CMD_DFU
#define DFU_ALT_INFO_MMC \
	"dfu_alt_info_mmc=" \
	"boot part 0 1;" \
	"rootfs part 0 2;" \
	"MLO fat 0 1;" \
	"MLO.raw mmc 100 100;" \
	"u-boot.img.raw mmc 300 400;" \
	"spl-os-args.raw mmc 80 80;" \
	"spl-os-image.raw mmc 900 2000;" \
	"spl-os-args fat 0 1;" \
	"spl-os-image fat 0 1;" \
	"u-boot.img fat 0 1;" \
	"uEnv.txt fat 0 1\0"
#ifdef CONFIG_NAND
#define CONFIG_DFU_NAND
#define DFU_ALT_INFO_NAND \
	"dfu_alt_info_nand=" \
	"SPL part 0 1;" \
	"SPL.backup1 part 0 2;" \
	"SPL.backup2 part 0 3;" \
	"SPL.backup3 part 0 4;" \
	"u-boot part 0 5;" \
	"u-boot-spl-os part 0 6;" \
	"kernel part 0 8;" \
	"rootfs part 0 9\0"
#else
#define DFU_ALT_INFO_NAND ""
#endif
#define CONFIG_DFU_RAM
#define DFU_ALT_INFO_RAM \
	"dfu_alt_info_ram=" \
	"kernel ram 0x80200000 0xD80000;" \
	"fdt ram 0x80F80000 0x80000;" \
	"ramdisk ram 0x81000000 0x4000000\0"
#define DFUARGS \
	"dfu_alt_info_emmc=rawemmc mmc 0 3751936\0" \
	DFU_ALT_INFO_MMC \
	DFU_ALT_INFO_RAM \
	DFU_ALT_INFO_NAND

/*
 * Default to using SPI for environment, etc.
 * 0x000000 - 0x020000 : SPL (128KiB)
 * 0x020000 - 0x0A0000 : U-Boot (512KiB)
 * 0x0A0000 - 0x0BFFFF : First copy of U-Boot Environment (128KiB)
 * 0x0C0000 - 0x0DFFFF : Second copy of U-Boot Environment (128KiB)
 * 0x0E0000 - 0x442000 : Linux Kernel
 * 0x442000 - 0x800000 : Userland
 */
#if defined(CONFIG_SPI_BOOT)
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_SPI_MAX_HZ		CONFIG_SF_DEFAULT_SPEED
#define CONFIG_ENV_SECT_SIZE		(4 << 10) /* 4 KB sectors */
#define CONFIG_ENV_OFFSET		(768 << 10) /* 768 KiB in */
#define CONFIG_ENV_OFFSET_REDUND	(896 << 10) /* 896 KiB in */
#define MTDIDS_DEFAULT			"nor0=m25p80-flash.0"
#define MTDPARTS_DEFAULT		"mtdparts=m25p80-flash.0:128k(SPL)," \
					"512k(u-boot),128k(u-boot-env1)," \
					"128k(u-boot-env2),3464k(kernel)," \
					"-(rootfs)"

#elif defined(CONFIG_EMMC_BOOT)
#undef CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV		1
#define CONFIG_SYS_MMC_ENV_PART		2
#define CONFIG_ENV_OFFSET		0x0
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#endif

/* SPI flash. */
#define CONFIG_CMD_SF
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_SPANSION
#define CONFIG_SPI_FLASH_MACRONIX
#define CONFIG_SPI_FLASH_BAR
#define CONFIG_SF_DEFAULT_SPEED		24000000

/* Network. */
#if 0
#define CONFIG_PHY_GIGE
#define CONFIG_PHYLIB
#define CONFIG_PHY_SMSC
#endif

#endif	/* ! __CONFIG_AM335X_BONEPHONE_H */
