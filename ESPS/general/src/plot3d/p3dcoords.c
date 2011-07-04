/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1993  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Rodney W. Johnson, Entropic Speech, Inc.
 * Checked by:
 * Revised by:
 *
 * Brief description:  Handle coordinate transformations.
 *
 */

static char *sccs_id = "@(#)p3dcoords.c	1.13	6/24/93	ESI/ERL";
 
#include <stdio.h>
#include <math.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/constants.h>
#include "plot3d.h"


extern int  debug_level;
extern int  force_monochrome_plot;

extern void drawline();
extern void drawlines();
extern void drawpoint();
extern void drawing_style();

void	    update_mats();
void	    make_rmat(), update_rmat();
void	    make_imats();
int	    base_trans();

int	    can_width = 250,
	    can_height = 250;

int	    length = 150,
	    width = 250,
	    height = 400;
int	    box_dims[3] = {150, 250, 400};

double	    hskew = 0.0,
	    vskew = 0.0,
	    finv = 0.0005;		/* 1/finv = coord normal to screen
					   of center of projection.
					   (0 => parallel proj.) */
int	    ori = -1;			/* orientation (handedness) of axes.
					   -1 = left, 1 = right. */

double	    rot = 0.0,	    sin_psi = 0.0,	cos_psi = 1.0,
	    bear = 0.0,	    sin_theta = 0.0,	cos_theta = 1.0,
	    elev = 0.0,	    sin_phi = 0.0,	cos_phi = 1.0;
double	    xlow = 0.0,	    xhigh = 100.0,	xscale = 1.0,
	    ylow = 0.0,	    yhigh = 100.0,	yscale = 1.0,
	    zlow = 0.0,	    zhigh = 100.0,	zscale = 1.0;

		/* transformation matrices */

double	    tmat[3][4];		/* plot coordinates to canvas coordinates */
double	    smat[3][4];		/* data coordinates to canvas coordinates */
double	    rmat[3][3];		/* rotation matrix, used in updating rota-
				/* tion angles with mouse movements */
double	    amat[3][3];		/* axis-label coords to canvas coords */
double	    base_imat[3][3];	/* canvas to data coords in base plane */

double	    pmatx[3][4],	/* project on vertical planes in data */
	    pmaty[3][4];	/*     space, for checking visibility */
double	    pconstx, pconsty;	/* used with pmats; coordinates locating the
				   vertical planes */

				/* normal vectors to box faces */
int	    face_nor[6][3] =   {{-1,  0,  0}, { 1,  0,  0},
				{ 0, -1,  0}, { 0,  1,  0},
				{ 0,  0, -1}, { 0,  0,  1}};
				/* edge lists of box faces */
int	    face_edge[6][4] =  {{ 4,  8,  5,  9},
				{ 6, 11,  7, 10},
				{ 0, 10,  1,  8},
				{ 2,  9,  3, 11},
				{ 0,  4,  2,  6},
				{ 1,  7,  3,  5}};
				/* face lists of box edges */
int	    edge_face[12][2] = {{2, 4}, {2, 5}, {3, 4}, {3, 5},
				{0, 4}, {0, 5}, {1, 4}, {1, 5},
				{0, 2}, {0, 3}, {1, 2}, {1, 3}};
				/* vertex lists of box edges */
				/* directed low end to high end */
int	    edge_vert[12][2] = {{0, 4}, {1, 5}, {2, 6}, {3, 7},
				{0, 2}, {1, 3}, {4, 6}, {5, 7},
				{0, 1}, {2, 3}, {4, 5}, {6, 7}};
				/* edge parallel to x-axis (0), y-axis (1),
				   or z-axis (2)? */
int	    edge_dir[12] =     {0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2};
				/* vertex lists of box faces
				   (counter-clockwise order as seen
				   from outside) */
int	    face_vert[6][4] =  {{0, 1, 3, 2},
				{4, 6, 7, 5},
				{0, 4, 5, 1},
				{2, 3, 7, 6},
				{0, 2, 6, 4},
				{1, 5, 7, 3}};
				/* normalized coordinates of box vertices
				   (mult by {length, width, height}
				   to get coords in plot space) */
int	    vert_coor[8][3] =  {{ 0,  0,  0},
				{ 0,  0,  1},
				{ 0,  1,  0},
				{ 0,  1,  1},
				{ 1,  0,  0},
				{ 1,  0,  1},
				{ 1,  1,  0},
				{ 1,  1,  1}};

