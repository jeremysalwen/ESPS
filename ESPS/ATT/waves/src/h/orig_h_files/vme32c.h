/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/* @(#)vme32c.h	1.2 9/26/95 ATT/ERL/ESI */
/*	Copyright (c) 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/

#include	<sys/file.h>
#include	<sys/ioctl.h>
#include	<sys/types.h>
#include	"eeprom.h"

/*
 *	SURFBOARD 32c/VME board
 */
struct {
	unsigned short	dc_ctl;		/* bits 0-7 used */
	unsigned short	dc_sta;		/* bits 0-7 used */
	unsigned short	dc_ivec;	/* WECo 300A, vector on IACK cycles */
	unsigned short	dc_null1;	/* hole */
	unsigned short	dc_iic1;	/* Sig/Philips 8584IIC bus controller */
	unsigned short	dc_iic2;
	unsigned short	dc_null2[26];	/* hole */
	DSP32C		dc_dsp[6];	/* six dsp32Cs */
#define	d0		dc_dsp[0]
#define	d1		dc_dsp[1]
#define	d2		dc_dsp[2]
#define	d3		dc_dsp[3]
#define	d4		dc_dsp[4]
#define	d5		dc_dsp[5]
	unsigned short	gwenio[128];	/* daughter card */
}	typedef DC_IO;

/*
 *	devices types:
 */
#define	DC_D0	0
#define	DC_D1	1
#define	DC_D2	2
#define	DC_D3	3
#define	DC_D4	4
#define	DC_D5	5
#define	DC_MEM	6
#define	DC_IOD	7
#define	DC_32C	0x3C
#define	DC_5E	0x5E

/*
 *	32c board I/O register file offsets (in bytes)
 */
#define	DC_CTL_OFFS		0
#define	DC_STA_OFFS		2
#define	DC_VEC_OFFS		4
#define	DC_IIC1_OFFS		8
#define	DC_IIC2_OFFS		10
#define	DC_DSP0_OFFS		0x40
#define	DC_DSP1_OFFS		0x60
#define	DC_DSP2_OFFS		0x80
#define	DC_DSP3_OFFS		0xa0
#define	DC_DSP4_OFFS		0xc0
#define	DC_DSP5_OFFS		0xe0
#define	DC_GWEN_OFFS		0x100

#define	DC_A24BASE	0x900000
#define	DC_A32BASE	0x90000000

#define	DC_CMDMASK	0x00ff
#define	DC_REGMASK	0x00ff

/*
 *	CTL bits
 */
#define	DC_CTL_SFDIS	1
#define	DC_CTL_LOZ	2
#define	DC_CTL_D0RUN	4
#define	DC_CTL_D3RUN	8
#define	DC_CTL_D1RUN	0x10
#define	DC_CTL_D4RUN	0x20
#define	DC_CTL_D2RUN	0x40
#define	DC_CTL_D5RUN	0x80
#define	DC_CTL_MBX00	0x100
#define	DC_CTL_MBX01	0x200
#define	DC_CTL_MBX30	0x400
#define	DC_CTL_MBX31	0x800
#define	DC_CTL_VPOKE0	0x1000
#define	DC_CTL_VPOKE3	0x2000

/*
 *	STA bits
 */
#define	DC_STA_MBX00	0x01
#define	DC_STA_MBX01	0x02
#define	DC_STA_MBX30	0x04
#define	DC_STA_MBX31	0x08
#define	DC_STA_NOGWEN	0x20
#define	DC_GWEN_IRQ	0x40
#define	DC_IIC_IRQ	0x80
#define	DC_D0_IRQ	0x0100
#define	DC_D1_IRQ	0x0200
#define	DC_D2_IRQ	0x0400
#define	DC_D3_IRQ	0x0800
#define	DC_D4_IRQ	0x1000
#define	DC_D5_IRQ	0x2000

#define	DC_IRQ_FLAGS	(DC_GWEN_IRQ | DC_IIC_IRQ | DC_D0_IRQ | DC_D1_IRQ | \
			DC_D2_IRQ | DC_D3_IRQ | DC_D4_IRQ | DC_D5_IRQ)

/*
 *	in Fab1.3 and later, the upper 8b of the interrupt vector are used
 *	to hold the lower two bits of dsp memory mode, and the interrupt
 *	sources for the 4 dsp32c-5e parts.
 *
 *	Fab1.2 hardwired the memory mode to 6.
 */
#define	DC_IVEC_POKE1	0x0100
#define	DC_IVEC_POKE4	0x0200
#define	DC_IVEC_POKE2	0x0400
#define	DC_IVEC_POKE5	0x0800
#define	DC_IVEC_AMMODE4	0
#define	DC_IVEC_AMMODE5	0x1000
#define	DC_IVEC_AMMODE6	0x2000
#define	DC_IVEC_AMMODE7	0x3000
#define	DC_IVEC_BMMODE4	0
#define	DC_IVEC_BMMODE5	0x4000
#define	DC_IVEC_BMMODE6	0x8000
#define	DC_IVEC_BMMODE7	0xc000
#define	DC_IVEC_DEFAULT	(DC_IVEC_POKE1 | DC_IVEC_POKE2 | DC_IVEC_POKE4 | \
			DC_IVEC_POKE5 | DC_IVEC_AMMODE6 | DC_IVEC_BMMODE6)

