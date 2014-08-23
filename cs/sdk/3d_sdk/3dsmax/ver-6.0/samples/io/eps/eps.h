#ifndef EPS_H
#define EPS_H

#define MAX_CHARS_PER_LINE 72
#define MAX_LINE_LENGTH 256
#define PS_UNITS_PER_INCH 72	/* Number of PostScript units per inch */
#define INCHES 0
#define MM 1
#define ASCII 0
#define BINARY 1
#define MM_PER_INCHES   25.4F
#define RGBIMAGE 0
#define GRAYIMAGE 1
#define PORTRAIT 0
#define LANDSCAPE 1
#define MAX_DSC_LINE_LENGTH 256

#define EPSF_COMMENT   "%!PS-Adobe"
#define END_COMMENTS   "%%EndComments"
#define CREATOR        "%%Creator:"
#define TITLE          "%%Title:"
#define CREATION_DATE  "%%CreationDate:"

#define CR  13
#define LF  10

// File header when preview is included in the file
// See page 729 in Appendix H of the big Red book
struct EPSFileHeader
{
    unsigned char  magic[4];
    ULONG          psStart;
    ULONG          psLength;
    ULONG          metafileStart;
    ULONG          metafileLength;
    ULONG          tiffStart;
    ULONG          tiffLength;
    USHORT         checksum;
};

typedef struct userSettable
{
    int        units;        // Inches or MM
    int        binary;       // Whether want binary image data or not
    int        preview;      // Whether want TIFF preview in file
    int        orientation;  // Options are portrait or landscape
    int        colorType;    // Whether image is output as rgb or gray
    float      paperHeight;  // Height of output (for centering image)
    float      paperWidth;   // Width of output (for centering image)
    float      xResolution;  // In dots per inch
    float      yResolution;  // In dots per inch
} UserSettable;

class BitmapIO_EPS : public BitmapIO {
    friend BOOL CALLBACK OutputCtrlDlgProc (HWND hDlg, UINT message, 
					    WPARAM wParam, LPARAM lParam);
    friend BOOL CALLBACK ImageInfoDlg (HWND hWnd, UINT message, WPARAM wParam,
				       LPARAM lParam) ;
private:
    TCHAR        creator[MAX_DSC_LINE_LENGTH + 1];
    TCHAR        title[MAX_DSC_LINE_LENGTH + 1];
    TCHAR        creationDate[MAX_DSC_LINE_LENGTH + 1];
    TCHAR        *iFilename;    // Input file name
    TCHAR        *oFilename;    // Output file name
    FILE         *ostream;     // Output file stream
    FILE         *istream;     // Input file stream
    int          includesPreview;

    // Used in reading PostScript header
    int          chPending;
    int          pendingCh;

    // User settable options
    UserSettable userSettings;

    float        tiffXResolution;   // In dots per inch
    float        tiffYResolution;   // In dots per inch
    float        tiffDownSample;

    ISpinnerControl *widthSpin;
    ISpinnerControl *heightSpin;
    ISpinnerControl *widthResSpin;
    ISpinnerControl *heightResSpin;

    // For sending progress reports to
    HWND          hWnd;

    // Methods for reading/writing the config file
    void Configure (void);
    void WriteConfigFile (void);
    double GetValue (const char *string);
    int GetOn (const char *string);
    int GetDataFormat (const char *string);
    int GetOrientation (const char *string);
    int GetColorType (const char *string);
    int GetUnits (const char *string);
    
    // Methods for writing the PostScript portion
    int        WriteHeader (Bitmap *bitmap, int xOffset, int yOffset);
    int        WriteImagePosition (Bitmap *bitmap, int xOffset, int yOffset);
    int        WriteAsciiRGBImage (Bitmap *bitmap);
    int        WriteBinaryRGBImage (Bitmap *bitmap);
    int        WriteAsciiGrayImage (Bitmap *bitmap);
    int        WriteBinaryGrayImage (Bitmap *bitmap);
    int        WriteTrailer ();
    void       Position (Bitmap *bitmap, int *xOffset, int *yOffset);

    // Methods for writing the TIFF preview
    int        WritePreview (int colorType, int orientation,
			     float x_resolution, float y_resolution,
			     Bitmap *bitmap);
    int        WritePSLength (int colorType, Bitmap *bitmap);
    int        WriteTiff (int ColorType, int orientation,
			  Bitmap *bitmap);
    int        WriteTiffHeader (int colorType, int orientation,
				Bitmap *bitmap);
    int        WriteTiffImage (int colorType, int orientation, Bitmap *bitmap);
    long       TiffWidth (Bitmap *bitmap);
    long       TiffLength (Bitmap *bitmap);
    long       TiffFileLength (int colorType, Bitmap *bitmap);
    int        WriteIFDLong (struct IFDLongEntry *entry);
    int        WriteIFDShort (struct IFDShortEntry *entry);

    // Used in handling combined PostScript with preview image
    BMMRES     ReadHeader (const TCHAR *filename);
    int        PositionToBeginOfPostScript ();
    int        PSReadLine (char *line);

    // UI functions
    BOOL       OutputControl (HWND hDlg, UINT message, 
			      WPARAM wParam, LPARAM lParam);
    BOOL       ImageInfoProc (HWND hWnd, UINT message, WPARAM wParam,
			      LPARAM lParam) ;

    // For file writing and reading
    char       *LocaleFloat (const char *string);
    char       *PeriodFloat (double num, const char *fmt);

public:

    // Constructors and destructors
    BitmapIO_EPS();
    ~BitmapIO_EPS();

    int               ExtCount();     // Number of extemsions supported
    const TCHAR *     Ext(int n);     // Extension #n (i.e. "3DS")

    // Descriptions
    const TCHAR *     LongDesc();     // Long ASCII description
    const TCHAR *     ShortDesc();    // Short ASCII description

    const TCHAR *     AuthorName();   // ASCII Author name
    const TCHAR *     CopyrightMessage();     // ASCII Copyright message
    const TCHAR *     OtherMessage1();        // Other message #1
    const TCHAR *     OtherMessage2();        // Other message #2

    unsigned int      Version();     // Version number * 100 (i.e. v3.01 = 301)

    int               Capability();           // Returns read/write capability

    BOOL              LoadConfigure (void *ptr);
    BOOL              SaveConfigure (void *ptr);
    DWORD             EvaluateConfigure ();

    void              ShowAbout(HWND hWnd);   // Show DLL's "About..." box
    BOOL              ShowControl(HWND hWnd, DWORD flag);  // DLL's control box

    // Return info about 
    BMMRES            GetImageInfo(BitmapInfo *info); 
    BMMRES            GetImageInfoDlg (HWND hWnd, BitmapInfo *info, 
				       const TCHAR *filename);

   // Image input
    BitmapStorage *   Load (BitmapInfo *fbi, Bitmap *map, BMMRES *status); 

    // Image output
    BMMRES            OpenOutput (BitmapInfo *fbi, Bitmap *map); 
    BMMRES            Save (const TCHAR *name,Bitmap *map);  
    BMMRES            Write (int frame);                     // Save image
    int               Close (int flag);
};

#endif   /* #ifndef EPS_H */
