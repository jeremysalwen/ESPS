/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1996 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Bill Byrne
 *   added conversion routine in get_feafilt_rec for older IIR filter
 *   file to reflect correct number of real pole/zero, Derek Lin
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)feafiltsupp.c	1.17	12/18/96	ERL";

/*
 * include files
 */

#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>
#if !defined(LINUX_OR_MAC)
#include <esps/unix.h>
#endif
#include <esps/feafilt.h>

char *savestring();
char **get_feafilt_xfields();
void free_feafilt_xfields();

/*
 * string array definitions.
 */
char *feafilt_type[] = {"NONE", "LP", "HP", "BP", "BS", "ARB", NULL};

char *feafilt_method[] = {"NONE", "PZ_PLACE", "PARKS_MC", "WMSE", 
			      "BUTTERWORTH", "BESSEL", "CHEBYSHEV1",
			      "CHEBYSHEV2", "ELLIPTICAL", "CONST_BASED",
			      "WINDOW_METH", NULL};

char *feafilt_func_spec[] = {"NONE", "BAND", "POINT", "IIR", NULL};

char *feafilt_yesno[] = {"NO", "YES", NULL};

/*
 * This function fills in the header of a FEA file to make it a
 * file of subtype FILT.
 */

int
init_feafilt_hd(hd, max_num, max_denom, fdp)
     struct header   *hd;
     long max_num; 
     long max_denom;
     filtdesparams   *fdp;
{
    float *fp1, *fp2;
    int k;
    long dim;

    spsassert(hd != NULL, "init_feafilt_hd: hd is NULL");
    spsassert(hd->common.type == FT_FEA,
		"init_feafilt_hd: called on non fea header");
    spsassert( max_num > 0 || max_denom > 0, 
	      "init_feafilt_hd: either max_num or max_denom must be greater than zero.\n");
    spsassert( max_num >= 0 && max_denom >= 0, 
	      "init_feafilt_hd: max_num and max_denom must not be negative.\n");

    /*
     * first, put in the scalar generic header items
     */

    if ( fdp == NULL ) {
      fdp = (filtdesparams *) calloc(1, sizeof(filtdesparams));
      spsassert( fdp != NULL, "init_feafilt_hd: calloc failed.");
    }

    *add_genhd_l("max_num", (long *) NULL, 1, hd) = max_num;
    *add_genhd_l("max_denom", (long *) NULL, 1, hd) = max_denom;
    *add_genhd_e("filter_complex", (short *) NULL, 1,  feafilt_yesno, hd) = fdp->filter_complex;
    *add_genhd_e("define_pz", (short *) NULL, 1,  feafilt_yesno, hd) = fdp->define_pz;
    *add_genhd_l("g_size", (long *) NULL, 1, hd) = fdp->g_size;
    *add_genhd_l("nbits", (long *) NULL, 1, hd) = fdp->nbits;
    *add_genhd_l("nbands", (long *) NULL, 1, hd) = fdp->nbands;
    *add_genhd_l("npoints", (long *) NULL, 1, hd) = fdp->npoints;
    *add_genhd_e("type", (short *) NULL, 1,  feafilt_type, hd) = fdp->type;
    *add_genhd_e("method", (short *) NULL, 1, feafilt_method, hd) = fdp->method;
    *add_genhd_e("func_spec", (short *) NULL, 1, feafilt_func_spec, hd) = fdp->func_spec;

    /*
     * put in the vector generic header items
      */
    if ( fdp->nbands > 0 ) {
	fp1 = (float *) add_genhd_f("bandedges", (float *)NULL, (int)(1+fdp->nbands), hd);
	for ( k=0; k<(1+fdp->nbands); k++ ) 
	  fp1[k] = fdp->bandedges[k];
      } else {
	*add_genhd_f("bandedges", (float *)NULL, 1, hd) = 0.0;
      }

    dim = 0;
    if ( fdp->func_spec == BAND ) 
	dim = fdp->nbands;
    if ( fdp->func_spec == POINT ) 
	dim = fdp->npoints;

    if ( dim > 0 ) {
	fp1 = (float *) add_genhd_f("gains", (float *)NULL, dim, hd);
	fp2 = (float *) add_genhd_f("wts", (float *)NULL, dim, hd);
	for (k=0; k<dim; k++) {
	    fp1[k] = fdp->gains[k];
	    fp2[k] = fdp->wts[k];
	  } 
      } else {
	  *add_genhd_f("gains", (float *)NULL, 1, hd) = 0.0;
	  *add_genhd_f("wts", (float *)NULL, 1, hd) = 0.0;
	}

    /*
     * Now define the record fields:
     */
    add_fea_fld("num_size", (long)1, (short)0, (long *)NULL, LONG, (char **)NULL, hd);
    add_fea_fld("denom_size", (long)1, (short)0, (long *)NULL, LONG, (char **)NULL, hd);

    if (max_num > 0) {
      add_fea_fld("re_num_coeff", max_num, (short)1, (long *)NULL, DOUBLE, (char **)NULL, hd);
      if (fdp->filter_complex)
	add_fea_fld("im_num_coeff", max_num, (short)1, (long *)NULL, DOUBLE, (char **)NULL, hd);
    }
    
    if (max_denom > 0) {
	add_fea_fld("re_denom_coeff", max_denom, (short)1, (long *)NULL, DOUBLE, (char **)NULL, hd);
	if (fdp->filter_complex)
	  add_fea_fld("im_denom_coeff", max_denom, (short)1, (long *)NULL, DOUBLE, (char **)NULL, hd);
      }

    if (fdp->define_pz) {
	add_fea_fld("pole_dim", (long)1, (short)0, (long *)NULL, LONG, (char **)NULL, hd);
	add_fea_fld("zero_dim", (long)1, (short)0, (long *)NULL, LONG, (char **)NULL, hd);	    
	if (max_num) 
	    add_fea_fld("zeros", max_num, (short)1, (long *)NULL, DOUBLE_CPLX, (char **)NULL, hd);
  	if (max_denom) 
	  add_fea_fld("poles", max_denom, (short)1, (long *)NULL, DOUBLE_CPLX, (char **)NULL, hd);
      }

    hd->hd.fea->fea_type = FEA_FILT;
    return 0;
}

