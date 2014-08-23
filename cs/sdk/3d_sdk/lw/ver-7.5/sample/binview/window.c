/*
======================================================================
window.c

Interface and event handling.  Part of the LightWave SDK BinView
sample plug-in.

Ernie Wright  5 Dec 99
====================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <lwserver.h>
#include <lwhost.h>
#include <lwpanel.h>
#include "binview.h"


static int linesize   = ROWBYTES;         // bytes per line
static int pagesize   = ROWBYTES * NROWS; // bytes per page
static int datatype   = 0;                // byte
static int byteorder  = 0;                // Intel
static int unsign     = 0;                // signed
static int searchtype = 0;                // text, text/hex, current

static int natorder;                      // the host's byte order

static char buf[ ROWBYTES * NROWS ];      // file buffer
static char filename[ 256 ];              // name of the file
static FILE *fp = NULL;                   // file pointer
static long filesize = 0;                 // size of the file
static long pos = 0;                      // current position in the file
static int inscroll = 0;                  // in scrollbar handler code

static LWFileReqFunc *filereq;            // file requester
static LWMessageFuncs *msgf;              // message functions
static LWRasterFuncs *rasf;               // raster functions
static LWRasterID cras[ 3 ];              // monospace font rasters
static LWPanelFuncs *panf;                // panel functions
static LWPanelID panel;                   // panel
static LWControl *ctl[ 20 ];              // panel gadgets
static LWPanControlDesc desc;             // required by macros in lwpanel.h
static LWValue
   ival = { LWT_INTEGER },
   sval = { LWT_STRING };


/*
======================================================================
outtext()

Write a string to the display.  Translates row, column position on the
virtual display into pixel positions on a panel, and draws the string
one character at a time.
====================================================================== */

void outtext( int row, int col, char *a )
{
   int x, y, i, len, c;

   if ( col <= 10 )
      c = 0;
   else if ( col <= 60 )
      c = 1;
   else
      c = 2;

   x = DX + CON_HOTX( ctl[ 0 ] ) + TXWIDTH * ( col - 1 );
   y = DY + CON_HOTY( ctl[ 0 ] ) + TXHEIGHT * ( row - 1 );
   len = strlen( a );

   for ( i = 0; i < len; i++, x += TXWIDTH )
      draw_char( a[ i ], x, y, cras[ c ], rasf, panel );
}


/*
======================================================================
clrtext()

Fill a rectangle on the virtual display with the background color.
====================================================================== */

void clrtext( int r1, int c1, int r2, int c2 )
{
   int x, y, w, h;

   x = DX + CON_HOTX( ctl[ 0 ] ) + TXWIDTH * ( c1 - 1 );
   y = DY + CON_HOTY( ctl[ 0 ] ) + TXHEIGHT * ( r1 - 1 );
   w = TXWIDTH * ( c2 - c1 + 1 );
   h = TXHEIGHT * ( r2 - r1 + 1 );

   panf->drawFuncs->drawBox( panel, COLOR_BG, x, y, w, h );
}


/*
======================================================================
drawcb_text()

Callback that updates the virtual display.  This fills the buffer with
a chunk of the file at the current position and calls the high-level
functions that write the virtual display.
====================================================================== */

static void drawcb_text( LWControl *ectl, void *edata, DrMode mode )
{
   static int size[] = { 1, 2, 4, 4, 8, 2, 4 };
   char a[ 10 ];
   int rlen;

   if ( !fp ) return;
   fseek( fp, pos, SEEK_SET );
   rlen = fread( buf, 1, pagesize, fp );

   SET_STR( ctl[ 9 ], itoa( pos % size[ datatype ], a, 10 ), sizeof( a ));

   show_byt( pos, rlen, linesize );
   show_prt( buf, rlen, linesize );
   show_num( buf, rlen, datatype, unsign, byteorder != natorder, linesize );
}


/*
======================================================================
drawcb_icon()

Callback that draws the BinView icon image.
====================================================================== */

