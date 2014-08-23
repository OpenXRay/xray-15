/*
======================================================================
box.h

Data definitions and function prototypes for the box tool plug-in.

Ernie Wright  30 Jun 01
====================================================================== */

#ifndef BOX_H
#define BOX_H

#include <lwserver.h>
#include <lwmodtool.h>
#include <lwxpanel.h>


typedef struct st_BoxData {
   LWDVector   size, center;
   char        surfname[ 128 ];
   char        vmapname[ 128 ];
   LWFVector   hpos[ 2 ];
   int         update, active, dirty;
} BoxData;


/* box.c */

void makebox( MeshEditOp *edit, BoxData *box );

/* ui.c */

BoxData *new_box( void );
int get_xpanf( GlobalFunc *global );
LWXPanelID Panel( BoxData *box );

/* tool.c */

void calc_handles( BoxData *box );

#endif