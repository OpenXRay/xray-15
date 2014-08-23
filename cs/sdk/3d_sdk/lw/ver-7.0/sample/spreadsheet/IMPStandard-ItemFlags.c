/*
 * IMPStandardBanks-ItemFlags.c
 */

#include "IMPStandard.h"

#include <stdio.h>

LWInterfaceInfo *ui;

/* Some Icon Colors */
const int ItemColors[][3] = {
  {  0,   0,   0},    /* Black       */
  {  0,  48, 128},    /* Dark Blue   */
  {  0,  96,   0},    /* Dark Green  */
  { 32,  96, 112},    /* Dark Cyan   */
  {112,   0,   0},    /* Dark Red    */
  {112,  32, 112},    /* Purple      */
  {112,  80,   0},    /* Brown       */
  {176, 176, 176},    /* Grey        */
  { 32, 160, 240},    /* Blue        */
  { 32, 224,  32},    /* Green       */
  { 96, 224, 240},    /* Cyan        */
  {240,  32,  32},    /* Red         */
  {240,  96, 240},    /* Magenta     */
  {240, 192,  32},    /* Orange      */
  {240, 240, 240}     /* White       */
};

/*
 * DrawLWIcon:  Draws some LW-ish icons for us
 */
enum IconType {
  ICON_DOT,
  ICON_BOUNDING_BOX,
  ICON_POINTS_ONLY,
  ICON_WIREFRAME,
  ICON_FRONTFACE,
  ICON_SOLID,
  ICON_TEXTURED,

  ICON_CHECK,
  ICON_LOCK,
  ICON_VISIBILITY_EYE };

int DrawLWIcon( LWRasterID raster, LWRasterFuncs *df,             
                int x, int y, enum IconType type,
                int prime_color, int second_color );

int EmptyBox( LWRasterID raster, LWRasterFuncs *df,
              int x, int y, int width, int height,
              int x_thickness, int y_thickness,
              int shine_color, int shadow_color );

/*
 *  BeginProcess()
 */
void ItemFlags_BeginProcess( long bank_id ) {
  ui = (LWInterfaceInfo *)global( LWINTERFACEINFO_GLOBAL, GFUSE_ACQUIRE );
}

/*
 *  EndProcess()
 */
void ItemFlags_EndProcess( long bank_id ) {
  global( LWINTERFACEINFO_GLOBAL, GFUSE_RELEASE );
}

/*
 *  Visibility
 */
void * Visibility_Query( int column, int row, LWItemID id, LWTime time ) {
  int vis  = ui->itemVis( id );
  int type = item_info->type( id );

  if( type == LWI_OBJECT ) {
    switch( vis ) {
      default:
      case LWOVIS_HIDDEN:        value_int = 0;    break;
      case LWOVIS_BOUNDINGBOX:   value_int = 1;    break;
      case LWOVIS_VERTICES:      value_int = 2;    break;
      case LWOVIS_WIREFRAME:     value_int = 3;    break;
      case LWOVIS_FFWIREFRAME:   value_int = 4;    break;
      case LWOVIS_SHADED:        value_int = 5;    break;
      case LWOVIS_TEXTURED:      value_int = 6;    break;
    }
  } else {
    value_int = (vis == LWIVIS_VISIBLE) ? 7 : 0;
  }

  return &value_int;
}

