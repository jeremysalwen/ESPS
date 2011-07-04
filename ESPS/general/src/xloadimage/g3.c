/** g3.c - read a Group 3 FAX file and product a bitmap
 **
 ** Adapted from Paul Haeberli's <paul@manray.sgi.com> G3 to Portable Bitmap 
 ** code.
 **/

#ifdef SCCS
    static char *sccs_id = "@(#)g3.c	1.1  10/13/90";
#endif

#include "xloadimage.h"
#include "g3.h"

/****
 **
 ** Local defines
 **
 ****/

#define BITS_TO_BYTES(bits)	(bits/8)+((bits-((bits/8)*8)?1:0))
#define TABSIZE(tab) (sizeof(tab)/sizeof(struct tableentry))

/****
 **
 ** Local variables
 **
 ****/

int g3_eof = 0;
int g3_eols;
int g3_rawzeros;
int	maxlinelen;
int	rows, cols;

/****
 **
 ** Local tables
 **
 ****/

tableentry *whash[HASHSIZE];
tableentry *bhash[HASHSIZE];

int	g3_addtohash(hash, te, n, a, b)
	tableentry	*hash[];
	tableentry	*te;
	int	n, a, b;
{
	unsigned int pos;

	while (n--) {
		pos = ((te->length+a)*(te->code+b))%HASHSIZE;
		if (hash[pos] != 0) {
			fprintf(stderr, "G3: Hash collision during initialization.\n");
			exit(1);
			}
		hash[pos] = te;
		te++;
	}
}

tableentry	*g3_hashfind(hash, length, code, a, b)
	tableentry	*hash[];
	int	length, code;
	int	a, b;
{
	unsigned int pos;
	tableentry *te;

	pos = ((length+a)*(code+b))%HASHSIZE;
	if (pos >= HASHSIZE) {
		fprintf(stderr, "G3: Bad hash position, length %d code %d pos %d.\n", 
			length, code, pos);
		exit(2);
		}
	te = hash[pos];
	return ((te && te->length == length && te->code == code) ? te : 0);
}

int	g3_getfaxrow(fd, bitrow)
	ZFILE	*fd;
	byte	*bitrow;
{
	int col;
	int curlen, curcode, nextbit;
	int count, color;
	tableentry *te;

	/* First make the whole row white... */
	memset((char *) bitrow, NULL, maxlinelen); 

	col = 0;
	g3_rawzeros = 0;
	curlen = 0;
	curcode = 0;
	color = 1;
	count = 0;
	while (!g3_eof) {
		if (col >= MAXCOLS) {
			fprintf(stderr, "G3: Input row %d is too long, skipping to EOL.\n", rows);
			g3_skiptoeol(fd);
			return (col); 
			}
		do {
			if (g3_rawzeros >= 11) {
				nextbit = g3_rawgetbit(fd);
				if (nextbit) {
					if ( col == 0 )
						/* 6 consecutive EOLs mean end of document */
						g3_eof = (++g3_eols >= 5);
					else
						g3_eols = 0;

					return (col); 
					}
				}
			else
				nextbit = g3_rawgetbit(fd);

			curcode = (curcode<<1) + nextbit; 
			curlen++;
			} while (curcode <= 0);

		/* No codewords are greater than 13 bytes */
		if (curlen > 13) {
			fprintf(stderr, "G3: Bad code word at row %d, col %d (len %d code 0x%2.2x), skipping to EOL.\n", rows, col, curlen, curcode );
			g3_skiptoeol(fd);
			return (col);
			}
		if (color) {
			/* White codewords are at least 4 bits long */
			if (curlen < 4)
				continue;
			te = g3_hashfind(whash, curlen, curcode, WHASHA, WHASHB);
			}
		else {
			/* Black codewords are at least 2 bits long */
			if (curlen < 2)
				continue;
			te = g3_hashfind(bhash, curlen, curcode, BHASHA, BHASHB);
		}
		if (!te)
			continue;
		switch (te->tabid) {
			case TWTABLE:
			case TBTABLE:
				count += te->count;
				if (col+count > MAXCOLS) 
					count = MAXCOLS-col;
				if (count > 0) {
					if (color) {
						col += count;
						count = 0;
						}
					else
						g3_bitson(bitrow, col, count);
					}
				curcode = 0;
				curlen = 0;
				color = !color;
				break;
			case MWTABLE:
			case MBTABLE:
				count += te->count;
				curcode = 0;
				curlen = 0;
				break;
			case EXTABLE:
				count += te->count;
				curcode = 0;
				curlen = 0;
				break;
			default:
				fprintf(stderr, "G3: Bad table id from table entry.\n");
				exit(3);
			}
		}
	return (0);
}

