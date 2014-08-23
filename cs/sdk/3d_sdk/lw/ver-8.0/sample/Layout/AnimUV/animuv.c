/************************************************************************
*                                                                       *
*       Anim UV.c:                                                      *
*                                                                       *
*       Contains The Code For The Animation UV Plugin.                  *
*                                                                       *
*       08.July 2003 Jamie Lisa Finch.                                  *
*                                                                       *
************************************************************************/

#include <lwsdk/lwserver.h>
#include <lwsdk/lwanimuv.h>
#include <lwsdk/lwpanel.h>
#include <lwsdk/lwtxtr.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Instance Structure. */

typedef struct st_animuv {

  /* Program Options. */

  double     uChangeRate;
  double     vChangeRate;

  /* Program Data. */

  LWFrame    FrameStart;
  LWFrame    FrameEnd;
  double     FramesPerSecond;
  double     Time;
  int        NumberOfVertices;
  int        wRepeat;
  int        hRepeat;
  LWControl *uChangeRateControl;
  LWControl *vChangeRateControl;
  } ANIMUV;

GlobalFunc *GlobalFunction     = (GlobalFunc *) NULL;

char       PluginName[]        = { "Animation UV Sample Plugin" };
char       PluginUser[]        = { "Animation UV Cycler" };
LWError    NotEnoughMemory     = { "Not Enough Memory" };
const char PluginDescription[] = { "Animation UV Sample Plugin"};
char       uChangeRate[]       = { "u Cycle Rate Per Second" };
char       vChangeRate[]       = { "v Cycle Rate Per Second" };


/************************************************************************
*                                                                       *
*       Log:                                                            *
*                                                                       *
*       Writes A Log Of Messages Into A Debug Device.                   *
*                                                                       *
*       Input:                                                          *
*                                                                       *
*       Func    = Address Of String Representing The Function.          *
*       Fmt     = Address Of String Representing The sprintf() Format.  *
*                                                                       *
*       Output:                                                         *
*                                                                       *
*       The Debug Messages Are Written To Log Device.                   *
*                                                                       *
*       11.July 2003 Jamie Lisa Finch.                                  *
*                                                                       *
************************************************************************/

/* Debug Version. */

//#ifndef RELEASE
#if( 0 )
  #ifdef _MSWIN

    #include <windows.h>

    void __cdecl Log( char *Func, char *Fmt, ... ) {

      int i;
      va_list va;
      char Msg[ 512 ];

      memset( Msg, ' ', sizeof( Msg ) - 1 );
      Msg[ sizeof( Msg ) - 1 ] = 0;

      for( i = 0; i < 30; i++ ) {
        if( *Func != 0 ) Msg[ i ] = *Func++;
        else break;
        }
      Msg[ 30 ] = ':';
      Msg[ 31 ] = ' ';

      va_start( va, Fmt );
      vsprintf( &Msg[ 32 ], Fmt, va );
      va_end(   va );

      i = strlen( Msg );
      Msg[ i     ] = '\n';
      Msg[ i + 1 ] = 0;

      OutputDebugStringA( Msg );
      }

  /* Mac Version. */

  #else
    void __cdecl Log( char *Func, char *Fmt, ... ) {
    }
  #endif

/* Release Version. */

#else

  #ifdef _MSWIN
    void __cdecl Log( char *Func, char *Fmt, ... ) {
      }
  #endif
  
  #ifdef _MACOS
    void Log( char *Func, char *Fmt, ... ) {
      }
  #endif
  
#endif


/************************************************************************
*                                                                       *
*       ByteOrderMotorola:                                              *
*                                                                       *
*       Assures The Byte Order Of The Data Is Correct.                  *
*                                                                       *
*       Input:                                                          *
*                                                                       *
*       Dest    = Destination Address Of Data In Machine Native Format. *
*       Sorc    = Source      Address Of Data In Motorola Format.       *
*       Length  = Number Of Bytes To Copy.                              *
*                                                                       *
*       Output:                                                         *
*                                                                       *
*       Copies A Packet Of Bytes Making Sure For Intel The Byte Order   *
*       Get Fliped Over And For Mac It Remains The Same.                *
*                                                                       *
*       11.July 2003 Jamie Lisa Finch.                                  *
*                                                                       *
************************************************************************/

#ifdef _MSWIN

