/*******************************************************************
 *
 *  EPS.CPP
 *
 *  DESCRIPTION:      .EPS file-format I/O DLL Header file
 *  Written by Autodesk for 3D Studio MAX
 *  
 *  (C) Copyright 1995 by Autodesk, Inc.
 *
 *  This program is copyrighted by Autodesk, Inc. and is licensed to you under
 *  the following conditions.  You may not distribute or publish the source
 *  code of this program in any form.  You may incorporate this code in object
 *  form in derivative works provided such derivative works are (i.) are de-
 *  signed and intended to work solely with Autodesk, Inc. products, and (ii.)
 *  contain Autodesk's copyright notice "(C) Copyright 1994 by Autodesk, Inc."
 *
 *  AUTODESK PROVIDES THIS PROGRAM "AS IS" AND WITH ALL FAULTS.  AUTODESK SPE-
 *  CIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
 *  A PARTICULAR USE.  AUTODESK, INC.  DOES NOT WARRANT THAT THE OPERATION OF
 *  THE PROGRAM WILL BE UNINTERRUPTED OR ERROR FREE.
 *
 *  This supports the following:
 *  1.  Outputs either an RGB image or gray image.  If the RGB image is 
 *      selected it will print properly whether or not the PostScript operator
 *      "colorimage" is supported on the interpreter.  If it is not it will
 *      convert the RGB image to gray scale using the NTSC conversion.
 *  2.  DSC compliance.  A fairly full set of DSC comments are output, not just
 *      the minimum necessary for EPSF compliance.
 *  3.  We put a "showpage" operator which is not required, but allowed.  This
 *      allows you to print this file as is (assuming you don't have a TIFF
 *      preview image).
 *
 *  Not supported:
 *  1.  Only preview image in TIFF format.
 *  2.  No CMYK output.  There is no attempt to convert RGB data to CMYK.
 *      Therefore, this EPSF is not suitable for CMYK separation.
 *
 *    AUTHOR:           Keith Trummel
 *
 *    HISTORY:          05/22/95 Originated
 *                      09/26/95 Updated for latest API
 *                      09/20/96 Fixed Globalization problem with ASCII files
 *                               and dithering true color
 *
 *******************************************************************/

/** include files **/

#define STRICT
#include <stdio.h>
#include "max.h"
#include "bmmlib.h"
#include "eps.h"
#include "epsres.h"

#define MAX_PATH_LENGTH 257
#define MAX_STRING_LENGTH  256

int triedToLoad = FALSE;
int resourcesLoaded = FALSE;
HINSTANCE hResource = NULL;

static INT_PTR CALLBACK ImageInfoDlg (HWND hWnd, UINT message, WPARAM wParam, 
                                   LPARAM lParam);


BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) 
{
    // If we have already tried to load the resource file and failed
    // just give up.
    if (triedToLoad && ! resourcesLoaded)
        return FALSE;

    // Load our resources.  We look for the file in the same directory 
    // where this DLL was found
    if (! resourcesLoaded) {
        char dirName[MAX_PATH_LENGTH];    // Where this DLL resides 
        char dllName[MAX_PATH_LENGTH];    // Full path name to resource DLL
        char *chPtr;

        GetModuleFileName (hinstDLL, dirName, MAX_PATH_LENGTH);

        // Strip off the file name
        chPtr = dirName + strlen (dirName);
        while (*(--chPtr) != '\\')
            ;
        *(chPtr+1) = 0;

        // Add in "epsres.dll"
        strcpy (dllName, dirName);
        strcat (dllName, "epsres.dll");

        // Load resource DLL
        // Turn off error reporting
        int errorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX);
        hResource = LoadLibraryEx (dllName, NULL, 0);
        SetErrorMode(errorMode);

        // Be sure to check to see if we succeeded loading resource DLL
        if (hResource) {
            resourcesLoaded = TRUE;
            InitCustomControls (hResource);
        } else {
            triedToLoad = TRUE;
            MessageBox (NULL, 
           "EPS Plugin failed to load due to missing resource file EPSRES.DLL",
            "EPS", MB_ICONINFORMATION);
            return FALSE;
        }
    }

    switch(fdwReason) {
    case DLL_PROCESS_ATTACH:
        //MessageBox(NULL,L"EPS.DLL: DllMain",L"EPS",MB_OK);
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        if (hResource) 
            FreeLibrary (hResource);
        break;
    }
    return(TRUE);
}

