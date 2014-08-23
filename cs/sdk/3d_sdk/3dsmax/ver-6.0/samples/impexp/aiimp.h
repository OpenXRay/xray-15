
/**********************************************************************
 *<
	FILE: aiimp.h

	DESCRIPTION:  .AI file import module header file

	CREATED BY: Tom Hudson

	HISTORY: created 28 June 1996

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/


#define SINGLE_SHAPE 0
#define MULTIPLE_SHAPES 1

class AIShapeImport : public SceneImport {
	friend INT_PTR CALLBACK ShapeImportOptionsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

public:
	static int		importType;
	BOOL			gotStuff;
	SplineShape *	splShape;
	BezierShape *	shape;
	Spline3D *		spline;
	int				shapeNumber;
					AIShapeImport();
					~AIShapeImport();
	int				ExtCount();					// Number of extensions supported
	const TCHAR *	Ext(int n);					// Extension #n (i.e. "3DS")
	const TCHAR *	LongDesc();					// Long ASCII description (i.e. "Autodesk 3D Studio File")
	const TCHAR *	ShortDesc();				// Short ASCII description (i.e. "3D Studio")
	const TCHAR *	AuthorName();				// ASCII Author name
	const TCHAR *	CopyrightMessage();			// ASCII Copyright message
	const TCHAR *	OtherMessage1();			// Other message #1
	const TCHAR *	OtherMessage2();			// Other message #2
	unsigned int	Version();					// Version number * 100 (i.e. v3.01 = 301)
	void			ShowAbout(HWND hWnd);		// Show DLL's "About..." box
	int				DoImport(const TCHAR *name,ImpInterface *i,Interface *gi, BOOL suppressPrompts=FALSE);	// Import file
	BOOL			StartWorkingShape();
	BOOL			FinishWorkingShape(BOOL forceFinish,ImpInterface *i);
	};

// Handy file class

class WorkFile {
private:
	FILE *stream;
public:
					WorkFile(const TCHAR *filename,const TCHAR *mode) { stream = _tfopen(filename,mode); };
					~WorkFile() { if(stream) fclose(stream); stream = NULL; };
	FILE *			Stream() { return stream; };
	};

#define RDERR(ptr,count) { if(!fread(ptr,count,1,stream)) return 0; }
