/*
======================================================================
envtest.c

A plug-in to test the envelope interpolation routine in interp.c.

Ernie Wright  31 Aug 00

This is what I used to test evalEnvelope().  It reads the Position.X
envelope for the camera, then compares what evalEnvelope() returns to
what LWEnvelopeFuncs->evaluate() says.
====================================================================== */

#include <lwserver.h>
#include <lwgeneric.h>
#include <lwrender.h>
#include <lwenvel.h>
#include <lwchannel.h>
#include <lwhost.h>
#include <lwpanel.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "envelope.h"


/* IDs for reading ENVL chunks in read_envb() */

#define ID_ENVL  LWID_('E','N','V','L')
#define ID_PRE   LWID_('P','R','E',' ')
#define ID_POST  LWID_('P','O','S','T')
#define ID_KEY   LWID_('K','E','Y',' ')
#define ID_SPAN  LWID_('S','P','A','N')
#define ID_TCB   LWID_('T','C','B',' ')
#define ID_HERM  LWID_('H','E','R','M')
#define ID_BEZI  LWID_('B','E','Z','I')
#define ID_BEZ2  LWID_('B','E','Z','2')
#define ID_LINE  LWID_('L','I','N','E')
#define ID_STEP  LWID_('S','T','E','P')


/* some globals */

LWMessageFuncs *msgf;
LWChannelInfo *chinfo;
LWEnvelopeFuncs *envf;
LWItemInfo *iteminfo;
LWFileIOFuncs *filef;
LWPanelFuncs *panf;


static int get_globals( GlobalFunc *global )
{
   msgf     = global( LWMESSAGEFUNCS_GLOBAL,  GFUSE_TRANSIENT );
   chinfo   = global( LWCHANNELINFO_GLOBAL,   GFUSE_TRANSIENT );
   envf     = global( LWENVELOPEFUNCS_GLOBAL, GFUSE_TRANSIENT );
   iteminfo = global( LWITEMINFO_GLOBAL,      GFUSE_TRANSIENT );
   filef    = global( LWFILEIOFUNCS_GLOBAL,   GFUSE_TRANSIENT );
   panf     = global( LWPANELFUNCS_GLOBAL,    GFUSE_TRANSIENT );

   return ( msgf && chinfo && envf && iteminfo && filef && panf );
}


#ifdef _WIN32
/*
=====================================================================
revbytes()

Reverses byte order in place.

INPUTS
   bp       bytes to reverse
   elsize   size of the underlying data type
   elcount  number of elements to swap

RESULTS
   Reverses the byte order in each of elcount elements.

IFF files use a byte order variously called "big-endian", "Motorola"
or "Internet order".  So do most systems.  The big exception is
Windows, which is where IFF code needs this function.  An empty macro
is substituted on other systems.

Called by read_envb().
===================================================================== */

void revbytes( void *bp, int elsize, int elcount )
{
   register unsigned char *p, *q;

   p = ( unsigned char * ) bp;

   if ( elsize == 2 ) {
      q = p + 1;
      while ( elcount-- ) {
         *p ^= *q;
         *q ^= *p;
         *p ^= *q;
         p += 2;
         q += 2;
      }
      return;
   }

   while ( elcount-- ) {
      q = p + elsize - 1;
      while ( p < q ) {
         *p ^= *q;
         *q ^= *p;
         *p ^= *q;
         ++p;
         --q;
      }
      p += elsize >> 1;
   }
}
#else
#define revbytes(b,s,c)
#endif


/*
======================================================================
free_env()

Free memory allocated by create_env(), read_enva() and read_envb().
====================================================================== */

static void free_env( Envelope *env )
{
   Key *key, *next;

   key = env->key;
   while ( key ) {
      next = key->next;
      free( key );
      key = next;
   }
   free( env );
}


/*
======================================================================
create_env()

Use the Animation Envelopes global to get the keys of an LWEnvelope
and create our own version.
====================================================================== */

