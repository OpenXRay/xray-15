//--------------------------------------------------------------------------
// ComRing Interaction Example                                   by Bob Hood
//
// This CustomObject plug-in is a tool used by the RingTest LScript.  It
// displays a circular area around the camera object to which it is applied,
// and then adjusts that area in real time as messages are sent to it from
// the LScript using the ComRing.
//
// As part of the "example" nature of this project, a chunk of data is
// sent to the LScript for decoding whenever this plug-in is activated.
//
//

#include <lwsdk/lwserver.h>
#include <lwsdk/lwhost.h>
#include <lwsdk/lwcustobj.h>
#include <lwsdk/lwcomring.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#if defined(_MSWIN)
#include <math.h>
#endif

#define RINGNAME    "ringtest_channel"

// this structure holds some 'dummy' data that we will send to the LScript
// to decode and display to the user.

typedef struct _dummydata
{
    char    message[3][100];
    int     an_int;
    double  a_double;
    float   array_float[5];
} DummyData;

// this is the structure that holds the data for the plug-in

typedef struct _ringtest *RingTestID;
typedef struct _ringtest
{
    double  radius;

    LWItemID    me;
    LWItemID    target;

    double  circle_points_xz[36][3];
    double  circle_points_yz[36][3];
    double  circle_points_yx[36][3];

} RingTest;

static GlobalFunc   *global;

// calculate a quarter of a circle, and mirror the points to
// the other three quadrants

static void calcNewCircle(RingTestID pd)
{
    double      angle,rad_angle;
    int         x;

    angle = 0.0;

    for(x = 0;x < 9;x++)
    {
        rad_angle = angle * 1.74532925199433E-002;

        pd->circle_points_xz[x][0] = pd->radius * sin(rad_angle);      // x
        pd->circle_points_xz[x][1] = 0.0;
        pd->circle_points_xz[x][2] = pd->radius * cos(rad_angle);      // z

        pd->circle_points_yz[x][0] = 0.0;
        pd->circle_points_yz[x][1] = pd->radius * sin(rad_angle);
        pd->circle_points_yz[x][2] = pd->radius * cos(rad_angle);

        pd->circle_points_yx[x][0] = pd->radius * sin(rad_angle);
        pd->circle_points_yx[x][1] = pd->radius * cos(rad_angle);
        pd->circle_points_yx[x][2] = 0.0;

        rad_angle = (angle + 90.0) * 1.74532925199433E-002;

        pd->circle_points_xz[x + 9][0] = pd->radius * sin(rad_angle);
        pd->circle_points_xz[x + 9][1] = 0.0;
        pd->circle_points_xz[x + 9][2] = pd->radius * cos(rad_angle);

        pd->circle_points_yz[x + 9][0] = 0.0;
        pd->circle_points_yz[x + 9][1] = pd->radius * sin(rad_angle);
        pd->circle_points_yz[x + 9][2] = pd->radius * cos(rad_angle);

        pd->circle_points_yx[x + 9][0] = pd->radius * sin(rad_angle);
        pd->circle_points_yx[x + 9][1] = pd->radius * cos(rad_angle);
        pd->circle_points_yx[x + 9][2] = 0.0;

        rad_angle = (angle + 180.0) * 1.74532925199433E-002;

        pd->circle_points_xz[x + 18][0] = pd->radius * sin(rad_angle);
        pd->circle_points_xz[x + 18][1] = 0.0;
        pd->circle_points_xz[x + 18][2] = pd->radius * cos(rad_angle);

        pd->circle_points_yz[x + 18][0] = 0.0;
        pd->circle_points_yz[x + 18][1] = pd->radius * sin(rad_angle);
        pd->circle_points_yz[x + 18][2] = pd->radius * cos(rad_angle);

        pd->circle_points_yx[x + 18][0] = pd->radius * sin(rad_angle);
        pd->circle_points_yx[x + 18][1] = pd->radius * cos(rad_angle);
        pd->circle_points_yx[x + 18][2] = 0.0;

        rad_angle = (angle + 270.0) * 1.74532925199433E-002;

        pd->circle_points_xz[x + 27][0] = pd->radius * sin(rad_angle);
        pd->circle_points_xz[x + 27][1] = 0.0;
        pd->circle_points_xz[x + 27][2] = pd->radius * cos(rad_angle);

        pd->circle_points_yz[x + 27][0] = 0.0;
        pd->circle_points_yz[x + 27][1] = pd->radius * sin(rad_angle);
        pd->circle_points_yz[x + 27][2] = pd->radius * cos(rad_angle);

        pd->circle_points_yx[x + 27][0] = pd->radius * sin(rad_angle);
        pd->circle_points_yx[x + 27][1] = pd->radius * cos(rad_angle);
        pd->circle_points_yx[x + 27][2] = 0.0;

        angle += 10.0;
    }
}

// as events are generated on our particular ComRing ("ringtest_channel", created
// by the controlling LScript), we will receive notification here in real-time.
// because it is in real-time -- i.e., we are holding onto the CPU time -- we must
// process here as quickly as possible.

static void ringEvent(void *clientData,void *portData,int eventCode,void *eventData)
{
    double          *ddata;
    int             *idata;
    DummyData       *dd;
    RingTestID pd;

    pd = (RingTestID)clientData;

    if(eventCode == (int)pd->me)
    {
        ddata = (double *)eventData;
        pd->radius = *ddata;

        // calculate a new circle

        calcNewCircle(pd);
    }
    else if(eventCode == 1)     // target selection
    {
        idata = (int *)eventData;
        pd->target = (LWItemID)*idata;
    }
    else if(eventCode == 2)     // more complex test data from the LScript
    {
        dd = (DummyData *)eventData;

        // you can examine the data here
        dd = NULL;
    }
}