int	g3_skiptoeol(fd)
	ZFILE	*fd;
{
	while (g3_rawzeros<11)
		(void) g3_rawgetbit(fd);
	while(!g3_rawgetbit(fd));
	return(0);
}

int	g3_rawgetbit(fd)
	ZFILE	*fd;
{
	int	b;
	static int	shdata;
	static int	curbit = 8;

	if (curbit >= 8) {
		shdata = zgetc(fd);
		if (shdata == EOF) {
			fprintf(stderr, "G3: Premature EOF at line %d.\n", rows);
			exit(4);
			}
		curbit = 0;
		}
	if (shdata & bmask[curbit]) {
		g3_rawzeros = 0;
		b = 1;
		}
	else {
		g3_rawzeros++;
		b = 0;
		}
	curbit++;
    return b;
}

int	g3_bitson(b, c, n)
	bit	*b;
	int	c, n;
{
	int	i, col;
	bit	*bP;

	bP = b;
	col = c;
	bP+=(c/8);
	i = (c - ((c/8)*8));
	while(col <= (c+n)) { 
		for(;col <= (c+n) && i < 8; i++) {
			*bP |= bmask[i];
			col++;
			}
		i = 0;
		bP++;
		}
	return(0);
}

int	g3_ident(fd)
	ZFILE	*fd;
{

	int	ret = 0;

	for (g3_rawzeros = 0; !g3_rawgetbit(fd););
	if(g3_rawzeros >=11 || g3_rawzeros <= 15)
		ret = 1;

	return(ret);

}

/* All G3 images begin with a G3 EOL codeword which is eleven binary 0's
 * followed by one binary 1.  There could be up to 15 0' so that the image
 * starts on a char boundary.
 */
int	g3Ident(fullname, name)
	char	*fullname, *name;
{
	ZFILE	*fd;
	int	ret;

	if ((fd = zopen(fullname)) == NULL)
		return(NULL);

	if (g3_ident(fd)) {
		printf("  %s is a G3 FAX image.\n", name);
		ret = 1;
		}

	zclose(fd);

	return(ret);
}

Image	*g3Load(fullname, name, verbose)
	char	*fullname, *name;
	unsigned int	verbose;
{

	ZFILE	*fd;
	Image	*image;
	int i, col;
	byte	*currline;

	if ((fd = zopen(fullname)) == NULL)
		return(NULL);

	if (!g3_ident(fd))
		return(NULL);
	if(verbose)
		printf("  %s is a G3 FAX image.\n", name);

	/* Initialize and load the hash tables */
	for ( i = 0; i < HASHSIZE; ++i )
		whash[i] = bhash[i] = (tableentry *) 0;
	g3_addtohash(whash, twtable, TABSIZE(twtable), WHASHA, WHASHB);
	g3_addtohash(whash, mwtable, TABSIZE(mwtable), WHASHA, WHASHB);
	g3_addtohash(whash, extable, TABSIZE(extable), WHASHA, WHASHB);
	g3_addtohash(bhash, tbtable, TABSIZE(tbtable), BHASHA, BHASHB);
	g3_addtohash(bhash, mbtable, TABSIZE(mbtable), BHASHA, BHASHB);
	g3_addtohash(bhash, extable, TABSIZE(extable), BHASHA, BHASHB);

	g3_eols = 0;

	/* Calulate the number of bytes needed for maximum number of columns 
	 * (bits), create a temprary storage area for it.
	 */
	maxlinelen = BITS_TO_BYTES(MAXCOLS);

	image = newBitImage(MAXCOLS, MAXROWS);

	currline = image->data;
	cols = 0;
	if(verbose) {
		printf("  Decoding image...");
		fflush(stdout);
		}
	for (rows = 0; rows < MAXROWS; ++rows) {
		col = g3_getfaxrow(fd, currline);
		if (g3_eof)
			break;
		if (col > cols)
			cols = col;
		currline += BITS_TO_BYTES(cols);
		}

	zclose(fd);
	image->title= dupString(name);
	image->width = cols;
	image->height = rows;

	if(verbose)
		printf("done.\n  Image is %dx%d.\n", image->width, image->height);
    return(image);
}