/*
 * This function allocates a record for the FEA file subtype FILT
 */

struct feafilt *
allo_feafilt_rec(hd)
     struct header *hd;
{
  struct fea_data *fea_rec;
  struct feafilt  *feafilt_rec;

  spsassert(hd, "allo_feafilt_rec: given a NULL hd");
  spsassert((hd->common.type == FT_FEA) && (hd->hd.fea->fea_type == FEA_FILT),
	    "allo_feafilt_rec: called on non fea_filt header");

  feafilt_rec = (struct feafilt *) calloc(1, sizeof(struct feafilt));
  spsassert(feafilt_rec, "allo_feafilt_rec: calloc failed!");
  feafilt_rec->fea_rec = fea_rec = allo_fea_rec(hd);

  /* if fields im_num_coeff, im_denom_coeff, poles, zeros are not defined,
     feafilt_rec pointers are set to NULL */

  feafilt_rec->num_size = (long *) get_fea_ptr(fea_rec, "num_size", hd);
  feafilt_rec->denom_size = (long *) get_fea_ptr(fea_rec, "denom_size", hd);

  feafilt_rec->re_num_coeff = (double *) get_fea_ptr(fea_rec, "re_num_coeff", hd);
  feafilt_rec->im_num_coeff = (double *) get_fea_ptr(fea_rec, "im_num_coeff", hd);

  feafilt_rec->re_denom_coeff = (double *) get_fea_ptr(fea_rec, "re_denom_coeff", hd);
  feafilt_rec->im_denom_coeff = (double *) get_fea_ptr(fea_rec, "im_denom_coeff", hd);

  feafilt_rec->zero_dim = (long *) get_fea_ptr(fea_rec, "zero_dim", hd);
  feafilt_rec->pole_dim = (long *) get_fea_ptr(fea_rec, "pole_dim", hd);
  feafilt_rec->zeros = (double_cplx *) get_fea_ptr(fea_rec, "zeros", hd);
  feafilt_rec->poles = (double_cplx *) get_fea_ptr(fea_rec, "poles", hd);

  return feafilt_rec;
}

/*
 * This routine writes a record of the FEA file subtype FILT
 */

