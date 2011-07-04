/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1990  Entropic Speech, Inc.; 
 *                   All rights reserved"
 *
*/

/*
muplay reads a headerless, mu-law encoded file and plays
it. The -x option plays through the external headphone jack,
so a powered speaker can be hooked up. If "-" is specified
as the only input file, standard input is read.
*/

static char sccsid[] = "@(#)muplay.c	1.1 4/2/90 ESI";

#include <sys/types.h>
#include <sys/dir.h>
#include <sys/file.h>

#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

#include <sys/ioctl.h>
#include <sbusdev/audioreg.h>
#include <sun/audioio.h>

#define	FPRINTF	(void) fprintf

#define BUFFER_SIZE	819200

int	Audio_fd;

extern	int	errno;

main(argc,argv)
	int	argc;
	char	*argv[];
{

	int offset = 1;

/*
 * check command line
*/

	if(argc < 2){
		FPRINTF(stderr, "Usage: %s [-x] <u-law file>\n", argv[0]);
		exit(1);
	}
	if(argv[1][0] == '-' && argv[1][1] == 'x'){
		offset++;
		if(argc < 3){
		      FPRINTF(stderr, "Usage: %s [-x] <u-law file>\n", argv[0]);
		      exit(1);
		}
	}

/*
 * Set up device and play files
*/

	audio_open(offset-1);

	play_files(argv, argc, offset);

	audio_close();

	/* NOTREACHED */
}

play_files(filename, n_args, offset)
	char	*filename[];
	int     n_args;
	int     offset;
{
	int	fd, i;
	int	rtn = 0, oldrtn = 0;
	char	buffer[BUFFER_SIZE];

	if(filename[offset][0] == '-'){/*read standard input*/
	  rtn = fread(buffer,sizeof(*buffer), BUFFER_SIZE, stdin);
	}
	else{/* read files named on command line*/
	  for(i = offset; i < n_args; i++){ 
		if ((fd = open(filename[i],0)) < 0) {
			FPRINTF(stderr, "error opening \"%s\" - exiting\n", filename[i]);
			exit(1);
		}
		oldrtn = rtn;
		rtn += read(fd,buffer+rtn,BUFFER_SIZE-oldrtn);

		if (rtn < oldrtn) {
		      FPRINTF(stderr,"\"%s\" is an empty file\n", filename[i]);
			exit(1);
		}
	      }
	(void) close(fd);
	}

	if (write(Audio_fd,buffer,rtn) < 0) {
		perror("/dev/audio");
	}
}

audio_open(external)
	int external;
{
	struct	audio_ioctl	tmp;

	if ((Audio_fd = open("/dev/audio",2)) < 0) {
	  FPRINTF(stderr,"muplay: Can't open audio device.\n");
	  exit(1);
		}

	/* Initialize the three gain registers to 26 db for internal
	   speaker and 15 dB for external speaker. */
	if(external == 1){/*use external jack*/
		audio_set_ger(10);
		audio_set_gr(5);
		audio_set_gx(5);
	}
	else{/*use internal speaker*/
		
		audio_set_ger(16);
		audio_set_gr(10);
		audio_set_gx(10);
	}

	/*
	 * Tell the chip to set the gains according to the
	 * register values we just set.
	 */

	tmp.control = AUDIO_MAP_MMR1;
	tmp.data[0] = AUDIO_MMR1_BITS_LOAD_GX |
	    AUDIO_MMR1_BITS_LOAD_GR |
	    AUDIO_MMR1_BITS_LOAD_GER;

	if (ioctl(Audio_fd,AUDIOSETREG,&tmp) < 0) {
			perror("Set REG");
		}


	/*  Initialize the MMR2 register to send the output to the builtin
	 *  speaker.  This is the default, but if "external" is set, we will 
	 *  send the signal to the earphone jack for external monitoring
	*/
	tmp.control = AUDIO_MAP_MMR2;
	if (ioctl(Audio_fd,AUDIOGETREG,&tmp) < 0) {
		perror("Set REG");
	}

	if (external == 1)/*use external jack*/
		tmp.data[0] &= ~AUDIO_MMR2_BITS_LS;
	else/*use builtin speaker*/
		tmp.data[0] |= AUDIO_MMR2_BITS_LS;
	if (ioctl(Audio_fd,AUDIOSETREG,&tmp) < 0) {
		perror("Set REG");
	}

}


	/*	These are tables of values to be loaded into various
		gain registers.

		Note that for the ger entry for -8db, we use the data
		sheet value for -7.5db.  The data sheet gives values for
		-8db which are wrong and produce too much gain.
	*/

