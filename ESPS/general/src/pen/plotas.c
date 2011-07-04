/*
 * plotas -- PLOT ASsembler -- deformat vector plot commands -- inverse 'plotdb'
 *
 plotas < stdin > stdout
	Output may be piped into 'pen', ppen', 'gpen', or 'apen'
	Input is formatted plot commands, sample follows:
m 500 1000		Move onto screen
d 2500 1000		Draw a box
d 2500 4000
d 500 4000		This is a legal comment field.
********** this is a comment because the first char is illegal vplot command
w 500 1000 2500 4000	Window xmin ymin xmax ymax
d 3840 6144 		Draw a line across window.
m 1920 0		Go to left middle of GIGI screen
c 6			Set color =6 (yellow)
t 10 1			text size 10, orientation 1
This is the text referred to by the 't' command on the previous line,\
which, using backslash, may be continued onto another line.
e			erase

mazama!jon claerbout
*/
/* Modified  10/20/83  S. Levin  Added exit(0) to flush buffers */

#ifndef lint
    static char *sccs_id = "@(#)plotas.c	3.1	10/20/87	ESI";
#endif

#include	<stdio.h>

#define MAXLINE 24*84


main()
{
    int i,x,y,xmin,xmax,ymin,ymax,size,orient,fat,key,X[16],Y[16],maskx,masky;
    int count;
    char line[MAXLINE];
    char c;

    while(fgets(line,MAXLINE,stdin) != NULL) {
	c= line[0];
	switch(c) {
	 case 'e':
	 case 'b':
	 case 'n':
		putc(c,stdout);
		break;
	 case 'o':
	 case 'm':
	 case 'd':
		putc(c,stdout);
		sscanf(line,"%*c %d %d",&x,&y);
		puth(x,stdout);  puth(y,stdout);
		break;
	 case 'i':
		putc (c,stdout);
		break;
	 case 't':
		putc(c,stdout);
		sscanf(line,"%*c %d %d",&size,&orient);
		key= size&037 | (32*orient);
		puth(key,stdout);
		text();
		break;
	 case 'T':
		putc(c,stdout);
		sscanf(line,"%*c %d %d",&size,&orient);
		puth(size,stdout);
		puth(orient,stdout);
		text();
		break;
	 case 'c':
	 case 'f':
		putc(c,stdout);
		sscanf(line,"%*c %d",&x);
		puth(x,stdout);
		break;
	 case 'w':
		putc(c,stdout);
		sscanf(line,"%*c %d %d %d %d",&xmin,&ymin,&xmax,&ymax);
		puth(xmin,stdout);   puth(ymin,stdout);
		puth(xmax,stdout);   puth(ymax,stdout);
		break;
	 case 'a':
		putc(c,stdout);
		sscanf(line,"%*c %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",&count,&fat,&maskx,&masky,
X,Y,X+1,Y+1,X+2,Y+2,X+3,Y+3,X+4,Y+4,X+5,Y+5,X+6,Y+6,X+7,Y+7,X+8,Y+8,X+9,Y+9,X+10,Y+10,X+11,Y+11,X+12,Y+12,X+13,Y+13,X+14,Y+14);
		puth(count,stdout);
		puth(fat,stdout);
		puth(maskx,stdout);
		puth(masky,stdout);
		for(i=0; i<=count; i++) {puth(X[i],stdout);puth(Y[i],stdout);}
		break;
	 case 'A':
		putc(c,stdout);
		sscanf(line,"%*c %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",&count,
X,Y,X+1,Y+1,X+2,Y+2,X+3,Y+3,X+4,Y+4,X+5,Y+5,X+6,Y+6,X+7,Y+7,X+8,Y+8,X+9,Y+9,X+10,Y+10,X+11,Y+11,X+12,Y+12,X+13,Y+13,X+14,Y+14,X+15,Y+15);
		puth(count,stdout);
		for(i=0; i<count; i++) {puth(X[i],stdout);puth(Y[i],stdout);}
		break;
	 case 'R':
	 case 'r':
		fprintf(stderr,"plotas: no provision for raster");
		exit(1);
	 default:
		; /* Treat unknown characters as comments */
	}
    }
	exit(0);
    /*NOTREACHED*/
}

text()
{
    char c;

    do {
	c=getc(stdin);
	if (c=='\\') {
		if((c=getc(stdin))!='\n') {
			fprintf(stderr,"Plotas: err, \\ not followed by <cr>");
			exit(1);
			}
		}
	else if (c == '\n')
		break;
	putc(c,stdout);
	} while(1);
    putc('\0',stdout);
}
