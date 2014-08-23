/**********************************************************************
 *
 * FILE:        cin.cpp
 * AUTHOR:      greg finch
 *
 * DESCRIPTION: SMPTE Digital Picture Exchange Format,
 *              SMPTE CIN, Kodak Cineon
 *
 * CREATED:     20 june, 1997
 *
 *
 * 
 * 
 * Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#include "cin.h"

#define BASIC_INFO_DLG //this is used to turn on/off the complete info dialog

TCHAR*
GetResIDCaption(int id)
{
    static TCHAR captionBuffer[256];

    if (hInst)  return LoadString(hInst, id, captionBuffer, sizeof(captionBuffer)) ? captionBuffer : NULL;
    else        return NULL;
}

//-----------------------< CIN_ Dialog Procs >----------------------
INT_PTR CALLBACK
CIN_AboutDialogProc(HWND hWnd, UINT msg, WPARAM wParam,LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG:
        CenterWindow(hWnd, GetParent(hWnd));
        SetCursor(LoadCursor(NULL,IDC_ARROW));
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            EndDialog(hWnd,TRUE);
            break;
        case IDCANCEL:
            EndDialog(hWnd, FALSE);
            break;
        }
        return TRUE;
    }
    return FALSE;
}

static INT_PTR CALLBACK
CIN_ImageInfoDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static BitmapIO_CIN* dpxInfo;
    switch (msg) {
    case WM_INITDIALOG: {
        dpxInfo = (BitmapIO_CIN*) lParam;
        CenterWindow(hWnd, GetParent(hWnd));
        SetCursor(LoadCursor(NULL,IDC_ARROW));

     // dialog text fields
        SetDlgItemText(hWnd, IDC_INFO_DLG_FILE_NAME,            dpxInfo->mCineonImage.GetImageFileName());
        SetDlgItemText(hWnd, IDC_INFO_DLG_FILE_VERSION,         dpxInfo->mCineonImage.GetVersionNumber());
        SetDlgItemText(hWnd, IDC_INFO_DLG_FILE_CREATION_DATE,   dpxInfo->mCineonImage.GetImageCreationDate());
        SetDlgItemText(hWnd, IDC_INFO_DLG_FILE_CREATION_TIME,   dpxInfo->mCineonImage.GetImageCreationTime());
#ifndef BASIC_INFO_DLG
        SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_FORMAT,         dpxInfo->mCineonImage.GetImageFormat());
        SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_FRAME_ATTRIBUTE,dpxInfo->mCineonImage.GetImageFrameAttribute());
        SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_INPUT_DEVICE,   dpxInfo->mCineonImage.GetImageInputDevice());
        SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_INPUT_DEVICE_MODEL_NUMBER,  dpxInfo->mCineonImage.GetImageInputDeviceModelNumber());
        SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_INPUT_DEVICE_SERIAL_NUMBER, dpxInfo->mCineonImage.GetImageInputDeviceSerialNumber());
#endif
        if (dpxInfo->mCineonImage.IsSupported())
            SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_IS_SUPPORTED, GetResIDCaption(IDS_YES));
        else
            SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_IS_SUPPORTED, GetResIDCaption(IDS_NO));

#ifndef BASIC_INFO_DLG
        switch (dpxInfo->mCineonImage.GetImageOrientation()) {
        case 0:
            SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_ORIENTATION, GetResIDCaption(IDS_CIN_Image_Orientation_0));
            break;
        case 1:
            SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_ORIENTATION, GetResIDCaption(IDS_CIN_Image_Orientation_1));
            break;
        case 2:
            SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_ORIENTATION, GetResIDCaption(IDS_CIN_Image_Orientation_2));
            break;
        case 3:
            SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_ORIENTATION, GetResIDCaption(IDS_CIN_Image_Orientation_3));
            break;
        case 4:
            SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_ORIENTATION, GetResIDCaption(IDS_CIN_Image_Orientation_4));
            break;
        case 5:
            SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_ORIENTATION, GetResIDCaption(IDS_CIN_Image_Orientation_5));
            break;
        case 6:
            SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_ORIENTATION, GetResIDCaption(IDS_CIN_Image_Orientation_6));
            break;
        case 7:
            SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_ORIENTATION, GetResIDCaption(IDS_CIN_Image_Orientation_7));
            break;
        default:
            SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_ORIENTATION, "");
            break;
        }

#endif
     // dialog numbered fields
        char    buf[64];

        sprintf(buf,"%10u",     dpxInfo->mCineonImage.GetFileSize());
        SetDlgItemText(hWnd, IDC_INFO_DLG_FILE_SIZE,            buf);
        sprintf(buf,"%2u",      dpxInfo->mCineonImage.GetNumberChannels());
        SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_NUMBER_CHANNELS,buf);
        sprintf(buf,"%3u",      dpxInfo->mCineonImage.GetBitsPerPixel());
        SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_BITS_PER_PIXEL, buf);
        sprintf(buf,"%10u",     dpxInfo->mCineonImage.GetPixelsPerLine());
        SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_PIXELS_PER_LINE,buf);
        sprintf(buf,"%10u",     dpxInfo->mCineonImage.GetLinesPerImage());
        SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_LINES_PER_IMAGE,buf);
        
#ifndef BASIC_INFO_DLG
        sprintf(buf,"%.2g , %.2g",dpxInfo->mCineonImage.GetWhitePt(0), dpxInfo->mCineonImage.GetWhitePt(1));
        SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_WHITE_PT,       buf);
        sprintf(buf,"%.2g , %.2g",dpxInfo->mCineonImage.GetRedPt(0),   dpxInfo->mCineonImage.GetRedPt(1));
        SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_RED_PT,         buf);
        sprintf(buf,"%.2g , %.2g",dpxInfo->mCineonImage.GetGreenPt(0), dpxInfo->mCineonImage.GetGreenPt(1));
        SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_GREEN_PT,       buf);
        sprintf(buf,"%.2g , %.2g",dpxInfo->mCineonImage.GetBluePt(0),  dpxInfo->mCineonImage.GetBluePt(1));
        SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_BLUE_PT,        buf);
        sprintf(buf,"%3u",      dpxInfo->mCineonImage.GetDataInterleave());
        SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_DATA_INTERLEAVE,buf);
        sprintf(buf,"%3u",      dpxInfo->mCineonImage.GetPacking());
        SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_PACKING,        buf);
        sprintf(buf,"%2u",      dpxInfo->mCineonImage.GetSigned());
        SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_SIGNED,         buf);
        sprintf(buf,"%2u",      dpxInfo->mCineonImage.GetSense());
        SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_SENSE,          buf);
        sprintf(buf,"%10u",     dpxInfo->mCineonImage.GetEOLPadding());
        SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_EOL_PADDING,    buf);
        sprintf(buf,"%10u",     dpxInfo->mCineonImage.GetEOCPadding());
        SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_EOC_PADDING,    buf);
#endif
        sprintf(buf,"%.2g",     dpxInfo->mCineonImage.GetImageGamma());
        SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_GAMMA,          buf);
#ifndef BASIC_INFO_DLG
        sprintf(buf,"%10d",     dpxInfo->mCineonImage.GetImageXOffset());
        SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_X_OFFSET,       buf);
        sprintf(buf,"%10d",     dpxInfo->mCineonImage.GetImageYOffset());
        SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_Y_OFFSET,       buf);
        sprintf(buf,"%.2g",     dpxInfo->mCineonImage.GetDeviceXPitch());
        SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_DEVICE_X_PITCH, buf);
        sprintf(buf,"%.2g",     dpxInfo->mCineonImage.GetDeviceYPitch());
        SetDlgItemText(hWnd, IDC_INFO_DLG_IMAGE_DEVICE_Y_PITCH, buf);
        sprintf(buf,"%10u",     dpxInfo->mCineonImage.GetFramePosition());
        SetDlgItemText(hWnd, IDC_INFO_DLG_FRAME_POSITION,       buf);
        sprintf(buf,"%3u",      dpxInfo->mCineonImage.GetFilmMFGID());
        SetDlgItemText(hWnd, IDC_INFO_DLG_FILM_MFG_ID,          buf);
        sprintf(buf,"%3u",      dpxInfo->mCineonImage.GetFilmType());
        SetDlgItemText(hWnd, IDC_INFO_DLG_FILM_TYPE,            buf);
        sprintf(buf,"%3u",      dpxInfo->mCineonImage.GetFilmOffset());
        SetDlgItemText(hWnd, IDC_INFO_DLG_FILM_OFFSET,          buf);
        sprintf(buf,"%.2g",     dpxInfo->mCineonImage.GetFrameRate());
        SetDlgItemText(hWnd, IDC_INFO_DLG_FRAME_RATE, buf);
#endif
        return TRUE;
        }
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
            case IDCANCEL:
            case IDOK:
                EndDialog(hWnd, 1);
                break;
            }
        return TRUE;
    }
    return FALSE;
}

//--------------------< CIN_ControlDialog Struct methods >--------------------

CIN_ControlDialog::CIN_ControlDialog()
{
    mWhitePtSpinner     = NULL;
    mBlackPtSpinner     = NULL;
    mDensityLUT         = NULL;
    mRefPts[0]          = REFERENCE_WHITE_CODE;
    mRefPts[1]          = REFERENCE_BLACK_CODE;
    //mWhiteXPtSpinner    = NULL;
    //mWhiteYPtSpinner    = NULL;
    //mRedXPtSpinner      = NULL;
    //mRedYPtSpinner      = NULL;
    //mGreenXPtSpinner    = NULL;
    //mGreenYPtSpinner    = NULL;
    //mBlueXPtSpinner     = NULL;
    //mBlueYPtSpinner     = NULL;
}

CIN_ControlDialog::~CIN_ControlDialog()
{
    if (mDensityLUT) free(mDensityLUT);
    if (mWhitePtSpinner) {
        ReleaseISpinner(mWhitePtSpinner);
        mWhitePtSpinner = NULL;
    }
    if (mBlackPtSpinner) {
        ReleaseISpinner(mBlackPtSpinner);
        mBlackPtSpinner = NULL;
    }
    /*
    if (mWhiteXPtSpinner) {
        ReleaseISpinner(mWhiteXPtSpinner);
        mWhiteXPtSpinner = NULL;
    }
    if (mWhiteYPtSpinner) {
        ReleaseISpinner(mWhiteYPtSpinner);
        mWhiteYPtSpinner = NULL;
    }
    if (mRedXPtSpinner) {
        ReleaseISpinner(mRedXPtSpinner);
        mRedXPtSpinner = NULL;
    }
    if (mRedYPtSpinner) {
        ReleaseISpinner(mRedYPtSpinner);
        mRedYPtSpinner = NULL;
    }
    if (mGreenXPtSpinner) {
        ReleaseISpinner(mGreenXPtSpinner);
        mGreenXPtSpinner = NULL;
    }
    if (mGreenYPtSpinner) {
        ReleaseISpinner(mGreenYPtSpinner);
        mGreenYPtSpinner = NULL;
    }
    if (mBlueXPtSpinner) {
        ReleaseISpinner(mBlueXPtSpinner);
        mBlueXPtSpinner = NULL;
    }
    if (mBlueYPtSpinner) {
        ReleaseISpinner(mBlueYPtSpinner);
        mBlueYPtSpinner = NULL;
    }
    */
}

