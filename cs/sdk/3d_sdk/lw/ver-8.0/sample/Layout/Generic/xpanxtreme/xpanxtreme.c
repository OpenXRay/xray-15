/****
 * Layout Generic Class
 * LWXPanels XTreme Example
 **
 * Copyright 1999 NewTek, Inc. 
 * Author: Ryan Mapes
 * Last Modified: 10.26.1999
 ****
 */

#include <stdio.h>
#include <string.h>

#include <lwserver.h>
#include <lwhost.h>
#include <lwglobsrv.h>
#include <lwgeneric.h>
#include <lwxpanel.h>

#define MIN(a,b)   ( ((a) < (b)) ? (a) : (b) )

//// Pointers to LightWave Global Services
GlobalFunc     *g_lwglobal = NULL;
LWXPanelFuncs  *g_lwxpf    = NULL;
LWMessageFuncs *g_msgf     = NULL;


//// Enum Definitions

// Some common control and group IDs
enum en_TPID {
  // controls
  TPC_ENABLE = 0x8000,
  TPC_PARENT,
  TPC_SISTER,
  TPC_BROTHER,
  TPC_AUNT,
  TPC_UNCLE,
  // groups
  TPG_ENABLE,
  TPG_COLORS,
  TPG_BAS,
  TPG_ADV,
  TPG_TAB,
  TPG_TABTWO
};


// Alternate method for ID definition
// (these are only used in the main panel)
#define TPM_PANEL               0

#define TPM_BUT_BASE            LWID_('B','A','S','E')
#define TPM_BUT_THUMB           LWID_('T','H','U','M')

#define TPM_BOL_ENABLE          LWID_('E','N','B','L')
#define TPM_POP_POP             LWID_('P','O','P',' ')
#define TPM_SLD_SLIDER          LWID_('R','A','N','G')
#define TPM_CHO_CHOICE          LWID_('A','L','T','R')
#define TPM_CMD_POPCMD          LWID_('C','A','L','C')
#define TPM_COL_COLOR           LWID_('C','O','L','R')

// A few layout group IDs
#define TPM_GRP_BUTTONS         LWID_('G','R','P','A')
#define TPM_GRP_ENABLE          LWID_('G','R','P','B')
#define TPM_GRP_STACK           LWID_('S','T','C','K')
#define TPM_GRP_SETA            LWID_('S','E','T','A')
#define TPM_GRP_SETB            LWID_('S','E','T','B')


//// Global Variables

int pop_show[] = {-1,2,1,0};

const char *pop_strlist[] = {
  "(none)",
  "Other Panels",
  "Things",
  "Ummm....",
  NULL
};

const char *cho_strlist[] = {
  "(none)",
  "Translate",
  "Rotate",
  "Scale",
  "Morf",
  NULL
};

const char *cmd_strlist[] = {
  "Half",
  "Normal",
  "Double",
  NULL
};




////////////////////////////////
////////////////////////////////
////     Basic Example      ////
////////////////////////////////
////////////////////////////////

  void
BASE_cb_notify (
  LWXPanelID pan,
  unsigned long cid,
  unsigned long vid,
  int event_code )
{
  return;
}

  void
