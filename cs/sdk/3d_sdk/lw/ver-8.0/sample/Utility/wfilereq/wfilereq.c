/*
======================================================================
wfilereq.c

Demonstrate the FileRequest class using the Windows file dialog.

Ernie Wright  18 Mar 00
====================================================================== */

#include <windows.h>
#include <shlobj.h>
#include <lwserver.h>
#include <lwdialog.h>
#include <lwdisplay.h>
#include <lwhost.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
======================================================================
build_filter()

Create a Win32 OPENFILENAME filter string based on the LightWave file
type string, which contains things like "Scenes" and "Images".  The
File Type Pattern global maps the descriptive name to a string of
filename patterns stored in the LightWave config file, and we add to
that the catch-all "*.*" filter.
====================================================================== */

char *build_filter( GlobalFunc *global, const char *typestr )
{
   LWFileTypeFunc *filetypes;
   const char *lwfilt;
   char *p, *filter = NULL;
   int len1 = 0, len2 = 0;

   if ( typestr ) {
      filetypes = global( LWFILETYPEFUNC_GLOBAL, GFUSE_TRANSIENT );
      if ( filetypes ) {
         lwfilt = filetypes( typestr );
         if ( lwfilt ) {
            len2 = strlen( lwfilt );
            if ( len2 )
               len1 = strlen( typestr );
         }
      }
   }

   filter = malloc( len1 + len2 + 12 );
   if ( filter ) {
      p = filter;
      if ( len1 && len2 ) {
         strcpy( p, typestr );
         p += len1 + 1;
         strcpy( p, lwfilt );
         p += len2 + 1;
      }
      memcpy( p, "All\0*.*\0\0", 9 );
   }

   return filter;
}


/*
======================================================================
append_ext()

Add an extension to a filename.  Called by frsave() to automatically
append an extension when the filename entered by the user lacks one.
====================================================================== */

static void append_ext( char *filename, char *filter )
{
   char *p;

   if ( !filter ) return;

   p = filter + strlen( filter ) + 1;
   p = strchr( p, '.' );
   if ( !p ) return;
   p = strtok( p, ";" );
   if ( strchr( p, '*' )) return;
   if ( strchr( p, '?' )) return;

   strcat( filename, p );
}


/*
======================================================================
parse_multinames()

Extract an array of filename strings from the lpstrFile field of the
OPENFILENAME.  Called by frmload().
====================================================================== */

static char **parse_multinames( char *namestr, int slen, int *count )
{
   char **name, *p;
   int i, n, len;


   /* skip the path string */

   namestr += strlen( namestr ) + 1;

   /* find number of names and buffer size */

   p = namestr;
   len = 0;
   for ( n = 0; *p && len < slen; n++ ) {
      i = strlen( p ) + 1;
      len += i;
      p += i;
   }
   if ( n == 0 ) return NULL;

   /* create an array of name pointers */

   name = calloc( n, sizeof( char * ));
   if ( !name ) return NULL;

   p = namestr;
   for ( i = 0; i < n; i++ ) {
      name[ i ] = p;
      p += strlen( p ) + 1;
   }

   *count = n;
   return name;
}


/*
======================================================================
frload()

Call the Win32 file dialog function for FREQ_LOAD requests.
====================================================================== */

static int frload( LWFileReqLocal *local, OPENFILENAME *ofn )
{
   int result;

   ofn->Flags |= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
   result = GetOpenFileName( ofn );

   if ( result ) {
      local->result = 1;

      if ( strlen( ofn->lpstrFile ) >= local->bufLen ) {
         local->result = -1;
         return 0;
      }
      strcpy( local->fullName, ofn->lpstrFile );
      strcpy( local->path, ofn->lpstrFile );
      local->path[ ofn->nFileOffset ] = 0;
      strcpy( local->baseName, ofn->lpstrFile + ofn->nFileOffset );
   }
   else
      local->result = 0;

   return 1;
}


/*
======================================================================
frsave()

Call the Win32 file dialog function for FREQ_SAVE requests.  If the
filename entered by the user doesn't have an extension, we supply one
by calling append_ext().
====================================================================== */

