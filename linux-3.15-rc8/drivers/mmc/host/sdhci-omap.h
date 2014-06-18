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


#ifndef _DRIVERS_MMC_SDHCI_OMAP_H
#define _DRIVERS_MMC_SDHCI_OMAP_H

/*
 * Ops and quirks for the TI OMAP HSMMC controller.
 */

#define OMAP_DEFAULT_QUIRKS	(SDHCI_QUIRK_NO_BUSY_IRQ | \
				 SDHCI_QUIRK_RESTORE_IRQS_AFTER_RESET)

#define OMAP_CTRL_4BITBUS          (0x1 << 1)
#define OMAP_CTRL_8BITBUS          (0x2 << 1)
#define OMAP_CTRL_BUSWIDTH_MASK    (0x3 << 1)

#define OMAP_SYSTEM_CONTROL    0x2c
#define OMAP_CLOCK_MASK        0x0000fff0
#define OMAP_PREDIV_SHIFT      8
#define OMAP_DIVIDER_SHIFT     4
#define OMAP_CLOCK_PEREN       0x00000004
#define OMAP_CLOCK_HCKEN       0x00000002
#define OMAP_CLOCK_IPGEN       0x00000001


#endif /* _DRIVERS_MMC_SDHCI_OMAP_H */
