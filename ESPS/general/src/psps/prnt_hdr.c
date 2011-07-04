/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1997  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Alan Parker, based on the SDS version by Joe Buck.
 * Checked by:
 * Revised by:  John Shore for ESPS 3.0
 *
 * Brief description:
 *	support routines for psps; these routines print headers
 */

static char *sccs_id = "%W%	%G%	ESI/ERL";

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>

void tab(), sort(), print_generic();
void	print_txtcomment();
void pr_full_header();
char *file_typeX();

int idx_ok();

extern vflag;
extern full;
extern Recursive;
extern eflag;

void
pr_zfunc(lev,label,zf)
int lev;
char *label;
struct zfunc *zf;
{
    int i;

    if(zf == NULL) {
        tab (lev);
	(void)printf("No %s zfunc\n",label);
	return;
    }
    tab (lev);
    (void)printf ("%s: nsiz: %d, dsiz: %d, { ", label,zf->nsiz,zf->dsiz);
    for (i = 0; i < zf -> nsiz; i++) (void)printf ("%.4g ", zf -> zeros[i]);
    (void)printf ("} / { ");
    for (i = 0; i < zf -> dsiz; i++) (void)printf ("%.4g ", zf -> poles[i]);
    (void)printf ("}\n");
}

void
tab(indent) {
    while (indent--) (void)printf(" ");
}

void
print_common(indent, hdr)
int indent;
struct header *hdr;
{
  int i;
  char *text = hdr->variable.typtxt;
  char *comment = hdr->variable.comment;
  char *refer = hdr->variable.refer;

  (void)printf("\n");
  tab(indent); 
  (void) printf("---Universal Part of Header---\n");
  tab(indent);
  (void) printf("File type: %s\n", file_typeX(hdr->common.type));
  tab(indent);
  (void) printf("File header date: %s, header version: %s\n",
	hdr->common.date, hdr->common.hdvers);
  tab(indent);
  (void) printf("Produced by: %s, version: %s, of: %s, ",
	hdr->common.prog, hdr->common.vers, hdr->common.progdate);
  (void) printf("user: %s\n",hdr->common.user);
  if (hdr->variable.current_path != NULL) {
    tab(indent);
    (void) printf("Hostname:path: %s\n",hdr->variable.current_path);
  }
  tab(indent);
  (void) printf("Machine type: %s; file is in %s format.\n",
	machine_codes[hdr->common.machine_code],hdr->common.edr?"EDR":"NATIVE");
  tab(indent);
  if (hdr->common.ndrec == -1)
    (void) printf("Number of data records: unknown (input is pipe or record size is variable)\n");
  else
    (void) printf("Number of data records: %d\n",hdr->common.ndrec);
  tab(indent);
  (void) printf("Data is %s tagged.\n", (hdr->common.tag ? "" : "not"));
  if(vflag) {
  	tab(indent);
  	(void) printf("Data format is %d doubles, %d floats, %d longs,\n",
		hdr->common.ndouble, hdr->common.nfloat, hdr->common.nlong);
        tab(indent);
        (void) printf("%d shorts, and %d chars (bytes).\n",hdr->common.nshort,
 		hdr->common.nchar);
  }
  if(text != NULL)  {
	tab(indent);
	(void)printf("typtxt:\n");
	print_txtcomment(text, indent + 1);
  }
  if(refer != NULL) {
	tab(indent);
	(void)printf("refer file: ");
	(void)fputs(refer,stdout);
	if(refer[strlen(refer)-1] != '\n')
		(void)putchar('\n');
  }
  if(comment != NULL) {
	tab(indent);
	(void)printf("comment text:\n");
	print_txtcomment(comment, indent + 1);
  }
  if(hdr->variable.source[0] != NULL) {
	int j=1;
	tab(indent);
	(void)printf("Source files: %s\n",hdr->variable.source[0]);
	while((hdr->variable.source[j] != NULL) && j < hdr->variable.nnames){
		tab(indent);
		(void)printf("              %s\n",hdr->variable.source[j++]);
	}
  }
  if(hdr->variable.refhd != NULL) {
	tab(indent);
	(void)printf("There is a reference header.\n");
  } else {
	tab(indent);
	(void)printf("There is no reference header.\n");
  }
  for (i=0; i<NSPARES; i++) 
	if (hdr->common.spares[i] != 0) {
	 tab(indent);
	 (void) printf("spares[%d]: 0x%x\n", i, (int) hdr->common.spares[i]);
	}
  (void)printf("\n");
  return;
}

void
print_generic(indent,hdr)
int indent;
struct header *hdr;
{
  char **names;
  int j,i,k=0,num,size,type;
  int strcmp ();
  int swap ();

