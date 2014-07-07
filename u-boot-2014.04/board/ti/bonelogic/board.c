/*
 * board.c
 *
 * Board functions for TI AM335X based boards
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <spl.h>
#include <asm/arch/cpu.h>
#include <asm/arch/hardware.h>
#include <asm/arch/omap.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mem.h>
#include <asm/io.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <i2c.h>
#include <miiphy.h>
#include <cpsw.h>
#include <power/tps65217.h>
#include <power/tps65910.h>
#include <environment.h>
#include <watchdog.h>
#include "board.h"

#include "../../../drivers/video/da8xx-fb.h"
#include <bmp_layout.h>


DECLARE_GLOBAL_DATA_PTR;

/* GPIO that controls power to DDR on EVM-SK */
#define GPIO_DDR_VTT_EN		7

static struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

static int board_video_init(void);

#if 0
static int read_eeprom()
{
	/* Check if baseboard eeprom is available */
	if (i2c_probe(CONFIG_SYS_I2C_EEPROM_ADDR)) {
		puts("Could not probe the EEPROM; something fundamentally "
			"wrong on the I2C bus.\n");
		return -ENODEV;
	}

	/* read the eeprom using i2c */
	if (i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR, 0, 2, (uchar *)header,
		     sizeof(struct am335x_baseboard_id))) {
		puts("Could not read the EEPROM; something fundamentally"
			" wrong on the I2C bus.\n");
		return -EIO;
	}

	if (header->magic != 0xEE3355AA) {
		/*
		 * read the eeprom using i2c again,
		 * but use only a 1 byte address
		 */
		if (i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR, 0, 1, (uchar *)header,
			     sizeof(struct am335x_baseboard_id))) {
			puts("Could not read the EEPROM; something "
				"fundamentally wrong on the I2C bus.\n");
			return -EIO;
		}

		if (header->magic != 0xEE3355AA) {
			printf("Incorrect magic number (0x%x) in EEPROM\n",
					header->magic);
			return -EINVAL;
		}
	}
	return 0;
}
#endif

#if defined(CONFIG_SPL_BUILD) || defined(CONFIG_NOR_BOOT)
static const struct ddr_data ddr2_data = {
	.datardsratio0 = ((MT47H128M16RT25E_RD_DQS<<30) |
			  (MT47H128M16RT25E_RD_DQS<<20) |
			  (MT47H128M16RT25E_RD_DQS<<10) |
			  (MT47H128M16RT25E_RD_DQS<<0)),
	.datawdsratio0 = ((MT47H128M16RT25E_WR_DQS<<30) |
			  (MT47H128M16RT25E_WR_DQS<<20) |
			  (MT47H128M16RT25E_WR_DQS<<10) |
			  (MT47H128M16RT25E_WR_DQS<<0)),
	.datawiratio0 = ((MT47H128M16RT25E_PHY_WRLVL<<30) |
			 (MT47H128M16RT25E_PHY_WRLVL<<20) |
			 (MT47H128M16RT25E_PHY_WRLVL<<10) |
			 (MT47H128M16RT25E_PHY_WRLVL<<0)),
	.datagiratio0 = ((MT47H128M16RT25E_PHY_GATELVL<<30) |
			 (MT47H128M16RT25E_PHY_GATELVL<<20) |
			 (MT47H128M16RT25E_PHY_GATELVL<<10) |
			 (MT47H128M16RT25E_PHY_GATELVL<<0)),
	.datafwsratio0 = ((MT47H128M16RT25E_PHY_FIFO_WE<<30) |
			  (MT47H128M16RT25E_PHY_FIFO_WE<<20) |
			  (MT47H128M16RT25E_PHY_FIFO_WE<<10) |
			  (MT47H128M16RT25E_PHY_FIFO_WE<<0)),
	.datawrsratio0 = ((MT47H128M16RT25E_PHY_WR_DATA<<30) |
			  (MT47H128M16RT25E_PHY_WR_DATA<<20) |
			  (MT47H128M16RT25E_PHY_WR_DATA<<10) |
			  (MT47H128M16RT25E_PHY_WR_DATA<<0)),
};