void
CIN_ControlDialog::DrawBox(HWND hDlg) {
    HWND hWnd = GetDlgItem(hDlg, IDC_VIEW);
    HDC hdc = GetDC(hWnd);
    RECT r;
    POINT pt[2];
    GetClientRect(hWnd, &r);
    int i;

    HBRUSH hb = CreateSolidBrush(RGB(255, 255, 255));
    r.bottom--; r.top++; r.left++; r.right--;
    FillRect(hdc, &r, hb);
    DeleteObject(hb);
    
    float numWSteps   = 1022.0f;
    float numHSteps   = 65536.0f;
    float numGrids    = 8;
    float stepWidth   = (float) ((r.right - r.left) / numWSteps);
    float stepHeight  = (float) ((r.bottom - r.top) / numHSteps);
    float gridWidth   = (float) ((r.right - r.left) / numGrids);
    float gridHeight  = (float) ((r.bottom - r.top) / numGrids);

    for (i = 1; i < numGrids; i++) {
        pt[0].x = (long) (r.left   + (gridWidth * i));
        pt[0].y = (long) (r.bottom);
        pt[1].x = (long) (r.left   + (gridWidth * i));
        pt[1].y = (long) (r.top);
        Polyline(hdc, pt, 2);
    }

    for (i = 1; i < numGrids; i++) {
        pt[0].x = (long) (r.left);
        pt[0].y = (long) (r.bottom - (gridHeight * i));
        pt[1].x = (long) (r.right);
        pt[1].y = (long) (r.bottom - (gridHeight * i));
        Polyline(hdc, pt, 2);
    }

    if (mDensityLUT) {
        for (i = 0; i < (int) numWSteps; i++) {
            pt[0].x = (long) (r.left   + (i * stepWidth));
            pt[0].y = (long) (r.bottom - (mDensityLUT[(U16) i] * stepHeight));
            pt[1].x = (long) (r.left   + ((i + 1.0f) * stepWidth));
            pt[1].y = (long) (r.bottom - (mDensityLUT[(U16) i + 1] * stepHeight));
            Polyline(hdc, pt, 2);
        }
    }
    //DeleteObject(pen);
    //pen  = CreatePen(PS_SOLID, 0, RGB(255,255,0));

    //DeleteObject(pen);
    DeleteDC(hdc);
}