void * Visibility_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  int type    = item_info->type( id );
  int vis     = *(int *)value;            // The mode we've been asked to set to
  int new_vis;                            // Passed to the command to change visibility

  if( vis < 0 )
    vis = 0;
  if( type == LWI_OBJECT ) {
    if( vis > 7 )
      vis = 7;
  } else {
    if( vis > 1 )
      vis = 1;
  }

  new_vis = vis;
  value_int = vis;                        // value_int will contain the new list index

  if( type == LWI_OBJECT ) {
    switch( vis ) {
      case 1:  new_vis = LWOVIS_BOUNDINGBOX;                   break;
      case 2:  new_vis = LWOVIS_VERTICES;                      break;
      case 3:  new_vis = LWOVIS_WIREFRAME;                     break;
      case 4:  new_vis = LWOVIS_FFWIREFRAME;                   break;
      case 5:  new_vis = LWOVIS_SHADED;                        break;

      default:
        if( vis <= 0 ) {
          new_vis = LWOVIS_HIDDEN;
          value_int = 0;
        } else {
          new_vis = LWOVIS_TEXTURED;
          value_int = 6;
        }
    }
  } else {
    new_vis   = (vis == 0) ? LWIVIS_HIDDEN : LWIVIS_VISIBLE;
    value_int = (vis == 0) ? 0 : 7;
  }

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "ItemVisibility %d", new_vis );      /* TODO:  Fix this when the command is supported */
    command( buffer );
  }

  return &value_int;
}

int Visibility_ListCount( int column, int row, LWItemID id ) {
  int count;
  if( (row == -1) || (id == NULL) ) {
    count = 8;
  } else {
    int type = item_info->type( id );
    count = ((type == LWI_OBJECT) ? 7 : 2);
  }

  return count;
}

const char *item_vis_list[] = {
  "Hidden",
  "Bounding Box",
  "Vertices",
  "Wireframe",
  "Front Face Wireframe",
  "Shaded Solid",
  "Textured Solid",
  "Visible" };

const char * Visibility_ListName( int column, int row, LWItemID id, int index ) {
  int type = (id == LWITEM_NULL) ? LWI_OBJECT : item_info->type( id );
  int count;

  if( (row == -1) || (id == NULL) )
    count = 7;
  else
    count = ((type == LWI_OBJECT) ? 6 : 1);

  if( index < 0 )
    return "";

  if( index > count )
    return "";

  if( type == LWI_OBJECT )
    return item_vis_list[ index ];

  return item_vis_list[ index == 0 ? 0 : 7 ];
}

/*
 *  Visibility_Draw.
 *   Draws both the cells and the column title
 */
const int icon_width = 14;
int Visibility_Draw( int column, void *value, LWRasterID raster, int x, int y, int w, int h, IMPGhostModes ghosting ) {
  int draw_w = w < 30 ? w : 30;
  int draw_x = x + 1 + (draw_w/2) - (icon_width/2);
  int draw_y = y + 2;
  int prime_color  = (ghosting == IMPGHOST_DISABLED) ? RGB_( 110, 110, 110 ) : RGB_( 80, 80, 80 );
  int second_color = RGB_( 128, 128, 128 );

  if( raster == NULL )
    return icon_width + 4;

  if( value == NULL ) {
    DrawLWIcon( raster, raster_funcs, draw_x, draw_y, ICON_VISIBILITY_EYE, RGB_( 0, 0, 0 ), second_color );

  } else  {
    int index = *(int *)value;
    if( index != 0 ) {
      int icon;
      switch( index ) {
        case 1:  icon = ICON_BOUNDING_BOX;  break;
        case 2:  icon = ICON_POINTS_ONLY;   break;
        case 3:  icon = ICON_WIREFRAME;     break;
        case 4:  icon = ICON_FRONTFACE;     break;
        case 5:  icon = ICON_SOLID;         break;
        case 6:  icon = ICON_TEXTURED;      break;
        default: icon = ICON_DOT;
      }

      DrawLWIcon( raster, raster_funcs, draw_x, draw_y, icon, prime_color, second_color );
    }

    if( (index >= 0) && (index < 7) ) {
      int text_color  = (ghosting == IMPGHOST_DISABLED) ? RGB_( 80, 80, 80 ) : RGB_( 0, 0, 0 );
      raster_funcs->drawText( raster, (char *)item_vis_list[ index ], text_color, draw_x + icon_width + 10, draw_y );
    }
  }

  return icon_width + 4;
}

