/* fix inconify for MTOS
try reducing number of pts in v_plines even more
 */
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "new_aes.h"
#include "multevnt.h"
#include "xwind.h"
#include "vdi.h"
#include "tos.h"
#include "math.h"
#include "ctype.h"
#include "worldclk.h"
#include "time.h"
#define EXTERN
#include "wc.h"

#define pts _VDIParBlk.ptsin

int char_w, char_h, bar_h;
int cliparr[4];
OBJECT *timepop, *wform;
int pixw, pixh;
unsigned int *data;
char mark_plus, ht_dep, **day_str[3], gem_err, **title,
    drew_hours, iconified;
int minx, width;
int etype=MU_MESAG, last_hour;
GRECT g;

typedef struct { int x, y, b, k; } Mouse;
typedef struct { int x, y, w, h; } Rect;

EMULTI emulti = { 0, 1, 1, 1,  0, 0, 0, 0, 0,  1, 0, 0, 0, 0,  0, 0 };
int work_in[] = { 1, 7, 1, 1, 1, 1, 1, 1, 1, 1, 2 }, work_out[57];

void undisp_city(void)
{
  if( cur_city )
  {
    graf_blit( 0L, &g );
    cur_city = 0L;
  }
}

int is_divis( int y, int n )
{
  return y%n == 0;
}

int is_leap( struct tm *tmptr )
{
  int y = tmptr->tm_year;
  
  return (y&3)==0 && (!is_divis(y,100) || is_divis(y,400));
}