static Envelope *create_env( LWChannelID *chan )
{
   LWChanGroupID group;
   LWEnvelopeID lwenv;
   LWEnvKeyframeID lwkey;
   Envelope *env;
   Key *key, *tail = NULL;
   double val;

   env = calloc( 1, sizeof( Envelope ));
   if ( !env ) return NULL;

   group = chinfo->channelParent( chan );
   lwenv = chinfo->channelEnvelope( chan );
   lwkey = NULL;

   envf->egGet( lwenv, group, LWENVTAG_PREBEHAVE,  &env->behavior[ 0 ] );
   envf->egGet( lwenv, group, LWENVTAG_POSTBEHAVE, &env->behavior[ 1 ] );

   while ( lwkey = envf->nextKey( lwenv, lwkey )) {
      key = calloc( 1, sizeof( Key ));
      if ( !key ) {
         free_env( env );
         return NULL;
      }
      key->prev = tail;
      if ( tail )
         tail->next = key;
      else
         env->key = key;
      tail = key;
      env->nkeys++;

      envf->keyGet( lwenv, lwkey, LWKEY_SHAPE, &key->shape );
      envf->keyGet( lwenv, lwkey, LWKEY_VALUE, &val );
      key->value = ( float ) val;
      envf->keyGet( lwenv, lwkey, LWKEY_TIME, &val );
      key->time = ( float ) val;
      envf->keyGet( lwenv, lwkey, LWKEY_TENSION, &val );
      key->tension = ( float ) val;
      envf->keyGet( lwenv, lwkey, LWKEY_CONTINUITY, &val );
      key->continuity = ( float ) val;
      envf->keyGet( lwenv, lwkey, LWKEY_BIAS, &val );
      key->bias = ( float ) val;
      envf->keyGet( lwenv, lwkey, LWKEY_PARAM_0, &val );
      key->param[ 0 ] = ( float ) val;
      envf->keyGet( lwenv, lwkey, LWKEY_PARAM_1, &val );
      key->param[ 1 ] = ( float ) val;
      envf->keyGet( lwenv, lwkey, LWKEY_PARAM_2, &val );
      key->param[ 2 ] = ( float ) val;
      envf->keyGet( lwenv, lwkey, LWKEY_PARAM_3, &val );
      key->param[ 3 ] = ( float ) val;
   }

   return env;
}


/*
======================================================================
read_envb()

Save the envelope using an LWIO_BINARY LWSaveState, then read it from
the resulting file.
====================================================================== */

