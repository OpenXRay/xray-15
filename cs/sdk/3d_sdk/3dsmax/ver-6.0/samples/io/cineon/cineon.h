/**********************************************************************
 *
 * FILE:        cineon.h
 * AUTHOR:      greg finch
 *
 * DESCRIPTION: SMPTE Digital Picture Exchange Format,
 *              SMPTE Cineon, Kodak Cineon
 *
 * CREATED:     20 june, 1997
 *
 *
 * 
 * Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#ifndef _CINEON_H_
#define _CINEON_H_

#define CINEON_HDR_VERSION_LENGTH               8
#define CINEON_HDR_FILENAME_LENGTH              100
#define CINEON_HDR_DATE_LENGTH                  12
#define CINEON_HDR_TIME_LENGTH                  12
#define CINEON_HDR_MAX_NUMBER_CHANNELS          8
#define CINEON_HDR_IMAGE_INFO_LABEL_LENGTH      200
#define CINEON_HDR_DEVICE_NAME_LENGTH           64
#define CINEON_HDR_DEVICE_MODEL_NUMBER_LENGTH   32
#define CINEON_HDR_DEVICE_SERIAL_NUMBER_LENGTH  32
#define CINEON_HDR_FILM_FORMAT_LENGTH           32
#define CINEON_HDR_FRAME_ATTRIBUTE_LENGTH       32
#define CINEON_HDR_FRAME_SLATE_INFO_LENGTH      200
#define CINEON_HDR_FIXED_GEN_SECTION_LENGTH     1024
#define CINEON_HDR_FIXED_MP_SECTION_LENGTH      1024
#define CINEON_HDR_VAR_USER_SECTION_LENGTH      0

#define CINEON_IMAGE_OFFSET     (CINEON_HDR_FIXED_GEN_SECTION_LENGTH + CINEON_HDR_FIXED_MP_SECTION_LENGTH + CINEON_HDR_VAR_USER_SECTION_LENGTH)

#define CINEON_HDR_MAGIC_NUMBER             0x802a5fd7
#define CINEON_HDR_VERSION_NUMBER           "V4.5"

//#define CINEON_USER_DATA_CHUNK_LENGTH      30208

#define CINEON_1_BYTE_CELLS                1
#define CINEON_2_BYTE_CELLS                2
#define CINEON_4_BYTE_CELLS                4

//#define CINEON_VERSION                      300

#define USE_ALGORITHM_1 FALSE   // this is the densities algorithm switch

#define U8      unsigned char
#define U8_UNDEF    0xff
#define U16     unsigned short
#define U16_UNDEF   0xffff
#define U32     unsigned int
#define U32_UNDEF   0xffffffff
#define S32     signed int
#define S32_UNDEF   0x80000000
#define R32     float
#define R32_UNDEF   0x7f800000  // + infinity
#define ASCII   char
#define ASCII_UNDEF '\0'

#define CALIBRATED_DENSITY      2.048f
#define NEGATIVE_FILM_GAMMA     0.6f
#define REFERENCE_WHITE_CODE    685
#define REFERENCE_BLACK_CODE    95
#define SIXTEEN_BIT_MAX         65535
#define TEN_BIT_MAX             1024

#define BIG_ENDIAN_MAGIC        0xd75f2a80
#define LITTLE_ENDIAN_MAGIC     0x802a5fd7


/*
class voidPtr {
    void*   ptr;
public:
    voidPtr();
    voidPtr(size_t, size_t);
   ~voidPtr();
    void*   GetVoidPtr();
    void    SetVoidPtr(void*);
};

voidPtr::voidPtr()
{
    ptr = NULL;
}

voidPtr::voidPtr(size_t num, size_t size)
{
    ptr = calloc(num, size);
}

voidPtr::~voidPtr()
{
    if (ptr) free(ptr);
}

void*
voidPtr::GetVoidPtr()
{
    return ptr;
}
void
voidPtr::SetVoidPtr(void* p)
{
    ptr = p;
}
*/

//#pragma pack(1)