//FIXME add user configurable ref values.
//90% w 685
//18% g 470
//2%  b 180

BOOL
CIN_ControlDialog::SetDensities()
{
    U8      bitsPerPixel = 10;
    R32     refWhite   = (mRefPts[0] > 0 && mRefPts[0] <= TEN_BIT_MAX) ? mRefPts[0] : (float) REFERENCE_WHITE_CODE;
    R32     refBlack   = (mRefPts[1] > 0 && mRefPts[1] <= TEN_BIT_MAX) ? mRefPts[1] : (float) REFERENCE_BLACK_CODE;
 // should be mSoftClip 0 < 50
    R32     mSoftclip   = 1.0f;
    U32     LUTsize     = (U32) pow(2, bitsPerPixel);
    R32     densityStep  = CALIBRATED_DENSITY / (float) LUTsize;

    if (mDensityLUT) free(mDensityLUT);

    if (!(mDensityLUT = (U16*) calloc(LUTsize, sizeof(U16)))) {
        return FALSE;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Look at cineon.cpp::SetDensities() for the description this algorithm
    double Gain = SIXTEEN_BIT_MAX / (1- pow(10, (refBlack - refWhite) * densityStep/0.6f ));
    double Offset = Gain - SIXTEEN_BIT_MAX;
    double dDensityLUT;

    for (U32 cnt = 0; cnt < LUTsize; cnt++) 
    {
        if (cnt < refBlack) 
        {
            mDensityLUT[cnt] = 0;
            continue;
        }

        dDensityLUT = pow(10,(cnt-refWhite)*densityStep/0.6f)*Gain - Offset;    
        if (dDensityLUT > SIXTEEN_BIT_MAX)
            mDensityLUT[cnt] = SIXTEEN_BIT_MAX;
        else
            mDensityLUT[cnt] = (U16)dDensityLUT;
    }
 
    return TRUE;
}

//FIXME
BOOL
CIN_ControlDialog::InitSpinners(HWND hWnd)
{
 // black and white spinners
    mWhitePtSpinner = GetISpinner(GetDlgItem(hWnd, IDC_WHITE_PT_SPINNER));
    mWhitePtSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_WHITE_PT_EDIT), EDITTYPE_INT);
    mBlackPtSpinner = GetISpinner(GetDlgItem(hWnd, IDC_BLACK_PT_SPINNER));
    mBlackPtSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_BLACK_PT_EDIT), EDITTYPE_INT);
    /*
     // white PT spinners
        mWhiteXPtSpinner = GetISpinner(GetDlgItem(hWnd, IDC_WHITE_X_PT_SPINNER));
        mWhiteXPtSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_WHITE_X_PT_EDIT), EDITTYPE_FLOAT);
        mWhiteYPtSpinner = GetISpinner(GetDlgItem(hWnd, IDC_WHITE_Y_PT_SPINNER));
        mWhiteYPtSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_WHITE_Y_PT_EDIT), EDITTYPE_FLOAT);
     // Red PT spinners
        mRedXPtSpinner = GetISpinner(GetDlgItem(hWnd, IDC_RED_X_PT_SPINNER));
        mRedXPtSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_RED_X_PT_EDIT), EDITTYPE_FLOAT);
        mRedYPtSpinner = GetISpinner(GetDlgItem(hWnd, IDC_RED_Y_PT_SPINNER));
        mRedYPtSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_RED_Y_PT_EDIT), EDITTYPE_FLOAT);
     // Green PT spinners
        mGreenXPtSpinner = GetISpinner(GetDlgItem(hWnd, IDC_GREEN_X_PT_SPINNER));
        mGreenXPtSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_GREEN_X_PT_EDIT), EDITTYPE_FLOAT);
        mGreenYPtSpinner = GetISpinner(GetDlgItem(hWnd, IDC_GREEN_Y_PT_SPINNER));
        mGreenYPtSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_GREEN_Y_PT_EDIT), EDITTYPE_FLOAT);
     // Blue PT spinners
        mBlueXPtSpinner = GetISpinner(GetDlgItem(hWnd, IDC_BLUE_X_PT_SPINNER));
        mBlueXPtSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_BLUE_X_PT_EDIT), EDITTYPE_FLOAT);
        mBlueYPtSpinner = GetISpinner(GetDlgItem(hWnd, IDC_BLUE_Y_PT_SPINNER));
        mBlueYPtSpinner->LinkToEdit(GetDlgItem(hWnd, IDC_BLUE_Y_PT_EDIT), EDITTYPE_FLOAT);
        */

 // black and white spinner
    mWhitePtSpinner->SetScale(1);
    mWhitePtSpinner->SetLimits(1, 1024);
    mWhitePtSpinner->SetValue(REFERENCE_WHITE_CODE, TRUE);
    mWhitePtSpinner->SetResetValue(REFERENCE_WHITE_CODE);

    mBlackPtSpinner->SetScale(1);
    mBlackPtSpinner->SetLimits(1, 1024);
    mBlackPtSpinner->SetValue(REFERENCE_BLACK_CODE, TRUE);
    mBlackPtSpinner->SetResetValue(REFERENCE_BLACK_CODE);
    
     // white PT spinners//fixme #define scale and limits
    /*
        mWhiteXPtSpinner->SetScale(0.0005f);
        mWhiteXPtSpinner->SetLimits(0.0001f, 1.0f);
        mWhiteXPtSpinner->SetResetValue(0.1f);
        mWhiteYPtSpinner->SetScale(0.0005f);
        mWhiteYPtSpinner->SetLimits(0.0001f, 1.0f);
        mWhiteYPtSpinner->SetResetValue(0.1f);
     // Red PT spinners
        mRedXPtSpinner->SetScale(0.0005f);
        mRedXPtSpinner->SetLimits(0.0001f, 1.0f);
        mRedXPtSpinner->SetResetValue(0.1f);
        mRedYPtSpinner->SetScale(0.0005f);
        mRedYPtSpinner->SetLimits(0.0001f, 1.0f);
        mRedYPtSpinner->SetResetValue(0.1f);
     // Green PT spinners
        mGreenXPtSpinner->SetScale(0.0005f);
        mGreenXPtSpinner->SetLimits(0.0001f, 1.0f);
        mGreenXPtSpinner->SetResetValue(0.1f);
        mGreenYPtSpinner->SetScale(0.0005f);
        mGreenYPtSpinner->SetLimits(0.0001f, 1.0f);
        mGreenYPtSpinner->SetResetValue(0.1f);
     // Blue PT spinners
        mBlueXPtSpinner->SetScale(0.0005f);
        mBlueXPtSpinner->SetLimits(0.0001f, 1.0f);
        mBlueXPtSpinner->SetResetValue(0.1f);
        mBlueYPtSpinner->SetScale(0.0005f);
        mBlueYPtSpinner->SetLimits(0.0001f, 1.0f);
        mBlueYPtSpinner->SetResetValue(0.1f);
        */
        /*
		if (UserData.defaultcfg) ReadCfg();
		SetDlgItemText(hWnd, IDC_CIN_USER, UserData.user);
		SetDlgItemText(hWnd, IDC_CIN_DESCRIPTION, UserData.desc);
		CheckDlgButton(hWnd,IDC_ALPHA,		UserData.usealpha);
		CheckDlgButton(hWnd,IDC_Z,		  	UserData.channels & BMM_CHAN_Z);
		CheckDlgButton(hWnd,IDC_MATERIAL,	UserData.channels & BMM_CHAN_MTL_ID);
		CheckDlgButton(hWnd,IDC_NODE,	  	UserData.channels & BMM_CHAN_NODE_ID);
		CheckDlgButton(hWnd,IDC_UV,			UserData.channels & BMM_CHAN_UV);
		CheckDlgButton(hWnd,IDC_NORMAL,   	UserData.channels & BMM_CHAN_NORMAL);
		CheckDlgButton(hWnd,IDC_CLAMPRGB, 	UserData.channels & BMM_CHAN_REALPIX);
		CheckDlgButton(hWnd,IDC_COVERAGE, 	UserData.channels & BMM_CHAN_COVERAGE);
		CheckRadioButton(hWnd,IDC_RGB8,	IDC_RGB16,	UserData.rgb16?IDC_RGB16:IDC_RGB8);
        */
	return TRUE;
}