static void drawcb_icon( LWControl *ectl, void *edata, DrMode mode )
{
   draw_icon( CON_HOTX( ectl ), CON_HOTY( ectl ), rasf, panel );
}


/*
======================================================================
set_scroll()

Update the position of the scrollbar.  Does nothing if called in the
context of the scrollbar callback.
====================================================================== */

static void set_scroll( void )
{
   int i;

   if ( !inscroll ) {
      i = ( int )( 1000.0 * ( double ) pos / ( double ) filesize );
      SET_INT( ctl[ 10 ], i );
   }
}


/*
======================================================================
change_pos()

Called to change the file position and update the display.
====================================================================== */

static void change_pos( int b, int n )
{
   if ( !fp ) return;

   pos = b + n;
   if ( pos > filesize - 1 ) pos = filesize - 1;
   if ( pos < 0 ) pos = 0;
   set_scroll();
   drawcb_text( ctl[ 0 ], NULL, 0 );
}


/*
======================================================================
handle_file()

Panel callback, called when the user presses Enter in the filename
edit field.  Also called by handle_new().

Opens a new file, sets the values of a couple of text controls to show
information about the file, and tells LWPanels to update the buffer
display.
====================================================================== */

static void handle_file( LWControl *ectl, void *edata )
{
   char a[ 24 ];
   struct stat s;
   time_t timestamp;

   GET_STR( ectl, filename, sizeof( filename ));
   if ( filename[ 0 ] == 0 ) return;

   if ( fp ) fclose( fp );
   if ( !( fp = fopen( filename, "rb" ))) {
      msgf->error( "Couldn't open", filename );
      return;
   }

   stat( filename, &s );
   filesize = s.st_size;
   timestamp = s.st_mtime;

   pos = 0;
   set_scroll();

   SET_STR( ctl[ 11 ], itoa( filesize, a, 10 ), sizeof( a ));
   strftime( a, sizeof( a ), "%d %b %y %X", localtime( &timestamp ));
   SET_STR( ctl[ 2 ], a, strlen( a ));

   ctl[ 0 ]->draw( ctl[ 0 ], DR_REFRESH );
}


/*
======================================================================
handle_new()

Panel callback, called when the user hits the "New" button.  Also
called by handle_key() when the user presses the 'n' key.  Opens a
file dialog, and if the user chooses a file, calls handle_file().
====================================================================== */

static void handle_new( LWControl *ectl, void *edata )
{
   static char
      node[ 256 ] = { 0 },
      path[ 256 ] = { 0 };
   LWDirInfoFunc *dif;
   const char *dir;

   if ( !filename[ 0 ] ) {
      if ( dif = panf->globalFun( LWDIRINFOFUNC_GLOBAL, GFUSE_TRANSIENT )) {
         dir = dif( "Content" );
         if ( dir )
            strcpy( path, dir );
      }
   }

   if ( 0 > filereq( "View", node, path, filename, sizeof( filename )))
      return;

   SET_STR( ctl[ 1 ], filename, sizeof( filename ));
   handle_file( ctl[ 1 ], NULL );
}


/*
======================================================================
handle_datatype()

Panel callback, called when the user selects a different data type.
====================================================================== */

static void handle_datatype( LWControl *ectl, void *edata )
{
   int i;

   GET_INT( ectl, i );
   if ( datatype != i ) {
      datatype = i;
      drawcb_text( ctl[ 0 ], NULL, 0 );
   }
}


/*
======================================================================
handle_unsigned()

Panel callback, called when the user toggles the Unsigned switch.
====================================================================== */

static void handle_unsigned( LWControl *ectl, void *edata )
{
   unsign = !unsign;
   drawcb_text( ctl[ 0 ], NULL, 0 );
}


/*
======================================================================
handle_byteorder()

Panel callback, called when the user toggles the byte order selection.
====================================================================== */

static void handle_byteorder( LWControl *ectl, void *edata )
{
   int i;

   GET_INT( ectl, i );
   if ( byteorder != i ) {
      byteorder = i;
      drawcb_text( ctl[ 0 ], NULL, 0 );
   }
}