void
put_feafilt_rec(feafilt_rec, hd, file)
    struct feafilt  *feafilt_rec;
    struct header   *hd;
    FILE            *file;
{
    spsassert(hd && file && (hd->common.type == FT_FEA)
		&& (hd->hd.fea->fea_type == FEA_FILT),
		"put_feafilt_rec: called on non fea_filt file");

    put_fea_rec(feafilt_rec->fea_rec, hd, file);
}

/*
 * This routine reads a record of the FEA file subtype FILT
 */

#define PRE_IIR 1.87            /* file version of old IIR filter files */

int
get_feafilt_rec(feafilt_rec, hd, file)
    struct feafilt  *feafilt_rec;
    struct header   *hd;
    FILE            *file;
{
  int i, status;
  spsassert(hd && file && (hd->common.type == FT_FEA)
	    && (hd->hd.fea->fea_type == FEA_FILT),
	    "get_feafilt_rec: called on non fea_filt file");

  status = get_fea_rec(feafilt_rec->fea_rec, hd, file);

  /* IIR filters old than ver. 1.87 don't produce the right number of
     real poles and zeros -- bellow converts it */
  if( atof(hd->common.hdvers) <= PRE_IIR ){
    int mt, func, type, num, numc, chpole= -1;
    double_cplx tmp[20];             /* have to declare memory statically,
					runtime malloc trashes good memory.*/
    func = *get_genhd_s("func_spec", hd);
      if(func == IIR){
	mt = *get_genhd_s("method", hd);
	if( mt == BUTTERWORTH || mt == CHEBYSHEV1 ){
	  type = *get_genhd_s("type", hd);
	  num = *feafilt_rec->zero_dim;
	  numc = 2*num;

	  switch(type){

	  case FILT_HP:
	  case FILT_LP:               /* double number of zeros at -1,0*/
	    *feafilt_rec->zero_dim = numc;
	    feafilt_rec->zeros = (double_cplx *) 
	      realloc( (void *) feafilt_rec->zeros,numc *sizeof(double_cplx));
	    for( i=num; i<numc; i++){
	      if( type == FILT_LP) feafilt_rec->zeros[i].real = -1;
	      else feafilt_rec->zeros[i].real = 1;
	      feafilt_rec->zeros[i].imag = 0;
	    }
	    break;

	  case FILT_BS:
	  case FILT_BP:
	    if( *feafilt_rec->pole_dim > 20 ){
	      Fprintf(stderr,"ERROR: Redesign the Bandpass or Bandstop IIR filter using iir_filt*.\n\tThis file of version <= 1.87 can't be converted. -- exiting.\n");
	      exit(1);
	    }
	    if(type == FILT_BP){         /* double zeros for bandpass at +-1 */
	      *feafilt_rec->zero_dim = numc;
	      feafilt_rec->zeros = (double_cplx *) 
	      realloc( (void *) feafilt_rec->zeros,numc *sizeof(double_cplx));
	      for(i=0; i<num; i++){
		feafilt_rec->zeros[i].real = 1;
		feafilt_rec->zeros[i].imag = 0;
	      }
	      for( i=num; i<numc; i++){
		feafilt_rec->zeros[i].real = -1;
		feafilt_rec->zeros[i].imag = 0;
	      }
	    }
	    /* Next 12 lines do nothing to change FILT_BS zeros, but
	       Need these 'magic' lines, or run time error for FILT_BS,
	       IIR, order 3, wideband 1000-3000 */
	    if(type == FILT_BS){
	      for(i=0;i<*feafilt_rec->zero_dim; i++){
		tmp[i].real = feafilt_rec->zeros[i].real;
		tmp[i].imag = feafilt_rec->zeros[i].imag;
	      }
	      feafilt_rec->zeros = (double_cplx *) 
	      realloc( (void *) feafilt_rec->zeros,numc *sizeof(double_cplx));
	      for(i=0; i<*feafilt_rec->zero_dim; i++){
		feafilt_rec->zeros[i].real = tmp[i].real;
		feafilt_rec->zeros[i].imag = tmp[i].imag;
	      }
	    }	      
		
	    /* A wideband filter, odd order. See7.25, 7.23 Parks & Burrus */
	    for(i=0; i<*feafilt_rec->pole_dim; i++)
 	      if( feafilt_rec->poles[i].imag == 0) chpole = i;
	    if(chpole >= 0){
	      num = *feafilt_rec->pole_dim;
	      *feafilt_rec->pole_dim = numc = num+1;
	      
	      for(i=0;i<num;i++){    /* need this awkward tmp, due 
   					   to realloc distroys data */
		tmp[i].real = feafilt_rec->poles[i].real;
		tmp[i].imag = feafilt_rec->poles[i].imag;
	      }
	      feafilt_rec->poles = (double_cplx *) 
		realloc((void *)feafilt_rec->poles,numc *sizeof(double_cplx));
	      spsassert(feafilt_rec->poles, 
			"get_feafilt_rec: feafilt_rec->poles malloc fails");
	      
	      for(i=0;i<num;i++){
		feafilt_rec->poles[i].real = tmp[i].real;
		feafilt_rec->poles[i].imag = tmp[i].imag;
	      }
	      feafilt_rec->poles[numc-1].real= tmp[chpole].real;
	      feafilt_rec->poles[numc-1].imag = 0;
	    }
	    break;
	    
	  defaults:
	    break;
	  }
	}
      }
  }
  return(status);
  
}