static Envelope *read_envb( LWChannelID *chan )
{
   LWChanGroupID group;
   LWEnvelopeID lwenv;
   LWSaveState *ss;
   LWError err;
   FILE *fp;
   Envelope *env;
   Key *key, *tail = NULL;
   float f[ 4 ];
   unsigned int id;
   unsigned short esz, sz, w;
   int i, nparams;


   group = chinfo->channelParent( chan );
   lwenv = chinfo->channelEnvelope( chan );

   /* save the envelope to a file */

   ss = filef->openSave( "test.env", LWIO_BINARY );
   if ( !ss ) return NULL;
   err = envf->save( lwenv, ss );
   filef->closeSave( ss );

   if ( err ) {
      msgf->info( "Error while saving envelope:", err );
      return NULL;
   }

   /* open the file */

   fp = fopen( "test.env", "rb" );
   if ( !fp ) {
      msgf->info( "Couldn't open test.env", NULL );
      return NULL;
   }

   fread( &id, 4, 1, fp );
   revbytes( &id, 4, 1 );
   fread( &esz, 2, 1, fp );
   revbytes( &esz, 2, 1 );

   if ( id != ID_ENVL ) {
      fclose( fp );
      return NULL;
   }

   /* allocate the Envelope structure */

   env = calloc( 1, sizeof( Envelope ));
   if ( !env ) goto Fail;

   /* 2-byte index */

   fseek( fp, 2, SEEK_CUR );

   /* first subchunk header */

   fread( &id, 4, 1, fp );
   revbytes( &id, 4, 1 );
   fread( &sz, 2, 1, fp );
   revbytes( &sz, 2, 1 );

   /* process subchunks as they're encountered */

   while ( 1 ) {
      sz += sz & 1;
      switch ( id ) {
         case ID_PRE:
            if ( sz < 2 ) goto Fail;
            fread( &w, 2, 1, fp );
            revbytes( &w, 2, 1 );
            env->behavior[ 0 ] = w;
            if ( sz > 2 ) fseek( fp, sz - 2, SEEK_CUR );
            break;

         case ID_POST:
            if ( sz < 2 ) goto Fail;
            fread( &w, 2, 1, fp );
            revbytes( &w, 2, 1 );
            env->behavior[ 1 ] = w;
            if ( sz > 2 ) fseek( fp, sz - 2, SEEK_CUR );
            break;

         case ID_KEY:
            if ( sz < 8 ) goto Fail;
            key = calloc( 1, sizeof( Key ));
            if ( !key ) goto Fail;
            key->prev = tail;
            if ( tail )
               tail->next = key;
            else
               env->key = key;
            tail = key;
            env->nkeys++;

            fread( f, 8, 1, fp );
            revbytes( f, 4, 2 );
            key->time = f[ 0 ];
            key->value = f[ 1 ];
            if ( sz > 8 ) fseek( fp, sz - 8, SEEK_CUR );
            break;

         case ID_SPAN:
            if ( !key ) goto Fail;
            if ( sz < 4 ) goto Fail;
            fread( &id, 4, 1, fp );
            revbytes( &id, 4, 1 );

            nparams = ( sz - 4 ) / 4;
            if ( nparams > 4 ) nparams = 4;
            if ( nparams ) {
               fread( f, 4, nparams, fp );
               revbytes( f, 4, nparams );
            }
            i = 4 * ( nparams + 1 );
            if ( sz > i ) fseek( fp, sz - i, SEEK_CUR );

            switch ( id ) {
               case ID_TCB:
                  if ( nparams < 3 ) goto Fail;
                  key->shape = SHAPE_TCB;
                  key->tension = f[ 0 ];
                  key->continuity = f[ 1 ];
                  key->bias = f[ 2 ];
                  break;

               case ID_BEZ2:
                  key->shape = SHAPE_BEZ2;
                  for ( i = 0; i < nparams; i++ )
                     key->param[ i ] = f[ i ];
                  break;

               case ID_BEZI:
                  key->shape = SHAPE_BEZI;
                  for ( i = 0; i < nparams; i++ )
                     key->param[ i ] = f[ i ];
                  break;

               case ID_HERM:
                  key->shape = SHAPE_HERM;
                  for ( i = 0; i < nparams; i++ )
                     key->param[ i ] = f[ i ];
                  break;

               case ID_LINE:
                  key->shape = SHAPE_LINE;
                  break;

               case ID_STEP:
                  key->shape = SHAPE_STEP;
                  break;

               default:
                  break;
            }
            break;

         default:
            fseek( fp, sz, SEEK_CUR );
      }

      /* end of the ENVL chunk? */

      if ( esz <= ftell( fp ) - 8 )
         break;

      /* get the next chunk header */

      fread( &id, 4, 1, fp );
      revbytes( &id, 4, 1 );
      fread( &sz, 2, 1, fp );
      revbytes( &sz, 2, 1 );
   }

   fclose( fp );
   return env;

Fail:
   if ( fp ) fclose( fp );
   if ( env ) free_env( env );
   return NULL;
}


/*
======================================================================
read_enva()

Save the envelope using an LWIO_ASCII LWSaveState, then read it from
the resulting file.
====================================================================== */