// initialize the plug-in data, and subscribe to the ComRing topic created
// by the LScript.  Also, send the LScript a structure pointer using the
// ComRing so it can decode the data.  because ComRing events occur in real
// time, we need not make the structure static.  when the ringMessage()
// function returns, all subscribers to the ComRing topic will have been
// serviced, and the data we sent no longer needed.

XCALL_(static LWInstance) Create(void *priv, LWItemID item, LWError *err)
{
    LWComRing       *cmFunc;
    RingTestID inst;
    DummyData       dd;

    if(inst = malloc(sizeof(RingTest)))
    {
        inst->me = item;
        inst->target = NULL;
        inst->radius = 1.0;

        calcNewCircle(inst);

        cmFunc = (*global)(LWCOMRING_GLOBAL,GFUSE_TRANSIENT);
        (*cmFunc->ringAttach)(RINGNAME,(LWInstance)inst,ringEvent);

        // test the caller's event function.  the ringtest.ls
        // script will display these values to the user when
        // they are decoded.

        memset(&dd,0,sizeof(DummyData));
        strcpy(dd.message[0],"Greetings!");
        strcpy(dd.message[1],"Salutations!");
        strcpy(dd.message[2],"Falicitations!");

        dd.an_int = 23876;
        dd.a_double = -5.49821;

        dd.array_float[0] = 1.1111f;
        dd.array_float[1] = 2.2222f;
        dd.array_float[2] = 3.3333f;
        dd.array_float[3] = 4.4444f;
        dd.array_float[4] = 5.5555f;

        (*cmFunc->ringMessage)(RINGNAME,1,(void *)&dd);
    }
    else
        *err = "Couldn't allocate instance data!";

    return inst;
}

// when we are done with the ComRing, we must detach from each topic to
// which we are subscribed.

XCALL_(static void) Destroy(RingTestID inst)
{
    LWComRing       *cmFunc;

    if(inst)
    {
        cmFunc = (*global)(LWCOMRING_GLOBAL,GFUSE_TRANSIENT);
        (*cmFunc->ringDetach)(RINGNAME,(LWInstance)inst);

        free(inst);
    }
}

XCALL_(static LWError) Copy(RingTestID to,RingTestID from)
{
   LWItemID me_tmp;

   me_tmp = to->me;
   *to = *from;
   to->me = me_tmp;

   return NULL;
}

XCALL_( static LWError )
Load( RingTestID inst, const LWLoadState *ls ) { return NULL; }

XCALL_( static LWError )
Save( RingTestID inst, const LWSaveState *ss ) { return NULL; }

XCALL_( static const char * )
Describe( RingTestID inst ) { return "ComRing Interaction Test"; }

XCALL_( static const LWItemID * )
UseItems( RingTestID inst ) { return NULL; }

XCALL_( static void )
ChangeID( RingTestID inst, const LWItemID *ids ) { }

XCALL_( static LWError )
Init( RingTestID inst, int mode ) { return NULL; }

XCALL_( static void )
Cleanup( RingTestID inst ) { }

XCALL_( static LWError )
NewTime( RingTestID inst, LWFrame fr, LWTime t ) { return NULL; }

XCALL_( static unsigned int )
Flags( RingTestID inst ) { return LWCOF_OVERLAY; }

// draw the custom object ring around the camera

XCALL_(static void) Evaluate(RingTestID pd,const LWCustomObjAccess *access)
{
    double      center[3];
    float       rgba[4];
    int         x;

    rgba[0] = 0.0f;
    rgba[1] = 1.0f;
    rgba[2] = 0.0f;
    rgba[3] = 0.5f;

    (*access->setPattern)(access->dispData,LWLPAT_SOLID);
    (*access->setColor)(access->dispData,rgba);

    center[0] = 0.0;center[1] = 0.0;center[2] = 0.0;

    for(x = 0;x < 35;x++)
    {
        (*access->triangle)(access->dispData,
                            center,
                            pd->circle_points_xz[x],
                            pd->circle_points_xz[x + 1],
                            LWCSYS_OBJECT);
    }

    (*access->triangle)(access->dispData,
                        center,
                        pd->circle_points_xz[35],
                        pd->circle_points_xz[0],
                        LWCSYS_OBJECT);
}

XCALL_(static int) COActivate(long version,GlobalFunc *g,LWCustomObjHandler *local,void *serverData)
{
    if(version != LWCUSTOMOBJ_VERSION)
       return AFUNC_BADVERSION;

    local->inst->create  = Create;
    local->inst->destroy = Destroy;
    local->inst->load    = Load;
    local->inst->save    = Save;
    local->inst->copy    = Copy;
    local->inst->descln  = Describe;

    if(local->item)
    {
       local->item->useItems = UseItems;
       local->item->changeID = ChangeID;
    }

    if(local->rend)
    {
       local->rend->init    = Init;
       local->rend->cleanup = Cleanup;
       local->rend->newTime = NewTime;
    }

    local->evaluate = Evaluate;
    local->flags    = Flags;

    global = g;

    return AFUNC_OK;
}

ServerRecord ServerDesc[] = {
    { LWCUSTOMOBJ_HCLASS, "ComRingTest", COActivate },
	{ NULL }
};
