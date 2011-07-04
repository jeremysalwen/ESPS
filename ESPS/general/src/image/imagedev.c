/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
| "Copyright (c) 1988, 1989 Entropic Speech, Inc. All rights reserved."	|
|									|
+-----------------------------------------------------------------------+
|									|
|  Module: imagedev.c							|
|									|
|  This program displays data from an ESPS file as a half-tone		|
|  gray-scale image.							|
|									|
|  Rodney W. Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
static char *sccs_id = "@(#)imagedev.c	1.6	6/28/93	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include "image.h"

#if defined(M5600) | defined(M5500)
void mcd_init();
#else
int mcd_init();
#endif

void		mcd_fin(), mcd_default_size();
int		mcd_depth();
void		typ_init(), typ_fin(), typ_default_size();
void		imp_init(), imp_fin(), imp_default_size();
void		ras_init(), ras_fin(), ras_default_size();
void		x11_fin(), x11_default_size();
int		x11_depth();
void		ps_init(), ps_fin(), ps_default_size();
void		hp_init(), hp_fin(), hp_default_size();
int		x11_init();

void		(*dev_initbits)();
void		(*dev_row)();
void		(*dev_plotline)();

extern long	lmarg, rmarg, tmarg, bmarg;

extern char	*ProgName;
extern int	debug_level;
extern int	mag;
extern int	dev;


void
dev_init()
{
    switch (dev)
    {
    case DEV_MCD:
	(void)mcd_init();
	break;
    case DEV_TYPE:
	typ_init();
	break;
    case DEV_IMAGEN:
	imp_init();
	break;
    case DEV_RAS:
	ras_init();
	break;
    case DEV_X11:
#ifdef SUNVUE
	if(!x11_init()) { /* failed to connect to server, try Suntools */
		dev =  DEV_MCD;
		if(!mcd_init()) {
		 Fprintf(stderr,"%s: cannot connect to Suntools or X.\n",
		  ProgName);
		 exit(1);
		}
	}
#else
	if(!x11_init()) {
		Fprintf(stderr,"%s: Cannot connect to X display.\n",
		 ProgName);
		exit(1);
	}
#endif
	break;
    case DEV_PS:
	ps_init();
	break;
    case DEV_HP:
	hp_init();
	break;
    default:
	Fprintf(stderr, "%s: device type not recognized.\n", ProgName);
	exit(1);
    }
}


void
dev_fin()
{
    switch (dev)
    {
    case DEV_MCD:
	mcd_fin();
	break;
    case DEV_TYPE:
	typ_fin();
	break;
    case DEV_IMAGEN:
	imp_fin();
	break;
    case DEV_RAS:
	ras_fin();
	break;
    case DEV_X11:
	x11_fin();
	break;
    case DEV_PS:
	ps_fin();
	break;
    case DEV_HP:
	hp_fin();
	break;
    default:
	Fprintf(stderr, "%s: device type not recognized.\n", ProgName);
	exit(1);
    }
}

void
get_default_size(w, h)
    long    *w, *h;
{
    switch (dev)
    {
    case DEV_MCD:
	mcd_default_size(w, h);
	break;
    case DEV_TYPE:
	typ_default_size(w, h);
	break;
    case DEV_IMAGEN:
	imp_default_size(w, h);
	break;
    case DEV_RAS:
	ras_default_size(w, h);
	break;
    case DEV_X11:
	x11_default_size(w, h);
	break;
    case DEV_PS:
	ps_default_size(w, h);
	break;
    case DEV_HP:
	hp_default_size(w, h);
	break;
    default:
	Fprintf(stderr, "%s: device type not recognized.\n", ProgName);
	exit(1);
    }
}

int depth()
{
    int value;

    switch (dev)
    {
    case DEV_MCD:
	return mcd_depth();
	break;
    case DEV_TYPE:
	return 1;
	break;
    case DEV_IMAGEN:
	return 1;
	break;
    case DEV_RAS:
	return 1;
	break;
    case DEV_X11:
#ifdef SUNVUE
	if((value = x11_depth()) == 0) { 
	/* failed to connect to server, try Suntools */
		dev =  DEV_MCD;
		if((value = mcd_depth()) == 0) {
		 Fprintf(stderr,"%s: cannot connect to Suntools or X.\n",
		  ProgName);
		 exit(1);
		}
	}
	return value;
#else
	if((value = x11_depth()) == 0) {
		Fprintf(stderr,"%s: Cannot connect to X display.\n",
		 ProgName);
		exit(1);
	}
	return value;
#endif
	break;
    case DEV_PS:
	return 1;
	break;
    case DEV_HP:
	return 1;
	break;
    default:
	Fprintf(stderr, "%s: device type not recognized.\n", ProgName);
	exit(1);
    }
    return 0;
}


void
get_scale(s)
    int	    *s;
{
    if (*s == 0)
	switch (dev)
	{
	case DEV_MCD:
	    *s = 2;
	    break;
	case DEV_TYPE:
	    *s = 2;
	    break;
	case DEV_IMAGEN:
	    *s = 1 + 5/mag;
	    break;
	case DEV_RAS:
	    *s = 2;
	    break;
	case DEV_X11:
	    *s = 2;
	    break;
	case DEV_PS:
	    *s = 1 + 5/mag;
	    break;
	case DEV_HP:
	    *s = 1 + 5/mag;
	    break;
	default:
	    Fprintf(stderr, "%s: device type not recognized.\n", ProgName);
	    exit(1);
	    /*NOTREACHED*/
	}
}
