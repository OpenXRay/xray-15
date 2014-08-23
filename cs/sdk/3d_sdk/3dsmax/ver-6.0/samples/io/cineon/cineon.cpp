/**********************************************************************
 *
 * FILE:        cineon.cpp
 * AUTHOR:      greg finch
 *
 * DESCRIPTION: SMPTE Digital Picture Exchange Format,
 *              SMPTE CIN, Kodak Cineon
 *
 * CREATED:     20 june, 1997
 *
 *
 * 
 * Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

// System includes
#include <windows.h>

// Some standard library includes
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

#include "cineon.h"

//test
//#define SET_TEST_PATTERN

unsigned int
swap(unsigned int x)
{
    return  ((x >> 24) & 0x000000ff) |
            ((x >>  8) & 0x0000ff00) |
            ((x <<  8) & 0x00ff0000) |
            ((x << 24) & 0xff000000);
}

float
swap(float f)
{
    unsigned int x;
    BOOL isFinite;
    memcpy(&x, &f, sizeof(float));
    x = swap(x);
    memcpy(&f, &x, sizeof(unsigned int));
    isFinite = _finite(f);  //FIXME do something non-finite numbers
    return f;
}

unsigned short
swap(unsigned short x)
{
    return  ((x >> 8) & 0x00ff) |
            ((x << 8) & 0xff00);
}

inline void SSWAP(unsigned short& s)    {s = swap(s);}
inline void LSWAP(unsigned int& l)      {l = swap(l);}
inline void FSWAP(float& f)             {f = swap(f);}


float
ffl(unsigned int x)
{
 // seee eeee efff ffff ffff ffff ffff ffff
    float   fraction    =   1.0f;
    int     sign        =  (x & 0x80000000) ? -1 : 1;
    int     mantissa    =  (x & 0x007fffff);    
    int     exp         = ((x & 0x7f800000) >> 23) - 0x7f;

 // clamp to finite numbers
    if (exp > 0x7f)         exp = 0x0000007f;
    if (exp < 0xffffff82)   exp = 0xffffff82;

    for (int i = 0; i < 24; i++)
        if ((mantissa << i) & 0x400000) fraction += 1.0f / (float) pow(2, i+1);

    return (float) (sign * fraction * pow(2,exp));
}

CineonFile::CineonFile()
{
    mDensityLUT = NULL;
    mLogLUT     = NULL;
    mStream     = NULL;
    mRefWhite   = REFERENCE_WHITE_CODE;
    mRefBlack   = REFERENCE_BLACK_CODE;
    InitHeader();
    InitData();
}

CineonFile::CineonFile(FILE* stream)
{
    mDensityLUT = NULL;
    mLogLUT     = NULL;
    mStream     = stream;
    mRefWhite   = REFERENCE_WHITE_CODE;
    mRefBlack   = REFERENCE_BLACK_CODE;
    if (stream)
        ReadHeader(mStream);
    InitData();
}

CineonFile::CineonFile(CineonHeader hdr, FILE* stream)
{
    mDensityLUT = NULL;
    mLogLUT     = NULL;
    mStream     = stream;
    mHeader     = hdr;
    mSwappedHeader  = FALSE;
    mRefWhite   = REFERENCE_WHITE_CODE;
    mRefBlack   = REFERENCE_BLACK_CODE;
    if (BigEndian())
        SwapHeader();
    InitData();
}

//FIXME
CineonFile::~CineonFile()
{
    if (mDensityLUT) free(mDensityLUT);
    if (mLogLUT) free(mLogLUT);
 /* we don't open this we just use it
    if (mStream) {
        fclose(mStream);
        mStream = NULL;
    }
  */
}

//NOT CURRENTLY USED
/*
 90% w 685
 18% g 470
 2%  b 180
 */
BOOL
CineonFile::ColorCorrect(U16& pixel , int n) {
    double   density;
    double  whiteOffset;
    double  logExposure;
    double  linearExposure;
    //R32     mRefWhite   = 685.0f; //REFERENCE_WHITE_CODE
    R32     desityStep  = CALIBRATED_DENSITY / (float) pow(2, GetPixelSize());
  
    density     = pixel * desityStep;
    logExposure = density / GetImageGamma();
    whiteOffset = mRefWhite * (desityStep / GetImageGamma());
    logExposure -= whiteOffset;
    linearExposure  = (double) SIXTEEN_BIT_MAX * pow(10, logExposure);
    if (linearExposure > SIXTEEN_BIT_MAX)
        linearExposure = SIXTEEN_BIT_MAX;
    pixel = (U16) linearExposure;
    return TRUE;
}

U8
CineonFile::GetBitsPerPixel(int ch)
{
    if (0 <= ch && ch < 8)
        return mHeader.imageInfo.channels[ch].bitsPerPixel;
    else
        return 0;
}

R32
CineonFile::GetBluePt(int ch)
{
    if (ch == 0 || ch == 1) {
        R32 value   = mHeader.imageInfo.bluePt[ch];
        if (NeedsToBeSwapped()) FSWAP(value);
        return _finite(value) ? value : -1.0f;
    } else
        return  -1.0f;
}

