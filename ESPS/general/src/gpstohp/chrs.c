
/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)chrs.c	1.2	10/4/91	ESI/ERL";

/*----------------------------------------------------------------------+
|									|
|   chrs -- encoded character-shape definitions for drawing text on	|
|	    plots							|
|									|
|   The ascii character definitions are used by the function text()	|
|									|
|   Rod Johnson, Entropic Speech, Inc.					|
|   Largely based on the text-drawing feature of the "pen" program by	|
|   Rob Clayton at Stanford University.					|
|									|
+----------------------------------------------------------------------*/
/*
 * Each byte in a string encodes the coordinates of one of a
 * sequence of points that are connected with lines to draw a
 * character.  The horizontal coordinate can be extracted from
 * the byte with the octal mask 0160.  The mask 07 gets the
 * vertical coordinate.  The mask 0200 extracts a flag that means
 * the pen should be lifted and a new stroke begun.  The mask 010
 * extracts a flag that marks the last byte in a string.
 * An initial `d' in a string means the character has a descender,
 * and the vertical coordinates should be reduced by 2; an initial
 * blank marks a character with no descender.
 */

/* ascii alphabet */


char *ascii[] =
   {
/*32*/ " \330",
/*33*/ " \267\062\107\067\261\060\101\071",
/*34*/ " \246\045\266\075",
/*35*/ " \066\260\126\224\124\222\132",
/*36*/ " \222\041\101\122\103\043\024\045\105\124\266\040\306\070",
/*37*/ " \126\046\025\044\065\046\300\061\102\121\110",
/*38*/ " \321\100\040\021\022\043\103\124\125\106\066\045\044\130",
/*39*/ " \244\076",

/*40*/ " \300\061\065\116",
/*41*/ " \240\061\065\056",
/*42*/ " \221\125\225\121\260\066\223\133",
/*43*/ " \223\123\265\071",
/*44*/ "d\241\103\064\043\072",
/*45*/ " \223\133",
/*46*/ " \260\101\062\041\070",
/*47*/ " \136",

/*48*/ " \240\021\025\046\106\125\121\100\050",
/*49*/ " \245\066\060\240\110",
/*50*/ " \225\046\106\125\124\103\043\022\020\120\131",
/*51*/ " \221\040\100\121\122\103\043\303\124\125\106\046\035",
/*52*/ " \300\106\022\132",
/*53*/ " \221\040\100\121\122\103\023\026\136",
/*54*/ " \325\106\046\025\021\040\100\121\122\103\043\032",
/*55*/ " \226\126\050",

/*56*/ " \240\021\022\043\103\122\121\100\040\243\024\025\046\106\125\124\113",
/*57*/ " \221\040\100\121\125\106\046\025\023\042\102\133",
/*58*/ " \261\102\044\065\104\042\071",
/*59*/ "d\241\103\045\066\105\043\072",
/*60*/ " \325\023\131",
/*61*/ " \224\124\222\132",
/*62*/ " \225\123\031",
/*63*/ " \225\046\106\125\124\103\063\062\261\070",

/*64*/ " \261\102\104\065\044\042\061\101\122\125\106\046\025\021\040\100\131",
/*65*/ " \025\046\106\125\120\222\132",
/*66*/ " \026\106\125\124\103\023\303\122\121\100\030",
/*67*/ " \321\100\040\021\025\046\106\135",
/*68*/ " \026\106\125\121\100\030",
/*69*/ " \026\126\223\103\220\130",
/*70*/ " \026\126\223\113",
/*71*/ " \302\122\121\100\040\021\025\046\106\106\135",

/*72*/ " \026\223\123\326\130",
/*73*/ " \240\100\260\066\246\116",
/*74*/ " \221\040\060\101\116",
/*75*/ " \026\222\126\264\130",
/*76*/ " \226\020\120\131",
/*77*/ " \026\063\126\130",
/*78*/ " \026\120\136",
/*79*/ " \240\021\025\046\106\125\121\100\050",

/*80*/ " \026\106\125\124\103\033",
/*81*/ " \240\021\025\046\106\125\121\100\040\301\130",
/*82*/ " \026\106\125\124\103\023\263\130",
/*83*/ " \221\040\100\121\122\103\043\024\025\046\106\135",
/*84*/ " \226\126\266\070",
/*85*/ " \226\021\040\100\121\136",
/*86*/ " \226\060\136",
/*87*/ " \226\040\063\100\136",

/*88*/ " \126\226\130",
/*89*/ " \226\063\126\263\070",
/*90*/ " \226\126\020\130",
/*91*/ " \326\066\060\130",
/*92*/ " \226\130",
/*93*/ " \060\066\036",
/*94*/ " \224\066\134",
/*95*/ "d\221\171",

/*96*/ " \266\114",
/*97*/ " \240\021\023\044\104\123\121\121\100\040\324\130",
/*98*/ " \026\223\044\104\123\121\100\040\031",
/*99*/ " \323\104\044\023\021\040\100\131",
/*100*/ " \326\120\321\100\040\021\023\044\104\133",
/*101*/ " \321\100\040\021\023\044\104\123\122\032",
/*102*/ " \240\045\066\106\125\223\113",
/*103*/ "d\221\040\100\121\126\325\106\046\025\023\042\102\133",

/*104*/ " \026\223\044\104\123\130",
/*105*/ " \260\064\266\077",
/*106*/ "d\221\040\060\101\105\306\117",
/*107*/ " \026\222\124\263\130",
/*108*/ " \260\076",
/*109*/ " \024\223\044\063\060\263\104\123\130",
/*110*/ " \024\223\044\104\123\130",
/*111*/ " \240\021\023\044\104\123\121\100\050",

/*112*/ "d\226\020\225\046\106\125\123\102\042\033",
/*113*/ "d\320\126\325\106\046\025\023\042\102\133",
/*114*/ " \024\223\044\104\133",
/*115*/ " \221\040\100\121\102\042\023\044\104\133",
/*116*/ " \300\060\041\045\224\114",
/*117*/ " \224\021\040\100\121\134",
/*118*/ " \224\060\134",
/*119*/ " \224\040\064\100\134",

/*120*/ " \124\224\130",
/*121*/ "d\240\100\121\126\323\102\042\023\036",
/*122*/ " \224\124\020\130",
/*123*/ " \326\106\065\064\043\062\061\100\130",
/*124*/ " \267\065\262\070",
/*125*/ " \040\061\062\103\064\065\046\036",
/*126*/ " \224\045\104\135",
/*127*/ " \330",
   };

