/****
 * Channel Handler Class
 * LWXPanels Example
 **
 * Copyright 1999 NewTek, Inc. 
 * Author: Ryan Mapes
 * Last Modified: 9.9.1999
 ****
 */

// General includes
#include <stdio.h>
#include <stdlib.h>

// LightWave includes
#include <lwserver.h>
#include <lwchannel.h>
#include <lwrender.h>
#include <lwxpanel.h>

//// Defines ////

//// Globals ////
GlobalFunc        *g_global = NULL;

////////////////////////////////
//    Handler Declarations    //
////////////////////////////////

//// Handler Data Instance ////
typedef struct st_CHANDATA {
  int is_upper;
  int is_lower;
  double upper;
  double lower;
} CHData;

////////////////////////////////
//   Interface Declarations   //
////////////////////////////////

// Control ID list
// NOTE: ID values must begin above 0x8000
// (lower values are reserved for internal use)

typedef enum en_IDLIST {
    // some control IDs
    CH_ENABLE_UPPER = 0x8001,
    CH_ENABLE_LOWER,
    CH_UPPER,
    CH_LOWER,
} en_idlist;

// These are declared static global for ease of reuse
// between the interface examples.
// The only requirement is the data description must persist
// the duration of the panel's existance.  In some cases, the
// panel itself may outlive a given handler instance since a
// panel may be shared or reused for each data instance of a
// handler server class for (see docs).

static LWXPanelControl ctrl_list[] = {
    { CH_ENABLE_UPPER, "Enable Upper Bound",   "iBoolean" },
    { CH_UPPER,        "Upper Bound",          "distance" },
    { CH_ENABLE_LOWER, "Enable Lower Bound",   "iBoolean" },
    { CH_LOWER,        "Lower Bound",          "distance" },
    {0}
};
static LWXPanelDataDesc data_descrip[] = {
    { CH_ENABLE_UPPER, "Enable Upper Bound",   "integer" },
    { CH_UPPER,        "Upper Bound",          "distance" },
    { CH_ENABLE_LOWER, "Enable Lower Bound",   "integer" },
    { CH_LOWER,        "Lower Bound",          "distance" },
    {0},
};

// These are common configuration hints
static LWXPanelHint common_hint[] = {
  XpENABLE_(CH_ENABLE_UPPER),XpH(CH_UPPER),XpEND,
  XpENABLE_(CH_ENABLE_LOWER),XpH(CH_LOWER),XpEND,
  XpEND
};

////////////////
// Interface  //
////////////////
////
// There are four panel interfaces which each presents a
// user interface using a different technique. Regardless of
// the interface used, the corresponding handler is the same
// with simply different data instances.
////
// Interface Activation creates the interface.
// It can either post a modal dialog itself or return the panel
// to Lightwave to display.
//
// _FNM - FORM panel NonModal
// _FM  - FORM panel Modal
// _VNM - VIEW panel NonModal
// _VM  - VIEW panel Modal
////


//// FORM Panel Interface Support Routines ////
// The following routines are only required for FORM type panels

// The following routine is only necessary for a FORM type panel.
// In this example, the plugin ("client") is maintaining its
// own data instance. Therefore, the client needs to retrieve
// the updated values from the panel and store them into its
// internal data instance. This is only required for FORM panels
// since VIEW panels update our data instance directly
// via the CH_view_get and CH_view_set methods.

void CH_cb_notify ( LWXPanelID panID, unsigned long cid, unsigned long vid, int event_type ) {

  void *value = NULL;

  CHData *inst = NULL;

  LWXPanelFuncs *lwxpf = NULL;

  lwxpf = (*g_global) ( LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT);
    if (!lwxpf) return;

  // A value in the panel has changed.
  // Need to update our data instance

  // Retrive the data instance from the panel
  // This was stored in the panel when the panel was created
  inst = (CHData*)(*lwxpf->getData)(panID, 0);

  if ( inst ) {
    switch ( vid ) {
      case ( CH_ENABLE_UPPER ):
    inst->is_upper = FGETINT ( lwxpf, panID, vid );

    break;
      case ( CH_ENABLE_LOWER ):
    inst->is_lower = FGETINT ( lwxpf, panID, vid );
    break;
      case ( CH_UPPER ):
    inst->upper = FGETFLT ( lwxpf, panID, vid );
    break;
      case ( CH_LOWER ):
    inst->lower = FGETFLT ( lwxpf, panID, vid );
    break;
    } // End switch vid

  }

  return;
}


