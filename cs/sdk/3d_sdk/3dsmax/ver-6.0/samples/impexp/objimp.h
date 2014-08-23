
/**********************************************************************
 *<
	FILE: objimp.h

	DESCRIPTION:  Wavefront .OBJ file import module header file

	CREATED BY: Don Brittain & Tom Hudson

	HISTORY: created 30 December 1994

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

class OBJImport : public SceneImport {
public:
					OBJImport();
					~OBJImport();
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
