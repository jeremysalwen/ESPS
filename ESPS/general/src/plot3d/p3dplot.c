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
 * Brief description: Plotting with hidden-line removal.
 *
 */

static char *sccs_id = "@(#)p3dplot.c	1.12	6/24/93	ESI/ERL";

#include <stdio.h>
#include <math.h>
#include <esps/esps.h>
#include "plot3d.h"


extern void	splotline();
extern void	splotpoint();
extern void	plotline();
extern void	drawline();
extern void	aplotline();
extern void	pxplotline();
extern void	make_amat();
extern void	make_pmats();
extern int	base_trans();
extern point	ptrans();
extern void	atrans();
extern void	drawing_style();
extern int	pdirx(), pdiry();
extern void	string_extents();

extern int	debug_level;
extern int      force_monochrome_plot;

static point	interp();
static point	intersect();
void		pr_points();

static void	plot_scale();
static void	end_label();
static int	residue();



void
extrema(a, m, b, n, cp, pp)
    point   *a, *b, **cp;
    int	    m, n, *pp;
{
    point   *c;
    int	    i, j, k;
    point   a0, b0, a1, b1;

    c = (point *) malloc((unsigned) 2 * (m + n) * sizeof (point));

    k = 0;

    if (m == 0)
    {
	for (j = 0; j < n; j++)
	    c[k++] = b[j];
    }
    else
    if (n == 0)
    {
	for (i = 0; i < m; i++)
	    c[k++] = a[i];
    }
    else
    {
	for (i = 0; i < m && a[i].x < b[0].x; i++)
	{
	    c[k++] = a[i];
	}

	for (j = 0; j < n && b[j].x < a[0].x; j++)
	{
	    c[k++] = b[j];
	}

	if (i == 0 && j == 0)	/* a[0].x == b[0].x */
	{
	    a0 = a[0];
	    b0 = b[0];
	    c[k++] = (a0.z > b0.z) ? a0 : b0;
	    i++;
	    j++;
	}
	else
	if (i == 0 && j < n)	/* b[j-1].x < a[0].x <= b[j].x */
	{
	    a0 = a[0];
	    if (a0.x < b[j].x)
	    {
		b0 = interp(b[j-1], b[j], a0.x);
		if (a0.z >= b0.z)
		{
		    c[k++] = a0;
		}
	    }
	    else		/* a[0].x == b[j].x */
	    {
		b0 = b[j];
		c[k++] = (a0.z > b0.z) ? a0 : b0;
		j++;
	    }
	    i++;
	}
	else	/* (j == 0) */
	if (i < m)		/* a[i-1].x < b[0].x <= a[i].x */
	{
	    b0 = b[0];
	    if (b0.x < a[i].x)
	    {
		a0 = interp(a[i-1], a[i], b0.x);
		if (b0.z >= a0.z)
		{
		    c[k++] = b0;
		}
	    }
	    else		/* b[0].x == a[i].x */
	    {
		a0 = a[i];
		c[k++] = (b0.z > a0.z) ? b0 : a0;
		i++;
	    }
	    j++;
	}

	while (i < m && j < n)
	{
	    if (a[i].x < b[j].x)
	    {
		a1 = a[i];
		b1 = interp(b0, b[j], a1.x);
		if (a1.z < b1.z)
		{
		    if (a0.z > b0.z)
		    {
			c[k++] = intersect(a0, a1, b0, b1);
		    }
		}
		else
		if (a1.z > b1.z)
		{
		    if (a0.z < b0.z)
		    {
			c[k++] = intersect(a0, a1, b0, b1);
		    }
		    c[k++] = a1;
		}
		else	/* a1.z == b1.z */
		{
		    c[k++] = a1;
		}
		i++;
	    }
	    else
	    if (a[i].x > b[j].x)
	    {
		b1 = b[j];
		a1 = interp(a0, a[i], b1.x);
		if (b1.z < a1.z)
		{
		    if (b0.z > a0.z)
		    {
			c[k++] = intersect(b0, b1, a0, a1);
		    }
		}
		else
		if (b1.z > a1.z)
		{
		    if (b0.z < a0.z)
		    {
			c[k++] = intersect(b0, b1, a0, a1);
		    }
		    c[k++] = b1;
		}
		else	/* b1.z == a1.z */
		{
		    c[k++] = b1;
		}
		j++;
	    }
	    else	/* a[i].x == b[j].x */
	    {
		a1 = a[i];
		b1 = b[j];
		if (a1.z < b1.z)
		{
		    if (a0.z > b0.z)
		    {
			c[k++] = intersect(a0, a1, b0, b1);
		    }
		    c[k++] = b1;
		}
		else
		if (b1.z < a1.z)
		{
		    if (b0.z > a0.z)
		    {
			c[k++] = intersect(b0, b1, a0, a1);
		    }
		    c[k++] = a1;
		}
		else	/* a1.z == b1.z */
		{
		    c[k++] = a1;
		}
		i++;
		j++;
	    }
	    a0 = a1;
	    b0 = b1;
	}

	while (i < m)
	{
	    c[k++] = a[i];
	    i++;
	}

	while (j < n)
	{
	    c[k++] = b[j];
	    j++;
	}
    }
	
    *cp = c;
    *pp = k;
}