U32
CineonFile::GetCellsPerLine(int ch)
{
    U8  fieldsPerCell   = GetFieldsPerCell(ch);
    U32 numPixels       = mHeader.imageInfo.channels[ch].pixelsPerLine;
    if (NeedsToBeSwapped()) LSWAP(numPixels);
    U32 numChs          = mHeader.imageInfo.numChannels;
    if (NeedsToBeSwapped()) LSWAP(numChs);
    U8  overflow        = ((numChs * numPixels) % fieldsPerCell) ? 1 : 0;
    
    return  overflow + ((numChs * numPixels) / fieldsPerCell);
}

char
CineonFile::GetCellBoundary()
{
    return (mHeader.imageDataFormat.packing & 0x07);
}

U16
CineonFile::GetCellSize()
{
    char    cellBoundary = GetCellBoundary();
    U16     cellSize;

    if (cellBoundary == 1 || cellBoundary == 2) cellSize = CINEON_1_BYTE_CELLS;
    if (cellBoundary == 3 || cellBoundary == 4) cellSize = CINEON_2_BYTE_CELLS;
    if (cellBoundary == 5 || cellBoundary == 6) cellSize = CINEON_4_BYTE_CELLS;

    if (NeedsToBeSwapped()) SSWAP(cellSize);
    return cellSize;
}

U8
CineonFile::GetDataInterleave()
{
    return mHeader.imageDataFormat.interleave;
}

U16
CineonFile::GetDensity(U16 index)
{
    if (mDensityLUT)
        return mDensityLUT[index];
    else
        return ~0;
}

U16
CineonFile::GetLog(U16 index)
{
    if (mLogLUT)
        return mLogLUT[index];
    else
        return ~0;
}

R32
CineonFile::GetDeviceXPitch()
{
    R32 value   = mHeader.imageOrigin.xPitch;
    if (NeedsToBeSwapped()) FSWAP(value);
    return _finite(value) ? value : -1.0f;
}

R32
CineonFile::GetDeviceYPitch()
{
    R32 value   = mHeader.imageOrigin.yPitch;
    if (NeedsToBeSwapped()) FSWAP(value);
    return _finite(value) ? value : -1.0f;
}

U32
CineonFile::GetEOCPadding()
{
    U32 value   = mHeader.imageDataFormat.eocPadding;
    if (NeedsToBeSwapped()) LSWAP(value);
    return value;
}

U32
CineonFile::GetEOLPadding()
{
    U32 value   = mHeader.imageDataFormat.eolPadding;
    if (NeedsToBeSwapped()) LSWAP(value);
    return value;
}

U8
CineonFile::GetFieldsPerCell(int ch)
{

    U8  bPP = mHeader.imageInfo.channels[ch].bitsPerPixel;
    U8  fPC = 0;

    while ((fPC+1) * bPP <= GetCellSize() * 8) {
        fPC++;
    }

    return  fPC;
}

U8
CineonFile::GetFilmOffset()
{
    return mHeader.filmInfo.offset;
}

U8
CineonFile::GetFilmMFGID()
{
    return mHeader.filmInfo.mfgID;
}

U32
CineonFile::GetFileSize()
{
    U32 value   = mHeader.fileInfo.imageFileSize;
    if (NeedsToBeSwapped()) LSWAP(value);
    return value;
}

U8
CineonFile::GetFilmType()
{
    return mHeader.filmInfo.type;
}

U32
CineonFile::GetFramePosition()
{
    U32 value   = mHeader.filmInfo.frame;
    if (NeedsToBeSwapped()) LSWAP(value);
    return value;
}

R32
CineonFile::GetFrameRate()
{
    R32 value   = mHeader.filmInfo.rate;
    if (NeedsToBeSwapped()) FSWAP(value);
    return _finite(value) ? value : -1.0f;
}

R32
CineonFile::GetGreenPt(int n)
{
    if (n == 0 || n == 1) {
        R32 value   = mHeader.imageInfo.greenPt[n];
        if (NeedsToBeSwapped()) FSWAP(value);
        return _finite(value) ? value : -1.0f;
    } else
        return  -1.0f;
}

BOOL
CineonFile::GetHeader(CineonHeader* hdr)
{
    if (VerifyHeader()) {
        *hdr = mHeader;
        return TRUE;
    } else return FALSE;
}

char*
CineonFile::GetImageCreationDate()
{
 // make sure the string is NULL terminated
    mHeader.fileInfo.date[CINEON_HDR_DATE_LENGTH-1] = NULL;
    return mHeader.fileInfo.date;
}

char*
CineonFile::GetImageCreationTime()
{
 // make sure the string is NULL terminated
    mHeader.fileInfo.time[CINEON_HDR_TIME_LENGTH-1] = NULL;
    return mHeader.fileInfo.time;
}

char*
CineonFile::GetImageFileName()
{
 // make sure the string is terminated
    mHeader.fileInfo.filename[CINEON_HDR_FILENAME_LENGTH-1] = NULL;
    return mHeader.fileInfo.filename;
}

U32
CineonFile::GetImageFileSize()
{
    U32 value   = mHeader.fileInfo.imageFileSize;
    if (NeedsToBeSwapped()) LSWAP(value);
    return value;
}

