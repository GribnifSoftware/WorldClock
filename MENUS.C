#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "aes.h"
#include "xwind.h"
#include "vdi.h"
#include "tos.h"
#include "math.h"
#include "ctype.h"
#include "worldclk.h"
#include "wc.h"

char hours_redraw;
OBJECT *daypop;

void draw_if( OBJECT *o, int num, int draw )
{
  if( draw ) objc_draw( o, num, 8, 0, 0, 0, 0 );
}

void sel_if( OBJECT *o, int n, int sel, int draw )
{
  unsigned int *i = &o[n].ob_state, old;
  
  old = *i;
  if(sel) *i |= SELECTED;
  else *i &= ~SELECTED;
  if( *i != old ) draw_if( o, n, draw );
}

void enab_if( OBJECT *o, int n, int en, int draw )
{
  unsigned int *i = &o[n].ob_state, old;
  
  old = *i;
  if(en) *i &= ~DISABLED;
  else *i |= DISABLED;
  if( *i != old ) draw_if( o, n, draw );
}

void edit_if( OBJECT *o, int n, int ed )
{
  if(ed) o[n].ob_flags |= EDITABLE;
  else o[n].ob_flags &= ~EDITABLE;
}

int is_sel( OBJECT *o, int n )
{
  return o[n].ob_state & SELECTED;
}

void spf_date( char *ptr, int a, int b, int c, int usep )
{
  char sep;
  
  if( c>99 ) c -= 100;
  if( (sep = (char)idt_fmt) == 0 ) sep = '/';
  switch( (int)idt_fmt&0xf00 )
  {
    case 0x000:
      if( usep ) sprintf( ptr, "%02d%c%02d%c%02d", a, sep, b, sep, c );
      else sprintf( ptr, "%02d%02d%02d", a, b, c );
      break;
    case 0x100:
      if( usep ) sprintf( ptr, "%02d%c%02d%c%02d", b, sep, a, sep, c );
      else sprintf( ptr, "%02d%02d%02d", b, a, c );
      break;
    default:
      if( usep ) sprintf( ptr, "%02d%c%02d%c%02d", c, sep, a, sep, b );
      else sprintf( ptr, "%02d%02d%02d", c, a, b );
      break;
    case 0x300:
      if( usep ) sprintf( ptr, "%02d%c%02d%c%02d", c, sep, b, sep, a );
      else sprintf( ptr, "%02d%02d%02d", c, b, a );
  }
}

int conv_idt( int **arr, int *m, int *d, int *y )
{
  switch( idt_fmt&0xf00 )
  {
    case 0x000:
      arr[0] = m;
      arr[1] = d;
      arr[2] = y;
      return 0;
    case 0x100:
      arr[0] = d;
      arr[1] = m;
      arr[2] = y;
      return 1;
    default:
      arr[0] = d;
      arr[1] = y;
      arr[2] = m;
      return 2;
    case 0x300:
      arr[0] = y;
      arr[1] = d;
      arr[2] = m;
      return 3;
  }
}

void conv_date( int *date, int m, int d, int y, char *fmt )
{
  int i, *arr[3];
  char *list, c, **p;
  
  rsrc_gaddr( 15, IDT, &p );
  list = *p + 3*conv_idt( arr, &m, &d, &y );
  if( (c = (char)idt_fmt) == 0 ) c = '/';
  for( i=0; i<3; i++ )
  {
    date[i] = *arr[i];
    *fmt++ = *fmt++ = *list++;
    *fmt++ = c;
  }
  *(fmt-1) = 0;
}

int is_num(int c)
{
  return( c >= '0' && c <= '9' );
}
void to_int( char *ptr, int *out )
{
  register int i;
  
  for( i=0; i<3; i++, ptr+=2, out++ )
  {
    if( i<2 && (!*ptr || !*(ptr+1)) )
    {
      *out = 999;
      return;
    }
    *out = is_num( *ptr ) ? (is_num( *(ptr+1) ) ? 10*(*ptr-'0') +
        *(ptr+1) - '0' : *ptr-'0') : 999;
  }
}

void set_time( int *time, int *date, char pm )
{
  unsigned int h;
  union
  {
    unsigned long l;
    struct
    {
      unsigned year:7;
      unsigned mon:4;
      unsigned day:5;
      unsigned hr:5;
      unsigned min:6;
      unsigned sec:5;
    } t;
  } t;
  
  t.l = 0;
  t.t.hr = h = ampm&&pm&&time[0]<12 ? time[0]+12 :
      (ampm&&!pm&&time[0]==12 ? 0 : time[0]);
  t.t.min = time[1];
  t.t.sec = time[2]>>1;
  t.t.year = date[2]<80 ? date[2]+100-80 : date[2]-80;
  t.t.mon = date[0];
  t.t.day = date[1];
  Settime( t.l );
  Tsetdate( ((unsigned)t.t.year<<9) | (date[0]<<5) | date[1] );
  Tsettime( ((unsigned)h<<11)|(time[1]<<5)|(time[2]>>1) );
}