static point
interp(a, b, x)
    point   a, b;
    double  x;
{
    point   c;
    double  r;

    r = (x - a.x)/(b.x - a.x);
    c.x = x;
    c.z = (1.0 - r)*a.z + r*b.z;
    return c;
}

static point
intersect(a0, a1, b0, b1)
    point   a0, a1, b0, b1;
{
    point   c;
    double  adx, adz, bdx, bdz;
    double  r;

    adx = a1.x - a0.x;	adz = a1.z - a0.z;
    bdx = b1.x - b0.x;	bdz = b1.z - b0.z;
    r = ((b0.x - a0.x)*bdz - bdx*(b0.z - a0.z)) / (adx*bdz - bdx*adz);
    c.x = a0.x + r*(a1.x - a0.x);
    c.z = a0.z + r*(a1.z - a0.z);
    return c;
}


void
plot(data, x, y, nrecs, nitems)
    float   **data;
    double  *x, *y;
    int	    nrecs, nitems;
{
    int	    i, i0, i1, j, k;
    point   *line, *hiline, *new_hiline;
    int	    h0, h1, hilen;
    int	    dirymin, dirymax;

    drawing_style(BFN_SRC, GET_COL_TOP_FG, LNS_SOLID);

    make_pmats(1, 1);

    dirymin = pdiry(y[0]);
    dirymax = pdiry(y[nitems-1]);

    if (debug_level)
	(void) fprintf(stderr,
		       "plot: dirymin = %d, dirymax = %d\n", dirymin, dirymax);

    line = (point *) malloc((unsigned) (nitems + 2) * sizeof(point));
    for (i = 0; i < nrecs && pdirx(x[i]) < 0; i++)
    { }
    i0 = i-1;

    if (debug_level)
	(void) fprintf(stderr, "plot: i0 = %d\n", i0);

    for ( ; i < nrecs && pdirx(x[i]) == 0; i++)
    {
	for (j = 0; j+1 < nitems; j++)
	    splotline(x[i], y[j], data[i][j], x[i], y[j+1], data[i][j+1]);
    }
    i1 = i;

    if (debug_level)
	(void) fprintf(stderr, "plot: i1 = %d\n", i1);

/*!*//* May need edge segment connecting to previous slice. */
    hiline = (point *) malloc((unsigned) 1 * sizeof(point));
    hilen = 0;

    for ( ; i < nrecs; i++)
    {
	k = 0;

	h0 = 0;
	switch (dirymin)
	{
	case -1:
	    if (i > i1)
	    {
		line[k++] = ptrans(x[i-1], y[0], data[i-1][0]);
		for (h0 = 1; h0 < hilen && hiline[h0].x <= line[0].x; h0++)
		{ }
		h0--;
	    }
	    break;
	case 0:
	    if (i > i1)
	    {
		splotline(x[i-1], y[0], data[i-1][0], x[i], y[0], data[i][0]);
		for (h0 = 1; h0 < hilen && hiline[h0].x <= line[0].x; h0++)
		{ }
		h0--;
	    }
	    break;
	case 1:
	    if (i+1 < nrecs)
		line[k++] = ptrans(x[i+1], y[0], data[i+1][0]);
	    break;
	}

	for (j = 0; j < nitems; j++)
	    line[k++] =
		ptrans(x[i], y[j], data[i][j]);

	h1 = hilen;
	switch (dirymax)
	{
	case -1:
	    if (i+1 < nrecs)
		line[k++] = ptrans(x[i+1], y[nitems-1], data[i+1][nitems-1]);
	    break;
	case 0:
	    if (i > i1)
	    {
		splotline(x[i-1], y[nitems-1], data[i-1][nitems-1],
			  x[i], y[nitems-1], data[i][nitems-1]);
		for (h1 = hilen-2; h1 >= 0 && hiline[h1].x >= line[k-1].x; h1--)
		{ }
		h1 += 2;
	    }
	    break;
	case 1:
	    if (i > i1)
	    {
		line[k++] = ptrans(x[i-1], y[nitems-1], data[i-1][nitems-1]);
		for (h1 = hilen-2; h1 >= 0 && hiline[h1].x >= line[k-1].x; h1--)
		{ }
		h1 += 2;
	    }
	    break;
	}

        if (debug_level >= 3)
	{
	    fprintf(stderr, "plot: i = %d\n", i);
	    pr_points("line", k, line);
	    pr_points("hiline", h1 - h0, hiline + h0);
	}

	extrema(line, k, hiline + h0, h1 - h0, &new_hiline, &hilen);
	free((char *) hiline);
	hiline = new_hiline;

	if (debug_level >= 3)
	{
	    pr_points("new hiline", hilen, hiline);
	    for (j = 0; j+1< hilen && hiline[j+1].x <= line[k-1].x; j++)
	    { }
	    (void) fprintf(stderr, "plotting %d segments\n", j);
	}

	for (j = 0; j+1< hilen && hiline[j+1].x <= line[k-1].x; j++)
	    pxplotline(hiline[j].x, hiline[j].z, hiline[j+1].x, hiline[j+1].z);
    }

    make_pmats(-1, 1);

/*!*//* May need edge segment connecting to previous slice. */
    hiline = (point *) malloc((unsigned) 1 * sizeof(point));
    hilen = 0;

    for (i = i0; i >= 0; i--)
    {
	k = 0;

	h0 = 0;
	switch (dirymin)
	{
	case -1:
	    if (i < i0)
	    {
		line[k++] = ptrans(x[i+1], y[0], data[i+1][0]);
		for (h0 = 1; h0 < hilen && hiline[h0].x <= line[0].x; h0++)
		{ }
		h0--;
	    }
	    break;
	case 0:
	    if (i < i0)
	    {
		splotline(x[i+1], y[0], data[i+1][0], x[i], y[0], data[i][0]);
		for (h0 = 1; h0 < hilen && hiline[h0].x <= line[0].x; h0++)
		{ }
		h0--;
	    }
	    break;
	case 1:
	    if(i-1 >= 0)
		line[k++] = ptrans(x[i-1], y[0], data[i-1][0]);
	    break;
	}

	for (j = 0; j < nitems; j++)
	    line[k++] =
		ptrans(x[i], y[j], data[i][j]);

	h1 = hilen;
	switch (dirymax)
	{
	case -1:
	    if (i-1 >= 0)
		line[k++] = ptrans(x[i-1], y[nitems-1], data[i-1][nitems-1]);
	    break;
	case 0:
	    if (i < i0)
	    {
		splotline(x[i+1], y[nitems-1], data[i+1][nitems-1],
			  x[i], y[nitems-1], data[i][nitems-1]);
		for (h1 = hilen-2; h1 >= 0 && hiline[h1].x >= line[k-1].x; h1--)
		{ }
		h1 += 2;
	    }
	    break;
	case 1:
	    if (i < i0)
	    {
		line[k++] = ptrans(x[i+1], y[nitems-1], data[i+1][nitems-1]);
		for (h1 = hilen-2; h1 >= 0 && hiline[h1].x >= line[k-1].x; h1--)
		{ }
		h1 += 2;
	    }
	    break;
	}

        if (debug_level >= 3)
	{
	    fprintf(stderr, "plot: i = %d\n", i);
	    pr_points("line", k, line);
	    pr_points("hiline", h1 - h0, hiline + h0);
	}

	extrema(line, k, hiline + h0, h1 - h0, &new_hiline, &hilen);
	free((char *) hiline);
	hiline = new_hiline;

	if (debug_level >= 3)
	{
	    pr_points("new hiline", hilen, hiline);
	    for (j = 0; j+1< hilen; j++)
	    { }
	    (void) fprintf(stderr, "plotting %d segments\n", j);
	}

	for (j = 0; j+1< hilen; j++)
	    pxplotline(hiline[j].x, hiline[j].z, hiline[j+1].x, hiline[j+1].z);
    }

    free((char *) hiline);
    free((char *) line);
}