IMPColumn col_Visibility = {
  "Item Visibility",                         /* Column title            */
  25,                                        /* default width in pixels */
  IMPCOLTYPE_LIST,                           /* column type             */
  "Item Visibility, Scene Editor",           /* Comment                 */
  NULL,                                      /* No envelope function    */
  RowZeroOnly_Ghost,                         /* Ghost function          */
  Visibility_Query,                          /* Query function          */
  Visibility_Evaluate,                       /* Evaluate function       */
  NULL,                                      /* No compare function     */
  Visibility_ListCount,                      /* List Count function     */
  Visibility_ListName,                       /* List Name function      */
  NULL,                                      /* No test item function   */
  Visibility_Draw,                           /* Custom draw function    */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Item Active
 */
void * ItemActive_Query( int column, int row, LWItemID id, LWTime time ) {
  int flags = item_info->flags( id );
  value_int = flags & LWITEMF_ACTIVE;

  return &value_int;
}

void * ItemActive_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  int flags = item_info->flags( id ) & LWITEMF_ACTIVE;
  value_int = *(int *)value;

  if( apply ) {
    if( ((value_int == 0) && (flags != 0)) ||
        ((value_int != 0) && (flags == 0)) ) {

      char buffer[ 100 ];
      sprintf( buffer, "SelectItem %x", id );
      command( buffer );

      command( "ItemActive" );
    }
  }

  return &value_int;
}

/*
 *  ItemActive_Draw.
 *   Draws both the cells and the column title
 */
int ItemActive_Draw( int column, void *value, LWRasterID raster, int x, int y, int w, int h, IMPGhostModes ghosting ) {
  int draw_x = x + (w/2) - (icon_width/2);
  int draw_y = y + 2;
  int prime_color  = (ghosting == IMPGHOST_DISABLED) ? RGB_( 110, 110, 110 ) : RGB_( 80, 80, 80 );
  int second_color = RGB_( 128, 128, 128 );

  if( raster == NULL )
    return icon_width + 4;

  if( value == NULL ) {
    DrawLWIcon( raster, raster_funcs, draw_x, draw_y, ICON_CHECK, RGB_(0, 0, 0), second_color );

  } else {
    int state = *(int *)value;
    if( state != 0 )
      DrawLWIcon( raster, raster_funcs, draw_x, draw_y, ICON_CHECK, prime_color, second_color );
  }

  return icon_width + 4;
}

IMPColumn col_ItemActive = {
  "Item Active",                             /* Column title            */
  25,                                        /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Item Active, Scene Editor",               /* Comment                 */
  NULL,                                      /* No envelope function    */
  RowZeroOnly_Ghost,                         /* Ghost function          */
  ItemActive_Query,                          /* Query function          */
  ItemActive_Evaluate,                       /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  ItemActive_Draw,                           /* Custom draw function    */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Locked
 */
void * Locked_Query( int column, int row, LWItemID id, LWTime time ) {
  int flags = ui->itemFlags( id );
  value_int = flags & LWITEMF_LOCKED;

  return &value_int;
}

void * Locked_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  int flags = ui->itemFlags( id ) & LWITEMF_LOCKED;
  value_int = *(int *)value;

  if( apply ) {
    if( ((value_int == 0) && (flags != 0)) ||
        ((value_int != 0) && (flags == 0)) ) {
      char buffer[ 100 ];
      sprintf( buffer, "SelectItem %x", id );
      command( buffer );

      command( "ItemLock" );
    }
  }

  return &value_int;
}

/*
 *  Locked_Draw.
 *   Draws both the cells and the column title
 */
int Locked_Draw( int column, void *value, LWRasterID raster, int x, int y, int w, int h, IMPGhostModes ghosting ) {
  int draw_x = x + (w/2) - (icon_width/2);
  int draw_y = y + 2;
  int prime_color  = (ghosting == IMPGHOST_DISABLED) ? RGB_( 110, 110, 110 ) : RGB_( 80, 80, 80 );
  int second_color = RGB_( 128, 128, 128 );

  if( raster == NULL )
    return icon_width + 4;

  if( value == NULL ) {
    DrawLWIcon( raster, raster_funcs, draw_x, draw_y, ICON_LOCK, RGB_( 0, 0, 0 ), second_color );

  } else {
    int state = *(int *)value;
    if( state != 0 )
      DrawLWIcon( raster, raster_funcs, draw_x, draw_y, ICON_LOCK, prime_color, second_color );
  }

  return icon_width + 4;
}