//------------------------------------------------------

class EPSClassDesc:public ClassDesc 
{
    public:
        int             IsPublic() { return 1; }
        void *          Create(BOOL loading=FALSE) { return new BitmapIO_EPS; }
        const TCHAR *   ClassName();
        SClass_ID       SuperClassID() { return BMM_IO_CLASS_ID; }
        Class_ID        ClassID() { return Class_ID(123423, 323491); }
        const TCHAR*    Category();
};

static EPSClassDesc EPSDesc;

//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() 
{ 
    static int loaded = 0;
    static TCHAR stringBuf[MAX_STRING_LENGTH];

    if (! loaded) {
        LoadString (hResource, IDS_LIB_DESCRIPTION, stringBuf, 
                    MAX_STRING_LENGTH);
        loaded = 1;
    }
    return stringBuf;

}

__declspec( dllexport ) int
LibNumberClasses() 
{ 
    return 1;
}

__declspec( dllexport ) ClassDesc *
LibClassDesc(int i) 
{
    switch(i) {
    case 0: return &EPSDesc; break;
    default: return 0; break;
    }
}

__declspec( dllexport ) ULONG LibVersion ( )  
{ 
    return ( VERSION_3DSMAX ); 
}

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}

const TCHAR *
EPSClassDesc::ClassName ()
{
    static int loaded = 0;
    static TCHAR stringBuf[MAX_STRING_LENGTH];

    if (! loaded) {
	LoadString (hResource, IDS_CLASS_NAME, stringBuf,
		    MAX_STRING_LENGTH);
	loaded = 1;
    }
    return stringBuf;
}

const TCHAR *
EPSClassDesc::Category ()
{
    static int loaded = 0;
    static TCHAR stringBuf[MAX_STRING_LENGTH];

    if (! loaded) {
	LoadString (hResource, IDS_CATEGORY, stringBuf,
		    MAX_STRING_LENGTH);
	loaded = 1;
    }
    return stringBuf;
}

//
// BitmapIO_EPS public class methods
//

BitmapIO_EPS::BitmapIO_EPS() 
{
    ostream = NULL;
    istream = NULL;
    iFilename = NULL;
    oFilename = NULL;

    hWnd = NULL;

    userSettings.units = INCHES;
    userSettings.binary = 0;
    userSettings.preview = 0;
    userSettings.orientation = PORTRAIT;
    userSettings.colorType = RGBIMAGE;
    userSettings.paperHeight = 11.0F;
    userSettings.paperWidth = 8.5F;
    userSettings.xResolution = 72.0F;
    userSettings.yResolution = 72.0F;

    tiffDownSample = 1.0F;

    // Set the defaults to last values used which are saved in 
    // a configuration file
    Configure();
}

BitmapIO_EPS::~BitmapIO_EPS() 
{
    if (ostream) 
        fclose (ostream);
    if (istream)
	fclose (istream);
    free (iFilename);
    free (oFilename);
}

int
BitmapIO_EPS::ExtCount() {
        return 2;
        }

const TCHAR *
BitmapIO_EPS::Ext(int n) {   // Extensions supported for import/export modules
    switch(n) {
    case 0:
        return _T("eps");
    case 1:
        return _T("ps");
    }
    return _T("");
}