static int frsave( LWFileReqLocal *local, OPENFILENAME *ofn )
{
   int result;

   ofn->Flags |= OFN_HIDEREADONLY;
   result = GetSaveFileName( ofn );

   if ( result ) {
      local->result = 1;

      if (( unsigned ) strlen( ofn->lpstrFile ) >= local->bufLen ) {
         local->result = -1;
         return 0;
      }

      if ( !strchr( ofn->lpstrFile, '.' ))
         append_ext( ofn->lpstrFile, ofn->lpstrFilter );

      strcpy( local->fullName, ofn->lpstrFile );
      strcpy( local->path, ofn->lpstrFile );
      local->path[ ofn->nFileOffset ] = 0;
      strcpy( local->baseName, ofn->lpstrFile + ofn->nFileOffset );

   }
   else
      local->result = 0;

   return 1;
}


/*
======================================================================
browse_callback()

Windows SHBrowseForFolder() callback used by frdir().  We enlarge the
dialog window and expand the initial path in the directory tree view.
====================================================================== */

#define BWX 64    /* width delta  */
#define BWY 128   /* height delta */

static int CALLBACK browse_callback( HWND hwnd, UINT msg, LPARAM lparam,
   LPARAM data )
{
   static int id[ 6 ] = { 0, 0x3742, 0x3743, 0x3741, 1, 2 };
   static int pos[ 6 ][ 4 ] = {
        0,   0, BWX, BWY,
        0,   0, BWX,   0,
        0,   0, BWX,   0,
        0,   0, BWX, BWY,
      BWX, BWY,   0,   0,
      BWX, BWY,   0,   0
   };

   LWFileReqLocal *local;
   HWND wchild;
   POINT pt;
   RECT r;
   char path[ MAX_PATH ], *node;
   int x, y, w, h, i;

   if ( msg != BFFM_INITIALIZED ) return 0;

   /* enlarge the dialog */

   GetWindowRect( hwnd, &r );
   x = r.left + pos[ 0 ][ 0 ];
   y = r.top  + pos[ 0 ][ 1 ];
   w = r.right - r.left + pos[ 0 ][ 2 ];
   h = r.bottom - r.top + pos[ 0 ][ 3 ];

   /* make sure it fits in the desktop work area */

   if ( SystemParametersInfo( SPI_GETWORKAREA, 0, &r, 0 )) {
      if ( x < r.left )       x = r.left;
      if ( x + w > r.right )  x = r.right - w;
      if ( y < r.top )        y = r.top;
      if ( y + h > r.bottom ) y = r.bottom - h;
   }
   MoveWindow( hwnd, x, y, w, h, TRUE );

   /* move/resize the dialog controls */

   for ( i = 1; i < 6; i++ ) {
      wchild = GetDlgItem( hwnd, id[ i ] );
      GetWindowRect( wchild, &r );

      pt.x = r.left;
      pt.y = r.top;
      ScreenToClient( hwnd, &pt );
      x = pt.x + pos[ i ][ 0 ];
      y = pt.y + pos[ i ][ 1 ];
      w = r.right - r.left + pos[ i ][ 2 ];
      h = r.bottom - r.top + pos[ i ][ 3 ];

      MoveWindow( wchild, x, y, w, h, FALSE );
   }

   local = ( LWFileReqLocal * ) data;
   GetFullPathName( local->path, MAX_PATH, path, &node );
   i = strlen( path );
   if ( path[ i - 1 ] == '\\' ) path[ i - 1 ] = 0;
   SendMessage( hwnd, BFFM_SETSELECTION, TRUE, ( LPARAM ) path );

   return 0;
}


/*
======================================================================
frdir()

Call the Win32 shell directory browser for FREQ_DIRECTORY requests.
====================================================================== */

static int frdir( LWFileReqLocal *local, HWND hwnd )
{
   BROWSEINFO bi;
   LPCITEMIDLIST idlist;
   char node[ MAX_PATH ];
   int result;

   bi.hwndOwner = hwnd;
   bi.pidlRoot = NULL;
   bi.pszDisplayName = node;
   bi.lpszTitle = local->title;
   bi.ulFlags = BIF_RETURNONLYFSDIRS;
   bi.lpfn = browse_callback;
   bi.lParam = ( LPARAM ) local;
   bi.iImage = 0;

   idlist = SHBrowseForFolder( &bi );
   if ( idlist ) {
      IMalloc *imalloc;

      result = SHGetPathFromIDList( idlist, local->path );
      if ( !result )
         local->result = 0;
      else {
         strcpy( local->baseName, node );
         strcat( local->path, "\\" );
         strcpy( local->fullName, local->path );
         local->result = 1;
      }
      if ( !SHGetMalloc( &imalloc )) {
         imalloc->lpVtbl->Free( imalloc, idlist );
         imalloc->lpVtbl->Release( imalloc );
      }
   }
   else
      local->result = 0;

   return 1;
}