/*
======================================================================
handle_rowsize()

Panel callback, called when the user changes the number of bytes per
row in the buffer display.
====================================================================== */

static void handle_rowsize( LWControl *ectl, void *edata )
{
   int i;

   GET_INT( ectl, i );
   if ( i < linesize ) {
      clrtext( 1, 13, NROWS, 60 );
      clrtext( 1, 63, NROWS, 78 );
   }
   linesize = i;
   pagesize = i * NROWS;
   drawcb_text( ctl[ 0 ], NULL, 0 );
}


/*
======================================================================
handle_jump()

Panel callback, called when the user presses Enter in the Jump edit
field.  Also called by handle_key() when the user presses the 'j' key.

The file pointer is moved.  The number in the edit field represents
the new position as

   1. an offset forward from the current position
   2. an offset backward from the current position
   3. a new absolute position
====================================================================== */

static void handle_jump( LWControl *ectl, void *edata )
{
   char a[ 24 ];
   int i, jcontext;

   GET_STR( ectl, a, sizeof( a ));
   GET_INT( ctl[ 12 ], jcontext );
   i = atoi( a );

   switch ( jcontext ) {
      case 1:   change_pos( pos, i );  break;
      case 2:   change_pos( pos, -i ); break;
      default:  change_pos( 0, i );    break;
   }
}


/*
======================================================================
handle_search()

Panel callback, called when the user presses Enter in the Search edit
field.  Also called by handle_key() when the user presses the 's' key.

The string in the edit field is the search key, and it can represent
literal text, a binary number in the current format, or a mixture of
text and hex codes.  The search begins at the current position and
looks forward ("down").
====================================================================== */

static void handle_search( LWControl *ectl, void *edata )
{
   char userstr[ 64 ];
   int newpos;

   if ( !fp ) return;

   GET_INT( ctl[ 14 ], searchtype );
   GET_STR( ctl[ 13 ], userstr, sizeof( userstr ));
   if ( strlen( userstr ) == 0 ) return;

   newpos = pos;
   if ( search( fp, &newpos, userstr, searchtype, datatype, unsign, byteorder ))
      change_pos( 0, newpos );
   else
      msgf->info( "Search string not found.", NULL );
}


/*
======================================================================
handle_scroll()

Panel callback, called when the user changes the scrollbar position.
====================================================================== */

static void handle_scroll( LWControl *ectl, void *edata )
{
   int i, newpos;

   inscroll = 1;
   GET_INT( ectl, i );
   newpos = i * ( filesize / 1000 );
   if ( newpos > filesize - pagesize )
      newpos = filesize - pagesize;
   if ( newpos < 0 ) newpos = 0;
   change_pos( 0, newpos );
   inscroll = 0;
}


/*
======================================================================
print_ui()

Called by handle_print() to set output options.
====================================================================== */