struct zfunc
feafilt_to_zfunc( feafiltrec ) 
struct feafilt *feafiltrec;
{
  struct zfunc res;
  int k;

  spsassert( feafiltrec != NULL, "feafiltrec_to_zfunc: passed null feafilt rec.");

  res.nsiz = *feafiltrec->num_size;
  res.dsiz = *feafiltrec->denom_size;

  if ( res.nsiz ) {
      res.zeros = (float *) calloc( res.nsiz, sizeof(float));
      spsassert( res.zeros != NULL, "feafiltrec_to_zfunc: can't allocate zeros.");
    }
  for (k=0; k<res.nsiz; k++)
    res.zeros[k] = feafiltrec->re_num_coeff[k];

  if (res.dsiz) {
      res.poles = (float *) calloc( res.dsiz, sizeof(float));
      spsassert( res.poles != NULL, "feafiltrec_to_zfunc: can't allocate poles.");
    }
  for (k=0; k<res.dsiz; k++)
    res.poles[k] = feafiltrec->re_denom_coeff[k];

  return( res );
}


void
print_feafilt_rec(p, hd, file)
struct feafilt *p;
struct header *hd;
FILE *file;
{
	short *filter_complex, *define_pz, j, i;
	long max_num, max_denom;
	char	**xfields;

	spsassert(p, "print_feafilt_rec: p is NULL");
	spsassert(hd, "print_feafilt_rec: hd is NULL");
	spsassert(file, "print_feafilt_rec: file is NULL");

	max_num = get_genhd_val("max_num", hd, -1.0);
	max_denom = get_genhd_val("max_denom", hd, -1.0);
	filter_complex = get_genhd_s("filter_complex", hd);
	define_pz = get_genhd_s("define_pz", hd);

	if (!filter_complex) {
		fprintf(file, "Warning: header item filter_complex is missing.\n");
		filter_complex = (short *)calloc(1,sizeof(short));
	}
	
	if (!define_pz) {
		fprintf(file, "Warning: header item define_pz is missing.\n");
		define_pz = (short *)calloc(1,sizeof(short));
	}
	
	if (max_num == -1) {
		fprintf(file, "Warning: header item max_num is missing.\n");
		max_num = 0;
	}
	if (max_denom == -1) {
		fprintf(file, "Warning: header item max_denom is missing.\n");
		max_denom = 0;
	}

	spsassert(p->num_size,"field num_size missing.");
	spsassert(p->denom_size,"field denom_size missing.");
	if(*define_pz) {
		spsassert(p->pole_dim,
			"field pole_dim missing w/ define_pz true");
		spsassert(p->zero_dim,
			"field zero_dim missing w/ define_pz true");
	}
	
	fprintf(file, "num_size: %d, denom_size: %d",
		*(p->num_size), *(p->denom_size));

	if (*define_pz) 
	  fprintf(file, ", zero_dim: %d, pole_dim: %d\n",
		*(p->zero_dim), *(p->pole_dim));
	else
	  fprintf(file, "\n");

	if (*(p->num_size)) {
	  fprintf(file, "\nre_num_coeff:\n  0: ");
	  j=1;
	  for(i=0; i<*(p->num_size); i++) {
	  	fprintf(file, "%10.5lg ",p->re_num_coeff[i]);
		if(j++ == 5 && i+1<*(p->num_size)) {
		  fprintf(file,"\n%3d: ",i+1);
		  j=1;
	        }
	  }
	  fprintf(file,"\n");
	}

	if (*filter_complex && *(p->num_size)) {
	  fprintf(file, "\nim_num_coeff:\n  0: ");
	  j=1;
	  for(i=0; i<*(p->num_size); i++) {
	  	fprintf(file, "%10.5lg ",p->im_num_coeff[i]);
		if(j++ == 5 && i+1<*(p->num_size)) {
		  fprintf(file,"\n%3d: ",i+1);
		  j=1;
	        }
	  }
	  fprintf(file,"\n");
	}

	if (*(p->denom_size)) {
	  fprintf(file, "\nre_denom_coeff:\n  0: ");
	  j=1;
	  for(i=0; i<*(p->denom_size); i++) {
	  	fprintf(file, "%10.5lg ",p->re_denom_coeff[i]);
		if(j++ == 5 && i+1<*(p->denom_size)) {
		  fprintf(file,"\n%3d: ",i+1);
		  j=1;
	        }
	  }
	  fprintf(file,"\n");
	}

	if (*filter_complex && *(p->denom_size)) {
	  fprintf(file, "\nim_denom_coeff:\n  0: ");
	  j=1;
	  for(i=0; i<*(p->denom_size); i++) {
	  	fprintf(file, "%10.5lg ",p->im_denom_coeff[i]);
		if(j++ == 5 && i+1<*(p->denom_size)) {
		  fprintf(file,"\n%3d: ",i+1);
		  j=1;
	        }
	  }
	  fprintf(file,"\n");
	}

	if (*define_pz && *(p->zero_dim)) {
	  fprintf(file, "\nzeros:\n  0: ");
	  j=1;
	  for(i=0; i<*(p->zero_dim); i++) {
	  	fprintf(file, "[%10.5lg,%10.5g] ",p->zeros[i].real,p->zeros[i].imag);
		if(j++ == 2 && i+1<*(p->zero_dim)) {
		  fprintf(file,"\n%3d: ",i+1);
		  j=1;
	        }
	  }
	  fprintf(file,"\n");
	}

	if (*define_pz && *(p->pole_dim)) {
	  fprintf(file, "\npoles:\n  0: ");
	  j=1;
	  for(i=0; i<*(p->pole_dim); i++) {
	  	fprintf(file, "[%10.5lg,%10.5g] ",p->poles[i].real,p->poles[i].imag);
		if(j++ == 2 && i+1<*(p->pole_dim)) {
		  fprintf(file,"\n%3d: ",i+1);
		  j=1;
	        }
	  }
	  fprintf(file,"\n");
	}
/* print any fields that are not part of the standard fea_filt file here */

	xfields = get_feafilt_xfields(hd);
	print_fea_recf(p->fea_rec, hd, file, xfields);
	free_feafilt_xfields(xfields);

}

	