IMPColumn col_Locked = {
  "Item Locking",                            /* Column title            */
  25,                                        /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Item Locking, Scene Editor",              /* Comment                 */
  NULL,                                      /* No envelope function    */
  RowZeroOnly_Ghost,                         /* Ghost function          */
  Locked_Query,                              /* Query function          */
  Locked_Evaluate,                           /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  Locked_Draw,                               /* Custom draw function    */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Item Color
 */
void * ItemColor_Query( int column, int row, LWItemID id, LWTime time ) {
/*
  int color = 0;
  switch( item_info->type( id ) ) {
    case LWI_OBJECT:  color = 10;  break;
    case LWI_BONE:    color =  1;  break;
    case LWI_LIGHT:   color = 12;  break;
    case LWI_CAMERA:  color =  9;  break;
  }

  value_int = color;
*/

  value_int = ui->itemColor( id );
  return &value_int;
}

void * ItemColor_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  int color = *(int *)value;

  if( color < 0 )
    color = 0;

  if( color > 14 )
    color = 14;

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "ItemColor %d", color );      /* TODO:  Fix this when the command is supported */
    command( buffer );
  }

  value_int = color;
  return &value_int;
}

int ItemColor_ListCount( int column, int row, LWItemID id ) {
  return 15;
}

const char *item_color_list[] = {
  "Black",
  "Dark Blue",
  "Dark Green",
  "Dark Cyan",
  "Dark Red",
  "Purple",
  "Brown",
  "Grey",
  "Blue",
  "Green",
  "Cyan",
  "Red",
  "Magenta",
  "Orange",
  "White"
};

const char * ItemColor_ListName( int column, int row, LWItemID id, int index ) {
  if( index < 0 )
    return "";

  if( index > 14 )
    return "";

  return item_color_list[ index ];
}

/*
 *  ItemColor_Draw.  We don't draw the title, so return 0 if value is NULL
 */
int ItemColor_Draw( int column, void *value, LWRasterID raster, int x, int y, int w, int h, IMPGhostModes ghosting ) {
  // Figure out the color square's size
  int draw_x = x +  6;
  int draw_y = y +  3;
  int draw_w = w - 12;
  int draw_h = h -  6;
  int color;
  int text_color = (ghosting == IMPGHOST_DISABLED) ? RGB_( 80, 80, 80 ) : RGB_( 0, 0, 0 );

  if( value == NULL )
    return 0;

  if( draw_w > 40 )
    draw_w = 40;

  if( raster == NULL )
    return 40;

  color = *(int *)value;

  // Draw the square
  EmptyBox( raster, raster_funcs, draw_x, draw_y, draw_w, draw_h, 1, 1,
            RGB_( 80, 80, 80 ), RGB_( 200, 200, 200 ) );

  raster_funcs->drawRGBBox( raster, ItemColors[ color ][ 0 ], ItemColors[ color ][ 1 ], ItemColors[ color ][ 2 ],
                            draw_x+1, draw_y+1, draw_w-2, draw_h-2 );

  // Draw the text
  if( (color >= 0) && (color < 15) )
    raster_funcs->drawText( raster, (char *)item_color_list[ color ], text_color, draw_x + draw_w + 10, draw_y );

  return 40;
}

