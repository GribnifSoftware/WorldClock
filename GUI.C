#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "aes.h"
#include "xwind.h"
#include "vdi.h"
#include "tos.h"
#include "worldclk.h"
#include "wc.h"

MFDB fdb0, fdb2;

void mblit( int flag, GRECT *r2 )
{
  int px[8];
  GRECT r = *r2;

  intersect( &max, &r );
  fdb2.fd_h = r.g_h;
  fdb2.fd_wdwidth = (fdb2.fd_w = r.g_w+16) >> 4;
  fdb2.fd_nplanes = vplanes;
  fdb2.fd_r1 = fdb2.fd_r2 = fdb2.fd_r3 = 0;
  wind_get( 0, WF_SCREEN, (int *)&fdb2.fd_addr, (int *)&fdb2.fd_addr+1, px, px );
  set_clip( &max );
  px[2] = (px[0] = !flag ? r.g_x : 0) + r.g_w - 1;
  px[3] = (px[1] = !flag ? r.g_y : 0) + r.g_h - 1;
  px[6] = (px[4] = !flag ? 0 : r.g_x) + r.g_w - 1;
  px[7] = (px[5] = !flag ? 0 : r.g_y) + r.g_h - 1;
  graf_mouse( M_OFF, 0L );
  vro_cpyfm( vdi_hand, 3, px, !flag ? &fdb0 : &fdb2, !flag ? &fdb2 : &fdb0 );
  graf_mouse( M_ON, 0L );
}

void graf_blit( GRECT *r1, GRECT *r2 )
{
  if( has_Geneva ) x_graf_blit( r1, r2 );
  else if( !r1 ) mblit( 1, r2 );
  else if( !r2 ) mblit( 0, r1 );
}

typedef struct { int x, y, w, h; } Rect;

OBJECT *u_object( OBJECT *tree, int obj )
{
  return tree+=obj;
}

int entry;

void draw_ent( OBJECT *tree, int state )
{
  objc_change( tree, entry, 0, max.g_x, max.g_y, max.g_w, max.g_h,
      (u_object(tree,entry)->ob_state&~SELECTED)|state, 1 );
}

void entry_off( OBJECT *tree )
{
  if( entry )
  {
    draw_ent( tree, 0 );
    entry=0;
  }
}

void new_entry( OBJECT *tree, int x )
{
  if( x!=entry )
  {
    entry_off(tree);
    if( !(u_object(tree,x)->ob_state&DISABLED) )
    {
      entry = x;
      draw_ent( tree, SELECTED );
    }
  }
}

int _mn_popup( MENU *mnu, int xpos, int ypos, MENU *mdata )
{
  Rect r, r2;
  OBJECT *tree;
  int xy[2], pxy[2], root, ret=0, i, state,
      mouse_x, mouse_y, mouse_b, mouse_k;

  wind_update( BEG_UPDATE );
  wind_update( BEG_MCTRL );
  tree = mnu->mn_tree;
  root = mnu->mn_menu;
  r.h = 0;
  for( i=u_object(tree,root)->ob_head; i!=root; i=u_object(tree,i)->ob_next )
    if( (xy[0] = u_object(tree,i)->ob_y + u_object(tree,i)->ob_height) > r.h ) r.h = xy[0];
  *(long *)&u_object(tree,root)->ob_x = 0L;
  r.w = u_object(tree,root)->ob_width;
  objc_offset( tree, root, pxy, pxy+1 );
  objc_offset( tree, mnu->mn_item, xy, xy+1 );
  if( xpos<4 ) xpos = 4;
  else if( xpos+r.w > max.g_w-5 ) xpos = max.g_w-5-r.w;
  u_object(tree,root)->ob_x = r.x = xpos;
  if( (i = ypos-xy[1])+pxy[1] < max.g_y ) i = max.g_y-pxy[1];
  else if( i+r.h > max.g_y+max.g_h-5 ) i = max.g_y+max.g_h-5-r.h;
  u_object(tree,root)->ob_y = i;
  r.y = i + pxy[1];
  r2=r;
  r2.x-=4; r2.y-=4;
  r2.w+=8; r2.h+=8;
  mblit( 0, (GRECT *)&r2 );
  objc_draw( tree, root, 8, max.g_x, max.g_y, max.g_w, max.g_h );
  graf_mkstate( &mouse_x, &mouse_y, &mouse_b, &mouse_k );
  state = mouse_b&1;
  for(;;)
  {
    if( (mouse_b&1) == state )
      if( (i=objc_find( tree, root, 8, mouse_x, mouse_y )) > 0 )
          new_entry( tree, i );
      else entry_off(tree);
    else
    {
      if( entry ) u_object(tree,entry)->ob_state &= ~SELECTED;
      while( mouse_b&1 )
          graf_mkstate( &mouse_x, &mouse_y, &mouse_b, &mouse_k );
      break;
    }
    graf_mkstate( &mouse_x, &mouse_y, &mouse_b, &mouse_k );
  }
  mblit( 1, (GRECT *)&r2 );
  if( entry && !(u_object(tree,entry)->ob_state&DISABLED) )
  {
    memcpy( mdata, mnu, sizeof(MENU) );
    mdata->mn_item = entry;
    mdata->mn_keystate = mouse_k;
    ret = 1;
  }
  entry = 0;
  wind_update( END_MCTRL );
  wind_update( END_UPDATE );
  return ret;
}

int mn_popup( MENU *in, int x, int y, MENU *out )
{
  if( popup_ok ) return menu_popup( in, x, y, out );
  return _mn_popup( in, x, y, out );
}
