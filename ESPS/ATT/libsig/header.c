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
 * Brief description:  Create, accesss, read, and write file headers.
 */

static char *sccs_id = "@(#)header.c	1.59	1/18/97	ATT/ESI/ERL";

#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feaspec.h>
#include <sys/types.h>
#include <unistd.h>
#include <Objects.h>
#ifdef OS5
#include <sys/fcntl.h>
#endif

#if !defined(OS5) && !defined(hpux) && !defined(IBM_RS6000) && !defined(SG) && !defined(DEC_ALPHA) && !defined(LINUX_OR_MAC)
char           *sprintf();
#endif
char           *savestring(), *read_all_types();
short           get_rec_len();

int             fea_sd_special = 0;	/* global flag indicates whether
					 * single-channel, non-complex,
					 * FEA_SD files should be read in as
					 * shorts and treated as traditional
					 * waves SD (extra fields ignored);
					 * if this flag is not set, a FEA_SD
					 * file will be treated like any
					 * other FEA file (set automatically
					 * if board present) */

extern int      debug_level;

#ifndef NULL
#define NULL 0
#endif

Selector        head_a23 = {"burst", "%hd", NULL, NULL},
                head_a22 = {"ph_bound", "%hd", NULL, &head_a23},
                head_a21 = {"end_time", "%lf", NULL, &head_a22},
                head_a20 = {"lpc_order", "%d", NULL, &head_a21},
                head_a19 = {"window_size", "%d", NULL, &head_a20},
                head_a18 = {"window_step", "%d", NULL, &head_a19},
                head_a17b = {"remark", "#astr", NULL, &head_a18},
                head_a17 = {"comment", "#cstr", NULL, &head_a17b},
                head_a16b = {"text", "#astr", NULL, &head_a17},
                head_a16 = {"title", "#cstr", NULL, &head_a16b},
                head_a15 = {"start_time", "%lf", NULL, &head_a16},
                head_a14 = {"type_name", "%s", NULL, &head_a15},
                head_a13 = {"type_code", "%x", NULL, &head_a14},
                head_a12 = {"frequency", "%lf", NULL, &head_a13},
                head_a11 = {"bandwidth", "%lf", NULL, &head_a12},
                head_a10 = {"duration", "%lf", NULL, &head_a11},
                head_a9 = {"samples", "%d", NULL, &head_a10},
                head_a8 = {"segments", "%d", NULL, &head_a9},
                head_a7 = {"dimensions", "%d", NULL, &head_a8},
                head_a6 = {"maximum", "%lf", NULL, &head_a7},
                head_a5 = {"minimum", "%lf", NULL, &head_a6},
                head_a4 = {"version", "%d", NULL, &head_a5},
                head_a3b = {"operation", "#astr", NULL, &head_a4},
                head_a3 = {"operator", "#cstr", NULL, &head_a3b},
                head_a2b = {"time", "#astr", NULL, &head_a3},
                head_a2 = {"date", "#cstr", NULL, &head_a2b},
                head_a1 = {"description", "#string", NULL, &head_a2},
                head_a0 = {"identifiers", "#list", NULL, &head_a1};

/*
 * This list determines which signal attributes from the Signal structure are
 * printed in the header on output.  The attributes will be printed in the
 * order of this table.
 */
static Selector *active_header_entries[] = {
   &head_a4,
   &head_a2b,
   &head_a13,
   &head_a12,
   &head_a9,
   &head_a15,
   &head_a21,
   &head_a11,
   &head_a7,
   &head_a6,
   &head_a5,
   &head_a0
};
#define N_HEADER_ENTRIES (sizeof(active_header_entries)/sizeof(Selector*))

typedef struct meth_key {
   char           *key;
   int             (*proc) ();
}               Meth_key;

int             shortpr(), intpr(), longpr(), floatpr(), doublepr(), charpr(),
                astrpr(), cstrpr(), qstrpr(), strpr(), shexpr(), lhexpr(), loctpr(),
                listpr(), stringpr(), soctpr();

Meth_key        var_types[] = {
   "%d", longpr,
   "%d", intpr,
   "%f", floatpr,
   "%lf", doublepr,
   "%c", charpr,
   "%s", strpr,
   "%hx", shexpr,
   "%x", lhexpr,
   "%ho", soctpr,
   "%o", loctpr,
   "%hd", shortpr,
   "#list", listpr,
   "#str", strpr,
   "#string", stringpr,
   "#cstr", cstrpr,
   "#astr", astrpr,
   "#strq", qstrpr,
   "#qstr", qstrpr,
   "the_end", NULL
};

typedef struct sppack {
   char            comment1[80];
   char            comment2[80];
   char            command[80];
   short           domain;
   short           framelength;
   float           frequency;
   short           aux1;
   short           aux2;
   short           magic;
   short           type;
   short           module[128];
}               Sppack;

