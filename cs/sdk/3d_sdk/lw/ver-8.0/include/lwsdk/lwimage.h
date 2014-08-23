/*
 * LWSDK Header File
 * Copyright 1999, NewTek, Inc.
 *
 * LWIMAGE.H -- LightWave Images
 */
#ifndef LWSDK_IMAGE_H
#define LWSDK_IMAGE_H

#include <lwsdk/lwtypes.h>
#include <lwsdk/lwimageio.h>
#include <lwsdk/lwio.h>


#define LWIMAGELIST_GLOBAL	"LW Image List 4"
#define LWIMAGEUTIL_GLOBAL	"Image Utility"

// change-event codes

enum {
        LWCEV_CHANGE = 0,
        LWCEV_REPLACE,
        LWCEV_DESTROY
};

// change-event callback prototype

typedef void (*LWImageEventFunc)(int eventCode,LWImageID);

// event regitration codes (second argument to changeEvent())

enum {
        LWERC_REGISTER = 0,
        LWERC_UNREGISTER
};

typedef struct st_LWImageList {
	LWImageID	(*first)    (void);
	LWImageID	(*next)     (LWImageID);
	LWImageID	(*load)     (const char *);
	const char *	(*name)     (LWImageID);
	const char *	(*filename) (LWImageID, LWFrame);
	int		(*isColor)  (LWImageID);
	void		(*needAA)   (LWImageID);
	void		(*size)     (LWImageID, int *w, int *h);
	LWBufferValue	(*luma)     (LWImageID, int x, int y);
	void		(*RGB)      (LWImageID, int x, int y, LWBufferValue[3]);
	double		(*lumaSpot) (LWImageID, double x, double y,
				     double spotSize, int blend);
	void		(*RGBSpot)  (LWImageID, double x, double y,
				     double spotSize, int blend, double[3]);
	void		(*clear)    (LWImageID);
	LWImageID       (*sceneLoad) (const LWLoadState *);
	void            (*sceneSave) (const LWSaveState *, LWImageID);

	int             (*hasAlpha) (LWImageID);
	LWBufferValue   (*alpha) (LWImageID, int x, int y);
	double          (*alphaSpot) (LWImageID, double x, double y,
				     double spotSize, int blend);
	LWPixmapID		(*evaluate) (LWImageID, LWTime t);

	void    		(*changeEvent) (LWImageEventFunc,int);
	int     		(*replace) (LWImageID,const char *);
} LWImageList;


typedef struct st_LWImageUtil {
	LWPixmapID	(*create) (int w, int h, LWImageType type);
	void		(*destroy) (LWPixmapID img);
	int		(*save) (LWPixmapID img, int saver,const char *name);
	void		(*setPixel) (LWPixmapID img, int x, int y, void *pix);
	void		(*getPixel) (LWPixmapID img, int x, int y, void *pix);
	void		(*getInfo) (LWPixmapID img, int *w, int *h, int *type);
	LWPixmapID	(*resample) (LWPixmapID img, int w, int h, int mode);
	int		(*saverCount) ();
	const char *	(*saverName) (int saver);
} LWImageUtil;

/* resample modes */
enum {
        LWISM_SUBSAMPLING = 0,
        LWISM_MEDIAN,		/* for shrinking */
        LWISM_SUPERSAMPLING,
        LWISM_BILINEAR,		/* for expanding */
        LWISM_BSPLINE,		/* for expanding */
        LWISM_BICUBIC		/* for expanding */
};

#endif