BASE_panel (
    LWXPanelID       panel,
  int             ctrl_id )
{
  LWXPanelID pan = NULL;
  int ival = 0;
  double dval = 0;

  static LWXPanelControl ctrl_list[] = {
    { TPC_ENABLE,  "Enable",  "iBoolean" },
    { TPC_PARENT,  "Parent",  "iSlider" },
    { TPC_SISTER,  "Sister",  "iSlider" },
    { TPC_BROTHER, "Brother", "iSliderText" },
    { TPC_AUNT,    "Aunt",    "float3" },
    { TPC_UNCLE,   "Uncle",   "angle3" },
    { 0 }
  };
  static LWXPanelDataDesc data_desc[] = {
    { TPC_ENABLE,  "Enable",  "integer" },
    { TPC_PARENT,  "Parent",  "integer" },
    { TPC_SISTER,  "Sister",  "integer" },
    { TPC_BROTHER, "Brother", "integer" },
    { TPC_AUNT,    "Aunt",    "float3" },
    { TPC_UNCLE,   "Uncle",   "angle3" },
    { 0 }
  };
  static char *veclbl[] = {{"-X-"},{"Y"},{"z"}};

  LWXPanelHint hint[] = {
    XpLABEL         ( 0, "Basic Panel" ),

    XpCHGNOTIFY     ( BASE_cb_notify ),
    // Set some ranges
    XpLABEL         ( TPC_PARENT, "Parent" ),
    XpRANGE         ( TPC_PARENT, -100, 100, 1 ),

    // This control's value is the parent's value (data) not its own data
    XpLABEL         ( TPC_SISTER, "Sister" ),
    XpRANGE         ( TPC_SISTER, -100, 100, 1 ),
    XpVALUE         ( TPC_SISTER, TPC_PARENT ),

    // This control is linked
    // This means it has its own data but updates when parent updates
    XpLABEL         ( TPC_BROTHER, "Brother" ),
    XpMIN           ( TPC_BROTHER, -100 ),
    XpMAX           ( TPC_BROTHER, 100 ),
    XpSTEP          ( TPC_BROTHER, 1 ),
    XpLINK          ( TPC_BROTHER, TPC_PARENT ),

    XpRANGE         ( TPC_AUNT, -250, 100, 5 ),
    XpRANGE         ( TPC_UNCLE, -90, 90, 1 ),

    // Vector element labels
    XpVECLABEL              ( TPC_AUNT, veclbl ),

    // Add some groupings
    XpALIAS_(TPG_ENABLE),
      XpH(TPC_SISTER),
      XpH(TPC_BROTHER),
      XpH(0),

    XpGROUP_(TPG_ENABLE),
      XpH(TPC_SISTER),
      XpH(TPC_BROTHER),
      XpH(0),
    XpLABEL ( TPG_ENABLE, "Children" ),

    XpALIAS_(TPG_ENABLE),
      XpH(TPC_SISTER),
      XpH(TPC_BROTHER),
      XpH(0),

    XpALIAS_(TPG_ENABLE),
      XpH(TPC_SISTER),
      XpH(TPC_BROTHER),
      XpH(0),

    XpALIAS_(TPG_ENABLE),
      XpH(TPC_SISTER),
      XpH(TPC_BROTHER),
      XpH(0),

    XpALIAS_(TPG_ENABLE),
      XpH(TPC_SISTER),
      XpH(TPC_BROTHER),
      XpH(0),

    XpENABLE_(TPC_ENABLE),
      XpH(TPG_ENABLE),
      XpH(0),

    XpTABS_(TPG_TAB),
      XpH(TPC_PARENT),
      XpH(TPG_ENABLE),
      XpH(0),

    XpTABS_(TPG_TABTWO),
      XpH(TPC_AUNT),
      XpH(TPC_UNCLE),
      XpH(0),

    // Select
    XpCTRLFRONT ( TPC_UNCLE ),
    XpDIVADD ( TPC_SISTER ),

    XpEND
  };

  // Create a panel using an ExoCompDefinition
  pan = (*g_lwxpf->create) ( LWXP_FORM, ctrl_list );

  if ( pan ) {

    // Describe the data
    (*g_lwxpf->describe) ( pan, data_desc, NULL, NULL );
    (*g_lwxpf->hint) ( pan, 0, hint );

    // Set some initial values
    FSETINT ( g_lwxpf, pan, TPC_ENABLE, 1 );
    FSETINT ( g_lwxpf, pan, TPC_PARENT, 50 );
    FSETINT ( g_lwxpf, pan, TPC_SISTER, 50 );
    FSETINT ( g_lwxpf, pan, TPC_BROTHER, 50 );
    FSETFLT ( g_lwxpf, pan, TPC_AUNT, 25 );
    FSETFLT ( g_lwxpf, pan, TPC_UNCLE, 3.14 );

    // Post the dialog
    if ( (*g_lwxpf->post) ( pan ) ) {
      // Ok
      ival = FGETINT ( g_lwxpf, pan, TPC_PARENT ),
      ival = FGETINT ( g_lwxpf, pan, TPC_SISTER );
      ival = FGETINT ( g_lwxpf, pan, TPC_BROTHER );
      dval = FGETFLT ( g_lwxpf, pan, TPC_AUNT );
      dval = FGETFLT ( g_lwxpf, pan, TPC_UNCLE );
    } // End if 'Ok'
    else {
      // Cancel
      ival = FGETINT ( g_lwxpf, pan, TPC_PARENT ),
      ival = FGETINT ( g_lwxpf, pan, TPC_SISTER );
      ival = FGETINT ( g_lwxpf, pan, TPC_BROTHER );
      dval = FGETFLT ( g_lwxpf, pan, TPC_AUNT );
      dval = FGETFLT ( g_lwxpf, pan, TPC_UNCLE );
    } // End else cancel

    // Destroy the panel
    (*g_lwxpf->destroy)  ( pan );

  } // End if panel

  return;
}

