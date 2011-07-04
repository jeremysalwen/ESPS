/*
 * This material contains unpublished, proprietary software of
 * Entropic Research Laboratory, Inc. Any reproduction, distribution,
 * or publication of this work must be authorized in writing by Entropic
 * Research Laboratory, Inc., and must bear the notice:
 *
 *    "Copyright (c) 1990  Entropic Speech, Inc.
 *    "Copyright (c) 1990-1994  Entropic Research Laboratory, Inc.
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 *
 * Written by:  Bill Byrne
 * Checked by:
 * Revised by:  Rod Johnson
 *
 * Brief description:
 * fft_cepstrum:    computes power cepstrum of complex sequence
 * fft_cepstrum_r:  computes power cepstrum of real sequence
 * fft_ccepstrum:   computes complex cepstrum of complex sequence
 * fft_ccepstrum_r: computes complex cepstrum of real sequence
 *
 */

static char *sccs_id = "@(#)fft_cepstrum.c	1.7	5/31/95	ESI/ERL";

#include <stdio.h>
#include <math.h>
#include <esps/esps.h>
#include <esps/constants.h>

void get_cfft();
void get_cfft_inv();
void get_rfft();
void get_fft_inv();
void unwrap();
void unwrapc();

double modulf();

extern int debug_level;

double log();
#ifndef IBM_RS6000
double atan2();
#endif

#define SMALLVAL 1E-35
#define Log(x) ( ((x)<SMALLVAL) ? -80.5905 : log((x)) )
#define Atan2(x,y) ( ((x)!=0.0&&(y)!=0.0) ? atan2(x,y) : atan2_except((x),(y)) )
double atan2_except(x,y)
double x, y;
{
    if ( x == 0.0 ) {
	if (y < 0.0)
	    return PI;
	return( 0.0 );
    }
    if ( y == 0.0 && x > 0.0 )
	return( PI / 2.0 );
    return( - PI / 2.0 );
}

double modulf(data)
float_cplx data;
{
    double res;
    double sqrt();

    res = (double) data.real * (double) data.real;
    res += (double) data.imag * (double) data.imag;
    res = sqrt(res);

    return(res);
}

void
fft_cepstrum( data, order)
float_cplx *data;
long order;
{
    static int l_order=0, tlen=0;
    int j ;

    spsassert( data != NULL && order > 0, "fft_cepstrum called with null data.");

    if (order != l_order) {
	l_order = order;
	tlen = 1 << l_order;
    }

    get_cfft( data, (int) order);

    for (j=0; j<tlen; j++) {
	data[j].real  = (float) Log( modulf(data[j]) );
	data[j].imag = 0.0;
    }

    get_cfft_inv( data, (int) order);

    return;
}

void
fft_cepstrum_r( data_r, data_i, order)
float *data_r, *data_i;
long order;
{
    static int l_order=0, tlen=0;
    double dummy;
    float fdummy;
    int j ;

    spsassert( data_r != NULL && data_i != NULL && order > 0,
	      "fft_cepstrum_r called with null data.");


    if (order != l_order) {
	l_order = order;
	tlen = 1 << l_order;
    }

    get_rfft( data_r, data_i, (int) order);

    for (j=0; j<tlen; j++) {
	dummy = (double) data_r[j] * (double) data_r[j]
	    + (double) data_i[j] * (double) data_i[j];
	data_r[j] = (float) Log(dummy)/2.0;
	data_i[j] = 0.0;
    }

    get_fft_inv( data_r, data_i, (int) order);

    return;
}


void
fft_ccepstrum( data, order)
float_cplx *data;
long order;
{
    static int l_order=0;
    static long tlen=0;
    int j ;
    double rr, ii;

    void get_cfft();
    void get_cfft_inv();

    spsassert( data != NULL && order > 0,
	      "fft_ccepstrum called with null data.");

    if (order != l_order) {
	l_order = order;
	tlen = 1 << l_order;
    }

    get_cfft( data, (int) order);
    for (j=0; j<tlen; j++) {
	rr = modulf(data[j]);
	rr  = Log( (double) rr);
	ii = Atan2( (double) data[j].imag, (double) data[j].real);
	data[j].real = (float) rr;
	data[j].imag = (float) ii;
    }
    unwrapc( data, tlen);
    get_cfft_inv( data, (int) order);
    return;
}

