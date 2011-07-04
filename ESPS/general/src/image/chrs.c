/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
| "Copyright (c) 1986, 1987 Entropic Speech, Inc. All rights reserved."	|
|									|
+-----------------------------------------------------------------------+
|									|
|  chrs -- encoded character-shape definitions for drawing text on	|
|	   plots							|
|									|
|  The ascii character definitions are used by the function text() in 	|
|  mlplot.c and genplot.c.						|
|									|
|  Rod Johnson, Entropic Speech, Inc.					|
|  Largely based on the text-drawing feature of the "pen" program by	|
|  Rob Clayton at Stanford University.					|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
    static char *sccs_id = "@(#)chrs.c	1.1	6/2/88	ESI";
#endif

/*
Each byte in a string encodes the coordinates of one of a
sequence of points that are connected with lines to draw a
character.  The horizontal coordinate can be extracted from
the byte with the octal mask 0160.  The mask 07 gets the
vertical coordinate.  The mask 0200 extracts a flag that means
the pen should be lifted and a new stroke begun.
*/

/* ascii alphabet */

char *ascii[] =
   {
/*32*/ " \320",
/*33*/ " \267\062\107\067\261\060\101\061",
/*34*/ " \246\045\266\065",
/*35*/ " \066\260\126\224\124\222\122",
/*36*/ " \222\041\101\122\103\043\024\045\105\124\266\040\306\060",
/*37*/ " \126\046\025\044\065\046\300\061\102\121\100",
/*38*/ " \321\100\040\021\022\043\103\124\125\106\066\045\044\120",
/*39*/ " \244\066",

/*40*/ " \300\061\065\106",
/*41*/ " \240\061\065\046",
/*42*/ " \221\125\225\121\260\066\223\123",
/*43*/ " \223\123\265\061",
/*44*/ "d\241\103\064\043\062",
/*45*/ " \223\123",
/*46*/ " \260\101\062\041\060",
/*47*/ " \126",

/*48*/ " \240\021\025\046\106\125\121\100\040",
/*49*/ " \245\066\060\240\100",
/*50*/ " \225\046\106\125\124\103\043\022\020\120\121",
/*51*/ " \221\040\100\121\122\103\043\303\124\125\106\046\025",
/*52*/ " \300\106\022\122",
/*53*/ " \221\040\100\121\122\103\023\026\126",
/*54*/ " \325\106\046\025\021\040\100\121\122\103\043\022",
/*55*/ " \226\126\040",

/*56*/ " \240\021\022\043\103\122\121\100\040\243\024\025\046\106\125\124\103",
/*57*/ " \221\040\100\121\125\106\046\025\023\042\102\123",
/*58*/ " \261\102\044\065\104\042\061",
/*59*/ "d\241\103\045\066\105\043\062",
/*60*/ " \325\023\121",
/*61*/ " \224\124\222\122",
/*62*/ " \225\123\021",
/*63*/ " \225\046\106\125\124\103\063\062\261\060",

/*64*/ " \261\102\104\065\044\042\061\101\122\125\106\046\025\021\040\100\121",
/*65*/ " \025\046\106\125\120\222\122",
/*66*/ " \026\106\125\124\103\023\303\122\121\100\020",
/*67*/ " \321\100\040\021\025\046\106\125",
/*68*/ " \026\106\125\121\100\020",
/*69*/ " \026\126\223\103\220\120",
/*70*/ " \026\126\223\103",
/*71*/ " \302\122\121\100\040\021\025\046\106\106\125",

/*72*/ " \026\223\123\326\120",
/*73*/ " \240\100\260\066\246\106",
/*74*/ " \221\040\060\101\106",
/*75*/ " \026\222\126\264\120",
/*76*/ " \226\020\120",
/*77*/ " \026\063\126\120",
/*78*/ " \026\120\126",
/*79*/ " \240\021\025\046\106\125\121\100\040",

/*80*/ " \026\106\125\124\103\023",
/*81*/ " \240\021\025\046\106\125\121\100\040\301\120",
/*82*/ " \026\106\125\124\103\023\263\120",
/*83*/ " \221\040\100\121\122\103\043\024\025\046\106\125",
/*84*/ " \226\126\266\060",
/*85*/ " \226\021\040\100\121\126",
/*86*/ " \226\060\126",
/*87*/ " \226\040\063\100\126",

/*88*/ " \126\226\120",
/*89*/ " \226\063\126\263\060",
/*90*/ " \226\126\020\120",
/*91*/ " \326\066\060\120",
/*92*/ " \226\120",
/*93*/ " \060\066\026",
/*94*/ " \224\066\124",
/*95*/ "d\201\141",

/*96*/ " \266\104",
/*97*/ " \240\021\023\044\104\123\121\121\100\040\324\120",
/*98*/ " \026\223\044\104\123\121\100\040\021",
/*99*/ " \323\104\044\023\021\040\100\121",
/*100*/ " \326\120\321\100\040\021\023\044\104\123",
/*101*/ " \321\100\040\021\023\044\104\123\122\022",
/*102*/ " \240\045\066\106\125\223\103",
/*103*/ "d\221\040\100\121\126\325\106\046\025\023\042\102\123",

/*104*/ " \026\223\044\104\123\120",
/*105*/ " \260\064\266\067",
/*106*/ "d\221\040\060\101\105\306\107",
/*107*/ " \026\222\124\263\120",
/*108*/ " \260\066",
/*109*/ " \024\223\044\063\060\263\104\123\120",
/*110*/ " \024\223\044\104\123\120",
/*111*/ " \240\021\023\044\104\123\121\100\040",

/*112*/ "d\226\020\225\046\106\125\123\102\042\023",
/*113*/ "d\320\126\325\106\046\025\023\042\102\123",
/*114*/ " \024\223\044\104\123",
/*115*/ " \221\040\100\121\102\042\023\044\104\123",
/*116*/ " \300\060\041\045\224\104",
/*117*/ " \224\021\040\100\121\324\120",
/*118*/ " \224\060\124",
/*119*/ " \224\040\064\100\124",

/*120*/ " \124\224\120",
/*121*/ "d\240\100\121\126\323\102\042\023\026",
/*122*/ " \224\124\020\120",
/*123*/ " \326\106\065\064\043\062\061\100\120",
/*124*/ " \267\065\262\060",
/*125*/ " \040\061\062\103\064\065\046\026",
/*126*/ " \224\045\104\125",
/*127*/ " \320",
   };