////////////////////////////////
////////////////////////////////
////   Thumbnails Example   ////
////////////////////////////////
////////////////////////////////

  void
THUM_cb_notify (
  LWXPanelID pan,
  unsigned long cid,
  unsigned long vid,
  int event_code )
{
  void *val = NULL;

  if ( !pan ) return;

  // Invalidate thumbnail if "B" changes
  if ( cid == TPC_SISTER ) FSETINT ( g_lwxpf, pan, TPC_AUNT, 1 );
  if ( cid == TPC_BROTHER ) FSETINT ( g_lwxpf, pan, TPC_UNCLE, 1 );

  return;
}

  void
THUM_cb_drawthum (
  LWXPanelID pan,
  unsigned long cid,
  LWXPDrAreaID reg,
  int w,
  int h )
{
  int i=0;        int j=0;

  int k=0;

  LWXPDrawFuncs *drawf = g_lwxpf->drawf;
  int b_val;

  if ( !pan ) return;
  if ( !reg ) return;

  if ( cid == TPC_AUNT ) b_val = FGETINT ( g_lwxpf, pan, TPC_SISTER );
  if ( cid == TPC_UNCLE ) b_val = FGETINT ( g_lwxpf, pan, TPC_BROTHER );

  for ( j=0; j<h; j++ ) {
    for ( i=0; i<w; i++ ) {
      (*drawf->drawRGBPixel)( reg, MIN(i,255), MIN(j,255), b_val, i, j );
    } // End for i
  } // End for j

  return;
}

  void
THUM_cb_zoomthum (
  LWXPanelID pan,
  unsigned long cid,
  int x, int y,
  int *region )
{

  int dummy=0;

  if ( !pan ) return;

  // Dummy is just something we can use as a debugger breakpoint
  dummy = x;
  dummy += y;
  dummy += region[0];
  dummy += region[1];
  dummy += region[2];
  dummy += region[3];

  return;
}


  void