#define ByteOrderMotorola( Dest, Sorc, Length ) \
  {                                             \
    int  Len;                                   \
    char *pDest, *pSorc;                        \
    Len   = Length;                             \
    pDest = (char *) Dest;                      \
    pSorc = (char *) Sorc;                      \
    for( pSorc += Len; Len > 0; --Len ) {       \
      --pSorc;                                  \
      *pDest++ = *pSorc;                        \
      }                                         \
    }

#else

#define ByteOrderMotorola( Dest, Sorc, Length ) \
  memcpy( Dest, Sorc, Length );
  #endif


/************************************************************************
*                                                                       *
*       Instance Calls:                                                 *
*                                                                       *
*       These Are The Instance Calls That All Plugins Must Handle.      *
*                                                                       *
*       17.July 2003 Jamie Lisa Finch.                                  *
*                                                                       *
************************************************************************/


/************************************************************************
*                                                                       *
*       AnimUVCreate:                                                   *
*                                                                       *
*       Creates The Structure And Initilized The Data.                  *
*                                                                       *
*       Input:                                                          *
*                                                                       *
*       Priv    = Address Of Private Data.                              *
*       Id      = LightWave Item Id.                                    *
*       Error   = Address Of LightWave Error Message.                   *
*                                                                       *
*       Output:                                                         *
*                                                                       *
*       The AnimUV Structure Is Created And Initilized.                 *
*                                                                       *
*       11.July 2003 Jamie Lisa Finch.                                  *
*                                                                       *
************************************************************************/

XCALL_ (LWInstance) AnimUVCreate( void *Priv, LWItemID Id, LWError *Error ) {

  ANIMUV      *AnimUV;
  LWSceneInfo *SceneInfo;

  XCALL_INIT;

//  Log( "AnimUVCreate", "Entered" );

  if( AnimUV = malloc( sizeof( ANIMUV ) ) ) {
    memset( AnimUV, 0, sizeof( ANIMUV ) );

//    Log( "AnimUVCreate", "Allocated Memory" );

    /* Set Some Default Values. */

    AnimUV->uChangeRate        = 1.0;
    AnimUV->vChangeRate        = 1.0;
    AnimUV->FrameStart         = 1;
    AnimUV->FrameEnd           = 1;
    AnimUV->FramesPerSecond    = 30;
    AnimUV->Time               = 0;
    AnimUV->NumberOfVertices   = 0;
    AnimUV->wRepeat            = TXRPT_RESET;
    AnimUV->hRepeat            = TXRPT_RESET;
    AnimUV->uChangeRateControl = (LWControl *) NULL;
    AnimUV->vChangeRateControl = (LWControl *) NULL;

    /* Read Data From Scene Info. */

    if( SceneInfo = (GlobalFunction)( LWSCENEINFO_GLOBAL, GFUSE_TRANSIENT ) ) {
      AnimUV->FrameStart      = SceneInfo->frameStart;       
      AnimUV->FrameEnd        = SceneInfo->frameEnd;   
      AnimUV->FramesPerSecond = SceneInfo->framesPerSecond;   
      }
    }
  else *Error = NotEnoughMemory;

//  Log( "AnimUVCreate", "Exit" );

  return (void *) AnimUV;
  }


/************************************************************************
*                                                                       *
*       AnimUVDestroy:                                                  *
*                                                                       *
*       Removes The Instance Of The Anim UV.                            *
*                                                                       *
*       Input:                                                          *
*                                                                       *
*       AnimUV  = Address Of Anim UV Structure.                         *
*                                                                       *
*       Output:                                                         *
*                                                                       *
*       All Data Associated With The Anim UV Structure Is Freed.        *
*                                                                       *
*       11.July 2003 Jamie Lisa Finch.                                  *
*                                                                       *
************************************************************************/

XCALL_ (void) AnimUVDestroy( void *vAnimUV ) {

  ANIMUV *AnimUV;

  XCALL_INIT;

//  Log( "AnimUVDestroy", "Entered" );

  AnimUV = (ANIMUV *) vAnimUV;
  if( AnimUV != (ANIMUV *) NULL ) {
    Log( "AnimUVDestroy", "Freeing Memroy" );
    free( AnimUV );
    AnimUV = (ANIMUV *) NULL;
    }

//  Log( "AnimUVDestroy", "Exit" );
  }


