/*
 * LWSDK Header File
 * Copyright 2003, NewTek, Inc.
 *
 * LWLSCRIPT.H -- LScript Interface Definitions
 *
 * This header contains declarations for the public services
 * provided by the LightWave LScript subsystem.
 */

#ifndef LSCRIPT_INCL
#define LSCRIPT_INCL

#define OAMAJOR     1
#define OAMINOR     4

#define LSINTEGER   1
#define LSNUMBER    2
#define LSSTRING    3
#define LSVECTOR    4
#define LSFILE      5
#define LSARRAY     6

#define LSNIL       99

#define MAXINSTANCE  10

typedef struct _ls_var
{
    int     type;
    const   void *extra;

    union {
        double      number;
        const char *string;
        double      vector[3];
        FILE       *file;
    } data;
} LS_VAR;

typedef struct _lsfunc
{
    void    *data;

    char    * (*strdup)(void *,const char *);
    void    * (*malloc)(void *,int);
    void      (*free)(void *,void *);
    void      (*info)(void *,const char *);
    void      (*error)(void *,const char *);

    /* the following pointers will be NULL if the calling script already */
    /* has a monitor posted, or we are in a situation where they would   */
    /* be inappropriate (i.e., LS/DM)                                    */

    void      (*moninit)(void *,const char *,int);
    int       (*monstep)(void *,int);
    void      (*monend)(void *);
} LSFunc;

typedef struct _lsdll
{
    const char *ClassName;
    const char *InstanceName;

    LS_VAR  *instance[MAXINSTANCE];
    LSFunc  *func;
} LSDLL;

/* FileData is additional information about files that are being returned */
/* from a DLL function.  this data is attached to the 'extra' bytes       */
/* available in LS_VAR, and MUST be populated for returned files         */

typedef struct _filedata
{
    char  name[256];
    char  openmode[10];
} FileData;

typedef LS_VAR * (*DLLFunc)(int *,LS_VAR *,LSFunc *);
typedef LS_VAR * (*UserObjMethod)(int *,LS_VAR *,LSDLL *);
typedef LS_VAR * (*UserObjAlias)(char *,int *,LS_VAR *,LSDLL *);

#endif
