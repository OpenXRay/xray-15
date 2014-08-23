#include    <stdio.h>
#include    <stdlib.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include    <windows.h>
#endif

#include    <lwsdk/lwserver.h>
#include    <lwsdk/lwhost.h>
#include    <lwsdk/lwgeneric.h>
#include    <lwsdk/lwlcore.h>

const char  *ui_script[] = {
                         "@define MSG     \"This is cool!\"",

                         "msg_x = 101;",
                         "c1..5;",

                         "options",
                         "{",
                             "reqbegin(\"LScript Universal Requester test\");",
                             "reqsize(240,280);",

                             "c1 = ctlpopup(\"Mode\",lsur_mode,@\"World\",\"Local\"@);",
                             "ctlposition(c1,10,10);",

                             "c2 = ctlchoice(\"X\",lsur_axis,@\"Off\",\"c\",\"-\",\"+\"@);",
                             "ctlposition(c2,10,40);",
                             "c3 = ctldistance(\"X Pos\",lsur_abs);",
                             "ctlposition(c3,10,70);",

                             "c4 = ctlpopup(\"Scale Axis\",lsur_scaleAxis,@\"None\",\"All\",\"X\",\"Y\",\"Z\",\"XY\",\"XZ\",\"YZ\"@);",
                             "ctlposition(c4,10,100);",

                             "c5 = ctlinfo(100,30,\"info_redraw\");",
                             "ctlposition(c5,10,150);",

                             "if(reqpost(\"marquee\",50))",
                             "{",
                                 "lsur_mode = getvalue(c1);",
                                 "lsur_axis = getvalue(c2);",
                                 "lsur_abs = getvalue(c3);",
                                 "lsur_scaleAxis = getvalue(c4);",
                             "}",

                             "reqend();",
                         "}",

                         "info_redraw",
                         "{",
                          "drawbox(<132,130,132>,0,0,100,30);",
                             "if(msg_x > 100)",
                                "msg_x = -1 * drawtextwidth(MSG);",
                             "drawtext(MSG,<0,0,0>,msg_x, integer((30 - drawtextheight(MSG)) / 2));",
                             "drawborder(0,0,100,30,true);",
                         "}",

                         "marquee",
                         "{",
                             "msg_x += 2;",
                             "requpdate(c5);",
                         "}",
                         NULL
                         };

static int Activate ( long              version,
                      GlobalFunc        *global,
                      LWLayoutGeneric   *gen,
                      void              *serverData)
{
    const char      *fname;
    LSURFuncs       *lsurFunc;
    LWMessageFuncs  *msgFuncs;
    LWLSURID        script;
    char            *messages[10];
    int             x,ok,err;
    int             lsur_mode,lsur_axis,lsur_scaleAxis;
    double          lsur_abs;
    void            *vars[4];

    lsurFunc = (*global)(LWLSUR_GLOBAL,GFUSE_TRANSIENT);
    msgFuncs = (*global)(LWMESSAGEFUNCS_GLOBAL,GFUSE_TRANSIENT);

    script = (*lsurFunc->compile)(ui_script,messages,10);

    for(x = 0,err = 0;x < 10;x++)
    {
        if(!messages[x]) break;

        // could be a problem with the script.
        // Info messages are prefixed with "i#",
        // Warnings by "w#", and Errors by "e#".

        if(!strncmp(messages[x],"e#",2))
        {
            ++err;
            (*msgFuncs->error)(&messages[x][2],"");
        }

        free(messages[x]);
    }

    if(err) return(AFUNC_OK);

    // set the initial values for the controls

    lsur_mode = 1;
    lsur_axis = 1;
    lsur_abs = 1.0;
    lsur_scaleAxis = 2;

    vars[0] = (void *)&lsur_mode;
    vars[1] = (void *)&lsur_axis;
    vars[2] = (void *)&lsur_abs;
    vars[3] = (void *)&lsur_scaleAxis;

    ok = (*lsurFunc->post)(script,vars);
    (*lsurFunc->release)(script);

    if(ok)
    {
        char    buf[300];

        // user pressed "Ok", and values have been updated

        sprintf(buf,"%d %d %g %d",
                lsur_mode,
                lsur_axis,
                lsur_abs,
                lsur_scaleAxis);

        (*msgFuncs->info)(buf,"");
    }

    return(AFUNC_OK);
}

ServerRecord ServerDesc[] = {
  { LWLAYOUTGENERIC_CLASS, "TestUniveralRequester", Activate },
  { NULL }
};