static int print_ui( char *outname, int *range, int *mode, int *cols,
   char *comment )
{
   char *rlist[] = { "Current Page", "Entire File", NULL };
   char *mlist[] = { "Write", "Append", NULL };
   char *tlist[] = { " ", NULL };
   LWPanelID panel;
   LWControl *ctl[ 8 ];
   int i, x, y, dy, ok;

   if( !( panel = PAN_CREATE( panf, "Print to File" )))
      return 0;

   ctl[ 0 ] = FILE_CTL( panf, panel, "File", 40 );
   ctl[ 1 ] = HCHOICE_CTL( panf, panel, "Range", rlist );
   ctl[ 2 ] = HCHOICE_CTL( panf, panel, "Mode", mlist );
   ctl[ 3 ] = TEXT_CTL( panf, panel, "Columns", tlist );
   ctl[ 4 ] = BOOLBUTTON_CTL( panf, panel, "Position" );
   ctl[ 5 ] = BOOLBUTTON_CTL( panf, panel, "Buffer" );
   ctl[ 6 ] = BOOLBUTTON_CTL( panf, panel, "Printable" );
   ctl[ 7 ] = STR_CTL( panf, panel, "Comment", 40 );

   for ( i = 0; i < 4; i++ ) {
      ival.intv.value = 100 - CON_LW( ctl[ i ] );
      ctl[ i ]->set( ctl[ i ], CTL_X, &ival );
   }

   x = CON_HOTX( ctl[ 2 ] );
   y = CON_Y( ctl[ 2 ] );
   dy = y - CON_Y( ctl[ 1 ] );
   y += dy;

   MOVE_CON( ctl[ 4 ], x, y );
   x += CON_W( ctl[ 4 ] );
   MOVE_CON( ctl[ 5 ], x, y );
   x += CON_W( ctl[ 5 ] );
   MOVE_CON( ctl[ 6 ], x, y );
   x = 100 - CON_LW( ctl[ 7 ] );
   y += dy;
   MOVE_CON( ctl[ 7 ], x, y );

   PAN_SETH( panf, panel, y + dy );

   SET_STR( ctl[ 0 ], outname, 256 );
   SET_INT( ctl[ 1 ], *range );
   SET_INT( ctl[ 2 ], *mode );
   SET_INT( ctl[ 4 ], ( *cols & 1 ));
   SET_INT( ctl[ 5 ], ( *cols & 2 ));
   SET_INT( ctl[ 6 ], ( *cols & 4 ));
   SET_STR( ctl[ 7 ], comment, 256 );

   ok = panf->open( panel, PANF_BLOCKING | PANF_CANCEL );

   if ( ok ) {
      GET_STR( ctl[ 0 ], outname, 256 );
      GET_INT( ctl[ 1 ], *range );
      GET_INT( ctl[ 2 ], *mode );
      GET_INT( ctl[ 4 ], i );  if ( i ) *cols |= 1; else *cols &= ~1;
      GET_INT( ctl[ 5 ], i );  if ( i ) *cols |= 2; else *cols &= ~2;
      GET_INT( ctl[ 6 ], i );  if ( i ) *cols |= 4; else *cols &= ~4;
      GET_STR( ctl[ 7 ], comment, 256 );
   }

   PAN_KILL( panf, panel );

   return ok;
}


/*
======================================================================
handle_print()

Panel callback, called when the user presses 'p'.  Writes the display
to a file.
====================================================================== */

static void handle_print( void )
{
   static char outname[ 256 ] = { "out.txt" };
   static char comment[ 256 ] = { "BinView Output" };
   static int range = 0, mode = 0, flags = 7;
   FILE *ofp;
   int rlen, ok;
   long p, plen;


   ok = print_ui( outname, &range, &mode, &flags, comment );
   if ( !ok ) return;
   ofp = fopen( outname, mode ? "a" : "w" );
   if ( !ofp ) return;

   fprintf( ofp, "%s\n", comment );

   p = range ? 0 : pos;
   plen = range ? filesize : pagesize;
   fseek( fp, p, SEEK_SET );
   while ( plen ) {
      rlen = fread( buf, 1, pagesize, fp );
      if ( rlen <= 0 ) break;
      print_buf( buf, rlen, datatype, unsign, byteorder != natorder,
         linesize, p, flags, ofp );
      p += rlen;
      plen -= rlen;
   }
   fclose( ofp );
   fseek( fp, pos, SEEK_SET );
}


/*
======================================================================
handle_key()

Panel callback, called when the user presses a key.  In many cases,
this function calls the panel callback for the corresponding control.
====================================================================== */