/* numbering of box's
 *
 *                faces                               edges
 *
 *        (0)
 *          \      (5)
 *         _________|____                      _____(5)______
 *        |\  \     |    \                    |\             \
 *        | \       |   | \                   | \           | \
 *        |  \  \          \                  | (1)           (3)
 *        |   \         |   \                (8)  \        (9)  \
 *        |    \ ____________\                |    \ _____(7)____\
 *        |     |       |     |               |     |       |     |
 *   (2)-----   |           - |--(3)          |     |             |
 *        |_  _ |_  _  _|     |               |_  _ |(4)_  _|     |
 *         \    |        \    |                \  (10)          (11)
 *          \   |             |                 \   |         \   |
 *           \  |       \  \  |                 (0) |         (2) |
 *            \ |   |    \    |                   \ |           \ |
 *             \|_________\__\|                    \|_____(6)_____|
 *                  |      \
 *                 (4)     (1)
 *
 *
 *
 *               vertices
 *
 *
 *       (1)___________(3)
 *        |\             \
 *        | \           | \
 *        |  \             \
 *        |   \         |   \
 *        |    (5)___________(7)              z
 *        |     |       |     |               |
 *        |     |             |               |
 *       (0)_  _| _  _ (2)    |               |_____y
 *         \    |        \    |                \
 *          \   |             |                 \
 *           \  |          \  |                  x
 *            \ |             |
 *             (4)___________(6)
 *
 */



void
set_canv_dimens(w, h)
    int	    w, h;
{
    can_width = w;
    can_height = h;
    update_mats();
}


void
set_box_len(n)
    int     n;
{
    length = box_dims[0] = n;
    xscale = length / (xhigh - xlow);
    update_mats();
}

int
get_box_len()
{
    return length;
}


void
set_box_wid(n)
    int     n;
{
    width = box_dims[1] = n;
    yscale = width / (yhigh - ylow);
    update_mats();
}

int
get_box_wid()
{
    return width;
}


void
set_box_hgt(n)
    int     n;
{
    height = box_dims[2] = n;
    zscale = height / (zhigh - zlow);
    update_mats();
}

int
get_box_hgt()
{
    return height;
}


void
set_hskew(x)
    double  x;
{
    hskew = x;
    update_mats();
}

double
get_hskew()
{
    return hskew;
}
   

void
set_vskew(x)
    double  x;
{
    vskew = x;
    update_mats();
}

double
get_vskew()
{
    return vskew;
}
   

void
set_finv(x)
    double x;
{
    finv = 0.00001*x;
    update_mats();
}

double
get_finv()
{
    return 100000.0*finv;
}


void
set_ori(n)
    int	    n;
{
    if (n == ORI_RIGHT)
	ori = 1;
    else
	ori = -1;

    update_mats();
}

int
get_ori()
{
    return (ori == 1) ? ORI_RIGHT : ORI_LEFT;
}

void
set_rot(x)
    double x;
{
    rot = x;
    sin_psi = sin(rot);
    cos_psi = cos(rot);
    update_mats();
}

double
get_rot()
{
    return rot;
}


void
set_bear(x)
    double x;
{
    bear = x;
    sin_theta = sin(bear);
    cos_theta = cos(bear);
    update_mats();
}

double
get_bear()
{
    return bear;
}


void
set_elev(x)
    double x;
{
    elev = x;
    sin_phi = sin(elev);
    cos_phi = cos(elev);
    update_mats();
}

double
get_elev()
{
    return elev;
}


