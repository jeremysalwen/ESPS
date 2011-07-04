#if	!defined (YES)
#define	YES	1
#endif

#if	!defined (NO)
#define	NO	0
#endif

#if	!defined (EOS)
#define	EOS	'\0'
#endif

#if	!defined (TAB)
#define	TAB	'\t'
#endif

#if	!defined (NULL)
#define	NULL	0
#endif

#if	defined (CTRL)
#undef	CTRL
#endif
#define	CTRL(x)	((x) & 0xbf)

#define	LOW_GCHAR	' '
#define	HIGH_GCHAR	'~'

#ifdef	DEBUG
#define	ENTER(name)	static char *rname = "name";			\
			{						\
			Level++;					\
			fprintf (stderr, "%*s%s\n", -(Level*4), "-",	\
				 rname);				\
			fflush (stderr);				\
			}
#define	RETURN(x)	{Level--; return (x);}
#define	EXIT		{Level--; return; }
#else
#define	ENTER(name)
#define	RETURN		return
#define	EXIT		return
#endif

#define	when		break;case
#define	otherwise	break;default

#ifndef	EXTERN
#define	EXTERN	extern
#endif

EXTERN	int	Level;		/* level of function call debug */
