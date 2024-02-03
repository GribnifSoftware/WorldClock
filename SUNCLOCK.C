/*
    Sun clock

    Designed and implemented by John Walker in November of 1988.

    Version for the Sun Workstation.

    The algorithm used to calculate the position of the Sun is given in
    Chapter 18 of:

    "Astronomical  Formulae for Calculators" by Jean Meeus, Third Edition,
    Richmond: Willmann-Bell, 1985.  This book can be obtained from:

       Willmann-Bell
       P.O. Box 35025
       Richmond, VA  23235
       USA
       Phone: (804) 320-7016

    This program was written by:

       John Walker
       Autodesk, Inc.
       2320 Marinship Way
       Sausalito, CA  94965
       USA
       Fax:   (415) 389-9418
       Voice: (415) 332-2344 Ext. 2829
       Usenet: {sun,well,uunet}!acad!kelvin
       or: kelvin@acad.uu.net

    This  program is in the public domain: "Do what thou wilt shall be the
    whole of the law".  I'd appreciate  receiving  any  bug  fixes  and/or
    enhancements,  which  I'll  incorporate  in  future  versions  of  the
    program.  Please leave the original attribution information intact    so
    that credit and blame may be properly apportioned.

    Revision history:

    1.0  12/21/89  Initial version.
          8/24/89  Finally got around to submitting.

*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <tos.h>
#include <new_aes.h>
#include <vdi.h>
#include "wc.h"

#define TRUE 1
#define FALSE 0

/*----- Prototypes -----*/
void pw_setmode( int );
void cdecl pw_vector( int, int, int, int );
void pw_b_on( void* );
void pw_b_off( void* );
static void   cevent( void );
static long   jdate( struct tm* );
static double jtime( struct tm* );
static double kepler( double, double );
static void   sunpos( double, int, double*, double*, double*, double* );
static double gmst( double );
static void projillum( short*, int, int, double );
static void xspan( int, int, int );

OBJECT  *dialog_addr;

#define abs(x) ((x) < 0 ? (-(x)) : x)              /* Absolute value */
#define sgn(x) (((x) < 0) ? -1 : ((x) > 0 ? 1 : 0))      /* Extract sign */
#define dtr(x) ((x) * (M_PI / 180.0))              /* Degree->Radian */
#define rtd(x) ((x) / (M_PI / 180.0))              /* Radian->Degree */
#define fixangle(a) ((a) - 360.0 * (floor((a) / 360.0)))  /* Fix angle      */

#define V      (void)

#define TERMINC  100           /* Circle segments for terminator */

#define PROJINT  (60 * 10)       /* Frequency of seasonal recalculation
                      in seconds. */

/*  Globals imported  */

extern time_t time();

/*  Local variables  */

static int onoon = -1;
static short *wtab, *wtab1, *wtabs;

static long lincr = 3600;
static long cctime;
static int lisec = 61;       /* Last iconic seconds */
static long lctime = 0;    /* Last full calculation time */
static char wait_draw;

void cdecl pw_vector( x1, y1, x2, y2 )
int     x1, x2, y1, y2;
{
    x1 += center.g_x;
    y1 += center.g_y;
    x2 += center.g_x;
    y2 += center.g_y;
    v_pline(vdi_hand,2,&x1);
}

void force_sun(void)
{
    /* If this is a full repaint of the window, force complete
       recalculation. */

    int i;

    lctime = 0;
    onoon = -1;
    lisec = 61;
    for (i = 0; i < center.g_h; i++)
       wtab1[i] = -1;
    wait_draw = 0;
}

short *mt_wtab, *mt_otab;
int mt_noon, mt_onoon;