const TCHAR *
BitmapIO_EPS::LongDesc()  // Long description 
{
    static int loaded = 0;
    static TCHAR stringBuf[MAX_STRING_LENGTH];

    if (! loaded) {
        LoadString (hResource, IDS_LONG_DESC, stringBuf,
                    MAX_STRING_LENGTH);
        loaded = 1;
    }
    return stringBuf;
}
        
const TCHAR *
BitmapIO_EPS::ShortDesc()     // Short ASCII description 
{
    static int loaded = 0;
    static TCHAR stringBuf[MAX_STRING_LENGTH];

    if (! loaded) {
        LoadString (hResource, IDS_SHORT_DESC, stringBuf,
                    MAX_STRING_LENGTH);
        loaded = 1;
    }
    return stringBuf;
}

const TCHAR *
BitmapIO_EPS::AuthorName()                     // ASCII Author name
{
    static int loaded = 0;
    static TCHAR stringBuf[MAX_STRING_LENGTH];

    if (! loaded) {
        LoadString (hResource, IDS_AUTHOR_NAME, stringBuf,
                    MAX_STRING_LENGTH);
        loaded = 1;
    }
    return stringBuf;
}

const TCHAR *
BitmapIO_EPS::CopyrightMessage()       // ASCII Copyright message
{
    static int loaded = 0;
    static TCHAR stringBuf[MAX_STRING_LENGTH];

    if (! loaded) {
        LoadString (hResource, IDS_COPYRIGHT, stringBuf,
                    MAX_STRING_LENGTH);
        loaded = 1;
    }
    return stringBuf;
}

const TCHAR *
BitmapIO_EPS::OtherMessage1() 
{
    static int loaded = 0;
    static TCHAR stringBuf[MAX_STRING_LENGTH];

    if (! loaded) {
        LoadString (hResource, IDS_OTHER_MESSAGE1, stringBuf,
                    MAX_STRING_LENGTH);
        loaded = 1;
    }
    return stringBuf;
}

const TCHAR *
BitmapIO_EPS::OtherMessage2() 
{
    static int loaded = 0;
    static TCHAR stringBuf[MAX_STRING_LENGTH];

    if (! loaded) {
        LoadString (hResource, IDS_OTHER_MESSAGE2, stringBuf,
                    MAX_STRING_LENGTH);
        loaded = 1;
    }
    return stringBuf;
}

unsigned int
BitmapIO_EPS::Version()      // Version number * 100 (i.e. v3.01 = 301)
{
    return 101;
}

int
BitmapIO_EPS::Capability() 
{
//    return BMMIO_WRITER | BMMIO_RANDOM_WRITES | BMMIO_EXTENSION;
    return BMMIO_WRITER | BMMIO_EXTENSION | BMMIO_INFODLG | BMMIO_CONTROLWRITE;
}

// Configuration Functions
BOOL 
BitmapIO_EPS::LoadConfigure (void *ptr)
{
    UserSettable *buf = (UserSettable *) ptr;
    memcpy (&userSettings, ptr, sizeof(UserSettable));
    return TRUE;
}

BOOL
BitmapIO_EPS::SaveConfigure (void *ptr)
{
    if (ptr) {
        memcpy (ptr, &userSettings, sizeof(UserSettable));
        return TRUE;
    } else
        return FALSE;
}

DWORD
BitmapIO_EPS::EvaluateConfigure () 
{
    return sizeof (UserSettable);
}

INT_PTR CALLBACK 
AboutDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) 
{
    switch (message) {
    case WM_INITDIALOG:
        return 1;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:              
            EndDialog(hWnd,1);
            break;
        }
        return 1;
    }
    return 0;
}

void
BitmapIO_EPS::ShowAbout(HWND hWnd) 
{                    // Optional
    DialogBoxParam (hResource,
                    MAKEINTRESOURCE (IDD_ABOUT_DIALOG),
                    hWnd,
                    AboutDlgProc,
                    0);
}

