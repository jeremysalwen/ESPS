/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*	Copyright (c) 1987, 1988 AT&T	*/
/*	  and 1993 Entropic Research Laboratory.	*/
/*	  All Rights Reserved.	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	AND ENTROPIC SPEECH, INC.				*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/

/* environment.c */
/* routines for dealing with environment variables and associated pathnames. */

#ifndef lint
static char *sccs_id = "@(#)environment.c	1.15	9/26/95	ATT/ERL";
#endif

#include <stdio.h>
#ifndef LBIN
#define LBIN "/usr/local/bin"
#endif

char *savestring();

/*************************************************************************/
setup_environ()
{
  static char root[50], bin[100], misc[100], text[100], src[100], lbin[100];
  static char dsp32_bin[100], dsp32c_bin[100], xvdodo[30];
  char *r, *getenv();
  
#if defined(SONY_RISC) || defined(CONVEX)
    sprintf(xvdodo,"/dev/null");
    setenv("EXTRASMENU",xvdodo,1);
#else
    strcpy(xvdodo,"EXTRASMENU=/dev/null");
    putenv(xvdodo);
#endif

  if(!(r=getenv("WAVES_ROOT"))) {
    r = DPATH;
#if defined(SONY_RISC) || defined(CONVEX)
    setenv("WAVES_ROOT",r,1);
#else
    sprintf(root,"WAVES_ROOT=%s",r);
    putenv(root);
#endif
  }
  if(!getenv("WAVES_BIN")) {
#ifdef STARDENT_3000
    sprintf(WAVES_BIN,"%s/bin",r);
#else
#if defined(SONY_RISC) || defined(CONVEX)
    sprintf(bin,"%s/bin",r);
    setenv("WAVES_BIN",bin,1);
#else
    sprintf(bin,"WAVES_BIN=%s/bin",r);
    putenv(bin);
#endif
#endif
  }
  if(!getenv("WAVES_MISC")) {
#ifdef STARDENT_3000
    sprintf(WAVES_MISC,"%s/lib/waves",r);
#else
#if defined(SONY_RISC) || defined(CONVEX)
    sprintf(misc,"%s/lib/waves",r);
    setenv("WAVES_MISC",misc,1);
#else
    sprintf(misc,"WAVES_MISC=%s/lib/waves",r);
    putenv(misc);
#endif
#endif
  }
  if(!getenv("WAVES_SRC")) {
#if defined(SONY_RISC) || defined(CONVEX)
    sprintf(src,"%s/src",r);
    setenv("WAVES_SRC",src,1);
#else
    sprintf(src,"WAVES_SRC=%s/src",r);
    putenv(src);
#endif
  }
  if(!getenv("WAVES_DOC")) {
#if defined(SONY_RISC) || defined(CONVEX)
    sprintf(text,"%s/text",r);
    setenv("WAVES_DOC",text,1);
#else
    sprintf(text,"WAVES_DOC=%s/text",r);
    putenv(text);
#endif
  }
  if(!getenv("LBIN")) {
#if defined(SONY_RISC) || defined(CONVEX)
    sprintf(lbin,"%s",LBIN);
    setenv("LBIN",lbin,1);
#else
    sprintf(lbin,"LBIN=%s",LBIN);
    putenv(lbin);
#endif
  if(!getenv("DSP32C_BIN")) {
#if defined(SONY_RISC) || defined(CONVEX)
    setenv("DSP32C_BIN",DSP32C_BIN,1);
#else
    sprintf(dsp32c_bin,"DSP32C_BIN=%s",DSP32C_BIN);
    putenv(dsp32c_bin);
#endif
  }
  if(!getenv("DSP32_BIN")) {
#if defined(SONY_RISC) || defined(CONVEX)
    setenv("DSP32_BIN",DSP32_BIN,1);
#else
    sprintf(dsp32_bin,"DSP32_BIN=%s",DSP32_BIN);
    putenv(dsp32_bin);
#endif
  }
  }
/*
    printf("DPATH:%s\nROOT:%s\nBIN:%s\nMISC:%s\nSRC:%s\nDOC:%s\n",DPATH,getenv("WAVES_ROOT"),
     getenv("WAVES_BIN"),getenv("WAVES_MISC"),getenv("WAVES_SRC"),
	 getenv("WAVES_DOC"));
*/
}

/*************************************************************************/
help_doc(what)
     register char *what;
{
  char str[500], *p, *getenv();

  if(what && *what) {
    if((p= getenv("WAVES_DOC")))
      sprintf(str,"cat %s/%s",p,what);
    else
      sprintf(str,"cat %s/text/%s",DPATH,what);
    system(str);
  }
  else
    printf("Bad command line (try another).\n");
}

/*************************************************************************/
char *
s_getenv(what)
     register char *what;
{
  static char defpath[] = " ";
  char *p, *getenv();

  if(what && *what && (p = getenv(what)))
    return(p);
  else
    return(defpath);
}

/*************************************************************************/
char *
full_path(env,leaf)
     register char *env, *leaf;
{
  static char path[200];
  char *p, *getenv();

#ifdef STARDENT_3000
  if(strcmp("WAVES_MISC",env) == 0) {
    sprintf(path,"%s/%s",WAVES_MISC,leaf);
    return(path);
  }
  if(strcmp("WAVES_BIN",env) == 0) {
    sprintf(path,"%s/%s",WAVES_BIN,leaf);
    return(path);
  }
  fprintf(stderr,"full_path: warning returning leaf.\n");
  return(leaf);
#else
  
  
  if(env && *env && leaf && (p = getenv(env)))
    sprintf(path,"%s/%s",p,leaf);
  else
    if(leaf && *leaf)
      return(leaf);
    else
      sprintf(path," ");

  return(path);
#endif
}
  
  