THUM_panel (
  LWXPanelID       panel,
  int             ctrl_id )
{
  int rc=0;
  LWXPanelID pan = NULL;

  static char *veclbl[] = {{"R"},{"G"},{"B"}};

  LWXPanelControl ctrl_def[] = {
    { TPC_ENABLE,  "",           "color" },
    { TPC_PARENT,  "",           "float3" },
    { TPC_SISTER,  "Blue Value", "integer" },
    { TPC_BROTHER, "",           "iSliderText" },
    { TPC_AUNT,    "Aunt",       "dThumbnail" },
    { TPC_UNCLE,   "Uncle",      "dThumbnail" },
    { 0 }
  };

  LWXPanelDataDesc data_desc[] = {
    { TPC_ENABLE,  "a",      "color" },
    { TPC_PARENT,  "b",      "float3" },
    { TPC_SISTER,  "c",      "integer" },
    { TPC_BROTHER, "d",      "integer" },
    { TPC_AUNT,    "Aunt",   "integer" },
    { TPC_UNCLE,   "Uncle",  "integer" },
    { 0 }
  };

  LWXPanelHint hint[] = {
    // Set some ranges
    XpLABEL ( 0, "Thumbnail Example" ),
    XpCHGNOTIFY ( THUM_cb_notify ),

    XpCTRLCFG ( TPC_AUNT, THUM_XLG | THUM_FULL ),
    XpCTRLCFG ( TPC_UNCLE, THUM_MED | THUM_ANAW ),
    XpDRAWCBFUNC ( TPC_AUNT, THUM_cb_drawthum ),
    XpDRAWCBFUNC ( TPC_UNCLE, THUM_cb_drawthum ),
    XpZOOMCBFUNC ( TPC_AUNT, THUM_cb_zoomthum, 0),
    XpZOOMCBFUNC ( TPC_UNCLE, THUM_cb_zoomthum, 2),

    // configure the color and percent
    XpVALUE  (TPC_ENABLE, TPC_PARENT),
    XpVECLABEL ( TPC_PARENT, veclbl ),
    XpMIN ( TPC_PARENT, 0 ),
    XpMAX ( TPC_PARENT, 1 ),

    // Layout
    XpGROUP_ (TPG_COLORS), XpH(TPC_ENABLE), XpH(TPC_PARENT), XpEND,
    XpGROUP_ (TPG_BAS), XpH(TPC_AUNT), XpH(TPC_SISTER), XpEND,
    XpGROUP_ (TPG_ADV), XpH(TPG_COLORS), XpH(TPC_UNCLE), XpH(TPC_BROTHER), XpEND,
    XpLABEL ( TPG_BAS, "Basic" ),
    XpLABEL ( TPG_ADV, "Advanced" ),
    XpTABS_ ( TPG_TAB ),
      XpH(TPG_BAS), XpH(TPG_ADV), XpEND,

    XpRANGE ( TPC_BROTHER, 0, 255, 1 ),
    XpRANGE ( TPC_SISTER, 0, 255, 1 ),
    XpIMMUPD ( TPC_SISTER, TPC_AUNT ),
    XpIMMUPD ( TPC_BROTHER, TPC_UNCLE ),

    XpLEFT   ( TPC_UNCLE ),
    XpNARROW ( TPC_PARENT ),

    XpEND
  };

  // Create panel pane
  pan = (*g_lwxpf->create) ( LWXP_FORM, ctrl_def );

  if ( pan ) {

    // Apply hints using the PanelSingleHint function
    (*g_lwxpf->hint)( pan, 0, hint );

    (*g_lwxpf->describe)( pan, data_desc, NULL, NULL );

    // Set initial thumbnail value
    FSETINT ( g_lwxpf, pan, TPC_BROTHER, 128 );
    FSETINT ( g_lwxpf, pan, TPC_SISTER, 184 );

    // Open window
    (*g_lwxpf->post) ( pan );

    // Destroy window (also destroys pane)
    (*g_lwxpf->destroy)  ( pan );

  } // End if panel

  return;
}



////////////////////////////////
////////////////////////////////
////      Main Window       ////
////////////////////////////////
////////////////////////////////

  void