static const struct cmd_control ddr2_cmd_ctrl_data = {
	.cmd0csratio = MT47H128M16RT25E_RATIO,
	.cmd0iclkout = MT47H128M16RT25E_INVERT_CLKOUT,

	.cmd1csratio = MT47H128M16RT25E_RATIO,
	.cmd1iclkout = MT47H128M16RT25E_INVERT_CLKOUT,

	.cmd2csratio = MT47H128M16RT25E_RATIO,
	.cmd2iclkout = MT47H128M16RT25E_INVERT_CLKOUT,
};

static const struct emif_regs ddr2_emif_reg_data = {
	.sdram_config = MT47H128M16RT25E_EMIF_SDCFG,
	.ref_ctrl = MT47H128M16RT25E_EMIF_SDREF,
	.sdram_tim1 = MT47H128M16RT25E_EMIF_TIM1,
	.sdram_tim2 = MT47H128M16RT25E_EMIF_TIM2,
	.sdram_tim3 = MT47H128M16RT25E_EMIF_TIM3,
	.emif_ddr_phy_ctlr_1 = MT47H128M16RT25E_EMIF_READ_LATENCY,
};

static const struct ddr_data ddr3_data = {
	.datardsratio0 = MT41J128MJT125_RD_DQS,
	.datawdsratio0 = MT41J128MJT125_WR_DQS,
	.datafwsratio0 = MT41J128MJT125_PHY_FIFO_WE,
	.datawrsratio0 = MT41J128MJT125_PHY_WR_DATA,
};

static const struct ddr_data ddr3_beagleblack_data = {
	.datardsratio0 = MT41K256M16HA125E_RD_DQS,
	.datawdsratio0 = MT41K256M16HA125E_WR_DQS,
	.datafwsratio0 = MT41K256M16HA125E_PHY_FIFO_WE,
	.datawrsratio0 = MT41K256M16HA125E_PHY_WR_DATA,
};

static const struct ddr_data ddr3_evm_data = {
	.datardsratio0 = MT41J512M8RH125_RD_DQS,
	.datawdsratio0 = MT41J512M8RH125_WR_DQS,
	.datafwsratio0 = MT41J512M8RH125_PHY_FIFO_WE,
	.datawrsratio0 = MT41J512M8RH125_PHY_WR_DATA,
};

static const struct cmd_control ddr3_cmd_ctrl_data = {
	.cmd0csratio = MT41J128MJT125_RATIO,
	.cmd0iclkout = MT41J128MJT125_INVERT_CLKOUT,

	.cmd1csratio = MT41J128MJT125_RATIO,
	.cmd1iclkout = MT41J128MJT125_INVERT_CLKOUT,

	.cmd2csratio = MT41J128MJT125_RATIO,
	.cmd2iclkout = MT41J128MJT125_INVERT_CLKOUT,
};

static const struct cmd_control ddr3_beagleblack_cmd_ctrl_data = {
	.cmd0csratio = MT41K256M16HA125E_RATIO,
	.cmd0iclkout = MT41K256M16HA125E_INVERT_CLKOUT,

	.cmd1csratio = MT41K256M16HA125E_RATIO,
	.cmd1iclkout = MT41K256M16HA125E_INVERT_CLKOUT,

	.cmd2csratio = MT41K256M16HA125E_RATIO,
	.cmd2iclkout = MT41K256M16HA125E_INVERT_CLKOUT,
};

static const struct cmd_control ddr3_evm_cmd_ctrl_data = {
	.cmd0csratio = MT41J512M8RH125_RATIO,
	.cmd0iclkout = MT41J512M8RH125_INVERT_CLKOUT,

	.cmd1csratio = MT41J512M8RH125_RATIO,
	.cmd1iclkout = MT41J512M8RH125_INVERT_CLKOUT,

	.cmd2csratio = MT41J512M8RH125_RATIO,
	.cmd2iclkout = MT41J512M8RH125_INVERT_CLKOUT,
};

static struct emif_regs ddr3_emif_reg_data = {
	.sdram_config = MT41J128MJT125_EMIF_SDCFG,
	.ref_ctrl = MT41J128MJT125_EMIF_SDREF,
	.sdram_tim1 = MT41J128MJT125_EMIF_TIM1,
	.sdram_tim2 = MT41J128MJT125_EMIF_TIM2,
	.sdram_tim3 = MT41J128MJT125_EMIF_TIM3,
	.zq_config = MT41J128MJT125_ZQ_CFG,
	.emif_ddr_phy_ctlr_1 = MT41J128MJT125_EMIF_READ_LATENCY |
				PHY_EN_DYN_PWRDN,
};

