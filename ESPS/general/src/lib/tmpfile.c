/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1993 Entropic Speech, Inc.; All rights reserved"
 *
*/

#ifndef lint
	static char *sccs_id = "@(#)tmpfile.c	1.4 1/4/93 ESI";
#endif


#include <stdio.h>
#include <esps/esps.h>

FILE  *e_temp_file();

FILE *tmpfile()
{
  FILE *tmp;
  
  char *file_name;
  
  tmp = e_temp_file(NULL, &file_name);
  
  spsassert(tmp,"Cannot create temporary file!\n");

  (void)unlink(file_name);
  return tmp;
}

