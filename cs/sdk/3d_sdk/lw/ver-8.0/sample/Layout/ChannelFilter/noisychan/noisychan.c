/* NoisyChan.c -- A Channel handler Plugin
 *   Add Noise to a channel using the texture global
 *      Arnie Cachelin
 */

#include <lwhost.h>
#include <lwserver.h>
#include <lwhandler.h>
#include <lwdialog.h>
#include <lwchannel.h>
#include <lwrender.h>
#include <lwtxtr.h>
#include <lwxpanel.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static    LWXPanelFuncs        *GlobalXPanFun;
static    LWMessageFuncs        *Gmessage;
static    LWTextureFuncs        *GlobalTextureFuncs;
static  GlobalFunc            *GGlobal=NULL;

/* Instance Data structure definition for this plugin */
typedef struct st_NoisyData {
    void                *ctxt;
    LWChannelID             self;
    double                 offset, scale, speed, phase;
    char                 desc[100];
} NoisyData;


#define STR_Offset_TEXT        "Offset"
#define STR_Scale_TEXT        "Scale"
#define STR_Speed_TEXT        "Speed"
#define STR_Phase_TEXT        "Phase"

/* ----------------- Plug-in Methods: LWInstanceFuncs  -----------------  */

static char *errAllocFailed ="Tiny Allocation Failed, I don't feel so good";

XCALL_(static LWInstance)NoisyChanCreate(void *data, LWChannelID chan, LWError *err)
{
    NoisyData *dat=NULL;
    XCALL_INIT;
    if(dat=malloc(sizeof(NoisyData)))
    {
        memset(dat,0,sizeof(*dat));
        dat->offset = 0.0;
        dat->speed = 1.0;
        dat->scale = 1.0;
        dat->phase = 0.0;
        dat->self = chan;
        dat->ctxt = data;
        sprintf(dat->desc,"Scale: %.2f Speed %2f", dat->scale, dat->speed );
    }
    else
        *err = errAllocFailed;
    return dat;
}


XCALL_(static void)NoisyChanDestroy(NoisyData *dat)
{
    XCALL_INIT;
    if(dat)
    {
        free(dat);
    }
}

XCALL_(static LWError)NoisyChanCopy(NoisyData    *to, NoisyData    *from)
{
    XCALL_INIT;

    *to = *from;
    return (NULL);
}

XCALL_(static LWError)NoisyChanLoad(NoisyData *dat,const LWLoadState    *lState)
{
    float fv = 0.0f;
    XCALL_INIT;
    (*lState->readFP)(lState->readData,&fv,1);
    dat->offset = (double)fv;
    (*lState->readFP)(lState->readData,&fv,1);
    dat->scale = (double)fv;
    (*lState->readFP)(lState->readData,&fv,1);
    dat->speed = (double)fv;
    (*lState->readFP)(lState->readData,&fv,1);
    dat->phase = (double)fv;
    return (NULL);
}

XCALL_(static LWError)NoisyChanSave(NoisyData *dat,const LWSaveState    *sState)
{
    float fv = 0.0f;
    XCALL_INIT;
    fv = (float)dat->offset;
    (*sState->writeFP)(sState->writeData,&fv,1);
    fv = (float)dat->scale;
    (*sState->writeFP)(sState->writeData,&fv,1);
    fv = (float)dat->speed;
    (*sState->writeFP)(sState->writeData,&fv,1);
    fv = (float)dat->phase;
    (*sState->writeFP)(sState->writeData,&fv,1);
    return (NULL);
}

XCALL_(static const char *)NoisyChanDescribe (LWInstance inst)
{
    NoisyData *dat = (NoisyData *)inst;
    XCALL_INIT;

    sprintf(dat->desc,"Scale: %.2f Speed %2f", dat->scale, dat->speed );

    return (dat->desc);
}


/* ----------------- Plug-in Methods: LWChannelHandler  -----------------  */

XCALL_(static unsigned int)NoisyChanFlags (NoisyData *inst)
{
    XCALL_INIT;

    return 0;
}

XCALL_(static void)NoisyChanEval (NoisyData *dat,    const LWChannelAccess *chanAcc)
{
    double val = 0.0, t, vec[3];
    XCALL_INIT;
    t = chanAcc->time*dat->speed;
    vec[0] = 10*t;
    vec[1] = dat->phase;
    vec[2] = 20;
    val = (*GlobalTextureFuncs->noise)(vec);
    val *= dat->scale;
    val += dat->offset;
    val += chanAcc->value;
    (*chanAcc->setChannel)(chanAcc->chan, val);
}

/* ----------------- Plug-in Activation  -----------------  */

