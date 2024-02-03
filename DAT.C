#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "ctype.h"
#include "aes.h"
#include "wc.h"
#include "worldclk.h"

typedef struct
{
  char desc[11];
  char size, count;
  void *data;
} DAT;

DAT dat[] = { { "Window", 	2, 4, &wsize.g_x },
            { "Sort",		1, 1, &sort },
            { "am_pm", 		1, 1, &ampm },
            { "DispCoords", 	1, 1, &disp_coords },
            { "DispDay",	1, 1, &disp_day },
            { "DispHours",	1, 1, &disp_hours },
            { "DispTime",	1, 1, &disp_time },
            { "Daylight",	1, 2*sizeof(DAY)+1, 0L },
            { "" } };

double dscale( double *d, double scale )
{
  double sign = *d<0.0 ? -1 : 1, t;
  
  *d = fabs(*d);
  t = trunc(*d);
  return sign * ((*d - t) * scale + t);
}

void fix_coord( double *d )
{
  *d = dscale( d, 100.0/60.0 );
}

char *next_str( char *p )
{
  while( *p && !isspace(*p) ) p++;
  while( *p && isspace(*p) ) p++;
  return p;
}

void read_str( char *p, int skip, char *name, int size )
{
  while( --skip>=0 ) p = next_str(p);
  strncpy( name, p, size );
  name[size] = 0;
  if( (p = strchr( name, '\n' )) != 0 ) *p = 0;
}

DAYLT *add_daylt( int num )
{
  int size;
  
  if( num >= num_daylt )
  {
    size = (num+1)*sizeof(DAYLT);
    if( !daylt ) daylt = malloc(size);
    else daylt = realloc( daylt, size );
    if( !daylt ) return 0L;
    memset( daylt+num_daylt, 0, sizeof(DAYLT)*(num-num_daylt+1) );
    num_daylt = num+1;
  }
  return &daylt[num];
}

char *tack_fname( char *name )
{
  strcpy( strrchr( loadpath, '\\' ) + 1, name );
  return loadpath;
}

FILE *_fopen( char *name, char *mode )
{
  return fopen( tack_fname(name), mode );
}

int read_dat(void)
{
  FILE *f;
  CITY *c;
  int n, i;
  char buf[100], buf2[10], *p;
  float fl;
  DAT *d;
  void *v;
  
  if( (f = _fopen("WORLDCLK.DAT","r")) != 0 )
  {
    for(;;)
      if( fgets( buf, sizeof(buf), f ) == NULL ) break;
      else if( buf[0]=='*' )
      {
        for( d=dat; d->desc[0]; d++ )
          if( !strncmp( buf+1, d->desc, strlen(d->desc) ) )
          {
            p = next_str(buf);
            if( (v = d->data) == 0L )
            {
              sscanf( p, "%c", buf2 );
              if( (v = add_daylt((buf2[0]&0x5F)-'A')) == 0L ) break;
              p = next_str(p);
            }
            for( n=0; n<d->count; n++ )
            {
              sscanf( p, "%d", &i );
              if( d->size==1 ) *((char *)v)++ = i;
              else *((int *)v)++ = i;
              p = next_str(p);
            }
            if( d->data==0L ) read_str( p, 0, (char *)v, sizeof(daylt->name)-1 );
            break;
          }
      }
      else
    {
      extra_cities = 10;
      cities++;
      if( !city ) city = malloc(11*sizeof(CITY));
      else city = realloc( city, (cities+10)*sizeof(CITY) );
      if( !city )
      {
        cities = extra_cities = 0;
        return 0;
      }
      c = &city[cities-1];
      if( (n=sscanf( buf, "%g %s %lg %lg", &fl, buf2, &c->lat,
          &c->lng )) > 0 )
      {
        c->s.active = (buf2[0]&=0x5f)=='A';
        if( (buf2[1]&0x5f)=='H' ) here = cities-1;
        c->s.daylight = (buf2[2]&0x5f)-'A';
        c->zone = fl * 60;
        fix_coord( &c->lng );
        fix_coord( &c->lat );
        read_str( buf, n, c->name, sizeof(c->name)-1 ); 
      }
      else
      {
        extra_cities++;
        cities--;
        break;
      }
    }
    fclose(f);
    city_coords();
    return 1;
  }
  return 0;
}

double fix_coord2( double d )
{
  return dscale( &d, 60.0/100.0 );
}

void write_dat(void)
{
  FILE *f;
  CITY *c;
  int n, dy;
  DAT *d;
  void *v;
  DAYLT *day;
  
  graf_mouse( BUSYBEE, 0L );
  if( (f = _fopen("WORLDCLK.DAT","w")) != 0 )
  {
    dy = 0;
    day = daylt;
    for( d=dat; d->desc[0]; d++ )
    {
      if( (v = d->data) == 0L )
      {
        if( dy++ == num_daylt ) continue;	/* out of daylts: go inc d */
        v = day++;
        fprintf( f, "*%s %c", d->desc, dy+'a'-1 );
      }
      else fprintf( f, "*%s", d->desc );
      for( n=0; n<d->count; n++ )
        fprintf( f, " %d", d->size==1 ? *((char *)v)++ :
            *((int *)v)++ );
      if( !d->data )
      {
        fprintf( f, "\t%s", v );
        d--;		/* daylt list: don't inc d */
      }
      fputc( '\n', f );
    }
    for( n=0, c=city; n<cities; n++, c++ )
      fprintf( f, "%g\t%c%c%c\t%0.2lg %0.2lg\t%s\n", (float)c->zone/60, c->s.active?'a':'-',
          n==here?'h':'-', c->s.daylight>=0?c->s.daylight+'a':'-',
          fix_coord2(c->lat), fix_coord2(c->lng), c->name );
    fclose(f);
    list_changed = 0;
  }
  else alert(BADDAT);
  graf_mouse( ARROW, 0L );
}

int cmp_name( CITY *a, CITY *b )
{
  return strcmp( a->name, b->name );
}

int cmp_ns( CITY *a, CITY *b )
{
  double d = b->lat - a->lat;

  return d<0 ? -1 : (d>0 ? 1 : 0 );
}

int cmp_ew( CITY *a, CITY *b )
{
  double d = a->lng - b->lng;

  return d<0 ? -1 : (d>0 ? 1 : 0);
}

int sort_cities( int sort, int move )
{
  static int (*compare[3])( CITY *a, CITY *b ) =
      { cmp_name, cmp_ns, cmp_ew };
  int i, ret=-1, h;

  if( cities )
  {
    for( i=0; i<cities; i++ )
      city[i].old_ind = i;
    qsort( city, cities, sizeof(CITY), compare[sort] );
    h = here;
    here = -1;
    for( i=0; i<cities; i++ )
    {
      if( ret<0 && city[i].old_ind==move ) ret = i;
      if( here<0 && city[i].old_ind==h ) here = i;
    }
  }
  return ret;
}