static	unsigned char ger_table[][2] = {
		0xaa,	0xaa,	/* -10db */
		0x79,	0xac,
		/*0x41,	0x91,*/
		0x31,	0x99,	/* -7.5db */
		0x9c,	0xde,
		0x74,	0x9c,	/* -6db */
		0x6a,	0xae,
		0xab,	0xdf,
		0x64,	0xab,
		0x2a,	0xbd,
		0x5c,	0xce,
		0x00,	0x99,	/* 0db */
		0x43,	0xdd,
		0x52,	0xef,
		0x55,	0x42,
		0x31,	0xdd,
		0x43,	0x1f,
		0x40,	0xdd,	/* 6db */
		0x44,	0x0f,
		0x31,	0x1f,
		0x10,	0xdd,
		0x41,	0x0f,
		0x60,	0x0b,
		0x42,	0x10,	/* 12db */
		0x11,	0x0f,
		0x72,	0x00,
		0x21,	0x10,
		0x22,	0x00,
		0x00,	0x0b,
		0x00,	0x0f,	/* 18db */
};


static	unsigned char gr_gx_table[][2] = {
		0x8b,	0x7c,	/* -18db */
		0x8b,	0x35,
		0x8b,	0x24,
		0x91,	0x23,
		0x91,	0x2a,
		0x91,	0x3b,
		0x91,	0xf9,	/* -12db */
		0x91,	0xb6,
		0x91,	0xa4,
		0x92,	0x32,
		0x92,	0xaa,
		0x93,	0xb3,
		0x9f,	0x91,	/* -6db */
		0x9b,	0xf9,
		0x9a,	0x4a,
		0xa2,	0xa2,
		0xaa,	0xa3,
		0xbb,	0x52,
		0x08,	0x08,	/* 0db */
		0x3d,	0xac,
		0x25,	0x33,
		0x21,	0x22,
		0x12,	0xa2,
		0x11,	0x3b,
		0x10,	0xf2,	/* 6db */
		0x02,	0xca,
		0x01,	0x5a,
		0x01,	0x12,
		0x00,	0x32,
		0x00,	0x13,
		0x00,	0x0e,	/* 12db */
};



audio_set_ger(value)
	int	value;
{
	struct	audio_ioctl	tmp;


	if ((value < -10) || (value > 18)) {
		FPRINTF(stderr,
		    "GER value %d out of range; %d <= GER <=  %d\n",
		    value,0,18);
		return;
	}

	/*  Add 10 to the value to get the index into the table.
	 */
	tmp.control = AUDIO_MAP_GER;
	tmp.data[0] = ger_table[value + 10][1];
	tmp.data[1] = ger_table[value + 10][0];

	if (ioctl(Audio_fd,AUDIOSETREG,&tmp) < 0) {
		perror("Set REG");
	}
}


audio_set_gr(value)
	int	value;
{
	struct	audio_ioctl	tmp;

	if ((value < -18) || (value > 12)) {
		FPRINTF(stderr,
		    "GR value %d out of range; %d <= GR <=  %d\n",
		    value,0,12);
		return;
	}

	tmp.control = AUDIO_MAP_GR;
	tmp.data[0] = gr_gx_table[value + 18][1];
	tmp.data[1] = gr_gx_table[value + 18][0];

	if (ioctl(Audio_fd,AUDIOSETREG,&tmp) < 0) {
		perror("Set REG");
	}
}


audio_set_gx(value)
	int	value;
{
	struct	audio_ioctl	tmp;

	if ((value < -18) || (value > 12)) {
		FPRINTF(stderr, 
		    "GX value %d out of range; %d <= GX <=  %d\n",
		    value,0,12);
		return;
	}

	/*  We add 18 to get the index into the table, since entry 0 represents
	 *  -18db.
	 */
	tmp.control = AUDIO_MAP_GX;
	tmp.data[0] = gr_gx_table[value + 18][1];
	tmp.data[1] = gr_gx_table[value + 18][0];

	if (ioctl(Audio_fd,AUDIOSETREG,&tmp) < 0) {
		perror("Set REG");
	}
}

audio_close()
{
        int dummy;
	unsigned int sleep = 50000;

	do{/* wait for write que to empty*/
	  usleep(sleep);
	  if(ioctl(Audio_fd, AUDIOWRITEQ,&dummy) < 0)
	    perror("AUDIOWRITEQ request failed");
	} while (dummy >= 1);

	/*Close up audio buffers*/
	if(ioctl(Audio_fd, AUDIOSTOP, &dummy) < 0)
	  perror("AUDIOSTOP failed");

	/*close up audio device*/
	(void)close(Audio_fd);
}


