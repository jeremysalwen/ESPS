/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1993 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Derek Lin
 * Checked by:
 * Revised by:
 *
 * Brief description: Public part of Bar widget class
 * @(#)Bar.h	1.2 6/18/93 ERL
 */

#ifndef BAR_H
#define BAR_H

#define XtNnumEntries "numEntries"
#define XtCNumEntries "NumEntries"
#define XtNvalues "values"
#define XtCValues "Values"
#define XtNmaxValue "maxValue"
#define XtCMaxValue "MaxValue"
#define XtNminValue "minValue"
#define XtCMinValue "MinValue"
#define XtNscale "scale"
#define XtCScale "Scale"
#define XtNdefaultBarWidth "defaultBarWidth"
#define XtCDefaultBarWidth "DefaultBarWidth"
#define XtNspace "space"
#define XtCSpace "Space"
#define XtNconstRC "constRC"
#define XtCConstRC "ConstRC"
#define XtNintv "intv"
#define XtCIntv "Intv"
#define XtNy "y"
#define XtCY "Y"
#define XtNx "x"
#define XtCY "Y"
#define XtNforegroundColor "foregroundColor"
#define XtCForegroundColor "ForegroundColor"
#define XtNbackgroundColor "backgroundColor"
#define XtCBackgroundColor "BackgroundColor"
#define XtNforegroundColorClip "foregroundColorClip"
#define XtCForegroundColorClip "ForegroundColorClip"
#define XtNbackgroundColorClip "backgroundColorClip"
#define XtCBackgroundColorClip "BackgroundColorClip"

extern WidgetClass XebarWidgetClass;
typedef struct _XeBarClassRec *XeBarWidgetClass;
typedef struct _XeBarRec *XeBarWidget;

#endif /* BAR_H */