static Envelope *read_enva( LWChannelID *chan )
{
   LWChanGroupID group;
   LWEnvelopeID lwenv;
   LWSaveState *ss;
   LWError err;
   FILE *fp;
   Envelope *env;
   Key *key, *tail = NULL;
   char *p, buf[ 256 ], sep[] = " \t\r\n";
   float f[ 9 ];
   int i, j;


   group = chinfo->channelParent( chan );
   lwenv = chinfo->channelEnvelope( chan );

   /* save the envelope to a file */

   ss = filef->openSave( "test.mot", LWIO_ASCII );
   if ( !ss ) return NULL;
   err = envf->save( lwenv, ss );
   filef->closeSave( ss );
   if ( err ) {
      msgf->info( "Error while saving envelope:", err );
      return NULL;
   }

   /* open the file */

   fp = fopen( "test.mot", "r" );
   if ( !fp ) {
      msgf->info( "Couldn't open test.mot", NULL );
      return NULL;
   }

   /* allocate the Envelope structure */

   env = calloc( 1, sizeof( Envelope ));
   if ( !env ) goto Fail;

   /* first line should be "{ Envelope" */

   p = fgets( buf, sizeof( buf ), fp );
   if ( !p || feof( fp )) goto Fail;

   p = strtok( buf, sep );
   if ( !p || p[ 0 ] != '{' ) goto Fail;

   p = strtok( NULL, sep );
   if ( !p || strcmp( p, "Envelope" )) goto Fail;

   /* second line should be number of keys */

   p = fgets( buf, sizeof( buf ), fp );
   if ( !p || feof( fp )) goto Fail;

   p = strtok( buf, sep );
   if ( !p ) goto Fail;
   env->nkeys = atoi( p );

   /* next nkeys lines should contain keys */

   for ( i = 0; i < env->nkeys; i++ ) {
      key = calloc( 1, sizeof( Key ));
      if ( !key ) goto Fail;
      key->prev = tail;
      if ( tail )
         tail->next = key;
      else
         env->key = key;
      tail = key;

      p = fgets( buf, sizeof( buf ), fp );
      if ( !p || feof( fp )) goto Fail;

      p = strtok( buf, sep );
      if ( !p || strcmp( p, "Key" )) goto Fail;

      for ( j = 0; j < 9; j++ ) {
         p = strtok( NULL, sep );
         if ( !p ) goto Fail;
         f[ j ] = ( float ) atof( p );
      }

      key->value = f[ 0 ];
      key->time  = f[ 1 ];
      key->shape = ( int ) f[ 2 ];

      if ( key->shape == SHAPE_TCB ) {
         key->tension    = f[ 3 ];
         key->continuity = f[ 4 ];
         key->bias       = f[ 5 ];
      }

      if ( key->shape == SHAPE_BEZ2 ) {
         key->param[ 0 ] = f[ 3 ];
         key->param[ 1 ] = f[ 4 ];
         key->param[ 2 ] = f[ 5 ];
         key->param[ 3 ] = f[ 6 ];
      }
      else {
         key->param[ 0 ] = f[ 6 ];
         key->param[ 1 ] = f[ 7 ];
      }
   }

   /* next line should be "Behaviors <pre> <post>" */

   p = fgets( buf, sizeof( buf ), fp );
   if ( !p || feof( fp )) goto Fail;

   p = strtok( buf, sep );
   if ( !p || strcmp( p, "Behaviors" )) goto Fail;

   p = strtok( NULL, sep );
   if ( !p ) goto Fail;
   env->behavior[ 0 ] = atoi( p );

   p = strtok( NULL, sep );
   if ( !p ) goto Fail;
   env->behavior[ 1 ] = atoi( p );

   fclose( fp );
   return env;

Fail:
   if ( fp ) fclose( fp );
   if ( env ) free_env( env );
   return NULL;
}


static int write_mot57( Envelope *env, char *filename )
{
   FILE *fp;
   Key *key;
   int i;

   fp = fopen( filename, "w" );
   if ( !fp ) return 0;

   fprintf( fp, "LWMO\n2\n9\n  %d\n", env->nkeys );
   key = env->key;
   for ( i = 0; i < env->nkeys; i++ ) {
      fprintf( fp, "  %g %g 2 %g %g %g\n", key->value, key->time, key->tension,
         key->continuity, key->bias );
      key = key->next;
   }
   for ( i = 2; i < 9; i++ )
      fprintf( fp, "  1\n  0 0.000000 2 0 0 0\n" );
   fclose( fp );
   return 1;
}


/*
======================================================================
test_eval()

See whether our envelope evaluation function gets the same answer as
LW's built-in one.
====================================================================== */

#define TEST_OUT "envtest.txt"