void
update_mats()
{
    double  m[4][4];
    int	    i, j;
    double  ctr_x, ctr_y, ctr_z, ctr_u, ctr_v;

    /* matrix for rotation in xy-, xz-, and yz-planes (homogeneous coords) */
    /*
     *       /          0 \
     *   m = |   rmat   0 |
     *       |          0 |
     *       \ 0  0  0  1 /
     */

    make_rmat();
    for (i = 0; i < 3; i++)
    {
	for (j = 0; j < 3; j++)
	    m[i][j] = rmat[i][j];
	m[i][3] = 0.0;
    }
    for (j = 0; j < 3; j++)
	m[3][j] = 0.0;
    m[3][3] = 1.0;

    /* precede with orientation setting */
    /*
     *         / -1    0    0  0 \
     *   m = m |  0  - ori  0  0 |
     *         |  0    0    1  0 |
     *         \  0    0    0  1 /
     */

    for (i = 0; i < 3; i++)
    {
	m[i][0] = -m[i][0];
	m[i][1] = -ori*m[i][1];
    }

    /* ... and with translation of center of box to origin */
    /*
     *         / 1  0  0  - length/2 \
     *   m = m | 0  1  0  - width/2  |
     *         | 0  0  1  - height/2 |
     *         \ 0  0  0       1     /
     */

    ctr_x = 0.5*length;	ctr_y = 0.5*width;  ctr_z = 0.5*height;
    for (i = 0; i < 3; i++)
	m[i][3] = - m[i][0]*ctr_x - m[i][1]*ctr_y - m[i][2]*ctr_z;

    /* follow with projection into yz-plane */
    /*
     *       / - hskew  1  0  0 \
     *   m = | - vskew  0  1  0 | m
     *       \ - finv   0  0  1 /
     */

    for (j = 0; j < 4; j++)
    {
	m[1][j] -= hskew*m[0][j];
	m[2][j] -= vskew*m[0][j];
	m[3][j] -= finv*m[0][j];
    }

    /* map into canvas coordinates (translate origin to center of canvas) */
    /*
     *          / 1    0  can_width/2  \
     *   tmat = | 0  - 1  can_height/2 | m
     *          \ 0    0        1      /
     */

    ctr_u = 0.5*can_width;	ctr_v = 0.5*can_height;
    for (j = 0; j < 4; j++)
    {
	tmat[0][j] = m[1][j] + ctr_u*m[3][j];
	tmat[1][j] = - m[2][j] + ctr_v*m[3][j];
	tmat[2][j] = m[3][j];
    }

    if (debug_level >= 2)
	printmat("tmat", &tmat[0][0], 3, 4);

    /* tmat defined--now do smat */
    /* precede transformation with scaling from data units to plot units */
    /*
     *               / xscale     0       0    0 \
     *   smat = tmat |    0    yscale     0    0 |
     *               |    0       0    zscale  0 |
     *               \    0       0       0    1 /
     */

    for (i = 0; i < 3; i++)
    {
	smat[i][0] = tmat[i][0]*xscale;
	smat[i][1] = tmat[i][1]*yscale;
	smat[i][2] = tmat[i][2]*zscale;
	smat[i][3] = tmat[i][3];
    }
	
    if (debug_level >= 2)
	printmat("scale", &smat[0][0], 3, 4);

    /* precede that with translation of coordinate low limits to origin */
    /*
     *               / 1  0  0  - xlow \
     *   smat = smat | 0  1  0  - ylow |
     *               | 0  0  1  - zlow |
     *               \ 0  0  0     1   /
     */

    for (i = 0; i < 3; i++)
	smat[i][3] -= (smat[i][0]*xlow + smat[i][1]*ylow + smat[i][2]*zlow);

    if (debug_level >= 2)
	printmat("trans smat", &smat[0][0], 3, 4);

    make_imats();
}


void
make_rmat()
{
    double  t;
    int	    i, j;

    for (i = 0; i < 3; i++)
	for (j = 0; j < 3; j++)
	    rmat[i][j] = 0.0;

    /* rotation in xy-plane */
    /*
     *          /   cos theta  sin theta  0 \
     *   rmat = | - sin theta  cos theta  0 |
     *          \      0           0      1 /
     */

    rmat[0][0] = cos_theta;	rmat[0][1] = sin_theta;
    rmat[1][0] = -sin_theta;	rmat[1][1] = cos_theta;
    rmat[2][2] = 1.0;

    /* rotation in xz-plane */
    /*
     *          /   cos phi  0  sin phi \
     *   rmat = |     0      1     0    | rmat
     *          \ - sin phi  0  cos phi /
     */

    for (j = 0; j < 3; j++)
    {
	t = cos_phi*rmat[0][j] + sin_phi*rmat[2][j];
	rmat[2][j] = - sin_phi*rmat[0][j] + cos_phi*rmat[2][j];
	rmat[0][j] = t;
    }

    /* rotation in yz-plane */
    /*
     *          / 1      0         0    \
     *   rmat = | 0    cos psi  sin psi | rmat
     *          \ 0  - sin psi  cos psi /
     */

    for (j = 0; j < 3; j++)
    {
	t = cos_psi*rmat[1][j] + sin_psi*rmat[2][j];
	rmat[2][j] = - sin_psi*rmat[1][j] + cos_psi*rmat[2][j];
	rmat[1][j] = t;
    }

    if (debug_level >= 2)
    {
	(void) fprintf(stderr, "make_rmat:\n  sin theta %g,\tcos theta %g\n",
			sin_theta, cos_theta);
	(void) fprintf(stderr, "  sin phi   %g,\tcos phi   %g\n",
			sin_phi, cos_phi);
	(void) fprintf(stderr, "  sin psi   %g,\tcos psi   %g\n",
			sin_psi, cos_psi);
	printmat("rmat", &rmat[0][0], 3, 3);
    }
	
}