char*
CineonFile::GetImageFormat()
{
 // make sure the string is NULL terminated
    mHeader.filmInfo.format[CINEON_HDR_FILM_FORMAT_LENGTH-1] = NULL;
    return mHeader.filmInfo.format;
}

char*
CineonFile::GetImageFrameAttribute()
{
 // make sure the string is NULL terminated
    mHeader.filmInfo.attribute[CINEON_HDR_FRAME_ATTRIBUTE_LENGTH-1] = NULL;
    return mHeader.filmInfo.attribute;
}

//FIXME images that don't set the gamma
R32
CineonFile::GetImageGamma()
{
    R32 value   = mHeader.imageOrigin.gamma;
    if (NeedsToBeSwapped()) FSWAP(value);
 // do something with file that don't set gamma
    return (0.005 > value || value > 5) ? NEGATIVE_FILM_GAMMA : value;
}

char*
CineonFile::GetImageInputDevice()
{
 // make sure the string is NULL terminated
    mHeader.imageOrigin.device[CINEON_HDR_DEVICE_NAME_LENGTH-1] = NULL;
    return mHeader.imageOrigin.device;
}

char*
CineonFile::GetImageInputDeviceModelNumber()
{
 // make sure the string is NULL terminated
    mHeader.imageOrigin.modelNum[CINEON_HDR_DEVICE_MODEL_NUMBER_LENGTH-1] = NULL;
    return mHeader.imageOrigin.modelNum;
}

char*
CineonFile::GetImageInputDeviceSerialNumber()
{
 // make sure the string is NULL terminated
    mHeader.imageOrigin.serialNum[CINEON_HDR_DEVICE_SERIAL_NUMBER_LENGTH-1] = NULL;
    return mHeader.imageOrigin.serialNum;
}

U32
CineonFile::GetImageOffset()
{
    U32 imageOffset = mHeader.fileInfo.imageOffset;
    if (NeedsToBeSwapped()) LSWAP(imageOffset);
    return  imageOffset;
}

U8
CineonFile::GetImageOrientation()
{

    return mHeader.imageInfo.orientation;
}

S32
CineonFile::GetImageXOffset()
{
    S32 value   = mHeader.imageOrigin.xOffset;
    if (NeedsToBeSwapped()) LSWAP((unsigned int&)value);
    return value;
}

S32
CineonFile::GetImageYOffset()
{
    S32 value   = mHeader.imageOrigin.yOffset;
    if (NeedsToBeSwapped()) LSWAP((unsigned int&)value);
    return value;
}

U32
CineonFile::GetLinesPerImage(int ch)
{
    U32 numLines = mHeader.imageInfo.channels[ch].linesPerImage;
    if (NeedsToBeSwapped()) LSWAP(numLines);
    return numLines;
}

// this reads a Cineon line (pixel interleave) and converts it to a MAX line
//FIXME return a pointer to the allocated line.
BOOL
CineonFile::GetScanLine(U16* line, U32 lineNum, U32 lineLen)
{
    U8*     CINLine     = NULL;
    U16     pixel[3];
    U32     pixelSize   = GetPixelSize();
    U32     pixelCnt    = 0;
    U8      chCnt       = 0;
    U8*     cell        = NULL;
    U32     cellCnt     = 0;
    U32     cellsPL     = GetCellsPerLine();
    U32     cellSize    = GetCellSize();
    U8      fieldsPC    = GetFieldsPerCell();
    U32     linesPI     = GetLinesPerImage();
    U32     imageOffset = GetImageOffset();
    U32     lineOffset  = imageOffset + (lineNum * cellsPL * cellSize);

    if (!line)          return FALSE;
    if (!mStream)       return FALSE;

    if (cellsPL  == ~0) return FALSE;
    if (0 > lineNum || lineNum > linesPI)   return FALSE;
    if (lineLen < GetPixelsPerLine())       return FALSE;

    if (!(CINLine = (U8*) calloc(cellsPL, cellSize))) {
        return FALSE;
    }

    if (fseek(mStream, lineOffset, SEEK_SET)) {
        if (CINLine) { free(CINLine); CINLine = NULL; }
        return FALSE;
    }

 // big time hit
    if (!fread(CINLine, sizeof(cellSize), cellsPL, mStream)) {
        if (CINLine) { free(CINLine); CINLine = NULL; }
        return FALSE;
    }

    if (BigEndian()) {
        cell = CINLine;
        for (U32 i = 0; i < cellsPL; i++) {
            if (cellSize == CINEON_4_BYTE_CELLS) {
                LSWAP((U32&) *cell);
            }
            if (cellSize == CINEON_2_BYTE_CELLS) {
                SSWAP((U16&) *cell);
            }
            cell += cellSize;
        }
     }

#ifdef GET_TEST_PATTERN
 // test colors grad
    float r1 = ((float) lineNum) / ((float) linesPI);
    for (unsigned int i = 0; i < cellsPL; i++) {
        float r2 = ((float) i) / ((float) cellsPL);
        pixel[0] = (int) (65535.f *  r1);
        pixel[1] = (int) (65535.f *  r2);
        pixel[2] = (int) (65535.f * (r1 * r2));
        memcpy(line+(pixelCnt * 4), pixel, sizeof(pixel));
        pixelCnt++;
    }
#else
 // convert a line of cells to a line of pixels
 // set the mask
    U32 mask;
    U8  align;
    if (LeftJustified()) {
        mask    = ~(~((U32)0) >> (pixelSize + 32 - (cellSize * 8)));
        align   = 0;
    } else {
        mask    = ~(~((U32)0) << pixelSize);
        align   = cellSize - (fieldsPC * pixelSize);
    }
    cell = CINLine;
    for (cellCnt = 0; cellCnt < cellsPL; cellCnt++) {
     // break cell into fields and write the fields
        for (int i = 0; i < fieldsPC; i++) {
            pixel[chCnt] = (mask & (*(U32*)cell  << ((pixelSize * chCnt) + align))) >> ((cellSize * 8) - pixelSize);
         // big time hit so store in LUT
         // ColorCorrect(pixel[chCnt], chCnt);
         // line[(pixelCnt * 4)+chCnt] = pixel[chCnt];
            line[(pixelCnt * 4)+chCnt] = GetDensity(pixel[chCnt]);
            if (++chCnt == 3) {   //we are only writing MAX's r,g,b
                chCnt = 0;
                pixelCnt++;
            }
        }
        cell += cellSize;
    }
#endif

    if (CINLine) { free(CINLine); CINLine = NULL; }

    return TRUE;
}