#pragma warn -par
int i_option( OBJECT *o )
{
  sel_if( o, OPAMPM, ampm, 0 );
  sel_if( o, OP24, !ampm, 0 );
  sel_if( o, OPDATE, disp_day, 0 );
  sel_if( o, OPHOUR, disp_hours, 0 );
  sel_if( o, OPCOOR, disp_coords, 0 );
  sel_if( o, OPTIME, disp_time, 0 );
  return 0;
}

int x_option( OBJECT *o, int num )
{
  if( num==OPOK )
  {
    ampm = is_sel( o, OPAMPM );
    disp_day = is_sel( o, OPDATE );
    disp_hours = is_sel( o, OPHOUR );
    disp_coords = is_sel( o, OPCOOR );
    disp_time = is_sel( o, OPTIME );
    hours_redraw++;
  }
  return 1;
}

CITY *ed_city;
int ed_sort, cit_num;

double dfrac( double *d )
{
  double t;
  
  t = fabs(*d);
  return t - trunc(t);
}

void show_dec( OBJECT *o, int num, double d, int draw )
{
  sprintf( o[num].ob_spec.tedinfo->te_ptext, "%d", abs(d) );
  draw_if( o, num++, draw );
  sprintf( o[num].ob_spec.tedinfo->te_ptext, "%02d", (int)(dfrac(&d)*61.0) );
  draw_if( o, num++, draw );
  sel_if( o, num++, d>=0, draw );
  sel_if( o, num, d<0, draw );
}

void set_edpop( OBJECT *o, int draw )
{
  char **p, *q;
  int i;

  p = &o[EDAD].ob_spec.free_string;
  if( *p != (q=daypop[(i=ed_city->s.daylight)<0 ? 1 : i+2].ob_spec.free_string) )
  {
    *p = q;
    draw_if( o, EDAD, draw );
  }
}

void show_city( OBJECT *o, int draw )
{
  static char edits[] = { EDPLACE, EDLED, EDLEDM, EDOED, EDOEDM, EDGED };
  int i;
  
  for( i=0; i<sizeof(edits); i++ )
    edit_if( o, edits[i], cities );
  if( cities )
  {
    strcpy( o[EDPLACE].ob_spec.tedinfo->te_ptext, ed_city->name );
    draw_if( o, EDPLACE, draw&1 );
    enab_if( o, EDPMI, cities && cit_num, draw );
    enab_if( o, EDPPL, cit_num<cities-1, draw );
    draw &= 1;
    show_dec( o, EDLED, ed_city->lat, draw );
    show_dec( o, EDOED, ed_city->lng, draw );
    sprintf( o[EDGED].ob_spec.tedinfo->te_ptext, "%02d", abs(ed_city->zone/60) );
    sprintf( o[EDGMN].ob_spec.tedinfo->te_ptext, "%02d", abs(ed_city->zone)%60 );
    draw_if( o, EDGED, draw );
    draw_if( o, EDGMN, draw );
    sel_if( o, EDGE, ed_city->zone<0, draw );
    sel_if( o, EDGW, ed_city->zone>=0, draw );
    sel_if( o, EDAH, cit_num==here, draw );
    sel_if( o, EDAS, ed_city->s.active, draw );
    set_edpop( o, draw );
    if( o[EDBOX].ob_flags & HIDETREE )
    {
      o[EDBOX].ob_flags &= ~HIDETREE;
      draw_if( o, EDBOX, draw );
    }
  }
  else if( !(o[EDBOX].ob_flags & HIDETREE) )
  {
    o[EDBOX].ob_flags |= HIDETREE;
    draw_if( o, 0, draw );
  }
}

int get_dec( OBJECT *o, int num, double *d )
{
  int d1, d2;
  char *p;
  
  d2 = atoi(p=o[num+1].ob_spec.tedinfo->te_ptext);
  if( !*(p+1) ) d2 *= 10;	/* convert 12.3_ to 12.30 */
  if( (d1=atof(o[num].ob_spec.tedinfo->te_ptext))>180 || d1<0 ||
      d2>59 || d2<0 )
  {
    alert(BADCOOR);
    return 0;
  }
  *d = d1 + (double)d2 / 60;
  if( is_sel( o, num+3 ) ) *d = -*d;
  return 1;
}