void
update_rmat(th, ph, theta, phi, psi)
    double  th, ph, *theta, *phi, *psi;
{
    if (th == 0.0 && ph == 0.0)
    {
	*theta = bear;	*phi = elev;	*psi = rot;
    }
    else
    {
	double	r, c, s, t, ps, dif;
	int	j;

	r = sqrt(th*th + ph*ph);
	th /= r;	ph /= r;
	c = cos(r);	s = sin(r);

	/* transform axis of rotation to z axis */
	/*
	 *          / 1   0    0 \
	 *   rmat = | 0   th  ph | rmat
	 *          \ 0  -ph  th /
	 */
	
	for (j = 0; j < 3; j++)
	{
	    t = th*rmat[1][j] + ph*rmat[2][j];
	    rmat[2][j] = -ph*rmat[1][j] + th*rmat[2][j];
	    rmat[1][j] = t;
	}

	/* do the rotation */
	/*
	 *          /  c  s  0 \
	 *   rmat = | -s  c  0 | rmat
	 *          \  0  0  1 /
	 */
	
	for (j = 0; j < 3; j++)
	{
	    t = c*rmat[0][j] + s*rmat[1][j];
	    rmat[1][j] = -s*rmat[0][j] + c*rmat[1][j];
	    rmat[0][j] = t;
	}

	/* transform z axis back to rotation axis */
	/*
	 *          / 1   0   0  \
	 *   rmat = | 0  th  -ph | rmat
	 *          \ 0  ph   th /
	 */

	for (j = 0; j < 3; j++)
	{
	    t = th*rmat[1][j] - ph*rmat[2][j];
	    rmat[2][j] = ph*rmat[1][j] + th*rmat[2][j];
	    rmat[1][j] = t;
	}

	if (debug_level >= 2)
	{
	    (void) fprintf(stderr, "update_rmat:  th %g, ph %g\n", th, ph);
	    printmat("rmat", &rmat[0][0], 3, 3);
	}

	/* solve for new Euler angles */

	*phi = asin(rmat[0][2]);

	s = rmat[1][2];		/* sin psi cos phi */
	c = rmat[2][2];		/* cos psi cos phi */
	ps = (s == 0.0 && c == 0.0) ? 0.0 : atan2(s, c);
				/* approx. psi */

	s = rmat[0][1];	/* cos phi sin theta */
	c = rmat[0][0];	/* cos phi cos theta */
	th = (s == 0.0 && c == 0.0) ? 0.0 : atan2(s, c);
				/* approx. theta */

	/* psi and theta may be poorly determined if cos phi is small;
	 * but their sum or difference is well determined.
	 */
/*!*//* if cos phi not small output ps and th ... */

	if (*phi > 0.0)
	{
	    s = - rmat[1][0] - rmat[2][1];
				/* (1 + sin phi) sin (psi + theta) */
	    c = rmat[1][1] - rmat[2][0];
				/* (1 + sin phi) cos (psi + theta) */
	    dif = atan2(s, c) - (ps + th);
	    if (dif > PI) dif -= 2.0*PI;
	    if (dif < -PI) dif += 2.0*PI;
	    *psi = ps + 0.5*dif;
	    *theta = th + 0.5*dif;
	}
	else	/* phi <= 0.0 */
	{
	    s = rmat[1][0] - rmat[2][1];
				/* (1 - sin phi) sin (psi - theta) */
	    c = rmat[1][1] + rmat[2][0];
				/* (1 - sin phi) cos (psi - theta) */
	    dif = atan2(s, c) - (ps - th);
	    if (dif > PI) dif -= 2.0*PI;
	    if (dif < -PI) dif += 2.0*PI;
	    *psi = ps + 0.5*dif;
	    *theta = th - 0.5*dif;
	}
	if (*psi > PI) *psi -= 2.0*PI;
	if (*psi < -PI) *psi += 2.0*PI;
	if (*theta > PI) *theta -= 2.0*PI;
	if (*theta < -PI) *theta += 2.0*PI;
    }
}