U8
CineonFile::GetNumberChannels()
{
    return mHeader.imageInfo.numChannels;
}

U32
CineonFile::GetPixelsPerLine(int ch)
{
    U32 numPixels = mHeader.imageInfo.channels[ch].pixelsPerLine;
    if (NeedsToBeSwapped()) LSWAP(numPixels);
    return numPixels;
}

U8
CineonFile::GetPixelSize(int ch)
{
    return mHeader.imageInfo.channels[ch].bitsPerPixel;
}

R32
CineonFile::GetRedPt(int n)
{
    
    if (n == 0 || n == 1) {
        R32 value   = mHeader.imageInfo.redPt[n];
        if (NeedsToBeSwapped()) FSWAP(value);
        return _finite(value) ? value : -1.0f;
    } else
        return  -1.0f;
}

U8
CineonFile::GetPacking()
{
    return mHeader.imageDataFormat.packing;
}

U8
CineonFile::GetSense()
{
    return mHeader.imageDataFormat.sense;
}

U8
CineonFile::GetSigned()
{
    return mHeader.imageDataFormat.signedType;
}

char*
CineonFile::GetVersionNumber()
{
 // make sure the string is NULL terminated
    mHeader.fileInfo.hdrVersion[CINEON_HDR_VERSION_LENGTH-1] = NULL;
    return mHeader.fileInfo.hdrVersion;
}

R32
CineonFile::GetWhitePt(int n)
{
    if (n == 0 || n == 1) {
        R32 value   = mHeader.imageInfo.whitePt[n];
        if (NeedsToBeSwapped()) FSWAP(value);
        return _finite(value) ? value : -1.0f;
    } else
        return  -1.0f;
}

//FIXME
void
CineonFile::InitHeader() {
    mSwappedHeader  = FALSE;
    //memset(&mHeader, 0xFF, sizeof(CineonHeader));
    mHeader.imageDataFormat.packing = 0x5;
    mHeader.imageInfo.channels[0].designator[0] = 0;
    mHeader.imageInfo.channels[0].designator[1] = 1;
    mHeader.imageInfo.channels[1].designator[0] = 0;
    mHeader.imageInfo.channels[1].designator[1] = 2;
    mHeader.imageInfo.channels[2].designator[0] = 0;
    mHeader.imageInfo.channels[2].designator[1] = 3;
}

BOOL
CineonFile::InitData()
{
    U8  pixelSize;
    if (VerifyHeader()) {
        pixelSize = GetPixelSize();
    } else {
     // FIXME set defaults
        pixelSize   = 10;
        mDensityLUT = NULL;
        mLogLUT     = NULL;
    }
    if (SetDensities(pixelSize) == FALSE) return FALSE;
    if (SetLog(pixelSize) == FALSE) return FALSE;
    return TRUE;
}

//FIXME add wider support
BOOL
CineonFile::IsSupported()
{
    if (mHeader.imageInfo.orientation != 0)
        return FALSE;
    if (mHeader.imageInfo.numChannels != 3)
        return FALSE;
    if (mHeader.imageInfo.channels[0].bitsPerPixel != 10)
        return FALSE;
    if (mHeader.imageDataFormat.interleave != 0)
        return FALSE;
    if (mHeader.imageDataFormat.packing != 5)
        return FALSE;
    if (mHeader.imageDataFormat.signedType != 0)
        return FALSE;
    if (mHeader.imageDataFormat.sense != 0)
        return FALSE;

    return  TRUE;
}

BOOL
CineonFile::LeftJustified()
{
    char cellBoundary = GetCellBoundary();
    return ((cellBoundary == 1 ||
             cellBoundary == 3 ||
             cellBoundary == 5) ? TRUE : FALSE);
}

