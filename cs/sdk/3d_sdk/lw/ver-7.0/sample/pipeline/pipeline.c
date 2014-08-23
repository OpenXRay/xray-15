/*
======================================================================
pipeline.c

Custom Startup and Shutdown, trace function and server records.

Ernie Wright  16 Jul 01

The pipeline plug-ins are a set of 18 handler plug-in skeletons that
can be used to trace LightWave's execution during animation and
rendering.  These include two motion handlers (before IK and after
IK) and three displacement handlers (before bones, object and world
coordinates), since all of these are called at different times.

There are three files in the pipeline source directory that help with
running the plug-ins.

   pipeanim.txt  - A fake animation file that the anim loader can
                   pretend to load.  This has to be a file that no
                   other loader in the chain will attempt to load.

   cube.lwo      - An object with the object-specific handlers
                   applied.

   pipeline.lws  - A scene file with the scene-specific handlers
                   applied.  This scene loads pipeanim.txt and
                   cube.lwo and saves rendered "frames" to an
                   animation file called pipeanim.out.

The custom Startup() function opens a text file, and calls to the
trace() function from each of the handler callbacks write text lines
in this file.  The Shutdown() callback closes the file.  To do a quick
test, build pipeline.p, add it in Layout, load pipeline.lws, and F10
a single frame.  Unless you change the filename, the output of the
trace() function will be in a file called pipeline.txt in your current
content directory.

To test the FrameBufferHandler, you'll have to manually set it as your
Render Display in the Render Options panel.
====================================================================== */

#include <lwanimlod.h>
#include <lwanimsav.h>
#include <lwchannel.h>
#include <lwcustobj.h>
#include <lwdisplce.h>
#include <lwenviron.h>
#include <lwframbuf.h>
#include <lwfilter.h>
#include <lwmotion.h>
#include <lwmaster.h>
#include <lwobjrep.h>
#include <lwtexture.h>
#include <lwshader.h>
#include <lwvolume.h>
#include <stdarg.h>
#include "pipeline.h"


static FILE *fp;


void trace( const char *func, const char *plug, const char *fmt, ... )
{
   if ( fp ) {
      if ( func )  fprintf( fp, "%-11.11s", func );
      if ( plug )  fprintf( fp, "%-23.23s", plug + 9 );
      if ( fmt ) {
         va_list ap;
         va_start( ap, fmt );
         vfprintf( fp, fmt, ap );
         va_end( ap );
      }
      fprintf( fp, "\n" );
   }
}


void *Startup( void )
{
   fp = fopen( "pipeline.txt", "w" );
   return fp;
}


void Shutdown( void *serverData )
{
   if ( fp ) fclose( fp );
}


/* prototypes for the activation functions */

XCALL_( int ) AnimLoad( long version, GlobalFunc *global,
   LWAnimLoaderHandler *local, void *serverData );
XCALL_( int ) AnimSave( long version, GlobalFunc *global,
   LWAnimSaverHandler *local, void *serverData );
XCALL_( int ) Channel( long version, GlobalFunc *global,
   LWChannelHandler *local, void *serverData );
XCALL_( int ) CustObject( long version, GlobalFunc *global,
   LWCustomObjHandler *local, void *serverData );
XCALL_( int ) Displace1( long version, GlobalFunc *global,
   LWDisplacementHandler *local, void *serverData );
XCALL_( int ) Displace2( long version, GlobalFunc *global,
   LWDisplacementHandler *local, void *serverData );
XCALL_( int ) Displace3( long version, GlobalFunc *global,
   LWDisplacementHandler *local, void *serverData );
XCALL_( int ) Environment( long version, GlobalFunc *global,
   LWEnvironmentHandler *local, void *serverData );
XCALL_( int ) FrameBuffer( long version, GlobalFunc *global,
   LWFrameBufferHandler *local, void *serverData );
XCALL_( int ) ImageFilter( long version, GlobalFunc *global,
   LWImageFilterHandler *local, void *serverData );
XCALL_( int ) Motion1( long version, GlobalFunc *global,
   LWItemMotionHandler *local, void *serverData );
XCALL_( int ) Motion2( long version, GlobalFunc *global,
   LWItemMotionHandler *local, void *serverData );
XCALL_( int ) Master( long version, GlobalFunc *global,
   LWMasterHandler *local, void *serverData );
XCALL_( int ) ObjRep( long version, GlobalFunc *global,
   LWObjReplacementHandler *local, void *serverData );
XCALL_( int ) PixelFilter( long version, GlobalFunc *global,
   LWPixelFilterHandler *local, void *serverData );
XCALL_( int ) Texture( long version, GlobalFunc *global,
   LWTextureHandler *local, void *serverData );
XCALL_( int ) Shader( long version, GlobalFunc *global,
   LWShaderHandler *local, void *serverData );
XCALL_( int ) Volume( long version, GlobalFunc *global,
   LWVolumetricHandler *local, void *serverData );


/* the server records */

ServerRecord ServerDesc[] = {
   { LWANIMLOADER_HCLASS,     ANLD_PNAME,  AnimLoad    },
   { LWANIMSAVER_HCLASS,      ANSV_PNAME,  AnimSave    },
   { LWCHANNEL_HCLASS,        CHAN_PNAME,  Channel     },
   { LWCUSTOMOBJ_HCLASS,      COBJ_PNAME,  CustObject  },
   { LWDISPLACEMENT_HCLASS,   DISP_PNAME1, Displace1   },
   { LWDISPLACEMENT_HCLASS,   DISP_PNAME2, Displace2   },
   { LWDISPLACEMENT_HCLASS,   DISP_PNAME3, Displace3   },
   { LWENVIRONMENT_HCLASS,    ENVI_PNAME,  Environment },
   { LWFRAMEBUFFER_HCLASS,    FBUF_PNAME,  FrameBuffer },
   { LWIMAGEFILTER_HCLASS,    IFLT_PNAME,  ImageFilter },
   { LWITEMMOTION_HCLASS,     MOTN_PNAME1, Motion1     },
   { LWITEMMOTION_HCLASS,     MOTN_PNAME2, Motion2     },
   { LWMASTER_HCLASS,         MAST_PNAME,  Master      },
   { LWOBJREPLACEMENT_HCLASS, OREP_PNAME,  ObjRep      },
   { LWPIXELFILTER_HCLASS,    PFLT_PNAME,  PixelFilter },
   { LWTEXTURE_HCLASS,        TXTR_PNAME,  Texture     },
   { LWSHADER_HCLASS,         SHAD_PNAME,  Shader      },
   { LWVOLUMETRIC_HCLASS,     VOLU_PNAME,  Volume      },
   { NULL }
};