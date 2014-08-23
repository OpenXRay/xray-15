#include <lwserver.h>
#include <lwmotion.h>
#include <lwxpanel.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef PI
#define PI    3.1415926535897932384626433
#define TWOPI 6.2831853071795864769252868
#endif

#define EPSILON 1e-10


typedef struct {
   double      a, b;       // semimajor and semiminor axes
   double      period;     // orbital period, in seconds
} Orbit;


static LWInstUpdate *lwupdate = NULL;


/*
======================================================================
kepler()

Solve Kepler's orbit equation for a given ellipse.  From Peter
Duffett-Smith, ASTRONOMY WITH YOUR PERSONAL COMPUTER 2nd ed.,
Cambridge University Press, 1990, ISBN 0-521-38093-6.

   a        semimajor axis
   b        semiminor axis
   t        time, expressed as number of revolutions
   x        storage for position coordinate
   y        storage for position coordinate
====================================================================== */

static void kepler( double a, double b, double t, double *x, double *y )
{
   double m, d,
      ec,      // eccentricity
      am,      // mean anomaly
      ae,      // eccentric anomaly
      at;      // true anomaly

   ec = sqrt( a * a - b * b ) / a;
   am = TWOPI * t;
   m = am - TWOPI * ( int )( am / TWOPI );
   ae = m;

   while ( 1 ) {
      d = ae - ( ec * sin( ae )) - m;
      if ( fabs( d ) < EPSILON ) break;
      d = d / ( 1.0 - ( ec * cos( ae )));
      ae = ae - d;
   }

   at = 2.0 * atan( sqrt(( 1.0 + ec ) / ( 1.0 - ec )) * tan( ae / 2.0 ));
   *x = a * cos( at ) - ( ec * a );
   *y = b * sin( at );
}


/*
======================================================================
Create()

Handler callback.  Allocate and initialize instance data.
====================================================================== */

XCALL_( static LWInstance )
Create( void *priv, LWItemID item, LWError *err )
{
   Orbit *inst;

   if ( inst = calloc( 1, sizeof( Orbit ))) {
      inst->a = inst->b = 1.0;
      inst->period = 5.0;
   }
   else
      *err = "Couldn't allocate instance data.";

   return inst;
}


/*
======================================================================
Destroy()

Handler callback.  Free resources allocated by Create().
====================================================================== */

XCALL_( static void )
Destroy( Orbit *inst )
{
   free( inst );
}


/*
======================================================================
Copy()

Handler callback.  Copy instance data.
====================================================================== */

XCALL_( static LWError )
Copy( Orbit *to, Orbit *from )
{
   *to = *from;
   return NULL;
}


/*
======================================================================
Load()

Handler callback.  Read instance data.
====================================================================== */

XCALL_( static LWError )
Load( Orbit *inst, const LWLoadState *ls )
{
   float ab[ 2 ];

   LWLOAD_FP( ls, ab, 2 );
   inst->a = ab[ 0 ];
   inst->b = ab[ 1 ];
   return NULL;
}


/*
======================================================================
Save()

Handler callback.  Write instance data.
====================================================================== */

XCALL_( static LWError )
Save( Orbit *inst, const LWSaveState *ss )
{
   float ab[ 2 ];

   ab[ 0 ] = ( float ) inst->a;
   ab[ 1 ] = ( float ) inst->b;
   LWSAVE_FP( ss, ab, 2 );
   return NULL;
}


/*
======================================================================
Describe()

Handler callback.  Write a short, human-readable string describing
the instance data.
====================================================================== */

XCALL_( static const char * )
Describe( Orbit *inst )
{
   static char desc[ 80 ];

   sprintf( desc, "Kepler a = %g, b = %g", inst->a, inst->b );
   return desc;
}


/*
======================================================================
Flags()

Handler callback.
====================================================================== */

XCALL_( static int )
Flags( Orbit *inst )
{
   return 0;
}


/*
======================================================================
Evaluate()

Handler callback.  This is where we can modify the item's motion.
====================================================================== */