BOOL
CineonFile::PackFields()
{
    return  mHeader.imageDataFormat.packing & 0x80;
}

BOOL
CineonFile::ReadHeader(FILE* stream)
{
    if (stream) {
        if (fread(&mHeader, sizeof(CineonHeader), 1, stream)) {
            mSwappedHeader = FALSE;
            if (BigEndian())
                return SwapHeader();
            else
                return TRUE;
        } else return FALSE;
    } else return FALSE;
}

R32
CineonFile::SetDeviceXPitch(R32 value)
{
    mHeader.imageOrigin.xPitch = value;
    return mHeader.imageOrigin.xPitch;
}

R32
CineonFile::SetDeviceYPitch(R32 value)
{
    mHeader.imageOrigin.yPitch = value;
    return mHeader.imageOrigin.yPitch;
}

BOOL
CineonFile::SetImageFileName(ASCII* filename)
{
    strncpy(mHeader.fileInfo.filename, filename, CINEON_HDR_FILENAME_LENGTH);
    mHeader.fileInfo.filename[CINEON_HDR_FILENAME_LENGTH - 1] = NULL;
    return TRUE;
}

BOOL
CineonFile::SetImageCreationDate(ASCII* date)
{
    strncpy(mHeader.fileInfo.date, date, CINEON_HDR_DATE_LENGTH);
    mHeader.fileInfo.date[CINEON_HDR_DATE_LENGTH - 1] = NULL;
    return TRUE;
}

BOOL
CineonFile::SetImageCreationTime(ASCII* time)
{
    strncpy(mHeader.fileInfo.time, time, CINEON_HDR_TIME_LENGTH);
    mHeader.fileInfo.time[CINEON_HDR_TIME_LENGTH - 1] = NULL;
    return TRUE;
}

BOOL
CineonFile::SetImageInputDevice(ASCII* device)
{
    strncpy(mHeader.imageOrigin.device, device, CINEON_HDR_DEVICE_NAME_LENGTH);
    mHeader.imageOrigin.device[CINEON_HDR_DEVICE_NAME_LENGTH - 1] = NULL;
    return TRUE;
}

BOOL
CineonFile::SetImageInputDeviceModelNumber(ASCII* model)
{
    strncpy(mHeader.imageOrigin.modelNum, model, CINEON_HDR_DEVICE_MODEL_NUMBER_LENGTH);
    mHeader.imageOrigin.modelNum[CINEON_HDR_DEVICE_MODEL_NUMBER_LENGTH - 1] = NULL;
    return TRUE;
}

BOOL
CineonFile::SetImageInputDeviceSerialNumber(ASCII* serial)
{
    strncpy(mHeader.imageOrigin.serialNum, serial, CINEON_HDR_DEVICE_SERIAL_NUMBER_LENGTH);
    mHeader.imageOrigin.serialNum[CINEON_HDR_DEVICE_SERIAL_NUMBER_LENGTH - 1] = NULL;
    return TRUE;
}

BOOL
CineonFile::SetImageOffset(U32 offset)
{
    mHeader.fileInfo.imageOffset = offset;
    return TRUE;
}

BOOL
CineonFile::SetImageOrientation(U8 orientation)
{
    mHeader.imageInfo.orientation = orientation;
    return TRUE;
}

BOOL
CineonFile::SetFileSize(U32 size)
{
    mHeader.fileInfo.imageFileSize = size;
    return TRUE;
}

BOOL
CineonFile::SetNumberChannels(U8 numChs)
{
    mHeader.imageInfo.numChannels = numChs;
    return TRUE;
}

BOOL
CineonFile::SetBitsPerPixel(U8 ch, U8 num)
{
    mHeader.imageInfo.channels[ch].bitsPerPixel = num;
    return TRUE;
}

BOOL
CineonFile::SetPixelsPerLine(U8 ch, U32 num)
{
    mHeader.imageInfo.channels[ch].pixelsPerLine = num;
    return TRUE;
}

BOOL
CineonFile::SetLinesPerImage(U8 ch, U32 num)
{
    mHeader.imageInfo.channels[ch].linesPerImage = num;
    return TRUE;
}

BOOL
CineonFile::SetWhitePt(R32 x, R32 y)
{
    mHeader.imageInfo.whitePt[0] = x;
    mHeader.imageInfo.whitePt[1] = y;
    return TRUE;
}

BOOL
CineonFile::SetRedPt(R32 x, R32 y)
{
    mHeader.imageInfo.redPt[0] = x;
    mHeader.imageInfo.redPt[1] = y;
    return TRUE;
}

BOOL
CineonFile::SetGreenPt(R32 x, R32 y)
{
    mHeader.imageInfo.greenPt[0] = x;
    mHeader.imageInfo.greenPt[1] = y;
    return TRUE;
}

BOOL
CineonFile::SetBluePt(R32 x, R32 y)
{
    mHeader.imageInfo.bluePt[0] = x;
    mHeader.imageInfo.bluePt[1] = y;
    return TRUE;
}