//// VIEW Panel Interface Support Routines ////

// The following routines are only required for VIEW type panels

void *CH_view_get ( void *myinst, unsigned long vid ) {

  CHData *inst = (CHData*)(myinst);

  void *result = NULL;
  static int ival = 0; // static so persist after function returns
  static double dval = 0.0;

  if ( !inst ) return result;

  switch ( vid ) {
    case (CH_ENABLE_UPPER):
      ival = inst->is_upper;
      result = &ival;
      break;
    case (CH_ENABLE_LOWER):
      ival = inst->is_lower;
      result = &ival;
      break;
    case (CH_UPPER):
      dval = inst->upper;
      result = &dval;
      break;
    case (CH_LOWER):
      dval = inst->lower;
      result = &dval;
      break;
    default:
      result=NULL; // unknown vid
      break;
  } // End switch vid

  return result;
}

int CH_view_set ( void *myinst, unsigned long vid, void *value ) {

  CHData *inst = (CHData*)(myinst);

  int rc=0; // return code: true --> setfunc handled vid

  if ( !inst ) return rc;

  switch ( vid ) {
    case (CH_ENABLE_UPPER):
      inst->is_upper = *((int*)value);
      break;
    case (CH_ENABLE_LOWER):
      inst->is_lower = *((int*)value);
      break;
    case (CH_UPPER):
      inst->upper = *((double*)value);
      break;
    case (CH_LOWER):
      inst->lower = *((double*)value);
      break;
    default:
      break;
  } // End switch vid

  return rc;
}


//// NonModal Panel Interface Support Routines ////
void CH_cb_destroy ( LWXPanelID panID ) {

  void *mydata = NULL;

  LWXPanelFuncs *lwxpf = NULL;

  lwxpf = (*g_global) ( LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT);
    if (!lwxpf) return;

  // Get data instance from panel
  // (not applicable here but this is how one _would_ do it)

  mydata = (*lwxpf->getData)(panID, 0);

  // Release any INTERFACE SPECIFIC allocations

  // (none)

  // Destroy the panel
    (*lwxpf->destroy)(panID);

  return;
}

//// Modal Panel Interface Support Routines ////

// These post the interfaces for modal panels.
// These are also the functions given to Lightwave via
// the LWInterface "options" pointer.

LWError CH_iface_ModalView ( LWInstance myinst ) {

  const char *err = NULL;

  LWXPanelFuncs *lwxpf = NULL;
  LWXPanelID     panID = NULL;

  CHData *inst = (CHData*)myinst;

  static LWXPanelHint hint[] = {
    XpLABEL(0,"XPanelExample - FORM - Modal"),
    XpCHGNOTIFY(CH_cb_notify),
      XpEND
  };

    lwxpf = (LWXPanelFuncs*)(*g_global)( LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT);
    if ( lwxpf ) {
    panID = (*lwxpf->create)( LWXP_VIEW, ctrl_list );
        if(panID) {

      // Apply some hints
      (*lwxpf->hint) ( panID, 0, common_hint );
      (*lwxpf->hint) ( panID, 0, hint );

      // Describe the structure of the data instance to the panel
      (*lwxpf->describe)( panID, data_descrip, CH_view_get, CH_view_set );

      // Now give the data instance to the panel
      (*lwxpf->viewInst)( panID, inst );

      // Set the userdata
      (*lwxpf->setData)(panID, 0, inst);

      // Post the panel
      if ( (*lwxpf->post) ( panID ) ) {

    // Ok processing
    // This is a VIEW panel so the data instance is already up to date

      } // End if ok
      else {

    // Cancel processing
    // This is a VIEW panel so recovery of original state
    // should already be applied to the data instance

      } // End else cancel

      // Destroy the panel
      (*lwxpf->destroy) ( panID );
    }
  }

  return err;
}