/************************************************************************
*                                                                       *
*       AnimUVLoad:                                                     *
*                                                                       *
*       Loads The Previously Saved Data From The Scene File.            *
*                                                                       *
*       Input:                                                          *
*                                                                       *
*       AnimUV  = Address Of Anim UV Structure.                         *
*       lState  = Address Of Load State Structure.                      *
*                                                                       *
*       Output:                                                         *
*                                                                       *
*       The Saved Anim UV Values Are Read From The Scene File.          *
*                                                                       *
*       11.July 2003 Jamie Lisa Finch.                                  *
*                                                                       *
************************************************************************/

XCALL_ (LWError) AnimUVLoad( void *vAnimUV, const LWLoadState *lState ) {

  ANIMUV *AnimUV;
  float  fp[ 2 ];

  XCALL_INIT;

//  Log( "AnimUVLoad", "Entered" );

  AnimUV = (ANIMUV *) vAnimUV;
  if( ( AnimUV         != (ANIMUV *) NULL ) &&
      ( lState->ioMode == LWIO_SCENE      ) ) {

//    Log( "AnimUVLoad", "Loading Data" );

    LWLOAD_FP( lState, fp, sizeof( fp ) / sizeof( fp[ 0 ] ) );
    AnimUV->uChangeRate = fp[ 0 ];
    AnimUV->vChangeRate = fp[ 1 ];
    }

//  Log( "AnimUVLoad", "Exit" );

  return NULL;
  }


/************************************************************************
*                                                                       *
*       AnimUVSave:                                                     *
*                                                                       *
*       Save The Current Values Into Scene File.                        *
*                                                                       *
*       Input:                                                          *
*                                                                       *
*       AnimUV  = Address Of Anim UV Structure.                         *
*       sState  = Address Of Save State Structure.                      *
*                                                                       *
*       Output:                                                         *
*                                                                       *
*       Saves The Anim UV Values Into The Scene File.                   *
*                                                                       *
*       11.July 2003 Jamie Lisa Finch.                                  *
*                                                                       *
************************************************************************/

XCALL_ (LWError) AnimUVSave( void *vAnimUV, const LWSaveState *sState ) {

  ANIMUV *AnimUV;
  float  fp[ 2 ];

  XCALL_INIT;

//  Log( "AnimUVSave", "Entered" );

  AnimUV = (ANIMUV *) vAnimUV;
  if( ( AnimUV         != (ANIMUV *) NULL ) &&
      ( sState->ioMode == LWIO_SCENE      ) ) {

//    Log( "AnimUVSave", "Saving Data" );

    fp[ 0 ] = AnimUV->uChangeRate;
    fp[ 1 ] = AnimUV->vChangeRate;
    LWSAVE_FP( sState, fp, sizeof( fp ) / sizeof( fp[ 0 ] ) );
    }

//  Log( "AnimUVSave", "Exit" );

  return NULL;
  }


/************************************************************************
*                                                                       *
*       AnimUVCopy:                                                     *
*                                                                       *
*       Copies One AnimUV Structure To Another.                         *
*                                                                       *
*       Input:                                                          *
*                                                                       *
*       ToAnimUV   = Address Of Destination AnimUV Structure.           *
*       FromAnimUV = Address Of Source      AnimUV Structure.           *
*                                                                       *
*       Output:                                                         *
*                                                                       *
*       The AnimUV Structure Is Copy From One To The Other.             *
*                                                                       *
*       17.July 2003 Jamie Lisa Finch.                                  *
*                                                                       *
************************************************************************/

XCALL_ (LWError) AnimUVCopy( void *vToAnimUV, void *vFromAnimUV ) {

  ANIMUV *ToAnimUV, *FromAnimUV;

  XCALL_INIT;

    ToAnimUV = (ANIMUV *)   vToAnimUV;
  FromAnimUV = (ANIMUV *) vFromAnimUV;

  if( (   ToAnimUV != (ANIMUV *) NULL ) &&
      ( FromAnimUV != (ANIMUV *) NULL ) ) {
    *ToAnimUV = *FromAnimUV;
    }
  return NULL;
  }


/************************************************************************
*                                                                       *
*       AnimUVDescribe:                                                 *
*                                                                       *
*       Returns A Character String Telling The Called Which Plugin      *
*       This Is.                                                        *
*                                                                       *
*       Input:                                                          *
*                                                                       *
*       AnimUV  = Address Of Destination AnimUV Structure.              *
*                                                                       *
*       Output:                                                         *
*                                                                       *
*       The Description String Is Returned.                             *
*                                                                       *
*       17.July 2003 Jamie Lisa Finch.                                  *
*                                                                       *
************************************************************************/

