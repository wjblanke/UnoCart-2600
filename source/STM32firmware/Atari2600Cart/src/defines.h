/**
 *  Defines for your entire project at one place
 *
 *	@author 	Tilen Majerle
 *	@email		tilen@majerle.eu
 *	@website	http://stm32f4-discovery.com
 *	@version 	v1.0
 *	@ide		Keil uVision 5
 *	@license	GNU GPL v3
 *
 * |----------------------------------------------------------------------
 * | Copyright (C) Tilen Majerle, 2014
 * |
 * | This program is free software: you can redistribute it and/or modify
 * | it under the terms of the GNU General Public License as published by
 * | the Free Software Foundation, either version 3 of the License, or
 * | any later version.
 * |
 * | This program is distributed in the hope that it will be useful,
 * | but WITHOUT ANY WARRANTY; without even the implied warranty of
 * | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * | GNU General Public License for more details.
 * |
 * | You should have received a copy of the GNU General Public License
 * | along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * |----------------------------------------------------------------------
 */
#ifndef TM_DEFINES_H
#define TM_DEFINES_H

/* Put your global defines for all libraries here used in your project */

/* MCUDEV DevEBox F407VGT6 microSD: SDIO 4-bit
 *   D0=PC8, D1=PC9, D2=PC10, D3=PC11, CK=PC12, CMD=PD2
 *   Card-detect pin is NC on this board — do not use PB6 CD.
 */
#define FATFS_USE_SDIO			1
#define FATFS_SDIO_4BIT			1
#define FATFS_USE_DETECT_PIN		0
#define FATFS_USE_WRITEPROTECT_PIN	0
/* SDIO_CK = 48MHz / (DIV+2). 0x01=16MHz was flaky; 0x04≈8MHz is more reliable. */
#define SDIO_TRANSFER_CLK_DIV		((uint8_t)0x04)

#endif