XCALL_(static int) NoisyChannel (
    long                  version,
    GlobalFunc            *global,
    LWChannelHandler    *local,
    void                *serverData)
{
    XCALL_INIT;
    if (version != LWCHANNEL_VERSION)
        return (AFUNC_BADVERSION);

    Gmessage = (*global) ("Info Messages", GFUSE_TRANSIENT);
    if (!Gmessage )
        return AFUNC_BADGLOBAL;

    GlobalTextureFuncs = (*global) (LWTEXTUREFUNCS_GLOBAL, GFUSE_TRANSIENT);
    if (!GlobalTextureFuncs )
        return AFUNC_BADGLOBAL;

    local->inst->create  = NoisyChanCreate;
    local->inst->destroy = NoisyChanDestroy;
    local->inst->load    = NoisyChanLoad;
    local->inst->save    = NoisyChanSave;
    local->inst->copy    = NoisyChanCopy;

    local->evaluate     = NoisyChanEval;
    local->flags         = NoisyChanFlags;
    local->inst->descln        = NoisyChanDescribe;
    return (AFUNC_OK);
}


/* -----------------  User Interface  ----------------- */

enum  {    CH_OFFSET = 0x8001, CH_SCALE, CH_SPEED, CH_PHASE };

static LWXPanelControl ctrl_list[] = {
    { CH_OFFSET,       STR_Offset_TEXT,          "float" },
    { CH_SCALE,        STR_Scale_TEXT,          "float" },
    { CH_SPEED,        STR_Speed_TEXT,          "float" },
    { CH_PHASE,        STR_Phase_TEXT,          "float" },
    {0}
};
static LWXPanelDataDesc data_descrip[] = {
    { CH_OFFSET,        STR_Offset_TEXT,          "float" },
    { CH_SCALE,        STR_Scale_TEXT,          "float" },
    { CH_SPEED,        STR_Speed_TEXT,          "float" },
    { CH_PHASE,        STR_Phase_TEXT,          "float" },
    {0},
};

void *NoiseData_get ( void *myinst, unsigned long vid )
{
  NoisyData *dat = (NoisyData*)myinst;
  void *result = NULL;
  static double val = 0.0;

  if ( dat )
      switch ( vid ) {
        case CH_OFFSET:
          val = dat->offset;
          result = &val;
          break;
        case CH_SCALE:
          val = dat->scale;
          result = &val;
          break;
        case CH_SPEED:
          val = dat->speed;
          result = &val;
          break;
        case CH_PHASE:
          val = dat->phase;
          result = &val;
          break;
      }
  return result;
}

int NoiseData_set ( void *myinst, unsigned long vid, void *value )
{
  NoisyData *dat = (NoisyData*)myinst;
  int rc=0;
  if ( dat )
      switch ( vid ) {
        case CH_OFFSET:
          dat->offset = *((double*)value);
          rc = 1;
          break;
        case CH_SCALE:
          dat->scale = *((double*)value);
          rc = 1;
          break;
        case CH_SPEED:
          dat->speed = *((double*)value);
          rc = 1;
          break;
        case CH_PHASE:
          dat->phase = *((double*)value);
          rc = 1;
          break;
      }
  return rc;
}


LWXPanelID NoisyXPanel(GlobalFunc *global, NoisyData *dat)
{
    LWXPanelFuncs *lwxpf = NULL;
    LWXPanelID     panID = NULL;
    static LWXPanelHint hint[] = {
        XpLABEL(0,"Noisy Channel"),
        XpEND
    };

    lwxpf = (LWXPanelFuncs*)(*global)( LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT);
    if ( lwxpf )
    {
        panID = (*lwxpf->create)( LWXP_VIEW, ctrl_list );
        if(panID)
        {
            (*lwxpf->hint) ( panID, 0, hint );
            (*lwxpf->describe)( panID, data_descrip, NoiseData_get, NoiseData_set );
            (*lwxpf->viewInst)( panID, dat );
            (*lwxpf->setData)(panID, 0, dat);
        }
    }
    return panID;
}

XCALL_(static int) NoisyChannel_UI (
    long             version,
    GlobalFunc        *global,
    LWInterface        *UI,
    void            *serverData)
{
    XCALL_INIT;
    if (version != LWINTERFACE_VERSION)
        return (AFUNC_BADVERSION);
    GGlobal = global;

    UI->panel    = NoisyXPanel(global, UI->inst);
    UI->options    = NULL;
    UI->command    = NULL;
    return AFUNC_OK;
}

ServerRecord ServerDesc[] = {
    { LWCHANNEL_HCLASS,        "NoisyChannel",        NoisyChannel },
    { LWCHANNEL_ICLASS,        "NoisyChannel",        NoisyChannel_UI },
    { NULL }
};
