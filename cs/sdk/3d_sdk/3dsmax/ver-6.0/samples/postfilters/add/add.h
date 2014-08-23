//-----------------------------------------------------------------------------
// ----------------
// File ....: Add.h
// ----------------
// Author...: Gus J Grubba
// Date ....: September 1995
// Descr....: Additive Compositor
//
// History .: Sep, 27 1995 - Started
//            
//-----------------------------------------------------------------------------
        
#ifndef _ADDCLASS_
#define _ADDCLASS_

#define DLLEXPORT __declspec(dllexport)

//-----------------------------------------------------------------------------
//-- Class Definition ---------------------------------------------------------
//

class ImageFilter_Add : public ImageFilter {

     public:
     
        //-- Constructors/Destructors
        
                       ImageFilter_Add( ) {};
                      ~ImageFilter_Add( ) {};
               
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

//-- EOF: Add.h ----------------------------------------------------------