static struct emif_regs ddr3_beagleblack_emif_reg_data = {
	.sdram_config = MT41K256M16HA125E_EMIF_SDCFG,
	.ref_ctrl = MT41K256M16HA125E_EMIF_SDREF,
	.sdram_tim1 = MT41K256M16HA125E_EMIF_TIM1,
	.sdram_tim2 = MT41K256M16HA125E_EMIF_TIM2,
	.sdram_tim3 = MT41K256M16HA125E_EMIF_TIM3,
	.zq_config = MT41K256M16HA125E_ZQ_CFG,
	.emif_ddr_phy_ctlr_1 = MT41K256M16HA125E_EMIF_READ_LATENCY,
};

static struct emif_regs ddr3_evm_emif_reg_data = {
	.sdram_config = MT41J512M8RH125_EMIF_SDCFG,
	.ref_ctrl = MT41J512M8RH125_EMIF_SDREF,
	.sdram_tim1 = MT41J512M8RH125_EMIF_TIM1,
	.sdram_tim2 = MT41J512M8RH125_EMIF_TIM2,
	.sdram_tim3 = MT41J512M8RH125_EMIF_TIM3,
	.zq_config = MT41J512M8RH125_ZQ_CFG,
	.emif_ddr_phy_ctlr_1 = MT41J512M8RH125_EMIF_READ_LATENCY |
				PHY_EN_DYN_PWRDN,
};

#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
	/* break into full u-boot on 'c' */
	return (serial_tstc() && serial_getc() == 'c');
}
#endif

#define OSC	(V_OSCK/1000000)
const struct dpll_params dpll_ddr = {
		266, OSC-1, 1, -1, -1, -1, -1};
const struct dpll_params dpll_ddr_evm_sk = {
		303, OSC-1, 1, -1, -1, -1, -1};
const struct dpll_params dpll_ddr_bone_black = {
		400, OSC-1, 1, -1, -1, -1, -1};

void am33xx_spl_board_init(void)
{
	//struct am335x_baseboard_id header;
	int mpu_vdd;

	//enable_i2c0_pin_mux();
	//if (read_eeprom(&header) < 0)
	puts("BoneLogic booting....\n");

	/* Get the frequency */
	dpll_mpu_opp100.m = am335x_get_efuse_mpu_max_freq(cdev);

#if 1 /* BoneLogic... based on BeagleBone*/
	int usb_cur_lim;

	//TODO: original source returned here for BB rev A1, suggesting that PMIC shouldn't be init'd on this rev...
	//make sure that this doesn't apply to us.
	puts("CV TODO: fixup  spl_board_init!\n");
		return;
#if 0
	if (i2c_probe(TPS65217_CHIP_PM))
		return;

	//check for AC power...
	uchar pmic_status_reg;
	if (tps65217_reg_read(TPS65217_STATUS,&pmic_status_reg))
		return;
	if (!(pmic_status_reg & TPS65217_PWR_SRC_AC_BITMASK)) 
	{
		puts("No AC power, disabling frequency switch\n");
		return;
	}
	
	//FYI setting PLL for 1GHz (BBB)
	//dpll_mpu_opp100.m = MPUPLL_M_1000;

	usb_cur_lim = TPS65217_USB_INPUT_CUR_LIMIT_1300MA;
	mpu_vdd = TPS65217_DCDC_VOLT_SEL_1275MV;

	if (tps65217_reg_write(TPS65217_PROT_LEVEL_NONE,TPS65217_POWER_PATH,usb_cur_lim,TPS65217_USB_INPUT_CUR_LIMIT_MASK))
			puts("tps65217_reg_write failure\n");

	// DCDC3 (CORE) voltage to 1.125V
	if (tps65217_voltage_update(TPS65217_DEFDCDC3,TPS65217_DCDC_VOLT_SEL_1125MV)) {
			puts("tps65217_voltage_update failure\n");
			return;
	}

	/* Set CORE Frequencies to OPP100 */
	do_setup_dpll(&dpll_core_regs, &dpll_core_opp100);

	/* Set DCDC2 (MPU) voltage */
	if (tps65217_voltage_update(TPS65217_DEFDCDC2, mpu_vdd)) {
		puts("tps65217_voltage_update failure\n");
		return;
	}

	TODO: LDO settings
	//LDO3, LDO4 output voltage to 3.3V for Beaglebone.
	//LDO3 to 1.8V and LDO4 to 3.3V for Beaglebone Black.
	if (tps65217_reg_write(TPS65217_PROT_LEVEL_2,TPS65217_DEFLS1,TPS65217_LDO_VOLTAGE_OUT_1_8,TPS65217_LDO_MASK))
		--OR--
	if (tps65217_reg_write(TPS65217_PROT_LEVEL_2,TPS65217_DEFLS1,TPS65217_LDO_VOLTAGE_OUT_3_3,TPS65217_LDO_MASK))
		puts("tps65217_reg_write failure\n");

	if (tps65217_reg_write(TPS65217_PROT_LEVEL_2,TPS65217_DEFLS2,TPS65217_LDO_VOLTAGE_OUT_3_3,TPS65217_LDO_MASK))
			puts("tps65217_reg_write failure\n");
#endif

#else
//EVM-SK
	int sil_rev;
	/*
	 * The GP EVM, IDK and EVM SK use a TPS65910 PMIC.  For all
	 * MPU frequencies we support we use a CORE voltage of
	 * 1.1375V.  For MPU voltage we need to switch based on
	 * the frequency we are running at.
	 */
	if(i2c_probe(TPS65910_CTRL_I2C_ADDR))
		return;

	/*
	 * Depending on MPU clock and PG we will need a different
	 * VDD to drive at that speed.
	 */
	sil_rev = readl(&cdev->deviceid) >> 28;
	mpu_vdd = am335x_get_tps65910_mpu_vdd(sil_rev,dpll_mpu_opp100.m);

	/* Tell the TPS65910 to use i2c */
	tps65910_set_i2c_control();

	/* First update MPU voltage. */
	if (tps65910_voltage_update(MPU, mpu_vdd))
		return;

	/* Second, update the CORE voltage. */
	if (tps65910_voltage_update(CORE, TPS65910_OP_REG_SEL_1_1_3))
		return;

	/* Set CORE Frequencies to OPP100 */
	do_setup_dpll(&dpll_core_regs, &dpll_core_opp100);

#endif

	/* Set MPU Frequency to what we detected now that voltages are set */
	do_setup_dpll(&dpll_mpu_regs, &dpll_mpu_opp100);
}

