/*
======================================================================
avisave.c

Save an AVI from within LW.  Compile as usual for a plug-in, and link
with VFW32.LIB.

Ernie Wright  14 Mar 00
====================================================================== */

#define  STRICT
#define  INC_OLE2
#include <windows.h>
#include <mmsystem.h>
#include <vfw.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <lwserver.h>
#include <lwdisplay.h>
#include <lwrender.h>
#include <lwanimsav.h>


typedef struct {
   char *             filename;
   PAVIFILE           avifile;
   PAVISTREAM         ps;
   PAVISTREAM         pscomp;
   AVICOMPRESSOPTIONS opts;
   BITMAPINFO *       bmi;
   BYTE *             bits;
   RECT               r;
   int                fwidth;
   int                fheight;
   int                index;
   int                y;
   int                rowbytes;
} SampleAVI;


static GlobalFunc *gglobal;
HostDisplayInfo *hdi;


/*
======================================================================
alloc_bmi()

Allocate memory for a Windows device-independent bitmap.

Makes space for a 24-bit image in CF_DIB format.  If rowbytes isn't
NULL, it's set to the number of bytes per scanline (DIB scanlines are
longword-aligned).  The BITMAPINFOHEADER is initialized for a 24-bit
DIB, and the memory is returned as a pointer to BITMAPINFO.
====================================================================== */

static BITMAPINFO *alloc_bmi( int width, int height, int *rowbytes )
{
   BITMAPINFO *bmi;
   int rb;

   rb = (( width * 3 + 3 ) >> 2 ) << 2;
   if ( rowbytes ) *rowbytes = rb;
   bmi = malloc( sizeof( BITMAPINFOHEADER ) + rb * height );
   if ( !bmi ) return NULL;

   bmi->bmiHeader.biSize          = sizeof( BITMAPINFOHEADER );
   bmi->bmiHeader.biWidth         = width;
   bmi->bmiHeader.biHeight        = height;
   bmi->bmiHeader.biPlanes        = 1;
   bmi->bmiHeader.biBitCount      = 24;
   bmi->bmiHeader.biCompression   = BI_RGB;
   bmi->bmiHeader.biSizeImage     = rb * height;
   bmi->bmiHeader.biXPelsPerMeter = 3780;
   bmi->bmiHeader.biYPelsPerMeter = 3780;
   bmi->bmiHeader.biClrUsed       = 0;
   bmi->bmiHeader.biClrImportant  = 0;

   return bmi;
}


/*
======================================================================
get_sceneinfo()

Get scene settings for playback rate and limited region.

Calculates the subimage rectangle based on whether Limited Region is
active, and calculates the default AVI playback rate based on the
frames per second and step size settings.
====================================================================== */

static void get_sceneinfo( int w, int h, int *fps, RECT *r )
{
   LWSceneInfo *lws;
   int wl, hl;


   *fps = 30;
   r->left   = 0;
   r->top    = 0;
   r->right  = w - 1;
   r->bottom = h - 1;

   if ( lws = gglobal( LWSCENEINFO_GLOBAL, GFUSE_TRANSIENT )) {
      *fps = ( int )( lws->framesPerSecond / fabs( lws->frameStep ));

      if ( lws->renderOpts & LWROPT_LIMITEDREGION ) {
         wl = lws->limitedRegion[ 2 ] - lws->limitedRegion[ 0 ] + 1;
         hl = lws->limitedRegion[ 3 ] - lws->limitedRegion[ 1 ] + 1;
         if ( wl < w || hl < h ) {
            r->left   = lws->limitedRegion[ 0 ];
            r->top    = lws->limitedRegion[ 1 ];
            r->right  = lws->limitedRegion[ 2 ];
            r->bottom = lws->limitedRegion[ 3 ];
         }
      }
   }

   if ( *fps <  1 ) *fps =  1;
   if ( *fps > 60 ) *fps = 60;
}


/*
======================================================================
open_avi()

Open an AVI file and prepare it to receive an image sequence.

The RECT argument defines the size of the frames that will be saved in
the file.  The w and h arguments are the size of the frames that will
be sent to the Write() function, which will differ from the RECT if
Limited Region is turned on.

Returns AVIERR_OK if successful, or one of the error codes defined in
VFW.H.

The call to the VFW function AVIFileOpen() will fail if the filename
extension isn't associated with an appropriate RIFF handler in the
Windows registry.  The error returned is REGDB_E_CLASSNOTREG.  This
error means either that the filename doesn't end in ".avi", or the
registry doesn't contain an association for the ".avi" extension.
====================================================================== */

