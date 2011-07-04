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
 * Written by:  David Talkin
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)stubs.c	1.2	11/7/96	ERL";

#include <esps/esps.h>

open_label_font(){}

update_file_panel(){}

not_explicitly_named(){}

setup_output_dir(){}

get_default_label_font(){}

waves_send(){}

int w_verbose = 0, max_buff_bytes = 20000000;

extern char	*ProgName;
extern char	*Version;
extern char	*Date;

void
set_pvd(hdr)
    struct header   *hdr;
{
    (void) strcpy(hdr->common.prog, ProgName);
    (void) strcpy(hdr->common.vers, Version);
    (void) strcpy(hdr->common.progdate, Date);
}