const struct dpll_params *get_dpll_ddr_params(void)	
{
	//enable_i2c0_pin_mux();
	//i2c_init(CONFIG_SYS_OMAP24_I2C_SPEED, CONFIG_SYS_OMAP24_I2C_SLAVE);
	
	//return &dpll_ddr_evm_sk;
	//return &dpll_ddr_bone_black;
	return &dpll_ddr;
}

void set_mux_conf_regs(void)
{

	enable_board_pin_mux(); //mux.c
}

const struct ctrl_ioregs ioregs_evmsk = {
	.cm0ioctl		= MT41J128MJT125_IOCTRL_VALUE,
	.cm1ioctl		= MT41J128MJT125_IOCTRL_VALUE,
	.cm2ioctl		= MT41J128MJT125_IOCTRL_VALUE,
	.dt0ioctl		= MT41J128MJT125_IOCTRL_VALUE,
	.dt1ioctl		= MT41J128MJT125_IOCTRL_VALUE,
};

const struct ctrl_ioregs ioregs_bonelt = {
	.cm0ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
	.cm1ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
	.cm2ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
	.dt0ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
	.dt1ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
};

const struct ctrl_ioregs ioregs_evm15 = {
	.cm0ioctl		= MT41J512M8RH125_IOCTRL_VALUE,
	.cm1ioctl		= MT41J512M8RH125_IOCTRL_VALUE,
	.cm2ioctl		= MT41J512M8RH125_IOCTRL_VALUE,
	.dt0ioctl		= MT41J512M8RH125_IOCTRL_VALUE,
	.dt1ioctl		= MT41J512M8RH125_IOCTRL_VALUE,
};

const struct ctrl_ioregs ioregs = {
	.cm0ioctl		= MT47H128M16RT25E_IOCTRL_VALUE,
	.cm1ioctl		= MT47H128M16RT25E_IOCTRL_VALUE,
	.cm2ioctl		= MT47H128M16RT25E_IOCTRL_VALUE,
	.dt0ioctl		= MT47H128M16RT25E_IOCTRL_VALUE,
	.dt1ioctl		= MT47H128M16RT25E_IOCTRL_VALUE,
};

