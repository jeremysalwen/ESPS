/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1995  Entropic Research Laboratory, Inc. 
 *                   All rights reserved."
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * @(#)epaths.h	1.26 2/20/96 ERL
 *
 * Written by:  John Shore
 * Checked by:
 * Revised by:
 *
 * Brief description: contains cover macros for find_esps_file()
 *	and related function declarations.
 *
 */

#ifndef epaths_H
#define epaths_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

/* Position independence in ESPS and waves+ is achieved primarily
 * by use of find_esps_file().  The defines below are cover functions
 * for find_esps_file, used with specific default paths and environment
 * variables for important special cases.  
 */

/*
 * Function declarations.
 */

char *
find_esps_file ARGS((char *fullpath,
		     char *filename, char *defpath, char *env_var_name));

char *
build_esps_path ARGS((char *append_string));

char *
build_filename ARGS((char *into_string, char *filename, char *dirname));

char *
get_esps_base ARGS((char *base_name));


/*
 * Macro definitions.
 */
#define FIND_ESPS_MENU(a,b) \
  find_esps_file(a, b, ".:$ESPS_BASE/lib/menus", "ESPS_MENU_PATH")

#define FIND_ESPS_BIN(a,b) \
  find_esps_file(a, b, ".:$ESPS_BASE/bin", "PATH")

#define FIND_ESPS_PARAM_FILE(a,b) \
  find_esps_file(a, b, ".:$ESPS_BASE/lib/params", "ESPS_PARAMS_PATH")

#define FIND_ESPS_FILTER(a,b) \
  find_esps_file(a, b, ".:$ESPS_BASE/lib/filters", "ESPS_FILTERS_PATH")

#define FIND_ESPS_LIB(a,b) \
  find_esps_file(a, b, "$ESPS_BASE/lib", "ESPS_LIB_PATH")

#define FIND_ESPS_INPUT(a,b) \
  find_esps_file(a, b, ".:$ESPS_BASE/lib/files", "ESPS_INPUT_PATH")

#define FIND_WAVES_INPUT(a,b) \
  find_esps_file(a, b, ".:$ESPS_BASE/lib/waves/files", "WAVES_INPUT_PATH")

#define FIND_WAVES_LIB(a,b) \
  find_esps_file(a, b, "$ESPS_BASE/lib/waves", "WAVES_LIB_PATH")

#define FIND_WAVES_MENU(a,b) \
  find_esps_file(a, b, ".:$ESPS_BASE/lib/waves/menus", "WAVES_MENU_PATH")

#define FIND_WAVES_COMMAND(a,b) \
  find_esps_file(a, b, ".:$ESPS_BASE/lib/waves/commands", "WAVES_COMMAND_PATH")

#define FIND_WAVES_COLORMAP(a,b) \
  find_esps_file(a, b, ".:$ESPS_BASE/lib/waves/colormaps", "WAVES_COLORMAP_PATH")

#define FIND_WAVES_PROFILE(a,b) \
  find_esps_file(a, b,  "$HOME:$ESPS_BASE/lib/waves", "WAVES_PROFILE_PATH")

#define FIND_FAB2_BIN(a,b) \
  find_esps_file(a, b,  "$ESPS_BASE/32bin:$HOME", "DSP32_BIN_PATH")

#define FIND_SURF_BIN(a,b) \
  find_esps_file(a, b,  "$ESPS_BASE/32cbin", "DSP32C_BIN_PATH")

#define FIND_ARIELS32C_BIN(a,b) \
  find_esps_file(a, b,  "$ESPS_BASE/s32cbin", "ARIELS32C_BIN_PATH")

#define FIND_LSISC30_BIN(a,b) \
  find_esps_file(a, b,  "$ESPS_BASE/sc30bin", "LSISC30_BIN_PATH")

/* ************************************************************************ */
/* the following macros are for use by ERS */

#define FIND_ERS_COLORMAP(a,b) \
  find_esps_file(a, b, \
  "$HOME/ers_colormaps:$ERS_BASE/ers_colormaps:$ESPS_BASE/ers/ers_colormaps:./ers_colormaps", \
  "ERS_COLORMAP_PATH")

#define FIND_ERS_CONFIG(a,b) \
  find_esps_file(a, b, \
  "$HOME/ers_configs:$ERS_BASE/ers_configs:$ESPS_BASE/ers/ers_configs:./ers_configs", \
  "ERS_CONFIG_PATH")

#define FIND_ERS_TEXT(a,b) \
  find_esps_file(a, b, "$ERS_BASE/text:$ESPS_BASE/ers/text:./text", \
  "ERS_TEXT_PATH")

#define FIND_ERS_LSISC30_BIN(a,b) \
  find_esps_file(a, b,  "$ERS_BASE/sc30bin:$ESPS_BASE/ers/sc30bin:./sc30bin", \
  "ERS_LSISC30_BIN_PATH")

#define FIND_ERS_SURF_BIN(a,b) \
  find_esps_file(a, b,  "$ERS_BASE/32cbin:$ESPS_BASE/ers/32cbin:./32cbin", \
  "ERS_DSP32C_BIN_PATH")

#define FIND_ERS_ARIELS32C_BIN(a,b) \
  find_esps_file(a, b,  "$ERS_BASE/32cbin:$ESPS_BASE/ers/32cbin:./32cbin", \
  "ERS_ARIELS32C_BIN_PATH")


#ifdef __cplusplus
}
#endif

#endif /* epaths_H */
