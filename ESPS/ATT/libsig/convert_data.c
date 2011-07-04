/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*	Copyright (c) 1987, 1988 AT&T	*/
/*	  and Entropic Speech, Inc.	*/
/*	  All Rights Reserved.	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	AND ENTROPIC SPEECH, INC.				*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/

/* convert_data.c */
/* convert signals from one internal C type to another */

#ifndef lint
static char *sccs_id = "@(#)convert_data.c	1.8	9/26/95	ATT/ERL";
#endif

#include <Objects.h>

#define round(x) ( ((dtmp = (x)) >= 0.0)? (dtmp+0.5) : (dtmp-0.5) )

Signal *new_signal(), *convert();

Signal *
d_to_c(s)
     Signal *s;
{
  Signal *s2;
  char **data, *p;
  double *q, **dpp, dtmp;
  int i, j, k;
  Header *h;

  if((s2 = new_signal(s->name,SIG_UNKNOWN,s->header,NULL,s->buff_size,
		      s->freq,s->dim))) {
    if((data = (char**)malloc(sizeof(char*) * s->dim))) {
      dpp = (double**)s->data;
      for(i=0 ; i < s->dim; i++)
	if((data[i] = (char*)malloc(sizeof(char) * s->buff_size))) {
	  for(j=0, p = data[i], q = dpp[i], k = s->buff_size;
	      j++ < k;)
	    *p++ = round(*q++);
	} else {
	  printf("Can't allocate array in d_to_c\n");
	  return(NULL);
	}
      s2->data = (caddr_t)data;
      s2->type = (s->type & ~VECTOR_SIGNALS) | P_CHARS;
      s2->start_time = BUF_START_TIME(s2);
      s2->start_samp = 0;
      s2->file_size = s2->buff_size;
      s2->version += 1;
      if(convert_header(s,s2,"double","char"))
	 return(s2);
    } else
	  printf("Can't allocate pointer array in d_to_c\n");
  } else
    printf("Can't create new_signal in d_to_c\n");
  return(NULL);
}


Signal *
d_to_s(s)
     Signal *s;
{
  Signal *s2;
  short **data, *p;
  double *q, **dpp, dtmp;
  int i, j, k;
  Header *h;
  
  if((s2 = new_signal(s->name,SIG_UNKNOWN,s->header,NULL,s->buff_size,
		      s->freq,s->dim))) {
    if((data = (short**)malloc(sizeof(short*) * s->dim))) {
      dpp = (double**)s->data;
      for(i=0 ; i < s->dim; i++)
	if((data[i] = (short*)malloc(sizeof(short) * s->buff_size))) {
	  for(j=0, p = data[i], q = dpp[i], k = s->buff_size;
	      j++ < k;)
	    *p++ = round(*q++);
	} else {
	  printf("Can't allocate array in d_to_s\n");
	  return(NULL);
	}
      s2->data = (caddr_t)data;
      s2->type = (s->type & ~VECTOR_SIGNALS) | P_SHORTS;
      s2->start_time = BUF_START_TIME(s2);
      s2->start_samp = 0;
      s2->file_size = s2->buff_size;
      s2->version += 1;
      if(convert_header(s,s2,"double","short"))
	 return(s2);
    } else
      printf("Can't allocate pointer array in d_to_s\n");
  } else
    printf("Can't create new_signal in d_to_s\n");
  return(NULL);
}


Signal *
d_to_i(s)
     Signal *s;
{
  Signal *s2;
  int **data, *p;
  double *q, **dpp, dtmp;
  int i, j, k, junk = 0;
  Header *h;

  if((s2 = new_signal(s->name,SIG_UNKNOWN,s->header,NULL,s->buff_size,
		      s->freq,s->dim))) {
    if((data = (int**)malloc(sizeof(int*) * s->dim))) {
      dpp = (double**)s->data;
      for(i=0 ; i < s->dim; i++)
	if((data[i] = (int*)malloc(sizeof(int) * s->buff_size))) {
	  for(j=0, p = data[i], q = dpp[i], k = s->buff_size;
	      j++ < k;)
	    *p++ = round(*q++);
	} else {
	  printf("Can't allocate array in d_to_i\n");
	  return(NULL);
	}
      s2->data = (caddr_t)data;
      s2->type = (s->type & ~VECTOR_SIGNALS) | P_INTS;
      s2->start_time = BUF_START_TIME(s2);
      s2->start_samp = 0;
      s2->file_size = s2->buff_size;
      s2->version += 1;
      if(convert_header(s,s2,"double","int"))
	 return(s2);
    } else
      printf("Can't allocate pointer array in d_to_i\n");
  } else
    printf("Can't create new_signal in d_to_i\n");
  return(NULL);
}