LWError CH_iface_ModalForm ( LWInstance myinst ) {
  const char *err = NULL;

  LWXPanelFuncs *lwxpf = NULL;
  LWXPanelID     panID = NULL;

  CHData *inst = (CHData*)myinst;

  static LWXPanelHint hint[] = {
    XpLABEL(0,"XPanelExample - FORM - Modal"),
    XpCHGNOTIFY(CH_cb_notify),
      XpEND
  };

    lwxpf = (LWXPanelFuncs*)(*g_global)( LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT);
    if ( lwxpf ) {
    panID = (*lwxpf->create)( LWXP_FORM, ctrl_list );
        if(panID) {

      // Apply some hints
      (*lwxpf->hint) ( panID, 0, common_hint );
      (*lwxpf->hint) ( panID, 0, hint );

      // Describe the structure of the data instance to the panel
      (*lwxpf->describe)( panID, data_descrip, NULL, NULL );

      // Set some initial values
            FSETINT ( lwxpf, panID, CH_ENABLE_UPPER, inst->is_upper );
            FSETINT ( lwxpf, panID, CH_ENABLE_LOWER, inst->is_lower );
            FSETFLT ( lwxpf, panID, CH_UPPER, inst->upper );
            FSETFLT ( lwxpf, panID, CH_LOWER, inst->lower );

      // Set the userdata
      (*lwxpf->setData)(panID, 0, inst);

      // Post the panel
      if ( (*lwxpf->post) ( panID ) ) {

    // Ok processing
    // This is a VIEW panel so the data instance is already up to date

                inst->is_upper = FGETINT ( lwxpf, panID, CH_ENABLE_UPPER );
                inst->is_lower = FGETINT ( lwxpf, panID, CH_ENABLE_LOWER );
                inst->upper    = FGETFLT ( lwxpf, panID, CH_UPPER );
                inst->lower    = FGETFLT ( lwxpf, panID, CH_LOWER );

      } // End if ok
      else {

    // Cancel processing
    // This is a VIEW panel so recovery of original state
    // should already be applied to the data instance

      } // End else cancel

      // Destroy the panel
      (*lwxpf->destroy) ( panID );
    }
  }

  return err;
}

//// NonModal Panel Interfaces ////
// NonModal interfaces are created on activation
// and the LWInterface "panel" field is set with the LWXPanelID pointer
// The panel will be displayed later when a handler needs to be view
// This same interface may also be used, or even shared, by multiple handler instances
int CHIFaceActivate_FNM ( long version,
              GlobalFunc *global,
              void *local,
              void *serverdata ) {
  int rc = AFUNC_OK;

    LWXPanelFuncs *lwxpf = NULL;
  LWXPanelID     panID = NULL;

  LWInterface *iface = (LWInterface*)local;
  CHData *inst = (CHData*)iface->inst;

  static LWXPanelHint hint[] = {
    XpLABEL(0,"XPanelExample - FORM - NonModal"),
    XpCHGNOTIFY(CH_cb_notify),
    XpDESTROYNOTIFY(CH_cb_destroy),
      XpEND
  };

  // Check the version
  // NOTE: The interface version is different from handler version
  if ( version < LWINTERFACE_VERSION ) {
    rc = AFUNC_BADVERSION;
  }
  else {

    // Store the global for later
    g_global = global;

    // Create panel
      lwxpf = (LWXPanelFuncs*)(*g_global)( LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT);
      if ( lwxpf ) {
      panID = (*lwxpf->create)( LWXP_FORM, ctrl_list );
          if(panID) {

        // Apply some hints
        (*lwxpf->hint) ( panID, 0, common_hint );
        (*lwxpf->hint) ( panID, 0, hint );

        // Describe the structure of the data instance to the panel
        (*lwxpf->describe)( panID, data_descrip, NULL, NULL );

        // Some initial values
        FSETINT ( lwxpf, panID, CH_ENABLE_UPPER, 1 );
        FSETINT ( lwxpf, panID, CH_ENABLE_LOWER, 0 );
        FSETFLT ( lwxpf, panID, CH_UPPER, 1.0 );
        FSETFLT ( lwxpf, panID, CH_LOWER, 0.0 );

        // Set the userdata
        (*lwxpf->setData)(panID, 0, inst);

      }
    }

    // Set the interface panel pointer
    iface->panel = panID;

  } // End else good version

  // Return and let LightWave play with the panel :)
  return rc;
}