extern void draw_string();

void
axes()
{
    extern void	box_canv_coords(), face_visibility();
    extern void text();
    extern int	edge_face[12][2], edge_vert[12][2];
    extern char	*Harg, *targ, *Varg;	/* x-, y-, and z-axis text */

    /* edges to try for x-, y-, and z-axes (see p3dcoords.c for numbering) */
    static int	x_cand[4] = { 0,  2,  1,  3},
    	    	y_cand[4] = { 4,  6,  5,  7},
    	    	z_cand[4] = { 8, 10,  9, 11};
    static int	*cand_lists[3] = {x_cand, y_cand, z_cand};
    static int	vertical[6] = {1, 1, 1, 1, 0, 0};
					/* 1: face is front, back or side.
					   0: face is top or bottom. */
    int	    ax;
    int	    i, j, k;
    int	    face_vis[6];
    double  p_vert[8][2];
    int	    drawn[8][6];

    if (debug_level)
	Fprintf(stderr, "axes\n");

    /* Find canvas coords of projections of box vertices. */
    box_canv_coords(p_vert);

    /* Check visibility of faces. */
    face_visibility(p_vert, face_vis);

    drawing_style(BFN_XOR, GET_COL_AXIS_FG, LNS_SOLID);

    /* Short perpendicular segments will be drawn at the ends of the axes.
	If one of these is common to two axes, it might be drawn twice
	and so canceled out by the xor drawing mode.
	Set up to remember the vertex and normal face of each one drawn
	to keep this from happening.  */

    for (i = 0; i < 8; i++)
	for (k = 0; k < 6; k++)
	    drawn[i][k] = 0;
	
    /* Find suitable edges along which to draw axis labels, etc.; draw them */

    for (ax = 0; ax < 3; ax++)	/* for 'x', 'y', 'z' */
    {
	int	*cands = cand_lists[ax];
	int	edge = -1;	/* -1: not found yet */
	int	periph[4];	/* Is candidate edge on periphery of
				   projection of box? */

	/* Prefer a peripheral edge in a visible "vertical" (not top or
	   bottom) face ... */

	for (j = 0; j < 4; j++)
	{
	    int	    face0, face1;
	    int	    vis0, vis1;

	    face0 = edge_face[cands[j]][0];
	    face1 = edge_face[cands[j]][1];
	    vis0 = face_vis[face0];
	    vis1 = face_vis[face1];

	    periph[j] = (vis0 == 1 && vis1 != 1) || (vis0 != 1 && vis1 == 1);

	    if (edge == -1 && periph[j]
		&& ((vis0 == 1 && vertical[face0])
		    || (vis1 == 1 && vertical[face1])))
		edge = cands[j];
	}

	/* ... but will settle for just a peripheral edge. */

	for (j = 0; edge == -1 && j < 4 ; j++)
	    if (periph[j])
		edge = cands[j];

	if (edge != -1)		/* If found, draw. */
	{
	    int	    i;
	    double  cp[2];	/* Areas of projections of rectangles
				   in 2 poss. planes containing axis labels */
	    int	    sgn;	/* Handedness of projected axis label coord
				   system */
	    int	    vrt;	/* Which way is up in axis label rect? */
	    int	    vrts[2];	/* Tenatative valuse for vrt */
	    double  ax_len;	/* Length of axis (plot units) */
	    double  ax_wid = 42.0;  /* width of plot-label rectangle */
	    int	    nf;		/* Face perpendicular to plane in which
				   axis labels are drawn */
	    double  low, high;	/* Data values at ends of axes */
	    int	    n;		/* Number of tick marks */
	    double  start;	/* First data value to get tick mark */
	    double  step;	/* Data interval between tick marks */
	    int	    mod1, res1, mod2, res2;
				/* Values used in deciding whether a tick
				   mark is primary (longest) or secondary
				   (medium length) */
	    double  len2, len3; /* Lengths of secondary and tertiary (shortest)
				   ticks relative to primary */
	    double  u, v;	/* Canv coords of projections of vertices of
				   rectangle */
	    double  du0, dv0, du1, dv1;
				/* Difference vectors of projected vertices
				   of rectangle */

	    /* Draw in plane of one of the faces adjacent to the edge
	       (and so perpendicular to the other face).
	       Which one?
	       Make the choice that gives the greater value for the area
	       of the projection onto the canvas of a rectangle containing
	       the axis labels, etc.
	       Assume this implies the less oblique view. */

	    for (i = 0; i < 2; i++)
	    {
		make_amat(edge, edge_face[edge][i], &ax_len);
		atrans(ax_len, ax_wid, &du0, &dv0);
		atrans(0.0, 0.0, &u, &v);
		du0 -= u;    dv0 -= v;		/* Diagonal of axis rect. */
		atrans(0.0, ax_wid, &du1, &dv1);
		vrts[i] = (v > dv1) ? 1 : -1;
		atrans(ax_len, 0.0, &u, &v);
		du1 -= u;    dv1 -= v;		/* Other diagonal. */
		/* Cross prod of diagonals = 2 time area of quatrilateral */
		cp[i] = du0*dv1 - du1*dv0;
	    }

	    if (fabs(cp[0]) > fabs(cp[1]))  /* If first was better, go back. */
	    {
		nf = edge_face[edge][0];
		make_amat(edge, nf, &ax_len);
		sgn = (cp[0] > 0) ? -1 : 1;
		vrt = vrts[0];
	    }
	    else
	    {
		nf = edge_face[edge][1];
		sgn = (cp[1] > 0) ? -1 : 1;
		vrt = vrts[1];
	    }

	    /* Strokes at ends of axis.  Avoid doing twice. */
	    if (!drawn[edge_vert[edge][0]][nf]++)
		aplotline(0.0, 0.0, 0.0, ax_wid);
	    if (!drawn[edge_vert[edge][1]][nf]++)
		aplotline(ax_len, 0.0, ax_len, ax_wid);

	    aplotline(0.0, 0.0, ax_len, 0.0);

	    /* Data values at ends of axes. */

	    switch (ax)
	    {
		extern double	xlow, xhigh, ylow, yhigh, zlow, zhigh;

	    case 0:
		low = xlow;	high = xhigh;
		break;
	    case 1:
		low = ylow;	high = yhigh;
		break;
	    case 2:
		low = zlow;	high = zhigh;
		break;
	    }

	    /* Labels at extremes of axes. */

	    end_label(0.0, ax_len, ax_wid, low);
	    end_label(ax_len, 0.0, ax_wid, high);

	    /* Text along axes. */

	    switch (ax)
	    {
	    case 0:
		text(6.0, 22.0, sgn*vrt*12, vrt*20, Harg);
		break;
	    case 1:
		text(6.0, 22.0, sgn*vrt*12, vrt*20, targ);
		break;
	    case 2:
		text(6.0, 22.0, sgn*vrt*12, vrt*20, Varg);
		break;
	    }

	    /* aplotline(0.0, ax_wid, ax_len, ax_wid); */

	    /* Draw tick marks. */

	    plot_scale(low, high, 10.0*(high - low)/ax_len,
		       &start, &step, &n,
		       &mod1, &res1, &mod2, &res2, &len2, &len3);

	    for (i = 0; i < n; i++)
	    {
		double  val;	/* data value at tick mark */
		double  loc;	/* plot coord of tick mark */
		double  len;	/* length of tick mark */

		val = start + i*step;
		if (fabs(val) < 0.5*step) val = 0.0;
		loc = (val - low) / (high - low) * ax_len;
		len = 12.0*((i%mod1 == res1) ? 1.0
			    : (i%mod2 == res2) ? len2
			    : len3);
		aplotline(loc, 0.0, loc, len);
	    }

	
	}

	/* Fill in just a line for other peripheral edges. */

	for (j = 0; j < 4 ; j++)
	{
	    int	    e = cands[j];

	    if (e != edge && periph[j])
	    {
		double	*p0, *p1;	/* Endpoints of edge (canvas coords). */

		p0 = p_vert[edge_vert[e][0]];
		p1 = p_vert[edge_vert[e][1]];
		drawline(p0[0], p0[1], p1[0], p1[1]);
	    }
	}

/*!*//* Should do portions of back lines (dotted by draw_box) not hidden
	by data plot. */
    }
}