int dbig( double *d )
{
  return (int)((*d+.005) * 120.0);
}

int dcmp( double *d1, double *d2 )
{
  return dbig(d1) != dbig(d2);
}

int get_city( OBJECT *o )
{
  char change=0, *p;
  double d1, d2;
  int i1, i2;

  if( !cities ) return -1;
  if( strcmp( p=o[EDPLACE].ob_spec.tedinfo->te_ptext, ed_city->name ) )
  {
    strcpy( ed_city->name, p );
    change = ed_sort==0;
    list_changed = 1;
  }
  if( !get_dec( o, EDLED, &d1 ) || !get_dec( o, EDOED, &d2 ) ) return -1;
  if( dcmp( &ed_city->lat, &d1 ) || dcmp( &ed_city->lng, &d2 ) )
    if( fabs(d1) >= 180 || fabs(d2) > 180 )
    {
      alert(BADCOOR);
      return -1;
    }
    else
    {
      ed_city->lat = d1;
      ed_city->lng = d2;
      if( ed_sort > 0 ) change = 1;
      list_changed = 1;
    }
  if( is_sel( o, EDAH ) && here != cit_num )
  {
    here = cit_num;
    list_changed = 1;
  }
  if( (i1=atoi(o[EDGED].ob_spec.tedinfo->te_ptext)) > 12 || i1<0 ||
      (i2=atoi(o[EDGMN].ob_spec.tedinfo->te_ptext)) > 59 || i2<0 ||
      (i1=i1 * 60 + i2) > 12*60 )
  {
    alert(BADGMT);
    return -1;
  }
  else
  {
    if( is_sel( o, EDGE ) ) i1 = -i1;
    if( ed_city->zone != i1 )
    {
      ed_city->zone = i1;
      list_changed = 1;
    }
  }
  if( (i1 = is_sel( o, EDAS )) != ed_city->s.active )
  {
    ed_city->s.active = i1;
    list_changed = 1;
  }
  if( change )
  {
    cit_num = sort_cities( ed_sort, cit_num );
    ed_city = &city[cit_num];
    show_city( o, 2 );
    return 1;
  }
  return 0;
}

int i_edit( OBJECT *o )
{
  int i;

  rsrc_gaddr( 0, DAYPOP, &daypop );
  for( i=0; i<num_daylt; i++ )
    daypop[i+2].ob_spec.free_string = daylt[i].name;
  daypop[0].ob_height = (num_daylt+1)*daypop[1].ob_height;
  daypop[0].ob_tail = i+1;
  daypop[i+1].ob_next = 0;
  if( !cities )
  {
    cit_num = -1;
    ed_city = 0L;
  }
  else ed_city = &city[cit_num];
  show_city( o, 0 );
  sel_if( o, EDSN, (ed_sort=sort)==0, 0 );
  sel_if( o, EDSNS, ed_sort==1, 0 );
  sel_if( o, EDSEW, ed_sort==2, 0 );
  return EDPLACE;
}

int add_city(void)
{
  long size;
  CITY *c;
  
  if( extra_cities )
  {
    extra_cities--;
    return 1;
  }
  if( !_app )
  {
    alert(CITROOM);
    return 0;
  }
  size = (cities+10) * sizeof(CITY);
  if( (c = malloc(size)) == 0 )
  {
    alert(NOCITY);
    return 0;
  }
  if( city )
  {
    memcpy( c, city, size - 10*sizeof(CITY) );
    free(city);
  }
  city = c;
  extra_cities += 9;
  return 1;
}

int x_edit( OBJECT *o, int num )
{
  int buf[8];

  if( cities && get_city(o) < 0 ) return 0;
  switch(num)
  {
    case EDOK:
      sort = ed_sort;
      break;
    case EDNEW:
      if( add_city() )
      {
        ed_city = &city[cit_num=cities++];
        memset( ed_city, 0, sizeof(CITY) );
        ed_city->s.active = 1;
        ed_city->s.daylight = -1;
        show_city( o, -1 );
        list_changed = 1;
      }
      return 0;
    case EDDEL:
      if( cities )
      {
        memcpy( ed_city, ed_city+1, (--cities-cit_num)*sizeof(CITY) );
        extra_cities++;
        if( cit_num > cities-1 ) cit_num = cities-1;
        if( here == cit_num ) here = -1;
        else if( here >= cit_num ) here--;
        ed_city = cit_num>=0 ? &city[cit_num] : 0L;
        show_city( o, -1 );
        list_changed = 1;
      }
      return 0;
    case EDGOTO:
      sort = ed_sort;
      ed_goto = cit_num;
      break;
  }
  if( here<0 && alert(NOHERE)==1 ) return 0;
  buf[0] = WM_REDRAW;
  buf[2] = 0;
  buf[3] = handle;
  *(GRECT *)&buf[4] = center;
  appl_write( buf[1] = apid, 16, buf );
  city_coords();
  return 1;
}

