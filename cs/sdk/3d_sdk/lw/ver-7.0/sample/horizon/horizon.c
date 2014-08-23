/*
 *      horizon.c -- A procedural texture plugin 
 *
 *      Copyright 1999 NewTek, Inc.
 */


#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <lwserver.h>
#include <lwhandler.h>
#include <lwenviron.h>


#define ZEN_R   0.0 
#define ZEN_G   40.0/255 
#define ZEN_B   40.0/255


#define SKY_R   120.0/255 
#define SKY_G   180.0/255 
#define SKY_B   240.0/255


#define GND_R   50.0/255 
#define GND_G   40.0/255
#define GND_B   30.0/255


#define NAD_R   100.0/255 
#define NAD_G   80.0/255  
#define NAD_B   60.0/255 


typedef struct st_HorizonBkgData {
       double          skySq, gndSq;
       double          zenith[3], sky[3], ground[3], nadir[3];
} HorizonBkgData;


double squeezeColor(double sq, double frac, double *min, double *max, double *rgb)
{
    double val = 0.0f  ,c=0.0f;
    int i;
    val = pow(frac,sq);
    for(i=0;i<3;i++)
    {
        c = val*((max[i] - min[i])); // should be >0 ... 
        rgb[i] = (min[i] + c);
    }
    return val;
}
#define PI      3.141592


HorizonBkgData *horizonCreate(void *priv, void *cntxt, LWError *err)
{
    HorizonBkgData *dat;
    err = NULL;
    if(dat = malloc(sizeof(HorizonBkgData)))
    {
        memset( dat, 0, sizeof(HorizonBkgData) );
        dat->skySq = 2.0;
        dat->gndSq = 2.0;
        dat->zenith[0] = ZEN_R; dat->zenith[1] = ZEN_G; dat->zenith[2] = ZEN_B;
        dat->sky[0] = SKY_R; dat->sky[1] = SKY_G; dat->sky[2] = SKY_B;
        dat->ground[0] = GND_R; dat->ground[1] = GND_G; dat->ground[2] = GND_B;
        dat->nadir[0] = NAD_R; dat->nadir[1] = NAD_G; dat->nadir[2] = NAD_B;
    }
    return dat;
}


void horizonDestroy(HorizonBkgData *dat)
{
    if(dat)
        free(dat);
}


LWError horizonCopy(HorizonBkgData *to, HorizonBkgData *from)
{
    if(to && from)
        *to = *from;
    return NULL;
}


LWError horizonNewTime(HorizonBkgData *dat, LWFrame f, LWTime t)
{
    // Here, Now, Time Stands Still
    // Animated Backdrop Sky?
    // Catch This Time of Day
    return NULL;
}
/*
 * Fill the corner colors: in this case, the heading doesn't matter!
 */
static double dmin=0.0, dmax=0.0, noise = 0.0;
static int tst = 0;


LWError horizonEvaluate(HorizonBkgData *dat, LWEnvironmentAccess *evr)
{
    double f, vec[3]={0.0,0.0,0.0};


    f = (evr->dir[1]);
    if(f>0)
        squeezeColor(dat->skySq, f, dat->sky, dat->zenith, (evr->color));
    else
        squeezeColor(dat->gndSq, f, dat->ground, dat->nadir, (evr->color));


    return  NULL;
}


static double csc = 1.0, gsc = 1.0, grd = 0.105;
// Cool grid bkg.. 
LWError tstEvaluate(HorizonBkgData *dat, LWEnvironmentAccess *evr)
{
    double d;


    evr->color[0] = fabs(csc*evr->dir[0]);
    evr->color[1] = fabs(csc*evr->dir[1]);
    evr->color[2] = fabs(csc*evr->dir[2]);
    d = 10.0*gsc*fabs(evr->dir[0]);
    d -= floor(d);
    if(d<=grd)
    {
        evr->color[0] = 0.0;
        evr->color[1] = 0.0;
        evr->color[2] = 0.0;
        return  NULL;
    }


    d = 10.0*gsc*fabs((evr->dir[1]));
    d -= floor(d);
    if(d<=grd)
    {
        evr->color[0] = 0.0;
        evr->color[1] = 0.0;
        evr->color[2] = 0.0;
        return  NULL;
    }


    d = 10.0*gsc*fabs(evr->dir[2]);
    d -= floor(d);
    if(d<=grd*0.5)
    {
        evr->color[0] = 0.80;
        evr->color[1] = 0.70;
        evr->color[2] = 0.10;
        return  NULL;
    }
    return  NULL;
}


int horizonFlags( HorizonBkgData *dat)
{
    return 0;
}


XCALL_(int)HorizonActivate (
    long                     version,
    GlobalFunc              *global,
    LWEnvironmentHandler     *local,
    void                    *serverData)
{
    XCALL_INIT;
    if(version != LWENVIRONMENT_VERSION)
        return AFUNC_BADVERSION;


    local->inst->create = horizonCreate;
    local->inst->destroy = horizonDestroy;
    local->inst->copy = horizonCopy;
    local->rend->newTime = horizonNewTime;
    local->evaluate = tst ? tstEvaluate : horizonEvaluate;
    local->flags = horizonFlags;


    return AFUNC_OK;
}






ServerRecord ServerDesc[] = {
    { LWENVIRONMENT_HCLASS,        "GradientBackdrop",        HorizonActivate },
    { NULL }
};
