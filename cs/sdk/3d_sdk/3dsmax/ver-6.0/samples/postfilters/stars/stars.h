//-----------------------------------------------------------------------------
// ---------------------
// File ....: stars.h
// ---------------------
// Author...: Tom Hudson
// Date ....: April 1996
// Descr....: Starfield Image Filter
//
// History .: April 25 1996 - Started
//            
//-----------------------------------------------------------------------------
        
#ifndef _STARSCLASS_
#define _STARSCLASS_

#define DLLEXPORT __declspec(dllexport)

// Class ID
#define STARS_CLASS_ID Class_ID(0x97621334,0x652136f2)

//-----------------------------------------------------------------------------
//-- Configuration Block ------------------------------------------------------
//

#define BRITE_LINEAR 0
#define BRITE_LOG 1

#define DATA_RANDOM 0
#define DATA_CUSTOM 1

#define COMP_BACK 0
#define COMP_FORE 1

#define STARSVERSION 100

// Forward references
extern TCHAR *GetString(int id);

// Node enumeration callback
class NodeCallBack {
	public:
		virtual BOOL CallBack(INode *n)=0;	// Return TRUE to keep going
	};

class FindCameraNode : public NodeCallBack {
	public:
		TSTR name;
		INode *node;
		FindCameraNode(TSTR n) { name=n; node=NULL; }
		BOOL CallBack(INode *n);
		BOOL Found() { return (node == NULL) ? FALSE : TRUE; }
	};

class CameraFiller : public NodeCallBack {
	public:
		BOOL ok;
		HWND hWnd;
		int id;
		CameraFiller(HWND h, int i) { ok=FALSE; hWnd=h; id=i; }
		BOOL CallBack(INode *n);
		BOOL OK() { return ok; }
	};

//Random number generator class
class RandGen {
	public:
		ULONG next;
		RandGen() { next = 1; }
		void Seed(ULONG seed) { next = seed; }
		int Random() { next=next*1103515245 + 12345; return((unsigned int)(next/65536) % 32768); }
	};

typedef struct tagSTARSDATA {
	DWORD version;
	TCHAR camera[256];
	int dimmest, brightest;
	int briteType;
	float size;
	BOOL useBlur;
	int blurAmount, blurDim;
	int dataType;
	int seed, count;
	char filename[256];
	int compositing;
} STARSDATA;

// Misc data types

/* Star database */

#pragma pack(1)
typedef struct
{
Point3 p;
float fmag;
unsigned char mag;
} Star;
#pragma pack()

typedef struct
{
float x,y;
short vis;
} StarPos;

// Camera view info

typedef struct
{
Matrix3 tm;
float fov;
} CameraInfo;

// Process() line types

#define EVEN_LINES 1
#define ODD_LINES 2
#define ALL_LINES 3

//-----------------------------------------------------------------------------
//-- Class Definition ---------------------------------------------------------
//

class ImageFilter_Stars : public ImageFilter {
    
		STARSDATA       data;

		/* Pointer to star database */

		Star *stardata;
		StarPos *pos1, *pos2;

		int starcount;
		float dev_aspect;
		int field_type;
		float blurpct;
		RandGen rand;

	public:
     
        //-- Constructors/Destructors
        
                       ImageFilter_Stars( );
                      ~ImageFilter_Stars( ) { FreeData(); }
               
        //-- Filter Info  ---------------------------------

        const TCHAR   *Description         ( ) { return GetString(IDS_STARFIELD);}
        const TCHAR   *AuthorName          ( ) { return _T("Tom Hudson");}
        const TCHAR   *CopyrightMessage    ( ) { return _T("Copyright 1996, Hudson/O'Connell Design");}
        UINT           Version             ( ) { return (STARSVERSION);}

        //-- Filter Capabilities --------------------------
        
        DWORD          Capability          ( ) { return(IMGFLT_FILTER | IMGFLT_CONTROL); }

        //-- Show DLL's About & Control box ---------------
        
        void           ShowAbout           ( HWND hWnd );  
        BOOL           ShowControl         ( HWND hWnd );  

        //-- Showtime -------------------------------------
        
        BOOL           Render              ( HWND hWnd );

        //-- Filter Configuration -------------------------
        
        BOOL           LoadConfigure			( void *ptr );
        BOOL           SaveConfigure			( void *ptr );
        DWORD          EvaluateConfigure		( );

        //-- Local Methods --------------------------------
        
		BOOL			Control					(HWND ,UINT ,WPARAM ,LPARAM );
		void			Process	(int lines, RenderInfo *rinfo, CameraInfo *i1, CameraInfo *i2);
		BOOL			LoadCameras ( HWND hWnd );
		void			ComputeCamera(INode *node, TimeValue t, CameraInfo *ci);
		void			TransformStars (CameraInfo *c, StarPos *p);
		void			Plot(int x, int y, int v, BMM_Color_48 *c);
		void			FRect(float x, float y, float w, float h,int brite);
		void			AALine(float x1,float y1,float x2,float y2,float width,int brite);
		int				FCirc(float x, float y, float radius, float aspect, int brite, BMM_Color_48 *c);
		int				FindAngle(float deltax,float deltay,float *angle,float *angle90);
		void			FreeData();
		void			ErrorMessage(int id);
		void			EnableStarControls(HWND hWnd);
};

#endif

//-- EOF: stars.h ----------------------------------------------------------
