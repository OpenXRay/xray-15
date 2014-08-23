/**********************************************************************
 *<
	FILE: objimp.cpp

	DESCRIPTION:  Wavefront .OBJ file import module

	CREATED BY: Don Brittain and Tom Hudson

	HISTORY: created 30 December 1994

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "Max.h"
#include <stdio.h>
#include <direct.h>
#include <commdlg.h>
#include "objimp.h"
#include "objires.h"

// The file stream


HINSTANCE hInstance;

TCHAR *GetString(int id)
	{
	static TCHAR buf[256];
	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
	}

// Jaguar interface code

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
	hInstance = hinstDLL;
	switch(fdwReason) {
		case DLL_PROCESS_ATTACH:
			//MessageBox(NULL,L"OBJIMP.DLL: DllMain",L"OBJIMP",MB_OK);
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
		}
	return(TRUE);
	}


//------------------------------------------------------

class OBJClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new OBJImport; }
	const TCHAR *	ClassName() { return GetString(IDS_TH_WAVEFRONT); }
	SClass_ID		SuperClassID() { return SCENE_IMPORT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(1,0); }
	const TCHAR* 	Category() { return GetString(IDS_TH_SCENEIMPORT);  }
	};

static OBJClassDesc OBJDesc;

//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_TH_OBJIMPORTDLL); }

__declspec( dllexport ) int
LibNumberClasses() { return 1; }

__declspec( dllexport ) ClassDesc *
LibClassDesc(int i) {
	switch(i) {
		case 0: return &OBJDesc; break;
		default: return 0; break;
		}

	}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG 
LibVersion() { return VERSION_3DSMAX; }

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}

//
// .OBJ import module functions follow:
//

OBJImport::OBJImport() {
	}

OBJImport::~OBJImport() {
	}

int
OBJImport::ExtCount() {
	return 1;
	}

const TCHAR *
OBJImport::Ext(int n) {		// Extensions supported for import/export modules
	switch(n) {
		case 0:
			return _T("OBJ");
		}
	return _T("");
	}

const TCHAR *
OBJImport::LongDesc() {			// Long ASCII description (i.e. "Targa 2.0 Image File")
	return GetString(IDS_TH_WAVEFRONTOBJFILE);
	}
	
const TCHAR *
OBJImport::ShortDesc() {			// Short ASCII description (i.e. "Targa")
	return GetString(IDS_TH_WAVEFRONT);
	}

const TCHAR *
OBJImport::AuthorName() {			// ASCII Author name
	return GetString(IDS_TH_DON_AND_TOM);
	}

const TCHAR *
OBJImport::CopyrightMessage() {	// ASCII Copyright message
	return GetString(IDS_TH_COPYRIGHT_YOST_GROUP);
	}

const TCHAR *
OBJImport::OtherMessage1() {		// Other message #1
	return _T("");
	}

const TCHAR *
OBJImport::OtherMessage2() {		// Other message #2
	return _T("");
	}

unsigned int
OBJImport::Version() {				// Version number * 100 (i.e. v3.01 = 301)
	return 100;
	}

void
OBJImport::ShowAbout(HWND hWnd) {			// Optional
 	}

int normparse(TCHAR *s, int *nnum)
{
    int slashcnt;

    slashcnt = 0;
    while(*s) {
        if(*s++ == '/')
            slashcnt++;
        if(slashcnt == 2) {
            *nnum = _ttoi(s) - 1;
            return 1;
        }
    }
    return 0;
}

int texparse(TCHAR *s, int *tnum)
{
    int slashcnt;

    slashcnt = 0;
    while(*s) {
        if(*s++ == '/')
            slashcnt++;
        if(slashcnt == 1 && *s != '/') {
            *tnum = _ttoi(s) - 1;
            return 1;
        }
    }
    return 0;
}

int parse(TCHAR *s, TCHAR **v)
{
    int c;                 /* number of tokens */

    c = 0;
    while(1) {
		      /* skip spaces or tabs - currently only delimiters */
		while((*s == ' ') || (*s == '\t'))
            s++;
		if(*s && *s != '\n' && *s != '\r') {  /* found a token - set ptr and bump counter */
            v[c++] = s;
		}
        if(*s == '"') {      /* in quoted string */
            v[c-1] += 1;
            s++;
	    	while(*s != '"' && *s && *s != '\n' && *s != '\r')  /* skip over string */
	        	s++;
           	if(*s == '"')
               	*s++ = '\0';
        }
		else                 /* in ordinary token: skip over it */
		    while(*s != ' ' && *s != '\t' && *s && *s != '\n' && *s != '\r')
	    	    s++;
		if(*s == 0 || *s == '\n' || *s == '\r') {   /* if at end of input string, leave */
	   		*s = 0;
    	    break;
		}
		else                 /* else terminate the token string */
            *s++ = 0;
    }
    v[c] = _T("");
    return c;
}