XCALL_( static void )
Evaluate( Orbit *inst, const LWItemMotionAccess *access )
{
   LWDVector pos = { 0.0 };
   double x, z;

   kepler( inst->a, inst->b, access->time / inst->period, &x, &z );

   access->getParam( LWIP_POSITION, access->time, pos );
   pos[ 0 ] += x;
   pos[ 2 ] += z;
   access->setParam( LWIP_POSITION, pos );
}


/*
======================================================================
Handler()

Handler activation function.  Check the version and fill in the
callback fields of the handler structure.
====================================================================== */

XCALL_( static int )
Handler( long version, GlobalFunc *global, LWItemMotionHandler *local,
   void *serverData)
{
   if ( version != LWITEMMOTION_VERSION ) return AFUNC_BADVERSION;

   local->inst->create  = Create;
   local->inst->destroy = Destroy;
   local->inst->load    = Load;
   local->inst->save    = Save;
   local->inst->copy    = Copy;
   local->inst->descln  = Describe;
   local->evaluate      = Evaluate;
   local->flags         = Flags;

   return AFUNC_OK;
}


/* interface stuff --------------------------------------------------- */

static LWXPanelFuncs *xpanf;

enum { ID_ELA = 0x8001, ID_ELB, ID_PERIOD };


/*
======================================================================
xgetval()

Xpanels callback for LWXP_VIEW panels.  Returns a pointer to the data
for a given control value.
====================================================================== */

void *xgetval( Orbit *inst, unsigned long vid )
{
   switch ( vid ) {
      case ID_ELA:     return &inst->a;
      case ID_ELB:     return &inst->b;
      case ID_PERIOD:  return &inst->period;
      default:         return NULL;
   }
}


/*
======================================================================
xsetval()

Xpanels callback for LWXP_VIEW panels.  Store a value in our instance
data.
====================================================================== */

int xsetval( Orbit *inst, unsigned long vid, void *value )
{
   double d = *(( double * ) value );

   switch ( vid ) {
      case ID_ELA:     inst->a = d;       break;
      case ID_ELB:     inst->b = d;       break;
      case ID_PERIOD:  inst->period = d;  break;
      default:         return 0;
   }

   if ( lwupdate ) lwupdate( LWITEMMOTION_HCLASS, inst );
   return 1;
}


/*
======================================================================
get_panel()

Create and initialize an LWXP_VIEW panel.  Called by Interface().
====================================================================== */

LWXPanelID get_panel( Orbit *inst )
{
   static LWXPanelControl xctl[] = {
      { ID_ELA,    "Semimajor axis",   "float" },
      { ID_ELB,    "Semiminor axis",   "float" },
      { ID_PERIOD, "Period (seconds)", "float" },
      { 0 }
   };
   static LWXPanelDataDesc xdata[] = {
      { ID_ELA,    "", "float" },
      { ID_ELB,    "", "float" },
      { ID_PERIOD, "", "float" },
      { 0 }
   };
   static LWXPanelHint xhint[] = {
      XpLABEL( 0, "Kepler Orbit"),
      XpEND
   };

   LWXPanelID xpanel;


   xpanel = xpanf->create( LWXP_VIEW, xctl );
   if ( !xpanel ) return NULL;

   xpanf->hint( xpanel, 0, xhint );
   xpanf->describe( xpanel, xdata, xgetval, xsetval );
   xpanf->viewInst( xpanel, inst );
   xpanf->setData( xpanel, 0, inst );

   return xpanel;
}


/*
======================================================================
Interface()

The interface activation function.
====================================================================== */

XCALL_( int )
Interface( long version, GlobalFunc *global, LWInterface *local,
   void *serverData )
{
   if ( version != LWINTERFACE_VERSION ) return AFUNC_BADVERSION;

   xpanf = global( LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT );
   if ( !xpanf ) return AFUNC_BADGLOBAL;

   lwupdate = global( LWINSTUPDATE_GLOBAL, GFUSE_TRANSIENT );

   local->panel   = get_panel( local->inst );
   local->options = NULL;
   local->command = NULL;

   return local->panel ? AFUNC_OK : AFUNC_BADGLOBAL;
}


/*
======================================================================
The server description.
====================================================================== */

ServerRecord ServerDesc[] = {
   { LWITEMMOTION_HCLASS, "Demo_Kepler", Handler },
   { LWITEMMOTION_ICLASS, "Demo_Kepler", Interface },
   { NULL }
};