/*  Update current displayed image.  */
int updt_sun( int always )
{
    int i, xl;
    struct tm *ct;
    double jt, sunra, sundec, sunrv, sunlong, gt;

    V time(&cctime);

    ct = gmtime(&cctime);

    jt = jtime(ct);
    sunpos(jt, FALSE, &sunra, &sundec, &sunrv, &sunlong);
    gt = gmst(jt);

    /* Projecting the illumination curve  for the current seasonal
           instant is costly.  If we're running in real time, only  do
       it every PROJINT seconds.  */

    if ( always || ((cctime - lctime) > PROJINT)) {
       if( wait_draw )
       {
         force_sun();
         always = 1;
       }
       projillum(wtab, center.g_w, center.g_h, sundec);
       wtabs = wtab;
       wtab = wtab1;
       wtab1 = wtabs;
       lctime = cctime;
    }

    sunlong = fixangle(180.0 + (sunra - (gt * 15)));
    xl = sunlong * (center.g_w / 360.0);

    /* If the subsolar point has moved at least one pixel, update
       the illuminated area on the screen.    */

    if ( always || (onoon != xl)) {
       mt_wtab = wtab1;
       mt_noon = xl;
       mt_otab = wtab;
       mt_onoon = onoon;
       onoon = xl;
       wait_draw = 1;
       return 1;
    }
    return 0;
}

int sunclock( int always )
{
    if( always ) force_sun();
    return updt_sun( always );
}

int init_sun( int ymax )
{
    wtab = (short *) malloc((long) ymax * sizeof(short));
    wtab1 = (short *) malloc((long) ymax * sizeof(short));
    if( !wtab || !wtab1 ) return 0;
    return 1;
}

/*  JDATE  --  Convert internal GMT date and time to Julian day
           and fraction.  */

static long jdate(t)
struct tm *t;
{
    long c, m, y;

    y = t->tm_year + 1900;
    m = t->tm_mon + 1;
    if (m > 2)
       m = m - 3;
    else {
       m = m + 9;
       y--;
    }
    c = y / 100L;           /* Compute century */
    y -= 100L * c;
    return t->tm_mday + (c * 146097L) / 4 + (y * 1461L) / 4 +
        (m * 153L + 2) / 5 + 1721119L;
}

/* JTIME --    Convert internal GMT  date  and    time  to  astronomical
           Julian  time  (i.e.   Julian  date  plus  day fraction,
           expressed as a double).    */

static double jtime(t)
struct tm *t;
{
    return (jdate(t) - 0.5) + 
       (((long) t->tm_sec) +
         60L * (t->tm_min + 60L * t->tm_hour)) / 86400.0;
}

/*  KEPLER  --    Solve the equation of Kepler.  */
static double kepler(m, ecc)
double m, ecc;
{
    double e, delta, cs;

#define EPSILON 1E-6

    e = m = dtr(m);
    do {
       delta = e - ecc * sin(e) - m;
       cs = 1 - (ecc * cos(e));    /* Use 'cs' to avoid bug in TC v1.0 */
       e -= delta / cs;
    } while (abs(delta) > EPSILON);
    return e;
}

/*  SUNPOS  --    Calculate position of the Sun.    JD is the Julian  date
        of  the  instant for which the position is desired and
        APPARENT should be nonzero if  the  apparent  position
        (corrected  for  nutation  and aberration) is desired.
                The Sun's co-ordinates are returned  in  RA  and  DEC,
        both  specified  in degrees (divide RA by 15 to obtain
        hours).  The radius vector to the Sun in  astronomical
                units  is returned in RV and the Sun's longitude (true
        or apparent, as desired) is  returned  as  degrees  in
        SLONG.    */
