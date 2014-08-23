/*
======================================================================
wmain.c

Ernie Wright  4 Dec 00

Windows stub for testing Flex image loading and saving.
====================================================================== */

#include <windows.h>
#include <stdio.h>
#include "flex.h"

#define IDM_OPEN 100
#define IDM_SAVE 101
#define IDM_EXIT 102


static BITMAPINFOHEADER dib;
static unsigned char *bits = NULL;


/*
======================================================================
file_request()

Get a filename from the user.
====================================================================== */

char *file_request( HWND hwnd, int load )
{
   static char name[ 260 ], path[ 260 ], node[ 260 ];
   OPENFILENAME ofn = { 0 };
   int result;

   strcpy( name, node );

   ofn.lStructSize     = sizeof( OPENFILENAME );
   ofn.hwndOwner       = hwnd;
   ofn.lpstrFilter     = "All\0*.*\0\0";
   ofn.nFilterIndex    = 1;
   ofn.lpstrFile       = name;
   ofn.nMaxFile        = sizeof( name );
   ofn.lpstrFileTitle  = node;
   ofn.nMaxFileTitle   = sizeof( node );
   ofn.lpstrInitialDir = path;
   ofn.Flags           = OFN_EXPLORER | OFN_NOCHANGEDIR;

   if ( load ) {
      ofn.lpstrTitle = "Load Flex Image";
      ofn.Flags |= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
      result = GetOpenFileName( &ofn );
   }
   else {
      ofn.lpstrTitle = "Save As Flex Image";
      ofn.Flags |= OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
      result = GetSaveFileName( &ofn );
   }

   if ( result ) {
      strcpy( path, name );
      path[ ofn.nFileOffset ] = 0;
      strcpy( node, name + ofn.nFileOffset );
      return name;
   }

   return NULL;
}


/*
======================================================================
load_image()

Load a Flex image, convert it to DIB and blit it to the window.
====================================================================== */

static int load_image( HWND hwnd )
{
   Flex *flex;
   Layer *layer;
   char *filename, *buf = NULL;
   int rowbytes, rsize, i, ok = 0;


   /* get the filename */

   filename = file_request( hwnd, 1 );
   if ( !filename ) return 1;

   /* open and scan the file */

   flex = flexReadBegin( filename, NULL, NULL );
   if ( !flex ) goto Done;

   /* initialize a Windows DIB for displaying the image */
   /* DIB rasters are longword-aligned */

   rowbytes = (( flex->hdr.width * 3 + 3 ) >> 2 ) << 2;

   dib.biSize          = sizeof( BITMAPINFOHEADER );
   dib.biWidth         = flex->hdr.width;
   dib.biHeight        = flex->hdr.height;
   dib.biPlanes        = 1;
   dib.biBitCount      = 24;
   dib.biCompression   = 0;
   dib.biSizeImage     = rowbytes * flex->hdr.height;
   dib.biXPelsPerMeter = 3780;
   dib.biYPelsPerMeter = 3780;
   dib.biClrUsed       = 0;
   dib.biClrImportant  = 0;

   if ( bits ) free( bits );
   bits = malloc( dib.biSizeImage );
   if ( !bits ) goto Done;

   /* how big are the RGB component layers? */

   rsize = flexLayerSize( flex, 0, Layer_RED );
   if ( rsize <= 0 ) goto Done;

   /* allocate memory for the layer to be written into */

   buf = malloc( rsize );
   if ( !buf ) goto Done;

   /* read the red, green and blue layers */

   for ( i = 0; i < 3; i++ ) {
      layer = flexReadLayer( flex, 0, Layer_RED + i, buf );
      if ( !layer ) goto Done;

      /* convert to 8-bit and write in the DIB */

      flexLayerToByte( layer, buf, bits + dib.biSizeImage - rowbytes + 2 - i,
         -rowbytes, 3 );
   }

   /* success */

   ok = 1;

Done:
   if ( buf ) free( buf );
   if ( !ok ) if ( bits ) { free( bits ); bits = NULL; }
   if ( flex ) flexReadDone( flex );

   /* send the window a WM_PAINT message */

   InvalidateRect( hwnd, NULL, TRUE );
   UpdateWindow( hwnd );

   return ok;
}


/*
======================================================================
save_image()

Save the current DIB as a Flex image.
====================================================================== */