static
char *standard_fields[] = {"num_size", "denom_size", "zero_dim",
	"pole_dim", "re_num_coeff", "im_num_coeff", "re_denom_coeff",
	"im_denom_coeff", "zeros", "poles", NULL};


char **
get_feafilt_xfields(hd)
struct header *hd;
{

char **list;
int  list_size = 1;
long i,j;

	spsassert(hd, "get_feafilt_xfields: hd is NULL");
	spsassert(hd->common.type == FT_FEA && hd->hd.fea->fea_type == FEA_FILT,
		"get_feafilt_xfields: hd is not FEA or not FEA_FILT");
	list = (char **)malloc(sizeof(char *));
	list[0] = NULL;

	for (i=0; hd->hd.fea->names[i] != NULL; i++) {
		for (j=0; standard_fields[j] != NULL; j++) {
		   if (strcmp(hd->hd.fea->names[i], standard_fields[j]) == 0)
			break;
		}
		if (standard_fields[j] == NULL) {
		   list = (char **)realloc((char *)list, 
			(list_size+1)*sizeof(char *));
		   list[list_size-1] = savestring(hd->hd.fea->names[i]);
		   list[list_size++] = NULL;
		}
	}
	return list;
}

void
free_feafilt_xfields(list)
    char    **list;
{
    int	    i;

    for (i = 0; list[i]; i++)
	free(list[i]);
    free((char *) list);
}