void
make_imats()
{
    static double   dir_mat[3][3];
    static double   *in[3] = {dir_mat[0], dir_mat[1], dir_mat[2]};
    static double   *out[3] = {base_imat[0], base_imat[1], base_imat[2]};
    double	    matrix_inv_d(), cond;
    int		    i, j;

    for (i = 0; i < 3; i++)
    {
	for (j = 0; j < 2; j++)
	    dir_mat[i][j] = smat[i][j];
	dir_mat[i][2] = zlow*smat[i][2] + smat[i][3];
    }

    cond = matrix_inv_d(in, out, 3);

    if (cond == -1.0)
	for (i = 0; i < 3; i++)
	    for (j = 0; j < 3; j++)
		base_imat[i][j] = 0.0;

    if (debug_level >= 2)
    {
	printmat("dir_mat", &dir_mat[0][0], 3, 3);
	fprintf(stderr, "cond %g\n", cond);
	printmat("base_imat", &base_imat[0][0], 3, 3);
    }
}

/*
 * Translate canvas coords to data coords in base plane of box (z = zlow);
 * return indicates whether point is in base rectangle.
 */
int
base_trans(u, v, x, y)
    double  u, v, *x, *y;
{
    double  xy[3];
    int	    i, j;

    for (i = 0; i < 3; i++)
	xy[i] = base_imat[i][0]*u + base_imat[i][1]*v + base_imat[i][2];

    if (xy[2] == 0)
    {
	return NO;
    }
    else
    {
	*x = xy[0]/xy[2];
	*y = xy[1]/xy[2];
	return xlow <= *x && *x <= xhigh && ylow <= *y && *y <= yhigh;
    }
}


void
make_pmats(xdir, ydir)
    int	    xdir;	/* proj. in pos. x direction (1) or neg. (-1)? */
    int	    ydir;	/* proj. in pos. y direction (1) or neg. (-1)? */
{
    double  f, g, h;
    double  t;
    double  cx, cy;

    /* inverse transform homogenous coordinates (1, hskew, vskew, finv) of
       viewpoint to (f, g, h, finv) in data space */

    g = cos_psi*hskew - sin_psi*vskew;
    h = sin_psi*hskew + cos_psi*vskew;

    f = cos_phi - sin_phi*h;
    h = sin_phi + cos_phi*h;

    t = cos_theta*f - sin_theta*g;
    g = sin_theta*f + cos_theta*g;
    f = t;

    f = -f;
    g = -ori*g;

    f += 0.5*finv*length;
    g += 0.5*finv*width;
    h += 0.5*finv*height;

    f /= xscale;
    g /= yscale;
    h /= zscale;

    f += finv*xlow;
    g += finv*ylow;
    h += finv*zlow;

    /* pick x- and y-coordinates cx and cy  for the planes to project into;
     * the exact values don't matter as long as they're safely beyond the
     * edges of the bounding box
     */

    cx = (xdir == -1)
	 ? xlow + (xlow - xhigh)
	 : xhigh + (xhigh - xlow);
  
    cy = (ydir == -1)
	 ? ylow + (ylow - yhigh)
	 : yhigh + (yhigh - ylow);
  
    /* build the matrices */

    /*
     *           /   g   finv*cx - f       0       -g*cx \
     *   pmatx = |   h        0       finv*cx - f  -h*cx |
     *           \ finv       0            0         -f  /
     */

    pmatx[0][0] = g;
    pmatx[0][1] = finv*cx - f;
    pmatx[0][2] = 0.0;
    pmatx[0][3] = -g*cx;

    pmatx[1][0] = h;
    pmatx[1][1] = 0.0;
    pmatx[1][2] = finv*cx - f;
    pmatx[1][3] = -h*cx;

    pmatx[2][0] = finv;
    pmatx[2][1] = 0.0;
    pmatx[2][2] = 0.0;
    pmatx[2][3] = -f;

    pconstx = cx;

    /*
     *           / finv*cy - g    f        0       -f*cy \
     *   pmaty = |      0         h   finv*cy - g  -h*cy |
     *           \      0       finv       0         -g  /
     */

    pmaty[0][0] = finv*cy - g;
    pmaty[0][1] = f;
    pmaty[0][2] = 0.0;
    pmaty[0][3] = -f*cy;

    pmaty[1][0] = 0.0;
    pmaty[1][1] = h;
    pmaty[1][2] = finv*cy - g;
    pmaty[1][3] = -h*cy;

    pmaty[2][0] = 0.0;
    pmaty[2][1] = finv;
    pmaty[2][2] = 0.0;
    pmaty[2][3] = -g;

    pconsty = cy;
}