void
CIN_ControlDialog::RemoveSpinners()
{
    if (mWhitePtSpinner) {
        ReleaseISpinner(mWhitePtSpinner);
        mWhitePtSpinner = NULL;
    }
    if (mBlackPtSpinner) {
        ReleaseISpinner(mBlackPtSpinner);
        mBlackPtSpinner = NULL;
    }
    /*
    if (mWhiteXPtSpinner) {
        ReleaseISpinner(mWhiteXPtSpinner);
        mWhiteXPtSpinner = NULL;
    }
    if (mWhiteYPtSpinner) {
        ReleaseISpinner(mWhiteYPtSpinner);
        mWhiteYPtSpinner = NULL;
    }
    if (mRedXPtSpinner) {
        ReleaseISpinner(mRedXPtSpinner);
        mRedXPtSpinner = NULL;
    }
    if (mRedYPtSpinner) {
        ReleaseISpinner(mRedYPtSpinner);
        mRedYPtSpinner = NULL;
    }
    if (mGreenXPtSpinner) {
        ReleaseISpinner(mGreenXPtSpinner);
        mGreenXPtSpinner = NULL;
    }
    if (mGreenYPtSpinner) {
        ReleaseISpinner(mGreenYPtSpinner);
        mGreenYPtSpinner = NULL;
    }
    if (mBlueXPtSpinner) {
        ReleaseISpinner(mBlueXPtSpinner);
        mBlueXPtSpinner = NULL;
    }
    if (mBlueYPtSpinner) {
        ReleaseISpinner(mBlueYPtSpinner);
        mBlueYPtSpinner = NULL;
    }
    */
}