int t_edit( OBJECT *o, int num )
{
  int i, x, y;
  MENU m, out;
  
  switch(num)
  {
    case EDAD:
      m.mn_tree = daypop;
      m.mn_menu = m.mn_scroll = 0;
      m.mn_item = (i=ed_city->s.daylight)<0 ? 1 : i+2;
      objc_offset( o, EDAD, &x, &y );
      if( mn_popup( &m, x, y, &out ) && (x=out.mn_item-2) != i )
      {
        ed_city->s.daylight = x;
        set_edpop( o, 1 );
        list_changed = 1;
      }
      break;
    case EDPMI:
      if( get_city(o) < 0 ) break;	/* cit_num might change */
      if( cit_num )
      {
        cit_num--;
        ed_city--;
        show_city( o, -1 );
      }
      break;
    case EDPPL:
      if( get_city(o) < 0 ) break;	/* cit_num might change */
      if( cit_num<cities-1 )
      {
        cit_num++;
        ed_city++;
        show_city( o, -1 );
      }
      break;
    case EDSN:
    case EDSNS:
    case EDSEW:
      if( (i=num-EDSN) != ed_sort )
      {
        ed_sort=-1;		/* never sort in get_city() */
        get_city(o);		/* sort even if bad range */
        ed_sort = i;
        if( cities )
        {
          cit_num = sort_cities( i, cit_num );
          ed_city = &city[cit_num];
          show_city( o, 2 );
        }
      }
  }
  return 0;
}

int i_date( OBJECT *o )
{
  register unsigned int t;
  int hr, min, sec, year, mon, day, pm;
  
  t = Tgettime();
  hr = (t>>11) & 0x1F;
  pm = 0;
  if( ampm )
    if( !hr ) hr = 12;
    else if( hr>=12 )
    {
      pm++;
      if( hr>12 ) hr-=12;
    }
  min = (t>>5) & 0x3F;
  sec = (t&0x1F) << 1;
  t = Tgetdate();
  year = ((t>>9) & 0x7F) + 80;
  mon = (t>>5) & 0x0F;
  day = t & 0x1F;
  sprintf( o[EDTIME].ob_spec.tedinfo->te_ptext, "%02d%02d%02d", hr, min, sec );
  spf_date( o[EDDATE].ob_spec.tedinfo->te_ptext, mon, day, year, 0 );
  if( ampm )
  {
    o[EDAMPM].ob_flags &= ~HIDETREE;
    o[ED24].ob_flags |= HIDETREE;
    sel_if( o, EDAM+pm, 1, 0 );
    sel_if( o, EDAM+!pm, 0, 0 );
  }
  else
  {
    o[EDAMPM].ob_flags |= HIDETREE;
    o[ED24].ob_flags &= ~HIDETREE;
    o[ED24].ob_y = o[EDAMPM].ob_y;
  }
  return EDTIME;
}

int x_date( OBJECT *o, int num )
{
  int time[3], date[3];
  char **p, temp[100], temp2[10], temp3[10], pm;
  
  if( num != EDDOK ) return 1;
  to_int( o[EDTIME].ob_spec.tedinfo->te_ptext, time );
  if( ampm && (time[0]<=0 || time[0]>12) ) alert( BADHOUR );
  else if( !ampm && (time[0]<0 || time[0]>23) ) alert( BADHOUR2 );
  else if( time[1]<0 || time[1]>59 || time[2]<0 || time[2]>59 )
      alert( BADTIME );
  else
  {
    to_int( o[EDDATE].ob_spec.tedinfo->te_ptext, date );
    conv_date( date, date[0], date[1], date[2], temp2 );
    if( date[0]<=0 || date[0]>12 || date[1]<=0 || date[1]>31 ||
        date[2]>99 )
    {
      rsrc_gaddr( 15, BADDATE, &p );
      spf_date( temp3, 5, 2, 94, 1 );
      sprintf( temp, *p, temp2, temp3 );
      form_alert( 1, temp );
    }
    else
    {
      if( ampm ) pm = o[EDPM].ob_state & SELECTED;
      set_time( time, date, pm );
      hours_redraw++;
      return 1;
    }
  }
  return 0;
}

