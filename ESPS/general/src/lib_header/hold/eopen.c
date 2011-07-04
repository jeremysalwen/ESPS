/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
|    "Copyright (c) 1987 Entropic Speech, Inc. All rights reserved."	|
|									|
+-----------------------------------------------------------------------+
|									|
|  eopen - open ESPS file, read header, and check type			|
|									|
|  Rodney W. Johnson							|
|									|
|  This utility performs a number of tasks usually required when ESPS	|
|  programs open files.  The function attempts to open a named file.	|
|  If opening the file for reading, the function attempts to read the	|
|  header and check the type; if the file is a feature file, the func-	|
|  tion also attempts to check the subtype.  A file name of "-" is	|
|  treated specially as meaning standard input or standard output.	|
|  In case of error the program prints an error message and exits       |
|									|  
|  Modified to return a FEA_SD file header if input is an old SD        |
|  header and the caller asked for a FEA_SD file.                       |
|									|  
+----------------------------------------------------------------------*/
#ifdef SCCS
static char *sccsid = "@(#)eopen.c	1.7 1/3/93 ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>

struct header *sdtofea();


char *
eopen(prog_name, file_name, mode, type, subtype, header, stream)
    char    *prog_name,
	    *file_name,
	    *mode;
    int	    type,
	    subtype;
    struct header   **header;
    FILE    **stream;
{
    if (strcmp(file_name, "-") != 0)
    {
	if ((*stream = fopen(file_name, mode)) == NULL)
	{
	    Fprintf(stderr, "%s: can't open ", prog_name);
	    perror(file_name);
	    exit(1);
	}
    }
    else
    if (mode[0] == 'r')
    {
	file_name = "<stdin>";
	*stream = stdin;
    }
    else
    {
	file_name = "<stdout>";
	*stream = stdout;
    }

    if (mode[0] == 'r')
    {
	if ((*header = read_header(*stream)) == NULL)
	{
	    Fprintf(stderr, "%s: %s is not an ESPS file.\n",
		    prog_name, file_name);
	    exit(1);
	}

/* if we are looking for a FEA_SD file and we found an old FT_SD file, 
   then convert it to a FEA_SD file and continue 
*/

	if ((*header)->common.type == FT_SD 
	    && type == FT_FEA && subtype == FEA_SD)
	{
	    (*header) = sdtofea((*header));
	    
	} else

	if (type != NONE)
	{
	    if ((*header)->common.type != type)
	    {
		Fprintf(stderr, "%s: %s is not an ESPS %s file.\n",
			prog_name, file_name, file_type[type] + 3);
		    /* "+ 3" to skip initial "FT_" of type symbol */
		exit(1);
	    }

	    if ((*header)->common.type == FT_FEA && subtype != NONE)
	    {
		if ((*header)->hd.fea->fea_type != subtype)
		{
		    Fprintf(stderr, "%s: %s is not a %s file.\n",
			    prog_name, file_name, fea_file_type[subtype]);
		    exit(1);
		}
	    }
	}
    } /* end if (mode[0] == 'r') */

    return file_name;
}



char *
eopen2(prog_name, file_name, mode, type, subtype, header, stream)
    char    *prog_name,
	    *file_name,
	    *mode;
    int	    type,
	    subtype;
    struct header   **header;
    FILE    **stream;
{
    if (strcmp(file_name, "-") != 0)
    {
	if ((*stream = fopen(file_name, mode)) == NULL)
	{
	    Fprintf(stderr, "%s: can't open ", prog_name);
	    *header = NULL;
	    return(file_name);
	}
    }
    else
    if (mode[0] == 'r')
    {
	file_name = "<stdin>";
	*stream = stdin;
    }
    else
    {
	file_name = "<stdout>";
	*stream = stdout;
    }

    if (mode[0] == 'r')
    {
	if ((*header = read_header(*stream)) == NULL)
	{
	    Fprintf(stderr, "%s: %s is not an ESPS file.\n",
		    prog_name, file_name);
	    return(file_name);
	}

/* if we are looking for a FEA_SD file and we found an old FT_SD file, 
   then convert it to a FEA_SD file and continue 
*/

	if ((*header)->common.type == FT_SD 
	    && type == FT_FEA && subtype == FEA_SD)
	{
	    (*header) = sdtofea((*header));
	    
	} else

	if (type != NONE)
	{
	    if ((*header)->common.type != type)
	    {
		Fprintf(stderr, "%s: %s is not an ESPS %s file.\n",
			prog_name, file_name, file_type[type] + 3);
		    /* "+ 3" to skip initial "FT_" of type symbol */
		*header = NULL;
		return(file_name);
	    }

	    if ((*header)->common.type == FT_FEA && subtype != NONE)
	    {
		if ((*header)->hd.fea->fea_type != subtype)
		{
		    Fprintf(stderr, "%s: %s is not a %s file.\n",
			    prog_name, file_name, fea_file_type[subtype]);
		    *header = NULL;
		    return(file_name);
		}
	    }
	}
    } /* end if (mode[0] == 'r') */

    return file_name;
}



