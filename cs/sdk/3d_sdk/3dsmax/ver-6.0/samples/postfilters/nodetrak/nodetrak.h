//-----------------------------------------------------------------------------
// ---------------------
// File ....: NodeTrak.h
// ---------------------
// Author...: Gus J Grubba
// Date ....: February 1996
// Descr....: NodeTrak Image Filter
//
// History .: Feb, 21 1995 - Started
//            
//-----------------------------------------------------------------------------
        
#ifndef _NODETRAKCLASS_
#define _NODETRAKCLASS_

#define DLLEXPORT __declspec(dllexport)

//-----------------------------------------------------------------------------
//-- Configuration Block ------------------------------------------------------
//

#define NODETRAKVERSION 100

typedef struct tagNODETRAKDATA {
     DWORD	version;
     TCHAR	nodename[MAX_PATH];
} NODETRAKDATA;

//-----------------------------------------------------------------------------
//-- Class Definition ---------------------------------------------------------
//

//-- Used with DoHitByNameDialog();

class ImageFilter_NodeTrak;

class NodeDlg : public HitByNameDlgCallback {

		ImageFilter_NodeTrak *flt;

	public:

					NodeDlg(ImageFilter_NodeTrak *f) {flt = f;} 

		TCHAR		*dialogTitle()				{ return _T("Node Tracker"); }
		TCHAR		*buttonText() 				{ return _T("Ok"); }
		BOOL		singleSelect()				{ return TRUE; }
		BOOL		useFilter()					{ return FALSE; }
		BOOL		useProc()					{ return TRUE; }
		void		proc(INodeTab &nodeTab);

};

class ImageFilter_NodeTrak : public ImageFilter {
    
		NODETRAKDATA			data;
		NodeDlg					*nodeDlg;

     public:
     
        //-- Constructors/Destructors
        
                       ImageFilter_NodeTrak( );
                      ~ImageFilter_NodeTrak( ) {}
               
        //-- Filter Info  ---------------------------------

        const TCHAR   *Description         ( ) ;
        const TCHAR   *AuthorName          ( ) { return _T("Gus J Grubba");}
        const TCHAR   *CopyrightMessage    ( ) { return _T("Copyright 1996, Yost Group");}
        UINT           Version             ( ) { return (NODETRAKVERSION);}

        //-- Filter Capabilities --------------------------
        
        DWORD          Capability          ( ) { return(IMGFLT_FILTER	| 
        																IMGFLT_CONTROL); }

        //-- Show DLL's About box -------------------------
        
        void           ShowAbout           ( HWND hWnd );  
        BOOL           ShowControl         ( HWND hWnd );  

        //-- Show Time ------------------------------------
        
        BOOL           Render              ( HWND hWnd );

        //-- Filter Configuration -------------------------
        
        BOOL           LoadConfigure			( void *ptr );
        BOOL           SaveConfigure			( void *ptr );
        DWORD          EvaluateConfigure		( );
        
        //-- Local Methods --------------------------------
        
		BOOL				Control					(HWND ,UINT ,WPARAM ,LPARAM );
		void				SetNodeName				(TCHAR *name) {_tcscpy(data.nodename,name);}

};

#endif

//-- EOF: NodeTrak.h ----------------------------------------------------------
