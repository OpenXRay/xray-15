/*
======================================================================
laymon2.h

Definitions and typedefs for the laymon2 global.

Ernie Wright  1 May 00
====================================================================== */

#ifndef LAYMON2_H
#define LAYMON2_H

#include <lwmonitor.h>

#define LAYOUTMONITORFUNCS_GLOBAL "LayoutMonitor2"

typedef struct st_LayoutMonitorFuncs {
   LWMonitor *  ( *create )  ( const char *title, const char *caption );
   void         ( *destroy ) ( LWMonitor * );
} LayoutMonitorFuncs;

#endif