// Execution Functions
BMMRES 
BitmapIO_EPS::GetImageInfo(BitmapInfo *info) 
{
   // MessageBox(NULL,_T(".EPS ImageInfo called"),_T("About EPS.BMI"),MB_OK);
    return BMMRES_INVALIDFORMAT;   // We don't handle reading
}

BMMRES
BitmapIO_EPS::GetImageInfoDlg (HWND hWnd, BitmapInfo *info, 
                               const TCHAR *filename)
{
    free (iFilename);
    iFilename = NULL;

    if (filename) {
        iFilename = (TCHAR *) malloc ((_tcslen(filename)+1)*sizeof(TCHAR));
        if (iFilename)
            _tcscpy (iFilename, filename);
        ReadHeader (iFilename);
    } else if (info->Name ()) {
        iFilename = (TCHAR *) 
	    malloc ((_tcslen(info->Name ())+1)*sizeof(TCHAR));
        if (iFilename)
            _tcscpy (iFilename, info->Name ());
        ReadHeader (iFilename);
    } else {
        MessageBox (hWnd, _T("EPS GetImageInfoDlg called with no file name"),
                    _T("EPS Plugin"), MB_ICONINFORMATION);
        return BMMRES_FILENOTFOUND;
    }

    DialogBoxParam (hResource,
                    MAKEINTRESOURCE (IDD_INFO_DIALOG),
                    hWnd,
                    ImageInfoDlg,
                    (LPARAM) this);

    return BMMRES_SUCCESS;
}

INT_PTR CALLBACK
OutputCtrlDlgProc (HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) 
{
    BitmapIO_EPS *epsPtr;

    if (message == WM_INITDIALOG) {
        epsPtr = (BitmapIO_EPS *) lParam;
        SetWindowLongPtr (hWnd, GWLP_USERDATA, (LONG_PTR) epsPtr);
    } else
        epsPtr = (BitmapIO_EPS *) GetWindowLongPtr (hWnd, GWLP_USERDATA);

    if (epsPtr)
        return epsPtr->OutputControl (hWnd, message, wParam, lParam);
    else
        return FALSE;
}
        