static int save_image( HWND hwnd )
{
   Flex flex;
   Layer layer;
   char *filename, *buf = NULL;
   int rowbytes, i, ok = 0;


   if ( !bits ) return 0;

   /* get the filename */

   filename = file_request( hwnd, 0 );
   if ( !filename ) return 1;

   /* initialize an FPHeader for saving the image */

   flex.hdr.width           = ( short ) dib.biWidth;
   flex.hdr.height          = ( short ) dib.biHeight;
   flex.hdr.numLayers       = 3;
   flex.hdr.numFrames       = 1;
   flex.hdr.numBuffers      = 1;
   flex.hdr.flags           = Source_FP;      /* a white lie */
   flex.hdr.srcLayerDepth   = 4;
   flex.hdr.pad2            = 0;
   flex.hdr.framesPerSecond = 30.0f;
   flex.hdr.pixelWidth      = ( float ) dib.biXPelsPerMeter;
   flex.hdr.pixelAspect     = ( float ) dib.biXPelsPerMeter / dib.biYPelsPerMeter;

   /* open the file and write the FPHD and FLEX */

   if ( !flexWriteBegin( &flex, filename )) goto Done;
   if ( !flexWriteFrame( &flex, 3 )) goto Done;

   /* initialize a Layer structure */

   layer.w               = flex.hdr.width;
   layer.h               = flex.hdr.height;
   layer.size            = layer.w * layer.h * 4;
   layer.hdr.flags       = Layer_FP;
   layer.hdr.layerDepth  = 4;
   layer.hdr.compression = VerticalRLE;
   layer.hdr.blackPoint  = 0.0f;
   layer.hdr.whitePoint  = 1.0f;
   layer.hdr.gamma       = 1.0f;

   /* allocate memory for the layer to be written from */

   buf = malloc( layer.size );
   if ( !buf ) goto Done;

   /* DIB rasters are longword-aligned */

   rowbytes = (( flex.hdr.width * 3 + 3 ) >> 2 ) << 2;

   /* write the red, green and blue layers */

   for ( i = 0; i < 3; i++ ) {
      layer.hdr.layerType = Layer_RED + i;
      flexByteToLayer( &layer, bits + dib.biSizeImage - rowbytes + 2 - i, buf,
         -rowbytes, 3 );
      ok = flexWriteLayer( &flex, &layer, buf );
      if ( !ok ) goto Done;
   }

   /* success */

   ok = 1;

Done:
   if ( buf ) free( buf );
   if ( ok )
      flexWriteDone( &flex );
   else
      if ( flex.fp )
         fclose( flex.fp );

   return ok;
}


/*
======================================================================
WndProc()

The window callback.
====================================================================== */

LRESULT CALLBACK WndProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam )
{
   PAINTSTRUCT ps;

   switch ( message )
   {
      case WM_PAINT:
         BeginPaint( hwnd, &ps );
         if ( bits )
            StretchDIBits( ps.hdc, 0, 0, dib.biWidth, dib.biHeight, 0, 0,
               dib.biWidth, dib.biHeight, bits, ( BITMAPINFO * ) &dib,
               DIB_RGB_COLORS, SRCCOPY );
         EndPaint( hwnd, &ps );
         return 0;

      case WM_COMMAND:
         switch ( LOWORD( wparam )) {
            case IDM_OPEN:
               if ( !load_image( hwnd ))
                  MessageBox( hwnd, "Couldn't load the image.", "Load Error", MB_OK );
               return 0;
            case IDM_SAVE:
               if ( !save_image( hwnd ))
                  MessageBox( hwnd, "Couldn't save the image.", "Save Error", MB_OK );
               return 0;
            case IDM_EXIT:
               SendMessage( hwnd, WM_CLOSE, 0, 0 );
               return 0;
         }
         break;

      case WM_DESTROY:
         PostQuitMessage( 0 );
         return 0;
   }

   return DefWindowProc( hwnd, message, wparam, lparam );
}


/*
======================================================================
WinMain()

The entry point.  Create a window and enter the message loop.
====================================================================== */

int PASCAL WinMain( HINSTANCE inst, HINSTANCE pinst, LPSTR cmdline, int cmdshow )
{
   static char appname[] = "FlexTest";
   HWND hwnd;
   HMENU menu, pop;
   WNDCLASS wc;
   MSG msg;

   if ( !pinst ) {
      wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
      wc.lpfnWndProc   = WndProc;
      wc.cbClsExtra    = 0;
      wc.cbWndExtra    = 0;
      wc.hInstance     = inst;
      wc.hIcon         = LoadIcon( NULL, IDI_APPLICATION );
      wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
      wc.hbrBackground = GetStockObject( GRAY_BRUSH );
      wc.lpszClassName = appname;
      wc.lpszMenuName  = NULL;
      RegisterClass( &wc );
   }

   menu = CreateMenu();
   pop = CreateMenu();
   AppendMenu( pop, MF_STRING, IDM_OPEN, "&Open..." );
   AppendMenu( pop, MF_STRING, IDM_SAVE, "&Save As..." );
   AppendMenu( pop, MF_STRING, IDM_EXIT, "E&xit" );
   AppendMenu( menu, MF_POPUP, ( UINT ) pop, "&File" );

   hwnd = CreateWindow( appname, appname,
      WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
      GetSystemMetrics( SM_CXSCREEN ) / 2, GetSystemMetrics( SM_CYSCREEN ) / 2,
      NULL, menu, inst, NULL );

   ShowWindow( hwnd, cmdshow );
   UpdateWindow( hwnd );

   while ( GetMessage( &msg, NULL, 0, 0 )) {
      TranslateMessage( &msg );
      DispatchMessage( &msg );
   }

   return msg.wParam;
}