void
fft_ccepstrum_r( data_r, data_i, order)
float *data_r, *data_i;
long order;
{
    static int l_order=0, tlen=0;
    int j ;
    double rr, ii;

    spsassert( data_r != NULL && data_i != NULL && order > 0,
	      "fft_ccepstrum_r called with null data.");

    if (order != l_order) {
	l_order = order;
	tlen = 1 << l_order;
    }
    get_rfft( data_r, data_i, (int) order);

    for (j=0; j<tlen; j++) {
	rr =  ((double) data_r[j] * (double) data_r[j]
	       + (double) data_i[j] * (double) data_i[j]);
	rr =  Log( rr ) / 2.0;
	ii =  Atan2( (double) data_i[j], (double) data_r[j]);
	data_r[j] = (float) rr;
	data_i[j] = (float) ii;
    }

    unwrap( data_i, tlen);

    get_fft_inv(data_r, data_i, (int) order);

    return;
}


void
unwrap( phase, length)
float *phase;
int length;
{
    int j;
    int no_err = 1;
    float adjust=0.0;
    float m0, hold_phase, correction, ph0;

    double fabs();
    double floor();


    spsassert( length != 0 && phase != NULL,
	      "Unwrapping routine called with null parameters.");

    adjust = 2.0 * PI;

    correction = 0.0;
    hold_phase = phase[0];
    for (j=0; j<length-1; j++) {
	if ( (phase[j+1] - hold_phase) > PI)
	    correction = correction - adjust;
	if ( (hold_phase - phase[j+1]) > PI )
	    correction = correction + adjust;
	hold_phase = phase[j+1];
	phase[j+1] += correction;
    }

    m0 = floor(0.5 + ((double) phase[length/2] - phase[0]) / PI);
    correction = 2.0 * m0 * PI / (double) length;
    ph0 = phase[0];
    for (j=0; j<length; j++)
	phase[j] -= ph0 + j*correction;

    if ( no_err && debug_level) {
	for (j=0; j<length-1; j++)
	    if ( fabs( (double) (phase[j]-phase[j+1])) > PI ) {
		Fprintf( stderr, "Phase unwrapping failed.\n");
		no_err = 0;
		break;
	    }
	if (no_err && (fabs((double) (phase[length-1] - phase[0])) > PI)) {
	    Fprintf(stderr, "Phase unwrapping failed.\n");
	    no_err = 0;
	}
    }

    return;
}

void
unwrapc( data, length)
float_cplx *data;
long length;
{
    int j;
    static int no_err = 1;
    static float adjust=0.0;
    float m0, hold_phase, correction;

    double fabs();
    double floor();

    spsassert( length != 0 && data != NULL,
	      "Unwrapping routine called with null parameters.");

    if (adjust == 0)
	adjust = 2.0 * PI;

    correction = 0.0;
    hold_phase = data[0].imag;
    for (j=0; j<length-1; j++) {
	if ( (data[j+1].imag - hold_phase) > PI)
	    correction = correction - adjust;
	if ( (hold_phase - data[j+1].imag) > PI )
	    correction = correction + adjust;
	hold_phase = data[j+1].imag;
	data[j+1].imag += correction;
    }
    if (data[0].imag - hold_phase > PI)
	correction = correction - adjust;
    if (hold_phase - data[0].imag > PI)
	correction = correction + adjust;

    m0 = floor(0.5 + ((double) correction) / ((double) adjust));
    correction = 2.0 * m0 * PI / (double) length;
    for (j=0; j<length; j++)
	data[j].imag -= j * correction;

    if ( no_err && debug_level) {
	for (j=0; j<length-1; j++)
	    if ( fabs( (double) (data[j].imag - data[j+1].imag)) > PI ) {
		no_err = 0;
		Fprintf(stderr, "Phase unwrapping failed.\n");
		break;
	    }
	if (no_err &&
	    (fabs((double) (data[length-1].imag - data[0].imag)) > PI))
	{
	    no_err = 0;
	    Fprintf(stderr, "Phase unwrapping failed.\n");
	}
    }

    return;
}

#undef Log
#undef Atan2