static void handle_key( LWPanelID panel, void *pdata, LWDualKey key )
{
   if ( !fp ) return;

   switch ( key )
   {
      case LWDK_SC_UP:     change_pos( pos, -linesize );         break;
      case LWDK_SC_DOWN:   change_pos( pos, linesize );          break;
      case LWDK_SC_LEFT:   change_pos( pos, -1 );                break;
      case LWDK_SC_RIGHT:  change_pos( pos, 1 );                 break;

      case LWDK_PAGEUP:
      case LWDK_PAD_9:     change_pos( pos, -pagesize );         break;
      case LWDK_PAGEDOWN:
      case LWDK_PAD_3:     change_pos( pos, pagesize );          break;
      case LWDK_HOME:
      case LWDK_PAD_7:     change_pos( 0, 0 );                   break;
      case LWDK_END:
      case LWDK_PAD_1:     change_pos( 0, filesize - pagesize ); break;

      case 'p':  handle_print();                  break;
      case 'n':  handle_new( ctl[ 16 ], NULL );   break;
      case 'j':  handle_jump( ctl[ 7 ], NULL );   break;
      case 's':  handle_search( ctl[ 8 ], NULL ); break;
      case '+':  SET_INT( ctl[ 12 ], 1 );         break;
      case '-':  SET_INT( ctl[ 12 ], 2 );         break;
      case '*':  SET_INT( ctl[ 12 ], 0 );         break;
      case 't':  SET_INT( ctl[ 14 ], 0 );         break;
      case 'h':  SET_INT( ctl[ 14 ], 1 );         break;
      case 'c':  SET_INT( ctl[ 14 ], 2 );         break;

      case 'b':
         SET_INT( ctl[ 3 ], 0 );
         handle_datatype( ctl[ 3 ], NULL );
         break;
      case 'w':
         SET_INT( ctl[ 3 ], 1 );
         handle_datatype( ctl[ 3 ], NULL );
         break;
      case 'l':
         SET_INT( ctl[ 3 ], 2 );
         handle_datatype( ctl[ 3 ], NULL );
         break;
      case 'f':
         SET_INT( ctl[ 3 ], 3 );
         handle_datatype( ctl[ 3 ], NULL );
         break;
      case 'd':
         SET_INT( ctl[ 3 ], 4 );
         handle_datatype( ctl[ 3 ], NULL );
         break;
      case 'u': {
         int i = !unsign;
         SET_INT( ctl[ 4 ], i );
         handle_unsigned( ctl[ 4 ], NULL );
         }
         break;
      case 'i':
         SET_INT( ctl[ 5 ], 0 );
         handle_byteorder( ctl[ 5 ], NULL );
         break;
      case 'm':
         SET_INT( ctl[ 5 ], 1 );
         handle_byteorder( ctl[ 5 ], NULL );
         break;
      case ',':
         if ( linesize > 1 ) {
            SET_INT( ctl[ 6 ], linesize - 1 );
            handle_rowsize( ctl[ 6 ], NULL );
         }
         break;
      case '.':
         if ( linesize < 16 ) {
            SET_INT( ctl[ 6 ], linesize + 1 );
            handle_rowsize( ctl[ 6 ], NULL );
         }
         break;
      default:
         break;
   }
}


/*
======================================================================
handle_panopen()

Panel callback, called just after the panel is opened.  We call
handle_new() to let the user choose the first file to be displayed.
====================================================================== */

static void handle_panopen( LWPanelID panel, void *pdata )
{
   handle_new( ctl[ 16 ], NULL );
}


/*
======================================================================
create_controls()

Called by open_window() to create the panel controls.
====================================================================== */