struct CineonFileInfo {
    U32     magicNum;       //Magic number (802A5FD7 hex) - indicates start of image file and byte ordering.
    U32     imageOffset;    //offset to image data in bytes.
    U32     genericHdrLen;  //Generic section header length in bytes (fixed format)
    U32     industryHdrLen; //Industry Specific section header length in bytes (fixed format)
    U32     userHdrLen;     //User section header length in bytes (variable format)
    U32     imageFileSize;  //total image file size in bytes (includees header, image data, and padding, if any
    ASCII   hdrVersion[CINEON_HDR_VERSION_LENGTH];   //version number of header format
    ASCII   filename[CINEON_HDR_FILENAME_LENGTH];        //image filename
    ASCII   date[CINEON_HDR_DATE_LENGTH];                //creation date (eg. yyyy:mm:dd)
    ASCII   time[CINEON_HDR_TIME_LENGTH];                //creation time (eg. hh:mm:ssxxx xxx-timezone, eg.EST)
    ASCII   reserved[36];   //reserved by Kodak for future use
    CineonFileInfo() {
        magicNum        = CINEON_HDR_MAGIC_NUMBER;
        imageOffset     = CINEON_IMAGE_OFFSET;
        genericHdrLen   = CINEON_HDR_FIXED_GEN_SECTION_LENGTH;
        industryHdrLen  = CINEON_HDR_FIXED_MP_SECTION_LENGTH;
        userHdrLen      = CINEON_HDR_VAR_USER_SECTION_LENGTH;

        imageFileSize   = U32_UNDEF;
        //hdrVersion[0]   = ASCII_UNDEF;
        strcpy(hdrVersion, "V4.5");
        filename[0]     = ASCII_UNDEF;
        date[0]         = ASCII_UNDEF;
        time[0]         = ASCII_UNDEF;
        reserved[0]     = ASCII_UNDEF;
    };
   ~CineonFileInfo() {};
};