void sdram_init(void)
{
	/*
	 * EVM SK 1.2A and later use gpio0_7 to enable DDR3.
	 * This is safe enough to do on older revs.
	 */
	//fur das EVM-SK...
	//gpio_request(GPIO_DDR_VTT_EN, "ddr_vtt_en");
	//gpio_direction_output(GPIO_DDR_VTT_EN, 1);

	/* pick one & only one... :-) */
	//config_ddr(303, &ioregs_evmsk, &ddr3_data,&ddr3_cmd_ctrl_data, &ddr3_emif_reg_data, 0);
	//config_ddr(400, &ioregs_bonelt,&ddr3_beagleblack_data,&ddr3_beagleblack_cmd_ctrl_data,&ddr3_beagleblack_emif_reg_data, 0);
	config_ddr(266, &ioregs, &ddr2_data,&ddr2_cmd_ctrl_data, &ddr2_emif_reg_data, 0);
}
#endif /* line 82 */

/*
 * Basic board specific setup.  Pinmux has been handled already.
 */
int board_init(void)
{
#if defined(CONFIG_HW_WATCHDOG)
	hw_watchdog_init();
#endif

	board_video_init();

	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#if defined(CONFIG_NOR) || defined(CONFIG_NAND)
	gpmc_init();
#endif

	return 0;
}

int board_late_init(void)
{
	return 0;
}

#if (defined(CONFIG_DRIVER_TI_CPSW) && !defined(CONFIG_SPL_BUILD)) || \
	(defined(CONFIG_SPL_ETH_SUPPORT) && defined(CONFIG_SPL_BUILD))
static void cpsw_control(int enabled)
{
	/* VTP can be added here */

	return;
}

static struct cpsw_slave_data cpsw_slaves[] = {
	{
		.slave_reg_ofs	= 0x208,
		.sliver_reg_ofs	= 0xd80,
		.phy_addr	= 0,
	},
	{
		.slave_reg_ofs	= 0x308,
		.sliver_reg_ofs	= 0xdc0,
		.phy_addr	= 1,
	},
};

static struct cpsw_platform_data cpsw_data = {
	.mdio_base		= CPSW_MDIO_BASE,
	.cpsw_base		= CPSW_BASE,
	.mdio_div		= 0xff,
	.channels		= 8,
	.cpdma_reg_ofs		= 0x800,
	.slaves			= 1,
	.slave_data		= cpsw_slaves,
	.ale_reg_ofs		= 0xd00,
	.ale_entries		= 1024,
	.host_port_reg_ofs	= 0x108,
	.hw_stats_reg_ofs	= 0x900,
	.bd_ram_ofs		= 0x2000,
	.mac_control		= (1 << 5),
	.control		= cpsw_control,
	.host_port_num		= 0,
	.version		= CPSW_CTRL_VERSION_2,
};
#endif

/*
 * This function will:
 * Read the eFuse for MAC addresses, and set ethaddr/eth1addr/usbnet_devaddr
 * in the environment
 * Perform fixups to the PHY present on certain boards.  We only need this
 * function in:
 * - SPL with either CPSW or USB ethernet support
 * - Full U-Boot, with either CPSW or USB ethernet
 * Build in only these cases to avoid warnings about unused variables
 * when we build an SPL that has neither option but full U-Boot will.
 */
#if((defined(CONFIG_SPL_ETH_SUPPORT) || defined(CONFIG_SPL_USBETH_SUPPORT)) \
		&& defined(CONFIG_SPL_BUILD)) || \
	((defined(CONFIG_DRIVER_TI_CPSW) || \
	  defined(CONFIG_USB_ETHER) && defined(CONFIG_MUSB_GADGET)) && \
	 !defined(CONFIG_SPL_BUILD))