  if(hdr->variable.ngen == 0) return;
  (void)printf("\n");
  tab(indent);
  (void) printf("---Generic Part of Header---");
  names = genhd_list(&num,hdr);
  
  /* sort the generic header names */
     sort (names, num, strcmp, swap);
 
  for (i=0; i<num; i++) {
    char *cptr; double *dptr; float *fptr; long *lptr; short *sptr;   
    FILE *Fptr; struct header *hptr;
    type = genhd_type(names[i],&size,hdr);
    (void)printf("\n");
    tab(indent);
    if (size > 7 && (type != EFILE && type != AFILE))
	(void)printf("%s[0]: %s%s%s ", names[i],vflag ? "(":"",
		vflag ? type_codes[type]:"", vflag ? ")":"");
    else
	(void)printf("%s: %s%s%s ", names[i], vflag ? "(":"",
		vflag ? type_codes[type]:"", vflag ? ")":"");
    k=0;
    cptr = get_genhd(names[i],hdr);
    if (type == CHAR && size > 1) {
	(void)printf("%s",cptr);
 	tab(indent);
    }
    else if ((type == AFILE || type == EFILE) && !eflag) {
      (void)printf("%s",cptr);
      tab(indent);
    }
    else if (type == AFILE && eflag) {
      if ((Fptr = get_genhd_afile(names[i],hdr)) == NULL) 
	(void)printf("Cannot get external file %s.\n",cptr);
       else {
	int c;
	(void)printf("External AFILE: %s\n",cptr);
	tab(indent+1);
	while((c=fgetc(Fptr)) != EOF) {
	  (void)putchar(c);
	  if(c == '\n') tab(indent+1);
	}
	(void)printf("---EOF on %s---\n",cptr);
	tab(indent+1);
      }
    }
    else if (type == EFILE && eflag) {
	if ((hptr = get_genhd_efile(names[i],hdr)) == NULL)
	  (void)printf(
           "cannot get external file %s, or it is not an ESPS file.\n", cptr);
	else {
	  (void)printf("External EFILE ");
	  pr_full_header (cptr, hptr, Recursive, indent+1);
	}
      }
    else
      for (j=0;j<size; j++) {
	k++;
	if(k>7) { (void)printf("\n"); 
                  tab(indent); 
                  (void)printf("%s[%d]: ",names[i],j); 
		  k=0; 
		}
	switch(type) {
	 case DOUBLE: 
    		dptr = get_genhd_d(names[i],hdr);
		(void)printf("%lg ",dptr[j]);
		break;
	 case FLOAT: 
    		fptr = get_genhd_f(names[i],hdr);
		(void)printf("%g ",fptr[j]);
		break;
	 case SHORT: 
    		sptr = get_genhd_s(names[i],hdr);
		(void)printf("%d ",sptr[j]);
		break;
	 case LONG: 
    		lptr = get_genhd_l(names[i],hdr);
		(void)printf("%ld ",lptr[j]);
		break;
	 case CHAR: 
    		cptr = get_genhd(names[i],hdr);
		(void)printf("0x%x ",cptr[j]);
		break;
	 case CODED:  {
		char **codes = genhd_codes(names[i],hdr);
    		sptr = get_genhd_s(names[i],hdr);
		if(idx_ok(sptr[j],codes))
		  (void)printf("%s ",codes[sptr[j]]);
		else
		  (void)printf("invalid code %d ",sptr[j]);
		break;
	        }
	}
       }
        if (vflag && type == CODED) {
          char **s;
          tab(indent);
          (void)printf("\n Possible coded values:");
          s = genhd_codes(names[i],hdr);
          while(*s != NULL) {
            tab(indent);
            (void)printf("\n %s",*s++);
          }
        }
        if (vflag && size > 1) {
          tab(indent);
	  (void)printf("\n Size: %d",size);
 	}
  }
  (void)printf("\n");
}