static void test_eval( LWChannelID chan )
{
   static char ftype[] = "GBA";
   static char stype[] = "THBLSZ";
   static char sep1[] = "------------+-----------+---+-----------+-----------+-----------+-----------+-----------+-----------+-----------";
   static char sep2[] = "-----------+-----------+----------------------+----------------------+----------------------";

   LWEnvelopeID lwenv;
   Envelope *env[ 3 ];
   Key *key[ 3 ];
   FILE *fp;
   float lo, hi, tlen, t, tp, a, b;
   int i, j;


   env[ 0 ] = create_env( chan );
   env[ 1 ] = read_envb( chan );
   env[ 2 ] = read_enva( chan );
   lwenv = chinfo->channelEnvelope( chan );

   if ( !env[ 0 ] || !env[ 1 ] || !env[ 2 ] || !lwenv ) {
      msgf->error( "Couldn't get the envelope.", NULL );
      return;
   }

   write_mot57( env[ 1 ], "test57.mot" );

   fp = fopen( TEST_OUT, "w" );
   if ( !fp ) {
      msgf->error( "Couldn't open " TEST_OUT, NULL );
      return;
   }

   fprintf( fp,
      "(G)lobal:  values from LWEnvelopeFuncs->keyGet()\n"
      "(B)inary:  values from LWEnvelopeFuncs->save() LWIO_BINARY\n"
      "(A)SCII:   values from LWEnvelopeFuncs->save() LWIO_ASCII\n\n" );

   for ( j = 0; j < 3; j++ ) {
      fprintf( fp, "%c   %d keys   Pre %d  Post %d\n", ftype[ j ],
         env[ j ]->nkeys, env[ j ]->behavior[ 0 ], env[ j ]->behavior[ 1 ] );
      key[ j ] = env[ j ]->key;
   }
   fprintf( fp, "\n" );

   fprintf( fp, "%s\n", sep1 );
   fprintf( fp, "        Time|      Value|Typ|    Tension| Continuity|       Bias|    Param 0|    Param 1|    Param 2|    Param 3\n" );
   fprintf( fp, "%s\n", sep1 );

   for ( i = 0; i < env[ 0 ]->nkeys; i++ ) {
      for ( j = 0; j < 3; j++ ) {
         fprintf( fp, "%c%11.6f|%11.6f| %c |%11.6f|%11.6f|%11.6f|%11.6f|%11.6f|%11.6f|%11.6f\n",
            ftype[ j ],
            key[ j ]->time,
            key[ j ]->value,
            stype[ key[ j ]->shape ],
            key[ j ]->tension,
            key[ j ]->continuity,
            key[ j ]->bias,
            key[ j ]->param[ 0 ],
            key[ j ]->param[ 1 ],
            key[ j ]->param[ 2 ],
            key[ j ]->param[ 3 ] );
         key[ j ] = key[ j ]->next;
      }
      fprintf( fp, "%s\n", sep1 );
   }
   fprintf( fp, "\n\n" );

   key[ 0 ] = env[ 0 ]->key;
   lo = key[ 0 ]->time;
   while ( key[ 0 ]->next ) key[ 0 ] = key[ 0 ]->next;
   hi = key[ 0 ]->time;
   tlen = hi - lo;
   lo -= tlen * 2;
   hi += tlen * 2;
//   tlen *= 4.9f;
   tlen = hi - lo;

   fprintf( fp, "%s\n", sep2 );
   fprintf( fp, "       Time|      Value|    G Value      Error|    B Value      Error|    A Value      Error\n" );
   fprintf( fp, "%s\n", sep2 );

   t = lo;
   for ( i = 0; i < 100; i++ ) {
      a = ( float ) envf->evaluate( lwenv, ( double ) t );
      fprintf( fp, "%11.6f|%11.6f", t, a );
      for ( j = 0; j < 3; j++ ) {
         b = evalEnvelope( env[ j ], t );
         fprintf( fp, "|%11.6f%11.6f", b, b - a );
      }
      fprintf( fp, "\n" );

      tp = t;
      t = lo + tlen * ( i + 1 ) / 100.0f;
      if ( tp < env[ 0 ]->key->time && t >= env[ 0 ]->key->time )
         fprintf( fp, "%s\n", sep2 );
      if ( tp < key[ 0 ]->time && t >= key[ 0 ]->time )
         fprintf( fp, "%s\n", sep2 );
   }

   fclose( fp );
   for ( j = 0; j < 3; j++ )
      if ( env[ j ] ) free_env( env[ j ] );
}


