/*
 * LWSDK Header File
 * Copyright 2003, NewTek, Inc.
 *
 * LWCOMRING.H -- LightWave Communications Ring
 *
 * This header contains declarations necessary to engage in a
 * communications ring among plug-ins
 */
#ifndef LWSDK_COMRING_H
#define LWSDK_COMRING_H

#define LWCOMRING_GLOBAL    "LW Communication Ring"

typedef void (*RingEvent)(void *clientData,void *portData,int eventCode,void *eventData);

typedef struct st_LWComRing {
    int     (*ringAttach)(char *topic,LWInstance pidata,RingEvent eventCallback);
    void    (*ringDetach)(char *topic,LWInstance pidata);
    void    (*ringMessage)(char *topic,int eventCode,void *eventData);
} LWComRing;

#endif