void
CIN_ControlDialog::SpinnersChanged(WORD which)
{
    switch(which) {
        case IDC_WHITE_PT_SPINNER:
            if (mWhitePtSpinner) {
                if (mWhitePtSpinner->GetIVal() >= mRefPts[1])
                    mRefPts[0] = mWhitePtSpinner->GetIVal();
                else
                    mWhitePtSpinner->SetValue(mBlackPtSpinner->GetIVal(), FALSE);
            }
        break;
        case IDC_BLACK_PT_SPINNER:
            if (mBlackPtSpinner) {
                if (mBlackPtSpinner->GetIVal() <= mRefPts[0])
                    mRefPts[1] = mBlackPtSpinner->GetIVal();
                else
                    mBlackPtSpinner->SetValue(mWhitePtSpinner->GetIVal(), FALSE);
            }
        break;
        /*
    case IDC_WHITE_X_PT_SPINNER:
        if (mWhiteXPtSpinner) 
            mWhite[0]   = mWhiteXPtSpinner->GetFVal();
        break;
    case IDC_WHITE_Y_PT_SPINNER:
        if (mWhiteYPtSpinner) 
            mWhite[1]   = mWhiteYPtSpinner->GetFVal();
        break;
    case IDC_RED_X_PT_SPINNER:
        if (mRedXPtSpinner) 
            mRed[0]     = mRedXPtSpinner->GetFVal();
        break;
    case IDC_RED_Y_PT_SPINNER:
        if (mRedYPtSpinner) 
            mRed[1]     = mRedYPtSpinner->GetFVal();
        break;
    case IDC_GREEN_X_PT_SPINNER:
        if (mGreenXPtSpinner) 
            mGreen[0]   = mGreenXPtSpinner->GetFVal();
        break;
    case IDC_GREEN_Y_PT_SPINNER:
        if (mGreenYPtSpinner) 
            mGreen[1]   = mGreenYPtSpinner->GetFVal();
        break;
    case IDC_BLUE_X_PT_SPINNER:
        if (mBlueXPtSpinner) 
            mBlue[0]    = mBlueXPtSpinner->GetFVal();
        break;
    case IDC_BLUE_Y_PT_SPINNER:
        if (mBlueYPtSpinner) 
            mBlue[1]    = mBlueYPtSpinner->GetFVal();
        break;
        */
    default:
        break;
    }
    SetDensities();
}

INT_PTR CALLBACK
CIN_ControlDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static CIN_ControlDialog theCtrlDlg;
    static BitmapIO_CIN*       CIN_BitmapIO;
    switch (msg) {
    case WM_INITDIALOG:
        CIN_BitmapIO = (BitmapIO_CIN*) lParam;
        CenterWindow(hWnd,GetParent(hWnd));
        SetCursor(LoadCursor(NULL,IDC_ARROW));

        theCtrlDlg.InitSpinners(hWnd);

        CIN_BitmapIO->ReadINI();
        theCtrlDlg.mRefPts[0] = CIN_BitmapIO->mUserData.mRefWhite;
        theCtrlDlg.mRefPts[1] = CIN_BitmapIO->mUserData.mRefBlack;

        theCtrlDlg.mWhitePtSpinner->SetValue(theCtrlDlg.mRefPts[0], TRUE);
        theCtrlDlg.mBlackPtSpinner->SetValue(theCtrlDlg.mRefPts[1], TRUE);
        theCtrlDlg.SetDensities();

        return TRUE;
        
    case WM_PAINT:
        theCtrlDlg.DrawBox(hWnd);
        break;
    case CC_SPINNER_CHANGE:
    case WM_CUSTEDIT_ENTER:
        theCtrlDlg.SpinnersChanged(LOWORD(wParam));
        theCtrlDlg.DrawBox(hWnd);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {                
        case IDOK:
            /*
            UserData.usealpha  	= IsDlgButtonChecked(hWnd,IDC_ALPHA);
			UserData.channels 	= 0;
			UserData.rgb16		= IsDlgButtonChecked(hWnd,IDC_RGB16)?1:0;
			if (IsDlgButtonChecked(hWnd,IDC_Z))
				UserData.channels |= BMM_CHAN_Z;
			if (IsDlgButtonChecked(hWnd,IDC_MATERIAL))
				UserData.channels |= BMM_CHAN_MTL_ID;
			if (IsDlgButtonChecked(hWnd,IDC_NODE))
				UserData.channels |= BMM_CHAN_NODE_ID;
			if (IsDlgButtonChecked(hWnd,IDC_UV))
				UserData.channels |= BMM_CHAN_UV;
			if (IsDlgButtonChecked(hWnd,IDC_NORMAL))
				UserData.channels |= BMM_CHAN_NORMAL;
			if (IsDlgButtonChecked(hWnd,IDC_CLAMPRGB))
				UserData.channels |= BMM_CHAN_REALPIX;
			if (IsDlgButtonChecked(hWnd,IDC_COVERAGE))
				UserData.channels |= BMM_CHAN_COVERAGE;

            SendDlgItemMessage(hWnd, IDC_CIN_DESCRIPTION, WM_GETTEXT, 128, (LPARAM)UserData.desc);
			UserData.desc[127] = 0;
			SendDlgItemMessage(hWnd, IDC_CIN_USER, WM_GETTEXT, 31, (LPARAM)UserData.user);
			UserData.user[31] = 0;
			WriteCfg();
            */
            //FIXME get the PT values
            CIN_BitmapIO->mUserData.mRefWhite = theCtrlDlg.mRefPts[0];
            CIN_BitmapIO->mUserData.mRefBlack = theCtrlDlg.mRefPts[1];
            
            CIN_BitmapIO->WriteINI();

			EndDialog(hWnd, TRUE);
			break;
        case IDCANCEL:
            EndDialog(hWnd, FALSE);
            break;
	    }
		return TRUE;
        case WM_DESTROY:
            theCtrlDlg.RemoveSpinners();
            break;
    }

	return FALSE;
}



//--------------------< BitmapIO_CIN Class methods >--------------------
BitmapIO_CIN::BitmapIO_CIN()
{
    mStream         = NULL;
}

BitmapIO_CIN::~BitmapIO_CIN()
{
    if (mStream) {
        fclose(mStream);
        mStream = NULL;
    }
}

const TCHAR*
BitmapIO_CIN::AuthorName(void)
{
    return GetResIDCaption(IDS_CIN_Author);
}