char *oldtext;

int i_about( OBJECT *o )
{
  ICONBLK *i;
  
  i = o[AICON].ob_spec.iconblk = icon[1].ob_spec.iconblk;
  i->ib_wtext = 0;
  oldtext = i->ib_ptext;
  i->ib_ptext = "";
  i->ib_xicon = 0;
  o[AICON].ob_type = icon[1].ob_type;
  return 1;
}

int x_about( OBJECT *o, int num )
{
  ICONBLK *i;
  
  i = icon[1].ob_spec.iconblk;
  i->ib_xicon = 20;
  i->ib_wtext = 72;
  i->ib_ptext = oldtext;
  return 1;
}
#pragma warn +par

void do_form( int num, int init( OBJECT *o ), int exit( OBJECT *o, int num ),
    int touch( OBJECT *o, int num ) )
{
  OBJECT *o;
  GRECT g;
  int edit=0, ret, this, tmp1;
  
  rsrc_gaddr( 0, num, &o );
  if( !has_Geneva )
  {
    tmp1 = this = 0;
    while( this != -1 )
      if( o[this].ob_tail != tmp1 )
      {
        o[tmp1=this].ob_state &= ~X_PREFER;
        this = o[tmp1].ob_head;
        if( this == -1 ) this = o[tmp1].ob_next;
      }
      else
      {
        tmp1 = this;
        this = o[tmp1].ob_next;
      }
  }
  if( init ) edit = (*init)(o);
  form_center( o, &g.g_x, &g.g_y, &g.g_w, &g.g_h );
  g.g_x -= 3;
  g.g_y -= 3;
  g.g_w += 6;
  g.g_h += 6;
  wind_update( BEG_UPDATE );
  wind_update( BEG_MCTRL );
  form_dial( FMD_START, 0, 0, 0, 0, g.g_x, g.g_y, g.g_w, g.g_h );
  objc_draw( o, 0, 8, g.g_x, g.g_y, g.g_w, g.g_h );
  for(;;)
  {
    num = form_do( o, o[edit].ob_flags&EDITABLE ? edit : 0 ) & 0x7fff;
    if( o[num].ob_flags & TOUCHEXIT && !(o[num].ob_state & DISABLED) )
    {
      if( touch && (*touch)( o, num ) ) break;
    }
    else if( o[num].ob_flags & EXIT )
    {
      objc_change( o, num, 0, g.g_x, g.g_y, g.g_w, g.g_h,
          o[num].ob_state & ~SELECTED, ret = exit && !(*exit)( o, num ) );
      if( !ret ) break;
    }
  }
  form_dial( FMD_FINISH, 0, 0, 0, 0, g.g_x, g.g_y, g.g_w, g.g_h );
  if( hours_redraw )
  {
    redraw_hours();
    hours_redraw = 0;
  }
  wind_name();
  wind_update( END_MCTRL );
  wind_update( END_UPDATE );
}

void action( int num )
{
  switch( num )
  {
    case POPTION:
      do_form( OPTIONS, i_option, x_option, 0L );
      break;
    case PPLACE:
      do_form( EDITCITY, i_edit, x_edit, t_edit );
      break;
    case PSET:
      do_form( EDITDT, i_date, x_date, 0L );
      break;
    case PSAVE:
      write_dat();
      break;
    case PABOUT:
      do_form( ABOUT, i_about, x_about, x_about );
      break;
  }
}

void do_menu( int shift, int key )
{
  MENU m, out;
  int x, y, d;
  
  if( key )
  {
    if( shift==4 ) switch(key&0xFF00)
    {
      case 0x1800:	/* ^O */
      	action(POPTION);
      	break;
      case 0x1900:	/* ^P */
      	action(PPLACE);
      	break;
      case 0x2000:	/* ^D */
      	action(PSET);
      	break;
      case 0x1F00:	/* ^S */
      	action(PSAVE);
      	break;
      case 0x1E00:	/* ^A */
      	action(PABOUT);
      	break;
      case 0x1000:	/* ^Q */
        close_wind();
        break;
    }
  }
  else
  {
    rsrc_gaddr( 0, POP, &m.mn_tree );
    m.mn_menu = m.mn_item = m.mn_scroll = 0;
    graf_mkstate( &x, &y, &d, &d );
    if( mn_popup( &m, x-(m.mn_tree[0].ob_width>>1), y+4, &out ) )
        action(out.mn_item);
  }
}