static HRESULT open_avi( SampleAVI *sa, const char *filename, int w, int h,
   int fps, RECT *rect )
{
   PAVIFILE avifile = NULL;
   PAVISTREAM ps = NULL, pscomp = NULL;
   AVISTREAMINFO streaminfo = { 0 };
   AVICOMPRESSOPTIONS *aopts[ 1 ] = { &sa->opts };
   BITMAPINFO *bmi = NULL;
   FILE *fp;
   HRESULT hr;
   int rowbytes;


   if ( HIWORD( VideoForWindowsVersion() ) < 0x010A )
      return AVIERR_UNSUPPORTED;

   bmi = alloc_bmi( rect->right - rect->left + 1,
                    rect->bottom - rect->top + 1, &rowbytes );
   if ( !bmi ) return AVIERR_MEMORY;

   // VFW doesn't truncate existing files to zero length when OF_CREATE
   // is one of the open flags.  Maybe there's a reason for this, but it
   // doesn't seem kosher.  We'll be aggressive and do it here.

   if ( fp = fopen( filename, "rb" )) {
      fclose( fp );
      remove( filename );
   }

   AVIFileInit();

   hr = AVIFileOpen( &avifile, filename, OF_WRITE | OF_CREATE, NULL );
   if ( hr != AVIERR_OK ) goto Error;

   streaminfo.fccType    = streamtypeVIDEO;
   streaminfo.fccHandler = 0;
   streaminfo.dwScale    = 1;
   streaminfo.dwRate     = fps;
   streaminfo.dwSuggestedBufferSize = bmi->bmiHeader.biSizeImage;

   SetRect( &streaminfo.rcFrame, 0, 0, bmi->bmiHeader.biWidth,
     bmi->bmiHeader.biHeight );

   hr = AVIFileCreateStream( avifile, &ps, &streaminfo );
   if ( hr != AVIERR_OK ) goto Error;

   hr = AVIERR_USERABORT;
   if ( TRUE != AVISaveOptions( hdi->window,
        ICMF_CHOOSE_KEYFRAME | ICMF_CHOOSE_DATARATE,
        1, &ps, ( AVICOMPRESSOPTIONS ** ) &aopts )) goto Error;

   hr = AVIMakeCompressedStream( &pscomp, ps, &sa->opts, NULL );
   if ( hr != AVIERR_OK ) goto Error;

   hr = AVIERR_MEMORY;
   if ( sa->filename = malloc( strlen( filename ) + 1 ))
      strcpy( sa->filename, filename );
   else goto Error;

   sa->avifile  = avifile;
   sa->ps       = ps;
   sa->pscomp   = pscomp;
   sa->bmi      = bmi;
   sa->bits     = ( BYTE * ) sa->bmi->bmiColors;
   sa->r        = *rect;
   sa->fwidth   = w;
   sa->fheight  = h;
   sa->index    = 0;
   sa->rowbytes = rowbytes;

   return AVIERR_OK;

Error:
   if ( bmi ) free( bmi );
   if ( ps ) AVIStreamClose( ps );
   if ( pscomp ) AVIStreamClose( pscomp );
   if ( avifile ) AVIFileClose( avifile );
   AVIFileExit();
   if ( sa->filename ) {
      free( sa->filename );
      sa->filename = NULL;
   }
   return hr;
}


/*
======================================================================
add_frame()

Save a device-independent bitmap as the next frame in an AVI.

Returns AVIERR_OK or one of the error codes in VFW.H.
====================================================================== */

static HRESULT add_frame( SampleAVI *sa )
{
   HRESULT hr;

   if ( sa->index == 0 ) {
      hr = AVIStreamSetFormat( sa->pscomp, 0, sa->bmi,
         sa->bmi->bmiHeader.biSize );
      if ( hr != AVIERR_OK ) return hr;
   }

   hr = AVIStreamWrite( sa->pscomp, sa->index, 1, sa->bits,
      sa->bmi->bmiHeader.biSizeImage, AVIIF_KEYFRAME, NULL, NULL );
   ++sa->index;

   return hr;
}