BOOL
CineonFile::SetDataInterleave(U8 bits)
{
    mHeader.imageDataFormat.interleave = bits;
    return TRUE;
}

BOOL
CineonFile::SetPacking(U8 bits)
{
    mHeader.imageDataFormat.packing = bits;
    return TRUE;
}

BOOL
CineonFile::SetSigned(U8 bits)
{
    mHeader.imageDataFormat.signedType = bits;
    return TRUE;
}

BOOL
CineonFile::SetSense(U8 bits)
{
    mHeader.imageDataFormat.sense = bits;
    return TRUE;
}

BOOL
CineonFile::SetEOLPadding(U32 eolPadding)
{
    mHeader.imageDataFormat.eolPadding = eolPadding;
    return TRUE;
}

BOOL
CineonFile::SetEOCPadding(U32 eocPadding)
{
    mHeader.imageDataFormat.eocPadding = eocPadding;
    return TRUE;
}

BOOL
CineonFile::SetImageGamma(R32 gamma)
{
    mHeader.imageOrigin.gamma = gamma;
    return TRUE;
}

BOOL
CineonFile::SetImageXOffset(S32 offset)
{
    mHeader.imageOrigin.xOffset = offset;
    return TRUE;
}

BOOL
CineonFile::SetImageYOffset(S32 offset)
{
    mHeader.imageOrigin.yOffset = offset;
    return TRUE;
}

BOOL
CineonFile::SetFramePosition(U32 frameNum)
{
    mHeader.filmInfo.frame = frameNum;
    return TRUE;
}

BOOL
CineonFile::SetLUTs(int bPP, R32 refW, R32 refB)
{
    mRefWhite   = refW;
    mRefBlack   = refB;
    SetDensities(bPP);
    SetLog(bPP);
    return TRUE;
}

//FIXME add user configurable ref values.
//90% w 685
//18% g 470
//2%  b 180
BOOL
CineonFile::SetDensities(U8 bitsPerPixel)
{
    R32     mSoftclip   = 0.0f;
    U32     LUTsize     = (U32) pow(2, bitsPerPixel);
    R32     densityStep  = CALIBRATED_DENSITY / (float) LUTsize;

    if (mDensityLUT) free(mDensityLUT);

    
    if (!(mDensityLUT = (U16*) calloc(LUTsize, sizeof(U16)))) {
        return FALSE;
    }

    // Check which version of the file we are reading
    bool bOldMaxFormat=(strcmp(GetImageInputDevice(),"3D Studio MAX")==0) && (strcmp(GetImageInputDeviceModelNumber(),"2.0")==0)?true:false;

    if(bOldMaxFormat)
    {
        // Max used to save files using a weird algorithm. This file format is not used anymore.
        // We are keeping the algorithm to provide backward compatibility for older files.
        double   density;
        double  whiteOffset;
        double  logExposure;
        double  linearExposure;

        for (U32 cnt = 0; cnt < LUTsize; cnt++) 
        {
            if (cnt < mRefBlack) {
                mDensityLUT[cnt] = 0;
                continue;
            }
            density     = cnt * densityStep;
            logExposure = density / GetImageGamma();
            whiteOffset = mRefWhite * (densityStep / GetImageGamma());
            logExposure -= whiteOffset;
            linearExposure  = (double) SIXTEEN_BIT_MAX * pow(10, logExposure);
            linearExposure *= GetImageGamma();
            if (linearExposure > SIXTEEN_BIT_MAX)
                linearExposure = SIXTEEN_BIT_MAX;
            mDensityLUT[cnt] = (U16) linearExposure;
        }
    }
    else
    {
        // Here's a description of the algorithm being used (straight from the documentation):
        // 
        // OUT  = 10^((IN-RefWhite)* 0.002/0.6 ^ Dispgamma/1.7 * Gain - Offset
        // 
        // Gain = 255 / (1 - 10^((Refblack-RefWhite)* 0.002/0.6)^(Dispgamma/1.7))
        // Offset = Gain - 255
        //
        // 255 = 8 bit conversion. In our case, we use 16 bit so it's going to be replaced by 65535
        // 0,002 = density step. This is base on the file format for cineon : 8bit,10bit...
        //         Our algorithm recomputes it on the fly based on the input data
        //
        // Dispgamma : On Unix systems, they assume 1.5 so they ignore the second they ignore ^(Dispgamma/1.7)
        //             I can't really say whether we should ignore it or not. I believe "combustion" ignores it so
        //             I'll ignore it too.

        double Gain = SIXTEEN_BIT_MAX / (1- pow(10, (mRefBlack - mRefWhite) * densityStep/0.6f ));
        double Offset = Gain - SIXTEEN_BIT_MAX;;
        double dDensityLUT;

        for (U32 cnt = 0; cnt < LUTsize; cnt++) 
        {
            if (cnt < mRefBlack) 
            {
                mDensityLUT[cnt] = 0;
                continue;
            }

            dDensityLUT = pow(10,(cnt-mRefWhite)*densityStep/0.6f)*Gain - Offset;
                
            if (dDensityLUT > SIXTEEN_BIT_MAX)
                mDensityLUT[cnt] = SIXTEEN_BIT_MAX;
            else
                mDensityLUT[cnt] = (U16) (dDensityLUT);
        }
    }
    return TRUE;
}

