/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/* @(#)dsp32c.h	1.2 9/26/95 ATT/ERL/ESI */
/*	Copyright (c) 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/

/*
 *	Header file for the dsp32C signal processor IC
 *	this is for the production WAVES board, with 8b
 *	PIO, 16b registers, and 32b DMA. jhs, Feb 1990
 *
 *	REMEMBER to read/write D<08:15> last on wide register accesses!!
 */

struct dsp32c	{		/* 16b "register map", but 8b physical port */
	unsigned short	dc_parl;
	unsigned short	dc_parh;
	unsigned short	dc_pdrl;
	unsigned short	dc_pdrh;
	unsigned short	dc_emrl;
	unsigned short	dc_emrh;
	unsigned short	dc_esr;
	unsigned short	dc_pcrl;
	unsigned short	dc_pirl;
	unsigned short	dc_pirh;
	unsigned short	dc_pcrh;
	unsigned short	dc_pare;
	unsigned short	dc_pdr2l;
	unsigned short	dc_pdr2h;
	unsigned short	dc_null[2];
}	typedef	DSP32C;

/*
 *	offsets for dsp registers, in shorts
 */
#define	DC_PIO_PARL	0
#define	DC_PIO_PARH	1
#define	DC_PIO_PDRL	2
#define	DC_PIO_PDRH	3
#define	DC_PIO_EMRL	4
#define	DC_PIO_EMRH	5
#define	DC_PIO_ESR	6
#define	DC_PIO_PCRL	7
#define	DC_PIO_PIRL	8
#define	DC_PIO_PIRH	9
#define	DC_PIO_PCRH	10
#define	DC_PIO_PARE	11
#define	DC_PIO_PDR2L	12
#define	DC_PIO_PDR2H	13

/*	pcrl bits	*/

#define	DC_PCRL_RUN		1
#define	DC_PCRL_I16		2
#define	DC_PCRL_ENI		4
#define	DC_PCRL_DMA		8
#define	DC_PCRL_AUTO		0x10
#define	DC_PCRL_PDF		0x20
#define	DC_PCRL_PIF		0x40

/*	pcrh bits	*/

/*
 *  General pcrh folklore: FLG should >always< be set; PIO16 should >never<
 *  be set (on this board) because we use the upper 8 bits for our control regs
 */

#define	DC_PCRH_DMA32		1
#define	DC_PCRH_PIO16		2
#define	DC_PCRH_FLG		4

/*
 *	pcr standard modes
 */
#define	DC_PCRL_MODE		DC_PCRL_I16
#define	DC_PCRL_DMODE		(DC_PCRL_DMA | DC_PCR_MODE)
#define	DC_PCRL_AMODE		(DC_PCRL_AUTO | DC_PCR_MODE)
#define	DC_PCRL_DMAMODE		(DC_PCRL_DMA | DC_PCRL_AUTO | DC_PCR_MODE)
#define	DC_PCRH_MODE		(DC_PCRH_DMA32 | DC_PCRH_FLG)

/*
 *	error source register bits
 */
#define	DC_ESR_NAN	2
#define	DC_ESR_PIF	4
#define	DC_ESR_DAU	0x10
#define	DC_ESR_ADR	0x20
#define	DC_ESR_SAN	0x40
#define	DC_ESR_SYNC	0x80

/*
 *	error mask register bits:
 *	EMRI masks PIF, EMRH masks HALT
 *	set to 1 to mask, clear to enable
 */
#define	DC_EMRI_NAN	2
#define	DC_EMRI_DAU	0x10
#define	DC_EMRI_ADR	0x20
#define	DC_EMRI_SAN	0x40
#define	DC_EMRI_SYNC	0x80
#define	DC_EMRH_NAN	0x0200
#define	DC_EMRH_DAU	0x1000
#define	DC_EMRH_ADR	0x2000
#define	DC_EMRH_SAN	0x4000
#define	DC_EMRH_SYNC	0x8000