/*
======================================================================
done()

Finish processing an open AVI.

Closes the AVI and frees associated resources.  If ok is FALSE, the
AVI file is deleted.
====================================================================== */

static void done( SampleAVI *sa, int ok )
{
   if ( sa ) {
      if ( sa->bmi ) {
         free( sa->bmi );
         sa->bmi = NULL;
      }
      if ( sa->ps ) {
         AVIStreamClose( sa->ps );
         sa->ps = NULL;
      }
      if ( sa->pscomp ) {
         AVIStreamClose( sa->pscomp );
         sa->pscomp = NULL;
      }
      if ( sa->avifile ) {
         AVIFileClose( sa->avifile );
         sa->avifile = NULL;
      }
      AVIFileExit();
      if ( sa->filename ) {
         if ( !ok ) remove( sa->filename );
         free( sa->filename );
         sa->filename = NULL;
      }
   }
}


/*
======================================================================
Create()

Standard handler callback.  Create instance data.
====================================================================== */

XCALL_( static LWInstance )
Create( void *priv, void *context, LWError *emsg )
{
   SampleAVI *avi;

   avi = calloc( 1, sizeof( SampleAVI ));
   if ( !avi ) {
      *emsg = "Couldn't allocate instance data.";
      return NULL;
   }

   return ( LWInstance ) avi;
}


/*
======================================================================
Destroy()

Standard handler callback.  Free resources allocated by Create().
====================================================================== */

XCALL_( static void )
Destroy( SampleAVI *avi )
{
   if ( avi ) {
      if ( avi->opts.dwFlags & AVICOMPRESSF_VALID ) {
         AVICOMPRESSOPTIONS *aopts[ 1 ] = { &avi->opts };
         AVISaveOptionsFree( 1, ( AVICOMPRESSOPTIONS ** ) &aopts );
      }
      free( avi );
   }
}


/*
======================================================================
Copy(), Load(), Save(), DescLn(), UseItems(), ChangeID()

Standard handler callbacks.  Our AVI loader doesn't use these, but
they're here in case we want them in the future.
====================================================================== */

XCALL_( static LWError )
Copy( SampleAVI *to, SampleAVI *from ) { return NULL; }

XCALL_( static LWError )
Load( SampleAVI *avi, const LWLoadState *ls ) { return NULL; }

XCALL_( static LWError )
Save( SampleAVI *avi, const LWSaveState *ss ) { return NULL; }

XCALL_( static const char * )
DescLn( SampleAVI *avi ) { return "SampleAVI"; }

XCALL_( static const LWItemID * )
UseItems( SampleAVI *avi ) { return NULL; }

XCALL_( static void )
ChangeID( SampleAVI *avi, const LWItemID *idlist ) {}


/*
======================================================================
Open()

Standard handler callback.  This is called when rendering begins.  We
open and initialize the AVI file.
====================================================================== */

XCALL_( static LWError )
Open( SampleAVI *avi, int w, int h, const char *filename )
{
   RECT rect;
   int fps;
   HRESULT hr;

   XCALL_INIT;

   get_sceneinfo( w, h, &fps, &rect );

   hr = open_avi( avi, filename, w, h, fps, &rect );
   switch ( hr ) {
      case AVIERR_OK:
         break;
      case REGDB_E_CLASSNOTREG:
         return "AVI file type not recognized.  The filename must include "
                "a valid extension (\".avi\", for example).";
      case AVIERR_USERABORT:
         return "AVI saving was cancelled.  No big deal.";
      case AVIERR_MEMORY:
         return "Insufficient memory to initialize the AVI file.";
      case AVIERR_FILEOPEN:
      case AVIERR_FILEWRITE:
         return "A disk error occurred while opening the AVI file.";
      default:
         return "Unable to initialize the AVI file.";
   }

   return NULL;
}


/*
======================================================================
Close()

Standard handler callback.  Called when rendering is finished.  We
close the AVI file.
====================================================================== */

XCALL_( static void )
Close( SampleAVI *avi )
{
   done( avi, 1 );
}


/*
======================================================================
Begin()

Standard handler callback.  Called at the start of each frame.  We
initialize our scanline index.
====================================================================== */

XCALL_( static LWError )
Begin( SampleAVI *avi )
{
   avi->y = 0;
   return NULL;
}