point
ptrans(x, y, z)
    double  x, y, z;
{
    point   p;
    double  scl;

    scl = pmatx[2][0]*x + pmatx[2][1]*y + pmatx[2][2]*z + pmatx[2][3];
    p.x = (pmatx[0][0]*x + pmatx[0][1]*y + pmatx[0][2]*z + pmatx[0][3]) / scl;
    p.z = (pmatx[1][0]*x + pmatx[1][1]*y + pmatx[1][2]*z + pmatx[1][3]) / scl;
    return p;
}

/* is direction of coord x from center of projection the positive x
   direction (+1), the negative x direction (-1), or neither (0)? */

int
pdirx(x)
    double  x;
{
    double  d;

    d = pmatx[2][0]*x + pmatx[2][3];
    return (d > 0.0) ? 1 : (d < 0.0) ? -1 : 0;
}

/* is direction of coord x from center of projection the positive x
   direction (+1), the negative x direction (-1), or neither (0)? */

int
pdiry(y)
    double  y;
{
    double  d;

    d = pmaty[2][1]*y + pmaty[2][3];
    return (d > 0.0) ? 1 : (d < 0.0) ? -1 : 0;
}

/* translate plot coords to canvas coords */

void
transl(x, y, z, u, v)
    double  x, y, z;
    double  *u, *v;
{
    double  scale;

    scale = tmat[2][0]*x + tmat[2][1]*y + tmat[2][2]*z + tmat[2][3];
    *u = (tmat[0][0]*x + tmat[0][1]*y + tmat[0][2]*z + tmat[0][3]) / scale;
    *v = (tmat[1][0]*x + tmat[1][1]*y + tmat[1][2]*z + tmat[1][3]) / scale;
}

/* translate data coords to canvas coords */

void
strans(x, y, z, u, v)
    double  x, y, z;
    double  *u, *v;
{
    double  scale;

    scale = smat[2][0]*x + smat[2][1]*y + smat[2][2]*z + smat[2][3];
    *u = (smat[0][0]*x + smat[0][1]*y + smat[0][2]*z + smat[0][3]) / scale;
    *v = (smat[1][0]*x + smat[1][1]*y + smat[1][2]*z + smat[1][3]) / scale;
}

/* plot point given in data coords */

void splotpoint(x, y, z)
    double  x, y, z;
{
    double  u, v;

    strans(x, y, z, &u, &v);
    drawpoint(u, v);
}

/* plot line segment given in plot coordinates */

void
plotline(x1, y1, z1, x2, y2, z2) 
    int	    x1, y1, z1, x2, y2, z2;
{
    double  scale;
    double  u1, v1, u2, v2;

    scale = tmat[2][0]*x1 + tmat[2][1]*y1 + tmat[2][2]*z1 + tmat[2][3];
    u1 = (tmat[0][0]*x1 + tmat[0][1]*y1 + tmat[0][2]*z1 + tmat[0][3]) / scale;
    v1 = (tmat[1][0]*x1 + tmat[1][1]*y1 + tmat[1][2]*z1 + tmat[1][3]) / scale;
    scale = tmat[2][0]*x2 + tmat[2][1]*y2 + tmat[2][2]*z2 + tmat[2][3];
    u2 = (tmat[0][0]*x2 + tmat[0][1]*y2 + tmat[0][2]*z2 + tmat[0][3]) / scale;
    v2 = (tmat[1][0]*x2 + tmat[1][1]*y2 + tmat[1][2]*z2 + tmat[1][3]) / scale;
    drawline(u1, v1, u2, v2);
}


/* plot line segment given in data coordinates */

void
splotline(x1, y1, z1, x2, y2, z2) 
    double  x1, y1, z1, x2, y2, z2;
{
    double  scale;
    double  u1, v1, u2, v2;

    scale = smat[2][0]*x1 + smat[2][1]*y1 + smat[2][2]*z1 + smat[2][3];
    u1 = (smat[0][0]*x1 + smat[0][1]*y1 + smat[0][2]*z1 + smat[0][3]) / scale;
    v1 = (smat[1][0]*x1 + smat[1][1]*y1 + smat[1][2]*z1 + smat[1][3]) / scale;
    scale = smat[2][0]*x2 + smat[2][1]*y2 + smat[2][2]*z2 + smat[2][3];
    u2 = (smat[0][0]*x2 + smat[0][1]*y2 + smat[0][2]*z2 + smat[0][3]) / scale;
    v2 = (smat[1][0]*x2 + smat[1][1]*y2 + smat[1][2]*z2 + smat[1][3]) / scale;
    drawline(u1, v1, u2, v2);
}

/* plot line segment from projection on constant-x plane */