struct CineonChannel {
    U8      designator[2];  //channel designator codes (see table)
    U8      bitsPerPixel;   //bits per pixel
    U8      align;          //unused 1 byte for word alignment
    U32     pixelsPerLine;  //pixel per line
    U32     linesPerImage;  //lines per image
    R32     minValue;       //minimum data value
    R32     minQuantity;    //minimum quantity repersented
    R32     maxValue;       //maximum data value
    R32     maxQuantity;    //maximum quantity repersented
    CineonChannel() {
        designator[0]   = U8_UNDEF;
        designator[1]   = U8_UNDEF;
        align           = U8_UNDEF;
        pixelsPerLine   = U32_UNDEF;
        linesPerImage   = U32_UNDEF;
        minValue        = R32_UNDEF;
        minQuantity     = R32_UNDEF;
        maxValue        = R32_UNDEF;
        maxQuantity     = R32_UNDEF;
    };
   ~CineonChannel() {};
};
struct CineonImageInfo {
    U8      orientation;    //image orientation (see documentation)
    U8      numChannels;    //number of channels (1-8)
    U8      align[2];       //unused 2 bytes, space for word alignment
    CineonChannel  channels[CINEON_HDR_MAX_NUMBER_CHANNELS];  //channel specifiers
    R32     whitePt[2];     //white point (color temperature); x,y pair
    R32     redPt[2];       //RED   primary chromaticity; x,y pair
    R32     greenPt[2];     //GREEN primary chromaticity; x,y pair
    R32     bluePt[2];      //BLUE  primary chromaticity; x,y pair
    ASCII   label[CINEON_HDR_IMAGE_INFO_LABEL_LENGTH];   //label text (other label info in user area; font, size, location)
    ASCII   reserved[28];   //reserved by Kodak for future use
    CineonImageInfo() {
        orientation     = U8_UNDEF;
        numChannels     = U8_UNDEF;
        align[0]        = U8_UNDEF;
        align[1]        = U8_UNDEF;
        whitePt[0]      = R32_UNDEF;
        whitePt[1]      = R32_UNDEF;
        redPt[0]        = R32_UNDEF;
        redPt[1]        = R32_UNDEF;
        greenPt[0]      = R32_UNDEF;
        greenPt[1]      = R32_UNDEF;
        bluePt[0]       = R32_UNDEF;
        bluePt[1]       = R32_UNDEF;
        label[0]        = ASCII_UNDEF;
        reserved[0]     = ASCII_UNDEF;
    };
   ~CineonImageInfo() {};
};
struct CineonImageDataFormat {
    U8      interleave;     //data interleave (see documentation)
    U8      packing;        //data packing (see documentation)
    U8      signedType;     //data signed or unsigned (see documentation)
    U8      sense;          //image sense (see documentation)
    U32     eolPadding;     //number of bytes for end of line padding
    U32     eocPadding;     //number of bytes for end of channel padding
    ASCII   reserved[20];   //reserved by Kodak for future use
    CineonImageDataFormat() {
        interleave      = U8_UNDEF;
        packing         = U8_UNDEF;
        signedType      = U8_UNDEF;
        sense           = U8_UNDEF;
        eolPadding      = U32_UNDEF;
        eocPadding      = U32_UNDEF;
        reserved[0]     = ASCII_UNDEF;
    };
   ~CineonImageDataFormat() {};
};
struct CineonImageOrigin {
    S32     xOffset;        //X Offset (correlate digital data to source media)
    S32     yOffset;        //Y Offset (correlate digital data to source media)
    ASCII   filename[CINEON_HDR_FILENAME_LENGTH];     //image filename
    ASCII   date[CINEON_HDR_DATE_LENGTH];             //creation date (eg. yyyy:mm:dd)
    ASCII   time[CINEON_HDR_TIME_LENGTH];             //creation time (eg. hh:mm:ssxxx xxx-timezone, eg.EST)
    ASCII   device[CINEON_HDR_DEVICE_NAME_LENGTH];    //input device
    ASCII   modelNum[CINEON_HDR_DEVICE_MODEL_NUMBER_LENGTH];   //input device model number
    ASCII   serialNum[CINEON_HDR_DEVICE_SERIAL_NUMBER_LENGTH]; //input device serial number
    R32     xPitch;         //X (x determined by image orientation) input device pitch (samples/mm.)
    R32     yPitch;         //Y (y determined by image orientation) input device pitch (samples/mm.)
    R32     gamma;          //image gamma of capture device
    ASCII   reserved[40];   //reserved by Kodak for future use
    CineonImageOrigin() {
        xOffset         = S32_UNDEF;
        yOffset         = S32_UNDEF;
        filename[0]     = ASCII_UNDEF;
        date[0]         = ASCII_UNDEF;
        time[0]         = ASCII_UNDEF;
        device[0]       = ASCII_UNDEF;
        modelNum[0]     = ASCII_UNDEF;
        serialNum[0]    = ASCII_UNDEF;
        xPitch          = R32_UNDEF;
        yPitch          = R32_UNDEF;
        gamma           = R32_UNDEF;
        reserved[0]     = ASCII_UNDEF;
    };
   ~CineonImageOrigin() {};
};
struct CineonFilmInfo {
    U8      mfgID;          //file mfg. ID code  - 2 digit code from KEYKODE
    U8      type;           //file type. ID code - 2 digit code from KEYKODE
    U8      offset;         //Offset in perfs    - 2 digit code from KEYKODE
    U8      align;          //unused byte, space for word alignment
    U32     prefix;         //Prefix             - 6 digit code from KEYKODE
    U32     count;          //Count              - 4 digit code from KEYKODE
    ASCII   format[CINEON_HDR_FILM_FORMAT_LENGTH];     //Format - eg. "ACADEMY", "VISTAVISION", etc...
    U32     frame;          //frame position in sequence
    R32     rate;           //frame rate of original (frames per second)
    ASCII   attribute[CINEON_HDR_FRAME_ATTRIBUTE_LENGTH];     //Frame attribute - eg. "KEYFRAME"
    ASCII   slateInfo[CINEON_HDR_FRAME_SLATE_INFO_LENGTH];    //Slate information
    ASCII   reserved[740];  //reserved by Kodak for future use
    CineonFilmInfo() {
        mfgID           = U8_UNDEF;
        type            = U8_UNDEF;
        offset          = U8_UNDEF;
        align           = U8_UNDEF;
        prefix          = U32_UNDEF;
        count           = U32_UNDEF;
        format[0]       = ASCII_UNDEF;
        frame           = U32_UNDEF;
        rate            = R32_UNDEF;
        attribute[0]    = ASCII_UNDEF;
        slateInfo[0]    = ASCII_UNDEF;
        reserved[0]     = ASCII_UNDEF;
    };
   ~CineonFilmInfo() {};
};

struct CineonHeader {
    CineonFileInfo         fileInfo;
    CineonImageInfo        imageInfo;
    CineonImageDataFormat  imageDataFormat;
    CineonImageOrigin      imageOrigin;
    CineonFilmInfo         filmInfo;
};

//#pragma pack()