char           *get_date();
static char     scratch[200];

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
int
signal_magic()
{
   int             i;

   strncpy((char *) (&i), "SIG\n", 4);
   return (i);
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*
 * Allocate and initialize a new header large enough for n+1 bytes (data +
 * possible null terminator).  The internal header structure is described in
 * Objects.h The external form of the header (in a file) consists of a simple
 * char array.  The first four bytes are ascii for SIG<newline>; the next
 * eight are 7 ascii decimal numerals (or leading blanks) for the header size
 * and a <newline>. Thus, the header may be meaningfully read entirely as an
 * ascii string.
 */
Header         *
w_new_header(n)
   int             n;
{
   Header         *h;
   char           *c;

   if (debug_level >= 1)
      printf("w_new_header: function entered, n = %d.\n", n);

   if ((h = (Header *) malloc(sizeof(Header)))) {
      if (n) {
	 if (!(c = malloc(n + 1))) {
	    printf("Can't allocate a header structure in w_new_header()\n");
	    return (NULL);
	 }
	 h->nbytes = n;
	 h->header = c;
      } else {
	 h->nbytes = 0;
	 h->header = NULL;
      }
      h->magic = SIGNAL_MAGIC;
      h->npad = 0;
      h->esps_hdr = NULL;
      h->esps_nbytes = 0;
      h->e_scrsd = 0;
      h->e_short = 0;
      h->strm = NULL;
      h->esps_clean = 0;
      return (h);
   }
   printf("Can't allocate a header structure in w_new_header()\n");
   return (NULL);

}

/***********************************************************************/
/*
 * Put arbitrary keyword-format-value triples in a header.  The format must
 * be exactly one of the types specified above in the "var_types" list. If
 * the keyword appears in the "head_a0" list, the format is implied and will
 * not be printed in the header.
 */

#define STR_ALLOC 500

head_printfx(head, key, format, value)
   Header         *head;
   char           *key, *format, *value;
{
   Selector        sl;
   int             n;

   if (!(n = head_printf(head, key, value))) {
      static char    *str = NULL;
      static int      strsize;

      if (!str) {
	 if (!(str = malloc(STR_ALLOC + 1))) {
	    printf("Can't allocate scratch space in head_printfx()\n");
	    return 0;
	 }
	 strsize = STR_ALLOC;
      }
      sl.name = key;
      sl.format = format;
      if (*format == '#')
	 sl.dest = (char *) (&value);
      else
	 sl.dest = value;
      if ((n = arg_to_stringx(&str, &strsize, &sl)))
	 head_append(head, str, n);
   }
   return (n);
}


/***********************************************************************/
/*
 * Retrieve arbitrary data from the header by its keyword.  If the keyword
 * appears in the "head_a0" list, the corresponding format is used to read
 * the data.  Otherwise IT IS ASSUMED THAT A FORMAT SPECIFICATION IMMEDIATELY
 * FOLLOWS THE NON-STANDARD KEYWORD IN THE HEADER.  THIS WILL BE TRUE IF
 * head_printfx() WAS USED TO INSTALL THE NON-STANDARD INFORMATION. Returns 1
 * on success, 0 on failure. NOTE: When head_scanf() is called to retrieve a
 * standard C type, dest should be a pointer to the variable to be assigned.
 * When it is called to retrieve #string, #cstr, #astr, or #list types, dest
 * must be a pointer to a pointer to the free()-able variable structure (or
 * NULL) so that new structures can be allocated as needed to accommodate the
 * (variable size) input data.
 */
head_scanf(head, key, dest)
   Header         *head;
   char           *key, *dest;
{
   Selector       *sl, *slt, s;
   int             its_in_list = FALSE;

   sl = &head_a0;
   while (sl) {
      if (!strcmp(sl->name, key)) {
	 sl->dest = dest;
	 its_in_list = TRUE;
	 slt = sl;
      } else
	 sl->dest = NULL;
      sl = sl->next;
   }
   if (its_in_list)
      return (get_header_args(head->header, &head_a0));
   else {
      s.name = key;
      s.format = NULL;		/* this will be retrieved from the header */
      s.dest = dest;
      s.next = &head_a0;
      return (get_header_args(head->header, &s));
   }
}

/***********************************************************************/
/*
 * Scans the null-terminated character array, alist, for keywords present in
 * the Selector list adscr (like head_a0).  If keywords are encountered, the
 * associated value is decoded from alist and assigned to the variable
 * pointed to by the "dest" element of the Selector. Keywords may occur in
 * any order and may be repeated any number of times in alist.  The value
 * corresponding to the last occurrence of the keyword is retained.
 * Get_header_args() returns the number of assignments performed.
 */
get_header_args(alist, adscr)
   register char  *alist;
   Selector       *adscr;
{
   return (scan_header(alist, adscr, (char *) NULL, (char *) NULL));
}


/***********************************************************************/
/*
 * Given a Header, h, and a keyword, key, return a pointer to a string
 * specifying the data type of key's argument.
 */
char           *
get_keyword_format(h, key)
   Header         *h;
   char           *key;
{
   static char     type[10];
   Selector       *sl;

   sl = &head_a0;
   while (sl) {
      if (!strcmp(sl->name, key))
	 return (sl->format);
      sl->dest = NULL;		/* to disable assignments for what may follow */
      sl = sl->next;
   }
   *type = 0;
   scan_header(h->header, &head_a0, key, type);
   if (*type)
      return (type);
   return (NULL);
}

/***********************************************************************/
/*
 * Return a pointer to the value corresponding to the keyword, key, most
 * recently entered in the header, h.  The format specification for the value
 * will be copied into type.  Returns NULL on errors.
 */
char           *
get_header_item(h, key, type)
   Header         *h;
   char           *key, *type;
{
   char           *tp;
   static double   dr;
   static int      ir;
   static char     cr, str[500], *stringr = NULL;
   static short    sr;
   static float    fr;
   static List    *lpr = NULL;

   if ((tp = (char *) get_keyword_format(h, key))) {
      strcpy(type, tp);
      switch (*type) {
      case '%':		/* it is a standard C type */
	 if (!(strcmp(type, "%d") && strcmp(type, "%o") && strcmp(type, "%x"))) {
	    head_scanf(h, key, (char *) &ir);
	    return ((char *) &ir);
	 }
	 if (!(strcmp(type, "%hd") && strcmp(type, "%ho") && strcmp(type, "%hx"))) {
	    head_scanf(h, key, (char *) &sr);
	    return ((char *) &sr);
	 }
	 if (!strcmp(type, "%lf")) {
	    head_scanf(h, key, (char *) &dr);
	    return ((char *) &dr);
	 }
	 if (!strcmp(type, "%f")) {
	    head_scanf(h, key, (char *) &fr);
	    return ((char *) &fr);
	 }
	 if (!strcmp(type, "%c")) {
	    head_scanf(h, key, &cr);
	    return (&cr);
	 }
	 if (!strcmp(type, "%s")) {
	    head_scanf(h, key, str);
	    return (str);
	 }
      case '#':
	 if (strcmp(type, "#list")) {
	    head_scanf(h, key, (char *) &lpr);
	    return ((char *) lpr);
	 }
	 if (strcmp(type, "#string")) {
	    head_scanf(h, key, (char *) &stringr);
	    return (stringr);
	 }
	 if (strcmp(type, "#cstr")) {
	    head_scanf(h, key, (char *) &stringr);
	    return (stringr);
	 }
	 if (strcmp(type, "#astr")) {
	    head_scanf(h, key, &stringr);
	    return (stringr);
	 }
      default:
	 break;
      }
   }
   return (NULL);
}

/***********************************************************************/
/*
 * If called with NULL pointers for key and type, see get_header_args().  If
 * key is a keyword and type points to a character array, scan_header() also
 * places the format specification for key in type (see get_keyword_type()).
 */
scan_header(alist, adscr, key, type)
   register char  *alist, *key, *type;
   Selector       *adscr;
{
   int             gotsome = 0, gotone;
   char            name[32], junk[200], *get_next_item();
   Selector       *ad;

   if (alist && *alist && adscr) {
      while ((*alist == ' ') || (*alist == '\t') || (*alist == '\n'))
	 alist++;		/* skip initial blanks and blank lines */
      while (*alist) {
	 strnscan(alist, name, sizeof(name));
	 if (!*(alist = get_next_item(alist))) {
	    printf("No argument for selector %s\n", name);
	    return (gotsome);
	 }
	 ad = adscr;
	 gotone = 0;
	 while (ad) {
	    if (!strcmp(name, ad->name)) {
	       if (ad->format) {
		  alist = read_all_types(alist, ad->format, ad->dest);
		  if (key && !strcmp(name, key))
		     strcpy(type, ad->format);
	       } else {		/* format should be in header... */
		  sscanf(alist, "%s", junk);
		  alist = get_next_item(alist);
		  if ((*junk != '%') && (*junk != '#')) {
		     printf("Free-format keyword %s without format spec.; %s ignored\n",
			    name, junk);
		     break;
		  }
		  alist = read_all_types(alist, junk, ad->dest);
		  if (key && !strcmp(name, key))
		     strcpy(type, junk);
	       }
	       if (ad->dest)
		  gotsome++;	/* if an assignment was made */
	       gotone = 1;
	       break;
	    } else
	       ad = ad->next;
	 }
	 if (!gotone) {
	    strnscan(alist, junk, sizeof(junk));
	    if ((*junk == '%') || (*junk == '#')) {	/* looks like a format
							 * spec. */
	       if (key && !strcmp(name, key))
		  strcpy(type, junk);
	       alist = get_next_item(alist);	/* skip the non-standard item */
	       alist = read_all_types(alist, junk, NULL);
	    } else {		/* punt */
	       printf("Unknown selector %s; argument %s ignored\n", name, junk);
	       alist = get_next_item(alist);
	    }
	 }
      }
   }
   return (gotsome);
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
void
chcopy(out, in, n)
   register char  *out, *in;
   register int    n;
{
   for (; n-- > 0;)
      *out++ = *in++;
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* Return a header structure which is an identical copy of header h. */
Header         *
dup_header(h)
   Header         *h;
{
   Header         *h2;

   if (debug_level >= 1)
      printf("dup_header: nbytes = %d.\n", h->nbytes);
   if (h && (h2 = w_new_header(h->nbytes))) {
      if (h->nbytes) {
	 chcopy(h2->header, h->header, h->nbytes);
	 h2->header[h->nbytes] = 0;
      }
      h2->magic = h->magic;
      h2->esps_hdr = h->esps_hdr;
/* since this contains a pointer to the same esps header
   do not let it be freed
*/
      h->esps_clean = 0;
      h2->esps_clean = 0;
      h2->esps_nbytes = h->esps_nbytes;
      h2->strm = h->strm;
      h2->npad = h->npad;
      return (h2);
   } else
      printf("Error in dup_header\n");
   return (NULL);
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
void
setup_access(sig)
   Signal         *sig;
{
   head_a21.dest = (char *) &(sig->end_time);
   head_a15.dest = (char *) &(sig->start_time);
   head_a13.dest = (char *) &(sig->type);
   head_a12.dest = (char *) &(sig->freq);
   head_a11.dest = (char *) &(sig->band);
   head_a9.dest = (char *) &(sig->file_size);
   head_a7.dest = (char *) &(sig->dim);
   head_a6.dest = (char *) &(sig->smax[0]);
   head_a5.dest = (char *) &(sig->smin[0]);
   head_a4.dest = (char *) &(sig->version);
   head_a0.dest = (char *) &(sig->idents);
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*
 * Get information from ESPS header and initialize Waves Signal structure
 * values accordingly.
 */

static int est_file_size();

#define BAD_HDR_ITEM(itemname, type) \
printf("%s: header item \"%s\" missing or not %s.\n", \
"read_esps_hdr", (itemname), (type))

int
read_esps_hdr(sig, hdr)
   Signal         *sig;
   struct header  *hdr;
{
   switch (hdr->common.type) {
   case FT_SD:
      {

	 sig->idents = (List *) malloc(sizeof(List));
	 sig->idents->str = savestring("samples");
	 sig->idents->next = NULL;
      }
      sig->header->e_scrsd = 1;

      if (get_sd_type(hdr) == SHORT) {
	 sig->header->e_short = 1;
	 if (debug_level >= 1)
	    printf("read_esps_hdr: FT_SD is SHORT, setting e_short\n");
      }
      sig->version = 1;

      sig->dim = 1;
      sig->file_size = hdr->common.ndrec;
      if (debug_level >= 1)
	 printf("read_esps_hdr: sig->file_size = %d.\n", sig->file_size);

      sig->band = 0.5 * hdr->hd.sd->sf;
      sig->freq = hdr->hd.sd->sf;
      if (debug_level >= 1)
	 printf("read_esps_hdr: sig->freq = %g.\n", sig->freq);
      sig->type = P_SHORTS;

      sig->start_time = get_genhd_val("start_time", hdr, 0.0);
      if (genhd_type("start_time", (int *) NULL, hdr) == HD_UNDEF)
	 *add_genhd_d("start_time", (double *) NULL, 1, hdr) =
	    sig->start_time;

      sig->end_time =
	 get_genhd_val("end_time", hdr,
		       sig->file_size / sig->freq + sig->start_time);
      if (genhd_type("end_time", (int *) NULL, hdr) == HD_UNDEF)
	 *add_genhd_d("end_time", (double *) NULL, 1, hdr) =
	    sig->end_time;

      break;

   case FT_FEA:
      switch (hdr->hd.fea->fea_type) {
      case FEA_SPEC:
	 {
	    double          sf;
	    short           freq_format;
	    float          *hd_freqs;
	    double         *sig_freqs;
	    int             i;

	    if (genhd_type("freq_format", (int *) NULL, hdr) != CODED) {
	       BAD_HDR_ITEM("freq_format", "CODED");
	       return FALSE;
	    } else
	       switch (freq_format = *get_genhd_s("freq_format", hdr)) {
	       case SPFMT_SYM_CTR:
		  printf("%s: freq_format SYM_CTR not supported.\n",
			 "read_esps_hdr");
		  return FALSE;
		  break;
	       case SPFMT_ASYM_CTR:
		  printf("%s: freq_format ASYM_CTR not supported.\n",
			 "read_esps_hdr");
		  return FALSE;
		  break;
	       case SPFMT_ARB_VAR:
		  printf("%s: freq_format ARB_VAR not supported.\n",
			 "read_esps_hdr");
		  return FALSE;
		  break;
	       default:
		  break;
	       }

	    if (genhd_type("spec_type", (int *) NULL, hdr) != CODED) {
	       BAD_HDR_ITEM("spec_type", "CODED");
	       return FALSE;
	    } else
	       switch (*get_genhd_s("spec_type", hdr)) {
	       case SPTYP_DB:
		  break;
	       default:
		  printf(
		     "%s: only spec_type SPTYP_DB supported in FEA_SPEC.\n",
			 "read_esps_hdr");
		  return FALSE;
		  break;
	       }

/*!*//* idents? */

	    sig->version = 1;

	    if (genhd_type("num_freqs", (int *) NULL, hdr) == LONG)
	       sig->dim = *get_genhd_l("num_freqs", hdr);
	    else {
	       BAD_HDR_ITEM("num_freqs", "LONG");
	       return FALSE;
	    }

	    if (hdr->common.ndrec == -1)
	    {
	       /* Pipe or variable record size (e.g. Esignal Ascii).
		* Can't get the file size without reading the whole
		* file and counting, so use an overestimate
		* and assume that "check_file_size" will supply the
		* right value when we actually reach the end of file.
		*/
	       sig->file_size = est_file_size(sig);
	    }
	    else
	       sig->file_size = hdr->common.ndrec;

	    if (debug_level >= 1)
		printf("read_esps_hdr: sig->file_size = %d.\n",
		       sig->file_size);

	    switch (freq_format) {
	    case SPFMT_SYM_EDGE:
	    case SPFMT_ASYM_EDGE:
	    case SPFMT_SYM_CTR:
	    case SPFMT_ASYM_CTR:
	       if (genhd_type("sf", (int *) NULL, hdr) == FLOAT)
		  sf = *get_genhd_f("sf", hdr);
	       else {
		  BAD_HDR_ITEM("sf", "FLOAT");
		  return FALSE;
	       }
	       break;
	    default:
/*!*//* assign dummy value to sf? */
	       break;
	    }

	    switch (freq_format) {
	    case SPFMT_ASYM_EDGE:
	       sig->band = sf;
	       sig->band_low = -0.5 * sf;
	       sig->y_dat = NULL;
	       break;
	    case SPFMT_SYM_EDGE:
	       sig->band = 0.5 * sf;
	       sig->band_low = 0.0;
	       sig->y_dat = NULL;
	       break;
	    case SPFMT_ARB_FIXED:
	       if (genhd_type("freqs", (int *) NULL, hdr) == FLOAT)
		  hd_freqs = get_genhd_f("freqs", hdr);
	       else {
		  BAD_HDR_ITEM("freqs", "FLOAT");
		  return FALSE;
	       }

	       sig_freqs = (double *) malloc(sig->dim * sizeof(double));
	       if (!sig_freqs) {
		  printf("%s: can't allocate space for freqs array.\n",
			 "read_esps_hdr");
		  return FALSE;
	       }
	       sig->y_dat = sig_freqs;
	       for (i = 0; i < sig->dim; i++)
		  sig_freqs[i] = hd_freqs[i];

	       /*
	        * FEA_SPEC support functions assumed to enforce monotone
	        * increasing frequencies.  If not, add code to the loop to
	        * find the max and min freqs.
	        */

	       sig->band_low = hd_freqs[0];
	       sig->band = hd_freqs[sig->dim - 1] - sig->band_low;
	       break;
	    default:
	       break;
	    }

	    sig->freq = get_genhd_val("record_freq", hdr, 1.0);
	    if (sig->freq == 0.0)
	       sig->freq = 1.0;

	    sig->type = SIG_SPECTROGRAM | P_CHARS;

	    sig->start_time = get_genhd_val("start_time", hdr, 0.0);

	    sig->end_time
	       = sig->start_time + sig->file_size / sig->freq;
	 }
	 break;
      case FEA_SD:
	 /*
	  * if single-channel, non-complex, and if the the data is SHORT or
	  * the fea_sd_special global is set, treat here; otherwise, drop
	  * through to general FEA file
	  */

	 if ((is_field_complex(hdr, "samples") == NO
	      && hdr->hd.fea->field_count == 1
	      && (fea_sd_special || (get_fea_type("samples", hdr) == SHORT)))
	     || get_genhd_val("encoding", hdr, (double) 0.0)) {

	    List            feasd_ident;
	    List           *end_ptr = &feasd_ident;
	    char            buf[250];

	    if ((get_fea_siz("samples", hdr, (short *) NULL, (long **) NULL)
		 == 1)) {
	       sig->header->e_scrsd = 1;

	       if (debug_level >= 1)
		  fprintf(stderr,
			  "read_esps_hdr: 1-chan., non-complex, FEA_SD; setting e_scrsd flag\n");
	    }
	    if (get_fea_type("samples", hdr) == SHORT) {
	       sig->header->e_short = 1;
	       if (debug_level >= 1)
		  fprintf(stderr,
		  "read_esps_hdr: \"samples\" is SHORT, setting e_short\n");
	    }
	    if (sig->header->e_scrsd != 1) {
               int i;
	       for (i = 0; i < get_fea_siz("samples", hdr, (short *) NULL,
					   (long **) NULL); i++) {
		  sprintf(buf, "samples[%d]", i);
		  end_ptr->next = (List *) malloc(sizeof(List));
		  end_ptr = end_ptr->next;
		  if (!end_ptr) {
		     fprintf(stderr, "%s: can't allocate ident node.\n",
			     "read_esps_hdr");
		     return FALSE;
		  }
		  end_ptr->str = savestring(buf);
	       }
	       end_ptr->next = NULL;
	       sig->idents = feasd_ident.next;
	    } else {
	       sig->idents = (List *) malloc(sizeof(List));
	       sig->idents->str = savestring("samples");
	       sig->idents->next = NULL;
	    }

	    sig->version = 1;

	    sig->dim =
	       get_fea_siz("samples", hdr, (short *) NULL, (long **) NULL);

	    if (hdr->common.ndrec == -1)
	    {
	       /* Pipe or variable record size (e.g. Esignal Ascii).
		* Can't get the file size without reading the whole
		* file and counting, so use an overestimate
		* and assume that "check_file_size" will supply the
		* right value when we actually reach the end of file.
		*/
	       sig->file_size = est_file_size(sig);
	    }
	    else
	       sig->file_size = hdr->common.ndrec;

	    if (debug_level >= 1)
	       printf("read_esps_hdr: sig->file_size = %d.\n", sig->file_size);

	    sig->band = 0.5 * get_genhd_val("record_freq", hdr, 1.0);

	    sig->freq = get_genhd_val("record_freq", hdr, 1.0);
	    if (debug_level >= 1)
	       printf("read_esps_hdr: sig->freq = %g.\n", sig->freq);
	    sig->type = P_SHORTS;

	    sig->start_time = get_genhd_val("start_time", hdr, 0.0);
	    if (genhd_type("start_time", (int *) NULL, hdr) == HD_UNDEF)
	       *add_genhd_d("start_time", (double *) NULL, 1, hdr) =
		  sig->start_time;

	    sig->end_time =
	       get_genhd_val("end_time", hdr,
			     sig->file_size / sig->freq + sig->start_time);
	    if (genhd_type("end_time", (int *) NULL, hdr) == HD_UNDEF)
	       *add_genhd_d("end_time", (double *) NULL, 1, hdr) =
		  sig->end_time;
	    break;
	 }			/* if not e_src FEA_SD, fall through to
				 * general FEA */
      default:			/* General FEA file. */
	 sig->version = 1;
	 sig->dim = get_rec_len(hdr);

	 if (hdr->common.ndrec == -1)
	 {
	    /* Pipe or variable record size (e.g. Esignal Ascii).
	     * Can't get the file size without reading the whole
	     * file and counting, so use an overestimate
	     * and assume that "check_file_size" will supply the
	     * right value when we actually reach the end of file.
	     */
	    sig->file_size = est_file_size(sig);
	 }
	 else
	    sig->file_size = hdr->common.ndrec;

	 if (debug_level >= 1)
	     printf("read_esps_hdr: sig->file_size = %d.\n", sig->file_size);

	 sig->band = 0.5 * get_genhd_val("record_freq", hdr, 1.0);
	 sig->freq = get_genhd_val("record_freq", hdr, 1.0);
	 if (sig->freq == 0.0)
	    sig->freq = 1.0;

	 sig->start_time = get_genhd_val("start_time", hdr, 0.0);

	 if (!hdr->common.tag)
	    sig->end_time = sig->file_size / sig->freq + sig->start_time;
	 else
	    sig->end_time = get_genhd_val("end_time", hdr,
					  sig->start_time - 1.0);	/* to flag not known! */

	 sig->type = P_MIXED;
	 sig->types = (int *) malloc(sig->dim * sizeof(int));
	 if (!sig->types) {
	    printf("%s: can't allocate space for type array.\n",
		   "read_esps_hdr");
	    return FALSE;
	 }

	 {
	    int             i, k;
	    int             j;
	    List            fea_idents;	/* dummy node precedes first actual
					 * node while list is being built */
	    List           *end_ptr = &fea_idents;
	    int             n = 0;

	    for (i = 0; i < hdr->hd.fea->field_count; i++) {
	       int             type, real;
	       int             rank;
	       long            size, *dim;
	       static unsigned maxrank;
	       static int     *coords = NULL;
	       char            buf[250], *suffix;

	       switch (hdr->hd.fea->types[i]) {
	       case BYTE:
	       case CHAR:
		  type = P_CHARS;
		  real = YES;
		  break;
	       case SHORT:
	       case CODED:
		  type = P_SHORTS;
		  real = YES;
		  break;
	       case LONG:
		  type = P_INTS;
		  real = YES;
		  break;
	       case FLOAT:
		  type = P_FLOATS;
		  real = YES;
		  break;
	       case DOUBLE:
		  type = P_DOUBLES;
		  real = YES;
		  break;
	       case BYTE_CPLX:
		  type = P_CHARS;
		  real = NO;
		  break;
	       case SHORT_CPLX:
		  type = P_SHORTS;
		  real = NO;
		  break;
	       case LONG_CPLX:
		  type = P_INTS;
		  real = NO;
		  break;
	       case FLOAT_CPLX:
		  type = P_FLOATS;
		  real = NO;
		  break;
	       case DOUBLE_CPLX:
		  type = P_DOUBLES;
		  real = NO;
		  break;
	       default:
		  printf("%s: unrecognized ESPS data type.\n",
			 "read_esps_hdr");
		  return FALSE;
	       }

	       size = hdr->hd.fea->sizes[i];
	       rank = (hdr->hd.fea->ranks)
		  ? hdr->hd.fea->ranks[i] : 1;
	       dim = (hdr->hd.fea->dimens && rank != 1)
		  ? hdr->hd.fea->dimens[i] : &size;

	       if (debug_level) {
		  fprintf(stderr,
			  "%s: size %d,  rank %d,\n\tdim  ",
			  "read_esps_hdr", size, rank);
		  for (k = 0; k < rank; k++)
		     fprintf(stderr, "%d ", dim[k]);
		  fprintf(stderr, "\n");
	       }
	       if (!coords) {
		  if ((maxrank = rank) > 0) {
		     coords = (int *)
			malloc(maxrank * sizeof(int));
		     if (!coords) {
			printf("%s: can't allocate coordinate array.\n",
			       "read_esps_hdr");
			return FALSE;
		     }
		  }
	       } else if (maxrank < rank) {
		  if ((maxrank = rank) > 0) {
		     coords = (int *)
			realloc((char *) coords,
				maxrank * sizeof(int));
		     if (!coords) {
			printf("%s: can't realloc coordinate array.\n",
			       "read_esps_hdr");
			return FALSE;
		     }
		  }
	       }
	       for (k = 0; k < rank; k++)
		  coords[k] = 0;

	       strcpy(buf, hdr->hd.fea->names[i]);
	       suffix = buf + strlen(buf);

	       for (j = 0; j < size; j++) {
		  int             h;
		  char           *end = suffix;

		  for (k = 0; k < rank; k++) {
		     sprintf(end, "[%d]", coords[k]);
		     end = end + strlen(end);
		  }

		  for (h = 0; h < (real ? 1 : 2); h++) {
		     static char    *re_im[2] = {".r", ".i"};

		     sig->types[n++] = type;

		     end_ptr->next = (List *) malloc(sizeof(List));
		     end_ptr = end_ptr->next;
		     if (!end_ptr) {
			printf("%s: can't allocate ident node.\n",
			       "read_esps_hdr");
			return FALSE;
		     }
		     if (!real)
			strcpy(end, re_im[h]);
		     end_ptr->str = savestring(buf);
		  }

		  /* Advance coordinates; check overflow */

		  for (k = rank - 1;
		       k >= 0 && coords[k] == dim[k] - 1;
		       k--)
		     coords[k] = 0;	/* propagate carries */
		  if (k >= 0)
		     coords[k]++;
		  else if (j < size - 1) {
		     printf("%s: field %s has inconsistent size and dimensions.\n",
			    "read_esps_hdr", hdr->hd.fea->names[i]);
		     return FALSE;
		  }
	       }
	       if (k >= 0) {
		  printf("%s: field %s has inconsistent size and dimensions.\n",
			 "read_esps_hdr", hdr->hd.fea->names[i]);
		  return FALSE;
	       }
	    }

	    end_ptr->next = NULL;
	    sig->idents = fea_idents.next;
	 }

	 break;
      }
      break;
   case FT_SPEC:
   default:
      printf("The only ESPS files supported so far are %s, %s, and %s.\n",
	     "sampled-data", "FEA_SPEC", "general FEA files");
      return FALSE;
      break;
   }

   return TRUE;

}

#undef BAD_HDR_ITEM

static int
est_file_size(sig)
    Signal  *sig;
{
    int     file;
    long    loc, len;

    file = sig->file;

    if (file < 0 && sig->name != NULL)
	file = open(sig->name, O_RDONLY);

    if (file < 0)		/* file nonexistent */
	return 0;

    loc = lseek(file, 0L, SEEK_CUR);

    if (loc < 0)		/* pipe */
	return INT_MAX;

    len = lseek(file, 0L, SEEK_END) - sig->bytes_skip;
    lseek(file, loc, SEEK_SET);	/* restore original position
				 * in case it was already open */

    if (sig->dim > 1)
	len /= sig->dim;	/* number of records, assuming
				 * at leas one byte per element */

    return (len < INT_MAX) ? len : INT_MAX;
}


/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*
 * Generate a Header structure containing all relevant information in the
 * Signal structure including any previous header.  Total header length will
 * always be modulo 4 bytes.
 */

Header         *
w_write_header(sig)
   Signal         *sig;
{
   Header         *h;
   register int    i, j;
   register char  *p, *q;
   int             size, used, nentries = N_HEADER_ENTRIES, npad;
   char           *buf;

   if (sig) {
      size = STR_ALLOC;
      if (sig->header)
	 size += sig->header->nbytes - sig->header->npad;

      if ((buf = malloc(size + 1))) {
	 if (sig->header) {
	    for (i = 0, p = buf, q = sig->header->header,
		 j = sig->header->nbytes - sig->header->npad; i++ < j;)
	       *p++ = *q++;
	    used = j;
	 } else {
	    p = buf;
	    used = 0;
	 }
	 setup_access(sig);
	 for (i = 0; i < nentries; i++) {
	    if (!head_build(&buf, &used, &size, active_header_entries[i]))
	       return (NULL);
	 }
	 if ((j = used & 3))	/* need to pad to INTEGER*4? */
	    npad = 4 - j;
	 else
	    npad = 0;
	 buf = realloc(buf, used + npad + 1);
	 for (j = 0; j < npad; j++)
	    buf[used + j - 1] = ' ';	/* pad with spaces */
	 used += npad;
	 buf[used] = 0;
	 h = w_new_header(0);
	 h->nbytes = used;
	 h->header = buf;
	 h->npad = npad;
	 head_printf(h, "time", get_date());
	 return (h);
      }
   }
   return (NULL);
}


/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

int
chk_str_len(str, avail, req)
   char          **str;
   int            *avail, req;
{
   if (req <= *avail)
      return FALSE;
   if (str && *str && (*str = realloc(*str, req + STR_ALLOC + 1))) {
      *avail = req + STR_ALLOC;
      return FALSE;
   }
   printf("chk_str_len: Can't realloc space.\n");
   return TRUE;
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*
 * Output the data described by the Selector as a formatted print to str.
 * Return the number of characters actually written.  Ordinary C types will
 * be followed by newline which is included in the count. NOTE: str must be
 * large enough to accommodate all types of output!
 */
arg_to_string(str, size, argp)
   char          **str;
   int            *size;
   Selector       *argp;
{
   int             i;

   if (str && *str && argp && argp->dest) {
      for (i = 0; var_types[i].proc; i++) {
	 if (!strcmp(argp->format, var_types[i].key))
	    return (var_types[i].proc(str, size, argp));
      }
   }
   return (0);
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*
 * This is the same as arg_to_string(), except that the format of the output
 * is placed (space separated) between the keyword and the value. This makes
 * possible printing and retrieving arbitrary header data.
 */
arg_to_stringx(str, size, argp)
   char          **str;
   int            *size;
   Selector       *argp;
{
   int             i;
   Selector        sl;
   char            name[100];

   if (str && *str && argp && argp->dest) {
      for (i = 0; var_types[i].proc; i++) {
	 if (!strcmp(argp->format, var_types[i].key)) {
	    sprintf(name, "%s %s", argp->name, argp->format);
	    sl.name = name;
	    sl.format = argp->format;
	    sl.dest = argp->dest;
	    return (var_types[i].proc(str, size, &sl));
	 }
      }
   }
   return (0);
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
shortpr(str, size, argp)
   char          **str;
   int            *size;
   Selector       *argp;
{
   char           *p;

   strcpy(*str, argp->name);
   p = *str + strlen(*str);
   sprintf(p, " %d\n", *(short *) argp->dest);
   return (strlen(*str));
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
intpr(str, size, argp)
   char          **str;
   int            *size;
   Selector       *argp;
{
   char           *p;

   strcpy(*str, argp->name);
   p = *str + strlen(*str);
   sprintf(p, " %d\n", *(int *) argp->dest);
   return (strlen(*str));
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
longpr(str, size, argp)
   char          **str;
   int            *size;
   Selector       *argp;
{
   char           *p;

   strcpy(*str, argp->name);
   p = *str + strlen(*str);
   sprintf(p, " %d\n", *(int *) argp->dest);
   return (strlen(*str));
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
floatpr(str, size, argp)
   char          **str;
   int            *size;
   Selector       *argp;
{
   char           *p;

   strcpy(*str, argp->name);
   p = *str + strlen(*str);
   sprintf(p, " %f\n", *(float *) argp->dest);
   return (strlen(*str));
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
doublepr(str, size, argp)
   char          **str;
   int            *size;
   Selector       *argp;
{
   char           *p;

   strcpy(*str, argp->name);
   p = *str + strlen(*str);
   sprintf(p, " %f\n", *(double *) argp->dest);
   return (strlen(*str));
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
charpr(str, size, argp)
   char          **str;
   int            *size;
   Selector       *argp;
{
   char           *p;

   strcpy(*str, argp->name);
   p = *str + strlen(*str);
   sprintf(p, " %c\n", *(char *) argp->dest);
   return (strlen(*str));
}


/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
strpr(str, size, argp)
   char          **str;
   int            *size;
   Selector       *argp;
{
   if (chk_str_len(str, size, strlen(argp->name) + 1 + strlen(argp->dest) + 1))
      return 0;
   strcpy(*str, argp->name);
   strcat(*str, " ");
   strcat(*str, argp->dest);
   strcat(*str, "\n");
   return (strlen(*str));
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
cstrpr(str, size, argp)
   char          **str;
   int            *size;
   Selector       *argp;
{
   char           *p;

   if (chk_str_len(str, size, strlen(argp->name) + 1 + strlen(argp->dest) + 1))
      return 0;
   strcpy(*str, argp->name);
   p = *str + strlen(*str);
   sprintf(p, " %s\n", *((char **) argp->dest));	/* append newline and
							 * null */
   return (strlen(*str) + 1);	/* include the null in length */
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
astrpr(str, size, argp)
   char          **str;
   int            *size;
   Selector       *argp;
{
   char           *p;

   if (chk_str_len(str, size, strlen(argp->name) + 1 + strlen(argp->dest) + 1))
      return 0;
   strcpy(*str, argp->name);
   p = *str + strlen(*str);
   sprintf(p, " %s\n", *((char **) argp->dest));	/* append a newline */
   return (strlen(*str));	/* null NOT included in length */
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
qstrpr(str, size, argp)		/* Cf. qstr type in read_all_types in parse.c */
   char          **str;
   int            *size;
   Selector       *argp;
{
   char           *p, *q;
   int             len = 0;
   int             need_quotes = 0;


   if ((!*argp->dest) || strchr(argp->dest, ' ') || (argp->dest[0] == '"'))
      need_quotes = 1;

   if (need_quotes)
      len = 2;			/* 2 delimiting quotes */

   for (q = argp->dest; *q; q++)
      switch (*q) {
      case '"':
      case '\\':
	 len += 2;		/* escaped */
	 break;
      default:
	 len += 1;
	 break;
      }

   if (chk_str_len(str, size, strlen(argp->name) + 1 + len + 1))
      return 0;

   strcpy(*str, argp->name);

   if (need_quotes)
      strcat(*str, " \"");
   else
      strcat(*str, " ");

   for (p = *str + strlen(*str), q = argp->dest; *q; q++)
      switch (*q) {
      case '"':
      case '\\':
	 *p++ = '\\';
	 /* Falls through. */
      default:
	 *p++ = *q;
	 break;
      }
   if (need_quotes)
      strcpy(p, "\"\n");
   else
      strcpy(p, "\n");

   return (strlen(*str));	/* null NOT included in length */
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
shexpr(str, size, argp)
   char          **str;
   int            *size;
   Selector       *argp;
{
   char           *p;

   strcpy(*str, argp->name);
   p = *str + strlen(*str);
   sprintf(p, " %x\n", *(short *) argp->dest);
   return (strlen(*str));
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
lhexpr(str, size, argp)
   char          **str;
   int            *size;
   Selector       *argp;
{
   char           *p;

   strcpy(*str, argp->name);
   p = *str + strlen(*str);
   sprintf(p, " %x\n", *(int *) argp->dest);
   return (strlen(*str));
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
loctpr(str, size, argp)
   char          **str;
   int            *size;
   Selector       *argp;
{
   char           *p;

   strcpy(*str, argp->name);
   p = *str + strlen(*str);
   sprintf(p, " %o\n", *(int *) argp->dest);
   return (strlen(*str));
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
soctpr(str, size, argp)
   char          **str;
   int            *size;
   Selector       *argp;
{
   char           *p;

   strcpy(*str, argp->name);
   p = *str + strlen(*str);
   sprintf(p, " %o\n", *(short *) argp->dest);
   return (strlen(*str));
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

listpr(str, size, argp)
   char          **str;
   int            *size;
   Selector       *argp;
{
   int             psize;
   register char  *p;
   List           *l;

   if ((l = *((List **) argp->dest))) {
      if (chk_str_len(str, size, strlen(argp->name) + 2))
	 return 0;
      strcpy(*str, argp->name);
      strcat(*str, " ;");
      psize = strlen(*str);
      while (l) {
	 p = *str + psize;
	 if (l->str && *l->str) {
	    psize += 1 + strlen(l->str);
	    if (chk_str_len(str, size, psize))
	       return 0;
	    sprintf(p, " %s", l->str);
	 }
	 l = l->next;
      }
      p = *str + psize;
      if (chk_str_len(str, size, psize + 3))
	 return 0;
      sprintf(p, " ;\n");
      return (psize + 3);
   }
   return (0);
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
stringpr(str, size, argp)
   char          **str;
   int            *size;
   Selector       *argp;
{
   int             psize, dsize;
   register int    i, n;
   register char  *p, *q, *s;

   if (argp->dest && *((char **) argp->dest)) {
      s = *((char **) argp->dest);
      sscanf(s, "%d", &dsize);
      sprintf(*str, "%s %d ", argp->name, dsize);
      /*
       * Skip the byte count (possibly preceeded by whitespace) and the
       * following (single) space or tab.
       */
      q = s;
      while ((*q < '0') || (*q > '9'))
	 q++;
      while ((*q >= '0') && (*q <= '9'))
	 q++;
      q++;			/* q now points to start of data source */
      p = *str + (psize = strlen(*str));	/* point to start of data
						 * dest. */
      if (chk_str_len(str, size, psize + dsize))
	 return 0;
      for (i = dsize; i-- > 0;)
	 *p++ = *q++;
      *p = 0;
      return (dsize + psize);
   }
   return (0);
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
head_build(buf, used, size, arg)
   char          **buf;		/* buffer into which the header is being
				 * built */
   Selector       *arg;		/* pointer to data destined for header */
   int            *size,	/* dimension(-1) of *buf */
                  *used;	/* number of bytes of buff that have been
				 * used */
{
   int             n;
   static char    *scr = NULL;	/* A scratch character buffer */
   static int      scrsize;

   if (!scr) {
      if (!(scr = malloc(STR_ALLOC + 1))) {
	 printf("Can't allocate scratch space in w_write_header()\n");
	 return FALSE;
      }
      scrsize = STR_ALLOC;
   }
   n = arg_to_string(&scr, &scrsize, arg);
   if ((n + *used) > *size) {	/* bump header block size */
      if (!(*buf = realloc(*buf, *size + n + STR_ALLOC + 1))) {
	 printf("Can't allocate enough buffer space in head_build()\n");
	 return (FALSE);
      }
      *size += STR_ALLOC + n;
   }
   strncpy((char *) ((*buf) + (*used)), scr, n);
   *used += n;
   return (TRUE);
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*
 * Put a keyword-argument pair in header h.  Selname points to a string
 * containing a defined keyword; argp points to a value of the corresponding
 * type.
 */
head_printf(h, selname, argp)
   Header         *h;
   char           *selname;
   char           *argp;
{
   Selector       *s;
   static char    *str = NULL;
   static int      strsize;
   int             n;

   if (!strcmp("identifiers", selname))
      return (head_ident(h, argp));

   if (!str) {
      if (!(str = malloc(STR_ALLOC + 1))) {
	 printf("Can't allocate scratch space in head_printf()\n");
	 return 0;
      }
      strsize = STR_ALLOC;
   }
   s = &head_a0;
   while (s) {
      if (!strcmp(selname, s->name)) {
	 if (*(s->format) == '#')
	    s->dest = (char *) (&argp);
	 else
	    s->dest = argp;
	 if ((n = arg_to_string(&str, &strsize, s)))
	    head_append(h, str, n);
	 s->dest = NULL;
	 return (n);
      }
      s = s->next;
   }
   return (0);
}

#undef STR_ALLOC

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
head_ident(h, str)
   Header         *h;
   char           *str;
{
   int             n, i;
   static char     delim[] = ";*#!|%@$^&()_:+/?=", ident[] = "identifiers";
   char           *cp;

   if (h && str && (n = strlen(str))) {
      i = 0;
      while (strrchr(str, delim[i])) {
	 if (!delim[i]) {
	    printf("Couldn't find a unique delimiter from the set %s in head_list()\n",
		   delim);
	    return (0);
	 } else
	    i++;
      }
      if ((cp = malloc(n + 6 + strlen(ident) + 1))) {
	 sprintf(cp, "%s %c %s %c\n", ident, delim[i], str, delim[i]);
	 head_append(h, cp, (n = strlen(cp)));
	 free(cp);
	 return (n);
      } else
	 printf("Can't allocate a buffer in head_ident()\n");
   } else
      printf("Bad arguments to head_ident()\n");

   return (0);
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*
 * Append "size" bytes drawn from "arg" to header "h."  The total header
 * length (h->nbytes) will be forced to be modulo-4.
 */
head_append(h, arg, size)
   Header         *h;
   char           *arg;
   int             size;
{
   register int    newpad, newhead_size, j;

   if (h && arg && size) {
      newhead_size = size + h->nbytes - h->npad;
      if ((j = newhead_size & 3))	/* force modulo-4 bytes */
	 newpad = 4 - j;
      else
	 newpad = 0;
      newhead_size += newpad;
      if (!(h->header = realloc(h->header, newhead_size + 1))) {
	 printf("Can't allocate enough buffer space in head_append()\n");
	 return (FALSE);
      }
      chcopy(h->header + h->nbytes - h->npad, arg, size);
      h->npad = newpad;
      for (; newpad; newpad--)
	 h->header[newhead_size - newpad] = ' ';	/* pad with spaces */
      h->header[newhead_size] = 0;
      h->nbytes = newhead_size;
      return (TRUE);
   }
   return (FALSE);
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*
 * Read a signal header and initialize the Signal structure values
 * accordingly.  Note that multiple versions of the header may be read, but
 * only the most recently specified values are retained in the Signal
 * structure.  Sig->bytes_skip will be set to the number of bytes offset in
 * the file required to skip the header.  Minimal support is provided here
 * for Nirvonics headers and Peter Kroon's headers.  Normal return is TRUE,
 * else FALSE.
 */
w_read_header(sig, head)
   Signal         *sig;
   Header         *head;
{
   if (sig && head) {
      if (debug_level >= 1)
	 printf("w_read_header: head->magic = 0x%x.\n", head->magic);
      if (head->magic == SIGNAL_MAGIC) {
	 if (debug_level >= 1)
	    printf("w_read_header: calling setup_access & get_header_args.\n");
	 setup_access(sig);
	 get_header_args(head->header, &head_a0);
	 sig->bytes_skip = head->nbytes + 12;
	 if (debug_level >= 1)
	    printf("w_read_header: sig->bytes_skip = %d.\n", sig->bytes_skip);
	 return (TRUE);
      }
      switch (head->magic) {
      case ESPS_MAGIC:
	 if (debug_level >= 1)
	    printf("w_read_header: calling read_esps_hdr.\n");
	 if (!read_esps_hdr(sig, head->esps_hdr)) {
	    printf("w_read_header: call to read_esps_hdr failed.\n");
	    return FALSE;
	 }
	 sig->bytes_skip = head->esps_nbytes;
	 {
	    Header         *h;

	    h = w_write_header(sig);
	    head->header = h->header;
	    head->nbytes = h->nbytes;
	    free(h);
	 }
	 if (debug_level >= 1)
	    printf("w_read_header: sig->bytes_skip = %d.\n", sig->bytes_skip);
	 return TRUE;
	 break;
#ifdef IBM_RS6000
      case -0xfeff01:
#else
      case HEADER_MAGIC:
#endif
      case RHEADR_MAGIC:
	 {
#define pk2(byt) (p=bpos+byt, ((int)p[1])&255 | (((int)p[0])&255)<<8)
#define pk4(byt) \
(p=bpos+byt, ((int)p[1])&255 | (((int)p[0])&255)<<8 \
	 | (((int)p[3])&255)<<16 | (((int)p[2])&255)<<24);
	    char           *bpos = head->header, *p;
	    int             type, size;
	    for (;;) {
	       size = pk4(0);
	       if (size < 8)
		  size = 8;
	       type = pk4(4);
	       if (type == 2) {
		  sig->freq = pk4(8);
		  sig->type = (pk2(14) ? P_SHORTS : P_USHORTS);
	       }
	       if (type == 0)
		  break;
	       bpos += size;
	    }
	    sig->bytes_skip = head->nbytes + 4;
	    return (TRUE);
	 }
      case SPPACK_MAGIC:
	 {
	    Header         *h;
	    Sppack         *sphdr;
	    int             length = 0;
	    int             fd = -1;

	    sig->bytes_skip = 01000;
	    if (sig->file >= 0) {	/* already open */
	       length = lseek(sig->file, 0L, 2) - sig->bytes_skip;
	       lseek(sig->file, (long) sig->bytes_skip, 0);
	    } else if ((fd = open(sig->name, O_RDONLY)) >= 0) {
	       length = lseek(fd, 0L, 2) - sig->bytes_skip;
	       close(fd);
	    }
	    if (length < 0)
	       length = 0;
	    sphdr = (Sppack *) (head->header);
	    if (sphdr->type < 0)
	       sphdr->type = -sphdr->type;
	    switch (sphdr->type / 1000) {
	    case 1:
	       sig->type = 4;
	       sig->file_size = length / sizeof(short);
	       break;
	    case 2:
	       sig->type = 5;
	       sig->file_size = length / sizeof(int);
	       break;
	    case 3:
	       sig->type = 7;
	       sig->file_size = length / sizeof(float);
	       break;
	    case 4:
	       sig->type = 8;
	       sig->file_size = length / sizeof(double);
	       break;
	    default:
	       printf("not able to handle this data type\n");
	       return (FALSE);
	    }
	    sig->freq = sphdr->frequency;
	    sig->band = sig->freq / 2.0;
	    if (sig->freq > 0)
	       sig->end_time = sig->start_time + sig->file_size / sig->freq;
	    if (head->header)
	       free(head->header);
	    if ((h = w_write_header(sig))) {
	       head->header = h->header;
	       head->nbytes = h->nbytes;
	       free(h);
	       return (TRUE);
	    } else {
	       printf("Problems with spp header in w_read_header\n");
	       return (FALSE);
	    }
	 }
      default:
	 printf("Header type %d is not yet recognized.\n", head->magic);
	 printf("Please add processing for this type to w_read_header()");
	 printf(" in header.c\n");
	 return (FALSE);
      }
   }
   return (FALSE);

}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*
 * Write a SIG header to a file.  The file must be open for writing and the
 * file pointer must be properly set (i.e. to the beginning of the file for
 * single-segment files, or the beginning of a segment for multi-segment
 * files).
 */
put_header(h, fd)
   Header         *h;
   int             fd;
{
   char            temp[10];
   int             i;

   if (fd >= 0) {
      if (h && h->nbytes && (h->magic == SIGNAL_MAGIC) && h->header) {
	 i = SIGNAL_MAGIC;
	 sprintf(temp, "%7d\n", h->nbytes);
	 if ((write(fd, &i, 4) == 4) && (write(fd, temp, 8) == 8)) {
	    if (write(fd, h->header, h->nbytes) == h->nbytes)
	       return (TRUE);
	    else
	       printf("Can't write body of header in put_header()\n");
	 } else
	    printf("Can't write header header in put_header()\n");
      } else if (h && h->magic == ESPS_MAGIC && h->esps_hdr && h->strm) {
	 write_header(h->esps_hdr, h->strm);
	 h->esps_nbytes = ftell(h->strm);
	 return TRUE;
      } else
	 printf("Bad header in put_header()\n");
   } else
      printf("File is not open in put_header()\n");
   return (FALSE);
}

void
byte_swap(buf, nbyt)
   char           *buf;
   int             nbyt;
{
   register char  *p = buf - 1, c, *p2 = p + (nbyt & -2);
   while (++p < p2) {
      c = *p;
      *p = p[1];
      *++p = c;
   }
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
its_a_pipe(fd)
   int             fd;
{
   return ((fd >= 0) && (lseek(fd, 0L, SEEK_CUR) < 0));
}

#ifdef NOT_USING_ESPS_DEF
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
is_an_esps_header(fd)
   int             fd;
{
   int             ok = 0;

   if (fd >= 0) {
      int             here = lseek(fd, 0L, SEEK_CUR);
      int             newd = dup(fd);
      FILE           *s;
      struct header  *h = NULL;

      if (newd > 0) {
	 if ((s = fdopen(newd, "r"))) {
	    if ((h = read_header(s))) {
	       free_header(h, (long) 0, (char *) NULL);
	       ok = TRUE;
	    }
	    fclose(s);
	 } else
	    close(newd);
      }
#ifdef OS5
      lseek(fd, here, SEEK_SET);
#else
      lseek(fd, here, L_SET);
#endif
   }
   return (ok);
}
#endif

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*
 * Read the header from the file whose file descriptor is "file." Return the
 * loaded Header structure.
 */
Header         *
get_header(file)
   int             file;
{
   int             magic, nbytes, sum, type;
   Header         *h = NULL;
   char            count[8];
   FILE           *strm = NULL;

   if (debug_level >= 1)
      printf("get_header: function entered; file descriptor %d.\n", file);
   if (file >= 0) {
      /* first we try to read an ESPS header */
      if (its_a_pipe(file) || is_an_esps_header(file)) {
	 struct header  *hd;

	 strm = fdopen(file, "r");
	 if (!strm) {
	    printf("get_header: Can't make stream for file descriptor.\n");
	    return (NULL);
	 }
	 hd = read_header(strm);

	 if (hd == NULL && debug_level >= 1)
	    printf("get_header: couldn't read ESPS header; maybe it's another kind.\n");

	 if (hd != NULL) {	/* process as ESPS file */
	    if (debug_level >= 1)
	       if (hd->common.type == FT_FEA)
		  printf("get_header: this is an ESPS file, type FT_FEA, subtype %s.\n",
			 fea_file_type[hd->hd.fea->fea_type]);
	       else
		  printf("get_header: this is an ESPS file, type %s.\n",
			 file_type[hd->common.type]);

	    h = w_new_header(0);
	    if (!h) {
	       printf("get_header: Can't create Waves header structure.\n");
	       return (NULL);
	    }
	    h->magic = ESPS_MAGIC;
	    h->esps_nbytes = ftell(strm);
	    if (debug_level >= 1)
	       printf("get_header: h->esps_nbytes = %d.\n", h->esps_nbytes);
	    h->e_scrsd = 0;
	    h->e_short = 0;
	    h->strm = strm;
	    /*
	     * here's the place to catch FT_SD and stick in a FEA_SD header
	     */
	    /* unless we want old-style SD to result in old-style SD */
	    h->esps_hdr = hd;
            h->esps_clean = 1;
	    return h;
	 }
	 fclose(strm);
	 return (NULL);
      } else {			/* Not an ESPS file */
#ifdef OS5
	 lseek(file, 0L, SEEK_SET);
#else
	 lseek(file, 0L, L_SET);
#endif
	 if ((read(file, &magic, 4) == 4)) {
	    if (debug_level >= 1)
	       printf("get_header: magic = %d.\n", magic);
	    if (magic == SIGNAL_MAGIC) {
	       if ((read(file, count, 8) == 8) && (count[7] == '\n')) {
		  nbytes = 0;
		  count[7] = 0;
		  if (strlen(count))
		     sscanf(count, "%d", &nbytes);
		  if (nbytes) {
		     if ((h = w_new_header(nbytes))
			 && (read(file, h->header, nbytes) == nbytes)) {
			h->header[nbytes] = 0;
			return (h);
		     } else
			printf("Can't allocate header and read file in get_header()\n");
		  } else
		     printf("Zero header size in get_header()\n");
	       } else
		  printf("Can't read header count in get_header()\n");
	       return (NULL);
	    }
	    switch (magic) {
#ifdef IBM_RS6000
	    case -0xfeff01:
#else
	    case HEADER_MAGIC:
#endif
	       {

#define tran4(x) \
    (magic == RHEADR_MAGIC ? \
	(int)(	 ((((unsigned) x)&0xff000000)>>24) \
		|((((unsigned) x)&0x00ff0000)>>8) \
		|((((unsigned) x)&0x0000ff00)<<8) \
		|((((unsigned) x)&0x000000ff)<<24)) \
	:(int)(  ((((unsigned) x)&0xffff0000)>>16) \
		|((((unsigned) x)&0x000000ff)<<16)))

		  sum = 4;
		  while ((read(file, &nbytes, 4) == 4) &&
			 (read(file, &type, 4) == 4)) {
		     type = tran4(type);
		     nbytes = tran4(nbytes);
		     if (nbytes < 8)
			nbytes = 8;
		     sum += nbytes;
		     if (type == 0)
			break;
#ifdef OS5
		     if (nbytes > 8)
			lseek(file, nbytes - 8, SEEK_CUR);
#else
		     if (nbytes > 8)
			lseek(file, nbytes - 8, L_INCR);
#endif
		  }
#ifdef OS5
		  lseek(file, 4, SEEK_SET);
#else
		  lseek(file, 4, L_SET);
#endif
		  if (h = w_new_header(sum)) {
		     if ((nbytes = read(file, h->header, sum)) != sum) {
			printf("Trouble reading Nirvonics header");
			break;
		     }
		     if (magic == RHEADR_MAGIC)
			byte_swap(h->header, sum);
		     h->magic = magic;
		     return (h);
		  } else
		     printf("Can't allocate header and read file in get_header()\n");
	       }
	       break;
	    default:		/* Check if it is an SPPACK header */
	       {
		  Sppack         *sphdr;

		  lseek(file, 0, 0);
		  if (!(sphdr = (Sppack *) malloc(sizeof(Sppack)))) {
		     printf("Can't allocate Sppack in get_header()\n");
		     return (NULL);
		  }
		  if ((read(file, sphdr, sizeof(Sppack)) == sizeof(Sppack))) {
		     if (sphdr->magic == SPPACK_MAGIC) {
			if ((h = w_new_header(0))) {
			   h->header = (char *) sphdr;
			   h->magic = sphdr->magic;
			   return (h);
			} else
			   printf("Can't allocate header and read file in get_header()\n");
			return (NULL);
		     }
		  }
	       }
	       /* printf("Unknown header type: %d in get_header()\n"); */
	       return (NULL);
	    }
	 } else
	    printf("Can't read the file\n");
      }
   } else
      printf("Bad file descriptor in get_header()\n");

   if (h)
      free(h);

   return (NULL);
}

/* ----------------------------------------------------------      */
char           *
get_date()
{
   time_t          tim, time();
   char           *date, *ctime(), *c;

   tim = time((long *) 0);
   if ((c = date = ctime(&tim))) {
      while (*c != '\n')
	 c++;
      *c = 0;
   }
   return (date);
}

/*
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 
 */
update_hdr_for_mod(sig)		/* add date & comment for modification */
   Signal         *sig;
{
   char            comment[256];

   head_printf(sig->header, "time", get_date());
   sprintf(comment, "modify_signal: signal %s", sig->name);
   head_printf(sig->header, "operation", comment);
   if (sig->header->magic == ESPS_MAGIC) {
      sprintf(comment, "%s\n", get_date());
      add_comment(sig->header->esps_hdr, comment);
      sprintf(comment, "modify_signal: signal %s\n", sig->name);
      add_comment(sig->header->esps_hdr, comment);
   }
   sig->header->esps_clean = 0;
   return;
}

/*
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 
 */
/*
 * These are currently only used in xspectrum.   If an ESPS file that is
 * really a NIST sphere file is close, it's header must be also closed or
 * else a memory leak results.
 */
int
is_feasd_sphere(sig)
   Signal         *sig;
{
   struct header  *esps_hdr = NULL;

   if (sig) {
      if (sig->header)
	 esps_hdr = sig->header->esps_hdr;

      if (esps_hdr &&
	  (esps_hdr->common.type == FT_FEA) &&
	  (esps_hdr->hd.fea->fea_type == FEA_SD) &&
	  get_genhd_val("sphere_hd_ptr", esps_hdr, (double) 0.0))
	 return 1;
   }
   return 0;
}

int
close_feasd_sphere(sig)
   Signal         *sig;
{
   struct header  *esps_hdr = NULL;
   unsigned long   tmp;

   if (!sig)
      return;

   if (sig->header)
      esps_hdr = sig->header->esps_hdr;

   if (esps_hdr) {
      tmp = (unsigned long) get_genhd_val("sphere_hd_ptr",
					  esps_hdr, (double) 0.0);
      sp_close(tmp);
   }
   return 1;
}

int
is_sphere_tempfile(sig)
   Signal         *sig;
{
   struct header  *esps_hdr = NULL;
   char           *sp;

   if (sig && is_feasd_sphere(sig)) {
      esps_hdr = sig->header->esps_hdr;
      sp = (char *) (long) get_genhd_val("sphere_hd_ptr", esps_hdr, 0.0);
      return sphere_temp(sp);
   } else
      return 0;
}
