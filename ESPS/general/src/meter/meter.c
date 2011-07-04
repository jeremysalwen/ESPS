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
 * Brief description: A front-end record control panel for ESPS record programs
 *
 */

static char *sccs_id = "@(#)meter.c	1.4	1/7/94	ERL";

#include <stdio.h>
#include <math.h>
#include <sys/types.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <Xm/Label.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include "Bar.h"

#include <esps/esps.h>
#include <esps/feasd.h>
#include <esps/fea.h>

#define SYNTAX USAGE("meter [-i update_time] [-c RC_const] [-s] [-m] [-r] [-z] [-] outfile")

#define TIMEOUT 0.1 /* time out inverval in seconds */
#define TIME_DC 20  /* DC level is computed from TIME_DC * TIMEOUT sec */

/* tag messages */
#define MRecording "Recording...."
#define MNot_Recording "    "
#define MPause "pause"
#define MResume "resume"
#define MRestart "Re-start"
#define MStart "Start"
#define MQuit "Stop/Quit"

int debug_level = 0;

void get_input();
static float *find_max();
static void do_dc_offset();
double log10();

/* callback list */
static void start_record();
static void pause_resume();
static void quit();
static void dc_offset_onoff();
static void meter_onoff();
static void dc_read_onoff();

/* global variables */
static int START = False;
static int RESTART = False;
static int PAUSE = True;
static int DC_OFFSET = False;
static int METER = True;
static int METER_READOUT = True;
static int QUIT = False;
static float INTERVAL = 0.1;

static Widget Bar[2];
Widget dbval_label[2];
Widget startB, pauseB, quitB, rc_label;

XmFontList myfontlist;  /*need be accessed in other */
XmFontList myfontlist2;

char *outfilename;
int magic = 0;

