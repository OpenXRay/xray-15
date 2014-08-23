//-----------------------------------------------------------------------------
// ------------------
// File ....: CrosFade.h
// ------------------
// Author...: Gus J Grubba
// Date ....: February 1996
// Descr....: CrosFade Transition
//
// History .: Feb, 08 1996 - Started
//            
//-----------------------------------------------------------------------------
        
#ifndef _CROSFADECLASS_
#define _CROSFADECLASS_

#define DLLEXPORT __declspec(dllexport)

//-----------------------------------------------------------------------------
//-- Class Definition ---------------------------------------------------------
//

class ImageFilter_CrosFade : public ImageFilter {
    
     public:
     
        //-- Constructors/Destructors
        
                       ImageFilter_CrosFade( ) {}
                      ~ImageFilter_CrosFade( ) {}
               
        //-- Filter Info  ---------------------------------

        const TCHAR   *Description         ( ) ;
        const TCHAR   *AuthorName          ( ) { return _T("Gus J Grubba");}
        const TCHAR   *CopyrightMessage    ( ) { return _T("Copyright 1995, Yost Group");}
        UINT           Version             ( ) { return (100);}

        //-- Filter Capabilities --------------------------
        
        DWORD          Capability          ( ) { return(IMGFLT_COMPOSITOR | IMGFLT_MASK); }

        //-- Show DLL's About box -------------------------
        
        void           ShowAbout           ( HWND hWnd );  
        BOOL           ShowControl         ( HWND hWnd ) { return (FALSE); }  

        //-- Show Time ------------------------------------
        
        BOOL           Render              ( HWND hWnd );

};

#endif

//-- EOF: CrosFade.h ----------------------------------------------------------
