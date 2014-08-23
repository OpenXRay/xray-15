/*
======================================================================
box.c

A Modeler plug-in that makes a box.

Ernie Wright  29 May 01
====================================================================== */

#include <lwserver.h>
#include <lwcmdseq.h>
#include <lwxpanel.h>


int makebox( LWModCommand *local, double *size, double *center,
   int nsegments )
{
   static LWCommandCode ccode = 0;
   DynaValue argv[ 3 ];

   argv[ 0 ].type = DY_VFLOAT;
   argv[ 0 ].fvec.val[ 0 ] = center[ 0 ] - 0.5 * size[ 0 ];
   argv[ 0 ].fvec.val[ 1 ] = center[ 1 ] - 0.5 * size[ 1 ];
   argv[ 0 ].fvec.val[ 2 ] = center[ 2 ] - 0.5 * size[ 2 ];

   argv[ 1 ].type = DY_VFLOAT;
   argv[ 1 ].fvec.val[ 0 ] = center[ 0 ] + 0.5 * size[ 0 ];
   argv[ 1 ].fvec.val[ 1 ] = center[ 1 ] + 0.5 * size[ 1 ];
   argv[ 1 ].fvec.val[ 2 ] = center[ 2 ] + 0.5 * size[ 2 ];

   if ( nsegments > 0 ) {
      argv[ 2 ].type = DY_VINT;
      argv[ 2 ].ivec.val[ 0 ] =
      argv[ 2 ].ivec.val[ 1 ] =
      argv[ 2 ].ivec.val[ 2 ] = nsegments;
   }
   else argv[ 2 ].type = DY_NULL;

   if ( !ccode )
      ccode = local->lookup( local->data, "MAKEBOX" );

   return local->execute( local->data, ccode, 3, argv, 0, NULL );
}


int get_user( LWXPanelFuncs *xpanf, double *size, double *center,
   int *nsegments )
{
   LWXPanelID panel;
   int ok = 0;

   enum { ID_SIZE = 0x8001, ID_CENTER, ID_NSEGMENTS };
   LWXPanelControl ctl[] = {
      { ID_SIZE,      "Size",     "distance3" },
      { ID_CENTER,    "Center",   "distance3" },
      { ID_NSEGMENTS, "Segments", "integer"   },
      { 0 }
   };
   LWXPanelDataDesc cdata[] = {
      { ID_SIZE,      "Size",     "distance3" },
      { ID_CENTER,    "Center",   "distance3" },
      { ID_NSEGMENTS, "Segments", "integer"   },
      { 0 }
   };
   LWXPanelHint hint[] = {
      XpLABEL( 0, "Box Tutorial Part 2" ),
      XpMIN( ID_NSEGMENTS, 1 ),
      XpMAX( ID_NSEGMENTS, 200 ),
      XpDIVADD( ID_SIZE ),
      XpDIVADD( ID_CENTER ),
      XpEND
   };

   panel = xpanf->create( LWXP_FORM, ctl );
   if ( !panel ) return 0;

   xpanf->describe( panel, cdata, NULL, NULL );
   xpanf->hint( panel, 0, hint );
   xpanf->formSet( panel, ID_SIZE, size );
   xpanf->formSet( panel, ID_CENTER, center );
   xpanf->formSet( panel, ID_NSEGMENTS, nsegments );

   ok = xpanf->post( panel );

   if ( ok ) {
      double *d;
      int *i;

      d = xpanf->formGet( panel, ID_SIZE );
      size[ 0 ] = d[ 0 ];
      size[ 1 ] = d[ 1 ];
      size[ 2 ] = d[ 2 ];

      d = xpanf->formGet( panel, ID_CENTER );
      center[ 0 ] = d[ 0 ];
      center[ 1 ] = d[ 1 ];
      center[ 2 ] = d[ 2 ];

      i = xpanf->formGet( panel, ID_NSEGMENTS );
      *nsegments = *i;
   }

   xpanf->destroy( panel );
   return ok;
}


XCALL_( int )
Activate( long version, GlobalFunc *global, LWModCommand *local,
   void *serverData )
{
   LWXPanelFuncs *xpanf;
   double size[ 3 ]   = { 1.0, 1.0, 1.0 };
   double center[ 3 ] = { 0.0, 0.0, 0.0 };
   int nsegments = 1;

   if ( version != LWMODCOMMAND_VERSION )
      return AFUNC_BADVERSION;

   xpanf = global( LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT );
   if ( !xpanf ) return AFUNC_BADGLOBAL;

   if ( get_user( xpanf, size, center, &nsegments ))
      makebox( local, size, center, nsegments );

   return AFUNC_OK;
}


ServerRecord ServerDesc[] = {
   { LWMODCOMMAND_CLASS, "Tutorial_Box2", Activate },
   { NULL }
};