/* partial greek alphabet & some other special characters */

char *greek[] =
   {
/*32*/ " \330",
/*33*/ " \330",
/*34*/ " \330",
/*35*/ " \330",
/*36*/ " \330",
/*37*/ " \330",
/*38*/ " \330",
/*39*/ " \330",

/*40*/ " \330",
/*41*/ " \330",
/*42*/ " \330",
/*43*/ " \330",
/*44*/ " \330",
/*45*/ " \330",
/*46*/ " \330",
/*47*/ " \330",

/*48*/ " \330",
/*49*/ " \330",
/*50*/ " \330",
/*51*/ " \330",
/*52*/ " \330",
/*53*/ " \330",
/*54*/ " \330",
/*55*/ " \330",

/*56*/ " \330",
/*57*/ " \330",
/*58*/ " \330",
/*59*/ " \330",
/*60*/ " \330",
/*61*/ " \330",
/*62*/ " \330",
/*63*/ " \330",

/*64*/ " \330",
/*65*/ " \330",
/*66*/ " \330",
/*67*/ " \260\026\126\070",
/*68*/ " \066\120\030",
/*69*/ " \330",
/*70*/ " \120\260\066\226\126\301\122\124\105\045\024\022\041\111",
/*71*/ " \026\126\135",

/*72*/ " \330",
/*73*/ " \240\061\066\117",
/*74*/ " \330",
/*75*/ " \330",
/*76*/ " \066\130",
/*77*/ " \330",
/*78*/ " \330",
/*79*/ " \221\024\046\106\125\122\100\040\021\223\133",

/*80*/ " \226\126\246\040\306\110",
/*81*/ " \330",
/*82*/ " \330",
/*83*/ " \325\126\026\063\020\120\131",
/*84*/ " \330",
/*85*/ " \330",
/*86*/ " \330",
/*87*/ " \040\041\023\024\046\106\124\123\101\100\130",

/*88*/ " \330",
/*89*/ " \226\045\023\042\102\123\105\126\266\060\240\110",
/*90*/ " \330",
/*91*/ " \330",
/*92*/ " \330",
/*93*/ " \330",
/*94*/ " \330",
/*95*/ " \330",

/*96*/ " \330",
/*97*/ " \324\060\040\021\023\044\064\130",
/*98*/ "d\220\046\067\107\126\125\104\045\304\103\062\053",
/*99*/ " \225\046\106\123\122\100\040\021\022\044\114",
/*100*/ " \326\107\067\046\123\122\100\040\021\022\044\114",
/*101*/ " \323\104\044\023\042\302\042\021\040\100\131",
/*102*/ "d\247\025\023\042\102\124\125\106\066\070",
/*103*/ "d\225\046\065\102\100\061\063\106\127\136",

/*104*/ "d\226\045\022\245\066\106\124\111",
/*105*/ " \330",
/*106*/ " \330",
/*107*/ " \330",
/*108*/ " \042\063\225\046\100\130",
/*109*/ "d\220\046\042\062\103\306\102\132",
/*110*/ " \223\044\040\061\104\075",
/*111*/ " \221\023\045\065\104\102\060\040\021\222\113",

/*112*/ " \040\044\223\044\105\126\305\101\130",
/*113*/ " \330",
/*114*/ "d\224\042\102\123\124\106\046\025\021\050",
/*115*/ " \264\102\101\060\040\022\023\044\134",
/*116*/ " \223\044\124\264\041\060\111",
/*117*/ " \330",
/*118*/ " \330",
/*119*/ " \244\023\021\040\061\263\061\100\121\123\114",

/*120*/ " \330",
/*121*/ "d\246\025\023\042\062\104\106\267\050",
/*122*/ " \330",
/*123*/ " \330",
/*124*/ " \330",
/*125*/ " \240\021\025\046\126\145\141\120\040\245\044\064\065\045\305\104\124\125\105\263\103\062\242\061\101\132",
/*126*/ " \240\021\025\046\126\145\141\120\040\245\044\064\065\045\305\104\124\125\105\263\103\062\241\062\102\131",
/*127*/ " \330",
   };