static void create_controls( void )
{
   static char *labtype[] = { "Byte", "Word", "Long", "Float", "Double", NULL };
   static char *labbyte[] = { "Intel", "Motorola", NULL };
   static char *labjump[] = { "Absolute", "Forward", "Backward", NULL };
   static char *labsrch[] = { "Text", "Text + Hex", "Current", NULL };
   int x, y, ph;


   /* create a control */

   ctl[ 1 ] = STR_CTL( panf, panel, "", 58 );

   /* find out how much vertical space the panel wants for drawing its
      own decorations */

   ph = PAN_GETH( panf, panel ) - CON_H( ctl[ 1 ] );

   /* create the rest of the controls */

   ctl[  0 ] = CANVAS_CTL( panf, panel, "", 78 * TXWIDTH + DX * 2,
      NROWS * TXHEIGHT + DY * 2 );

   ctl[  2 ] = STRRO_CTL( panf, panel, "", 21 );
   ctl[  3 ] = WPOPUP_CTL( panf, panel, "", labtype, 76 );
   ctl[  4 ] = BOOL_CTL( panf, panel, "Unsigned" );
   ctl[  5 ] = WPOPUP_CTL( panf, panel, "", labbyte, 76 );
   ctl[  6 ] = MINISLIDER_CTL( panf, panel, "", 4, 1, 16 );
   ctl[  7 ] = STR_CTL( panf, panel, "Jump", 12 );
   ctl[  8 ] = WBUTTON_CTL( panf, panel, "Search", 56 );
   ctl[  9 ] = STRRO_CTL( panf, panel, "Mod", 4 );
   ctl[ 10 ] = VSLIDER_CTL( panf, panel, "", NROWS * TXHEIGHT + DY * 2, 0, 1000 );
   ctl[ 11 ] = STRRO_CTL( panf, panel, "", 12 );
   ctl[ 12 ] = WPOPUP_CTL( panf, panel, "", labjump, 80 );
   ctl[ 13 ] = STR_CTL( panf, panel, "", 24 );
   ctl[ 14 ] = WPOPUP_CTL( panf, panel, "", labsrch, 80 );
   ctl[ 15 ] = CANVAS_CTL( panf, panel, "", 32, 32 );
   ctl[ 16 ] = WBUTTON_CTL( panf, panel, "New", 40 );

   /* position all of the controls */

   x = CON_X( ctl[ 0 ] );
   y = CON_Y( ctl[ 0 ] );
   y += CON_H( ctl[ 2 ] );
   MOVE_CON( ctl[ 0 ], x, y + 2 );

   y = CON_Y( ctl[ 0 ] );
   MOVE_CON( ctl[ 10 ], 79 * TXWIDTH + 10, y + 1 );

   y = CON_Y( ctl[ 1 ] ) + 2;
   x = CON_X( ctl[ 0 ] );
   MOVE_CON( ctl[ 16 ], x + DX, y );

   x += CON_W( ctl[ 16 ] );
   MOVE_CON( ctl[ 1 ], x, y );

   x = CON_X( ctl[ 1 ] );
   x += CON_W( ctl[ 1 ] );
   MOVE_CON( ctl[ 2 ], x, y );

   x = CON_X( ctl[ 10 ] );
   x += CON_W( ctl[ 10 ] );
   MOVE_CON( ctl[ 15 ], x - 32, y );

   y += CON_H( ctl[ 1 ] ) - 2;
   x = CON_X( ctl[ 0 ] );
   MOVE_CON( ctl[ 11 ], x - 3, y );

   x += CON_W( ctl[ 11 ] );
   MOVE_CON( ctl[ 9 ], x, y );

   x = CON_X( ctl[ 9 ] );
   x += CON_W( ctl[ 9 ] );
   MOVE_CON( ctl[ 6 ], x, y );

   x = CON_X( ctl[ 6 ] );
   x += CON_W( ctl[ 6 ] );
   MOVE_CON( ctl[ 5 ], x, y );

   x = CON_X( ctl[ 5 ] );
   x += CON_W( ctl[ 5 ] );
   MOVE_CON( ctl[ 3 ], x - 6, y );

   x = CON_X( ctl[ 3 ] );
   x += CON_W( ctl[ 3 ] );
   MOVE_CON( ctl[ 4 ], x, y );

   y = CON_Y( ctl[ 0 ] );
   y += CON_H( ctl[ 0 ] );
   x = CON_X( ctl[ 0 ] );
   MOVE_CON( ctl[ 7 ], x + DX, y );

   x = CON_X( ctl[ 7 ] );
   x += CON_W( ctl[ 7 ] );
   MOVE_CON( ctl[ 12 ], x - 4, y );

   x = CON_X( ctl[ 12 ] );
   x += CON_W( ctl[ 12 ] );
   MOVE_CON( ctl[ 8 ], x + 18, y );

   x = CON_X( ctl[ 8 ] );
   x += CON_W( ctl[ 8 ] );
   MOVE_CON( ctl[ 13 ], x - 4, y );

   x = CON_X( ctl[ 13 ] );
   x += CON_W( ctl[ 13 ] );
   MOVE_CON( ctl[ 14 ], x - 4, y );

   /* now that we know how much room the controls will take up, set the
      height of the panel */

   y = CON_Y( ctl[ 8 ] );
   y += CON_H( ctl[ 8 ] );
   PAN_SETH( panf, panel, y + ph - 6 );

   /* initialize the controls */

   SET_INT( ctl[ 3 ], datatype );
   SET_INT( ctl[ 4 ], unsign );
   SET_INT( ctl[ 5 ], byteorder );
   SET_INT( ctl[ 6 ], linesize );
   SET_STR( ctl[ 7 ], "0", 2 );
   SET_STR( ctl[ 9 ], "0", 2 );
   SET_INT( ctl[ 10 ], 0 );
   SET_STR( ctl[ 11 ], "0", 2 );
   SET_INT( ctl[ 12 ], 0 );
   SET_INT( ctl[ 14 ], 0 );

   /* set the control event callbacks */

   CON_SETEVENT( ctl[  1 ], handle_file, NULL );
   CON_SETEVENT( ctl[  3 ], handle_datatype, NULL );
   CON_SETEVENT( ctl[  4 ], handle_unsigned, NULL );
   CON_SETEVENT( ctl[  5 ], handle_byteorder, NULL );
   CON_SETEVENT( ctl[  6 ], handle_rowsize, NULL );
   CON_SETEVENT( ctl[  7 ], handle_jump, NULL );
   CON_SETEVENT( ctl[  8 ], handle_search, NULL );
   CON_SETEVENT( ctl[ 10 ], handle_scroll, NULL );
   CON_SETEVENT( ctl[ 13 ], handle_search, NULL );
   CON_SETEVENT( ctl[ 16 ], handle_new, NULL );

   /* set the control drawing callbacks */

   ival.ptr.ptr = drawcb_text;
   ctl[ 0 ]->set( ctl[ 0 ], CTL_USERDRAW, &ival );
   ival.ptr.ptr = drawcb_icon;
   ctl[ 15 ]->set( ctl[ 15 ], CTL_USERDRAW, &ival );
}


