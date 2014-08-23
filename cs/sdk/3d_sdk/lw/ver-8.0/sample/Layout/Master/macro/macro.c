//--------------------------------------------
// Master Class Macro Recorder     by Bob Hood
//
// This plug-in will monitor the stream of
// CommandSequnce events that take place in
// Layout, and will generate a v2.0 Generic
// LScript file that will reproduce the
// sequence of commands, including their
// timings.
//
// There's no nifty interface on the plug-in.
// Double-clicking to activate the interface
// only generates the LScript to a pre-defined
// location using a pre-defined name.
//--------------------------------------------

#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <time.h>

#include    <lwserver.h>
#include    <lwhost.h>
#include    <lwmonitor.h>
#include    <lwrender.h>
#include    <lwio.h>
#include    <lwdyna.h>
#include    <lwmaster.h>
#include    <lwpanel.h>

GlobalFunc        *global;

typedef int CommandFunc(const char *);

typedef struct _command
{
    struct  _command    *next;

    char    *cs;
    char    *ls;

    time_t  tick;
} MCCommand;

typedef struct _mcdata
{
    MCCommand   *head;
    MCCommand   *curr;

    time_t      lasttick;
} MCData;

LWMessageFuncs  *message;

const char      *describe(LWInstance);
LWInstance      create(void *,void *,LWError *);
void            destroy(LWInstance);
LWError         copy(LWInstance,LWInstance);
LWError         load(LWInstance,const LWLoadState *);
LWError         save(LWInstance,const LWSaveState *);
double          process(LWInstance,const LWMasterAccess *);
unsigned int    flags(LWInstance);

static int Activate(long              version,
                    GlobalFunc        *g,
                    LWMasterHandler   *local,
                    void              *data)
{
    if(version != LWMASTER_VERSION)
        return(AFUNC_BADVERSION);

    if(local->inst)
    {
        local->inst->create     = create;
        local->inst->destroy    = destroy;
        local->inst->load       = load;
        local->inst->save       = save;
        local->inst->copy       = copy;
        local->inst->descln     = describe;
    }

    if(local->item)
    {
        local->item->useItems   = NULL;
        local->item->changeID   = NULL;
    }

    local->event            = process;
    local->flags            = flags;

    global = g;

    message = (LWMessageFuncs *)(*global)("Info Messages",GFUSE_ACQUIRE);

    return(AFUNC_OK);
}

XCALL_(unsigned int) flags(LWInstance inst)
{
    return(0);
}

XCALL_(const char *) describe(LWInstance inst)
{
    return("Macro Recorder");
}

XCALL_(LWInstance) create(void *priv,void *context,LWError *err)
{
    MCData      *mydata;

    XCALL_INIT;

    mydata = (MCData *)malloc(sizeof(MCData));
    memset(mydata,0,sizeof(MCData));
    return(mydata);
}

XCALL_(void) destroy(LWInstance inst)
{
    MCData      *mydata;
    MCCommand   *t;

    XCALL_INIT;

    (*global)("Info Messages",GFUSE_RELEASE);

    mydata = (MCData *)inst;

    while(mydata->head)
    {
        t = mydata->head->next;
        if(mydata->head->cs) free(mydata->head->cs);
        if(mydata->head->ls) free(mydata->head->ls);
        free(mydata->head);
        mydata->head = t;
    }

    free(mydata);
}

XCALL_(LWError) copy(LWInstance d1,LWInstance d2)
{
    XCALL_INIT;
    return(NULL);
}

XCALL_(LWError) load(LWInstance inst,const LWLoadState *lState)
{
    XCALL_INIT;
    return(NULL);
}

XCALL_(LWError) save(LWInstance inst,const LWSaveState *sState)
{
    XCALL_INIT;
    return(NULL);
}

XCALL_(double) process(LWInstance inst,const LWMasterAccess *ma)
{
    MCData      *mydata;
    MCCommand   *newcommand;
    char        *h,*t,*command;
    char        buf[300];
    time_t      tick,tickdiff;

    XCALL_INIT;

    tick = time(NULL);

    mydata = (MCData *)inst;
    command = (char *)ma->eventData;

    if(*command == '*')
    {
        // this command cannot be processed
        return((double)0.0);
    }

    newcommand = (MCCommand *)malloc(sizeof(MCCommand));
    memset(newcommand,0,sizeof(MCCommand));

    if(!mydata->head)
    {
        mydata->head = newcommand;
        mydata->lasttick = tick;
        tickdiff = 0;
    }
    else
    {
        mydata->curr->next = newcommand;
        tickdiff = tick - mydata->lasttick;
        mydata->lasttick = tick;
    }

    mydata->curr = newcommand;
    mydata->curr->next = NULL;

    mydata->curr->tick = tickdiff;

    mydata->curr->cs = malloc( strlen( command ) + 1 );
    strcpy( mydata->curr->cs, command );

    // convert the event command into LScript format

    mydata->curr->ls = (char *)malloc(strlen(command) + 10);
    *mydata->curr->ls = 0;

    // the first space will delimit the command from its arguments

    strcpy(buf,command);
    h = strchr(buf,' ');

    // if there is no space, this command lacks arguments

    if(!h)
    {
        strcpy(mydata->curr->ls,command);
        strcat(mydata->curr->ls,"();");
    }
    else
    {
        *h++ = 0;
        strcpy(mydata->curr->ls,buf);
        strcat(mydata->curr->ls,"(");

        // every space after this is an argument separator

        while(h)
        {
            t = strchr(h,' ');
            if(t) *t++ = 0;

            if(strcmp(h,"(null)"))
                strcat(mydata->curr->ls,h);

            if(t)
                strcat(mydata->curr->ls,",");

            h = t;
        }

        strcat(mydata->curr->ls,");");
    }

    return((double)0.0);
}

XCALL_(static int) Interface(long version,
                             GlobalFunc *global,
                             void *inst,
                             void *serverData)
{
    MCData          *mydata;
    MCCommand       *t;
    FILE            *out;
    const   char    *tmp;
    char            buf[256];
    LWSceneInfo     *sceneInfo;

    XCALL_INIT;

    mydata = (MCData *)inst;

    if((tmp = getenv("TEMP")) == NULL)
    {
        if((tmp = getenv("TMP")) == NULL)
            tmp = "";
    }

    sceneInfo = (*global)("LW Scene Info", GFUSE_TRANSIENT);

    sprintf(buf,"%s/macro.ls",tmp);
    out = fopen(buf,"w");

    fprintf(out,"// Generated from scene \"%s\"\n//\n\n",sceneInfo->filename);
    fprintf(out,"@version 2.0\n\ngeneric\n{\n");
    t = mydata->head;
    while(t)
    {
        if(t->tick)
            fprintf(out,"    sleep(%d);\n",t->tick * 1000);
        fprintf(out,"    // %s\n    %s\n    NextFrame();PreviousFrame();\n\n",t->cs,t->ls);
        t = t->next;
    }

    fprintf(out,"}\n");
    fclose(out);

    (*message->info)("LS/GN macro generated to:",buf);

    // clear the event list for another generation

    while(mydata->head)
    {
        t = mydata->head->next;
        if(mydata->head->cs) free(mydata->head->cs);
        if(mydata->head->ls) free(mydata->head->ls);
        free(mydata->head);
        mydata->head = t;
    }

    return(AFUNC_OK);
}

ServerRecord ServerDesc[] = {
  { LWMASTER_HCLASS, "LW_MacroRecorder", Activate },
  { LWMASTER_ICLASS, "LW_MacroRecorder", Interface },
  { NULL }
};