static void sunpos(jd, apparent, ra, dec, rv, slong)
double jd;
int apparent;
double *ra, *dec, *rv, *slong;
{
    double t, t2, t3, l, m, e, ea, v, theta, omega,
           eps;

    /* Time, in Julian centuries of 36525 ephemeris days,
       measured from the epoch 1900 January 0.5 ET. */

    t = (jd - 2415020.0) / 36525.0;
    t2 = t * t;
    t3 = t2 * t;

    /* Geometric mean longitude of the Sun, referred to the
       mean equinox of the date. */

    l = fixangle(279.69668 + 36000.76892 * t + 0.0003025 * t2);

        /* Sun's mean anomaly. */

    m = fixangle(358.47583 + 35999.04975*t - 0.000150*t2 - 0.0000033*t3);

        /* Eccentricity of the Earth's orbit. */

    e = 0.01675104 - 0.0000418 * t - 0.000000126 * t2;

    /* Eccentric anomaly. */

    ea = kepler(m, e);

    /* True anomaly */

    v = fixangle(2 * rtd(atan(sqrt((1 + e) / (1 - e))  * tan(ea / 2))));

        /* Sun's true longitude. */

    theta = l + v - m;

    /* Obliquity of the ecliptic. */

    eps = 23.452294 - 0.0130125 * t - 0.00000164 * t2 + 0.000000503 * t3;

        /* Corrections for Sun's apparent longitude, if desired. */

    if (apparent) {
       omega = fixangle(259.18 - 1934.142 * t);
       theta = theta - 0.00569 - 0.00479 * sin(dtr(omega));
       eps += 0.00256 * cos(dtr(omega));
    }

        /* Return Sun's longitude and radius vector */

    *slong = theta;
    *rv = (1.0000002 * (1 - e * e)) / (1 + e * cos(dtr(v)));

    /* Determine solar co-ordinates. */

    *ra= fixangle(rtd(atan2(cos(dtr(eps)) * sin(dtr(theta)), cos(dtr(theta)))));
    *dec = rtd(asin(sin(dtr(eps)) * sin(dtr(theta))));
}

/*  GMST  --  Calculate Greenwich Mean Siderial Time for a given
          instant expressed as a Julian date and fraction.    */

static double gmst(jd)
double jd;
{
    double t, theta0;


    /* Time, in Julian centuries of 36525 ephemeris days,
       measured from the epoch 1900 January 0.5 ET. */

    t = ((floor(jd + 0.5) - 0.5) - 2415020.0) / 36525.0;

    theta0 = 6.6460656 + 2400.051262 * t + 0.00002581 * t * t;

    t = (jd + 0.5) - (floor(jd + 0.5));

    theta0 += (t * 24.0) * 1.002737908;

    theta0 = (theta0 - 24.0 * (floor(theta0 / 24.0)));

    return theta0;
}

/*  PROJILLUM  --  Project illuminated area on the map.  */

static void projillum(wtab, xdots, ydots, dec)
short *wtab;
int xdots, ydots;
double dec;
{
    int i, j, ftf = TRUE, ilon, ilat, lilon, lilat, xt;
    double m, x, y, z, th, lon, lat, s, c;

    /* Clear unoccupied cells in width table */

    for (i = 0; i < ydots; i++)
       wtab[i] = -1;

    /* Build transformation for declination */

    s = sin(-dtr(dec));
    c = cos(-dtr(dec));

    /* Increment over a semicircle of illumination */

    for (th = -(M_PI / 2); th <= M_PI / 2 + 0.001;
         th += M_PI / TERMINC) {

       /* Transform the point through the declination rotation. */

       x = -s * sin(th);
       y = cos(th);
       z = c * sin(th);

       /* Transform the resulting co-ordinate through the
          map projection to obtain screen co-ordinates. */

       lon = (y == 0 && x == 0) ? 0.0 : rtd(atan2(y, x));
       lat = rtd(asin(z));

       lat = 90 - atan( pow( 10.0, -lat*(M_PI/180) ) ) * (180/M_PI*2);
/*       lat = 90 - atan( pow( 10.0, (-lat*(M_PI/180)*height/ydots + miny - subtr)/divis ) ) *
           (180/M_PI*2); */
       ilat = /*ydots -*/ ((long)lat + 90) * ydots / 180;
       ilat = ydots - (long)(ilat/*-miny*/) * height / 330;
       ilon = (long)(lon * xdots) / 360;

       if (ftf) {

          /* First time.  Just save start co-ordinate. */

          lilon = ilon;
          lilat = ilat;
          ftf = FALSE;
       } else {

          /* Trace out the line and set the width table. */

          if (lilat == ilat) {
         wtab[(ydots - 1) - ilat] = ilon == 0 ? 1 : ilon;
          } else {
         m = ((double) (ilon - lilon)) / (ilat - lilat);
         for (i = lilat; i != ilat; i += sgn(ilat - lilat)) {
            xt = lilon + floor((m * (i - lilat)) + 0.5);
            if( (j = (ydots - 1) - i) >= 0 && j<ydots )
                wtab[j] = xt == 0 ? 1 : xt;
         }
          }
          lilon = ilon;
          lilat = ilat;
       }
    }

    /* Now tweak the widths to generate full illumination for
       the correct pole. */

    if (dec < 0.0) {
       ilat = ydots - 1;
       lilat = -1;
    } else {
       ilat = 0;
       lilat = 1;
    }

    for (i = ilat; i != ydots / 2; i += lilat) {
       if (wtab[i] != -1) {
          while (TRUE) {
         wtab[i] = xdots / 2;
         if (i == ilat)
            break;
         i -= lilat;
          }
          break;
       }
    }
}