XCALL_ (const char *) AnimUVDescribe( void *vAnimUV ) {

  XCALL_INIT;

  return PluginDescription;
  }


/************************************************************************
*                                                                       *
*       AnimUV Calls:                                                   *
*                                                                       *
*       These Are The Calls Made To Manipulate The UV's.                *
*                                                                       *
*       17.July 2003 Jamie Lisa Finch.                                  *
*                                                                       *
************************************************************************/


/************************************************************************
*                                                                       *
*       AnimUVGetOptions:                                               *
*                                                                       *
*       Returns The Number And The Array Of Options Data.  If The       *
*       Options Array Is Null, Simply Returns The Number Of Bytes       *
*       In Options Array.                                               *
*                                                                       *
*       Input:                                                          *
*                                                                       *
*       AnimUV  = Address Of AnimUV Structure.                          *
*       Options = Address Of Destination Options Data.                  *
*                                                                       *
*       Output:                                                         *
*                                                                       *
*       Returns -> Number Of Options Bytes.                             *
*                  If Options Array Not Null, Then The Options Bytes    *
*                  Are Copied Into The Array.                           *
*                                                                       *
*       17.July 2003 Jamie Lisa Finch.                                  *
*                                                                       *
************************************************************************/

XCALL_ (int) AnimUVGetOptions( void *vAnimUV, char *Options ) {

  ANIMUV *AnimUV;
  double  fp[ 2 ];

  XCALL_INIT;

  /* Check To See If We Are Just Being Asked For SizeOf The Data. */

  if( Options == (char *) NULL )
    return sizeof( fp );

  /* Copy The Data To Options. */

  AnimUV = (ANIMUV *) vAnimUV;
  if( AnimUV != (ANIMUV *) NULL ) {
    ByteOrderMotorola( &fp[ 0 ], &AnimUV->uChangeRate, sizeof( AnimUV->uChangeRate ) );
    ByteOrderMotorola( &fp[ 1 ], &AnimUV->vChangeRate, sizeof( AnimUV->vChangeRate ) );
    memcpy( Options, fp, sizeof( fp ) );
    return sizeof( fp );
    }
  return 0;
  }


/************************************************************************
*                                                                       *
*       AnimUVSetOptions:                                               *
*                                                                       *
*       Copies The Options Array Into The AnimUV Options Structure.     *
*                                                                       *
*       Input:                                                          *
*                                                                       *
*       AnimUV  = Address Of AnimUV Structure.                          *
*       Options = Address Of Source Options Data.                       *
*                                                                       *
*       Output:                                                         *
*                                                                       *
*       Returns -> 1 == Execution Without Error.                        *
*                  0 == Failed To Copy Options Array.                   *
*                                                                       *
*       17.July 2003 Jamie Lisa Finch.                                  *
*                                                                       *
************************************************************************/

XCALL_ (int) AnimUVSetOptions( void *vAnimUV, char *Options ) {

  ANIMUV *AnimUV;
  double  fp[ 2 ];

  XCALL_INIT;

  /* Copy The Optins Into Our Instance. */

  AnimUV = (ANIMUV *) vAnimUV;
  if( ( AnimUV  != (ANIMUV *) NULL ) &&
      ( Options != (char *)   NULL ) ) {
    memcpy( fp, Options, sizeof( fp ) );
    ByteOrderMotorola( &AnimUV->uChangeRate, &fp[ 0 ], sizeof( AnimUV->uChangeRate ) );
    ByteOrderMotorola( &AnimUV->vChangeRate, &fp[ 1 ], sizeof( AnimUV->vChangeRate ) );
    return 1;
    }
  return 0;
  }


/************************************************************************
*                                                                       *
*       AnimUVBegin:                                                    *
*                                                                       *
*       This Call If Made For Each New Surface Before It Is Drawn.      *
*                                                                       *
*       Input:                                                          *
*                                                                       *
*       AnimUV  = Address Of Anim UV Structure.                         *
*       Options = Address Of Options Structure.                         *
*       Time    = Value   Of Time In Seconds.                           *
*       NumberOfVerticies = Number Of Verticies In Surface.             *
*       wRepeat = Value   Of Width  Repeat Type.                        *
*       hRepeat = Value   Of Height Repeat Type.                        *
*                                                                       *
*       Output:                                                         *
*                                                                       *
*       Returns -> 1 == Execution Without Error.                        *
*                  0 == Failed.                                         *
*                                                                       *
*       17.July 2003 Jamie Lisa Finch.                                  *
*                                                                       *
************************************************************************/

