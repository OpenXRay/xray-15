//-----------------------------------------------------------------------------
// -------------------
// File ....: Cookie.h
// -------------------
// Author...: Gus J Grubba
// Date ....: February 1996
// Descr....: Cookie Cutter Image Filter
//
// History .: Feb, 18 1996 - Started
//            
//-----------------------------------------------------------------------------
        
#ifndef _COOKIECLASS_
#define _COOKIECLASS_

#define DLLEXPORT __declspec(dllexport)

//-----------------------------------------------------------------------------
//-- Class Definition ---------------------------------------------------------
//

class ImageFilter_Cookie : public ImageFilter {
    
     public:
     
        //-- Constructors/Destructors
        
                       ImageFilter_Cookie( ) {}
                      ~ImageFilter_Cookie( ) {}
               
        //-- Filter Info  ---------------------------------

        const TCHAR   *Description         ( ) ;
        const TCHAR   *AuthorName          ( ) { return _T("Gus J Grubba");}
        const TCHAR   *CopyrightMessage    ( ) { return _T("Copyright 1996, Yost Group");}
        UINT           Version             ( ) { return (100);}

        //-- Filter Capabilities --------------------------
        
        DWORD          Capability          ( ) { return(IMGFLT_FILTER | IMGFLT_MASK); }

        //-- Show DLL's About box -------------------------
        
        void           ShowAbout           ( HWND hWnd );

        //-- Showtime -------------------------------------
        
        BOOL           Render              ( HWND hWnd );

};

#endif

//-- EOF: Cookie.h ----------------------------------------------------------
