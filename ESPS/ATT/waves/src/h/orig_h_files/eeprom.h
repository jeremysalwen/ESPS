/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/* @(#)eeprom.h	1.2 9/26/95 ATT/ERL/ESI */
/*
 *	this file defines the configuration ROM formats for surfboard and
 *	daughter cards.
 */
#define	SURFBOARD	0
#define	CAITLIOA	1
#define	CAITLIOB	2
#define	GWENIO		3

/*
 *	there are potentially up to 8 8582A ROMs, so up to 8 daughter
 *	cards. 0xA4 is reserved for SURFboard, and the others could be
 *	either SIO or PIO daughter cards (although for this card, it's
 *	2xSIO and 1xPIO.
 *
 *	cards identify themselves as SIO with d32c_type == 1, or as
 *	PIO with d32c_type == 2. SURFboard has d32c_type == 0.
 */

#define	SURFBOARD_IIC	0xA4
#define	CAITLIO_A_IIC	0xA0
#define	CAITLIO_B_IIC	0xA2
#define	GWENIO_IIC	0xA6

/*
 *	this is a per-dsp descriptor: offsets are from 0 on the card, not from
 *	the beginning of SURFboard I/O addresses, since we may want to use the
 *	plug-in modules on other mother cards.
 */
struct	dspctl {
	unsigned char	dsp_type;	/* 0x3C == standard 32C, 0x5E == DSP32C-5E */
	unsigned char	dsp_addr;	/* offset to start of dsp PIO register file (downshifted 4b) */
	unsigned short	dsp_run;	/* this bit OR'd into dsp_ctl causes dsp to RUN */
	unsigned char	dsp_irq;	/* offset to control register with irq bit */
	unsigned char	dsp_irqp;	/* polarity of bit which causes irqs at dsp (0=active high) */
	unsigned short	dsp_irqb;	/* this bit OR'd into dsp_irq causes irq at dsp */
}	typedef	DSPCTL;

/*
 *	this is the gwenio dependent part
 */
struct	{
	unsigned char	dsp_options[2];	/* undefined, needed for 4 byte alignment */
	unsigned char	dsp_ctl;	/* offset to register with RUN and Z bits */
	unsigned char	dsp_3senb;	/* Z bit, 0=>high impedance (ie disabled) */
	DSPCTL	dsp_info[8];		/* gwenio can have dsps, we need to know where they */
					/* are for rtpi initialization */
	char	gwen_iic;		/* number of iic ports on the card (1 => only EEROM) */
	char	gwen_iicaddr[2];	/* address of iic ports (other than EEROM); if LSB is */
					/* 1, then this is an extended iic address, i.e. requires */
					/* two bytes */
					/* recognize that the 2 LSBs are hardwired at the connector */
}	typedef	GWENINFO;		/* mfr can include any other info in the remaining bytes */

/*
 *	this is the sio dependent part
 */
struct	{
	unsigned short	tbl_value;	/* value to write to sample rate controller */
	unsigned short	tbl_rate;	/* cait_div*tbl_rate = resultant rate in KHz */
}	typedef	CAIT_TBL;