IMPColumn col_ItemColor = {
  "Item Color",                              /* title                   */
  40,                                        /* default width in pixels */
  IMPCOLTYPE_LIST,                           /* column type             */
  "Item Color, Scene Editor",                /* Comment                 */
  NULL,                                      /* No envelope function    */
  RowZeroOnly_Ghost,                         /* Ghost function          */
  ItemColor_Query,                           /* Query function          */
  ItemColor_Evaluate,                        /* Evaluate function       */
  NULL,                                      /* No compare function     */
  ItemColor_ListCount,                       /* List Count function     */
  ItemColor_ListName,                        /* List Name function      */
  NULL,                                      /* No test item function   */
  ItemColor_Draw,                            /* Custom draw function    */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  The Bank
 */
IMPColumn *col_itemFlags[] = {
  &col_ItemActive,
  &col_Visibility,
  &col_Locked,
  &col_ItemColor,
  NULL };

IMPBank bank_itemFlags = {
  MakeBankID( '_', 'V', 'I', 'S' ),          /* id:  Standard (_) item Visibility and flags           */
  "Item Flags",                              /* Bank Title                                            */
  IMPBASE_ITEM,                              /* Item base type                                        */
  col_itemFlags,                             /* Columns in bank                                       */
  NULL,                                      /* Num Rows callbanks; Always one row, so set it to NULL */
  ItemFlags_BeginProcess,                    /* No begin process function                             */
  ItemFlags_EndProcess,                      /* No end process function                               */
};



/*
 * EmptyBox():
 *  Draws an empty box
 */
int EmptyBox( LWRasterID raster, LWRasterFuncs *df,
               int x, int y, int width, int height,
               int x_thickness, int y_thickness,
               int shine_color, int shadow_color ) {
  df->drawBox( raster, shine_color,  x,                     y,                      width,       x_thickness );
  df->drawBox( raster, shadow_color, x+width-y_thickness,   y,                      y_thickness, height      );
  df->drawBox( raster, shine_color,  x,                     y,                      y_thickness, height      );
  df->drawBox( raster, shadow_color, x,                     y+height-x_thickness,   width,       x_thickness );

  return 1;
}

/*
 * DrawLWIcon():
 *  Draws a Lightwave-like icon into the interface.
 */
int DrawLWIcon( LWRasterID raster, LWRasterFuncs *df,
                int x, int y, enum IconType type,
                int prime_color, int second_color ) {

  switch( type ) {
    case ICON_DOT:
      df->drawBox( raster, prime_color, x+5, y+4, 3, 1 );
      df->drawBox( raster, prime_color, x+4, y+5, 5, 3 );
      df->drawBox( raster, prime_color, x+5, y+8, 3, 1 );
      break;

    case ICON_BOUNDING_BOX:
      EmptyBox( raster, df, x+2, y+2, 9, 9, 1, 1,
                prime_color,  prime_color );
      break;

    case ICON_FRONTFACE:
      EmptyBox( raster, df, x+2, y+2, 9, 9, 1, 1,
                prime_color,  prime_color );
      df->drawLine( raster, prime_color, x+6, y+3, x+ 6, y+9 );
      df->drawLine( raster, prime_color, x+3, y+6, x+10, y+6 );
      break;

    case ICON_POINTS_ONLY:
      df->drawPixel( raster, prime_color, x+ 2, y+2 );
      df->drawPixel( raster, prime_color, x+ 6, y+2 );
      df->drawPixel( raster, prime_color, x+10, y+2 );

      df->drawPixel( raster, prime_color, x+ 4, y+4 );
      df->drawPixel( raster, prime_color, x+ 8, y+4 );

      df->drawPixel( raster, prime_color, x+ 2, y+6 );
      df->drawPixel( raster, prime_color, x+ 6, y+6 );
      df->drawPixel( raster, prime_color, x+10, y+6 );

      df->drawPixel( raster, prime_color, x+ 4, y+8 );
      df->drawPixel( raster, prime_color, x+ 8, y+8 );

      df->drawPixel( raster, prime_color, x+ 2, y+10 );
      df->drawPixel( raster, prime_color, x+ 6, y+10 );
      df->drawPixel( raster, prime_color, x+10, y+10 );
      break;

    case ICON_SOLID:
      df->drawBox( raster, prime_color, x+ 2, y+2, 9, 9 );
      break;

    case ICON_TEXTURED:
      df->drawBox( raster, prime_color,  x+ 2, y+2, 9, 9 );
      df->drawBox( raster, second_color, x+ 4, y+4, 5, 1 );
      df->drawBox( raster, second_color, x+ 6, y+4, 1, 5 );
      break;

    case ICON_WIREFRAME:
      EmptyBox( raster, df, x+2, y+2, 9, 9, 1, 1,
                prime_color,  prime_color );
      df->drawLine( raster, prime_color, x+6, y+3, x+6, y+9 );
      df->drawLine( raster, prime_color, x+3, y+6, x+9, y+6 );

      df->drawPixel( raster, prime_color, x+ 3, y+ 3 );
      df->drawPixel( raster, prime_color, x+ 4, y+ 4 );
      df->drawPixel( raster, prime_color, x+ 5, y+ 5 );
      df->drawPixel( raster, prime_color, x+ 7, y+ 5 );
      df->drawPixel( raster, prime_color, x+ 8, y+ 4 );
      df->drawPixel( raster, prime_color, x+ 9, y+ 3 );

      df->drawPixel( raster, prime_color, x+ 5, y+ 7 );
      df->drawPixel( raster, prime_color, x+ 4, y+ 8 );
      df->drawPixel( raster, prime_color, x+ 3, y+ 9 );
      df->drawPixel( raster, prime_color, x+ 9, y+ 9 );
      df->drawPixel( raster, prime_color, x+ 8, y+ 8 );
      df->drawPixel( raster, prime_color, x+ 7, y+ 7 );
      break;

    case ICON_CHECK:
      df->drawPixel( raster, prime_color, x+11, y+ 3 );
      df->drawBox(   raster, prime_color, x+10, y+ 4, 2, 1 );
      df->drawBox(   raster, prime_color, x+ 9, y+ 5, 2, 1 );
      df->drawBox(   raster, prime_color, x+ 2, y+ 6, 2, 1 );
      df->drawBox(   raster, prime_color, x+ 8, y+ 6, 2, 1 );
      df->drawBox(   raster, prime_color, x+ 7, y+ 7, 2, 1 );
      df->drawBox(   raster, prime_color, x+ 2, y+ 7, 3, 1 );
      df->drawBox(   raster, prime_color, x+ 3, y+ 8, 5, 1 );
      df->drawBox(   raster, prime_color, x+ 4, y+ 9, 3, 1 );
      df->drawPixel( raster, prime_color, x+ 5, y+10 );
      break;

    case ICON_LOCK:
      df->drawBox( raster, prime_color, x+5, y+2, 3, 1 );
      df->drawBox( raster, prime_color, x+4, y+3, 1, 3 );
      df->drawBox( raster, prime_color, x+8, y+3, 1, 3 );
      df->drawBox( raster, prime_color, x+3, y+6, 7, 5 );
      break;

    case ICON_VISIBILITY_EYE:
      df->drawBox(   raster, prime_color, x+ 1, y+4, 2, 1 );
      df->drawBox(   raster, prime_color, x+ 3, y+3, 6, 1 );
      df->drawBox(   raster, prime_color, x+ 9, y+4, 2, 1 );
      df->drawPixel( raster, prime_color, x+11, y+5 );

      df->drawPixel( raster, prime_color, x+ 1, y+7 );
      df->drawBox(   raster, prime_color, x+ 2, y+6, 2, 1 );
      df->drawBox(   raster, prime_color, x+ 4, y+5, 1, 4 );
      df->drawBox(   raster, prime_color, x+ 5, y+5, 3, 1 );
      df->drawBox(   raster, prime_color, x+ 6, y+6, 4, 1 );
      df->drawBox(   raster, prime_color, x+ 6, y+7, 2, 1 );
      df->drawBox(   raster, prime_color, x+ 5, y+8, 3, 1 );
      df->drawBox(   raster, prime_color, x+ 5, y+9, 2, 1 );

      df->drawBox(   raster, prime_color, x+ 9, y+7, 2, 1 );
      df->drawPixel( raster, prime_color, x+10, y+8 );
      df->drawPixel( raster, prime_color, x+ 9, y+9 );
      df->drawBox(   raster, prime_color, x+ 3, y+10, 6, 1 );
      df->drawPixel( raster, prime_color, x+ 2, y+9 );
      break;
  }

  return 1;
}