/*
======================================================================
EnvTest()

The activation function.
====================================================================== */

XCALL_( int )
EnvTest( long version, GlobalFunc *global, LWLayoutGeneric *local,
   void *serverData )
{
   LWItemID id;
   LWChanGroupID group;
   LWChannelID chan;

   if ( version != LWLAYOUTGENERIC_VERSION ) return AFUNC_BADVERSION;

   if ( !get_globals( global ))
      return AFUNC_BADGLOBAL;

   id = iteminfo->first( LWI_CAMERA, NULL );
   group = iteminfo->chanGroup( id );
   chan = chinfo->nextChannel( group, NULL );
   while ( chan ) {
      if ( !strcmp( chinfo->channelName( chan ), "Position.X" )) break;
      chan = chinfo->nextChannel( group, chan );
   }

   if ( !chan ) {
      msgf->info( "Couldn't find Position.X", NULL );
      return AFUNC_OK;
   }

   test_eval( chan );

   return AFUNC_OK;
}


typedef struct {
   LWChannelID chan;
   Envelope *env;
} ETInst;


XCALL_( static LWInstance )
Create( void *data, LWChannelID chan, LWError *err )
{
   ETInst *inst;

   if ( inst = calloc( 1, sizeof( ETInst ))) {
      inst->chan = chan;
      inst->env = create_env( chan );
   }
   else
      *err = "Instance allocation failed.";

   return inst;
}


XCALL_( static void )
Destroy( ETInst *inst )
{
   if ( inst ) {
      if ( inst->env ) free_env( inst->env );
      free( inst );
   }
}


XCALL_( static LWError ) Copy( ETInst *to, ETInst *from )
{
   to->chan = from->chan;
   to->env = create_env( from->chan );
   return NULL;
}


XCALL_( static LWError )
Load( void *dat, const LWLoadState *lState ) { return NULL; }

XCALL_( static LWError )
Save( void *dat, const LWSaveState *sState ) { return NULL; }

XCALL_( static const char * )
Describe( void *dat ) { return "Envelope Interpolation Tester"; }

XCALL_( static unsigned int )
Flags ( void *dat ) { return 0; }


XCALL_( static void )
ChanEval( ETInst *inst, const LWChannelAccess *ca )
{
   ca->setChannel( ca->chan,
      ( double ) evalEnvelope( inst->env, ( float ) ca->time ));
}


XCALL_( static int )
TestEnvChan( long version, GlobalFunc *global, LWChannelHandler *local,
   void *serverData )
{
   if ( version != LWCHANNEL_VERSION )
      return AFUNC_BADVERSION;

   if ( !get_globals( global ))
      return AFUNC_BADGLOBAL;

   local->inst->create  = Create;
   local->inst->destroy = Destroy;
   local->inst->copy    = Copy;
   local->inst->load    = Load;
   local->inst->save    = Save;
   local->inst->descln  = Describe;
   local->flags         = Flags;
   local->evaluate      = ChanEval;

   return AFUNC_OK;
}


XCALL_( static LWError )
sync_env( ETInst *inst )
{
   if ( inst->env ) free_env( inst->env );
   inst->env = create_env( inst->chan );
   return NULL;
}


XCALL_( static int )
TestEnvChanInterface( long version, GlobalFunc *global, LWInterface *lwi,
   void *serverData )
{
   if ( version != LWINTERFACE_VERSION )
      return AFUNC_BADVERSION;

   lwi->panel   = NULL;
   lwi->options = sync_env;
   lwi->command = NULL;

   return AFUNC_OK;
}


ServerRecord ServerDesc[] = {
   { LWLAYOUTGENERIC_CLASS, "EnvInterpolateTest", EnvTest },
   { LWCHANNEL_HCLASS, "EnvInterpChanTest", TestEnvChan },
   { LWCHANNEL_ICLASS, "EnvInterpChanTest", TestEnvChanInterface },
   { NULL }
};