XCALL_ (int) AnimUVBegin( LWInstance vAnimUV, char *Options, double Time, int NumberOfVertices, int wRepeat, int hRepeat ) {

  ANIMUV *AnimUV;
  double  fp[ 2 ];

  XCALL_INIT;

  AnimUV = (ANIMUV *) vAnimUV;
  if( AnimUV  != (ANIMUV *) NULL ) {

//    Log( "AnimUVBegin", "Time %f, NumberOfVertices %d", Time, NumberOfVertices );

    AnimUV->Time             = Time;
    AnimUV->NumberOfVertices = NumberOfVertices;
    AnimUV->wRepeat          = wRepeat;
    AnimUV->hRepeat          = hRepeat;

    if( Options != (char *) NULL ) {
      memcpy( fp, Options, sizeof( fp ) );
      ByteOrderMotorola( &AnimUV->uChangeRate, &fp[ 0 ], sizeof( AnimUV->uChangeRate ) );
      ByteOrderMotorola( &AnimUV->vChangeRate, &fp[ 1 ], sizeof( AnimUV->vChangeRate ) );
      }
    return 1;
    }
  return 0;
  }


/************************************************************************
*                                                                       *
*       AnimUVEvaluate:                                                 *
*                                                                       *
*       This Function Is Called For Each Vertex Just Before It Is Drawn.*
*                                                                       *
*       Input:                                                          *
*                                                                       *
*       AnimUV       = Address Of Anim UV Structure.                    *
*       VertexNumber = Which Vertex Is Being Evaluated, 0, 1, 2...      *
*       uv           = Address Of UV Array.                             *
*                                                                       *
*       Output:                                                         *
*                                                                       *
*       Returns -> 1 == Execution Without Error.                        *
*                  0 == Failed.                                         *
*                                                                       *
*       17.July 2003 Jamie Lisa Finch.                                  *
*                                                                       *
************************************************************************/

XCALL_ (int) AnimUVEvaluate( LWInstance vAnimUV, int VertexNumber, double *uv ) {

  ANIMUV *AnimUV;
  int     Count;
  double  Delta, d;

  XCALL_INIT;

  AnimUV = (ANIMUV *) vAnimUV;
  if( ( AnimUV  != (ANIMUV *) NULL ) &&
      ( uv      != (double *) NULL ) ) {

    Count    = (int) AnimUV->Time;
    Delta    = AnimUV->Time - Count;
    uv[ 0 ] += Delta * AnimUV->uChangeRate;
    uv[ 1 ] += Delta * AnimUV->vChangeRate;

#if( 0 )
    /* Deal With Width Repeat. */

    switch( AnimUV->wRepeat ) {
      case TXRPT_RESET:
        break;
      case TXRPT_REPEAT:
        uv[ 0 ] = uv[ 0 ] - floor( uv[ 0 ] );
        break;
      case TXRPT_MIRROR:
        d = uv[ 0 ] / 2;
        d = d - floor( d );
        uv[ 0 ] = uv[ 0 ] - floor( uv[ 0 ] );
        if( d > 0.5 )
          uv[ 0 ]= 1 - uv[ 0 ];
        break;
      case TXRPT_EDGE:
        if(      uv[ 0 ] <= 0 )
          uv[ 0 ] = 0.0001;
        else if( uv[ 0 ] >= 1 )
          uv[ 0 ] = 0.9999;
        break;
      }

    /* Deal With Height Repeat. */

    switch( AnimUV->hRepeat ) {
      case TXRPT_RESET:
        break;
      case TXRPT_REPEAT:
        uv[ 1 ] = uv[ 1 ] - floor( uv[ 1 ] );
        break;
      case TXRPT_MIRROR:
        d = uv[ 1 ] / 2;
        d = d - floor( d );
        uv[ 1 ] = uv[ 1 ] - floor( uv[ 1 ] );
        if( d > 0.5 )
          uv[ 1 ]= 1 - uv[ 1 ];
        break;
      case TXRPT_EDGE:
        if(      uv[ 1 ] <= 0 )
          uv[ 1 ] = 0.0001;
        else if( uv[ 1 ] >= 1 )
          uv[ 1 ] = 0.9999;
        break;
      }
#endif

//    Log( "AnimUVEvaluate", "Vertex %d, u %f, v %f, Delta %f", VertexNumber, uv[ 0 ], uv[ 1 ], Delta );

    return 1;
    }
  return 0;
  }