int year_hr( struct tm *tmptr, DAY *d )
{
  static int daysmo[12] = { 0,
                            31,
                            31+28,
                            31+28+31,
                            31+28+31+30,
                            31+28+31+30+31,
                            31+28+31+30+31+30,
                            31+28+31+30+31+30+31,
                            31+28+31+30+31+30+31+31,
                            31+28+31+30+31+30+31+31+30,
                            31+28+31+30+31+30+31+31+30+31,
                            31+28+31+30+31+30+31+31+30+31+30 },
             permo[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  int day, md, p;
  
  day = daysmo[d->month-1];
  if( d->month > 2 ) day += is_leap(tmptr);
  if( !d->which ) day += d->day;
  else
  {
    if( (md = (tmptr->tm_mday - tmptr->tm_wday + d->day)) < 0 ) md += 7;
    else md %= 7;
    if( d->which < 0 )
    {
      md += 28;
      p = permo[d->month-1];
      if( d->month==2 ) p += is_leap(tmptr);
      while( md > p ) md -= 7;
      day += md + 7*(d->which+1);
    }
    else day += md + 7*(d->which-1);
  }
  return (day-1)*24 + d->hour;
}

int is_daylight( struct tm *tmptr, CITY *c )
{
  int now, start, end;
  DAYLT *d;
  
  if( c->s.daylight < 0 ) return 0;
  d = &daylt[c->s.daylight];
  start = year_hr( tmptr, &d->start );
  end = year_hr( tmptr, &d->end );
  now = tmptr->tm_yday*24 + tmptr->tm_hour;
  if( start<end ) return now>=start && now<end;
  return now>=start || now<end;
}

long relday( struct tm *t )
{
  return t->tm_year * 356L + t->tm_yday;
}

void get_gmt( long *today, time_t *ti )
{
  struct tm *tmptr;
  CITY *c;

  timezone = (c=&city[here])->zone * -60L;
  daylight = 0;
  *ti = time(0L);
  tmptr = localtime( ti );
  if( is_daylight( tmptr, c ) )
  {
    daylight = 1;
    *ti = time(0L);
  }
  if( today ) *today = relday(tmptr);
}

void hide_if( OBJECT *tree, int num, int true )
{
  unsigned int *i = &tree[num].ob_flags;
  
  if( true ) *i &= ~HIDETREE;
  else *i |= HIDETREE;
}

void disp_city( CITY *c )
{
  int arr[4], larr[10], l, m, lines;
  long today, other;
  char timestr[50], pm, **day;
  time_t ti;
  struct tm *tmptr;
  
  if( c==cur_city ) return;
  undisp_city();
  if( here>=0 )
  {
    get_gmt( &today, &ti );
    timezone = c->zone * -60L;
    daylight = 0;
    tmptr = localtime( &ti );
    if( is_daylight( tmptr, c ) )
    {
      daylight = 1;
      tmptr = localtime( &ti );
    }
    other = relday(tmptr);
    if( other < today ) day = day_str[0];
    else if( other == today ) day = day_str[1];
    else day = day_str[2];
    pm = 0;
    if( ampm )
      if( !tmptr->tm_hour ) tmptr->tm_hour = 12;
      else if( tmptr->tm_hour>=12 )
      {
        pm++;
        if( tmptr->tm_hour>12 ) tmptr->tm_hour-=12;
      }
    sprintf( timestr, "%d:%02d:%02d%s", tmptr->tm_hour, tmptr->tm_min,
        tmptr->tm_sec, ampm ? (pm ? " pm" : " am") : "" );
/*%    if( daylight ) strcat( timestr, " D" );*/
  }
  l=strlen( c->name );
  if( here>=0 )
  {
    if( (m=strlen(timestr)) > l ) l = m;
    if( disp_day && (m=strlen(*day)) > l ) l = m;
  }
  g.g_w = (l+1)*char_w + 2;
  g.g_x = c->x + 20;
  if( g.g_x + g.g_w > max.g_w ) g.g_x = max.g_w-g.g_w;
  lines = here>=0 ? (disp_day ? 3 : 2) : 1;
  g.g_h = char_h*lines + 8;
  g.g_y = c->y - g.g_h - 20;
  if( g.g_y < 0 ) g.g_y = 0;
  if( g.g_y + g.g_h > max.g_y+max.g_h ) g.g_y = max.g_y+max.g_h-g.g_w;
  timepop[0].ob_width = timepop[1].ob_width = timepop[2].ob_width =
     timepop[3].ob_width = g.g_w - 2;
  timepop[0].ob_height = g.g_h - 2;
  timepop[0].ob_x = g.g_x;
  timepop[0].ob_y = g.g_y;
  graf_blit( &g, 0L );
  cur_city = c;
  timepop[1].ob_spec.tedinfo->te_ptext = c->name;
  hide_if( timepop, 2, lines>1 );
  timepop[2].ob_spec.tedinfo->te_ptext = here>=0 ? timestr : "";
  hide_if( timepop, 3, lines>2 );
  timepop[3].ob_spec.tedinfo->te_ptext = here>=0 ? *day : "";
  objc_draw( timepop, 0, 8, g.g_x, g.g_y, g.g_w, g.g_h );
}

void xy_2_longlat( int x, int y, int *lng, int *rlng, int *lat, int *rlat )
{
  double l;
  
  l = (double)(x-center.g_x) / center.g_w * 360 - 180;
  *rlng = (l - (*lng = trunc(l))) * 60;
  l = 90 - atan( pow( 10.0,
      ((double)(y-center.g_y)*height/center.g_h + miny - subtr)/divis ) ) *
      (180/M_PI*2);
  *rlat = (l - (*lat = trunc(l))) * 60;
}

void city_coords(void)
{
  CITY *c;
  int i;
  
  for( c=city, i=cities; --i>=0; c++ )
  {
    c->x = (int)((c->lng+180) / 360 * center.g_w) + center.g_x;
    c->y = (int)((log10( tan( (90-c->lat)*(M_PI/360) ) ) * divis + subtr - miny) *
        center.g_h / height) + center.g_y;
  }
}

void real_coord( unsigned int y, unsigned int *yo, unsigned int x, unsigned int *xo )
{
  *yo = (unsigned long)(y-miny) * center.g_h / height + center.g_y - 1;
  *xo = (unsigned long)(x-minx) * center.g_w / width + center.g_x - 1;
}

void plot_cities( int h )
{
  int i;
  CITY *c;
  
  if( mark_plus ) vsl_color( vdi_hand, RED );
  for( c=city, i=0; i<cities; i++, c++ )
    if( c->s.active )
    {
      pts[0] = c->x-2;
      pts[2] = c->x+2;
      pts[1] = pts[3] = c->y;
      v_pline( h, 2, pts );
      pts[0] = pts[2] -= 2;
      pts[1] -= 2;
      pts[3] += 2;
      v_pline( h, 2, pts );
      if( !mark_plus )
      {
        pts[0]--;
        pts[2]++;
        pts[1]++;
        pts[3]--;
        v_pline( h, 2, pts );
        pts[1] = c->y+1;
        pts[3] = c->y-1;
        v_pline( h, 2, pts );
      }
    }
}

void set_clip( GRECT *r )
{
  cliparr[2] = (cliparr[0] = r->g_x) + r->g_w - 1;
  cliparr[3] = (cliparr[1] = r->g_y) + r->g_h - 1;
  vs_clip( vdi_hand, 1, cliparr );
}

void goto_city(void)
{
  CITY *c = &city[ed_goto];
  int x, i;
  
  vswr_mode( vdi_hand, MD_XOR );
  vsf_perimeter( vdi_hand, 1 );
  vsf_interior( vdi_hand, FIS_HOLLOW );
  set_clip( &max );
  for( i=0; i<8; i++ )
    for( x=20; x>0; x-=3 )
    {
      v_ellipse( vdi_hand, c->x, c->y, x, x*pixw/pixh );
      Vsync();
    }
  vsf_perimeter( vdi_hand, 0 );
}

void draw_hours( int vdi_hand )
{
  time_t ti;
  struct tm *tmptr;
  int l, x, y, h, w, bw, arr[4], x2;
  char hstr[10], odd;

  if( here>=0 && (drew_hours=disp_hours) != 0 )
  {
    get_gmt( 0L, &ti );
    tmptr = gmtime( &ti );
    if( (h=(last_hour=tmptr->tm_hour)-12) < 0 ) h += 24;
    y = center.g_y;
    vsf_interior( vdi_hand, FIS_SOLID );
    vsf_color( vdi_hand, WHITE );
    arr[2] = (arr[0] = wsize.g_x) + wsize.g_w - 1;
    arr[3] = (arr[1] = y) + 7;
    v_bar( vdi_hand, arr );
    vsf_color( vdi_hand, BLACK );
    for( l=-1800, odd=0; l<1800; l+=w )
    {
      w = l==-1800 || l==1725 ? 75 : 150;
      bw = (long)w * wsize.g_w / 3600L;
      x = (int)((long)(l+1800) * wsize.g_w / 3600L) + wsize.g_x;
      if( !ampm ) itoa( h, hstr, 10 );
      else sprintf( hstr, "%d%c", !h ? 12 : (h>12 ? h-12 : h),
          h>=12 ? 'p' : 'a' );
      x2 = (bw - 6*strlen(hstr) + 1)>>1;
      if( x2>=0 ) v_gtext( vdi_hand, x + x2, y+1, hstr );
      if( (odd^=1) != 0 )
      {
        vswr_mode( vdi_hand, MD_XOR );
        arr[2] = (arr[0] = x) + bw;
        v_bar( vdi_hand, arr );
        vswr_mode( vdi_hand, MD_REPLACE );
      }
      if( ++h == 24 ) h = 0;
    }
  }
}

int cdecl draw_map( PARMBLK *pb )
{
  unsigned int *d, i;
  int *p, j;
  
  vswr_mode( vdi_hand, MD_REPLACE );
  set_clip( (GRECT *)&pb->pb_xc );
  if( draw_mode&DRAW_MAP )
  {
    vsl_type( vdi_hand, SOLID );
    vsf_color( vdi_hand, WHITE );
    vsf_interior( vdi_hand, FIS_SOLID );
    vr_recfl( vdi_hand, cliparr );
    vsl_color( vdi_hand, BLACK );
    d = data;
    while( *d )
    {
      i = *d++;
      for( p = pts, j=i; --j>=0; )
        real_coord( *d++, (unsigned int *)p++, *d++, (unsigned int *)p++ );
      v_pline( vdi_hand, i, pts );
    }
  }
  if( draw_mode&DRAW_HOURS ) draw_hours( vdi_hand );
  if( draw_mode&DRAW_CITIES ) plot_cities( vdi_hand );
  if( disp_sun && draw_mode&DRAW_SUN ) draw_sun();
  return pb->pb_currstate;
}

void fixrec( unsigned int *rec, int i )
{
  while( --i>=0 )
    *rec++ = (*rec>>8) | (*rec<<8);
}

void gem_size(void)
{
  DTA *old, new;
  
  old = Fgetdta();
  Fsetdta(&new);
  if( (gem_err = Fsfirst( tack_fname("WORLDCLK.GEM"), 0x21 )) == 0 )
    if( (data = malloc(new.d_length-26)) == 0 ) gem_err = 1;
  Fsetdta(old);
}

int read_gem(void)
{
  FILE *f;
  int i;
  unsigned int *prev, *dt;
  unsigned int rec[13];

  if( !gem_err && (f = _fopen("WORLDCLK.GEM","rb")) > 0 )
  {
    fread( rec, 13, 2, f );
    fixrec( rec, 13 );
    minx = rec[4];
    miny = rec[5];
    fseek( f, rec[1]<<1, 0 );
    dt = data;
    while( fread( rec, 8, 1, f ) != 0 )
    {
      fixrec( rec, 4 );
      if( rec[0]==9 )
      {
        prev = 0L;
        while( rec[1] > 0 )
        {
          i = rec[1] > 127 ? 127 : rec[1];
          *dt++ = i;
          if( prev != 0 )
          {
            ++*(dt-1);
            *((long *)dt)++ = *(long *)prev;
          }
          fread( dt, i, 4, f );
          rec[1] -= i;
          for( ; --i>=0; )
          {
            if( (*dt++ = (*dt>>8) | (*dt<<8)) > width ) width = *(dt-1);
            if( (*dt++ = (*dt>>8) | (*dt<<8)) > height ) height = *(dt-1);
          }
          prev = dt-2;
          *dt = 0;	/* last segment */
        }
      }
      else fseek( f, rec[1]*4+rec[2]*2, 1 );
    }
    width -= minx + 1;
    height -= miny - 2;
    subtr = 229 + miny;
    fclose(f);
    return dt != data;
  }
  else gem_err = -1;
  return 0;
}

void quit(void)
{
  if( list_changed && alert(SAVEME)==1 ) action(PSAVE);
  if( data ) free(data);
  if( city ) free(city);
  if( vdi_hand ) v_clsvwk( vdi_hand );
  rsrc_free();
  appl_exit();
  exit(0);
}

int alert( int num )
{
  char **str;
  
  rsrc_gaddr( 15, num, &str );
  return form_alert( 1, *str );
}

int intersect( GRECT *r1, GRECT *res )
{
  int x, y;
  
  x = r1->g_x > res->g_x ? r1->g_x : res->g_x;
  y = r1->g_y > res->g_y ? r1->g_y : res->g_y;
  res->g_w = (r1->g_x+r1->g_w < res->g_x+res->g_w ? r1->g_x+r1->g_w : res->g_x+res->g_w) - x;
  res->g_h = (r1->g_y+r1->g_h < res->g_y+res->g_h ? r1->g_y+r1->g_h : res->g_y+res->g_h) - y;
  res->g_x = x;
  res->g_y = y;
  return( res->g_w > 0 && res->g_h > 0 );
}

void redraw_wind( GRECT *g, int start )
{
  GRECT r1;
  
  if( !intersect( &max, g ) ) return;
  wind_update( BEG_UPDATE );
  wind_get( handle, WF_FIRSTXYWH, &r1.g_x, &r1.g_y, &r1.g_w, &r1.g_h );
  while( r1.g_w && r1.g_h )
  {
    if( intersect( g, &r1 ) ) objc_draw( wform, start, 8, r1.g_x,
        r1.g_y, r1.g_w, r1.g_h );
    wind_get( handle, WF_NEXTXYWH, &r1.g_x, &r1.g_y, &r1.g_w, &r1.g_h );
  }
  wind_update( END_UPDATE );
}

void resize(void)
{
  if( disp_sun ) sunclock(1);
  *(GRECT *)&wform[0].ob_x = center;
  wform[MAINCOOR].ob_x = (wform[MAINMENU].ob_x = (wform[MAINSIZE].ob_x =
      center.g_w - wform[MAINMENU].ob_width) - wform[MAINMENU].ob_width) -
      wform[MAINCOOR].ob_width - 2;
  wform[MAINCOOR].ob_y = wform[MAINMENU].ob_y = wform[MAINSIZE].ob_y =
      center.g_h - wform[MAINMENU].ob_height;
  city_coords();
}

void fix_size( int *w, int *h, int adj )
{
  for(;;)
  {
    if( ht_dep ) *h = (long)*w * pixw / pixh * height / width + adj;
    else *w = (long)(*h-adj) * pixh / pixw * width / height;
    if( !(*w&1) ) return;	/* make sure even width for sunpath */
    --*w;
  }
}

void fix_wind(void)
{
  wind_calc( WC_WORK, CLOSER|MOVER|NAME|FULLER,
      wsize.g_x, wsize.g_y, wsize.g_w, wsize.g_h,
      &center.g_x, &center.g_y, &center.g_w, &center.g_h );
  bar_h = center.g_y - wsize.g_y;
  fix_size( &center.g_w, &center.g_h, 0 );
  wind_calc( WC_BORDER, CLOSER|MOVER|NAME|FULLER,
      center.g_x, center.g_y, center.g_w, center.g_h,
      &wsize.g_x, &wsize.g_y, &wsize.g_w, &wsize.g_h );
}

void init_xor(void)
{
  vsl_color( vdi_hand, 1 );
  vsl_type( vdi_hand, 7 );
  vswr_mode( vdi_hand, 3 );
  vsl_udsty( vdi_hand, 0x5555 );
}

void draw_xor( Rect *r )
{
  int arr[10];
  
  arr[2] = arr[4] = (arr[0] = arr[6] = arr[8] = r->x) + r->w - 1;
  arr[5] = arr[7] = (arr[1] = arr[3] = arr[9] = r->y) + r->h - 1;
  graf_mouse( M_OFF, 0L );
  v_pline( vdi_hand, 5, arr );
  graf_mouse( M_ON, 0L );
}

int resize_mouse( GRECT *r )
{
  int x, y, xo, yo, ow, oh, dx, dy, lagx, lagy;
  register int state=0;
  Mouse m;

  wind_update( BEG_UPDATE );
  graf_mkstate( &m.x, &m.y, &m.b, &m.k );
  lagx = r->g_x+(ow=r->g_w)-m.x;
  lagy = r->g_y+(oh=r->g_h)-m.y;
  set_clip( &max );
  init_xor();
  while( m.b & 1 )
  {
    xo = r->g_x+r->g_w;
    yo = r->g_y+r->g_h;
    draw_xor( (Rect *)r );
    do
    {
      graf_mkstate( &m.x, &m.y, &m.b, &m.k );
      m.x += lagx - r->g_x;
      m.y += lagy - r->g_y;
      if( m.x > max.g_w ) m.x = max.g_w;
      else if( m.x < 100 ) m.x = 100;
      if( m.y > max.g_h ) m.y = max.g_h;
      else if( m.y < 50 ) m.y = 50;
      fix_size( &m.x, &m.y, bar_h );
      dx = m.x + r->g_x - xo;
      dy = m.y + r->g_y - yo;
    }
    while( (m.b&1) && !dx && !dy );
    draw_xor( (Rect *)r );
    r->g_w += dx;
    r->g_h += dy;
    state = 1;
  }
  wind_update( END_UPDATE );
  if( r->g_w == ow && r->g_h == oh ) return 0;
  return( state );
}

long get_idt_fmt(void)
{
  static unsigned int vals[] = { (0<<12) | (0<<8) | 0, 	  /* USA */
  				 (1<<12) | (1<<8) | '.',  /* Germany */
  				 (1<<12) | (1<<8) | 0,	  /* France */
  				 (1<<12) | (1<<8) | '.',  /* UK */
  				 (1<<12) | (1<<8) | 0,	  /* Spain */
  				 (1<<12) | (1<<8) | 0 };  /* Italy */
  unsigned int mode = (*(SYSHDR **)0x4f2)->os_base->os_palmode>>1;

  return mode>=sizeof(vals)/2 ? ((1<<12) | (2<<8) | '-') : vals[mode];
}

#define CJar_cookie     0x434A6172L     /* "CJar" */
#define CJar_xbios      0x434A          /* "CJ" */
#define CJar_OK         0x6172          /* "ar" */
#define IDT_cookie      0x5F494454L     /* "_IDT" */
#define CJar(mode,cookie,value)         xbios(CJar_xbios,mode,cookie,value)

void shorten_bb( BITBLK *bb )
{
  int j, *ptr;

  /* clear-out every second line in the image data */
  for( j=bb->bi_hl>>1, ptr=bb->bi_pdata; --j>=0; ptr+=bb->bi_wb )
    memset( ptr, 0, bb->bi_wb );
  /* now, expand the image description so that every second (used)
     line becomes tacked onto the end of the previous line */
  bb->bi_x += bb->bi_wb<<3;
  bb->bi_hl >>= 1;
  bb->bi_wb <<= 1;
}

void fix_rsc(void)
{
  RSHDR *r;
  unsigned int i;
  OBJECT *o;
  
  if( has_Geneva )
  {
    r = *(RSHDR **)&_GemParBlk.global[7];
    for( i=0, o=(OBJECT *)((long)r+r->rsh_object); i<r->rsh_nobs; i++, o++ )
      if( (o->ob_state&(X_MAGMASK|X_PREFER)) == (X_MAGIC|X_PREFER) ) o->ob_height += 2;
  }
  if( char_h < 16 ) shorten_bb( (BITBLK *)((long)r+r->rsh_bitblk) );
}

void redraw_hours(void)
{
  GRECT g;
  
  if( handle>0 && (disp_hours || drew_hours) )
  {
    draw_mode = disp_hours ? DRAW_HOURS : -1;
    g = center;
    g.g_h = 8;
    redraw_wind( &g, 0 );
    draw_mode = -1;
  }
}

void wind_name(void)
{
  time_t ti;
  struct tm *tmptr;
  static char name[50];
  char pm;

  if( !disp_time || here<0 ) wind_set( handle, WF_NAME, *title + 2 );
  else
  {
    get_gmt( 0L, &ti );
    tmptr = localtime( &ti );
    if( ampm )
    {
      pm = 0;
      if( tmptr->tm_hour>=12 )
      {
        if( tmptr->tm_hour > 12 ) tmptr->tm_hour -= 12;
        pm++;
      }
      else if( !tmptr->tm_hour ) tmptr->tm_hour = 12;
    }
    sprintf( name, "%s  %d:%02d%s", *title + 2, tmptr->tm_hour,
        tmptr->tm_min, ampm ? (pm ? " pm" : " am") : "" );
    wind_set( handle, WF_NAME, name );
  }
}

void check_time(void)
{
  time_t ti;
  struct tm *tmptr;
  static int last_min = -1;

  if( here<0 ) return;
  get_gmt( 0L, &ti );
  tmptr = gmtime( &ti );
  if( tmptr->tm_hour != last_hour )
  {
    last_hour = tmptr->tm_hour;
    if( disp_hours ) redraw_hours();
    if( disp_time ) wind_name();
    last_min = tmptr->tm_min;
  }
  else if( tmptr->tm_min != last_min )
  {
    if( disp_sun && sunclock(0) )
    {
      draw_mode = DRAW_SUN;
      redraw_wind( &max, 0 );
      draw_mode = -1;
    }
    if( disp_time ) wind_name();
    last_min = tmptr->tm_min;
  }
}

void mouse(void)
{
  int x, y, b, k, i, lastx=-1, lasty=-1, lng, lat, rlng, rlat;
  CITY *c, *last=0L;
  GRECT cobj;
  char buf[20];

  wind_update( BEG_UPDATE );
  wind_update( BEG_MCTRL );
  objc_offset( wform, MAINCOOR, &cobj.g_x, &cobj.g_y );
  *(long *)&cobj.g_w = *(long *)&wform[MAINCOOR].ob_width;
  for(;;)
  {
    graf_mkstate( &x, &y, &b, &k );
    if( b&1 )
    {
      for( c=city, i=0; i<cities; i++, c++ )
        if( c->s.active &&
            x >= c->x-2 && x <= c->x+2 && y >= c->y-2 && y <= c->y+2 )
        {
          disp_city( c );
          break;
        }
      if( i==cities ) undisp_city();
      if( disp_coords && (cur_city || (x != lastx || y != lasty)) &&
          (!cur_city || cur_city!=last) )
      {
        if( (last=cur_city) != 0 )
        {
          rlng = (cur_city->lng - (lng = trunc(cur_city->lng))) * 60;
          rlat = (cur_city->lat - (lat = trunc(cur_city->lat))) * 60;
        }
        else xy_2_longlat( lastx = x, lasty = y, &lng, &rlng, &lat, &rlat );
        if( x < center.g_x || x >= center.g_x+center.g_w ||
            y < center.g_y || y >= center.g_y+center.g_h ) buf[0] = 0;
        else sprintf( buf, "%dø%02d'%c,%dø%02d'%c", abs(lat), abs(rlat),
            lat<0?'S':'N', abs(lng), abs(rlng), lng<0?'W':'E' );
        sprintf( wform[MAINCOOR].ob_spec.tedinfo->te_ptext, "%17s", buf );
        wform[MAINCOOR].ob_flags &= ~HIDETREE;
        redraw_wind( &cobj, MAINCOOR );
      }
    }
    else break;
  }
  undisp_city();
  if( disp_coords && !(wform[MAINCOOR].ob_flags & HIDETREE) )
  {
    wform[MAINCOOR].ob_flags |= HIDETREE;
    redraw_wind( &cobj, 0 );
  }
  wind_update( END_UPDATE );
  wind_update( END_MCTRL );
}

void close_wind(void)
{
  if( handle>0 )
  {
    wind_close(handle);
    wind_delete(handle);
    handle = 0;
  }
  etype = MU_MESAG;
  if( _app ) quit();        /* running as a PRG, so quit */
}

void do_iconify( int handle, int buf[] )
{
  iconified = 1;
  /* turn dialog off so that applist's size and position will not change */
  wind_set( handle, WF_ICONIFY, buf[4], buf[5], buf[6], buf[7] );
  /* get working area of iconified window */
  wind_get( handle, WF_WORKXYWH, &icon[0].ob_x, &icon[0].ob_y,
      &icon[0].ob_width, &icon[0].ob_height );
  /* center the icon within the form */
  icon[1].ob_x = (icon[0].ob_width - icon[1].ob_width) >> 1;
  icon[1].ob_y = (icon[0].ob_height - icon[1].ob_height) >> 1;
  /* new (buttonless) dialog in main window */
  wind_set( handle, X_WF_DIALOG, icon );
}

void do_uniconify( int handle, int buf[] )
{
  iconified = 0;
  /* turn dialog off so that icon's size and position will not change */
  wind_set( handle, X_WF_DIALOG, 0L );
  wind_set( handle, WF_UNICONIFY, buf[4], buf[5], buf[6], buf[7] );
}

int getcookie( long cookie, unsigned long *ptr )
{
    register long *cookiejar;
    long stack = Super(0L);

    if( (cookiejar = *(long **)0x5a0) == 0 )
    {
      Super((void *)stack);
      return 0;
    }
    do {
        if (*cookiejar == cookie)
        {
          if( ptr ) *ptr = *(cookiejar+1);
          Super((void *)stack);
          return CJar_OK;
        }
        else cookiejar += 2;
    } while (*cookiejar);
    Super((void *)stack);
    return 0;
}

int main(void)
{
  char multitask, inside=0, fulled=0, got_dat, infold=0;
  int buf[8], dum, i;
  long cook;

  divis = 215;  
  if( (i=CJar( 0, IDT_cookie, &idt_fmt ))==CJar_xbios )
      i = getcookie( IDT_cookie, &idt_fmt );
  if( i!=CJar_OK ) idt_fmt = Supexec( get_idt_fmt );
  ampm = (idt_fmt&0xF000) == 0;
  disp_coords = disp_time = disp_sun = 1;
  draw_mode = -1;
  here = ed_goto = -1;
  loadpath[0] = Dgetdrv()+'A';
  loadpath[1] = ':';
  Dgetpath( loadpath+2, 0 );
  if( !Fsfirst( "WORLDCLK", FA_SUBDIR ) )
  {
    strcat( loadpath, "\\WORLDCLK" );
    infold = 1;
  }
  strcat( loadpath, "\\" );
  got_dat = read_dat();
  init_sun(400);		/* means tallest sun image is 400 lines! */
  gem_size();
  apid = appl_init();
  has_Geneva = CJar( 0, GENEVA_COOKIE, &cook ) == CJar_OK && cook!=0;
  popup_ok = 0;
  if( _GemParBlk.global[0] >= 0x399 )
    if( appl_getinfo( 9, buf, buf+1, buf, buf ) && buf[1] ) popup_ok = 1;
  if( _GemParBlk.global[0] >= 0x400 ) shel_write( SHW_MSGTYPE, 1, 0, 0L, 0L );
  vdi_hand = graf_handle( &char_w, &char_h, &dum, &dum );
  multitask = _GemParBlk.global[1]==-1;	/* intentionaly false if MagiC */
  i = 0;
  if( infold ) i = rsrc_load( tack_fname("WORLDCLK.RSC") );
  if( !i ) i = rsrc_load( "WORLDCLK.RSC" );
  if( !i )
  {
    form_alert( 1, "[1][WORLDCLK.RSC|not found!][Ok]" );
  }
  else if( read_gem() )
  {
    wind_get( 0, WF_WORKXYWH, &max.g_x, &max.g_y, &max.g_w, &max.g_h );
    work_in[0] = Getrez() + 2;
    v_opnvwk( work_in, &vdi_hand, work_out );
    fix_rsc();
    if( !vdi_hand )
    {
      alert( ALVDI );
      goto bad;
    }
    if( !got_dat ) alert( ALDAT );
    if( wsize.g_y < max.g_y ) wsize.g_y = max.g_y;
    if( wsize.g_w <= 0 ) wsize.g_w = max.g_w>>1;
    else if( wsize.g_w > max.g_w ) wsize.g_w = max.g_w;
    if( wsize.g_h <= 0 ) wsize.g_h = max.g_h>>1;
    else if( wsize.g_h > max.g_h ) wsize.g_h = max.g_h;
    pixw = work_out[3];
    pixh = work_out[4];
    ht_dep = (long)max.g_h*pixh/pixw*width/height >
             (long)max.g_w*pixw/pixh*height/width;	/* find the short edge */
    vq_extnd( vdi_hand, 1, work_out );
    mark_plus = (vplanes = work_out[4]) > 1;
    vst_point( vdi_hand, 8, buf, buf, buf, buf );
    vst_alignment( vdi_hand, 0, 5, buf, buf );
    vsf_perimeter( vdi_hand, 0 );
    for( i=0; i<3; i++ )
      rsrc_gaddr( 15, YESTER+i, &day_str[i] );
    rsrc_gaddr( 0, TIME, &timepop );
    rsrc_gaddr( 0, MAIN, &wform );
    wform[0].ob_spec.userblk->ub_code = draw_map;
    wform[MAINCOOR].ob_flags |= HIDETREE;
    rsrc_gaddr( 0, CLKICON, &icon );
    rsrc_gaddr( 15, ACCNAME, &title );
    if( !_app || multitask ) menu_register( apid, *title );
    if( _app )          /* running as PRG */
    {
      graf_mouse( ARROW, 0L );
      goto open;
    }
    for(;;)
    {
      *(GRECT *)&emulti.m1x = center;
      *(GRECT *)&emulti.m2x = center;
      emulti.type = etype;
      multi_evnt( &emulti, buf );
      if( emulti.event&MU_MESAG )
        switch( buf[0] )
        {
          case AC_OPEN:
open:       if( handle<=0 )
            {
              if( (handle=wind_create( has_Geneva ? 
                  CLOSER|MOVER|NAME|FULLER|SMALLER :
                  CLOSER|MOVER|NAME|FULLER,
                  max.g_x, max.g_y, max.g_w, max.g_h )) > 0 )
              {
                fix_wind();
                wind_name();
                resize();
                wind_open( handle, wsize.g_x, wsize.g_y, wsize.g_w, wsize.g_h );
                etype = MU_BUTTON|MU_MESAG|MU_M1|MU_M2|MU_KEYBD;
              }
              else alert( NOWIND );
              break;
            }
            buf[3] = handle;      /* fall through if already open */
          case WM_TOPPED:
            wind_set( buf[3], WF_TOP, buf[3] );
            break;
          case WM_FULLED:
            wind_get( handle, (fulled^=1)!=0 ? WF_FULLXYWH : WF_PREVXYWH,
                &buf[4], &buf[5], &buf[6], &buf[7] );
            /* fall through */
          case WM_MOVED:
            if( iconified )
            {
              wind_set( handle, WF_CURRXYWH, buf[4], buf[5], buf[6], buf[7] );
              break;
            }
            wsize = *(GRECT *)&buf[4];
new_size:   fix_wind();
            wind_set( handle, WF_CURRXYWH, wsize.g_x, wsize.g_y, wsize.g_w, wsize.g_h );
            wind_get( handle, WF_WORKXYWH, &center.g_x, &center.g_y, &dum, &dum );
            resize();
            if( buf[0] != WM_MOVED )
              if( !has_Geneva ) redraw_wind( (GRECT *)&wsize, 0 );
              else
              {
                buf[0] = WM_REDRAW;
                buf[3] = handle;
                *(GRECT *)&buf[4] = center;
                appl_write( buf[1]=apid, 16, buf );
              }
            break;
          case AC_CLOSE:
            handle=0;
            iconified = 0;
          case WM_CLOSED:
            close_wind();
            break;
          case WM_ICONIFY:
          case WM_ALLICONIFY:
            if( buf[3]==handle && !iconified ) /* main window */
            {
              do_iconify( handle, buf );
              etype &= ~MU_BUTTON;
            }
            break;
          case WM_UNICONIFY:
            if( buf[3]==handle && iconified ) /* main window */
            {
              do_uniconify( handle, buf);
              etype |= MU_BUTTON;
            }
            break;
          case WM_REDRAW:
            redraw_wind( (GRECT *)&buf[4], 0 );
            if( ed_goto>=0 )
            {
              wind_update( BEG_UPDATE );
              goto_city();
              wind_update( END_UPDATE );
              ed_goto = -1;
            }
            break;
          case AP_TERM:
            quit();
        }
      if( _app && handle<=0 ) break;
      if( emulti.event&MU_KEYBD ) do_menu( emulti.mouse_k, emulti.key );
      if( (emulti.event&(MU_M2|MU_BUTTON)) == (MU_M2|MU_BUTTON) ) inside = 0;
      if( (emulti.event&(MU_M1|MU_BUTTON)) == MU_M1 ) inside = 1;
      if( emulti.event&MU_BUTTON && inside )
        switch( objc_find( wform, 0, 8, emulti.mouse_x, emulti.mouse_y ) )
        {
          case 0:
            mouse();
            break;
          case MAINMENU:
            do_menu( 0, 0 );
            break;
          case MAINSIZE:
            if( resize_mouse( &wsize ) )
            {
              buf[0] = 0;
              goto new_size;
            }
        }
      if( disp_hours || disp_time || disp_sun ) check_time();
    }
  }
  else if( gem_err<0 ) alert(NOGEM);
  else if( gem_err>0 ) alert(GEMMEM);
bad:
  /* An error occurred. If either running as an app or under Geneva or
     MultiTOS, get out */
  if( _app || multitask ) quit();
  /* run as a desk accessory, but an error occurred, so just go into an
     infinite loop looking for AP_TERM messages */
  for(;;)
  {
    evnt_mesag(buf);
    if( buf[0]==AP_TERM ) quit();
  }
}