struct	{
	unsigned short	cait_a2d;	/* A/D parameters:					*/
					/*	b<0:7>	 number of a/d				*/
					/*	b<8:9>	 number of bytes/word (0 <-> 4)		*/
					/*	b<10:11> number of words/LD clock (0 <-> 4)	*/
					/*	b14: msb first (1) / lsb first (0)		*/
					/*	b15: active (1) / passive (0)			*/
	unsigned short	cait_d2a;	/* D/A parameters:					*/
					/*	b<0:7>	 number of d/a				*/
					/*	b<8:9>	 number of bytes/word (0 <-> 4)		*/
					/*	b<10:11> number of words/LD clock (0 <-> 4)	*/
					/*	b14: msb first (1) / lsb first (0)		*/
					/*	b15: active (1) / passive (0)			*/
	unsigned char	cait_iic;	/* number of iic ports on the card (1 => only EEROM)	*/
	unsigned char	cait_iicaddr[3];	/* address of iic ports (other than EEROM); if LSB is */
					/* 1, then this is an extended iic address, i.e. requires */
					/* two bytes						*/
	unsigned short	cait_xtal;	/* clock frequency in KHz				*/
	unsigned short	cait_div;	/* dual use prescaler: if cait_type == 0, then		*/
					/*    cait_xtal/cait_div is the effective dividend	*/
					/*    for generating the sample rate. if cait_type == 1 */
					/*    then the result (in KHz) of writing the integer "i" */
					/*    into the sample rate control register is		*/
					/*    cait_div*cait_tbl[i].tbl_rate			*/
	unsigned short	cait_type;	/* type 0 => any int divider, 1 => table driven		*/
	unsigned short	cait_min;	/* min setting						*/
	unsigned short	cait_max;	/* max setting						*/
	CAIT_TBL	cait_tbl[8];	/* table values						*/
}	typedef	CAITLINFO;		/* mfr can include any other info in the remaining bytes */

/*
 *	this is the SURFBOARD dependent part. for convenience of the
 *	device driver, any other device with dsps should match the first
 *	five fields (through dsp_info). Unused dsp_info fields can be
 *	reused, the driver doesn't look at them. (The user can get at them
 *	with the DC_CONFIG ioctl().)
 */
struct	{
	unsigned char	dsp_options[2];		/* undefined, needed for quad byte alignment */
	unsigned char	dsp_ctl;		/* offset to register with RUN and Z bits */
	unsigned char	dsp_3senb;		/* Z bit, 0=>high impedance (ie disabled) */
	DSPCTL		dsp_info[8];
	unsigned char	dsp_gwenpres;		/* number of GWENIO ports */
	unsigned char	dsp_rsv2;		/* alignment .. */
	unsigned short	dsp_gwenaddr;		/* register file offsets for GWENIO */
	unsigned char	rev_info[16];		/* pld revs */
}	typedef	SURFINFO;

/*
 *			... THIS IS THE EEROM FORMAT ...
 *
 *	The common (required) part takes 152 bytes of the 256 byte 8582, leaving
 *	104 bytes for card configuration info.
 */
struct {
	unsigned long	d32c_magic;		/* magic number:	0xA600D32C ("A GOOD 32C") */
	char	d32c_mfr[32];			/* manufacturer:	"AT&T Microelectronics" */
	char	d32c_model[20];			/* model name:		"SURFboard" */
	unsigned long	d32c_serial;		/* serial number:	1 */
	unsigned char	d32c_major;		/* major rev. number:	3 */
	unsigned char	d32c_minor;		/* minor rev. number:	2 */
	char	d32c_email[30];			/* e-mail address:	"surf@inet.att.com" */
	char	d32c_network[14];		/* network name:	"internet" */
	char	d32c_ccode[2];			/* country code:	? */
	char	d32c_telno[12];			/* telephone number:	"9085825667" */
	char	d32c_type;			/* board type:		0 => VME board (1=>SIO, 2=>PIO) */
	char	d32c_date[27];			/* unix date (ctime(3) format */
	char	d32c_drsize;			/* size of DRAM in multiples of 256KBy */
	unsigned char	d32c_pres;		/* binary encoding of DSPs present; 1 => D0 present,
						/* 5 => D0 and D2 present, etc. */
	unsigned short	d32c_clk;		/* clock freqency/MHz << 8 */
	union {
		char		info[104];	/* 104 bytes of board specific info */
		GWENINFO	gweninfo;	/* Parallel IO daughter cards */
		CAITLINFO	caitlinfo;	/* SIO daughter cards */
		SURFINFO	surfinfo;	/* SURFBOARD info */
	} d32c_bdinfo;
}	typedef	EEROM;