/************************************************************************
*                                                                       *
*       AnimUVEnd:                                                      *
*                                                                       *
*       This Function Is Called After Each Vertex Was Drawn.            *
*                                                                       *
*       Input:                                                          *
*                                                                       *
*       AnimUV  = Address Of Anim UV Structure.                         *
*                                                                       *
*       Output:                                                         *
*                                                                       *
*       Returns -> 1 == Execution Without Error.                        *
*                  0 == Failed.                                         *
*                                                                       *
*       17.July 2003 Jamie Lisa Finch.                                  *
*                                                                       *
************************************************************************/

XCALL_ (int) AnimUVEnd( LWInstance vAnimUV ) {

  ANIMUV *AnimUV;

  XCALL_INIT;

  AnimUV = (ANIMUV *) vAnimUV;
  if( AnimUV != (ANIMUV *) NULL ) {

//    Log( "AnimUVEnd", "Entered" );

    return 1;
    }
  return 0;
  }


/************************************************************************
*                                                                       *
*       AnimUVActivate:                                                 *
*                                                                       *
*       This Functions Creates The Anim UV Callback Functions.          *
*                                                                       *
*       Input:                                                          *
*                                                                       *
*       Version    = Query Version Of The Plugin.                       *
*       Global     = Address Of The Global Function.                    *
*       Local      = Address Of New LW Anim UV Hander Structure.        *
*       ServerData = Address Of Server Data.                            *
*                                                                       *
*       Output:                                                         *
*                                                                       *
*       Returns -> Value Of Error Code, AFUNC_OK, AFUNC_BADVERSION.     *
*                                                                       *
*       11.July 2003 Jamie Lisa Finch.                                  *
*                                                                       *
************************************************************************/

XCALL_ (int) AnimUVActivate( long Version, GlobalFunc *Global, void *vLocal, void *ServerData ) {

  LWAnimUVHandler *Local;

  XCALL_INIT;

  Local = (LWAnimUVHandler *) vLocal;

//  Log( "AnimUVActivate", "Entered" );

  if( ( Version != LWANIMUV_VERSION ) )
    return AFUNC_BADVERSION;

  GlobalFunction        = Global;

  Local->inst->create   = AnimUVCreate;
  Local->inst->destroy  = AnimUVDestroy;
  Local->inst->load     = AnimUVLoad;
  Local->inst->save     = AnimUVSave;
  Local->inst->copy     = AnimUVCopy;
  Local->inst->descln   = AnimUVDescribe; 

  Local->GetOptions     = AnimUVGetOptions;
  Local->SetOptions     = AnimUVSetOptions;
  Local->Begin          = AnimUVBegin;
  Local->Evaluate       = AnimUVEvaluate;
  Local->End            = AnimUVEnd;

//  Log( "AnimUVActivate", "Exit" );

  return AFUNC_OK;
  }


/************************************************************************
*                                                                       *
*       Interface Functions:                                            *
*                                                                       *
*       This Block Of Code Contains The Interface Panel Functions.      *
*                                                                       *
*       11.July 2003 Jamie Lisa Finch.                                  *
*                                                                       *
************************************************************************/


/************************************************************************
*                                                                       *
*       AnimUVPanel:                                                    *
*                                                                       *
*       This Functions Creates And Executes The Panel.                  *
*                                                                       *
*       Input:                                                          *
*                                                                       *
*       AnimUV = Address Of Anim UV Structure.                          *
*                                                                       *
*       Output:                                                         *
*                                                                       *
*       Returns -> 1 == New Values Set.                                 *
*                  0 == Values Were Not Changed.                        *
*                                                                       *
*       11.July 2003 Jamie Lisa Finch.                                  *
*                                                                       *
************************************************************************/

static LWValue fval = { LWT_FLOAT };    /* Required By Macros In lwpanel.h */