void
pxplotline(y1, z1, y2, z2)
    double  y1, z1, y2, z2;
{
    splotline(pconstx, y1, z1, pconstx, y2, z2);
}


void
box_canv_coords(p_vert)
    double  (*p_vert)[2];  /* 8x2:  1 coord pair for each of 8 vertices */
{
    int	    i;

    /* find canvas coords of projections of box vertices */

    for (i = 0; i < 8; i++)
    {
	double	x, y, z;
	double	scale;

	x = vert_coor[i][0] * length;
	y = vert_coor[i][1] * width;
	z = vert_coor[i][2] * height;

	scale = tmat[2][0]*x + tmat[2][1]*y + tmat[2][2]*z + tmat[2][3];
	p_vert[i][0] =
	    (tmat[0][0]*x + tmat[0][1]*y + tmat[0][2]*z + tmat[0][3]) / scale;
	p_vert[i][1] =
	    (tmat[1][0]*x + tmat[1][1]*y + tmat[1][2]*z + tmat[1][3]) / scale;
    }
}


void
face_visibility(p_vert, face_vis)
    double  (*p_vert)[2];    /* Canvas coords of vertices */
    int	    *face_vis;	    /* 1 value for each of 6 faces.  -1: hidden.
			       0: nearly edge on.  1: visible. */
{
    int	    k;

    /* check visibility of faces */

    for (k = 0; k < 6; k++)
    {
	long	u0, v0, u1, v1;
	double	*p;
	long	cp, dsq0, dsq1;

	/* diagonals of face */

	p = p_vert[face_vert[k][2]];
	u0 = LROUND(p[0]);	v0 = LROUND(p[1]);
	p = p_vert[face_vert[k][0]];
	u0 -= LROUND(p[0]);	v0 -= LROUND(p[1]);
	p = p_vert[face_vert[k][3]];
	u1 = LROUND(p[0]);	v1 = LROUND(p[1]);
	p = p_vert[face_vert[k][1]];
	u1 -= LROUND(p[0]);	v1 -= LROUND(p[1]);

	/* cross product (2 x area of projection of face;
	   positive seen from outside, negative from inside) */

	cp = - ori*(u0*v1 - u1*v0);

	if (cp > 0)
	    face_vis[k] = 1;		/* visible */
	else if (cp < 0)
	    face_vis[k] = -1;		/* hidden */

	/* if 2|area| / (longer diagonal) <= 1, average width of projected
	   face <= 1/2 pixel */

	dsq0 = u0*u0 + v0*v0;
	dsq1 = u1*u1 + v1*v1;
	    /* If not satisfactory, try const*sqrt() and adjust factor */
	if (fabs((double) cp) <= sqrt((double) MAX(dsq0, dsq1)))
	    face_vis[k] = 0;		/* nearly edge on */
    }
}


/* Make matrix to translate from axis-label coords to canvas coords */
/* (also return length of axis) */

void
make_amat(edge, face, len)
    int	    edge, face;
    double  *len;
{
    /*
     *              /         \
     *  amat = tmat | e  n  v |
     *              |         |
     *              \ 0  0  1 /
     *
     *	where column vector e is the unit vector in the direction of edge,
     *	n is the unit normal vector to face,
     *	and v is the initial vertex of edge.
     */

    double  e[3], n[3], v[3];
    int	    v0, v1;	/* vertices of edge */
    int	    i, k;

    *len = box_dims[edge_dir[edge]];

    v0 = edge_vert[edge][0];
    v1 = edge_vert[edge][1];

    for (i = 0; i < 3; i++)
    {
	e[i] = vert_coor[v1][i] - vert_coor[v0][i];
	n[i] = face_nor[face][i];
	v[i] = vert_coor[v0][i] * box_dims[i];
    }

    for (i = 0; i < 3; i++)
    {
	double	s0 = 0.0,
		s1 = 0.0,
		s2 = 0.0;

	for (k = 0; k < 3; k++)
	{
	    double  t = tmat[i][k];

	    s0 += t * e[k];
	    s1 += t * n[k];
	    s2 += t * v[k];
	}
	s2 += tmat[i][3];
	amat[i][0] = s0;
	amat[i][1] = s1;
	amat[i][2] = s2;
    }
}

void
atrans(x, y, u, v)
    double  x, y;
    double  *u, *v;
{
    double  scale;

    scale = amat[2][0]*x + amat[2][1]*y + amat[2][2];
    *u = (amat[0][0]*x + amat[0][1]*y + amat[0][2]) / scale;
    *v = (amat[1][0]*x + amat[1][1]*y + amat[1][2]) / scale;
}