BOOL
CineonFile::SetLog(U8 bitsPerPixel)
{
    //R32     mRefWhite   = 685.0f; //REFERENCE_WHITE_CODE 
    U32     LUTsize;
    double  logDensity;
    double  densityMAX  = (float) log10(SIXTEEN_BIT_MAX);
    R32     densityStep  = CALIBRATED_DENSITY / (float) pow(2, bitsPerPixel);

    if (mLogLUT) free(mLogLUT);

    LUTsize     = (U32) pow(2, 16);
    if (!(mLogLUT = (U16*) calloc(LUTsize, sizeof(U16)))) {
        return FALSE;
    }

    mLogLUT[0] = 0;


    double Gain = SIXTEEN_BIT_MAX / (1- pow(10, (mRefBlack - mRefWhite) * densityStep/0.6f ));
    double Offset = Gain - SIXTEEN_BIT_MAX;;

    for (U32 cnt = 1; cnt < LUTsize; cnt++) 
    {
        //////////////////////////////////////////////////////////////////////////////////////////////
        // The algorithm we use here is the opposite of what is desribed in the SetDensities method
        // The docs describe the logic as follow
        //
        // OUT = 685 + log10{((IN + Offset)/Gain)^(1.7/Dispgamma)}/(0.002/0.6)
        //
        // Gain = 255 / (1 - 10^((Refblack-RefWhite)* 0.002/0.6)^(Dispgamma/1.7))
        // Offset = Gain - 255
        //
        // 255 = 8 bit conversion. In our case, we use 16 bit so it's going to be replaced by 65535
        // 0,002 = density step. This is base on the file format for cineon : 8bit,10bit...
        //         Our algorithm recomputes it on the fly based on the input data
        //
        // Dispgamma : On Unix systems, they assume 1.5 so they ignore the second they ignore ^(Dispgamma/1.7)
        //             I can't really say whether we should ignore it or not. I believe "combustion" ignores it so
        //             I'll ignore it too.
        // 685 = White Reference
        //
        

        logDensity   = mRefWhite + log10((cnt+Offset)/Gain)/(densityStep/0.6);

        if (logDensity < 0)
            mLogLUT[cnt] = 0;
        else
            mLogLUT[cnt] = (U16) logDensity;
    }

    return TRUE;
}

BOOL
CineonFile::BigEndian()
{
    if (mSwappedHeader)
        return (mHeader.fileInfo.magicNum == LITTLE_ENDIAN_MAGIC);
    else
        return (mHeader.fileInfo.magicNum == BIG_ENDIAN_MAGIC);
}

BOOL
CineonFile::NeedsToBeSwapped()
{
    return ((mHeader.fileInfo.magicNum == BIG_ENDIAN_MAGIC) && !mSwappedHeader);
}

BOOL
CineonFile::SwapHeader()
{
    if (!VerifyHeader()) return FALSE;
    mSwappedHeader  = TRUE;
        LSWAP(mHeader.fileInfo.magicNum);
        LSWAP(mHeader.fileInfo.imageOffset);
        LSWAP(mHeader.fileInfo.genericHdrLen);
        LSWAP(mHeader.fileInfo.industryHdrLen);
        LSWAP(mHeader.fileInfo.userHdrLen);
        LSWAP(mHeader.fileInfo.imageFileSize);

        int numChs = mHeader.imageInfo.numChannels;
        for (int i = 0; i < numChs; i++) {
            LSWAP(mHeader.imageInfo.channels[i].pixelsPerLine);
            LSWAP(mHeader.imageInfo.channels[i].linesPerImage);
            FSWAP(mHeader.imageInfo.channels[i].minValue);
            FSWAP(mHeader.imageInfo.channels[i].minQuantity);
            FSWAP(mHeader.imageInfo.channels[i].maxValue);
            FSWAP(mHeader.imageInfo.channels[i].maxQuantity);
        }
        FSWAP(mHeader.imageInfo.whitePt[0]);
        FSWAP(mHeader.imageInfo.whitePt[1]);
        FSWAP(mHeader.imageInfo.redPt[0]);
        FSWAP(mHeader.imageInfo.redPt[1]);
        FSWAP(mHeader.imageInfo.greenPt[0]);
        FSWAP(mHeader.imageInfo.greenPt[1]);
        FSWAP(mHeader.imageInfo.bluePt[0]);
        FSWAP(mHeader.imageInfo.bluePt[1]);

        LSWAP(mHeader.imageDataFormat.eolPadding);
        LSWAP(mHeader.imageDataFormat.eocPadding);

        LSWAP((unsigned int&) mHeader.imageOrigin.xOffset);
        LSWAP((unsigned int&) mHeader.imageOrigin.yOffset);
        FSWAP(mHeader.imageOrigin.xPitch);
        FSWAP(mHeader.imageOrigin.yPitch);
        FSWAP(mHeader.imageOrigin.gamma);

        LSWAP(mHeader.filmInfo.prefix);
        LSWAP(mHeader.filmInfo.count);
        LSWAP(mHeader.filmInfo.frame);
        FSWAP(mHeader.filmInfo.rate);

    return TRUE;
}

