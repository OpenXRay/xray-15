/*
======================================================================
pipeline.h

Plug-in names and trace function prototype.

Ernie Wright  15 Jul 01
====================================================================== */

#include <lwserver.h>
#include <stdio.h>
#include <stdlib.h>

#define ANLD_PNAME  "Pipeline_AnimLoad"
#define ANSV_PNAME  "Pipeline_AnimSave"
#define CHAN_PNAME  "Pipeline_Channel"
#define COBJ_PNAME  "Pipeline_CustomObject"
#define DISP_PNAME1 "Pipeline_Displace_BeforeBones"
#define DISP_PNAME2 "Pipeline_Displace_Object"
#define DISP_PNAME3 "Pipeline_Displace_World"
#define ENVI_PNAME  "Pipeline_Environment"
#define FBUF_PNAME  "Pipeline_FrameBuffer"
#define IFLT_PNAME  "Pipeline_ImageFilter"
#define MOTN_PNAME1 "Pipeline_Motion_BeforeIK"
#define MOTN_PNAME2 "Pipeline_Motion_AfterIK"
#define MAST_PNAME  "Pipeline_Master"
#define OREP_PNAME  "Pipeline_ObjReplace"
#define PFLT_PNAME  "Pipeline_PixelFilter"
#define TXTR_PNAME  "Pipeline_Texture"
#define SHAD_PNAME  "Pipeline_Shader"
#define VOLU_PNAME  "Pipeline_Volume"

void trace( const char *func, const char *plug, const char *fmt, ... );