main(argc, argv)
     int argc;
     char *argv[];
{
  Widget toplevel;
  Widget backdrop, formM, rcpush, rctog, formB, formR;
  Widget ch_label[2];
  Widget chanT, chTb1, chTb2, dc_offB, meterB, dc_readB;

  Arg wargs[10];
  int n;
  XmString mystring;

  Dimension label_w, label_h;  /* label */
  Dimension met_w, met_h;
  Dimension tmpd;

  XFontStruct *font1, *font2;

  char c;
  float RC = -0.1;
  extern int optind;
  extern char *optarg;

  while((c=getopt(argc,argv,"i:c:smrz"))!= EOF){
    switch(c){
    case 'i':
      INTERVAL = atof(optarg);
      break;
    case 'c':
      RC = -1 * (float) atof(optarg);
      if(RC > 0) {
	fprintf(stderr,"meter: -c argument must be positive - exiting\n");
	exit(1);
      }
      break;
    case 's':
      DC_OFFSET = True;
      break;
    case 'm':
      METER = False;
      break;
    case 'r':
      METER_READOUT = False;
      break;
    case 'z':
      magic = 1;
      break;
    default:
      SYNTAX;
    }
  }
  
  if( argc <= 1)
    SYNTAX;

  outfilename = argv[argc - 1];
  if( 0==strcmp(argv[argc-2], "-")) argv[argc-2] = "a";

  toplevel = XtInitialize( argv[0], "Meter", NULL, 0, &argc, argv);

  font1 = XLoadQueryFont(XtDisplay(toplevel), "-adobe-helvetica-medium-r-normal--14-140-75-75-p-77-iso8859-1");
  myfontlist = XmFontListCreate(font1, "chset1");

  font2 = XLoadQueryFont(XtDisplay(toplevel), "-adobe-utopia-bold-i-normal--15-140-75-75-p-82-iso8859-1");
  myfontlist2 = XmFontListCreate(font2, "chset2");

  n = 0;
  XtSetArg( wargs[n], XmNx, 100); n++;
  XtSetArg( wargs[n], XmNy, 100); n++;
  XtSetArg( wargs[n], XmNresizable, False); n++;
  backdrop =
    XtCreateManagedWidget("backdrop",
			  xmFormWidgetClass, toplevel,
			  wargs, n);
  
  n = 0;
  XtSetArg( wargs[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg( wargs[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg( wargs[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  formB = XtCreateManagedWidget("formB", xmFormWidgetClass, backdrop,
				 wargs, n);

  n = 0;
  XtSetArg( wargs[n], XmNorientation, XmVERTICAL); n++;
  XtSetArg( wargs[n], XmNnumColumns, 1); n++;
  XtSetArg( wargs[n], XmNspacing, 5); n++;
  rcpush = XtCreateManagedWidget("rcpush", xmRowColumnWidgetClass, formB,
				 wargs, n);

  n = 0;
  mystring = XmStringCreateLtoR(MStart, "chset1");
  XtSetArg( wargs[n], XmNshadowThickness, 5); n++;
  XtSetArg( wargs[n], XmNfontList, myfontlist); n++;
  XtSetArg( wargs[n], XmNlabelString, mystring); n++;
  startB = XtCreateManagedWidget("startB", xmPushButtonWidgetClass, rcpush,
				 wargs, n);
  XmStringFree(mystring);
  XtAddCallback(startB, XmNactivateCallback, start_record, NULL);

  n = 0;
  mystring = XmStringCreateLtoR(MPause, "chset1");
  XtSetArg( wargs[n], XmNshadowThickness, 5); n++;
  XtSetArg( wargs[n], XmNfontList, myfontlist); n++;
  XtSetArg( wargs[n], XmNlabelString, mystring); n++;
  XtSetArg( wargs[n], XmNsensitive, False); n++;
  pauseB = XtCreateManagedWidget("pauseB", xmPushButtonWidgetClass, rcpush,
				 wargs, n);
  XmStringFree(mystring);
  XtAddCallback(pauseB, XmNactivateCallback, pause_resume, NULL);


  mystring = XmStringCreateLtoR("dc-offset", "chset1");
  n = 0;
  XtSetArg( wargs[n], XmNfontList, myfontlist); n++;
  XtSetArg( wargs[n], XmNlabelString, mystring); n++;
  XtSetArg( wargs[n], XmNshadowThickness, 0); n++;
  XtSetArg( wargs[n], XmNset, DC_OFFSET); n++;
  dc_offB = XmCreateToggleButton(rcpush, "dc_offB", wargs, n);
  XtAddCallback(dc_offB, XmNarmCallback, dc_offset_onoff, NULL);
  XtManageChild(dc_offB);
  XmStringFree(mystring);


  mystring = XmStringCreateLtoR("Meter", "chset1");
  n = 0;
  XtSetArg( wargs[n], XmNfontList, myfontlist); n++;
  XtSetArg( wargs[n], XmNlabelString, mystring); n++;
  XtSetArg( wargs[n], XmNshadowThickness, 0); n++;
  XtSetArg( wargs[n], XmNset, METER); n++;
  meterB = XmCreateToggleButton(rcpush, "meterB", wargs, n);
  XtAddCallback(meterB, XmNarmCallback, meter_onoff, NULL);
  XtManageChild(meterB);
  XmStringFree(mystring);


  mystring = XmStringCreateLtoR("Meter dB readout", "chset1");
  n = 0;
  XtSetArg( wargs[n], XmNfontList, myfontlist); n++;
  XtSetArg( wargs[n], XmNlabelString, mystring); n++;
  XtSetArg( wargs[n], XmNshadowThickness, 0); n++;
  XtSetArg( wargs[n], XmNset, METER_READOUT); n++;
  dc_readB = XmCreateToggleButton(rcpush, "dc_readB", wargs, n);
  XtAddCallback(dc_readB, XmNarmCallback, dc_read_onoff, NULL);
  XtManageChild(dc_readB);
  XmStringFree(mystring);

  n = 0;
  mystring = XmStringCreateLtoR(MQuit, "chset1");
  XtSetArg( wargs[n], XmNshadowThickness, 5); n++;
  XtSetArg( wargs[n], XmNfontList, myfontlist); n++;
  XtSetArg( wargs[n], XmNlabelString, mystring); n++;
  quitB = XtCreateManagedWidget("quitB", xmPushButtonWidgetClass, rcpush,
				 wargs, n);
  XmStringFree(mystring);
  XtAddCallback(quitB, XmNactivateCallback, quit, NULL);


  n = 0;
  XtSetArg( wargs[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg( wargs[n], XmNtopPosition, 10); n++;
  XtSetArg( wargs[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
  XtSetArg( wargs[n], XmNleftWidget, formB); n++;
  XtSetArg( wargs[n], XmNleftOffset, 20); n++;
  formM = 
    XtCreateManagedWidget("formM",
			  xmFormWidgetClass, backdrop,
			  wargs, n);

  mystring = XmStringCreateLtoR("Chan. 1", "chset1");
  n = 0;
  XtSetArg( wargs[n], XmNfontList, myfontlist); n++;
  XtSetArg( wargs[n], XmNlabelString, mystring); n++;
  XtSetArg( wargs[n], XmNmarginBottom, 6); n++;
  XtSetArg( wargs[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg( wargs[n], XmNleftAttachment, XmATTACH_FORM); n++;
  ch_label[0] = XtCreateManagedWidget("label", xmLabelWidgetClass,
				 formM, wargs,n);
  XmStringFree(mystring);

  mystring = XmStringCreateLtoR("Chan. 2", "chset1");
  n = 0;
  XtSetArg( wargs[n], XmNfontList, myfontlist); n++;
  XtSetArg( wargs[n], XmNlabelString, mystring); n++;
  XtSetArg( wargs[n], XmNmarginBottom, 6); n++;
  XtSetArg( wargs[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
  XtSetArg( wargs[n], XmNleftOffset, 10); n++;
  XtSetArg( wargs[n], XmNleftWidget, ch_label[0]); n++;
  ch_label[1] = XtCreateManagedWidget("label", xmLabelWidgetClass,
				 formM, wargs,n);
  XmStringFree(mystring);

  n = 0;
  XtSetArg( wargs[n], XtNwidth, &label_w); n++;
  XtSetArg( wargs[n], XtNheight, &label_h); n++;
  XtGetValues(ch_label[0], wargs, n);
  met_w = 15;
  met_h = 150;
  n = 0;
  XtSetArg( wargs[n], XmNheight, met_h); n++;
  XtSetArg( wargs[n], XmNwidth, met_w); n++;
  XtSetArg( wargs[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
  XtSetArg( wargs[n], XmNtopWidget, ch_label[0]); n++;
  tmpd = (Dimension ) (label_w/2) - (met_w/2);
  XtSetArg( wargs[n], XmNleftOffset, tmpd); n++;
  XtSetArg( wargs[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg( wargs[n], XtNconstRC, &RC); n++;
  XtSetArg( wargs[n], XtNintv, &INTERVAL); n++;
  Bar[0] = XtCreateManagedWidget("bar", XebarWidgetClass,
			      formM,
			      wargs, n);

  n = 0;
  XtSetArg( wargs[n], XmNheight, met_h); n++;
  XtSetArg( wargs[n], XmNwidth, met_w); n++;
  XtSetArg( wargs[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
  XtSetArg( wargs[n], XmNtopWidget, ch_label[1]); n++;
  tmpd = (Dimension ) (label_w/2) - (met_w/2);
  XtSetArg( wargs[n], XmNrightOffset, tmpd); n++;
  XtSetArg( wargs[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetArg( wargs[n], XtNconstRC, &RC); n++;
  XtSetArg( wargs[n], XtNintv, &INTERVAL); n++;
  Bar[1] = XtCreateManagedWidget("bar", XebarWidgetClass,
			      formM,
			      wargs, n);

  mystring = XmStringCreateLtoR("0", "chset1");
  n = 0;
  XtSetArg( wargs[n], XmNfontList, myfontlist); n++;
  XtSetArg( wargs[n], XmNlabelString, mystring); n++;
  XtSetArg( wargs[n], XmNmarginBottom, 6); n++;
  XtSetArg( wargs[n], XmNmarginTop, 6); n++;
  XtSetArg( wargs[n], XmNalignment, XmALIGNMENT_END); n++;
  XtSetArg( wargs[n], XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET); n++;
  XtSetArg( wargs[n], XmNrightWidget, Bar[0]); n++;
  XtSetArg( wargs[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
  XtSetArg( wargs[n], XmNtopWidget, Bar[0]); n++;
  dbval_label[0] = XtCreateManagedWidget("label", xmLabelWidgetClass,
				 formM, wargs,n);
  XmStringFree(mystring);

  mystring = XmStringCreateLtoR("0", "chset1");
  n = 0;
  XtSetArg( wargs[n], XmNfontList, myfontlist); n++;
  XtSetArg( wargs[n], XmNlabelString, mystring); n++;
  XtSetArg( wargs[n], XmNmarginBottom, 6); n++;
  XtSetArg( wargs[n], XmNmarginTop, 6); n++;
  XtSetArg( wargs[n], XmNalignment, XmALIGNMENT_END); n++;
  XtSetArg( wargs[n], XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET); n++;
  XtSetArg( wargs[n], XmNrightWidget, Bar[1]); n++;
  XtSetArg( wargs[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
  XtSetArg( wargs[n], XmNtopWidget, Bar[1]); n++;
  dbval_label[1] = XtCreateManagedWidget("label", xmLabelWidgetClass,
				 formM, wargs,n);
  XmStringFree(mystring);

  mystring = XmStringCreateLtoR(MNot_Recording, "chset2");
  n = 0;
  XtSetArg( wargs[n], XmNfontList, myfontlist2); n++;
  XtSetArg( wargs[n], XmNlabelString, mystring); n++;
  XtSetArg( wargs[n], XmNmarginBottom, 6); n++;
  XtSetArg( wargs[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
  XtSetArg( wargs[n], XmNtopOffset, 5); n++;
  XtSetArg( wargs[n], XmNtopWidget, dbval_label[0]); n++;
  XtSetArg( wargs[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  XtSetArg( wargs[n], XmNbottomOffset, 10); n++;
  rc_label = XtCreateManagedWidget("rc_label", xmLabelWidgetClass,
				   formM, wargs,n);
  XmStringFree(mystring);

  XtAddInput( fileno(stdin), XtInputReadMask, get_input, NULL);
  XtRealizeWidget(toplevel);

  XtMainLoop();
}

static struct header *ihd, *ohd;   /* input and output file headers */
static struct feasd *isd_rec, *z_rec; /* record for input and output data */
static FILE *isdfile = stdin,         /* input and output file stream */
  *osdfile = stdout;  
static int buffer_len;             /* no. samps read by get_feasd_rec */

void get_input(client_data, fid, id)
     caddr_t client_data;
     int *fid;
     XtInputId *id;
{
  int truncate();
  static int first_time = 1;

  static long offset;
  static char *iname = NULL,                /* input and output file names */
         *oname = NULL;
  static int num_chan = 1;
  static int num_segs;               /* no. update in buffer_len long samples*/
  static int nsamp_in_seg;           /* no. samps in an update */
  static float time = 0;

  int i,j;
  float *values = NULL;
  
  Arg args[10];
  int n;
  long num_rec;

  char string[10];
  XmString mystring;

  if(first_time){
    oname = eopen("meter",outfilename,"w",NONE, NONE, &ohd, &osdfile);
    iname = eopen("meter", "-","r", FT_FEA, FEA_SD, &ihd, &isdfile);
    num_chan = get_fea_siz("samples", ihd, (short *)NULL,(long **)NULL);
    write_header( ihd, osdfile );
    offset = ftell(osdfile);
    nsamp_in_seg = TIMEOUT * (*get_genhd_d("record_freq", ihd));

    num_segs = 1;
    buffer_len = num_segs * nsamp_in_seg;

    if( magic ){
      short *zeros, **zeros_ptr;
      if( num_chan == 1){
	isd_rec = allo_feasd_recs( ihd, SHORT, buffer_len, (char*) NULL, NO);
	z_rec = allo_feasd_recs( ihd, SHORT, 1, (char*) NULL, NO);
	zeros = (short *) z_rec->data;
	for(i=0; i<1; i++) zeros[i] = SHRT_MAX;
      }
      else{
	isd_rec = allo_feasd_recs( ihd, SHORT, buffer_len, (char*) NULL, YES);
	z_rec = allo_feasd_recs( ihd, SHORT, 1, (char*) NULL, YES);
	zeros_ptr = (short**) z_rec->ptrs;
	for(j=0; j<num_chan; j++)
	  for(i=0; i<1; i++) zeros_ptr[i][j] = SHRT_MAX;
      }
    }
    else{
      if( num_chan == 1)
	isd_rec = allo_feasd_recs( ihd, SHORT, buffer_len, (char*) NULL, NO);
      else
	isd_rec = allo_feasd_recs( ihd, SHORT, buffer_len, (char*) NULL, YES);
    }

    first_time = 0;
  }

  if((num_rec = get_feasd_recs( isd_rec, 0L, buffer_len, ihd, isdfile )) == 0){
    fclose(osdfile);
    fclose(isdfile);
    exit(0);
  }
  if(DC_OFFSET == True)
    (void) do_dc_offset( (short *) isd_rec->data,
			(short **) isd_rec->ptrs , num_rec, num_chan);

  if(METER == True || METER_READOUT == True)
    values =  (float *) find_max( (short *) isd_rec->data,
				 (short **) isd_rec->ptrs , num_rec, num_chan);

  if(time >= INTERVAL && ((METER==True) || METER_READOUT==True)){
    for( i = 0; i< num_chan; i++){
      if( values[i] >= SHRT_MAX )
	fprintf(stderr,"ERROR: clipping occured.\n");
      values[i] =  20. * log10( values[i] + 1.01);
      if(METER == True){
	n = 0;
	XtSetArg( args[n], XtNvalues, values+i); n++;
	XtSetValues( Bar[i], args, n);
      }
      if(METER_READOUT == True){
	n = 0;
	sprintf(string, "%3.0f", values[i]);
	mystring = XmStringCreateLtoR(string, "chset1");
	XtSetArg( args[n], XmNlabelString, mystring); n++;
	XtSetValues( dbval_label[i], args, n);
	XmStringFree(mystring);
      }
    }
    time = 0;
  }
  else
    time += TIMEOUT;
  if( PAUSE == False){
    if( RESTART == False && START == True){   /* append */
      put_feasd_recs( isd_rec, 0L, buffer_len, ihd, osdfile);
    }
    if( RESTART == True){      /* move the file pointer to beginning */
      fseek( osdfile, offset, 0);
      if(0> truncate( oname, offset)){
	fprintf(stderr,"ERROR: Can't do ftruncate() for Re-start\n");
	exit(1);
      }
      put_feasd_recs( isd_rec, 0L, buffer_len, ihd, osdfile);
      RESTART = False;
    }
  }
}  


static void do_dc_offset( array, array2, size_array, chan_size)
     short *array;
     short **array2;
     int size_array;
     int chan_size;
{
  static long sum[] = {0,0,0,0};  /* sum1, cnt1, sum2, cnt2 */
  static short time_dc_cnt = 0;
  static int dc_offset[2];
  register int i;

  if(chan_size == 1){
    register short *ptr = array;
    register long *sum_ptr = sum;
    register int *dc_ptr = dc_offset;

    if( time_dc_cnt++ == TIME_DC ){
      dc_ptr[0] = sum_ptr[0]/sum_ptr[1];
      time_dc_cnt = 0;
      sum_ptr[0] = 0; sum_ptr[1] = 0;
    }
    for(i = 0; i< size_array; i++){
      sum_ptr[0] += ptr[i];
      sum_ptr[1]++;
      ptr[i] = ptr[i] - dc_ptr[0];
    }
  }
  else {
    register short **ptr2 = array2;
    register long *sum_ptr = sum;
    register int *dc_ptr = dc_offset;

    if( time_dc_cnt++ == TIME_DC ){
      dc_ptr[0] =  sum_ptr[0]/sum_ptr[1];
      dc_ptr[1] = sum_ptr[2]/sum_ptr[3];
      time_dc_cnt = 0;
      sum_ptr[0] = 0; sum_ptr[2] = 0;
      sum_ptr[1] = 0; sum_ptr[3] = 0;
    }
    
    for(i = 0; i< size_array; i++){
      sum_ptr[0] += ptr2[i][0];
      sum_ptr[2] += ptr2[i][1];
      sum_ptr[1]++;
      sum_ptr[3]++;
      
      ptr2[i][0] = ptr2[i][0] - dc_ptr[0];
      ptr2[i][1] = ptr2[i][1] - dc_ptr[1];
    }
  }
}

static float *find_max( array, array2, size_array, chan_size)
     short *array;
     short **array2;
     int size_array;
     int chan_size;
{
  register int i;
  float max[2];

  if(chan_size == 1){
    register short *ptr;

    ptr = array;
    max[0] = -9999;

    for(i = 0; i< size_array; i+=3){
      if( ptr[i] <= 0) if( max[0] < -ptr[i]) max[0] = -ptr[i];
      if( ptr[i] > 0) if( max[0] < ptr[i]) max[0] = ptr[i];
    }
    return(max);
  }
  else {
    register short **ptr2;

    ptr2 = array2;
    max[0] = max[1] = -9999;

    for(i = 0; i< size_array; i+=3){
      if( ptr2[i][0] <= 0) if( max[0] < -ptr2[i][0]) max[0] = -ptr2[i][0];
      if( ptr2[i][0] > 0) if( max[0] < ptr2[i][0]) max[0] = ptr2[i][0];
      if( ptr2[i][1] <= 0) if( max[1] < -ptr2[i][1]) max[1] = -ptr2[i][1];
      if( ptr2[i][1] > 0) if( max[1] < ptr2[i][1]) max[1] = ptr2[i][1];
    }
    return(max);
  }
}


static void start_record(w, client_data, call_data)
     Widget w;
     caddr_t client_data;
     XmAnyCallbackStruct *call_data;
{
  Arg args[10];
  int n;
  XmString mystring;
  static firsttime = 1;

  if(firsttime == 1){
    START = True;
    RESTART = False;
    firsttime = 0;
  }
  else
    RESTART = True;

  if( RESTART == True) PAUSE = False;
  if( RESTART == False && START == True) PAUSE = False;

  mystring = XmStringCreateLtoR(MRestart, "chset1");
  n = 0;
  XtSetArg( args[n], XmNlabelString, mystring); n++;
  XtSetValues( startB, args, n);
  XmStringFree(mystring);

  mystring = XmStringCreateLtoR(MRecording, "chset2");
  n = 0;
  XtSetArg( args[n], XmNfontList, myfontlist2); n++;
  XtSetArg( args[n], XmNlabelString, mystring); n++;
  XtSetValues( rc_label, args, n);
  XmStringFree(mystring);

  n = 0;
  mystring = XmStringCreateLtoR(MPause, "chset1");
  XtSetArg( args[n], XmNlabelString, mystring ); n++;
  XtSetValues( pauseB, args, n);
  XmStringFree(mystring);

  n = 0;
  XtSetArg( args[n], XmNsensitive, True); n++;
  XtSetValues( pauseB, args, n);
}


static void pause_resume(w, client_data, call_data)
     Widget w;
     caddr_t client_data;
     XmAnyCallbackStruct *call_data;
{
  Arg args[10];
  int n;
  XmString mystring;

  PAUSE = (PAUSE == True) ? False : True;
  if( PAUSE == True ){

    if(magic) put_feasd_recs( z_rec, 0L, 1, ihd, osdfile);

    n = 0;
    mystring = XmStringCreateLtoR(MResume, "chset1");
    XtSetArg( args[n], XmNlabelString, mystring ); n++;
    XtSetValues( pauseB, args, n);
    XmStringFree(mystring);

    mystring = XmStringCreateLtoR(MNot_Recording, "chset2");
    n = 0;
    XtSetArg( args[n], XmNfontList, myfontlist2); n++;
    XtSetArg( args[n], XmNlabelString, mystring); n++;
    XtSetValues( rc_label, args, n);
    XmStringFree(mystring);
  }
  else{
    n = 0;
    mystring = XmStringCreateLtoR(MPause, "chset1");
    XtSetArg( args[n], XmNlabelString, mystring); n++;
    XtSetValues( pauseB, args, n);
    XmStringFree(mystring);

    mystring = XmStringCreateLtoR(MRecording, "chset2");
    n = 0;
    XtSetArg( args[n], XmNfontList, myfontlist2); n++;
    XtSetArg( args[n], XmNlabelString, mystring); n++;
    XtSetValues( rc_label, args, n);
    XmStringFree(mystring);

  }

}

static void quit(w, client_data, call_data)
     Widget w;
     caddr_t client_data;
     XmAnyCallbackStruct *call_data;
{
/*  if( RESTART == False && START == True){
    put_feasd_recs( z_rec, 0L, buffer_len, ihd, osdfile);
  }
*/

  XtCloseDisplay(XtDisplay(w));
  exit(0);
}

static void dc_offset_onoff(w, client_data, call_data)
     Widget w;
     caddr_t client_data;
     XmAnyCallbackStruct *call_data;
{
  DC_OFFSET = ((XmToggleButtonCallbackStruct *) call_data)->set;
  DC_OFFSET = (DC_OFFSET == True)? False: True;
}


static void meter_onoff(w, client_data, call_data)
     Widget w;
     caddr_t client_data;
     XmAnyCallbackStruct *call_data;
{
  METER = ((XmToggleButtonCallbackStruct *) call_data)->set;
  METER = (METER == True)? False: True;
}


static void dc_read_onoff(w, client_data, call_data)
     Widget w;
     caddr_t client_data;
     XmAnyCallbackStruct *call_data;
{
  METER_READOUT = ((XmToggleButtonCallbackStruct *) call_data)->set;
  METER_READOUT = (METER_READOUT == True)? False: True;
}