//FIXME add more checks
BOOL
CineonFile::VerifyHeader() {
    if (mHeader.fileInfo.magicNum != BIG_ENDIAN_MAGIC && 
        mHeader.fileInfo.magicNum != LITTLE_ENDIAN_MAGIC)
        return FALSE;

    if (strcmp("V4.5", GetVersionNumber()))
        return FALSE;

    if (GetNumberChannels() > 8)
        return FALSE;

    if (GetBitsPerPixel() > 16)
        return FALSE;

    /* QE would like to render NxN (not a good idea but...)
    if (GetPixelsPerLine() > 4096)
        return FALSE;

    if (GetLinesPerImage() > 6144)
        return FALSE;
    */

    return TRUE;
}

//FIXME write variable/user section
BOOL
CineonFile::WriteHeader(FILE* stream) {
    if (!BigEndian()) {
        SwapHeader();
        mSwappedHeader = FALSE;
    }
    if (!VerifyHeader()) return FALSE;
    if (stream) {
        if (fseek(stream, 0, SEEK_SET))
            return FALSE;
        if (fwrite(&mHeader, sizeof(CineonHeader), 1, stream) != 1)
            return FALSE;
        return TRUE;
    } else return FALSE;
}

// writes a Cineon line (pixel interleave)
BOOL
CineonFile::WriteScanLine(FILE* stream, U16* line, U32 lineNum, U32 lineLen, BOOL bigEndian)
{
    U8*     CINLine     = NULL;
    U16     pixel[3];
    U32     pixelSize   = GetPixelSize();
    U8*     cell        = NULL;
    U32     oneCell;
    U32     cellsPL     = GetCellsPerLine();
    U32     cellSize    = GetCellSize();
    U32     linesPI     = GetLinesPerImage();
    U32     imageOffset = GetImageOffset();
    U32     lineOffset  = imageOffset + (lineNum * cellsPL * cellSize);

    if (!line)          return FALSE;
    if (!stream)        return FALSE;

    if (cellsPL  == ~0) return FALSE;
    if (0 > lineNum || lineNum > linesPI)   return FALSE;
    if (lineLen < GetPixelsPerLine())       return FALSE;

    if (!(CINLine = (U8*) calloc(cellsPL, cellSize))) {
        return FALSE;
    }

    if (fseek(stream, lineOffset, SEEK_SET)) {
        if (CINLine) { free(CINLine); CINLine = NULL; }
        return FALSE;
    }

    U32 mask    = ~(~((U32)0) << pixelSize);

#ifdef SET_TEST_PATTERN
 // test colors grad
    float r1    = ((float) lineNum) / ((float) linesPI);
    float max   = (float) pow(2, 16);
    
    for (unsigned int pixelCnt = 0; pixelCnt < cellsPL; pixelCnt++) {
        oneCell = 0;
        float r2    = ((float) pixelCnt) / ((float) cellsPL);
        pixel[0]    = mLogLUT[(U16) (max *  r1)];
        pixel[1]    = mLogLUT[(U16) (U16) (max *  r2)];
        pixel[2]    = mLogLUT[(U16) (U16) (max * (r1 * r2))];
        oneCell |= (pixel[0] & mask) << 22;
        oneCell |= (pixel[1] & mask) << 12;
        oneCell |= (pixel[2] & mask) << 2;
        memcpy(CINLine + (pixelCnt * 4), &oneCell, sizeof(oneCell));
    }
#else
    U16*  pixelPtr = NULL;
    for (U32 pixelCnt = 0; pixelCnt < lineLen; pixelCnt++) {
        oneCell = 0;
        pixelPtr = line+(pixelCnt*4);

        pixel[0]    = (U16) (1.6f * (float) pixelPtr[0]);
        pixel[1]    = (U16) (1.6f * (float) pixelPtr[1]);
        pixel[2]    = (U16) (1.6f * (float) pixelPtr[2]);
        pixel[0]    = GetLog(pixelPtr[0]);
        pixel[1]    = GetLog(pixelPtr[1]);;
        pixel[2]    = GetLog(pixelPtr[2]);;
        oneCell    |= (pixel[0] & mask) << 22;
        oneCell    |= (pixel[1] & mask) << 12;
        oneCell    |= (pixel[2] & mask) << 2;

        
        memcpy(CINLine + (pixelCnt * 4), &oneCell, sizeof(oneCell));
    }
#endif

    if (bigEndian) {
        cell = CINLine;
        for (U32 i = 0; i < lineLen; i++) {
            if (cellSize == CINEON_4_BYTE_CELLS) {
                LSWAP((U32&) *cell);
            }
            if (cellSize == CINEON_2_BYTE_CELLS) {
                SSWAP((U16&) *cell);
            }
            cell += cellSize;
        }
    }

    if (fwrite(CINLine, cellsPL * cellSize, 1, stream) != 1) {
        if (CINLine) { free(CINLine); CINLine = NULL; }
        return FALSE;
    }

    if (CINLine) { free(CINLine); CINLine = NULL; }

    return TRUE;
}