//FIXME Add control for curve RefPts
int
BitmapIO_CIN::Capability()
{
    //return  BMMIO_CONTROLREAD | BMMIO_CONTROLWRITE | BMMIO_EXTENSION | BMMIO_INFODLG | BMMIO_READER | BMMIO_WRITER;
    return  BMMIO_EXTENSION | BMMIO_INFODLG | BMMIO_READER | BMMIO_WRITER | BMMIO_CONTROLWRITE | BMMIO_CONTROLREAD;
}

DWORD
BitmapIO_CIN::ChannelsRequired()
{
    return BMM_CHAN_NONE;
}

int
BitmapIO_CIN::Close(int flag)
{
    if (openMode != BMM_OPEN_W)
        return FALSE;
    else
        return TRUE;
}

const TCHAR*
BitmapIO_CIN::CopyrightMessage(void)
{
    return GetResIDCaption(IDS_CIN_Copyright);
}

DWORD
BitmapIO_CIN::EvaluateConfigure(void)
{
    return sizeof(CIN_UserData);
}

const TCHAR*
BitmapIO_CIN::Ext(int n)
{
    switch (n) {
    case 0:
        return _T("cin");
    default:
        assert(FALSE);
    }
    return NULL;
}

int          
BitmapIO_CIN::ExtCount()
{
    return 1;
}

BMMRES
BitmapIO_CIN::GetImageInfo(BitmapInfo* bmi)
{
    if (!bmi) {
        assert(FALSE);
        return ProcessImageIOError(bmi, GetResIDCaption(IDS_CIN_Internal_Error));
    }

    if (mStream)
        fclose(mStream);

    File file(bmi->Name(), _T("rb"));

    if (!file.mStream)
        return ProcessImageIOError(bmi, GetResIDCaption(IDS_CIN_File_Open_Error));

    mStream = file.mStream;

    if (openMode != BMM_NOT_OPEN)
        return ProcessImageIOError(bmi, GetResIDCaption(IDS_CIN_Internal_Error));

    if (mCineonImage.ReadHeader(mStream) == FALSE)
        return ProcessImageIOError(bmi, GetResIDCaption(IDS_CIN_Corrupt_File_Error));

    if (mCineonImage.VerifyHeader() == FALSE)
        return ProcessImageIOError(bmi, GetResIDCaption(IDS_CIN_Invalid_Header_Error));

 // FIXME read the header
    bmi->SetWidth(mCineonImage.GetPixelsPerLine());
    bmi->SetHeight(mCineonImage.GetLinesPerImage());
    bmi->SetType(BMM_TRUE_48);
    bmi->SetAspect(1.0f);
    bmi->SetGamma(mCineonImage.GetImageGamma());
    bmi->SetFirstFrame(0);
    bmi->SetLastFrame(0);

    return BMMRES_SUCCESS;
}

BMMRES
BitmapIO_CIN::GetImageInfoDlg(HWND hWnd, BitmapInfo* bmi, const TCHAR* fname)
{
    BitmapInfo  bInfo;
    if (!bmi) {
        assert(FALSE);
        return ProcessImageIOError(bmi, GetResIDCaption(IDS_CIN_Internal_Error));
    }

	bInfo.Copy(bmi);

    if (fname)
		bInfo.SetName(fname);

    BMMRES bmmResult = GetImageInfo(&bInfo);
    if (bmmResult != BMMRES_SUCCESS) {
        return bmmResult;
    }

#ifdef BASIC_INFO_DLG
    DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CIN_BASIC_INFO),
                   hWnd, (DLGPROC) CIN_ImageInfoDialogProc, (LPARAM) this);
#else
    DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CIN_INFO),
                   hWnd, (DLGPROC) CIN_ImageInfoDialogProc, (LPARAM) this);
#endif

    return BMMRES_SUCCESS;
}

//FIXME
BitmapStorage*
BitmapIO_CIN::Load(BitmapInfo* bmi, Bitmap* bm, unsigned short* status)
{
    BitmapStorage* bms = NULL;
    File file(bmi->Name(), _T("rb"));

    *status = BMMRES_SUCCESS;
    
    if (!file.mStream) {
        *status = ProcessImageIOError(bmi, GetResIDCaption(IDS_CIN_File_Open_Error));
        return NULL;
    }

    mStream = file.mStream;

    if (openMode != BMM_NOT_OPEN) {
        *status = ProcessImageIOError(bmi, GetResIDCaption(IDS_CIN_Internal_Error));
        return NULL;
    }

    CineonFile cineonImage(mStream);

    if (cineonImage.VerifyHeader() == FALSE) {
        *status = ProcessImageIOError(bmi, GetResIDCaption(IDS_CIN_Invalid_Header_Error));
        return NULL;
    }
    
    if (!cineonImage.IsSupported()) {
        *status = ProcessImageIOError(bmi, GetResIDCaption(IDS_CIN_Unsupported_File_Error));
        return NULL;
    }

    unsigned int pixelsPL = cineonImage.GetPixelsPerLine();
    unsigned int linesPI  = cineonImage.GetLinesPerImage();

    bmi->SetWidth(pixelsPL);
    bmi->SetHeight(linesPI);
 // bmi->SetGamma(cineonImage.GetImageGamma());
 // bmi->SetAspect();
    bmi->SetFirstFrame(0);
    bmi->SetLastFrame(0);
 // bmi->SetFlags(MAP_NOFLAGS);

    bms = BMMCreateStorage(bm->Manager(), BMM_TRUE_64);
    if (!bms) {
        *status = ProcessImageIOError(bmi, GetResIDCaption(IDS_CIN_Internal_Error));
        return NULL;
    }

    if (!(bms->Allocate(bmi, bm->Manager(), BMM_OPEN_R))) {
        *status = ProcessImageIOError(bmi, GetResIDCaption(IDS_CIN_Memory_Error));
        delete bms;
        bms = NULL;
        return NULL;
    }

    BMM_Color_64* scanLine = (BMM_Color_64*) calloc(pixelsPL, sizeof(BMM_Color_64));
    if (!scanLine) {
        *status = ProcessImageIOError(bmi, GetResIDCaption(IDS_CIN_Memory_Error));
        if (bms) { delete bms; bms = NULL; }
        return NULL;
    }

    if (!cineonImage.SetLUTs(10, (float) mUserData.mRefWhite, (float) mUserData.mRefBlack)) {
        *status = ProcessImageIOError(bmi, GetResIDCaption(IDS_CIN_Memory_Error));
        if (bms) { delete bms; bms = NULL; }
        return NULL;
    }

    for (unsigned int cnt = 0; cnt < linesPI; cnt++) {
     // 4 WORD Channels
        if (!cineonImage.GetScanLine((unsigned short*)scanLine, cnt, pixelsPL)) {
            *status = ProcessImageIOError(bmi, GetResIDCaption(IDS_CIN_File_IO_Error));
            if (bms) { delete bms; bms = NULL; }
            if (scanLine) { free(scanLine); scanLine = NULL; }
            return NULL;
        }
     // test colors ~ 5sec 1828x1332
        /*
        float r1 = ((float) cnt) / ((float) linesPI);
        for (unsigned int i = 0; i < pixelsPL; i++) {
            float r2 = ((float) i) / ((float) pixelsPL);
            scanLine[i].r = (int) (65535.f *  r1);
            scanLine[i].g = (int) (65535.f *  r2);
            scanLine[i].b = (int) (65535.f * (r1 * r2));
            scanLine[i].a = 0;  //CIN has no alpha
        }
        */

        if (!bms->PutPixels(0, cnt, pixelsPL, scanLine)) {
            *status = ProcessImageIOError(bmi, GetResIDCaption(IDS_CIN_Internal_Error));
            if (bms) { delete bms; bms = NULL; }
            if (scanLine) { free(scanLine); scanLine = NULL; }
            return NULL;
        }
    }

    if (scanLine) { free(scanLine); scanLine = NULL; }
    openMode = BMM_OPEN_R;
    return bms;
}