/*
======================================================================
Write()

Standard handler callback.  Called for each scanline of a frame.  We
gather the scanlines until Write() is called for the last one, at
which point we save the frame.

Since we've chosen to respect the Limited Region if it exists, we have
to adjust the channel pointers so that we start reading from them at
the left edge of the LR rectangle, and we ignore scanlines that are
above or below the rectangle.
====================================================================== */

XCALL_( static LWError )
Write( SampleAVI *avi, const unsigned char *r, const unsigned char *g,
   const unsigned char *b, const unsigned char *a )
{
   HRESULT hr;
   unsigned char *p;
   int i;

   if ( avi->y >= avi->r.top && avi->y <= avi->r.bottom ) {
      r += avi->r.left;
      g += avi->r.left;
      b += avi->r.left;
      p = avi->bits + ( avi->r.bottom - avi->y ) * avi->rowbytes;

      for ( i = 0; i < avi->bmi->bmiHeader.biWidth; i++ ) {
         *p++ = *b++;
         *p++ = *g++;
         *p++ = *r++;
      }
   }

   avi->y++;

   if ( avi->y == avi->fheight ) {
      hr = add_frame( avi );
      switch ( hr ) {
         case AVIERR_OK:  break;
         case AVIERR_BADFORMAT:
            return "Image format not supported by the selected AVI compressor.";
         case AVIERR_FILEWRITE:
            return "A disk error occurred while writing to the AVI file.";
         default:
            return "Unable to write AVI frame.";
      }
   }

   return NULL;
}


/*
======================================================================
Handler()

Handler activation function.
====================================================================== */

XCALL_( int )
Handler( long version, GlobalFunc *global, LWAnimSaverHandler *local,
   void *serverData )
{
   if ( version != LWANIMSAVER_VERSION )
      return AFUNC_BADVERSION;

   hdi = global( LWHOSTDISPLAYINFO_GLOBAL, GFUSE_TRANSIENT );
   if ( !hdi ) return AFUNC_BADGLOBAL;

   gglobal = global;

   local->inst->create  = Create;
   local->inst->destroy = Destroy;
   local->inst->copy    = Copy;
   local->inst->load    = Load;
   local->inst->save    = Save;
   local->inst->descln  = DescLn;

   if ( local->item ) {
      local->item->useItems = UseItems;
      local->item->changeID = ChangeID;
   }

   local->type  = LWAST_UBYTE;
   local->open  = Open;
   local->close = Close;
   local->begin = Begin;
   local->write = Write;

   return AFUNC_OK;
}


/*
======================================================================
Options()

Standard handler interface callback.  This is called when the user
selects our plug-in as the animation saver on the Render panel.
====================================================================== */

XCALL_( static LWError )
Options( SampleAVI *avi )
{
   COMPVARS cv;
   int result;

   if ( avi->opts.dwFlags & AVICOMPRESSF_VALID ) {
      cv.fccType    = avi->opts.fccType;
      cv.fccHandler = avi->opts.fccHandler;
      cv.lQ         = avi->opts.dwQuality;
      cv.dwFlags    = ICMF_COMPVARS_VALID;
   }

   cv.cbSize = sizeof( COMPVARS );
   result = ICCompressorChoose( hdi->window, ICMF_CHOOSE_ALLCOMPRESSORS,
      NULL, NULL, &cv, "SampleAVI Options" );

   if ( result ) {
      avi->opts.fccType    = cv.fccType;
      avi->opts.fccHandler = cv.fccHandler;
      avi->opts.dwQuality  = cv.lQ;
      avi->opts.dwFlags   |= AVICOMPRESSF_VALID;
   }

   ICCompressorFree( &cv );
   return NULL;
}


/*
======================================================================
Interface()

Handler interface activation function.
====================================================================== */

XCALL_( int )
Interface( long version, GlobalFunc *global, LWInterface *local,
   void *serverData )
{
   if ( version != LWINTERFACE_VERSION )
      return AFUNC_BADVERSION;

   local->panel   = NULL;
   local->options = Options;
   local->command = NULL;

   return AFUNC_OK;
}


ServerRecord ServerDesc[] = {
   { LWANIMSAVER_HCLASS, "SampleAVI(.avi)", Handler },
   { LWANIMSAVER_ICLASS, "SampleAVI(.avi)", Interface },
   { NULL }
};