/*  XSPAN  --  Complement a span of pixels.  Called with line in which
           pixels are contained, leftmost pixel in the  line,  and
           the   number   of   pixels   to     complement.   Handles
           wrap-around at the right edge of the screen.  */

static void xspan(pline, leftp, npix)
int pline, leftp, npix;
{
    leftp += npix;				/* invert XOR */
    npix = center.g_w - npix - 1;		/* invert XOR */
    leftp = leftp % center.g_w;
    if ((leftp + npix) > center.g_w) {
       V pw_vector(leftp, pline, center.g_w - 1, pline);
       V pw_vector(0, pline, (leftp + npix) - (center.g_w + 1),
         pline );
    } else {
       V pw_vector(leftp, pline, leftp + (npix - 1), pline );
    }
}

/*  Draw illuminated portion of the globe.  */

void draw_sun(void)
{
    int i, ol, oh, nl, nh;

    vswr_mode(vdi_hand, MD_XOR);
    vsl_color(vdi_hand, BLACK);
    for (i = 0; i < center.g_h; i++) {

       /* If line is off in new width table but is set in
          the old table, clear it. */

       if (mt_wtab[i] < 0) {
          if (mt_otab[i] >= 0) {
         xspan(i, (mt_onoon - mt_otab[i]) + center.g_w, mt_otab[i] * 2); 
/*         xspan(i, (mt_otab[i] - mt_onoon) + center.g_w, center.g_w - mt_otab[i]*2); */
          }
       } else {

          /* Line is on in new width table.  If it was off in
         the old width table, just draw it. */

          if (mt_otab[i] < 0) {
         xspan(i, (mt_noon - mt_wtab[i]) + center.g_w, mt_wtab[i] * 2);
/*         xspan(i, (mt_wtab[i] - mt_noon) + center.g_w, center.g_w - mt_wtab[i]*2); */
          } else {

         /* If both the old and new spans were the entire
                    screen, they're equivalent. */

         if ((mt_otab[i] == mt_wtab[i]) && (mt_wtab[i] == (center.g_w / 2)))
            continue;

         /* The line was on in both the old and new width
            tables.  We must adjust the difference in the
            span.  */

         ol =  ((mt_onoon - mt_otab[i]) + center.g_w) % center.g_w;
         oh = (ol + mt_otab[i] * 2) - 1;
         nl =  ((mt_noon - mt_wtab[i]) + center.g_w) % center.g_w;
         nh = (nl + mt_wtab[i] * 2) - 1;

         /* If spans are disjoint, erase old span and set
            new span. */

         if (oh < nl || nh < ol) {
            xspan(i, ol, (oh - ol) + 1);
            xspan(i, nl, (nh - nl) + 1);
         } else {
            /* Clear portion(s) of old span that extend
               beyond end of new span. */
            if (ol < nl) {
               xspan(i, ol, nl - ol);
               ol = nl;
            }
            if (oh > nh) {
               xspan(i, nh + 1, oh - nh);
               oh = nh;
            }
            /* Extend existing (possibly trimmed) span to
               correct new length. */
            if (nl < ol) {
               xspan(i, nl, ol - nl);
            }
            if (nh > oh) {
               xspan(i, oh + 1, nh - oh);
            }
         }
          }
       }
       mt_otab[i] = -1;  /*mt_wtab[i];*/
    }
  wait_draw = 0;
}