class CineonFile {
    CineonHeader    mHeader;
    FILE*           mStream;
    BOOL            mSwappedHeader;
    U16*            mDensityLUT;
    U16*            mLogLUT;
    R32             mRefWhite;
    R32             mRefBlack;
    
    BOOL    ColorCorrect(U16&, int);
    char    GetCellBoundary();
    U16     GetCellSize();
    U32     GetCellsPerLine(int = 0);
    U16     GetDensity(U16);
    U16     GetLog(U16);
    BOOL    SetDensities(U8);
    BOOL    SetLog(U8);
    U8      GetFieldsPerCell(int = 0);
    U32     GetImageOffset();
    U8      GetPixelSize(int = 0);
    BOOL    InitData();
    void    InitHeader();
    BOOL    LeftJustified();
    BOOL    PackFields();
    BOOL    SwapHeader();
    BOOL    NeedsToBeSwapped();
    BOOL    BigEndian();

public:
    CineonFile();
    CineonFile(FILE*);
    CineonFile(CineonHeader, FILE*);
   ~CineonFile();

    U8      GetBitsPerPixel(int = 0);
    R32     GetBluePt(int);
    U8      GetDataInterleave();
    R32     GetDeviceXPitch();
    R32     GetDeviceYPitch();
    U32     GetEOCPadding();
    U32     GetEOLPadding();
    U8      GetFilmMFGID();
    U8      GetFilmOffset();
    U32     GetFileSize();
    U8      GetFilmType();
    U32     GetFramePosition();
    R32     GetFrameRate();
    R32     GetGreenPt(int);
    BOOL    GetHeader(CineonHeader*);
    char*   GetImageCreationDate();
    char*   GetImageCreationTime();
    U32     GetImageFileSize();
    char*   GetImageFileName();
    char*   GetImageFormat();
    char*   GetImageFrameAttribute();
    R32     GetImageGamma();
    char*   GetImageInputDevice();
    char*   GetImageInputDeviceModelNumber();
    char*   GetImageInputDeviceSerialNumber();
    U8      GetImageOrientation();
    S32     GetImageXOffset();
    S32     GetImageYOffset();
    U32     GetLinesPerImage(int   = 0);
    BOOL    GetScanLine(U16*, U32, U32);
    U8      GetNumberChannels();
    U8      GetPacking();
    U32     GetPixelsPerLine(int  = 0);
    R32     GetRedPt(int);
    U8      GetSense();
    U8      GetSigned();
    char*   GetVersionNumber();
    R32     GetWhitePt(int);
    R32     SetDeviceXPitch(R32);
    R32     SetDeviceYPitch(R32);
    BOOL    SetLUTs(int, R32, R32);
    BOOL    SetImageFileName(ASCII*);
    BOOL    SetImageCreationDate(ASCII*);
    BOOL    SetImageCreationTime(ASCII*);
    BOOL    SetImageOffset(U32);
    BOOL    SetImageInputDevice(ASCII*);
    BOOL    SetImageInputDeviceModelNumber(ASCII*);
    BOOL    SetImageInputDeviceSerialNumber(ASCII*);
    BOOL    SetImageOrientation(U8);
    BOOL    SetFileSize(U32);
    BOOL    SetNumberChannels(U8);
    BOOL    SetBitsPerPixel(U8, U8);
    BOOL    SetPixelsPerLine(U8, U32);
    BOOL    SetLinesPerImage(U8, U32);
    BOOL    SetWhitePt(R32, R32);
    BOOL    SetRedPt(R32, R32);
    BOOL    SetGreenPt(R32, R32);
    BOOL    SetBluePt(R32, R32);
    BOOL    SetDataInterleave(U8);
    BOOL    SetPacking(U8);
    BOOL    SetSigned(U8);
    BOOL    SetSense(U8);
    BOOL    SetEOLPadding(U32);
    BOOL    SetEOCPadding(U32);
    BOOL    SetImageGamma(R32);
    BOOL    SetImageXOffset(S32);
    BOOL    SetImageYOffset(S32);
    BOOL    SetFramePosition(U32);
    BOOL    IsSupported();
    BOOL    ReadHeader(FILE*);
    BOOL    VerifyHeader();
    BOOL    WriteHeader(FILE*);
    BOOL    WriteScanLine(FILE*, U16*, U32, U32, BOOL=TRUE);
};

#endif