void
print_sd(indent, hdr)
int indent;
struct header *hdr;
{
  struct sd_header *h = hdr->hd.sd;
  int i;
  tab(indent);
  (void) printf("---Type Specific Part of Header (SD)---\n");
  tab(indent);
  if(idx_ok(h->equip,equip_codes))
	(void) printf("equip: %s\n",equip_codes[h->equip]);
  else
	(void) printf("equip: invalid code %d\n",(int)h->equip);
  tab(indent);
  (void) printf("max_value: %g, sf: %g, src_sf: %g\n",
	h->max_value, h->sf, h->src_sf);
  tab(indent);
  if(idx_ok(h->synt_method,synt_methods))
	(void) printf("synt_method: %s\n",synt_methods[h->synt_method]);
  else
	(void) printf("synt_method: invalid code %d\n",(int)h->synt_method);
  tab(indent);
  (void) printf("scale: %g, dcrem: %g\n",h->scale,h->dcrem);
  tab(indent);
  if(idx_ok(h->q_method,synt_methods))
	(void) printf("q_method: %s\n",quant_methods[h->q_method]);
  else
	(void) printf("q_method: invalid code %d\n",(int)h->q_method);
  tab(indent);
  if(idx_ok(h->v_excit_method,excit_methods))
	(void) printf("v_excit_method: %s\n",excit_methods[h->v_excit_method]);
  else
	(void) printf("v_excit_method: invalid code %d\n",(int)h->v_excit_method);
  tab(indent);
  if(idx_ok(h->uv_excit_method,excit_methods))
	(void) printf("uv_excit_method: %s\n",excit_methods[h->uv_excit_method]);
  else
	(void) printf("uv_excit_method: invalid code %d\n",(int)h->uv_excit_method);
  tab(indent);
  (void) printf("spare1: %d, nchan: %d\n",(int)h->spare1,(int)h->nchan);
  tab(indent);
  if(idx_ok(h->synt_interp,synt_inter_methods))
	(void) printf("synt_interp: %s\n",synt_inter_methods[h->synt_interp]);
  else
	(void) printf("synt_interp: invalid code %d\n",(int)h->synt_interp);
  tab(indent);
  if(idx_ok(h->synt_pwr,synt_inter_methods))
	(void) printf("synt_pwr: %s\n",synt_pwr_codes[h->synt_pwr]);
  else
	(void) printf("synt_pwr: invalid code %d\n",(int)h->synt_pwr);
  tab(indent);
  if(idx_ok(h->synt_rc,synt_inter_methods))
	(void) printf("synt_rc: %s\n",synt_ref_methods[h->synt_rc]);
  else
	(void) printf("synt_rc: invalid code %d\n",(int)h->synt_rc);
  tab(indent);
  (void) printf("synt_order: %d\n",(int)h->synt_order);
  tab(indent);
  (void) printf("start: %ld, nan: %ld\n",h->start,h->nan);
  for (i=0; i<SD_SPARES; i++) 
	if (h->spares[i] != 0) {
		tab(indent);
		(void) printf("spares[%d]: 0x%x\n", i,(int) h->spares[i]);
	}
  pr_zfunc(indent,"prefilter",h->prefilter);
  pr_zfunc(indent,"de_emp",h->de_emp);
  return;
}

void
print_spec(indent, hdr)
int indent;
struct header *hdr;
{
  int i;
  struct spec_header *h = hdr->hd.spec;
  tab(indent);
  (void) printf("---Type Specific Part of Header (SPEC)---\n");
  tab(indent);
  (void) printf("start: %ld, nan: %ld, frmlen: %d\n",
    h->start, h->nan, (int)h->frmlen);
  tab(indent);
  (void) printf("order_vcd: %d, order_unvcd: %d\n",
    (int)h->order_vcd, (int)h->order_unvcd);
  tab(indent);
  if(idx_ok(h->win_type,win_type_codes))
	(void) printf("win_type: %s\n",win_type_codes[h->win_type]);
  else
	(void) printf("win_type: invalid code %d\n",(int)h->win_type);
  tab(indent);
  (void) printf("sf: %g\n",h->sf);
  tab(indent);
  if(idx_ok(h->spec_an_meth,spec_an_methods))
	(void) printf("spec_an_meth: %s\n",spec_an_methods[h->spec_an_meth]);
  else
	(void) printf("spec_an_meth: invalid code %d\n",(int)h->spec_an_meth);
  tab(indent);
  (void) printf("dcrem: %g\n",h->dcrem);
  tab(indent);
  if(idx_ok(h->post_proc,post_proc_codes))
	(void) printf("post_proc: %s\n",post_proc_codes[h->post_proc]);
  else
	(void) printf("post_proc: invalid code %d\n",(int)h->post_proc);
  tab(indent);
  if(idx_ok(h->frame_meth,frame_methods))
	(void) printf("frame_meth: %s\n",frame_methods[h->frame_meth]);
  else
	(void) printf("frame_meth: invalid code %d\n",(int)h->frame_meth);
  tab(indent);
  (void) printf("voicing: %s\n", (h->voicing ? "YES" : "NO" ));
  tab(indent);
  if(idx_ok(h->freq_format,freq_format_codes))
	(void) printf("freq_format: %s\n",freq_format_codes[h->freq_format]);
  else
	(void) printf("freq_format: invalid code %d\n",(int)h->freq_format);
  tab(indent);
  if(idx_ok(h->spec_type,spec_type_codes))
	(void) printf("spec_type: %s\n",spec_type_codes[h->spec_type]);
  else
	(void) printf("spec_type: invalid code %d\n",(int)h->spec_type);
  tab(indent);
  (void) printf("contin: %s, num_freqs: %ld\n",
	    (h->contin ? "YES" : "NO" ), h->num_freqs);
  for (i=0; i<SPEC_SPARES; i++) 
	if (h->spares[i] != 0) {
		tab(indent);
		(void) printf("spares[%d]: 0x%x\n", i,(int) h->spares[i]);
	}
  pr_zfunc(indent,"pre_emp",h->pre_emp);
  if(h->freqs) {
	int j=6;
	for(i=0; i<h->num_freqs; i++) {
		if(j>5) {
			tab(indent);	
			(void)printf("freqs[%d]: ",i);
			j=0;
		}
		(void)printf("%10.4f",h->freqs[i]);
		j++;
	}
  }
  return;
}

