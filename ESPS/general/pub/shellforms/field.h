#define MAXFIELD	200

struct	field	{
	unsigned char	f_line;		/* line number (1-24) */
	unsigned char	f_col;		/* column number (1-80) */
	unsigned char	f_off;		/* offset from beg of line */
	unsigned char	f_len;		/* field size (bytes) */
	unsigned	f_attr;		/* attributes */
#define	FA_AUTOTAB	0x01		/* move to next field at eof */
#define	FA_SELECTION	0x02		/* selection field */
#define	FA_BLOCK	0x04		/* block type */
#define	FA_NUMERIC	0x08		/* nuermic only field */
	char		**f_sel;	/* ptr to selection array */
	char		**f_help;	/* ptr to help message array */
	unsigned char	f_sno;		/* current selection index */
	char		*f_var;		/* variable to set at end */
	};
