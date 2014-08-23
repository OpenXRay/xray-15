/****
 * Layout Generic Class
 * LWXPanels Example
 **
 * Copyright 1999 NewTek, Inc. 
 * Author: Ryan Mapes
 * Last Modified: 9.9.1999
 ****
 */

#include <stdio.h>
#include <string.h>

#include <lwserver.h>
#include <lwglobsrv.h>
#include <lwgeneric.h>
#include <lwxpanel.h>

GlobalFunc *g_lwglobal = NULL;

typedef enum en_TPID {
    // some control IDs
    TPC_PARENT = 0x8001,
    TPC_SISTER,
    TPC_BROTHER,
    TPC_AUNT,
    TPC_UNCLE,
    // And some group IDs
    TPG_ENABLE,
    TPG_TAB
};

    void
TP_cb_notify (
    LWXPanelID pan,
    unsigned long cid,
    unsigned long vid,
  int event_type )
{
  int   ival = 0;
  float fval = (float)0.0;
  char *str_ptr = NULL;

  LWXPanelFuncs *lwxpf = NULL;

  if ( !pan ) return;
  lwxpf = (*g_lwglobal) ( LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT);

  if ( lwxpf ) {
    ival = FGETINT(lwxpf,pan,TPC_PARENT);
    fval = (float)FGETFLT(lwxpf,pan,TPC_SISTER);
    fval = (float)FGETFLT(lwxpf,pan,TPC_BROTHER);
    fval = (float)FGETFLT(lwxpf,pan,TPC_AUNT);
    fval = (float)FGETFLT(lwxpf,pan,TPC_UNCLE);
  }

  return;
}

    XCALL_(int)
TP_Activate (
    long                     version,
    GlobalFunc              *global,
    LWLayoutGeneric         *local,
    void                    *serverData)
{

    LWXPanelID     pan   = NULL;
    LWXPanelFuncs *lwxpf = NULL;

    int ival = 0;
    float fval = 0;

    static LWXPanelControl ctrl_list[] = {
        { TPC_PARENT,  "Parent",  "iBoolean" },
        { TPC_SISTER,  "Sister",  "float" },
        { TPC_BROTHER, "Brother", "distance" },
        { TPC_AUNT,    "Aunt",    "percent" },
        { TPC_UNCLE,   "Uncle",   "angle" },
        {0}
    };

    static LWXPanelDataDesc data_descrip[] = {
        { TPC_PARENT,  "Parent",  "integer" },
        { TPC_SISTER,  "Sister",  "float" },
        { TPC_BROTHER, "Brother", "distance" },
        { TPC_AUNT,    "Aunt",    "percent" },
        { TPC_UNCLE,   "Uncle",   "angle" },
        {0},
    };

    static LWXPanelHint hint[] = {
        XpLABEL(0,"LWXPanel LayoutGeneric Example"),
        XpCHGNOTIFY(TP_cb_notify),
    {0}
    };

    XCALL_INIT;
    if (version < 2) return AFUNC_BADVERSION;

  g_lwglobal = global;

    lwxpf = (*global) ( LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT);
    if (!lwxpf)
        return AFUNC_BADGLOBAL;

    // Create panel
    pan = (*lwxpf->create)(LWXP_FORM, ctrl_list);

    if ( pan ) {

        // Describe data
        (*lwxpf->describe)( pan, data_descrip, NULL, NULL );

        // Apply some hints
        (*lwxpf->hint)(pan, 0, hint);

        // Some initial values
        FSETINT(lwxpf,pan,TPC_PARENT,1);
        FSETFLT(lwxpf,pan,TPC_SISTER,24);
        FSETFLT(lwxpf,pan,TPC_BROTHER,10);
        FSETFLT(lwxpf,pan,TPC_AUNT,0.55);
        FSETFLT(lwxpf,pan,TPC_UNCLE,3.141592);

        // Open Panel
        if ( (*lwxpf->post)(pan) ) {
            // Ok
            ival = FGETINT(lwxpf,pan,TPC_PARENT);
            fval = (float)FGETFLT(lwxpf,pan,TPC_SISTER);
            fval = (float)FGETFLT(lwxpf,pan,TPC_BROTHER);
            fval = (float)FGETFLT(lwxpf,pan,TPC_AUNT);
            fval = (float)FGETFLT(lwxpf,pan,TPC_UNCLE);
        }
        else {
            // Cancel
        }

        // Destroy panel
        (*lwxpf->destroy)(pan);

    } // End if panel

    return AFUNC_OK;
}

ServerRecord ServerDesc[] = {
    { "LayoutGeneric", "LWXPanelExample", TP_Activate },
    { NULL }
};

