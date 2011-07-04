/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/* @(#)Utils.h	1.2 9/26/95 ATT/ERL/ESI */
/*
 * Utils.h == structure to hold methods for internal use within `waves';
 *	      each `Signal' contains one of these.
 *
 * Not to be confused with `Methods' which coordinate the execution
 * of external commands (i.e. messages).
 */

typedef struct {

  int (*write_data)();		/* signal I/O routines */
  int (*read_data)();
  int (*free_data)();		/* special freeing methods (data specific) */
  double (*sig_dur)();		/* signal duration of entire file */
  double (*buf_start)();	/* start time of data in memory */
  double (*buf_end)();		/* end time of data in memory */
  double (*index_to_time)();    /* convert array index to time */
  int (*time_to_index)();	/* convert time to buffer array index */
} Utils;


/*
 * These macros use the above utility methods to replace the old macros
 * START_TIME, END_TIME, and DURATION.
 */

#define BUF_END_TIME(sig)	((sig)->utils->buf_end(sig))
#define BUF_START_TIME(sig)	((sig)->utils->buf_start(sig))
#define BUF_DURATION(sig)	(BUF_END_TIME(sig)-BUF_START_TIME(sig))
#define SIG_DURATION(sig)	(((sig)->utils->sig_dur)? \
				 (sig)->utils->sig_dur(sig) : \
				 (double)(sig)->file_size/(sig)->freq)
#define SIG_END_TIME(sig)       ((sig)->start_time + SIG_DURATION(sig))
#define SIG_START_TIME(sig)     ((sig)->start_time)