/*
======================================================================
frmload()

Call the Win32 file dialog function for FREQ_MULTILOAD requests.  The
pickName() callback is called for each selected filename.
====================================================================== */

static int frmload( LWFileReqLocal *local, OPENFILENAME *ofn )
{
   char **name;
   int result, count, i, len;

   if ( !local->pickName ) {
      local->result = -1;
      return 0;
   }

   ofn->Flags |= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT;
   result = GetOpenFileName( ofn );

   if ( result ) {
      local->result = 1;

      if ( ofn->lpstrFile[ ofn->nFileOffset - 1 ] ) {
         if (( unsigned ) strlen( ofn->lpstrFile ) >= local->bufLen ) {
            local->result = -1;
            return 0;
         }
         strcpy( local->fullName, ofn->lpstrFile );
         strcpy( local->path, ofn->lpstrFile );
         local->path[ ofn->nFileOffset ] = 0;
         strcpy( local->baseName, ofn->lpstrFile + ofn->nFileOffset );
         local->pickName();
      }
      else {
         name = parse_multinames( ofn->lpstrFile, 32767, &count );
         if ( !name ) { local->result = -1; return 0; }

         if (( unsigned ) strlen( ofn->lpstrFile ) >= local->bufLen ) {
            local->result = -1;
            return 0;
         }
         strcpy( local->path, ofn->lpstrFile );

         for ( i = 0; i < count; i++ ) {
            len = strlen( name[ i ] );
            if ( len >= local->bufLen ) break;
            strcpy( local->baseName, name[ i ] );

            len += strlen( local->path ) + 1;
            if ( len >= local->bufLen ) break;
            sprintf( local->fullName, "%s\\%s", local->path, name[ i ] );

            if ( local->pickName() ) break;
         }
         free( name );
      }
   }
   else
      local->result = 0;

   return 1;
}


/*
======================================================================
WinFileReq()

The activation function.  This mostly initializes the OPENFILENAME
structure, then calls specialized functions for each of the request
types.
====================================================================== */

XCALL_( static int )
WinFileReq( long version, GlobalFunc *global, LWFileReqLocal *local,
   void *serverData )
{
   HWND hwnd;
   OPENFILENAME ofn = { 0 };
   char *bname, *filter;
   int bsize, ok;

   if ( version != LWFILEREQ_VERSION ) return AFUNC_BADVERSION;

   hwnd = GetForegroundWindow();

   if ( local->reqType == FREQ_DIRECTORY ) {
      ok = frdir( local, hwnd );
      return ok ? AFUNC_OK : AFUNC_BADLOCAL;
   }

   bsize = ( local->reqType == FREQ_MULTILOAD ) ? 32767 : 260;
   bname = malloc( bsize );
   if ( !bname ) return AFUNC_BADLOCAL;
   strcpy( bname, local->baseName );

   filter = build_filter( global, local->fileType );

   ofn.lStructSize     = sizeof( OPENFILENAME );
   ofn.hwndOwner       = hwnd;
   ofn.lpstrFilter     = filter;
   ofn.nFilterIndex    = 0;
   ofn.lpstrFile       = bname;
   ofn.nMaxFile        = bsize;
   ofn.lpstrFileTitle  = local->baseName;
   ofn.nMaxFileTitle   = local->bufLen;
   ofn.lpstrInitialDir = local->path;
   ofn.lpstrTitle      = local->title;
   ofn.Flags           = OFN_EXPLORER | OFN_NOCHANGEDIR;

   switch ( local->reqType ) {
      case FREQ_LOAD:       ok = frload(  local, &ofn );  break;
      case FREQ_SAVE:       ok = frsave(  local, &ofn );  break;
      case FREQ_MULTILOAD:  ok = frmload( local, &ofn );  break;
   }

   free( bname );
   if ( filter ) free( filter );

   return ok ? AFUNC_OK : AFUNC_BADLOCAL;
}


ServerRecord ServerDesc[] = {
   { LWFILEREQ_CLASS, "Demo_FileReq", WinFileReq },
   { NULL }
};
