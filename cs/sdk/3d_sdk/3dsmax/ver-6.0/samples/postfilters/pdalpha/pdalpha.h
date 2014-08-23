//-----------------------------------------------------------------------------
// ---------------------
// File ....: PdAlpha.h
// ---------------------
// Author...: Gus J Grubba
// Date ....: February 1996
// Descr....: PdAlpha Image Filter
//
// History .: Feb, 21 1995 - Started
//            
//-----------------------------------------------------------------------------
        
#ifndef _PDALPHACLASS_
#define _PDALPHACLASS_

#define DLLEXPORT __declspec(dllexport)

//-----------------------------------------------------------------------------
//-- Class Definition ---------------------------------------------------------
//

class ImageFilter_PdAlpha : public ImageFilter {
    
     public:
     
        //-- Constructors/Destructors
        
                       ImageFilter_PdAlpha( ) {}
                      ~ImageFilter_PdAlpha( ) {}
               
        //-- Filter Info  ---------------------------------

        const TCHAR   *Description         ( ) ;
        const TCHAR   *AuthorName          ( ) { return _T("Gus J Grubba");}
        const TCHAR   *CopyrightMessage    ( ) { return _T("Copyright 1996, Yost Group");}
        UINT           Version             ( ) { return (100);}

        //-- Filter Capabilities --------------------------
        
        DWORD          Capability          ( ) { return(IMGFLT_FILTER | IMGFLT_COMPOSITOR);}

        //-- Show DLL's About box -------------------------
        
        void           ShowAbout           ( HWND hWnd );  

        //-- Show Time ------------------------------------
        
        BOOL           Render              ( HWND hWnd );

};

#endif

//-- EOF: PdAlpha.h ----------------------------------------------------------