MAIN_cb_notify (
  LWXPanelID pan,
  unsigned long cid,
  unsigned long vid,
  int event_code )
{

  int val = 0;
  double *dbl3 = NULL;
  double *agl3 = NULL;

  if ( !pan ) return;

  // Only interested in the parent control
  switch ( cid ) {
    case ( TPC_PARENT ):

      val = FGETINT ( g_lwxpf, pan, TPC_PARENT );

      // Need to update related panel values
      FSETINT ( g_lwxpf, pan, TPC_SISTER, val );
      FSETINT ( g_lwxpf, pan, TPC_BROTHER, (-1*val) );

      break;
    case ( TPC_SISTER ):

      val = FGETINT ( g_lwxpf, pan, TPC_SISTER );

      // Need to update related panel values
      FSETINT ( g_lwxpf, pan, TPC_PARENT, val );
      FSETINT ( g_lwxpf, pan, TPC_BROTHER, (-1*val) );

      break;
    default:
      // nothing
      break;
  } // End switch vid

  return;
}

  void
MAIN_cb_popcmd (
  LWXPanelID panel,
  int cid,
  int cmdid )
{
  (*g_msgf->info) ( "Popup Command Event", cmd_strlist[cmdid] );
  return;
}

  int
MAIN_panel (void)
{

  int rc=0;

  LWXPanelID pan = NULL;

  int  int_val = 0;
  void *parm_val = NULL;

  LWXPanelHint hint[] = {
    // Buttons to open other panels for panel create tests
    XpADD           ( TPM_BUT_BASE,     "vButton", NULL ),
    XpADD           ( TPM_BUT_THUMB,    "vButton", NULL ),

    XpLABEL         ( TPM_PANEL,        "Mother Hen" ),

    XpLABEL         ( TPM_BUT_BASE,     "Basic" ),
    XpLABEL         ( TPM_BUT_THUMB,    "Thumbnails" ),

    XpBUTNOTIFY     ( TPM_BUT_BASE,     BASE_panel ),
    XpBUTNOTIFY     ( TPM_BUT_THUMB,    THUM_panel ),

    // Basic control test
    XpADD           ( TPM_BOL_ENABLE,   "iBoolean",    TPM_BOL_ENABLE ),
    XpLABEL         ( TPM_BOL_ENABLE,   "Enable Controls" ),

    XpADD           ( TPM_POP_POP,      "iPopChoice",  TPM_POP_POP ),
    XpLABEL         ( TPM_POP_POP,      "Example" ),
    XpSTRLIST       ( TPM_POP_POP,      pop_strlist ),

    XpADD           ( TPM_SLD_SLIDER,   "iSlider",     TPM_SLD_SLIDER ),
    XpLABEL         ( TPM_SLD_SLIDER,   "Range" ),
    XpMIN           ( TPM_SLD_SLIDER,   0 ),
    XpMAX           ( TPM_SLD_SLIDER,   320 ),
    XpSTEP          ( TPM_SLD_SLIDER,   2 ),
    XpTRACK         ( TPM_SLD_SLIDER,   1 ),

    XpADD           ( TPM_CHO_CHOICE,   "iChoice",     TPM_CHO_CHOICE ),
    XpLABEL         ( TPM_CHO_CHOICE,   "Mode" ),
    XpSTRLIST       ( TPM_CHO_CHOICE,   cho_strlist ),
    XpORIENT        ( TPM_CHO_CHOICE,   1 ),

    XpADD           ( TPM_CMD_POPCMD,   "vPopCmd",     NULL ),
    XpLABEL         ( TPM_CMD_POPCMD,   "Click Here" ),
    XpSTRLIST       ( TPM_CMD_POPCMD,   cmd_strlist ),
    XpPOPCMDFUNC    ( TPM_CMD_POPCMD,   MAIN_cb_popcmd ),

    // color control not yet supported
    XpADD           ( TPM_COL_COLOR,    "cColor",      TPM_COL_COLOR ),
    XpLABEL         ( TPM_COL_COLOR,    "Color" ),
    XpTRACK         ( TPM_COL_COLOR,    1 ),

    // Control Enable Relationships

    // Layout Hints
    XpGROUP_(TPM_GRP_SETA),
      XpH(TPM_SLD_SLIDER),
      XpH(TPM_CHO_CHOICE),
      XpH(0),
    XpGROUP_(TPM_GRP_SETB),
      XpH(TPM_CMD_POPCMD),
      XpH(TPM_COL_COLOR),
      XpH(0),
    XpGROUP_(TPM_GRP_BUTTONS),
      XpH(TPM_BUT_BASE),
      XpH(TPM_BUT_THUMB),
      XpH(0),
    XpGROUP_(TPM_GRP_ENABLE),
      XpH(TPM_GRP_SETA),
      XpH(TPM_GRP_SETB),
      XpH(0),
    XpENABLEMSG_(TPM_BOL_ENABLE, "Not enabled"),
      XpH(TPM_GRP_ENABLE),
      XpH(TPM_GRP_BUTTONS),
      XpH(0),
    XpSTACK_MAP_(TPM_GRP_STACK,TPM_POP_POP,pop_show),
      XpH(TPM_GRP_SETA),
      XpH(TPM_GRP_SETB),
      XpH(TPM_GRP_BUTTONS),
      XpH(0),

    XpDIVREM(TPM_POP_POP),

    XpEND
  };
  LWXPanelDataDesc data_desc[] = {
    { TPM_BOL_ENABLE,  "Enable", "integer" },
    { TPM_POP_POP,  "PopTest", "integer" },
    { TPM_SLD_SLIDER,  "Range", "integer" },
    { TPM_CHO_CHOICE,  "Alter", "integer" },
    { 0 }
  };

  // Create the "mother" panel using only a hint array
  pan = (*g_lwxpf->create) ( LWXP_FORM, NULL );

  if ( pan ) {

    // Describe the data
    (*g_lwxpf->hint) ( pan, 0, hint );
    (*g_lwxpf->describe) ( pan, data_desc, NULL, NULL );

    // Set some control values
    FSETINT ( g_lwxpf, pan, TPM_BOL_ENABLE, 1 );
    FSETINT ( g_lwxpf, pan, TPM_POP_POP, 1 );
    FSETINT ( g_lwxpf, pan, TPM_SLD_SLIDER, 15 );
    FSETINT ( g_lwxpf, pan, TPM_CHO_CHOICE, 2 );


    //Open the dialog window
    rc = (*g_lwxpf->post) ( pan );

    if ( rc ) {
      // Ok
      // Get some control values
      int_val = FGETINT ( g_lwxpf, pan, TPM_BOL_ENABLE );
      int_val = FGETINT ( g_lwxpf, pan, TPM_POP_POP );
      int_val = FGETINT ( g_lwxpf, pan, TPM_SLD_SLIDER );
      int_val = FGETINT ( g_lwxpf, pan, TPM_CHO_CHOICE );
    }
    else {
      // Cancel
    }

    // Close self
    (*g_lwxpf->destroy) ( pan );

  } // End if panel

  return 0;
}


////////////////////////////////
////////////////////////////////
////      Activation        ////
////////////////////////////////
////////////////////////////////

  XCALL_(int)
XP_Activate (
  long                     version,
  GlobalFunc              *global,
  LWLayoutGeneric         *local,
  void                    *serverData)
{

  XCALL_INIT;
  if (version < 2) return AFUNC_BADVERSION;

  g_lwglobal = global;

  g_lwxpf = (*global) ( LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT);
  if ( !g_lwxpf ) return AFUNC_BADGLOBAL;

  g_msgf = (*g_lwglobal)( LWMESSAGEFUNCS_GLOBAL, GFUSE_TRANSIENT );
  if ( !g_msgf ) return AFUNC_BADGLOBAL;

    MAIN_panel ();

  return AFUNC_OK;
}

ServerRecord ServerDesc[] = {
  { "LayoutGeneric", "LWXPanelXTreme", XP_Activate },
  { NULL }
};