inline int getLine(FILE *fp, TCHAR *buf)
{
    return _fgetts(buf, 500, fp) != NULL;
}	

BOOL objFileRead(const TCHAR *filename, Mesh *msh)
{
    TCHAR buf[512];
    TCHAR *av[25];
	FILE *fp;
    int ac = 0;
    int vcnt = 0;
    int ncnt = 0;
    int tcnt = 0;
    int gons = 0;
    int i, vnum;
    int index;
    int normals;
    int texverts;
	DWORD vtxIndex[32];
	Point3 *vlst, *nlst, *tlst;
	TCHAR *stop;

	if((fp = _tfopen(filename, _T("r"))) == NULL)
		return 0;

	// scan to count verts and faces

    while(getLine(fp, buf)) {
		ac = parse(buf, av);
		switch(av[0][0]) {
		default :
	    	break;
		case 'v' :
	    	if(av[0][1] == 't') {
    	        tcnt++;
				break;
            }
	    	if(av[0][1] == 'n') {
    	        ncnt++;
				break;
            }
	    	vcnt++;
	    	break;
		case 'f' :
			gons++;
			if(ac > 4)
				gons++;
	    	break;
		}
    }
	fclose(fp);

	vlst = new Point3[vcnt];
	nlst = new Point3[ncnt];
	tlst = new Point3[tcnt];

	msh->setNumVerts(vcnt);
	msh->setNumTVerts(tcnt);
	msh->setNumFaces(gons);

	vcnt = ncnt = tcnt = gons = 0;

	if((fp = _tfopen(filename, _T("r"))) == NULL)
		return 0;

    while(getLine(fp, buf)) {
		ac = parse(buf, av);
		switch(av[0][0]) {
		default :
	    	break;
		case 'v' :
	    	if(av[0][1] == 't') {
	        	tlst[tcnt][0] = (float)_tcstod(av[1], &stop);
	        	tlst[tcnt][1] = (float)_tcstod(av[2], &stop);
	        	tlst[tcnt][2] = (float)_tcstod(av[3], &stop);
    	        tcnt++;
				break;
            }
	    	else if(av[0][1] == 'n') {
	        	nlst[ncnt][0] = (float)_tcstod(av[1], &stop);
	        	nlst[ncnt][1] = (float)_tcstod(av[2], &stop);
	        	nlst[ncnt][2] = (float)_tcstod(av[3], &stop);
    	        ncnt++;
				break;
            }
	    	vlst[vcnt][0] = (float)20.0 * (float)_tcstod(av[1], &stop);
	    	vlst[vcnt][1] = (float)20.0 * (float)_tcstod(av[2], &stop);
	    	vlst[vcnt][2] = (float)20.0 * (float)_tcstod(av[3], &stop);
	    	vcnt++;
	    	break;
		case 'f' :
            normals = normparse(av[1], &vnum);
			texverts = texparse(av[1], &vnum);
	    	for(i = 1; i < ac; i++) {
				vnum = _ttoi(av[i]) - 1;
				vtxIndex[i-1] = (DWORD)vnum;
				msh->setVert(vnum, vlst[vnum]);
        	    if(texverts) {
            	    texparse(av[i], &index);
					msh->setTVert(vnum, tlst[index]);
	            }
        	    if(normals) {
            	    normparse(av[i], &index);
					msh->setNormal(vnum, nlst[index]);
	            }
			}
			msh->faces[gons].setVerts(vtxIndex);
			// don't set texture verts yet -- wait till GFX supports it!

			if(ac == 4)	{	// we got a triangle
				msh->faces[gons++].setEdgeVisFlags(1, 1, 1);
			}
			else {	// we got >= quad -- assume it's a quad
				msh->faces[gons++].setEdgeVisFlags(1, 1, 0);
				vtxIndex[1] = vtxIndex[2];
				vtxIndex[2] = vtxIndex[3];
				msh->faces[gons].setVerts(vtxIndex);
				msh->faces[gons++].setEdgeVisFlags(0, 1, 1);
			}
			break;
		}
    }
	fclose(fp);
	msh->buildNormals();
	msh->buildBoundingBox();
	msh->InvalidateEdgeList();

    delete [] vlst;
    delete [] nlst;
    delete [] tlst;

	return TRUE;
}

int
OBJImport::DoImport(const TCHAR *filename,ImpInterface *i,Interface *gi, BOOL suppressPrompts) {
	TriObject *object = CreateNewTriObject();
	if(!object)
		return 0;
	if(objFileRead(filename, &object->GetMesh())) {
		ImpNode *node = i->CreateNode();
		if(!node) {
			delete object;
			return 0;
			}
		Matrix3 tm;
		tm.IdentityMatrix();
		node->Reference(object);
		node->SetTransform(0,tm);
		i->AddNodeToScene(node);
		node->SetName(GetString(IDS_TH_WAVE_OBJ_NAME));
		i->RedrawViews();
		return 1;
		}
	return 0;
	}

