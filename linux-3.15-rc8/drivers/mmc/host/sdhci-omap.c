/*
 * TI OMAP HSMMC controller driver.
 *
 * Author: Saket Sinha <saket.sinha89@gmail.com> *	    
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 */

#include <linux/err.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/mmc/host.h>
#include "sdhci-pltfm.h"
#include "sdhci-omap.h"

static u32 omap_readl(struct sdhci_host *host, int reg)
{
	u32 ret;
	ret = in_be32(host->ioaddr + reg);
	return ret;
}

static void omap_writel(struct sdhci_host *host, u32 val, int reg)
{

	sdhci_be32bs_writel(host, val, reg);
}

static void omap_writew(struct sdhci_host *host, u16 val, int reg)
{
	sdhci_be32bs_writew(host, val, reg);
}

static void omap_writeb(struct sdhci_host *host, u8 val, int reg)
{
	sdhci_be32bs_writeb(host, val, reg);
}

static void omap_of_set_clock(struct sdhci_host *host, unsigned int clock)
{
 
         int pre_div = 2;
         int div = 1;
         u32 temp;

         temp = sdhci_readl(host, OMAP_SYSTEM_CONTROL);
         temp &= ~(OMAP_CLOCK_IPGEN | OMAP_CLOCK_HCKEN | OMAP_CLOCK_PEREN
                 | OMAP_CLOCK_MASK);
         sdhci_writel(host, temp, OMAP_SYSTEM_CONTROL);
 

         while (host->max_clk / pre_div / 16 > clock && pre_div < 256)
                 pre_div *= 2;
 
         while (host->max_clk / pre_div / div > clock && div < 16)
                 div++;
 
         pre_div >>= 1;
         div--;

	 temp = sdhci_readl(host, OMAP_SYSTEM_CONTROL);
         temp |= (OMAP_CLOCK_IPGEN | OMAP_CLOCK_HCKEN | OMAP_CLOCK_PEREN
                 | (div << OMAP_DIVIDER_SHIFT)
                 | (pre_div << OMAP_PREDIV_SHIFT));
         sdhci_writel(host, temp, OMAP_SYSTEM_CONTROL);

}

static unsigned int omap_of_get_max_clock(struct sdhci_host *host)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	return pltfm_host->clock;
}

static unsigned int omap_of_get_min_clock(struct sdhci_host *host)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	return pltfm_host->clock / 256 / 16;
}

static u32 omap_proctl;
static void omap_of_suspend(struct sdhci_host *host)
{
	omap_proctl = sdhci_be32bs_readl(host, SDHCI_HOST_CONTROL);
}

static void omap_of_resume(struct sdhci_host *host)
{
	sdhci_be32bs_writel(host, omap_proctl, SDHCI_HOST_CONTROL);
}

static void omap_of_platform_init(struct sdhci_host *host)
{
		host->quirks |= OMAP_DEFAULT_QUIRKS;
}

static int omap_pltfm_bus_width(struct sdhci_host *host, int width)
{
	u32 ctrl;

	switch (width) {
	case MMC_BUS_WIDTH_8:
		ctrl = OMAP_CTRL_8BITBUS;
		break;

	case MMC_BUS_WIDTH_4:
		ctrl = OMAP_CTRL_4BITBUS;
		break;

	default:
		ctrl = 0;
		break;
	}

	clrsetbits_be32(host->ioaddr + SDHCI_HOST_CONTROL,
			OMAP_CTRL_BUSWIDTH_MASK, ctrl);

	return 0;
}

static const struct sdhci_ops sdhci_omap_ops = {
	.read_l = omap_readl,
	.write_l = omap_writel,
	.write_w = omap_writew,
	.write_b = omap_writeb,
	.set_clock = omap_of_set_clock,
	.get_max_clock = omap_of_get_max_clock,
	.get_min_clock = omap_of_get_min_clock,
	.platform_init = omap_of_platform_init,
	.platform_suspend = omap_of_suspend,
	.platform_resume = omap_of_resume,
	.platform_bus_width = omap_pltfm_bus_width,
};

static const struct sdhci_pltfm_data sdhci_omap_pdata = {
	.quirks = OMAP_DEFAULT_QUIRKS,
	.ops = &sdhci_omap_ops,
};

static int sdhci_omap_probe(struct platform_device *pdev)
{
	struct sdhci_pltfm_host *pltfm_host;
	struct sdhci_host *host;
	struct device_node *np;
	struct clk *clk;
	int ret;


	host = sdhci_pltfm_init(pdev, &sdhci_omap_pdata, 0);
	if (IS_ERR(host))
		return PTR_ERR(host);

	sdhci_get_of_property(pdev);

	np = pdev->dev.of_node;
	
	/* call to generic mmc_of_parse to support additional capabilities */
	mmc_of_parse(host->mmc);
	mmc_of_parse_voltage(np, &host->ocr_mask);

        clk = clk_get(mmc_dev(host->mmc), NULL);
        clk_prepare_enable(clk);
        pltfm_host->clk = clk;

	ret = sdhci_add_host(host);
	if (ret)
		sdhci_pltfm_free(pdev);

	return ret;
}

static int sdhci_omap_remove(struct platform_device *pdev)
{
	return sdhci_pltfm_unregister(pdev);
}

static u16 omap4_reg_offset = 0x100;
static const struct of_device_id sdhci_omap_of_match[] = {
	{ .compatible = "ti,omap2-hsmmc" },
	{ .compatible = "ti,omap3-hsmmc" },
	{ .compatible = "ti,omap4-hsmmc",
	  .data = &omap4_reg_offset,
	},
	{ }
};
MODULE_DEVICE_TABLE(of, sdhci_omap_of_match);

#define omap_hsmmc_prepare      NULL
#define omap_hsmmc_complete     NULL
#define omap_hsmmc_suspend      NULL
#define omap_hsmmc_resume       NULL

static struct dev_pm_ops omap_hsmmc_dev_pm_ops = {
         .suspend        = omap_hsmmc_suspend,
         .resume         = omap_hsmmc_resume,
         .prepare        = omap_hsmmc_prepare,
         .complete       = omap_hsmmc_complete,
};

static struct platform_driver omap_sdhci_driver = {
	.driver = {
		.name = "sdhci-omap",
		.owner = THIS_MODULE,
		.pm = &omap_hsmmc_dev_pm_ops,
		.of_match_table = sdhci_omap_of_match,		
	},
	.probe = sdhci_omap_probe,
	.remove = sdhci_omap_remove,
};

module_platform_driver(omap_sdhci_driver);

MODULE_DESCRIPTION("SDHCI OF driver for TI OMAP HSMMC");
MODULE_AUTHOR("Saket Sinha <saket.sinha89@gmail.com>");
MODULE_LICENSE("GPL v3");