int board_eth_init(bd_t *bis)
{
	int rv, n = 0;
	uint8_t mac_addr[6];
	uint32_t mac_hi, mac_lo;

	/* try reading mac address from efuse */
	mac_lo = readl(&cdev->macid0l);
	mac_hi = readl(&cdev->macid0h);
	mac_addr[0] = mac_hi & 0xFF;
	mac_addr[1] = (mac_hi & 0xFF00) >> 8;
	mac_addr[2] = (mac_hi & 0xFF0000) >> 16;
	mac_addr[3] = (mac_hi & 0xFF000000) >> 24;
	mac_addr[4] = mac_lo & 0xFF;
	mac_addr[5] = (mac_lo & 0xFF00) >> 8;

#if(defined(CONFIG_DRIVER_TI_CPSW) && !defined(CONFIG_SPL_BUILD)) || \
	(defined(CONFIG_SPL_ETH_SUPPORT) && defined(CONFIG_SPL_BUILD))
	if (!getenv("ethaddr")) {
		printf("<ethaddr> not set. Validating first E-fuse MAC\n");

		if (is_valid_ether_addr(mac_addr))
			eth_setenv_enetaddr("ethaddr", mac_addr);
	}

#ifdef CONFIG_DRIVER_TI_CPSW
	mac_lo = readl(&cdev->macid1l);
	mac_hi = readl(&cdev->macid1h);
	mac_addr[0] = mac_hi & 0xFF;
	mac_addr[1] = (mac_hi & 0xFF00) >> 8;
	mac_addr[2] = (mac_hi & 0xFF0000) >> 16;
	mac_addr[3] = (mac_hi & 0xFF000000) >> 24;
	mac_addr[4] = mac_lo & 0xFF;
	mac_addr[5] = (mac_lo & 0xFF00) >> 8;

	if (!getenv("eth1addr")) {
		if (is_valid_ether_addr(mac_addr))
			eth_setenv_enetaddr("eth1addr", mac_addr);
	}

	//ETH MII mode...
	/* option 1
	writel(MII_MODE_ENABLE, &cdev->miisel);
	cpsw_slaves[0].phy_if = cpsw_slaves[1].phy_if =	PHY_INTERFACE_MODE_MII;
	*/
	/* option 2
	writel((RGMII_MODE_ENABLE | RGMII_INT_DELAY), &cdev->miisel);
	cpsw_slaves[0].phy_if = cpsw_slaves[1].phy_if =	PHY_INTERFACE_MODE_RGMII;
	*/

	rv = cpsw_register(&cpsw_data);
	if (rv < 0)
		printf("Error %d registering CPSW switch\n", rv);
	else
		n += rv;
#endif

	/*
	 *
	 * CPSW RGMII Internal Delay Mode is not supported in all PVT
	 * operating points.  So we must set the TX clock delay feature
	 * in the AR8051 PHY.  Since we only support a single ethernet
	 * device in U-Boot, we only do this for the first instance.
	 */
#define AR8051_PHY_DEBUG_ADDR_REG	0x1d
#define AR8051_PHY_DEBUG_DATA_REG	0x1e
#define AR8051_DEBUG_RGMII_CLK_DLY_REG	0x5
#define AR8051_RGMII_TX_CLK_DLY		0x100

/* eth setup...
	const char *devname;
	devname = miiphy_get_current_dev();
	miiphy_write(devname, 0x0, AR8051_PHY_DEBUG_ADDR_REG,AR8051_DEBUG_RGMII_CLK_DLY_REG);
	miiphy_write(devname, 0x0, AR8051_PHY_DEBUG_DATA_REG,AR8051_RGMII_TX_CLK_DLY);
*/
#endif

#if  defined(CONFIG_USB_ETHER) && \
	(!defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_USBETH_SUPPORT))
	if (is_valid_ether_addr(mac_addr))
		eth_setenv_enetaddr("usbnet_devaddr", mac_addr);

	rv = usb_eth_initialize(bis);
	if (rv < 0)
		printf("Error %d registering USB_ETHER\n", rv);
	else
		n += rv;
#endif
	return n;
}

#endif
////////////////////////////////////////////////////////
#if !defined(CONFIG_SPL_BUILD)
static struct da8xx_panel lcd_panels[] = {
	/* NHD_480272MF_ATXI */
	[0] = {
		.name = "NHD_480272MF_ATXI",
		.width = 480,
		.height = 272,
		.hfp = 6,
		.hbp = 30,
		.hsw = 40,
		.vfp = 3,
		.vbp = 12,
		.vsw = 5,
		.pxl_clk = 9200000,
		.invert_pxl_clk = 0,
	},
	/* NORSE TFT */
	[1] = {
		.name = "NORSE_TFT",
		.width = 400,
		.height = 240,
		.hfp = 52,
		.hbp = 84,
		.hsw = 36,
		.vfp = 3,
		.vbp = 14,
		.vsw = 6,
		.pxl_clk = 6000000,
		.invert_pxl_clk = 0,
	},
};