/* partial greek alphabet & some other special characters */

char *greek[] =
   {
/*32*/ " \320",
/*33*/ " \320",
/*34*/ " \320",
/*35*/ " \320",
/*36*/ " \320",
/*37*/ " \320",
/*38*/ " \320",
/*39*/ " \320",

/*40*/ " \320",
/*41*/ " \320",
/*42*/ " \320",
/*43*/ " \320",
/*44*/ " \320",
/*45*/ " \320",
/*46*/ " \320",
/*47*/ " \320",

/*48*/ " \320",
/*49*/ " \320",
/*50*/ " \320",
/*51*/ " \320",
/*52*/ " \320",
/*53*/ " \320",
/*54*/ " \320",
/*55*/ " \320",

/*56*/ " \320",
/*57*/ " \320",
/*58*/ " \320",
/*59*/ " \320",
/*60*/ " \320",
/*61*/ " \320",
/*62*/ " \320",
/*63*/ " \320",

/*64*/ " \320",
/*65*/ " \320",
/*66*/ " \320",
/*67*/ " \260\026\126\060",
/*68*/ " \066\120\020",
/*69*/ " \320",
/*70*/ " \120\260\066\226\126\301\122\124\105\045\024\022\041\101",
/*71*/ " \026\126\125",

/*72*/ " \320",
/*73*/ " \240\061\066\107",
/*74*/ " \320",
/*75*/ " \320",
/*76*/ " \066\120",
/*77*/ " \320",
/*78*/ " \320",
/*79*/ " \221\024\046\106\125\122\100\040\021\223\123",

/*80*/ " \226\126\246\040\306\100",
/*81*/ " \320",
/*82*/ " \320",
/*83*/ " \325\126\026\063\020\120\121",
/*84*/ " \320",
/*85*/ " \320",
/*86*/ " \320",
/*87*/ " \040\041\023\024\046\106\124\123\101\100\120",

/*88*/ " \320",
/*89*/ " \226\045\023\042\102\123\105\126\266\060\240\100",
/*90*/ " \320",
/*91*/ " \320",
/*92*/ " \320",
/*93*/ " \320",
/*94*/ " \320",
/*95*/ " \320",

/*96*/ " \320",
/*97*/ " \324\060\040\021\023\044\064\120",
/*98*/ "d\220\046\067\107\126\125\104\045\304\103\062\043",
/*99*/ " \225\046\106\123\122\100\040\021\022\044\104",
/*100*/ " \326\107\067\046\123\122\100\040\021\022\044\104",
/*101*/ " \323\104\044\023\042\302\042\021\040\100\121",
/*102*/ "d\247\025\023\042\102\124\125\106\066\060",
/*103*/ "d\225\046\065\102\100\061\063\106\127\126",

/*104*/ "d\226\045\022\245\066\106\124\101",
/*105*/ " \320",
/*106*/ " \320",
/*107*/ " \320",
/*108*/ " \042\063\225\046\100\120",
/*109*/ "d\220\046\042\062\103\306\102\122",
/*110*/ " \223\044\040\061\104\065",
/*111*/ " \221\023\045\065\104\102\060\040\021\222\103",

/*112*/ " \040\044\223\044\105\126\305\101\120",
/*113*/ " \320",
/*114*/ "d\224\042\102\123\124\106\046\025\021\040",
/*115*/ " \264\102\101\060\040\022\023\044\124",
/*116*/ " \223\044\124\264\041\060\101",
/*117*/ " \320",
/*118*/ " \320",
/*119*/ " \244\023\021\040\061\263\061\100\121\123\104",

/*120*/ " \320",
/*121*/ "d\246\025\023\042\062\104\106\267\040",
/*122*/ " \320",
/*123*/ " \320",
/*124*/ " \320",
/*125*/ " \240\021\025\046\126\145\141\120\040\245\044\064\065\045\305\104\124\125\105\263\103\062\242\061\101\122",
/*126*/ " \240\021\025\046\126\145\141\120\040\245\044\064\065\045\305\104\124\125\105\263\103\062\241\062\102\121",
/*127*/ " \320",
   };