convert_header(s,s2,from,to)
     Signal *s, *s2;
     char *from;
{
  char temp[200];
  Header *h, *dup_header();

  if(!s->header) {
    s2->header = NULL;
    return(TRUE);
  }

  if((h = dup_header(s->header))) {
    head_printf(h,"version",&(s2->version));
    sprintf(temp,"convert: from %s to %s",from,to);
    head_printf(h,"operation",temp);
    if(s->start_time != s2->start_time)
      head_printf(h,"start_time",&(s2->start_time));
    if(s->file_size != s2->file_size)
      head_printf(h,"samples",&(s2->buff_size));
    head_printf(h,"type_code",&(s2->type));
    s2->header = h;
    return(TRUE);
  } else
    printf("Can't dup_header in \"from %s to %s\"",from,to);
  return (FALSE);
}


Signal *
d_to_f(s)
     Signal *s;
{
  Signal *s2;
  float **data, *p;
  double *q, **dpp;
  int i, j, k;
  Header *h;

  if((s2 = new_signal(s->name,SIG_UNKNOWN,s->header,NULL,s->buff_size,
		      s->freq,s->dim))) {
    if((data = (float**)malloc(sizeof(float*) * s->dim))) {
      dpp = (double**)s->data;
      for(i=0 ; i < s->dim; i++)
	if((data[i] = (float*)malloc(sizeof(float) * s->buff_size))) {
	  for(j=0, p = data[i], q = dpp[i], k = s->buff_size;
	      j++ < k;)
	    *p++ = *q++;
	} else {
	  printf("Can't allocate array in d_to_f\n");
          return(NULL);
        }
      s2->data = (caddr_t)data;
      s2->type = (s->type & ~VECTOR_SIGNALS) | P_FLOATS;
      s2->start_time = BUF_START_TIME(s2);
      s2->start_samp = 0;
      s2->file_size = s2->buff_size;
      s2->version += 1;
      if(convert_header(s,s2,"double","float"))
	return(s2);
    } else
	  printf("Can't allocate pointer array in d_to_f\n");
  } else
    printf("Can't create new_signal in d_to_f\n");
  return(NULL);
}


Signal *
c_to_d(s)
     Signal *s;
{
  Signal *s2;
  char  *q, **spp;
  double **data, *p;
  int i, j, k;
  Header *h;

  if((s2 = new_signal(s->name,SIG_UNKNOWN,s->header,NULL,s->buff_size,
		      s->freq,s->dim))) {
    if((data = (double**)malloc(sizeof(double*) * s->dim))) {
      spp = (char**)s->data;
      for(i=0 ; i < s->dim; i++)
	if((data[i] = (double*)malloc(sizeof(double) * s->buff_size))) {
	  for(j=0, p = data[i], q = spp[i], k = s->buff_size;
	      j++ < k;)
	    *p++ =  *q++;
	} else {
	  printf("Can't allocate array in c_to_d\n");
          return(NULL);
        }
          s2->data = (caddr_t)data;
	  s2->type = (s->type & ~VECTOR_SIGNALS) | P_DOUBLES;
	  s2->start_time = BUF_START_TIME(s2);
	  s2->start_samp = 0;
	  s2->file_size = s2->buff_size;
	  s2->version += 1;
      if(convert_header(s,s2,"char","double"))
	 return(s2);
    } else
	  printf("Can't allocate pointer array in c_to_d\n");
  } else
    printf("Can't create new_signal in c_to_d\n");
  return(NULL);
}

Signal *
s_to_d(s)
     Signal *s;
{
  Signal *s2;
  short  *q, **spp;
  double **data, *p;
  int i, j, k;
  Header *h;

  if((s2 = new_signal(s->name,SIG_UNKNOWN,s->header,NULL,s->buff_size,
		      s->freq,s->dim))) {
    if((data = (double**)malloc(sizeof(double*) * s->dim))) {
      spp = (short**)s->data;
      for(i=0 ; i < s->dim; i++)
	if((data[i] = (double*)malloc(sizeof(double) * s->buff_size))) {
	  for(j=0, p = data[i], q = spp[i], k = s->buff_size;
	      j++ < k;)
	    *p++ =  *q++;
	} else {
	  printf("Can't allocate array in s_to_d\n");
          return(NULL);
        }
          s2->data = (caddr_t)data;
	  s2->type = (s->type & ~VECTOR_SIGNALS) | P_DOUBLES;
	  s2->start_time = BUF_START_TIME(s2);
	  s2->start_samp = 0;
	  s2->file_size = s2->buff_size;
	  s2->version += 1;
      if(convert_header(s,s2,"short","double"))
	 return(s2);
    } else
	  printf("Can't allocate pointer array in s_to_d\n");
  } else
    printf("Can't create new_signal in s_to_d\n");
  return(NULL);
}