BOOL
BitmapIO_EPS::OutputControl (HWND hWnd, UINT message, WPARAM wParam,
                             LPARAM lParam) 
{
    float factor;
    switch (message) {
    case WM_INITDIALOG:
        factor = (userSettings.units == INCHES) ? 1.0F : MM_PER_INCHES;

        widthSpin = GetISpinner (GetDlgItem (hWnd, IDC_WIDTH_SPINNER));
        widthSpin->SetLimits (0, 1000, FALSE);
        widthSpin->SetScale (0.1F);
        widthSpin->LinkToEdit (GetDlgItem (hWnd, IDC_WIDTH_EDIT),
                               EDITTYPE_FLOAT);
        widthSpin->SetValue (userSettings.paperWidth * factor, FALSE);

        heightSpin = GetISpinner (GetDlgItem (hWnd, IDC_HEIGHT_SPINNER));
        heightSpin->SetLimits (0, 1000, FALSE);
        heightSpin->SetScale (0.1F);
        heightSpin->LinkToEdit (GetDlgItem (hWnd, IDC_HEIGHT_EDIT),
                                EDITTYPE_FLOAT);
        heightSpin->SetValue (userSettings.paperHeight * factor, FALSE);

        widthResSpin = GetISpinner (GetDlgItem (hWnd, 
                                                IDC_RES_WIDTH_SPINNER));
        widthResSpin->SetLimits (0, 1000, FALSE);
        widthResSpin->SetScale (0.1F);
        widthResSpin->LinkToEdit (GetDlgItem (hWnd, IDC_RES_WIDTH_EDIT), 
                                  EDITTYPE_FLOAT);
        widthResSpin->SetValue (userSettings.xResolution / factor, FALSE);

        heightResSpin = GetISpinner (GetDlgItem (hWnd, 
                                                 IDC_RES_HEIGHT_SPINNER));
        heightResSpin->SetLimits (0, 1000, FALSE);
        heightResSpin->SetScale (0.1F);
        heightResSpin->LinkToEdit (GetDlgItem (hWnd, IDC_RES_HEIGHT_EDIT), 
                                   EDITTYPE_FLOAT);
        heightResSpin->SetValue (userSettings.yResolution / factor, FALSE);
        
//      CenterWindow(hWnd,GetParent(hWnd));
        CheckRadioButton (hWnd, IDC_INCHES, IDC_MILLIMETERS, 
                          (userSettings.units == INCHES) ? 
                          IDC_INCHES : IDC_MILLIMETERS);  
        CheckRadioButton (hWnd, IDC_PORTRAIT, IDC_LANDSCAPE, 
                          (userSettings.orientation == PORTRAIT) ? 
                          IDC_PORTRAIT : IDC_LANDSCAPE);
        CheckRadioButton (hWnd, IDC_BINARY, IDC_ASCII,
                          (userSettings.binary) ? 
                          IDC_BINARY : IDC_ASCII);
        CheckRadioButton (hWnd, IDC_COLOR, IDC_GRAY,
                          (userSettings.colorType == RGBIMAGE) ? 
                          IDC_COLOR : IDC_GRAY);
        CheckDlgButton (hWnd, IDC_INCLUDE_PREVIEW, 
                        userSettings.preview);
        return 1;
    case WM_DESTROY:
        ReleaseISpinner (widthSpin);                      
        ReleaseISpinner (heightSpin);                     
        ReleaseISpinner (widthResSpin);                   
        ReleaseISpinner (heightResSpin);                          
        return 1;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_INCHES:
            // Don't do anything unless this is a change
            if (userSettings.units != INCHES)  {
                userSettings.units = INCHES;
                userSettings.paperHeight = heightSpin->GetFVal () / 
                    MM_PER_INCHES;
                userSettings.paperWidth = widthSpin->GetFVal () /
                    MM_PER_INCHES;
                userSettings.xResolution = widthResSpin->GetFVal () *
                    MM_PER_INCHES;
                userSettings.yResolution = heightResSpin->GetFVal () *
                    MM_PER_INCHES;
                widthSpin->SetValue (userSettings.paperWidth, FALSE);
                heightSpin->SetValue (userSettings.paperHeight, FALSE);
                widthResSpin->SetValue (userSettings.xResolution, FALSE);
                heightResSpin->SetValue (userSettings.yResolution, FALSE);
            }
            break;
        case IDC_MILLIMETERS:
            if (userSettings.units != MM)  {
                userSettings.units = MM;
                userSettings.paperHeight = heightSpin->GetFVal ();
                userSettings.paperWidth = widthSpin->GetFVal ();
                userSettings.xResolution = widthResSpin->GetFVal ();
                userSettings.yResolution = heightResSpin->GetFVal ();
                widthSpin->SetValue (userSettings.paperWidth *
                                     MM_PER_INCHES, FALSE);
                heightSpin->SetValue (userSettings.paperHeight *
                                      MM_PER_INCHES, FALSE);
                widthResSpin->SetValue (userSettings.xResolution /
                                        MM_PER_INCHES, FALSE);
                heightResSpin->SetValue (userSettings.yResolution / 
                                         MM_PER_INCHES, FALSE);
            }
            break;
        case IDOK:              
            userSettings.units = 
                IsDlgButtonChecked (hWnd, IDC_INCHES) ? INCHES : MM;
            factor = (userSettings.units == INCHES) ? 1.0F : 
            MM_PER_INCHES;
            userSettings.orientation = 
                IsDlgButtonChecked (hWnd, IDC_PORTRAIT) ? PORTRAIT : LANDSCAPE;
            userSettings.binary = 
                IsDlgButtonChecked (hWnd, IDC_BINARY);
            userSettings.colorType = 
                IsDlgButtonChecked (hWnd, IDC_COLOR) ? RGBIMAGE : 
            GRAYIMAGE;
            userSettings.preview = 
                IsDlgButtonChecked (hWnd, IDC_INCLUDE_PREVIEW);
            userSettings.paperHeight = heightSpin->GetFVal () / factor;
            userSettings.paperWidth = widthSpin->GetFVal () / factor;
            userSettings.xResolution = widthResSpin->GetFVal () * factor;
            userSettings.yResolution = heightResSpin->GetFVal () * factor;
	    WriteConfigFile ();
            EndDialog(hWnd,1);
            break;
        case IDCANCEL:
            EndDialog(hWnd,0);
            break;
        }
        return 1;
    }
    return 0;
}

