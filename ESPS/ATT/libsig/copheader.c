/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1987-1990  AT&T, Inc.
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1997  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  David Talkin
 * Checked by:
 * Revised by:  David Talkin, Rod Johnson, John Shore, Alan Parker
 *
 * Brief descripton:  Create a header for headerless files.
 */

static char *sccs_id = "@(#)copheader.c	1.17	1/18/97	ATT/ESI/ERL";

#include <Objects.h>
#include <esps/esps.h> 
#include <sys/param.h>

char default_header[MAXPATHLEN] = "def_head.feasd";

int valid_header; /*ugly global; used to indicate case where header is ok 
		    but couldn't read data for some reason (e.g., 
		    requested segment not in file */

extern int  w_verbose;
extern int  debug_level;

extern int  size_rec();


Header *
header_from_file(name)
     char *name;
{
  Header *h = NULL, *get_header();
  int fd;

  if((!name) || (! *name)) name = default_header;
  
  if((fd = open(name, O_RDONLY)) >= 0) {
    h = get_header(fd);
    close(fd);
  } else
    printf("Can't open file %s in header_from_file()\n",name);
  return(h);
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
Signal *
get_any_signal(name,start,page_size,access_proc)
     char *name;
     double start, page_size;
     int (*access_proc)();
{
  Signal *s;
  int fd, fdh, size, den, recsize;
  Header *h;
  void free_signal();

  if (debug_level) 
    (void) fprintf(stderr, "get_any_signal(%s): function entered\n",
		   name);
  
  valid_header = 0;

  s = get_signal(name,start,page_size,access_proc);  
  
  if (s) 
    return(s);
  else if (valid_header) 
    return(FALSE);
    
#ifdef DEMO
  return FALSE;
#endif

  if((fd = open(name, O_RDONLY)) >= 0) { /* conclude that header is missing */
      /* NOTE that if the header was missing but read_header used 
       * the DEF HEADER environment variable, then that case would have 
       * been taken care of in get_signal - thus THE ENVIRONMENT VARIABLE
       * OVERRIDES default_header (and the code below is not entered).
       */
    if (debug_level) 
      (void) fprintf(stderr, 
		     "get_any_signal: missing header, will use default\n");

    FIND_WAVES_INPUT(default_header, default_header);

    fdh = open(default_header, O_RDONLY);  

    if (fdh < 0) {
      find_esps_file(default_header, "def_head.feasd",
		     "$ESPS_BASE/lib/waves/files", NULL); 
      printf("get_any_signal: no readable default_header\n");
      printf("\ttrying %s as last resort\n", default_header); 
      fdh = open(default_header, O_RDONLY);  
    }
   
    if(fdh >= 0) {
      if((h = get_header(fdh))) {
	if (debug_level) 
	  (void) fprintf(stderr, "get_any_signal: got default header\n");
	if (w_verbose) 
	  (void) fprintf(stderr, "Headerless input file; using header %s\n", 
			 default_header);
	if(h->strm)
	  fclose(h->strm);	/* won't be needing this again! */
	h->strm = NULL;
	close(fdh);
	if((size = lseek(fd,0,2)) > 0) {
	  lseek(fd,0,0);
	  if((s = new_signal(name,fd,h,NULL,0,0.0,0))) {
	    s->bytes_skip = 0;	/* recall: header is not in file! */
	    if(access_proc) s->utils->read_data = access_proc;
	    if (s->header->magic == ESPS_MAGIC) {
	      if (debug_level) 
		(void) fprintf(stderr, "get_any_signal: def header is ESPS\n");
	      /* fill in stream pointer */
	      s->header->strm = fdopen(fd, "r");

	      recsize = size_rec(s->header->esps_hdr);
	      if (recsize == -1)
	      {
		  /* Variable record size (e.g. Esignal Ascii).
		   * Can't get the file size without reading the whole
		   * file and counting, so use an overestimate
		   * and assume that "check_file_size" will supply the
		   * right value when we actually reach the end of file.
		   */
		  s->file_size = (s->dim > 1) ? size/s->dim : size;
		  /* Assumes each element is at least one byte. */
	      }
	      else
		s->file_size = size/recsize;

	      if (debug_level > 1) 
		(void) fprintf(stderr, 
		     "get_any_signal: setting file_size to %d records\n", 
		      s->file_size); 
	      if(s->utils->read_data(s,start,page_size)) {
		add_comment(s->header->esps_hdr, "Header created by waves+");
		return(s);
	      } else {
		printf("Can't read_data() in get_any_signal(%s)\n",name);
		free_signal(s);
		return(NULL);
	      }
	    }
	    else { /*sig or otherwise*/
		if(!(s->type & SIG_ASCII)) {
		  switch(s->type & VECTOR_SIGNALS) {
		  case P_CHARS:
		    den =  1; break;
		  case P_UCHARS:
		    den =  1; break;
		  case P_USHORTS:
		    den =  sizeof(short); break;
		  case P_SHORTS:
		    den =  sizeof(short); break;
		  case P_INTS:
		    den =  sizeof(int); break;
		  case P_UINTS:
		    den =   sizeof(int); break;
		  case P_FLOATS:
		    den =   sizeof(float); break;
		  case P_DOUBLES:
		    den =   sizeof(double); break;
		  default:
		    printf("Unknown type specified (%x)\n",s->type);
		    free_signal(s);
		    return(NULL);
		  }
		  s->file_size = size/den; /*number of records*/
		}
		else
		  /* assume largest possible ascii file */
		  s->file_size = size/(s->dim * 2); 

		if(s->utils->read_data(s,start,page_size)) {
		  head_printf(s->header, "samples", &(s->file_size));
		  head_printf(s->header,"remark", "Header created by waves");
		  return(s);
		} else {
		  printf("Can't read_data() in get_any_signal(%s)\n",name);
		  free_signal(s);
		  return(NULL);
		}
	    }
	  } else
	    printf("Can't make new_signal() in get_any_signal()\n");
	} else
	  printf("Can't seek in file %s in get_any_signal()\n",name);
	free(h->header);
	free(h);
      } else
	printf("Can't get_header(%s) in get_any_signal()\n",default_header);
      close(fdh);
    } else
      printf("Can't open(%s) for header in get_any_signal()\n",default_header);
    close(fd);
  } else
    printf("Can't open(%s) in get_any_signal()\n",name);

  return(NULL);
}