/*
 *	the pif irq flag in dc_priv[]
 */
#define	DC_IRQ_PIF	1

/*
 *	struct for PCD8584 IIC bus controller. we need it because S1
 *	has separate registers for read and write.
 */
struct {
    unsigned char iic_s1rd;	/* NOIRQ,0,   STS,BER,LRB,AAS,  LAB, BUS BUSY */
    unsigned char iic_s1wr;	/* NOIRQ,ESer,ES1,ES2,ENI,START,STOP,ACK */
}   typedef	IIC_CTL;
/*
 *	IIC S1 read status
 */
#define	IIC_STA_PIN	0x80
#define	IIC_STA_SLVSTOP	0x20
#define	IIC_STA_BUSERR	0x10
#define	IIC_STA_NAK	0x08
#define	IIC_STA_ADRASLV	0x04
#define	IIC_STA_LOSTARB	0x02
#define	IIC_STA_BUSFREE	0x01
/*
 *	IIC S1 write controls
 */
#define	IIC_STA_NOIRQ	0x80
#define	IIC_STA_SENB	0x40
#define	IIC_STA_SELECT1	0x20
#define	IIC_STA_SELECT2	0x10
#define	IIC_STA_IRQENB	0x08
#define	IIC_STA_START	0x04
#define	IIC_STA_STOP	0x02
#define	IIC_STA_ACK	0x01
/*
 *	IIC S1 write compound controls ...
 *	use in place of IIC_SERIAL_ON, IIC_SELECT[12]
 */
#define	IIC_SEL_MY_ADR	0x0
#define	IIC_SEL_MY_VEC	0x10
#define	IIC_SEL_MY_CLK	0x20
#define	IIC_SEL_DATA	0x40
#define	IIC_LONG_LINES	0x60

#define	IIC_ADDR_READ	1
#define	IIC_ADDR_WRITE	0

/*
 *	unix ioctl parameter list
 */
struct ioctlio {
	long devreg;
	long data;
};

/*
 *	iic config struct
 */
struct iicconf {
	unsigned char	dc_iicdev;	/* IN:	0 -> surfboard,
						1 -> caitlioA,
						2 -> caitlioB,
						3 -> gwenio		*/
					/* OUT:	0 -> valid data returned,
						1 -> no device present	*/
	unsigned char	dc_iicrom[254];	/* 255 bytes of configuration eeprom */
};

/*
 *	iic read/write
 */
struct iicio {
	unsigned char	dc_iicdev;	/* IN: iic address of selected device */
					/* OUT:	0->valid data returned,
						1->no device present	*/
	unsigned char	dc_iicdata;	/* data read/written to device */
};

/*
 *	this is a per-dsp descriptor
 */
struct	dspinfo {
	unsigned int	dsp_type;	/* 0x3C == standard 32C, 0x5E == DSP32C-5E */
	unsigned int	dsp_ctl;	/* offset to register with RUN bits */
	unsigned int	dsp_run;	/* RUN bit */
	unsigned int	dsp_addr;	/* offset to start of dsp PIO register file */
	unsigned int	dsp_irq;	/* offset to control register with irq bit */
	unsigned int	dsp_irqh;	/* this bit OR'd into dsp_irq causes irq at dsp */
	unsigned int	dsp_irql;	/* this bit cleared from dsp_irq causes irq at dsp */
}	typedef	DSPINFO;

struct iicdspinfo {
	unsigned char	dc_iicdev;	/* IN:	0->surfboard,
						1->caitlio_a,
						2->caitlio_b,
						3->gwenio		*/
					/* OUT:	0->valid data returned, 
						1->no config rom	*/
	unsigned char	dc_iicdsp;	/* IN: which dsp? (0-7) */
					/* OUT: 1->no dsp, 0->dsp present */
	DSPINFO	dc_dspinfo;		/* returned by ioctl */
};

/*
 ********************************************************************************************
 *	ioctls
 ********************************************************************************************
 */
#ifdef	_IOW
/*
 *	ioctls for the sun driver
 */