XCALL_(LWError) AnimUVPanel( LWInstance vAnimUV ) {

  ANIMUV          *AnimUV;
  LWPanelFuncs    *Panel;
  LWPanelID        PanelID;
  int              Width, Height;
  LWPanControlDesc desc;                /* Required By Macros In lwpanel.h */

  XCALL_INIT;

//  Log( "AnimUVPanel", "Entered" );

  AnimUV = (ANIMUV *) vAnimUV;
  if( AnimUV != (ANIMUV *) NULL ) {

    Panel = (LWPanelFuncs *) GlobalFunction( PANEL_SERVICES_NAME, GFUSE_TRANSIENT );
    if( Panel != (LWPanelFuncs *) NULL ) {
      PanelID = Panel->create( PluginUser, Panel );
      if( PanelID != (LWPanelID) NULL ) {

        /* Set Panel Width And Height.*/

        Width  = 200;
        Height =  90;
        Panel->set( PanelID, PAN_W, &Width );
        Panel->set( PanelID, PAN_H, &Height );

        /* Send Panel Buttons. */

        AnimUV->uChangeRateControl = FLOAT_CTL( Panel, PanelID, uChangeRate );
        AnimUV->vChangeRateControl = FLOAT_CTL( Panel, PanelID, vChangeRate );

        /* Send Panel Button Data. */

        SET_FLOAT( AnimUV->uChangeRateControl, AnimUV->uChangeRate );
        SET_FLOAT( AnimUV->vChangeRateControl, AnimUV->vChangeRate );

        /* Place Button On Screen. */

        Panel->set( PanelID, PAN_USERDATA, AnimUV );
        if( Panel->open( PanelID, PANF_FRAME | PANF_BLOCKING | PANF_CANCEL ) == 1 ) {

          /* They Hit Ok. */

          GET_FLOAT( AnimUV->uChangeRateControl, AnimUV->uChangeRate );
          GET_FLOAT( AnimUV->vChangeRateControl, AnimUV->vChangeRate );
          }
        Panel->destroy( PanelID );
        PanelID = (LWPanelID) NULL;
        }
      }
    }

//  Log( "AnimUVPanel", "Exit" );

  return NULL;
  }


/************************************************************************
*                                                                       *
*       AnimUVInterface:                                                *
*                                                                       *
*       This Functions Creates The X Panel Interface For The Plugin     *
*       Options.                                                        *
*                                                                       *
*       Input:                                                          *
*                                                                       *
*       Version    = Query Version Of The Plugin.                       *
*       Global     = Address Of The Global Function.                    *
*       UI         = Address Of LightWave User Interface Structure.     *
*       ServerData = Address Of Server Data.                            *
*                                                                       *
*       Output:                                                         *
*                                                                       *
*       Returns -> Value Of Error Code, AFUNC_OK, AFUNC_BADVERSION Or   *
*                  AFUNC_BADGLOBAL.                                     *
*                                                                       *
*       11.July 2003 Jamie Lisa Finch.                                  *
*                                                                       *
************************************************************************/

XCALL_ (int) AnimUVInterface( long Version, GlobalFunc *Global, void *vUI, void *ServerData ) {

  LWInterface *UI;

  XCALL_INIT;

  UI = (LWInterface *) vUI;

//  Log( "AnimUVInterface", "Entered" );

  if( Version != LWINTERFACE_VERSION ) {
//    Log( "AnimUVInterface", "Exit Bad Version" );
    return AFUNC_BADVERSION;
    }

  UI->panel   = NULL;
  UI->options = AnimUVPanel;    /* Place Panel On The Screen. */
  UI->command = NULL; 

//  Log( "AnimUVInterface", "Exit" );

  return AFUNC_OK;
  }


/************************************************************************
*                                                                       *
*       ServerDesc:                                                     *
*                                                                       *
*       The Name "PluginName" Is The Unique Name That Identifies        *
*       The Plugin.                                                     *
*                                                                       *
*       10.July 2003 Jamie Lisa Finch.                                  *
*                                                                       *
************************************************************************/

static ServerTagInfo AnimUV_Tags[] = {
  { PluginUser, SRVTAG_USERNAME | LANGID_USENGLISH }, 
  { NULL } };

ServerRecord ServerDesc[] = {
  { LWANIMUV_HCLASS, PluginName, AnimUVActivate,  AnimUV_Tags },
  { LWANIMUV_ICLASS, PluginName, AnimUVInterface, AnimUV_Tags },
  { NULL }
  };