const TCHAR*
BitmapIO_CIN::LongDesc(void)
{
    return GetResIDCaption(IDS_CIN_LongDesc);
}

BOOL
BitmapIO_CIN::LoadConfigure(void* ptr)
{
    if (!ptr) {
        assert(FALSE);
        return FALSE;
    }

    CIN_UserData* buf = (CIN_UserData*) ptr;
    switch (buf->version) {
    case CIN_USERDATA_VERSION:
        memcpy((void*) &mUserData, ptr, sizeof(CIN_UserData));
        return TRUE;
    default:
        assert(FALSE);
    }
    return FALSE;
}

BMMRES
BitmapIO_CIN::OpenOutput(BitmapInfo* bmi, Bitmap* bm)
{
    if (openMode != BMM_NOT_OPEN)
        return ProcessImageIOError(bmi, GetResIDCaption(IDS_CIN_Internal_Error));

    if (!bm)
        return ProcessImageIOError(bmi, GetResIDCaption(IDS_CIN_Internal_Error));

    map   = bm;

    bi.CopyImageInfo(bmi);
    bi.SetUpdateWindow(bmi->GetUpdateWindow());

    openMode    = BMM_OPEN_W;
    return BMMRES_SUCCESS;
}

BOOL
BitmapIO_CIN::SaveConfigure(void* ptr)
{
    if (!ptr) {
        assert(FALSE);
        return FALSE;
    }
    
    memcpy(ptr, (void*) &mUserData, sizeof(CIN_UserData));
    return TRUE;
}

const TCHAR*
BitmapIO_CIN::ShortDesc(void)
{
    return GetResIDCaption(IDS_CIN_ShortDesc);
}

void
BitmapIO_CIN::ShowAbout(HWND hWnd)
{
    DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CIN_ABOUT),
                   hWnd,  (DLGPROC) CIN_AboutDialogProc, (LPARAM) NULL);
    return;
}

//FIXME
BOOL
BitmapIO_CIN::ShowControl(HWND hWnd, DWORD flags)
{
    if (flags & BMMIO_CONTROLREAD) {};
    if (flags & BMMIO_CONTROLWRITE) {};

    return DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CIN_CONTROL),
                          hWnd,  (DLGPROC) CIN_ControlDialogProc,
                          (LPARAM) this);
}

BOOL
BitmapIO_CIN::ShowImage(HWND hWnd, BitmapInfo* bmi)
{
    return FALSE;
}

unsigned int
BitmapIO_CIN::Version(void)
{
    return CIN_BITMAPIO_VERSION;
}

