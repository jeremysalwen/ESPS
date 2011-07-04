/* Last update: 01/13/88  10:43 AM  (Edition: 5) */
#include	<stdio.h>
#include	"basic.h"
/*----------------------------------------------------------------------+
|									|
|	fill_keyword : fill the field with possible matched keyword	|
|									|
+----------------------------------------------------------------------*/
fill_keyword (buf, len, list)
char		*buf;
unsigned	len;			/* size of input field */
char		**list;
	{
	int		i, j;
	unsigned	flen;		/* field length */
	unsigned char	match[80];
 
	ENTER (fill_keyword);
	flen = fldlen (buf, len);
 
	for (i=j=0; list[i] != NULL; i++) {
		if (kwcmp (buf, list[i], flen) == 0) {
			/* find a match */
			match[j++] = i;
			}
		}
	if (j == 1) bcopy (buf, list[match[0]], len);
	else if (j > 1)	 bcopy (buf, list[match[0]], flen);
 
	RETURN (j);
	}
 
/*----------------------------------------------------------------------+
|									|
|	kwcmp : compare a word with keyword (case insensitive)		|
|									|
+----------------------------------------------------------------------*/
kwcmp (src, dest, len)
char		*src;
char		*dest;
unsigned	len;
	{
	ENTER (kwcmp);
	while (len-- != 0) {
		if ((*src++ | 0x20) != (*dest++ | 0x20))
			 RETURN (1);			/* no match */
		}
	RETURN (0);		/* match found */
	}
