/******************************************************************************
*                                                                             *
*       Copyright (C) 1992-1995 Tony Robinson                                 *
*                                                                             *
*       See the file LICENSE for conditions on distribution and usage         *
*                                                                             *
******************************************************************************/

/* 
  set the atime and mtime of path1 to be the same as that of path0
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
/* Changed by Jon Fiscus */
# include <util/fob.h>
# include <sp/shorten/shorten.h>

/* From: fred@masterd.fdn.org (Frederic SAVOIR) */
#ifdef __WATCOMC__
#include <io.h>
#endif
/*
#ifdef NARCH_HP
typedef long time_t;
#endif
*/

/* edited by Alan Parker, ERL.   The following was not originally static
   and it conflicted with a symbol in the Unix clock() function on Sunos
   6/30/97
*/

static struct utimbuf {
  time_t  actime;  /* set the access time */
  time_t  modtime; /* set the modification time */
} times;


extern int	utime		PROTO((const char*, const struct utimbuf*));
extern int	chown		PROTO((const char*, long, long));

int dupfileinfo(path0, path1) char *path0, *path1; {
  int errcode;
  struct stat buf;


  errcode = stat(path0, &buf);
  if(!errcode) {
    /* do what can be done, and igore errors */
    (void) chmod(path1, buf.st_mode);
    times.actime  = buf.st_atime;
    times.modtime = buf.st_mtime;
    (void) utime(path1, &times);
#ifdef unix
    (void) chown(path1, buf.st_uid, -1);
    (void) chown(path1, -1, buf.st_gid);
#endif
  }
  return(errcode);
}

#ifdef PROGTEST
int main(int argc, char **argv) {
  return(dupfileinfo(argv[1], argv[2]));
}
#endif
