/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*	Copyright (c) 1987, 1988 AT&T	*/
/*	  and Entropic Speech, Inc.	*/
/*	  All Rights Reserved.		*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	AND ENTROPIC SPEECH, INC.				*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/

/* pole_methods.c */
/*  I/O routines for dealing with LPC poles */

#ifndef lint
static char *sccs_id = "@(#)pole_methods.c	1.2	9/26/95	ATT/ERL";
#endif

#include <Objects.h>
#include <tracks.h>

int read_poles(), write_poles();

Signal  *new_signal();


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
write_poles(s, start, page_size)
     Signal *s;
     double start, page_size;
{
  int fd;
  double fudge;
  
#ifdef DEMO
  return FALSE;
#endif

  if(s && s->data) {
    if((fd = s->file) < 0)
      if(!((s->name) && ((s->file = fd = open(s->name,O_RDWR|O_CREAT,0664))
			 >= 0))) {
	printf("write_poles(): Can't open the output data file or file doesn't exist\n");
	return(FALSE);
      }
    fudge = 0.5/s->freq;
    if((BUF_START_TIME(s) > (fudge+start)) || (page_size < fudge) ||
       ((page_size+start-fudge) > BUF_END_TIME(s))) {
      printf("Bad start time or page size in write_data()\n");
      printf("wd: START:%f END:%f; start:%f page_size:%f\n",BUF_START_TIME(s),
	     BUF_END_TIME(s), start, page_size);
      return(FALSE);
    }
    if((s->type & SPECIAL_SIGNALS) == SIG_LPC_POLES) {

      switch(s->type & SIG_FORMAT) {
      case SIG_ASCII:
	return(put_ascii_poles(s,start,page_size));
      case SIG_BINARY:
	return(put_binary_poles(s,start,page_size));
      default:
	printf("Output format %x not supported in write_poles\n",
	       s->type & SIG_FORMAT);
	return(FALSE);
      }
    } else
      printf("signal type %x unknown in write_poles()\n",s->type);
  } else
    printf("Null data passed to write_poles()\n");
  return(FALSE);
}

