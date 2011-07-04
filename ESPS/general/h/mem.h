/*
 *      (c) Copyright 1989 Sun Microsystems, Inc. Sun design patents
 *      pending in the U.S. and foreign countries. See LEGAL_NOTICE
 *      file for terms of the license.
 *
 *	Written for Sun Microsystems by Crucible, Santa Cruz, CA.
 */

#ifndef mem_H
#define mem_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

/* @(#) mem.h 25.1 90/04/10 Crucible */

/* @(#)mem.h	1.3 2/20/96 ERL */

extern void *MemAlloc();	/* malloc frontend */
extern void *MemCalloc();	/* calloc frontend */
extern void MemFree();		/* free frontend */

#define MemNew(t) ((t *)MemAlloc((unsigned int)sizeof(t)))
#define MemNewString(s) (strcpy(MemAlloc(strlen(s)+1),s))

#ifdef __cplusplus
}
#endif

#endif /* mem_H */