static void
end_label(x0, x1, y, val)
    double  x0, x1, y, val;
{
    double  u, v;	/* canv coords of projections of vertices of
			   rectangle */
    double  du0, dv0, du1, dv1;
			/* difference vectors of projected vertices
			   of rectangle */
    int	    ccw;	/* Are certain angles positive (counter-
			   clockwise)? */
    int	    corner;	/* Reference corner of bounding rect. of
			   axis label. */
#define	    NE_CORNER	1
#define	    NW_CORNER	2
#define	    SW_CORNER	3
#define	    SE_CORNER	4

    char    label[50];	/* Text of numerical axis label. */
    int	    label_width, label_ascent, label_descent, default_width;

    atrans(x0, 0.0, &u, &v);
    atrans(x1, 0.0, &du0, &dv0);
    du0 -= u;	dv0 -= v;	/* Vector along axis (projected
				   into canvas). */
    atrans(x0, y, &du1, &dv1);
    du1 -= u;	dv1 -= v;	/* Vector along end stroke. */
    ccw = (du0*dv1 - dv0*du1 < 0.0);
				/* Is angle from proj of axis to proj.
				   of end stroke counterclockwise? */

	/* Decide on reference corner of bd rect of label. */
    if (du1 > dv1)
    {
	corner =
	    (du1 > -dv1)	/* End stroke points roughly east. */
	    ? (ccw ? NW_CORNER : SW_CORNER)
	    : (ccw ? SW_CORNER : SE_CORNER);	    /* ... north. */
    }
    else /* du1 <= dv1 */
    {
	corner =
	    (du1 > -dv1)			    /* ... south. */
	    ? (ccw ? NE_CORNER : NW_CORNER)
	    : (ccw ? SE_CORNER : NE_CORNER);	    /* ... west. */
    }

    sprintf(label, "%g", val);
    atrans(x0, y, &u, &v);    /*!*//* Computed once already. */

    string_extents(label, &label_ascent,
		   &label_descent, &label_width, &default_width);

    switch (corner)
    {
    case NE_CORNER:
	u -= label_width + 0.5*default_width;
	v += label_ascent;
	break;
    case NW_CORNER:
	u += 0.5*default_width;
	v += label_ascent;
	break;
    case SW_CORNER:
	u += 0.5*default_width;
	v -= label_descent;
	break;
    case SE_CORNER:
	u -= label_width + 0.5*default_width;
	v -= label_descent;
	break;
    }

    draw_string(u, v, label);
    return;

#undef	    NE_CORNER
#undef	    NW_CORNER
#undef	    SW_CORNER
#undef	    SE_CORNER
}