void
print_filt(indent,hdr)
int indent;
struct header *hdr;
{
   int i,n;
   struct filt_header *h = hdr->hd.filt;
   tab(indent);
   (void) printf("---Type Specific Part of Header (FILT)---\n");
   tab(indent);
   (void) printf("max_num: %d, max_den: %d\n",h->max_num,h->max_den);
   tab(indent);
   if(idx_ok(h->func_spec,filt_func_spec))
	(void) printf("func_spec: %s\n",filt_func_spec[h->func_spec]);
   else
	(void) printf("func_spec: invalid code %d\n",h->func_spec);
   tab(indent);
   (void)printf("nbands: %d, npoints: %d, g_size: %d, nbits: %d\n",
	h->nbands,h->npoints,h->g_size,h->nbits);
   tab(indent);
   if(idx_ok(h->type,filt_type))
	(void) printf("type: %s\n",filt_type[h->type]);
   else
	(void) printf("type: invalid code %d\n",h->type);
   tab(indent);    
   if(idx_ok(h->method,filt_method))
	(void) printf("method: %s\n",filt_method[h->method]);
   else
	(void) printf("method: invalid code %d\n",h->method);
   for (i=0; i<FILT_SPARES; i++) 
	if (h->spares[i] != 0) {
		tab(indent);
		(void) printf("spares[%d]: 0x%x\n", i,(int) h->spares[i]);
	}
   
  if(h->nbands != 0) {
	int j=6;
	for(i=0; i<=h->nbands; i++) {
		if(j>5) {
			tab(indent);	
			(void) printf("bandedges[%d]:   ",i);
			j=0;
		}
		(void) printf("%g ",h->bandedges[i]);
		j++;
	}
	(void)printf("\n");
  }
  if(h->npoints != 0) {
	int j=6;
	for(i=0; i<h->npoints; i++) {
		if(j>5) {
			tab(indent);	
			(void) printf("points[%d]:      ",i);
			j=0;
		}
		(void) printf("%g ",h->points[i]);
		j++;
	}
	(void)printf("\n");
  }
  if(h->func_spec == BAND)
      n = h->nbands;
  else
      n = h->npoints;
  if(n != 0) {
	int j=6;
	for(i=0; i<n; i++) {
		if(j>5) {
			tab(indent);	
			(void) printf("gains[%d]:       ",i);
			j=0;
		}
		(void) printf("%g ",h->gains[i]);
		j++;
	}
	(void)printf("\n");
  }
  if(n != 0) {
	int j=6;
	for(i=0; i<n; i++) {
		if(j>5) {
			tab(indent);	
			(void) printf("wts[%d]:         ",i);
			j=0;
		}
		(void) printf("%g ",h->wts[i]);
		j++;
	}
	(void)printf("\n");
  }
  return;
}

void
print_scbk(indent,hdr)
int indent;
struct header *hdr;
{
   int i;
   struct scbk_header *h = hdr->hd.scbk;
   tab(indent);
   (void) printf("---Scaler Codebook File---\n");
   tab(indent);
   (void) printf("num_items: %ld, num_cdwds: %d\n",
	h->num_items,h->num_cdwds);
   tab(indent);
   (void) printf("convergence: %f, element_num: %d\n",
 	h->convergence,h->element_num);
   tab(indent);
   if(idx_ok(h->distortion,scbk_distortion))
	(void) printf("distortion: %s\n",scbk_distortion[h->distortion]);
   else
	(void) printf("distortion: invalid code %d\n",h->distortion);
   tab(indent);
   if(idx_ok(h->codebook_type,scbk_codebook_type))
	(void) printf("codebook_type: %s\n",scbk_codebook_type[h->codebook_type]);
   else
	(void) printf("codebook_type: invalid code %d\n",h->codebook_type);
   for (i=0; i<SCBK_SPARES; i++) 
	if (h->spares[i] != 0) {
		tab(indent);
		(void) printf("spares[%d]: 0x%x\n", i,(int) h->spares[i]);
	}
}