#define A_BUF_SIZE 8000

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
put_ascii_poles(s,start,page_size)
     Signal *s;
     double start, page_size;
{
  Signal  *convert();
  FILE *fp, *fdopen();
  register int i, j, dim, nlim;
  int sam_to_skip, seek, loc, samples, its_ok;
  POLE **spp;
  char bufin[A_BUF_SIZE];

  its_ok = FALSE;
  if(s) {
    sam_to_skip = ((start - s->start_time) * s->freq) + 0.5;
    if((fp = fdopen(s->file,"r+"))) {
      if(! fseek(fp, s->bytes_skip, 0)) {
	for(i=0; i < sam_to_skip; i++) {
	  if(! fgets(bufin,A_BUF_SIZE,fp)) { /* assume one sample per line! */
	    printf("Error while seeking in ascii file in put_ascii_poles()\n");
	    fclose(fp);
	    close_sig_file(s);
	    return(FALSE);
	  }
	}
	samples = 0.5 + (page_size * s->freq);
	spp = (POLE**)s->data;
	loc = sam_to_skip - s->start_samp;
	nlim = samples + loc;
	its_ok = TRUE;
	for(j=loc; j < nlim; j++) {
	  dim = spp[j]->npoles;
	  i = (2 * dim) + 2;
	  if(fprintf(fp,"%4d %8.1f %13.6e ",i,spp[j]->rms,spp[j]->change) ==
	     EOF) its_ok = FALSE;
	  for(i=0 ; i < dim; i++)
	      if(fprintf(fp,"%13.6e ",spp[j]->freq[i]) == EOF) its_ok = FALSE;
	  for(i=0 ; i < dim; i++)
	      if(fprintf(fp,"%13.6e ",spp[j]->band[i]) == EOF) its_ok = FALSE;
	  if(fprintf(fp,"\n") == EOF) its_ok = FALSE;
	}
	if(! its_ok) printf("Problems during write in put_ascii_poles()\n");
	fclose(fp);
	close_sig_file(s);
      } else
	printf("Can't seek before writing data in put_ascii_poles()\n");
    } else
      printf("Can't open file %s for formatted output in put_ascii_poles()\n",s->name);
  } else
    printf("Null signal pointer passed to put_ascii_poles()\n");
  return(its_ok);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
put_binary_poles(s,start,page_size)
     Signal *s;
     double start, page_size;
{
  Signal  *convert();
  FILE *fp, *fdopen();
  register int i, j, nlim;
  int sam_to_skip, seek, loc, dim, samples, its_ok;
  POLE **spp;
  short bufin[A_BUF_SIZE];

  if((s->type & VECTOR_SIGNALS) != P_SHORTS){
    fprintf(stderr,"can't write but shorts\n");
    return(FALSE);
  }  its_ok = FALSE;
  if(s) {
    sam_to_skip = ((start - s->start_time) * s->freq) + 0.5;
    if((fp = fdopen(s->file,"r+"))) {
      if(! fseek(fp, s->bytes_skip, 0)) {
	for(i=0; i < sam_to_skip; i++) {
	  if(fread(bufin,sizeof(short),1,fp) == 1){
	    dim = *bufin;
	    if(fread(bufin,sizeof(short),dim,fp) == dim)
	      continue;
	  }
/*		otherwise error in reading	*/
	  printf("Error while seeking in binary file in put_binary_poles()\n");
	  fclose(fp);
	  close_sig_file(s);
	  return(FALSE);
	}
	samples = 0.5 + (page_size * s->freq);
	spp = (POLE**)s->data;
	loc = sam_to_skip - s->start_samp;
	nlim = samples + loc;
	its_ok = TRUE;
	for(j=loc; j < nlim; j++) {
	  dim = spp[j]->npoles;
	  *bufin = (2 * dim) + 2;
	  bufin[1] = spp[j]->rms;
	  bufin[2] = spp[j]->change;
	  for(i=0 ; i < dim; i++)
	      bufin[i+3] = spp[j]->freq[i] + .5;
	  for(i=0 ; i < dim; i++)
	      bufin[i+dim+3] = spp[j]->band[i] + .5;
	  if(fwrite(bufin,sizeof(short),2 * dim + 3,fp)!= (2 * dim + 3))
	    its_ok = FALSE;
	}
	if(! its_ok) printf("Problems during write in put_binary_poles()\n");
	fclose(fp);
	close_sig_file(s);
      } else
	printf("Can't seek before writing data in put_binary_poles()\n");
    } else
      printf("Can't open file %s for formatted output in put_binary_poles()\n",s->name);
  } else
    printf("Null signal pointer passed to put_binary_poles()\n");
  return(its_ok);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
get_ascii_poles(s,start,page_size)
     Signal *s;
     double start, page_size;
{
  FILE *fp, *fdopen();
  char bufin[A_BUF_SIZE], *cp, *get_next_item();
  register int i, j, dim, nsread;
  POLE **spp;
  double dtmp;
  int sam_to_skip, nsamples, nbytes, old_size, seek, nelem,
      loc, nread, flag;

  sam_to_skip = ((start - s->start_time) * s->freq) + 0.5;
  old_size = s->buff_size;
  s->buff_size = (int)(0.5 + (page_size * s->freq));
  if(s->data && (old_size == s->buff_size) &&
     (sam_to_skip == s->start_samp)) return(TRUE);
  fp = fdopen(s->file,"r");
  if( ! fseek(fp,s->bytes_skip,0)) {
    j = sam_to_skip;
    while(j--) {
      if(! fgets(bufin,A_BUF_SIZE,fp)) {
	printf("Error while seeking in ascii file in get_ascii_poles()\n");
	fclose(fp);
	close_sig_file(s);
	return(FALSE);
      }
    }
    if(s->data) {
      free_poles(s->data,old_size);
    }
    if(s->data && (old_size != s->buff_size)) {
      free(s->data);
      s->data = NULL;
    }
    if(!(spp = (POLE**)s->data)) {
      j = sizeof(POLE*) * s->buff_size;
      spp = (POLE**)malloc(j);
    }
    if(spp) {
      s->data = (caddr_t)spp;
      loc = 0;
      for(j=0; j < s->buff_size; j++) {
	cp = bufin;
	if(fgets(cp,A_BUF_SIZE,fp) && sscanf(cp,"%lf",&dtmp)) {
	  cp = get_next_item(cp);
	  dim = (dtmp - 2)/2;
	  if(dim >= 0) {
	    if(!(spp[j] = (POLE*)malloc(sizeof(POLE)))) {
	      printf("Can't allocate the %dth POLE in get_ascii_poles\n",j);
	      printf("buff_size:%d  dim:%d\n",s->buff_size, dim);
	      fclose(fp);
	      close_sig_file(s);
	      return(FALSE);
	    }
	    sscanf(cp,"%lf",&(spp[j]->rms));
	    cp = get_next_item(cp);
	    sscanf(cp,"%lf",&(spp[j]->change));
	    cp = get_next_item(cp);
	    if(dim > 0 ) {
	      spp[j]->freq = (double*)malloc(sizeof(double)*dim);
	      spp[j]->band = (double*)malloc(sizeof(double)*dim);
	      for(i=0; i < dim; i++) {
		if( ! sscanf(cp,"%lf",&(spp[j]->freq[i]))) {
		  spp[j]->freq[i] = 0.0;
		} else
		cp = get_next_item(cp);
	      }
	      for(i=0; i < dim; i++) {
		if( ! sscanf(cp,"%lf",&(spp[j]->band[i]))) {
		  spp[j]->band[i] = 0.0;
		} else
		cp = get_next_item(cp);
	      }
	    } else {
	      spp[j]->freq = NULL;
	      spp[j]->band = NULL;
	    }
	    spp[j]->npoles = dim;
	  } else {
	    printf("Error while reading ascii input at sample %d\n",j);
	    fclose(fp);
	    close_sig_file(s);
	    return(FALSE);
	  }
	} else {
	  s->file_size = sam_to_skip + j;
	  s->buff_size = j;	/* also forces loop termination */
	  j = s->buff_size;	/* just in case compiler is too clever */
	}
      }
      s->start_samp = sam_to_skip;
      fclose(fp);
      close_sig_file(s);
      return(TRUE);
    } else
      printf("Can't allocate buffer pointer array in get_ascii_poles()\n");
  } else
    printf("Can't fseek to data start in get_ascii_poles()\n");
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
get_binary_poles(s,start,page_size)
     Signal *s;
     double start, page_size;
{
  FILE *fp, *fdopen();
  short bufin[A_BUF_SIZE], *cp;
  register int i, j, nsread;
  int dim;
  POLE **spp;
  short stmp;
  int dtmp;
  int sam_to_skip, nsamples, nbytes, old_size, seek, nelem,
      loc, nread, flag;

  if((s->type & VECTOR_SIGNALS) != P_SHORTS){
    fprintf(stderr,"can't read but shorts\n");
    return(FALSE);
  }

  sam_to_skip = ((start - s->start_time) * s->freq) + 0.5;
  old_size = s->buff_size;
  s->buff_size = (int)(0.5 + (page_size * s->freq));
  if(s->data && (old_size == s->buff_size) &&
     (sam_to_skip == s->start_samp)) return(TRUE);
  fp = fdopen(s->file,"r");
  if( ! fseek(fp,s->bytes_skip,0)) {
    j = sam_to_skip;
    while(j--) {
      if(fread(&stmp,sizeof(short),1,fp) == 1){
	dim = stmp;
	if(fread(bufin,sizeof(short),dim,fp) == dim)
	  continue;
      }
/*		otherwise error in reading	*/
      printf("Error while seeking in binary file in get_binary_poles()\n");
      fclose(fp);
      close_sig_file(s);
      return(FALSE);
    }

    if(s->data) {
      free_poles(s->data,old_size);
    }
    if(s->data && (old_size != s->buff_size)) {
      free(s->data);
      s->data = NULL;
    }
    if(!(spp = (POLE**)s->data)) {
      j = sizeof(POLE*) * s->buff_size;
      spp = (POLE**)malloc(j);
    }
    if(spp) {
      s->data = (caddr_t)spp;

      loc = 0;
      for(j=0; j < s->buff_size; j++) {
	cp = bufin;
	if(fread(&stmp,sizeof(short),1,fp)==1){
	  dtmp = stmp;
	  if(fread(cp,sizeof(short),dtmp,fp)==dtmp){
	    dim = (dtmp - 2)/2;
	    if(dim >= 0) {
	      if(!(spp[j] = (POLE*)malloc(sizeof(POLE)))) {
		printf("Can't allocate the %dth POLE in get_binary_poles\n",j);
		printf("buff_size:%d  dim:%d\n",s->buff_size, dim);
		fclose(fp);
		close_sig_file(s);
		return(FALSE);
	      }
	      spp[j]->rms = *cp++;
	      spp[j]->change = *cp++;
	      if(dim > 0 ) {
		spp[j]->freq = (double*)malloc(sizeof(double)*dim);
		spp[j]->band = (double*)malloc(sizeof(double)*dim);
		for(i=0; i < dim; i++)
		  spp[j]->freq[i] = *cp++;
		for(i=0; i < dim; i++)
		  spp[j]->band[i] = *cp++;
	      } else {
		spp[j]->freq = NULL;
		spp[j]->band = NULL;
	      }
	      spp[j]->npoles = dim;
	    } else {
	      printf("Error while reading binary input at sample %d\n",j);
	      fclose(fp);
	      close_sig_file(s);
	      return(FALSE);
	    }
	  } else {
	    s->file_size = sam_to_skip + j;
	    s->buff_size = j;	/* also forces loop termination */
	    j = s->buff_size;	/* just in case compiler is too clever */
	  }
	}
      }
      s->start_samp = sam_to_skip;
      fclose(fp);
      close_sig_file(s);
      return(TRUE);
    } else
      printf("Can't allocate buffer pointer array in get_binary_poles()\n");
  } else
    printf("Can't fseek to data start in get_binary_poles()\n");
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
read_poles(s, start, page_size)
     Signal *s;
     double start, page_size;
{
  int fd, free_pole_data();

#ifdef DEMO
  return FALSE;
#endif

  if(page_size == 0.0) return(TRUE);
  if(s) {
    if((s->type & SPECIAL_SIGNALS) == SIG_LPC_POLES) {
      s->utils->write_data = write_poles;
      s->utils->free_data = free_pole_data;
      if((fd = s->file) < 0)
	if(!((s->name) && ((s->file = fd = open(s->name,O_RDONLY)) >= 0))) {
	  printf("read_poles(): Can't open the data file or file doesn't exist\n");
	  return(FALSE);
	}
      if((page_size < 0.0) && (s->freq > 0.0)) /* read whole file? */
	page_size = ((double)s->file_size)/s->freq;
      if(s->start_time > start)
	start = s->start_time;
      if((start + page_size) > (s->start_time + ((double)s->file_size/s->freq)))
	 page_size = (s->start_time + ((double)s->file_size/s->freq)) - start;
	
      switch(s->type & SIG_FORMAT) {
      case SIG_ASCII:
	return(get_ascii_poles(s, start, page_size));
      case SIG_BINARY:
	return(get_binary_poles(s,start,page_size));
      default:
	printf("Can't read format %x(h) in read_poles()\n");
	return(FALSE);
      }
    } else
      printf("Type (%x) of signal %s is not SIG_LPC_POLES in read_poles()\n",
	     s->type,s->name);
  } else
    printf("NULL Signal pointer to read_poles()\n");
}

/*      ----------------------------------------------------------      */
free_poles(p,n)
     POLE **p;
     int n;
{
  int i;

  for(i=0; i < n; i++) {
    if(p[i]) {
      if(p[i]->freq)	free(p[i]->freq);
      if(p[i]->band)	free(p[i]->band);
      free(p[i]);
      p[i] = NULL;
    }
  }
  return(TRUE);
}

/*      ----------------------------------------------------------      */
free_pole_data(s)
     Signal *s;
{
  POLE **ppp;

  if(((s->type & SPECIAL_SIGNALS) == SIG_LPC_POLES) && (s->buff_size)) {
    ppp = (POLE**)s->data;
    free_poles(ppp,s->buff_size);
    free(ppp);
    return(TRUE);
  }
  return(FALSE);
}
    