static int
residue(x, y)
    double  x, y;
{
    double r;

#ifndef APOLLO_68K
    r = fmod(x, y);
    return (int) (r + ((r < 0) ? 10.5 : 0.5));
				/* 10.0 and 0.0 should work
				   since fmod is integer- valued.
				   Round for paranoia. */
#else
     r = x - y*(int)(x/y);
     if (r < 0) r += (y < 0) ? -y : y;
     return (int) (r + 0.5);
#endif

}


static void
plot_scale(low, high, minstep, start, step, num,
	   mod1, res1, mod2, res2, len2, len3)
    double  low, high, minstep;
    double  *start, *step;
    int	    *num;
    int	    *mod1, *res1, *mod2, *res2;
    double  *len2, *len3;
{
    double  t, d, s;
    int	    n;
    int	    c;
    double  q;
    int	    r;

    if (debug_level)
	fprintf(stderr, "plot_scale: (%g, %g, %g) -> ", low, high, minstep);

#ifndef IBM_RS6000
    t = pow(10.0, floor(log10(minstep)));
#else
    t = exp(log(10.0) * floor(log10(minstep)));
#endif
    if (t >= minstep)
    {
	d = t;
	c = 1;
    }
    else if (2.0*t >= minstep)
    {
	d = 2.0*t;
	c = 2;
    }
    else if (5.0*t >= minstep)
    {
	d = 5.0*t;
	c = 5;
    }
    else if (10.0*t >= minstep)
    {
	d = 10.0*t;
	c = 1;
    }
    else
    {
	d = 20.0*t;
	c = 2;
    }

    q = ceil((low + 0.5*minstep) / d);
    s = d*q;
    n = (high - 0.5*minstep - s + d) / d;
    if (n < 0) n = 0;

    *start = s;
    *step = d;
    *num = n;

    *mod1 = 10;
    r = residue(-q, 10.0);
    *res1 = r;

    switch (c)
    {
    case 1:
	*mod2 = 5;
	*res2 = r%5;
	*len2 = 2.0/3.0;
	*len3 = 1.0/3.0;
	break;
    case 2:
	*mod2 = 5;
	*res2 = r%5;
	*len2 = 1.0;
	*len3 = 0.5;
	break;
    case 5:
	*mod2 = 2;
	*res2 = r%2;
	*len2 = 2.0/3.0;
	*len3 = 1.0/3.0;
	break;
    }

    if (debug_level)
	fprintf(stderr, "(%g, %g, %d, %d, %d, %d, %d, %g, %g)\n",
		*start, *step, *num, *mod1, *res1, *mod2, *res2, *len2, *len3);
}