int CHIFaceActivate_VNM ( long version,
              GlobalFunc *global,
              void *local,
              void *serverdata ) {
  int rc = AFUNC_OK;

    LWXPanelFuncs *lwxpf = NULL;
  LWXPanelID     panID = NULL;

  LWInterface *iface = (LWInterface*)local;
  CHData *inst = (CHData*)iface->inst;

  static LWXPanelHint hint[] = {
    XpLABEL(0,"XPanelExample - VIEW - NonModal"),
    XpCHGNOTIFY(CH_cb_notify),
    XpDESTROYNOTIFY(CH_cb_destroy),
      XpEND
  };

  // Check the version
  // NOTE: The interface version is different from handler version
  if ( version < LWINTERFACE_VERSION ) {
    rc = AFUNC_BADVERSION;
  }
  else {

    // Store the global for later
    g_global = global;

    // Create panel
      lwxpf = (LWXPanelFuncs*)(*g_global)( LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT);
      if ( lwxpf ) {
      panID = (*lwxpf->create)( LWXP_VIEW, ctrl_list );
          if(panID) {

    // Apply some hints
    (*lwxpf->hint) ( panID, 0, common_hint );
    (*lwxpf->hint) ( panID, 0, hint );

    // Describe the structure of the data instance to the panel
    (*lwxpf->describe)( panID, data_descrip, CH_view_get, CH_view_set );

    // Now give the data instance to the panel
    (*lwxpf->viewInst)( panID, inst );

    // Set the userdata
    (*lwxpf->setData)(panID, 0, inst);

      }
    }

    // Set the interface panel pointer
    iface->panel = panID;

  } // End else good version

  // Return and let LightWave play with the panel :)
  return rc;
}

//// Modal Panel Interfaces ////
// The activation here simply sets the LWInterface "options"
// pointer to the function which opens a modal dialog. The
// function will be called later to open the panel.

// The view type panel
int CHIFaceActivate_VM ( long version,
              GlobalFunc *global,
              void *local,
              void *serverdata ) {
  int rc = AFUNC_OK;

  LWInterface *iface = (LWInterface*)local;

  // Check the version
  // NOTE: The interface version is different from handler version
  if ( version < LWINTERFACE_VERSION ) {
    rc = AFUNC_BADVERSION;
  }
  else {

    // Store the global for later
    g_global = global;

    // Set the function pointer
    iface->options = CH_iface_ModalView;

  } // End else good version

  return rc;
}

// The form type panel
int CHIFaceActivate_FM ( long version,
              GlobalFunc *global,
              void *local,
              void *serverdata ) {
  int rc = AFUNC_OK;

  LWInterface *iface = (LWInterface*)local;

  // Check the version
  // NOTE: The interface version is different from handler version
  if ( version < LWINTERFACE_VERSION ) {
    rc = AFUNC_BADVERSION;
  }
  else {

    // Store the global for later
    g_global = global;

    // Set the function pointer
    iface->options = CH_iface_ModalForm;

  } // End else good version

  return rc;
}

////////////////////////////////
//         Handler            //
////////////////////////////////
////
// This sample channel handler simply "governs" the channel
// value within an upper or lower bounds
////
static LWInstance CH_create ( void *priv,
                  void *context,
                  LWError *err ) {

  CHData *inst = NULL;

  XCALL_INIT;

  inst = malloc(sizeof(CHData));

  if( !inst) {
        *err = "Instance allocation failed.";
  }
    else {
    inst->is_upper = 0;
    inst->is_lower = 0;
    inst->upper    = 1.0;
    inst->lower    = 0.0;
  }

  return (LWInstance)inst;
}

static void CH_destroy ( LWInstance inst ) {
  XCALL_INIT;
  if( inst) free ( inst );
  return;
}