static const struct display_panel disp_panel = {
	WVGA,
	32,
	16,
	COLOR_ACTIVE,
};

static const struct lcd_ctrl_config lcd_cfg = {
	&disp_panel,
	.ac_bias		= 255,
	.ac_bias_intrpt		= 0,
	.dma_burst_sz		= 16,
	.bpp			= 32,
	.fdd			= 0x80,
	.tft_alt_mode		= 0,
	.stn_565_mode		= 0,
	.mono_8bit_mode		= 0,
	.invert_line_clock	= 1,
	.invert_frm_clock	= 1,
	.sync_edge		= 0,
	.sync_ctrl		= 1,
	.raster_order		= 0,
};

static int set_gpio(int gpio, int state)
{
	gpio_request(gpio, "temp");
	gpio_direction_output(gpio, state);
	gpio_set_value(gpio, state);
	gpio_free(gpio);
	return 0;
}

static int enable_backlight(void)
{
	set_gpio(113, 0);  //gpio 3-17... 32*3 + 17 = 113
	return 0;
}

static int enable_pwm(void)
{
	struct pwmss_regs *pwmss = (struct pwmss_regs *)PWMSS0_BASE;
	struct pwmss_ecap_regs *ecap;
	int ticks = PWM_TICKS;
	int duty = PWM_DUTY;

	ecap = (struct pwmss_ecap_regs *)AM33XX_ECAP0_BASE;
	/* enable clock */
	setbits_le32(&pwmss->clkconfig, ECAP_CLK_EN);
	/* TimeStam Counter register */
	writel(0xdb9, &ecap->tsctr);
	/* config period */
	writel(ticks - 1, &ecap->cap3);
	writel(ticks - 1, &ecap->cap1);
	setbits_le16(&ecap->ecctl2,(ECTRL2_MDSL_ECAP | ECTRL2_SYNCOSEL_MASK | 0xd0));
	/* config duty */
	writel(duty, &ecap->cap2);
	writel(duty, &ecap->cap4);
	/* start */
	setbits_le16(&ecap->ecctl2, ECTRL2_CTRSTP_FREERUN);
	return 0;
}

static struct dpll_regs dpll_lcd_regs = {
	.cm_clkmode_dpll = CM_WKUP + 0x98,
	.cm_idlest_dpll = CM_WKUP + 0x48,
	.cm_clksel_dpll = CM_WKUP + 0x54,
};

/* no console on this board */
int board_cfb_skip(void)
{
	return 1;
}

#define PLL_GET_M(v) ((v >> 8) & 0x7ff)
#define PLL_GET_N(v) (v & 0x7f)

static int get_clk(struct dpll_regs *dpll_regs)
{
	unsigned int val;
	unsigned int m, n;
	int f = 0;

	val = readl(dpll_regs->cm_clksel_dpll);
	m = PLL_GET_M(val);
	n = PLL_GET_N(val);
	f = (m * V_OSCK) / n;

	return f;
};

int clk_get(int clk)
{
	return get_clk(&dpll_lcd_regs);
};

static int conf_disp_pll(int m, int n)
{
	struct cm_perpll *cmper = (struct cm_perpll *)CM_PER;
	struct cm_dpll *cmdpll = (struct cm_dpll *)CM_DPLL;
	struct dpll_params dpll_lcd = {m, n, -1, -1, -1, -1, -1};

	u32 *const clk_domains[] = {
		&cmper->lcdclkctrl,
		0
	};
	u32 *const clk_modules_explicit_en[] = {
		&cmper->lcdclkctrl,
		&cmper->lcdcclkstctrl,
		&cmper->epwmss0clkctrl,
		0
	};
	do_enable_clocks(clk_domains, clk_modules_explicit_en, 1);
	writel(0x0, &cmdpll->clklcdcpixelclk);

	do_setup_dpll(&dpll_lcd_regs, &dpll_lcd);

	return 0;
}

static int board_video_init(void)
{
	//conf_disp_pll(24, 1);
	//da8xx_video_init(&lcd_panels[0], &lcd_cfg, lcd_cfg.bpp);

	//enable_pwm();
	//enable_backlight();

	return 0;
}
#endif
