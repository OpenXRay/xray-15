
/*****************************************************************

  This is just a header that contains all our dialog proc classes

******************************************************************/

#ifndef __BONESDEF_DLGPROC__H
#define __BONESDEF_DLGPROC__H

class BonesDefMod;
class BoneModData;


//this is the class that brings up the name selection dialog
//when the user adds bones
class DumpHitDialog : public HitByNameDlgCallback {
public:
	BonesDefMod *eo;
	DumpHitDialog(BonesDefMod *e) {eo=e;};
	TCHAR *dialogTitle() {return _T(GetString(IDS_PW_SELECTBONES));};
	TCHAR *buttonText() {return _T(GetString(IDS_PW_SELECT));};
	BOOL singleSelect() {return FALSE;};
	BOOL useProc() {return TRUE;};
	int filter(INode *node);
	void proc(INodeTab &nodeTab);
	};

//Dialog proc for the basic param block
class MapDlgProc : public ParamMap2UserDlgProc {
	public:
		BonesDefMod *mod;		
		MapDlgProc(BonesDefMod *m) {mod = m;}		
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {
				delete this;
				}
	};

//Dialog proc for the advance param block
class AdvanceMapDlgProc : public ParamMap2UserDlgProc {
	public:
		BonesDefMod *mod;		
		AdvanceMapDlgProc(BonesDefMod *m) {mod = m;}		
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {
				delete this;
				}
	};


//Dialog proc for the gizmo param block
class GizmoMapDlgProc : public ParamMap2UserDlgProc {
	public:
		BonesDefMod *mod;		
		GizmoMapDlgProc(BonesDefMod *m) {mod = m;}		
		void UpdateCopyPasteButtons();
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {
				delete this;
				}
	};


//Dialog proc for the joint gizmo param block
class GizmoParamsMapDlgProc : public ParamMap2UserDlgProc {
	public:
		GizmoJointClass *giz;		
		GizmoParamsMapDlgProc(GizmoJointClass *m) {giz = m;}		
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {
				delete this;
				}
		void FilloutText();
		
	};


//Mouse call back to insert cross sections
class CreateCrossSectionMouseProc : public MouseCallBack {
        private:
                BonesDefMod *mod;
                IObjParam *iObjParams;
                IPoint2 om;
                int type; // See above
				Point3 a,b;
				float SplineU;
        
        protected:
                HCURSOR GetTransformCursor();
                BOOL HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags );
                BOOL AnyHits( ViewExp *vpt ) { return vpt->NumSubObjHits(); }           

        public:
                CreateCrossSectionMouseProc(BonesDefMod* bmod, IObjParam *i) { mod=bmod; iObjParams=i; }
                int proc( 
                        HWND hwnd, 
                        int msg, 
                        int point, 
                        int flags, 
                        IPoint2 m );
                void SetType(int type) { this->type = type; }
				void GetHit(float &ou) {ou = SplineU;}
        };




//Command mode to handle inserting cross sections
class CreateCrossSectionMode : public CommandMode {
        private:
                ChangeFGObject fgProc;
                CreateCrossSectionMouseProc eproc;
                BonesDefMod* mod;
                int type; // See above

        public:
                CreateCrossSectionMode(BonesDefMod* bmod, IObjParam *i);// :
//                        fgProc(bmod), eproc(bmod,i) {mod=bmod;}

                int Class() { return MODIFY_COMMAND; }
                int ID() { return CID_CREATECROSS; }
                MouseCallBack *MouseProc(int *numPoints) { *numPoints=1; return &eproc; }
                ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
                BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
                void EnterMode();
                void ExitMode();
                void SetType(int type) { this->type = type; eproc.SetType(type); }
        };



//--- CustMod dlg proc ------------------------------
//Pick mode to pick/add a bone from the viewport
class PickControlNode : 
		public PickModeCallback,
		public PickNodeCallback {
	public:				
		BonesDefMod *mod;
		PickControlNode() {mod=NULL;}
		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);		
		BOOL Pick(IObjParam *ip,ViewExp *vpt);		
		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);		
		BOOL Filter(INode *node);
		PickNodeCallback *GetFilter() {return this;}
		BOOL RightClick(IObjParam *ip,ViewExp *vpt) {return TRUE;}
		HCURSOR GetDefCursor(IObjParam *ip);
		HCURSOR GetHitCursor(IObjParam *ip);


	};

#endif