static LWError CH_copy ( LWInstance to, LWInstance from ) {

  CHData *TO   = (CHData*)to;
    CHData *FROM = (CHData*)from;

  XCALL_INIT;

    TO->is_upper = FROM->is_upper;
  TO->is_lower = FROM->is_lower;
    TO->upper    = FROM->upper;
  TO->lower    = FROM->lower;

  return NULL;
}


static LWError CH_load ( LWInstance inst,
             const LWLoadState *lState ) {
  XCALL_INIT;
  return NULL;
}

static LWError CH_save ( LWInstance inst,
             const LWSaveState *sState ) {
  XCALL_INIT;
  return NULL;
}

static const char *CH_descln ( LWInstance inst ) {
  static const char *desc = "Hello World!";
  XCALL_INIT;
  return desc;
}

static void     CH_evaluate ( LWInstance myinst,
              const LWChannelAccess *access ) {

  CHData *inst = (CHData*)myinst;
  double value = 0.0;

  XCALL_INIT;

  if ( !inst ) return;

    // Get value at this time
    access->getChannel( access->chan, access->time, &value);

  // Upper Bound
  if ( (inst->is_upper) &&
       (value > inst->upper) ) {
      // set values for current time
      access->setChannel( access->chan, inst->upper);
  }

  // Lower Bound
  if ( (inst->is_lower) &&
       (value < inst->lower) ) {
      // set values for current time
      access->setChannel( access->chan, inst->lower);
  }

  return;
}

static unsigned int CH_flags ( LWInstance inst ) {
  XCALL_INIT;
  return 0; // no flags desired
}

//// Handler Activation ////
int CH_HandActivate ( long version,
              GlobalFunc *global,
              void *local,
              void *serverdata ) {
  int rc = AFUNC_OK;

    LWChannelHandler *handler = (LWChannelHandler*)local;

  if ( version < LWCHANNEL_VERSION ) {
        rc = AFUNC_BADVERSION;
  }
  else {
      handler->inst->copy     = CH_copy;
      handler->inst->create   = CH_create;
      handler->inst->descln   = CH_descln;
      handler->inst->destroy  = CH_destroy;
      handler->inst->load     = CH_load;
      handler->inst->save     = CH_save;
      handler->evaluate       = CH_evaluate;
      handler->flags          = CH_flags;
  }
  return rc;
}

// These just map the activation functions
int CH_HandActivate_FNM (
    long version, GlobalFunc *global, void *local, void *serverdata )
{
  return (CH_HandActivate ( version, global, local, serverdata ));
}
int CH_HandActivate_FM (
    long version, GlobalFunc *global, void *local, void *serverdata )
{
  return (CH_HandActivate ( version, global, local, serverdata ));
}
int CH_HandActivate_VNM (
    long version, GlobalFunc *global, void *local, void *serverdata )
{
  return (CH_HandActivate ( version, global, local, serverdata ));
}
int CH_HandActivate_VM (
    long version, GlobalFunc *global, void *local, void *serverdata )
{
  return (CH_HandActivate ( version, global, local, serverdata ));
}

////////////////////////////////
//       Server Record        //
////////////////////////////////
// And, finally, the all important server record

ServerRecord ServerDesc[] = {
  { LWCHANNEL_HCLASS, "XPanelExample_FORM_NonModal", CH_HandActivate_FNM },
    { LWCHANNEL_HCLASS, "XPanelExample_FORM_Modal",    CH_HandActivate_FM },
    { LWCHANNEL_HCLASS, "XPanelExample_VIEW_NonModal", CH_HandActivate_VNM },
    { LWCHANNEL_HCLASS, "XPanelExample_VIEW_Modal",    CH_HandActivate_VM },
    { LWCHANNEL_ICLASS,     "XPanelExample_FORM_NonModal",  CHIFaceActivate_FNM },
  { LWCHANNEL_ICLASS,   "XPanelExample_FORM_Modal",       CHIFaceActivate_FM },
  { LWCHANNEL_ICLASS,   "XPanelExample_VIEW_NonModal",  CHIFaceActivate_VNM },
  { LWCHANNEL_ICLASS,   "XPanelExample_VIEW_Modal",       CHIFaceActivate_VM },
    { NULL }
};