BOOL
BitmapIO_EPS::ShowControl(HWND hWnd, DWORD flag ) 
{
    return DialogBoxParam (hResource,
                           MAKEINTRESOURCE (IDD_OUTPUT_DIALOG),
                           hWnd,
                           OutputCtrlDlgProc,
                           (LPARAM) this);
}


static INT_PTR CALLBACK
ImageInfoDlg (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) 
{
    BitmapIO_EPS *epsPtr;

    if (message == WM_INITDIALOG) {
        epsPtr = (BitmapIO_EPS *) lParam;
        SetWindowLongPtr (hWnd, GWLP_USERDATA, (LONG_PTR) epsPtr);
    } else
        epsPtr = (BitmapIO_EPS *) GetWindowLongPtr (hWnd, GWLP_USERDATA);

    if (epsPtr)
        return epsPtr->ImageInfoProc (hWnd, message, wParam, lParam);
    else
        return FALSE;
}

BOOL 
BitmapIO_EPS::ImageInfoProc (HWND hWnd, UINT message, WPARAM wParam, 
                            LPARAM lParam) 
{
    // char buf[10];
    TCHAR yesOrNo[20];

    switch (message) {
    case WM_INITDIALOG: {
        CenterWindow(hWnd,GetParent(hWnd));
        SetCursor(LoadCursor(NULL,IDC_ARROW));
        
        if (iFilename)
            SetDlgItemText (hWnd, IDC_FILENAME, iFilename);

        if (title)
	        SetDlgItemText (hWnd, IDC_EPS_TITLE, title);

        SetDlgItemText (hWnd, IDC_CREATOR, creator);
        SetDlgItemText (hWnd, IDC_CREATION_DATE, creationDate);
        
		// Removed author info/copyright. CCJ - Sept. 15, 1997
        //SetDlgItemText (hWnd, IDC_DRIVENAME, LongDesc());
        //SetDlgItemText(hWnd, IDC_AUTHOR, AuthorName());
        //SetDlgItemText(hWnd, IDC_COPYRIGHT, CopyrightMessage());
        //sprintf(buf,"%4.02f",(float) Version() / 100.0f);
        //SetDlgItemText(hWnd, IDC_DRIVEVERSION, buf);
        
        if (includesPreview)
            LoadString (hResource, IDS_YES, yesOrNo, sizeof(yesOrNo));
        else
            LoadString (hResource, IDS_NO, yesOrNo, sizeof(yesOrNo));
        SetDlgItemText(hWnd, IDC_PREVIEW, yesOrNo);
            
        return 1;
    }
        
    case WM_COMMAND:
        
        switch (LOWORD(wParam)) {
            
            //-- Changes Accepted ---------------------
            
        case IDCANCEL:
        case IDOK:
            EndDialog(hWnd,1);
            break;
            
        }
        return 1;
        
    }
    return 0;
    
}


BitmapStorage *
BitmapIO_EPS::Load (BitmapInfo *fbi, Bitmap *map, BMMRES *status)
{
    *status = ProcessImageIOError(fbi,BMMRES_INTERNALERROR);
	 return (NULL);
}