void
pointplot(data, x, y, nrecs, nitems)
    float   **data;
    double  *x, *y;
    int	    nrecs, nitems;
{
    int	    i, j;

    if (debug_level)
	(void) fprintf(stderr, "pointplot: %d by %d.\n", nrecs, nitems);

    drawing_style(BFN_SRC, GET_COL_TOP_FG, LNS_SOLID);

    for (i = 0; i < nrecs; i++)
 	for (j = 0; j < nitems; j++)
	    splotpoint(x[i], y[j], data[i][j]);
}


void
testplot(data, nrecs, nitems)
    float   **data;
    int	    nrecs, nitems;
{
    int	    i, j;
    extern double   xlow, ylow;

    if (debug_level)
	(void) fprintf(stderr, "testplot: %d by %d.\n", nrecs, nitems);

    drawing_style(BFN_XOR, GET_COL_TOP_FG, LNS_SOLID);

    for (i = 0; i < nrecs; i++)
 	for (j = 0; j < nitems - 1; j++)
	    splotline((double) (i+xlow), (double) (j+ylow), data[i][j],
		      (double) (i+xlow), (double) (j+ylow+1), data[i][j+1]);
}


/* for debugging output */

void
pr_points(text, n, pts)
    char    *text;
    int	    n;
    point   *pts;
{
    int	    i;

    (void) fprintf(stderr, "%s - %d points:\n", text, n);
    for (i = 0; i < n; i++)
    {
        (void) fprintf(stderr, "{%g %g} ", pts[i].x, pts[i].z);
        if (i%4 == 3 || i == n - 1) (void) fprintf(stderr, "\n");
    }
}
