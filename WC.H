#ifndef EXTERN
  #define EXTERN extern
#endif

#define DRAW_MAP    1
#define DRAW_HOURS  2
#define DRAW_CITIES 4
#define DRAW_SUN    8

EXTERN int cities, extra_cities, here, handle, apid, ed_goto, vdi_hand,
    num_daylt, vplanes, miny, subtr, divis, height;
EXTERN unsigned long idt_fmt;
EXTERN char ampm, disp_coords, sort, disp_day, list_changed, disp_hours,
    disp_time, disp_sun, loadpath[120], has_Geneva, popup_ok, draw_mode;
EXTERN GRECT wsize, center, max;
EXTERN OBJECT *icon;

typedef struct
{
  int zone;
  double lat, lng;
  char name[28];
  struct
  {
    unsigned active:1;
    signed daylight:7;
  } s;
  int x, y, old_ind;
} CITY;

typedef struct
{
  char hour, which, month, day;
} DAY;

typedef struct
{
  DAY start;
  DAY end;
  char add, name[17];
} DAYLT;

EXTERN CITY *city, *cur_city;
EXTERN DAYLT *daylt;

void action( int num );
int alert( int num );
void city_coords(void);
void fix_coord( double *d );
int read_dat(void);
void redraw_wind( GRECT *g, int start );
void do_menu( int shift, int key );
void write_dat(void);
int sort_cities( int sort, int here );
void draw_hours( int vdi_hand );
void redraw_hours(void);
void close_wind(void);
void wind_name(void);
char *tack_fname( char *name );
FILE *_fopen( char *name, char *mode );
void graf_blit( GRECT *r1, GRECT *r2 );
int mn_popup( MENU *in, int x, int y, MENU *out );
int intersect( GRECT *r1, GRECT *res );
void set_clip( GRECT *r );
int sunclock( int always );
void draw_sun(void);
int init_sun( int ymax );
