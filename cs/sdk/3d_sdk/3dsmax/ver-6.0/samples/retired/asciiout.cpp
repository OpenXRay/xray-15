/**********************************************************************
 *<
	FILE: asciiout.cpp

	DESCRIPTION:  A utility that outputs an object in ASCII form

	CREATED BY: Rolf Berteig

	HISTORY: created December 26 1995

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "util.h"
#include "utilapi.h"
#include "polyshp.h"
#include "shape.h"

#ifndef NO_UTILITY_ASCIIOUTPUT	// russom - 12/04/01

#define ASCII_OUT_CLASS_ID		0x8fbc04ea

class AsciiOut : public UtilityObj {
	public:
		IUtil *iu;
		Interface *ip;
		HWND hPanel;
		ICustButton *iPick;
		BOOL objSpace;
		BOOL shapesAsBeziers;

		AsciiOut();
		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);
		void DeleteThis() {}

		void Init(HWND hWnd);
		void Destroy(HWND hWnd);

		void SetObjectSpace(BOOL sw) { objSpace = sw; }
		void SetShapesAsBeziers(BOOL sw) { shapesAsBeziers = sw; }
		void OutputObject(INode *node,TCHAR *fname);
	};
static AsciiOut theAsciiOut;

class AsciiOutClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return &theAsciiOut;}
	const TCHAR *	ClassName() {return GetString(IDS_RB_ASCIIOBJECTOUT);}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return Class_ID(ASCII_OUT_CLASS_ID,0);}
	const TCHAR* 	Category() {return _T("");}
	};

static AsciiOutClassDesc asciiOutDesc;
ClassDesc* GetAsciiOutDesc() {return &asciiOutDesc;}

class AsciiOutPickNodeCallback : public PickNodeCallback {
	public:		
		BOOL Filter(INode *node);
	};

BOOL AsciiOutPickNodeCallback::Filter(INode *node)
	{
	ObjectState os = node->EvalWorldState(theAsciiOut.ip->GetTime());
	if ((os.obj->SuperClassID()==GEOMOBJECT_CLASS_ID &&
		os.obj->IsRenderable()) || os.obj->SuperClassID()==SHAPE_CLASS_ID) return TRUE;
	else return FALSE;
	}

static AsciiOutPickNodeCallback thePickFilt;

class AsciiOutPickModeCallback : public PickModeCallback {
	public:		
		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);
		
		void EnterMode(IObjParam *ip) {theAsciiOut.iPick->SetCheck(TRUE);}
		void ExitMode(IObjParam *ip) {theAsciiOut.iPick->SetCheck(FALSE);}

		PickNodeCallback *GetFilter() {return &thePickFilt;}
		BOOL RightClick(IObjParam *ip,ViewExp *vpt) {return TRUE;}
	};

static AsciiOutPickModeCallback thePickMode;

BOOL AsciiOutPickModeCallback::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
	{
	return ip->PickNode(hWnd,m,&thePickFilt)?TRUE:FALSE;
	}

BOOL AsciiOutPickModeCallback::Pick(IObjParam *ip,ViewExp *vpt)
	{
	INode *node = vpt->GetClosestHit();
	if (node) {
		static TCHAR fname[256] = {'\0'};
		OPENFILENAME ofn;
		memset(&ofn,0,sizeof(ofn));
		FilterList fl;
		fl.Append( GetString(IDS_RB_ASCIIFILES));
		fl.Append( _T("*.asc"));		
		TSTR title = GetString(IDS_RB_SAVEOBJECT);

		ofn.lStructSize     = sizeof(OPENFILENAME);
	    ofn.hwndOwner       = ip->GetMAXHWnd();
	    ofn.lpstrFilter     = fl;
	    ofn.lpstrFile       = fname;
	    ofn.nMaxFile        = 256;    
	    ofn.lpstrInitialDir = ip->GetDir(APP_EXPORT_DIR);
	    ofn.Flags           = OFN_HIDEREADONLY|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
	    ofn.lpstrDefExt     = _T("asc");
		ofn.lpstrTitle      = title;

tryAgain:
		if (GetSaveFileName(&ofn)) {
			if (DoesFileExist(fname)) {
				TSTR buf1;
				TSTR buf2 = GetString(IDS_RB_SAVEOBJECT);
				buf1.printf(GetString(IDS_RB_FILEEXISTS),fname);
				if (IDYES!=MessageBox(
					theAsciiOut.hPanel,
					buf1,buf2,MB_YESNO|MB_ICONQUESTION)) {
					goto tryAgain;
					}
				}
			theAsciiOut.OutputObject(node,fname);
			}
		}
	return TRUE;
	}


static INT_PTR CALLBACK AsciiOutDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG:
			theAsciiOut.Init(hWnd);			
			break;
		
		case WM_DESTROY:
			theAsciiOut.Destroy(hWnd);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					theAsciiOut.iu->CloseUtility();
					break;				
		
				case IDC_ASCIIOUT_PICK:
					theAsciiOut.ip->SetPickMode(&thePickMode); 
					break;

				case IDC_OBJECT_SPACE:
					theAsciiOut.SetObjectSpace(IsDlgButtonChecked(hWnd, IDC_OBJECT_SPACE));
					break;

				case IDC_SHAPES_AS_BEZIERS:
					theAsciiOut.SetShapesAsBeziers(IsDlgButtonChecked(hWnd, IDC_SHAPES_AS_BEZIERS));
					break;
				}
			break;

		default:
			return FALSE;
		}
	return TRUE; 
	}

AsciiOut::AsciiOut()
	{
	iu = NULL;
	ip = NULL;	
	hPanel = NULL;	
	iPick = NULL;
	objSpace = FALSE;
	shapesAsBeziers = FALSE;
	}

void AsciiOut::BeginEditParams(Interface *ip,IUtil *iu) 
	{
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_ASCIIOUT_PANEL),
		AsciiOutDlgProc,
		GetString(IDS_RB_ASCIIOBJECTOUT),
		0);
	}
	
void AsciiOut::EndEditParams(Interface *ip,IUtil *iu) 
	{
	ip->ClearPickMode();
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
	}

void AsciiOut::Init(HWND hWnd)
	{
	iPick = GetICustButton(GetDlgItem(hWnd,IDC_ASCIIOUT_PICK));
	iPick->SetType(CBT_CHECK);
	iPick->SetHighlightColor(GREEN_WASH);
	CheckDlgButton(hWnd, IDC_OBJECT_SPACE, objSpace);
	CheckDlgButton(hWnd, IDC_SHAPES_AS_BEZIERS, shapesAsBeziers);
	}

void AsciiOut::Destroy(HWND hWnd)
	{
	ReleaseICustButton(iPick);
	iPick = NULL;
	}

class NullView: public View {
	public:
		Point2 ViewToScreen(Point3 p) { return Point2(p.x,p.y); }
		NullView() { worldToView.IdentityMatrix(); screenW=640.0f; screenH = 480.0f; }
	};

void AsciiOut::OutputObject(INode *node,TCHAR *fname)
	{
	ObjectState os = node->EvalWorldState(theAsciiOut.ip->GetTime());
	if(os.obj->SuperClassID()==GEOMOBJECT_CLASS_ID) {
		BOOL needDel;
		NullView nullView;
		Mesh *mesh = ((GeomObject*)os.obj)->GetRenderMesh(ip->GetTime(),node,nullView,needDel);
		if (!mesh) return;

		FILE *file = fopen(fname,_T("wt"));
		Matrix3 tm = node->GetObjTMAfterWSM(theAsciiOut.ip->GetTime());
		
		if (file) {
			fprintf(file,"\nNamed Object: \"%s\"\n",node->GetName());
			fprintf(file,"Tri-mesh, Vertices: %d     Faces: %d\n",
				mesh->getNumVerts(), mesh->getNumFaces());
			fprintf(file,"Vertex list:\n");
			for (int i=0; i<mesh->getNumVerts(); i++) {
				Point3 v = objSpace ? mesh->verts[i] : (tm * mesh->verts[i]);
				fprintf(file,"Vertex %d: X: %f     Y: %f     Z: %f\n",
					i, v.x, v.y, v.z);
				}

			fprintf(file,"Face list:\n");
			for (i=0; i<mesh->getNumFaces(); i++) {
				fprintf(file,"Face %d:    A:%d B:%d C:%d AB:%d BC:%d CA:%d\n",
					i, mesh->faces[i].v[0], mesh->faces[i].v[1], mesh->faces[i].v[2],
					mesh->faces[i].getEdgeVis(0) ? 1 : 0,
					mesh->faces[i].getEdgeVis(1) ? 1 : 0,
					mesh->faces[i].getEdgeVis(2) ? 1 : 0);
				fprintf(file,"Smoothing: ");			
				for (int j=0; j<32; j++) {
					if (mesh->faces[i].smGroup & (1<<j)) {
						if (mesh->faces[i].smGroup>>(j+1)) {
							fprintf(file,"%d, ",j+1);
						} else {
							fprintf(file,"%d ",j+1);
							}
						}
					}
				fprintf(file,"\n");
				}
			
			fclose(file);
			}	
		if (needDel) delete mesh;
		}
	else
	if(os.obj->SuperClassID()==SHAPE_CLASS_ID) {
		FILE *file = fopen(fname,_T("wt"));
		Matrix3 tm = node->GetObjTMAfterWSM(theAsciiOut.ip->GetTime());
		
		if (file) {
			if(shapesAsBeziers && ((ShapeObject *)os.obj)->CanMakeBezier()) {
				BezierShape shape;
				((ShapeObject *)os.obj)->MakeBezier(ip->GetTime(), shape);
				Spline3D *spline = shape.GetSpline(0);
				fprintf(file,"\nNamed Object: \"%s\"\n",node->GetName());

				for (int i=0; i<spline->KnotCount(); i++) {
					Point3 k = spline->GetKnotPoint(i);
					Point3 in = spline->GetInVec(i);
					Point3 out = spline->GetOutVec(i);
					if(!objSpace) {
						k = k * tm;
						in = in * tm;
						out = out * tm;
						}
					fprintf(file,"Knot %d: %f %f %f In: %f %f %f Out: %f %f %f\n", i,
						k.x, k.y, k.z, in.x, in.y, in.z, out.x, out.y, out.z);
					}
				}
			else {
				PolyShape shape;
				((ShapeObject *)os.obj)->MakePolyShape(ip->GetTime(), shape);
				fprintf(file,"\nNamed Object: \"%s\"\n",node->GetName());
				fprintf(file,"Shape, Lines: %d\n",shape.numLines);

				for (int poly=0; poly<shape.numLines; poly++) {
					PolyLine &line = shape.lines[poly];
					fprintf(file,"Line %d: %d vertices [%s]\n",poly, line.numPts, line.IsClosed() ? "CLOSED":"OPEN");
					for(int vert = 0; vert < line.numPts; ++vert) {
						PolyPt &p = line.pts[vert];
						Point3 v = objSpace ? p.p : (tm * p.p);
						fprintf(file,"Vertex %d: X: %f     Y: %f     Z: %f  ",
							vert, v.x, v.y, v.z);
						if(p.flags & POLYPT_KNOT)
							fprintf(file,"[KNOT]");
						if(p.flags & POLYPT_INTERPOLATED)
							fprintf(file,"[INTERP]");
						if(p.flags & POLYPT_SMOOTH)
							fprintf(file,"[SM]");
						if(p.flags & POLYPT_SEG_SELECTED)
							fprintf(file,"[SEL]");
						fprintf(file,"\n");
						}
					}
				}
			
			fclose(file);
			}	
		}
	else
		assert(0);
	}

#endif // NO_UTILITY_ASCIIOUTPUT