void
print_fea(indent,hdr)
int indent;
struct header *hdr;
{
   int i, j;
   struct fea_header *h = hdr->hd.fea;
   extern char	*fea_file_type[];
   tab(indent);
   (void) printf("---Type Specific Part of Header (FEA)---\n");
   tab(indent);
   if(idx_ok(h->fea_type,fea_file_type))
	(void) printf("fea_type: %s, ", fea_file_type[h->fea_type]);
   else
	(void) printf("fea_type: invalid code %d, ",h->fea_type);

   printf("segment_labeled: %s, field_count: %d\n",
		(h->segment_labeled ? "YES" : "NO" ), h->field_count);
#ifndef NOPAD
   if(h->field_order == YES && indent == 0)
	(void)printf("File is in field_order.\n");
#endif

 if (vflag)
   for (i=0; i<h->field_count; i++) {
     tab(indent);
     (void)printf("Item name: %s, ", h->names[i]);
     tab(indent);
     if(idx_ok(h->types[i],type_codes))
      (void)printf("type: %s, ",type_codes[h->types[i]]);
     else
      (void)printf("type: invalid code: %d, ",h->types[i]);
     tab(indent);
     (void)printf("size: %ld", h->sizes[i]);
     (void)printf(", rank: %d", h->ranks[i]);
     (void)printf("\n");
     if(h->ranks[i] >= 1) {
      tab(indent);
      (void)printf(" Dimensions: ");
      for(j=0; j<h->ranks[i]; j++)
	   (void)printf("% ld",h->dimens[i][j]);
      (void)printf("\n");
     }
     if (h->types[i] == CODED) {
      char **s;
      tab(indent);
      (void)printf(" Possible coded values:\n");
      s = h->enums[i];
      while(*s != NULL) {
        tab(indent);
	(void)printf(" %s\n",*s++);
      }
     }
     if(h->derived[i]) {
	char **s;
	tab(indent);
	(void)printf(" Derived from:\n");
	s = h->srcfields[i];
	while(*s != NULL) {
	  tab(indent);
	  (void)printf(" %s\n",*s++);
	}
      }

   } /* end for (i = 0; i < h->field_count; i++) */

 if (vflag)
   for (i=0; i<FEA_SPARES; i++) 
	if (h->spares[i] != 0) {
		tab(indent);
		(void) printf("spares[%d]: 0x%x\n", i,(int) h->spares[i]);
	}
}


void
sort (v, n, comp, exch)	    
char *v[];
int n;
int (*comp) (),  (*exch) ();
{
/* sort strings v[0]...v[n-1] 
 */
    int gap, i, j;

    for (gap = n/2; gap > 0; gap /= 2)
	for (i = gap; i < n; i++)
	    for (j = i-gap; j >= 0; j -= gap) {
		if ((*comp) (v[j], v[j+gap]) <= 0)
		    break;
		(*exch) (&v[j], &v[j+gap]);
	    }
}

swap (px, py)	/* interchange *px and *py */
char *px[], *py[];
{
    char *temp;

    temp = *px;
    *px = *py;
    *py = temp;
}


#define LINE_LENGTH 79

void
print_txtcomment(text, indent)
char *text;			/* text to print */
int  indent;			/* number of spaces to indent */
{
/* used to print out the text field and comment field in the
 * header; if any text exceeds the LINE_LENGTH limit, a line-feed
 * is put out and the indentation of the continuation is set to
 * an additional 2 spaces
 */
    int count = 0;		/* position in line */
    spsassert(text != NULL, "print_txtcomment: null text"); 
    while (*text != '\0') {
	if (count == 0) {
	    tab(indent);
	    (void) fputc(*text, stdout);
	    count = indent + 1;	    
	}
	else if (count < LINE_LENGTH) {
	    (void) fputc(*text, stdout);
	    count++;
	}
	else {
	    (void) putchar('\n');
	    tab(indent + 2);
	    (void) fputc(*text, stdout);
	    count = indent + 3;
	}
    if (*text == '\n') count = 0;
    text++;
    }
    if(text[strlen(text)-1] != '\n') (void)putchar('\n');
}