Signal *
i_to_d(s)
     Signal *s;
{
  Signal *s2;
  int  *q, **spp;
  double **data, *p;
  int i, j, k;
  Header *h;

  if((s2 = new_signal(s->name,SIG_UNKNOWN,s->header,NULL,s->buff_size,
		      s->freq,s->dim))) {
    if((data = (double**)malloc(sizeof(double*) * s->dim))) {
      spp = (int**)s->data;
      for(i=0 ; i < s->dim; i++)
	if((data[i] = (double*)malloc(sizeof(double) * s->buff_size))) {
	  for(j=0, p = data[i], q = spp[i], k = s->buff_size;
	      j++ < k;)
	    *p++ =  *q++;
	} else {
	  printf("Can't allocate array in i_to_d\n");
          return(NULL);
        }
          s2->data = (caddr_t)data;
	  s2->type = (s->type & ~VECTOR_SIGNALS) | P_DOUBLES;
	  s2->start_time = BUF_START_TIME(s2);
	  s2->start_samp = 0;
	  s2->file_size = s2->buff_size;
	  s2->version += 1;
      if(convert_header(s,s2,"int","double"))
	 return(s2);
    } else
	  printf("Can't allocate pointer array in i_to_d\n");
  } else
    printf("Can't create new_signal in i_to_d\n");
  return(NULL);
}

Signal *
f_to_d(s)
     Signal *s;
{
  Signal *s2;
  float  *q, **spp;
  double **data, *p;
  int i, j, k;
  Header *h;

  if((s2 = new_signal(s->name,SIG_UNKNOWN,s->header,NULL,s->buff_size,
		      s->freq,s->dim))) {
    if((data = (double**)malloc(sizeof(double*) * s->dim))) {
      spp = (float**)s->data;
      for(i=0 ; i < s->dim; i++)
	if((data[i] = (double*)malloc(sizeof(double) * s->buff_size))) {
	  for(j=0, p = data[i], q = spp[i], k = s->buff_size;
	      j++ < k;)
	    *p++ =  *q++;
	} else {
	  printf("Can't allocate array in f_to_d\n");
          return(NULL);
        }
          s2->data = (caddr_t)data;
	  s2->type = (s->type & ~VECTOR_SIGNALS) | P_DOUBLES;
	  s2->start_time = BUF_START_TIME(s2);
	  s2->start_samp = 0;
	  s2->file_size = s2->buff_size;
	  s2->version += 1;
      if(convert_header(s,s2,"float","double"))
	 return(s2);
    } else
	  printf("Can't allocate pointer array in f_to_d\n");
  } else
    printf("Can't create new_signal in f_to_d\n");
  return(NULL);
}

Signal *
convert(s,type)
     Signal *s;
     int type;
{
  Signal *s2, *s3;
  int i,j,n;

  if((type & ~SIG_FORMAT) == (s->type & ~SIG_FORMAT)) return(s);
  if(IS_GENERIC(type)) {
    if(s){
      if((type & VECTOR_SIGNALS) == (s->type & VECTOR_SIGNALS)) return(s);
      s2 = NULL;
      switch(s->type & VECTOR_SIGNALS) {
      case P_UCHARS:
      case P_CHARS:
	s2 = c_to_d(s);
	break;
      case P_SHORTS:
	s2 = s_to_d(s);
	break;
      case P_INTS:
	s2 = i_to_d(s);
	break;
      case P_FLOATS:
	s2 = f_to_d(s);
	break;
      case P_DOUBLES:
	s2 = s;
	break;
      default:
	printf("Unknown type %d for input in convert()\n",type);
	return(NULL);
      }
      if(s2) {
	switch(type & VECTOR_SIGNALS) {
	case P_UCHARS:
	case P_CHARS:
	  s3 = d_to_c(s2);
	  if(s != s2) free_signal(s2);
	  return(s3);
	case P_SHORTS:
	  s3 = d_to_s(s2);
	  if(s != s2) free_signal(s2);
	  return(s3);
	case P_INTS:
	  s3 = d_to_i(s2);
	  if(s != s2) free_signal(s2);
	  return(s3);
	case P_FLOATS:
	  s3 = d_to_f(s2);
	  if(s != s2) free_signal(s2);
	  return(s3);
	case P_DOUBLES:
	  return(s2);
	default:
	  printf("Unknown output type %d in convert()\n",s->type);
	  return(NULL);
	}
      } else
	printf("Conversion to intermediate doubles failed in convert()\n");
    } else
      printf("Null Signal structure on input\n");
  } else
    printf("Signal (%d) is not a VECTOR_SIGNAL; can't convert to type %d\n",
	   s->type, type);
  return(NULL);
}
