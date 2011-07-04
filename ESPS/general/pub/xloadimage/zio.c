/* zio.c:
 *
 * this properly opens and reads from an image file, compressed or otherwise.
 *
 * jim frost 10.03.89
 *
 * Copyright 1989 Jim Frost.  See included file "copyright.h" for complete
 * copyright information.
 */

#ifdef SCCS
    static char *sccs_id = "@(#)zio.c	1.1  10/13/90";
#endif

#include "copyright.h"
#include "image.h"

ZFILE *zopen(name)
     char *name;
{ ZFILE *zf;
  char   buf[BUFSIZ];

  zf= (ZFILE *)lmalloc((unsigned int)sizeof(ZFILE));
  if ((strlen(name) > 2) && !strcmp(".Z", name + (strlen(name) - 2))) {
    zf->type= ZPIPE;
    sprintf(buf, "uncompress -c %s", name);
    if (! (zf->stream= popen(buf, "r"))) {
      lfree((byte *)zf);
      return(NULL);
    }
    return(zf);
  }
  zf->type= ZSTANDARD;
  if (! (zf->stream= fopen(name, "r"))) {
    lfree((byte *)zf);
    return(NULL);
  }
  return(zf);
}

int zread(zf, buf, len)
     ZFILE        *zf;
     byte         *buf;
     unsigned int  len;
{ int r;

  if ((r= fread(buf, len, 1, zf->stream)) != 1)
    return(r);
  return(len);
}

int zgetc(zf)
     ZFILE *zf;
{
  return(fgetc(zf->stream));
}

char *zgets(buf, size, zf)
     char         *buf;
     unsigned int  size;
     ZFILE        *zf;
{
  return(fgets(buf, size, zf->stream));
}

void zclose(zf)
     ZFILE *zf;
{
  switch(zf->type) {
  case ZSTANDARD:
    fclose(zf->stream);
    break;
  case ZPIPE:
    pclose(zf->stream);
    break;
  default:
    printf("zclose: bad ZFILE structure\n");
    exit(1);
  }
  lfree((byte *)zf);
}
