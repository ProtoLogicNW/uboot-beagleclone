/*
 * mux.c
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/hardware.h>
#include <asm/arch/mux.h>
#include <asm/io.h>
#include <i2c.h>
#include "board.h"

static struct module_pin_mux uart0_pin_mux[] = {
	{OFFSET(uart0_rxd), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* UART0_RXD */
	{OFFSET(uart0_txd), (MODE(0) | PULLUDEN)},		/* UART0_TXD */
	{-1},
};

static struct module_pin_mux uart1_pin_mux[] = {
	{OFFSET(uart1_rxd), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* UART1_RXD */
	{OFFSET(uart1_txd), (MODE(0) | PULLUDEN)},		/* UART1_TXD */
	{-1},
};

static struct module_pin_mux uart2_pin_mux[] = {
	{OFFSET(spi0_sclk), (MODE(1) | PULLUP_EN | RXACTIVE)},	/* UART2_RXD */
	{OFFSET(spi0_d0), (MODE(1) | PULLUDEN)},		/* UART2_TXD */
	{-1},
};

static struct module_pin_mux uart3_pin_mux[] = {
	{OFFSET(spi0_cs1), (MODE(1) | PULLUP_EN | RXACTIVE)},	/* UART3_RXD */
	{OFFSET(ecap0_in_pwm0_out), (MODE(1) | PULLUDEN)},	/* UART3_TXD */
	{-1},
};

static struct module_pin_mux uart4_pin_mux[] = {
	{OFFSET(gpmc_wait0), (MODE(6) | PULLUP_EN | RXACTIVE)},	/* UART4_RXD */
	{OFFSET(gpmc_wpn), (MODE(6) | PULLUDEN)},		/* UART4_TXD */
	{-1},
};

static struct module_pin_mux uart5_pin_mux[] = {
	{OFFSET(lcd_data9), (MODE(4) | PULLUP_EN | RXACTIVE)},	/* UART5_RXD */
	{OFFSET(lcd_data8), (MODE(4) | PULLUDEN)},		/* UART5_TXD */
	{-1},
};

static struct module_pin_mux mmc0_pin_mux[] = {
	{OFFSET(mmc0_dat3), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT3 */
	{OFFSET(mmc0_dat2), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT2 */
	{OFFSET(mmc0_dat1), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT1 */
	{OFFSET(mmc0_dat0), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT0 */
	{OFFSET(mmc0_clk), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_CLK */
	{OFFSET(mmc0_cmd), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_CMD */
	{OFFSET(mcasp0_aclkr), (MODE(4) | RXACTIVE)},		/* MMC0_WP */
	{OFFSET(spi0_cs1), (MODE(5) | RXACTIVE | PULLUP_EN)},	/* MMC0_CD */
	{-1},
};

static struct module_pin_mux mmc0_no_cd_pin_mux[] = {
	{OFFSET(mmc0_dat3), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT3 */
	{OFFSET(mmc0_dat2), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT2 */
	{OFFSET(mmc0_dat1), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT1 */
	{OFFSET(mmc0_dat0), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT0 */
	{OFFSET(mmc0_clk), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_CLK */
	{OFFSET(mmc0_cmd), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_CMD */
	{OFFSET(mcasp0_aclkr), (MODE(4) | RXACTIVE)},		/* MMC0_WP */
	{-1},
};

static struct module_pin_mux mmc0_pin_mux_sk_evm[] = {
	{OFFSET(mmc0_dat3), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT3 */
	{OFFSET(mmc0_dat2), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT2 */
	{OFFSET(mmc0_dat1), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT1 */
	{OFFSET(mmc0_dat0), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT0 */
	{OFFSET(mmc0_clk), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_CLK */
	{OFFSET(mmc0_cmd), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_CMD */
//	{OFFSET(spi0_cs1), (MODE(5) | RXACTIVE | PULLUP_EN)},	/* MMC0_CD */
	{-1},
};

static struct module_pin_mux mmc1_pin_mux[] = {
	{OFFSET(gpmc_ad3), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT3 */
	{OFFSET(gpmc_ad2), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT2 */
	{OFFSET(gpmc_ad1), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT1 */
	{OFFSET(gpmc_ad0), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT0 */
	{OFFSET(gpmc_csn1), (MODE(2) | RXACTIVE | PULLUP_EN)},	/* MMC1_CLK */
	{OFFSET(gpmc_csn2), (MODE(2) | RXACTIVE | PULLUP_EN)},	/* MMC1_CMD */
	{OFFSET(gpmc_csn0), (MODE(7) | RXACTIVE | PULLUP_EN)},	/* MMC1_WP */
	{OFFSET(gpmc_advn_ale), (MODE(7) | RXACTIVE | PULLUP_EN)},	/* MMC1_CD */
	{-1},
};

static struct module_pin_mux i2c0_pin_mux[] = {
	{OFFSET(i2c0_sda), (MODE(0) | RXACTIVE |
			PULLUDEN | SLEWCTRL)}, /* I2C_DATA */
	{OFFSET(i2c0_scl), (MODE(0) | RXACTIVE |
			PULLUDEN | SLEWCTRL)}, /* I2C_SCLK */
	{-1},
};

static struct module_pin_mux i2c1_pin_mux[] = {
	{OFFSET(spi0_d1), (MODE(2) | RXACTIVE |
			PULLUDEN | SLEWCTRL)},	/* I2C_DATA */
	{OFFSET(spi0_cs0), (MODE(2) | RXACTIVE |
			PULLUDEN | SLEWCTRL)},	/* I2C_SCLK */
	{-1},
};

static struct module_pin_mux spi0_pin_mux[] = {
	{OFFSET(spi0_sclk), (MODE(0) | RXACTIVE | PULLUDEN)},	/* SPI0_SCLK */
	{OFFSET(spi0_d0), (MODE(0) | RXACTIVE |	PULLUDEN | PULLUP_EN)},			/* SPI0_D0 */
	{OFFSET(spi0_d1), (MODE(0) | RXACTIVE | PULLUDEN)},	/* SPI0_D1 */
	{OFFSET(spi0_cs0), (MODE(0) | RXACTIVE |PULLUDEN | PULLUP_EN)},			/* SPI0_CS0 */
//	{OFFSET(spi0_cs1), (MODE(0) | RXACTIVE |PULLUDEN | PULLUP_EN)},			/* SPI0_CS1 */
	{-1},
};

static struct module_pin_mux gpio0_7_pin_mux[] = {
	{OFFSET(ecap0_in_pwm0_out), (MODE(7) | PULLUDEN)},	/* GPIO0_7 */
	{-1},
};

static struct module_pin_mux rgmii1_pin_mux[] = {
	{OFFSET(mii1_txen), MODE(2)},			/* RGMII1_TCTL */
	{OFFSET(mii1_rxdv), MODE(2) | RXACTIVE},	/* RGMII1_RCTL */
	{OFFSET(mii1_txd3), MODE(2)},			/* RGMII1_TD3 */
	{OFFSET(mii1_txd2), MODE(2)},			/* RGMII1_TD2 */
	{OFFSET(mii1_txd1), MODE(2)},			/* RGMII1_TD1 */
	{OFFSET(mii1_txd0), MODE(2)},			/* RGMII1_TD0 */
	{OFFSET(mii1_txclk), MODE(2)},			/* RGMII1_TCLK */
	{OFFSET(mii1_rxclk), MODE(2) | RXACTIVE},	/* RGMII1_RCLK */
	{OFFSET(mii1_rxd3), MODE(2) | RXACTIVE},	/* RGMII1_RD3 */
	{OFFSET(mii1_rxd2), MODE(2) | RXACTIVE},	/* RGMII1_RD2 */
	{OFFSET(mii1_rxd1), MODE(2) | RXACTIVE},	/* RGMII1_RD1 */
	{OFFSET(mii1_rxd0), MODE(2) | RXACTIVE},	/* RGMII1_RD0 */
	{OFFSET(mdio_data), MODE(0) | RXACTIVE | PULLUP_EN},/* MDIO_DATA */
	{OFFSET(mdio_clk), MODE(0) | PULLUP_EN},	/* MDIO_CLK */
	{-1},
};

static struct module_pin_mux mii1_pin_mux[] = {
	{OFFSET(mii1_rxerr), MODE(0) | RXACTIVE},	/* MII1_RXERR */
	{OFFSET(mii1_txen), MODE(0)},			/* MII1_TXEN */
	{OFFSET(mii1_rxdv), MODE(0) | RXACTIVE},	/* MII1_RXDV */
	{OFFSET(mii1_txd3), MODE(0)},			/* MII1_TXD3 */
	{OFFSET(mii1_txd2), MODE(0)},			/* MII1_TXD2 */
	{OFFSET(mii1_txd1), MODE(0)},			/* MII1_TXD1 */
	{OFFSET(mii1_txd0), MODE(0)},			/* MII1_TXD0 */
	{OFFSET(mii1_txclk), MODE(0) | RXACTIVE},	/* MII1_TXCLK */
	{OFFSET(mii1_rxclk), MODE(0) | RXACTIVE},	/* MII1_RXCLK */
	{OFFSET(mii1_rxd3), MODE(0) | RXACTIVE},	/* MII1_RXD3 */
	{OFFSET(mii1_rxd2), MODE(0) | RXACTIVE},	/* MII1_RXD2 */
	{OFFSET(mii1_rxd1), MODE(0) | RXACTIVE},	/* MII1_RXD1 */
	{OFFSET(mii1_rxd0), MODE(0) | RXACTIVE},	/* MII1_RXD0 */
	{OFFSET(mdio_data), MODE(0) | RXACTIVE | PULLUP_EN}, /* MDIO_DATA */
	{OFFSET(mdio_clk), MODE(0) | PULLUP_EN},	/* MDIO_CLK */
	{-1},
};

static struct module_pin_mux lcdc_pin_mux[] = {
	{OFFSET(lcd_data0), (MODE(0) | PULLUDDIS)},	/* LCD_DAT0 */
	{OFFSET(lcd_data1), (MODE(0) | PULLUDDIS)},	/* LCD_DAT1 */
	{OFFSET(lcd_data2), (MODE(0) | PULLUDDIS)},	/* LCD_DAT2 */
	{OFFSET(lcd_data3), (MODE(0) | PULLUDDIS)},	/* LCD_DAT3 */
	{OFFSET(lcd_data4), (MODE(0) | PULLUDDIS)},	/* LCD_DAT4 */
	{OFFSET(lcd_data5), (MODE(0) | PULLUDDIS)},	/* LCD_DAT5 */
	{OFFSET(lcd_data6), (MODE(0) | PULLUDDIS)},	/* LCD_DAT6 */
	{OFFSET(lcd_data7), (MODE(0) | PULLUDDIS)},	/* LCD_DAT7 */
	{OFFSET(lcd_data8), (MODE(0) | PULLUDDIS)},	/* LCD_DAT8 */
	{OFFSET(lcd_data9), (MODE(0) | PULLUDDIS)},	/* LCD_DAT9 */
	{OFFSET(lcd_data10), (MODE(0) | PULLUDDIS)},	/* LCD_DAT10 */
	{OFFSET(lcd_data11), (MODE(0) | PULLUDDIS)},	/* LCD_DAT11 */
	{OFFSET(lcd_data12), (MODE(0) | PULLUDDIS)},	/* LCD_DAT12 */
	{OFFSET(lcd_data13), (MODE(0) | PULLUDDIS)},	/* LCD_DAT13 */
	{OFFSET(lcd_data14), (MODE(0) | PULLUDDIS)},	/* LCD_DAT14 */
	{OFFSET(lcd_data15), (MODE(0) | PULLUDDIS)},	/* LCD_DAT15 */
	{OFFSET(gpmc_ad8), (MODE(1))},			/* LCD_DAT16 */
	{OFFSET(gpmc_ad9), (MODE(1))},		/* LCD_DAT17 */
	{OFFSET(gpmc_ad10), (MODE(1))},		/* LCD_DAT18 */
	{OFFSET(gpmc_ad11), (MODE(1))},		/* LCD_DAT19 */
	{OFFSET(gpmc_ad12), (MODE(1))},		/* LCD_DAT20 */
	{OFFSET(gpmc_ad13), (MODE(1))},		/* LCD_DAT21 */
	{OFFSET(gpmc_ad14), (MODE(1))},		/* LCD_DAT22 */
	{OFFSET(gpmc_ad15), (MODE(1))},		/* LCD_DAT23 */
	{OFFSET(lcd_vsync), (MODE(0))},		/* LCD_VSYNC */
	{OFFSET(lcd_hsync), (MODE(0))},		/* LCD_HSYNC */
	{OFFSET(lcd_pclk), (MODE(0))},		/* LCD_PCLK */
	{OFFSET(lcd_ac_bias_en), (MODE(0))},	/* LCD_AC_BIAS_EN */
	{OFFSET(mcasp0_ahclkr), (MODE(7))},	/* backlight enable */
	{-1},
};

void enable_uart0_pin_mux(void)
{
	configure_module_pin_mux(uart0_pin_mux);
}

void enable_uart1_pin_mux(void)
{
	configure_module_pin_mux(uart1_pin_mux);
}

void enable_uart2_pin_mux(void)
{
	configure_module_pin_mux(uart2_pin_mux);
}

void enable_uart3_pin_mux(void)
{
	configure_module_pin_mux(uart3_pin_mux);
}

void enable_uart4_pin_mux(void)
{
	configure_module_pin_mux(uart4_pin_mux);
}

void enable_uart5_pin_mux(void)
{
	configure_module_pin_mux(uart5_pin_mux);
}

void enable_i2c0_pin_mux(void)
{
	configure_module_pin_mux(i2c0_pin_mux);
}


void enable_board_pin_mux()
{
	puts("Mux'in for BoneLogicR1, bitches!\n");
	configure_module_pin_mux(lcdc_pin_mux);
//	configure_module_pin_mux(i2c1_pin_mux);
//	configure_module_pin_mux(gpio0_7_pin_mux);
//	configure_module_pin_mux(rgmii1_pin_mux);
//	configure_module_pin_mux(mmc0_pin_mux_sk_evm);
	configure_module_pin_mux(spi0_pin_mux);
	configure_module_pin_mux(uart0_pin_mux);
	//hang();
}