void
aplotline(x1, y1, x2, y2)
    double  x1, y1, x2, y2;
{
    double  u1, v1, u2, v2;

    atrans(x1, y1, &u1, &v1);
    atrans(x2, y2, &u2, &v2);
    drawline(u1, v1, u2, v2);
}

void
aplotlines(n, x, y)
    int		    n;
    double	    *x, *y;
{
    static int	    alloc_size = -1;
    static double   *u, *v;
    int		    i;

    if (alloc_size < n)
    {
	if (alloc_size == -1)
	{
	    u = (double *) malloc((unsigned) n * sizeof(double));
	    v = (double *) malloc((unsigned) n * sizeof(double));
	}
	else
	{
	    u = (double *) realloc((char *) u, (unsigned) n * sizeof(double));
	    v = (double *) realloc((char *) v, (unsigned) n * sizeof(double));
	}
	alloc_size = n;
    }

    for (i = 0; i < n; i++)
	atrans(x[i], y[i], &u[i], &v[i]);

    drawlines(n, u, v);
}

void
draw_box()
{
    int	    i, j;
    int	    face_vis[6];
    double  p_vert[8][2];

    if (debug_level)
	Fprintf(stderr, "draw_box\n");

    /* find canvas coords of projections of box vertices */
    box_canv_coords(p_vert);

    /* check visibility of faces */
    face_visibility(p_vert, face_vis);

    /* draw edges */
	/* Do visible edges first, then hidden edges.  This seems to fix
	   problem with inconsistent rendering of dashed lines (under 2.0
	   beta).  Try explicit resetting of dash_offset in gc if it
	   recurs. */
    for (i = 0; i < 2; i++)
    {
	int	lns;

	lns = (i == 0) ? LNS_SOLID : LNS_DOT;

	drawing_style(BFN_XOR, GET_COL_BOX_FG, lns);

	for (j = 0; j < 12; j++)
	{
	    double  *p0, *p1;
	    int	    vis0, vis1;

	    vis0 = face_vis[edge_face[j][0]];
	    vis1 = face_vis[edge_face[j][1]];

	    if ((i == 0) ? (vis0 == 1 || vis1 == 1)
			 : (vis0 == -1 && vis1 == -1))
	    {
		p0 = p_vert[edge_vert[j][0]];
		p1 = p_vert[edge_vert[j][1]];
	
		drawline(p0[0], p0[1], p1[0], p1[1]);

		if (debug_level >= 2)
		{
		    fprintf(stderr, "%s(%g, %g; %g, %g)",
			(lns == LNS_DOT) ? "D" : "S",
			p0[0], p0[1], p1[0], p1[1]);
		    if (j%4 == 3)
		    fprintf(stderr, "\n");
		}
	    }
	}
    }
}


void
set_x_axis_lims(x0, x1)
    double  x0, x1;
{
    xlow = x0;	xhigh = x1;
    xscale = length / (x1 - x0);
    update_mats();
}


void
set_y_axis_lims(y0, y1)
    double  y0, y1;
{
    ylow = y0;	yhigh = y1;
    yscale = width / (y1 - y0);
    update_mats();
}


void
set_z_axis_lims(z0, z1)
    double  z0, z1;
{
    zlow = z0;	zhigh = z1;
    zscale = height / (z1 - z0);
    update_mats();
}


void
set_axis_lims(x0, x1, y0, y1, z0, z1)
    double  x0, x1, y0, y1, z0, z1;
{
    xlow = x0;	xhigh = x1;
    ylow = y0;	yhigh = y1;
    zlow = z0;	zhigh = z1;
    xscale = length / (x1 - x0);
    yscale = width / (y1 - y0);
    zscale = height / (z1 - z0);
    update_mats();
}


void
get_axis_lims(x0, x1, y0, y1, z0, z1)
    double  *x0, *x1, *y0, *y1, *z0, *z1;
{
    *x0 = xlow;	*x1 = xhigh;
    *y0 = ylow;	*y1 = yhigh;
    *z0 = zlow;	*z1 = zhigh;
}


printmat(text, mat, m, n)
    char    *text;
    double  *mat;
    int	    m, n;
{
    int	    i, j;

    (void) fprintf(stderr, "%s\n", text);
    for (i = 0; i < m; i++)
    {
	for (j = 0; j < n; j++)
	    (void) fprintf(stderr, "\t%g", *(mat++));
	(void) fprintf(stderr, "\n");
    }
}