BMMRES
BitmapIO_CIN::Write(int frameNum)
{
    TCHAR filename[MAX_PATH];

    if (openMode != BMM_OPEN_W)
        return ProcessImageIOError(&bi, GetResIDCaption(IDS_CIN_Internal_Error));
    
    if (frameNum != BMM_SINGLEFRAME) {
        if (!BMMCreateNumberedFilename(bi.Name(), frameNum, filename)) {
            return ProcessImageIOError(&bi, BMMRES_NUMBEREDFILENAMEERROR);
        }
    } else {
        _tcscpy(filename, bi.Name());
    }

    File file(filename, _T("wb"));

    if (!file.mStream)
        return ProcessImageIOError(&bi, GetResIDCaption(IDS_CIN_File_Open_Error));

    mStream = file.mStream;

	BitmapStorage*  bms = map->Storage();
    if (!bms)
        return ProcessImageIOError(&bi, GetResIDCaption(IDS_CIN_Internal_Error));

	unsigned int height  = map->Height();
    unsigned int width   = map->Width();

    CineonFile cineonImage;

    cineonImage.SetImageFileName(filename);

    char            cDate[128] = "";
    char            cTime[128] = "";
    WIN32_FIND_DATA findFile;
    HANDLE          findhnd = FindFirstFile(bi.Name(), &findFile);
	SYSTEMTIME      time;
    SYSTEMTIME      local;
	FindClose(findhnd);
	if (findhnd != INVALID_HANDLE_VALUE) {
	    FileTimeToSystemTime(&findFile.ftLastWriteTime, &time);
		if (!SystemTimeToTzSpecificLocalTime(NULL, &time, &local))
		    local = time;
		sprintf(cDate,"%d:%02d:%02d", local.wYear, local.wMonth, local.wDay);
        sprintf(cTime,"%02d:%02d:%02d", local.wHour, local.wMinute, local.wSecond);
	}
	
    cineonImage.SetImageCreationDate(cDate);
    cineonImage.SetImageCreationTime(cTime);
    cineonImage.SetImageInputDevice("3D Studio MAX");
    cineonImage.SetImageInputDeviceModelNumber("4.0");
    char buf[CINEON_HDR_DEVICE_SERIAL_NUMBER_LENGTH];
    sprintf(buf, "%d", HardwareLockID());
    cineonImage.SetImageInputDeviceSerialNumber(buf);
    cineonImage.SetImageOrientation(0);
    
    cineonImage.SetNumberChannels(3);
    for (int i = 0; i < 3; i++) {
        cineonImage.SetBitsPerPixel(i, 10);
        cineonImage.SetPixelsPerLine(i, width);
        cineonImage.SetLinesPerImage(i, height);
    }

 // Put in the NTSC, defaults. max don't allow for correction/modification
 // FIXME
    cineonImage.SetWhitePt(0.3324f, 0.3474f);
    cineonImage.SetRedPt(0.67f, 0.33f);
    cineonImage.SetGreenPt(0.21f, 0.71f);
    cineonImage.SetBluePt(0.14f, 0.08f);
    cineonImage.SetDataInterleave(0);
    cineonImage.SetPacking(5);
    cineonImage.SetSigned(0);
    cineonImage.SetSense(0);
    cineonImage.SetEOLPadding(0);
    cineonImage.SetEOCPadding(0);
    cineonImage.SetImageGamma(1.0f);
    cineonImage.SetImageXOffset(0);
    cineonImage.SetImageYOffset(0);
    cineonImage.SetFramePosition(frameNum);
    
    if (cineonImage.VerifyHeader() == FALSE)
        return ProcessImageIOError(&bi, GetResIDCaption(IDS_CIN_Invalid_Header_Error));

    if (!cineonImage.IsSupported())
        return ProcessImageIOError(&bi, GetResIDCaption(IDS_CIN_Unsupported_File_Error));

    BMM_Color_64*   scanLine = (BMM_Color_64*) calloc(width, sizeof(BMM_Color_64));
    if (!scanLine)
        return ProcessImageIOError(&bi, GetResIDCaption(IDS_CIN_Memory_Error));

    if (!cineonImage.SetLUTs(10, (float) mUserData.mRefWhite, (float) mUserData.mRefBlack))
        return ProcessImageIOError(&bi, GetResIDCaption(IDS_CIN_Memory_Error));

    for (unsigned int lineCnt = 0; lineCnt < height; lineCnt++) {
        GetOutputPixels(0, lineCnt, width, scanLine);
        if (!cineonImage.WriteScanLine(file.mStream, (unsigned short*) scanLine, lineCnt, width)) {
            if (scanLine) { free(scanLine); scanLine = NULL; }
            return ProcessImageIOError(&bi, GetResIDCaption(IDS_CIN_File_IO_Error));
        }
    }

    if (scanLine) { free(scanLine); scanLine = NULL; }

    fpos_t fileSize;
    fgetpos(file.mStream, &fileSize);
    cineonImage.SetFileSize((unsigned int) fileSize);

    if (cineonImage.WriteHeader(mStream) == FALSE)
        return ProcessImageIOError(&bi, GetResIDCaption(IDS_CIN_File_IO_Error));

    return BMMRES_SUCCESS;
}

void
BitmapIO_CIN::GetINIFilename(TCHAR* filename)
{
    if (!filename) {
        assert(FALSE);
    }

    _tcscpy(filename, TheManager->GetDir(APP_PLUGCFG_DIR));

    if (filename)
        if (_tcscmp(&filename[_tcslen(filename)-1], _T("\\")))
            _tcscat(filename, _T("\\"));
    _tcscat(filename, CIN_INI_FILENAME);   
}

//FIXME
void
BitmapIO_CIN::ReadINI()
{
    TCHAR filename[MAX_PATH];
    TCHAR buffer[256];
    GetINIFilename(filename);

    GetPrivateProfileString(CIN_INI_SECTION, CIN_INI_VERSION, buffer, buffer, sizeof(buffer), filename);
    mUserData.version = (DWORD) atoi(buffer);
    if (mUserData.version != CIN_USERDATA_VERSION) {
        mUserData.version   = CIN_USERDATA_VERSION;
        mUserData.mRefWhite = REFERENCE_WHITE_CODE;
        mUserData.mRefBlack = REFERENCE_BLACK_CODE;
    } else {
        GetPrivateProfileString(CIN_INI_SECTION, CIN_INI_REF_WHITE,   buffer, buffer, sizeof(buffer), filename);
        mUserData.mRefWhite = (DWORD) atoi(buffer);
        GetPrivateProfileString(CIN_INI_SECTION, CIN_INI_REF_BLACK,   buffer, buffer, sizeof(buffer), filename);
        mUserData.mRefBlack = (DWORD) atoi(buffer);
    }
}

//FIXME
void
BitmapIO_CIN::WriteINI()
{
    TCHAR filename[MAX_PATH];
    TCHAR buffer[256];
    GetINIFilename(filename);
    
    wsprintf(buffer,"%d", CIN_USERDATA_VERSION);
    WritePrivateProfileString(CIN_INI_SECTION, CIN_INI_VERSION, buffer, filename);

    wsprintf(buffer,"%d", mUserData.mRefWhite);
    WritePrivateProfileString(CIN_INI_SECTION, CIN_INI_REF_WHITE, buffer, filename);
    wsprintf(buffer,"%d", mUserData.mRefBlack);
    WritePrivateProfileString(CIN_INI_SECTION, CIN_INI_REF_BLACK, buffer, filename);
}