/*
======================================================================
free_window()

Perform cleanup after the window has been closed.  Called by BinView()
in main.c.
====================================================================== */

void free_window( void )
{
   if ( fp ) {
      fclose( fp );
      fp = NULL;
   }
}


/*
======================================================================
open_window()

Create and display the user interface.  Called by BinView() in main.c.
This function doesn't return until the user closes the window.
====================================================================== */

int open_window( GlobalFunc *global )
{
   int i;

   natorder = native_order();

   msgf    = global( LWMESSAGEFUNCS_GLOBAL, GFUSE_TRANSIENT );
   filereq = global( LWFILEREQFUNC_GLOBAL,  GFUSE_TRANSIENT );
   rasf    = global( LWRASTERFUNCS_GLOBAL,  GFUSE_TRANSIENT );
   panf    = global( LWPANELFUNCS_GLOBAL,   GFUSE_TRANSIENT );
   if ( !msgf || !filereq || !rasf || !panf )
      return AFUNC_BADGLOBAL;

   panf->globalFun = global;

   if( !( panel = PAN_CREATE( panf, PANEL_TITLE ))) {
      msgf->error( PLUGIN_NAME " couldn't create its panel,", "not sure why." );
      return AFUNC_OK;
   }

   panf->set( panel, PAN_USERKEYS, handle_key );
   panf->set( panel, PAN_USEROPEN, handle_panopen );

   cras[ 0 ] = create_char( COLOR_WHITE, SYSTEM_Ic( 30 ), rasf );
   cras[ 1 ] = create_char( COLOR_BLACK, SYSTEM_Ic(  2 ), rasf );
   cras[ 2 ] = create_char( COLOR_BLACK, SYSTEM_Ic( 15 ), rasf );

   create_controls();
   panf->open( panel, PANF_BLOCKING );

   PAN_KILL( panf, panel );

   for ( i = 0; i < 3; i++ )
      free_char( cras[ i ], rasf );
   free_icon( rasf );

   return AFUNC_OK;
}
