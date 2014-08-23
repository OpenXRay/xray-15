/*
======================================================================
box.h

Function prototypes for the box plug-in

Ernie Wright  10 Jun 01
====================================================================== */

/* from ui.c */

void free_surflist( void );
int init_surflist( LWSurfaceFuncs *surff );
int get_user( LWXPanelFuncs *xpanf, double *size, double *center,
   char *surfname, char *vmapname );

/* from cmdline.c */

int parse_cmdline( DynaConvertFunc *convert, const char *cmdline,
   double *size, double *center, char *surfname, char *vmapname );