BMMRES
BitmapIO_EPS::OpenOutput(BitmapInfo *fbi, Bitmap *map) 
{
	if (openMode != BMM_NOT_OPEN)
		return (ProcessImageIOError(fbi,BMMRES_INTERNALERROR));
		
	if (!map)
		return (ProcessImageIOError(fbi,BMMRES_INTERNALERROR));
		
    // Save Image Info Data
    bi.CopyImageInfo (fbi);
    
    // Above doesn't copy window so do that here
    hWnd = fbi->GetUpdateWindow();

    this->map = map;
    openMode = BMM_OPEN_W;

    return BMMRES_SUCCESS;
}

BMMRES
BitmapIO_EPS::Write(int frame) 
{
	//-- If we haven't gone through an OpenOutput(), leave

	if (openMode != BMM_OPEN_W)
		return (ProcessImageIOError(&bi,BMMRES_INTERNALERROR));

	//-- Resolve Filename --------------------------------

	TCHAR filename[MAX_PATH];

	if (frame == BMM_SINGLEFRAME) {
		_tcscpy(filename,bi.Name());
	} else {
		if (!BMMCreateNumberedFilename(bi.Name(),frame,filename))
			return (ProcessImageIOError(&bi,BMMRES_NUMBEREDFILENAMEERROR));
	}
     
	return Save(filename,map);
}

int
BitmapIO_EPS::Close(int option) 
{
    if (ostream) {
        fclose (ostream);
        ostream = NULL;
	hWnd = NULL;
    }

    return 1;
}


/* This function handles writing out EPS files.  It is written to be
 * as "PostScript friendly as possible", i.e. it:
 *   1) Uses a fairly full set of DSC comments
 *   2) Uses its own dictionary instead of assuming there is room in
 *      the current one
 *   3) If colorimage is not supported on the PostScript interpreter
 *      and an RGB image was selected the image will be rendered as grayscale
 *      instead using the NTSC conversion.
 */
BMMRES
BitmapIO_EPS::Save(const TCHAR *name, Bitmap *bitmap) 
{
    int xOffset = 0;     // Offset for the lower left corner
    int yOffset = 0;     // of the image */
    int status;

	if (! bitmap) 
		return (ProcessImageIOError(&bi,BMMRES_INTERNALERROR));

    /* Compute the position on the page to output the image */
    Position (bitmap, &xOffset, &yOffset);
   
    if ((ostream = _tfopen(name, _T("wb"))) == NULL)
		return (ProcessImageIOError(&bi));

    /* If a preview is desired then write the TIFF section */
    if (userSettings.preview)
    {
        status = WritePreview (userSettings.colorType, 
			       userSettings.orientation,
			       userSettings.xResolution,
			       userSettings.yResolution,
			       bitmap);
        if (status != BMMRES_SUCCESS)
        {
            fclose (ostream);
            return status;
        }
    }

    /* First the PostScript header */
    status = WriteHeader (bitmap, xOffset, yOffset);
    if (status != BMMRES_SUCCESS)
    {
        fclose (ostream);
        return status;
    }

    /* Now the image data itself */
    status = WriteImagePosition (bitmap, xOffset, yOffset);
    if (status != BMMRES_SUCCESS)
    {
        fclose (ostream);
        return status;
    }
    if (userSettings.binary && userSettings.colorType == RGBIMAGE)
        status = WriteBinaryRGBImage (bitmap);
    else if (userSettings.binary && userSettings.colorType == GRAYIMAGE)
        status = WriteBinaryGrayImage (bitmap);
    else if (userSettings.colorType == RGBIMAGE)
        status = WriteAsciiRGBImage (bitmap);
    else 
        status = WriteAsciiGrayImage (bitmap);
    if (status != BMMRES_SUCCESS)
    {
        fclose (ostream);
        return status;
    }

    /* Now the trailer */
    status = WriteTrailer ();

    /* If a preview was requested we need to now figure out the length of the
     * PostScript portion and write it to the header
     */
    if (userSettings.preview)
        status = WritePSLength (userSettings.colorType, map);
    
    fclose (ostream);

    return (status);
}