#define	DC_CLRSIG	_IO(d, 1)	/* mjm: no data (was _IOWR + ioctlio) */
#define	DC_WAIT4IRQ	_IO(d, 2)	/* mjm: no data (was _IOWR + ioctlio) */
#define	DC_RESET	_IO(d, 3)
#define	DC_RUN		_IO(d, 4)	/* mjm: no data (was _IOWR + ioctlio) */
#define	DC_STOP		_IO(d, 5)	/* mjm: no data (was _IOWR + ioctlio) */
#define	DC_SIG		_IO(d, 6)	/* mjm: no data (was _IOWR + ioctlio) */
#define	DC_IRQ		_IOWR(d, 7, struct ioctlio)	/* mjm: unused ?? */
#define	DC_CLRSEM	_IOWR(d, 8, struct ioctlio)
#define	DC_SETSEM	_IOWR(d, 9, struct ioctlio)
#define	DC_GETDREG	_IOWR(d, 10, struct ioctlio)
#define	DC_SETDREG	_IOWR(d, 11, struct ioctlio)
#define	DC_GETBREG	_IOWR(d, 12, struct ioctlio)
#define	DC_SETBREG	_IOWR(d, 13, struct ioctlio)
#define	DC_GETGWENB	_IOWR(d, 14, struct ioctlio)
#define	DC_SETGWENB	_IOWR(d, 15, struct ioctlio)
#define	DC_GETGWENS	_IOWR(d, 16, struct ioctlio)
#define	DC_SETGWENS	_IOWR(d, 17, struct ioctlio)
#define	DC_GETSEM	_IOWR(d, 18, struct ioctlio)
#define	DC_WAIT4PDFL	_IOWR(d, 19, struct ioctlio)
#define	DC_WAIT4PDFH	_IOWR(d, 20, struct ioctlio)
#define	DC_ENBPIF	_IO(d, 21)	/* mjm: no data (was _IOWR + ioctlio) */
#define	DC_DISPIF	_IO(d, 22)	/* mjm: no data (was _IOWR + ioctlio) */
#define	DC_DISGWEN	_IO(d, 23)	/* mjm: no data (was _IOWR + ioctlio) */
#define	DC_ENBGWEN	_IO(d, 24)	/* mjm: no data (was _IOWR + ioctlio) */
#define	DC_POKE		_IO(d, 25)	/* mjm: no data (was _IOWR + ioctlio) */
#define	DC_IRQSTA	_IOWR(d, 26, struct ioctlio)
#define	DC_GETDRAMSIZE	_IOR(d, 27, struct ioctlio)
#define	DC_INIT		_IO(d,28)
#define	DC_CONFIG	_IOWR(d, 29, struct iicconf)
#define	DC_IICRD	_IOWR(d, 30, struct iicio)
#define	DC_IICWR	_IOWR(d, 31, struct iicio)
#define	DC_DSPINFO	_IOWR(d, 32, struct iicdspinfo)

#else			/* sysV */
	/* sysV code untested, and probably not up-to-date */
#define	D3CIO	('D' << 8)
#define	DC_CLRSIG	(D3CIO | 0x1)
#define	DC_WAIT4IRQ	(D3CIO | 0x2)
#define	DC_RESET	(D3CIO | 0x3)
#define	DC_RUN		(D3CIO | 0x4)
#define	DC_STOP		(D3CIO | 0x5)
#define	DC_SIG		(D3CIO | 0x6)
#define	DC_IRQ		(D3CIO | 0x7)
#define	DC_CLRSEM	(D3CIO | 0x8)
#define	DC_SETSEM	(D3CIO | 0x9)
#define	DC_GETDREG	(D3CIO | 0xa)
#define	DC_SETDREG	(D3CIO | 0xb)
#define	DC_GETBREG	(D3CIO | 0xc)
#define	DC_SETBREG	(D3CIO | 0xd)
#define	DC_GETGWENB	(D3CIO | 0xe)
#define	DC_SETGWENB	(D3CIO | 0xf)
#define	DC_GETGWENS	(D3CIO | 0x10)
#define	DC_SETGWENS	(D3CIO | 0x11)
#define	DC_GETGWENL	(D3CIO | 0x12)
#define	DC_SETGWENL	(D3CIO | 0x13)
#define	DC_WAIT4PDF	(D3CIO | 0x14)
#define	DC_ENBPIF	(D3CIO | 0x15)
#define	DC_ENBSEM	(D3CIO | 0x16)
#define	DC_ENBBPT       (D3CIO | 0x17)
#define	DC_ENBGWEN	(D3CIO | 0x18)
#define	DC_POKE		(D3CIO | 0x19)
#define	DC_IRQSTA	(D3CIO | 0x1a)
#define	DC_GETDRAMSIZE	(D3CIO | 0x1b)
#endif

/*
 *	bits in dcpriv[].status
 */
#define	DC_IRQBSY	1
#define	DC_TIMEOUT	2
#define	DC_OPENED	4			/* mjm: device is open */
#define	DC_EXCL		8			/* mjm: open f exclusive use */

#define O_PRIVATE	(O_EXCL|O_APPEND)	/* mjm: kludge O_APPEND use */

#define	SIGDSP		31

/*
 *	defines for board registers, in shorts
 */
#define	DC_CSR		0
#define	DC_STA		1
#define	DC_IVEC		2
#define	DC_IIC1		4
#define	DC_IIC2		5

/*
 *	how many loops before we conclude that pdr is hung?
 */
#define	DC_DMA_CNT	10

/*
 *	how big is DRAM? This should actually be an ioctl.
 *
 *	later ..
 */
#define	DC_DRAMSIZE	0x100000
#define	DC_DRAMSTART	0x800000

/*
 *	dsp address space (ie, 24b)
 */
#define	DC_MEMSIZE	0x1000000

#define	DC_MEM5SIZE	0x2000		/* DSP32C5E's -- no EMI */

/*
 *	number of SURFboard devices:
 *	6 dsps + DRAM + IO page => 8
 */
#define	SURFBOARDDEVS	8
