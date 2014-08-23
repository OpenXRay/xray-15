/**********************************************************************
 *<
	FILE: exprctrl.cpp

	DESCRIPTION: An expression-based controller

	CREATED BY: Don Brittain

	HISTORY: created 24 August 1995 (Windows 95 debut!)

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#include "ctrl.h"
#include "units.h"
#include "exprlib.h"
#include <stdio.h>

extern HINSTANCE hInstance;

#define EXPR_POS_CONTROL_CNAME		GetString(IDS_DB_POSITION_EXPR)
#define EXPR_P3_CONTROL_CNAME		GetString(IDS_DB_POINT3_EXPR)
#define EXPR_FLOAT_CONTROL_CNAME	GetString(IDS_DB_FLOAT_EXPR)
#define EXPR_SCALE_CONTROL_CNAME	GetString(IDS_DB_SCALE_EXPR)
#define EXPR_ROT_CONTROL_CNAME		GetString(IDS_DB_ROTATION_EXPR)

#define EXPR_TICKS		0
#define EXPR_SECS		1
#define EXPR_FRAMES		2
#define EXPR_NTIME		3

class ExprControl;

// for the debug floater window
class ExprDebug : public TimeChangeCallback {
public:
	HWND hWnd;
	ExprControl *ec;
	TimeValue t;
	static int winX, winY;

	ExprDebug(HWND hParent, ExprControl *exprControl);
	~ExprDebug();

	void Invalidate();
	void SetTime(TimeValue tm)	{ t = tm; }
	void Update();
	void Init(HWND hWnd);

	void TimeChanged(TimeValue t)	{ SetTime(t); Update(); }
};

// scalar variables
class SVar {
public:
	TSTR	name;
	int		regNum;	// register number variable is assigned to
	int		refID;	// < 0 means constant
	float	val;	// value, if constant
	int		subNum;
	int		offset;	// tick offset
};

class VVar {
public:
	TSTR	name;
	int		regNum;	// reg num this var is assigned to
	int		refID;	// < 0 means constant
	Point3	val;	// value, if const
	int 	subNum;
	int		offset;	// tick offset
};

MakeTab(SVar);
MakeTab(VVar);

class VarRef {
public:
	VarRef()	{ client = NULL; refCt = 0; }
	VarRef(ReferenceTarget *c)	{ client = c; refCt = 1; }
	ReferenceTarget *client;
	int				refCt;
};

MakeTab(VarRef);

class ExprControl : public StdControl 
{
public:
	int			type;
	Expr		expr;
	int			timeSlots[4];
	int			sRegCt;
	int			vRegCt;
	int			curIndex;
	Point3		curPosVal;
	float		curFloatVal;
	Interval	ivalid;
	Interval	range;
	HWND		hParams;
	IObjParam *	ip;
	SVarTab		sVars;
	VVarTab		vVars;
	VarRefTab	refTab;
	TSTR		desc;
	HFONT		hFixedFont;
	HWND		hDlg;
	ExprDebug	*edbg;
	static int	winX, winY;
	BOOL blockGetNodeName; // RB 3/23/99: See imp of getNodeName()

	void	updRegCt(int val, int type);
	BOOL	dfnVar(int type, TCHAR *buf, int slot, int offset);
	int		getVarCount(int type) { return type == SCALAR_VAR ? sVars.Count() : vVars.Count(); }
	TCHAR * getVarName(int type, int i);
	int		getVarOffset(int type, int i);
	int		getRegNum(int type, int i);
	float	getScalarValue(int i);
	Point3	getVectorValue(int i);
	BOOL	assignScalarValue(int i, float val);
	BOOL	assignVectorValue(int i, Point3 &val);
	BOOL	assignController(int type, int i, ReferenceTarget *client, int subNum);
	void	deleteAllVars();
	void    getNodeName(ReferenceTarget *client,TSTR &name);

	ExprControl(int type, ExprControl &ctrl);
	ExprControl(int type, BOOL loading);
	~ExprControl();

	// Animatable methods
	int TrackParamsType() { return TRACKPARAMS_WHOLE; }
	
	void DeleteThis() { delete this; }
	int IsKeyable() { return 0; }		
	BOOL IsAnimated() {return TRUE;}
	Interval GetTimeRange(DWORD flags) { return range; }
	void EditTimeRange(Interval range,DWORD flags);
	void Hold();
	void MapKeys( TimeMap *map, DWORD flags );

	void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev );
	void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next );

	// Animatable's Schematic View methods
	SvGraphNodeReference SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags);
	TSTR SvGetRelTip(IGraphObjectManager *gom, IGraphNode *gNodeTarger, int id, IGraphNode *gNodeMaker);
	bool SvHandleRelDoubleClick(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker);

	void EditTrackParams(
			TimeValue t,	// The horizontal position of where the user right clicked.
			ParamDimensionBase *dim,
			TCHAR *pname,
			HWND hParent,
			IObjParam *ip,
			DWORD flags);

	// Reference methods
	int NumRefs() { return StdControl::NumRefs() + refTab.Count(); }
	ReferenceTarget* GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);
	RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage);
	void RefDeleted();

	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);

	// Control methods
	void Copy(Control *from);
	RefTargetHandle Clone(RemapDir& remap=NoRemap()) { 
		ExprControl *ctrl = new ExprControl(this->type, *this);
		CloneControl(ctrl,remap);
		BaseClone(this, ctrl, remap);
		return ctrl; 
		}		
	BOOL IsLeaf() { return TRUE; }
	void GetValueLocalTime(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);	
	void SetValueLocalTime(TimeValue t, void *val, int commit, GetSetMethod method) {}
	void Extrapolate(Interval range,TimeValue t,void *val,Interval &valid,int type);
	void *CreateTempValue();
	void DeleteTempValue(void *val);
	void ApplyValue(void *val, void *delta);
	void MultiplyValue(void *val, float m);

	void GetAbsoluteControlValue(INode *node,TimeValue t,Point3 *pt,Interval &iv);
};

int ExprControl::winX = -1;
int ExprControl::winY = -1;

class ExprPosControl : public ExprControl 
{
public:
	ExprPosControl(ExprPosControl &ctrl) : ExprControl(EXPR_POS_CONTROL_CLASS_ID, ctrl) {}
	ExprPosControl(BOOL loading=FALSE) : ExprControl(EXPR_POS_CONTROL_CLASS_ID, loading) {}
	~ExprPosControl() {}
	RefTargetHandle Clone(RemapDir& remap=NoRemap()) {  ExprPosControl *newob = new ExprPosControl(*this); BaseClone(this, newob, remap); return(newob); }		
	void GetClassName(TSTR& s) { s = EXPR_POS_CONTROL_CNAME; }
	Class_ID ClassID() { return Class_ID(EXPR_POS_CONTROL_CLASS_ID,0); }  
	SClass_ID SuperClassID() { return CTRL_POSITION_CLASS_ID; }  		
};

class ExprP3Control : public ExprControl 
{
public:
	ExprP3Control(ExprP3Control &ctrl) : ExprControl(EXPR_P3_CONTROL_CLASS_ID, ctrl) {}
	ExprP3Control(BOOL loading=FALSE) : ExprControl(EXPR_P3_CONTROL_CLASS_ID, loading) {}
	~ExprP3Control() {}

	RefTargetHandle Clone(RemapDir& remap=NoRemap()) {  ExprP3Control *newob = new ExprP3Control(*this); BaseClone(this, newob, remap); return(newob); }		
	void GetClassName(TSTR& s) { s = EXPR_P3_CONTROL_CNAME; }
	Class_ID ClassID() { return Class_ID(EXPR_P3_CONTROL_CLASS_ID,0); }  
	SClass_ID SuperClassID() { return CTRL_POINT3_CLASS_ID; }  		
};

class ExprFloatControl : public ExprControl 
{
public:
	ExprFloatControl(ExprFloatControl &ctrl) : ExprControl(EXPR_FLOAT_CONTROL_CLASS_ID, ctrl) {}
	ExprFloatControl(BOOL loading=FALSE) : ExprControl(EXPR_FLOAT_CONTROL_CLASS_ID, loading) {}
	~ExprFloatControl() {}

	RefTargetHandle Clone(RemapDir& remap=NoRemap()) {  ExprFloatControl *newob = new ExprFloatControl(*this); BaseClone(this, newob, remap); return(newob); }		
	void GetClassName(TSTR& s) { s = EXPR_FLOAT_CONTROL_CNAME; }
	Class_ID ClassID() { return Class_ID(EXPR_FLOAT_CONTROL_CLASS_ID,0); }  
	SClass_ID SuperClassID() { return CTRL_FLOAT_CLASS_ID; }  		
};

class ExprScaleControl : public ExprControl 
{
public:
	ExprScaleControl(ExprScaleControl &ctrl) : ExprControl(EXPR_SCALE_CONTROL_CLASS_ID, ctrl) {}
	ExprScaleControl(BOOL loading=FALSE) : ExprControl(EXPR_SCALE_CONTROL_CLASS_ID, loading) {}
	~ExprScaleControl() {}

	RefTargetHandle Clone(RemapDir& remap=NoRemap()) {  ExprScaleControl *newob = new ExprScaleControl(*this); BaseClone(this, newob, remap); return(newob); }		
	void GetClassName(TSTR& s) { s = EXPR_SCALE_CONTROL_CNAME; }
	Class_ID ClassID() { return Class_ID(EXPR_SCALE_CONTROL_CLASS_ID,0); }  
	SClass_ID SuperClassID() { return CTRL_SCALE_CLASS_ID; }  		
};

class ExprRotControl : public ExprControl 
{
public:
	ExprRotControl(ExprRotControl &ctrl) : ExprControl(EXPR_ROT_CONTROL_CLASS_ID, ctrl) {}
	ExprRotControl(BOOL loading=FALSE) : ExprControl(EXPR_ROT_CONTROL_CLASS_ID, loading) {}
	~ExprRotControl() {}

	RefTargetHandle Clone(RemapDir& remap=NoRemap()) {  ExprRotControl *newob = new ExprRotControl(*this); BaseClone(this, newob, remap); return(newob); }		
	void GetClassName(TSTR& s) { s = EXPR_ROT_CONTROL_CNAME; }
	Class_ID ClassID() { return Class_ID(EXPR_ROT_CONTROL_CLASS_ID,0); }  
	SClass_ID SuperClassID() { return CTRL_ROTATION_CLASS_ID; }  		
};

//********************************************************
// EXPRESSION CONTROL
//********************************************************
static Class_ID exprPosControlClassID(EXPR_POS_CONTROL_CLASS_ID,0); 
class ExprPosClassDesc:public ClassDesc 
{
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new ExprPosControl(loading); }
	const TCHAR *	ClassName() { return EXPR_POS_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_POSITION_CLASS_ID; }
	Class_ID		ClassID() { return exprPosControlClassID; }
	const TCHAR* 	Category() { return _T("");  }
};
static ExprPosClassDesc exprPosCD;
ClassDesc* GetExprPosCtrlDesc() {return &exprPosCD;}

static Class_ID exprP3ControlClassID(EXPR_P3_CONTROL_CLASS_ID,0); 
class ExprP3ClassDesc:public ClassDesc 
{
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new ExprP3Control(loading); }
	const TCHAR *	ClassName() { return EXPR_P3_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_POINT3_CLASS_ID; }
	Class_ID		ClassID() { return exprP3ControlClassID; }
	const TCHAR* 	Category() { return _T("");  }
};
static ExprP3ClassDesc exprP3CD;
ClassDesc* GetExprP3CtrlDesc() {return &exprP3CD;}

static Class_ID exprFloatControlClassID(EXPR_FLOAT_CONTROL_CLASS_ID,0); 
class ExprFloatClassDesc:public ClassDesc 
{
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new ExprFloatControl(loading); }
	const TCHAR *	ClassName() { return EXPR_FLOAT_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_FLOAT_CLASS_ID; }
	Class_ID		ClassID() { return exprFloatControlClassID; }
	const TCHAR* 	Category() { return _T("");  }
};
static ExprFloatClassDesc exprFloatCD;
ClassDesc* GetExprFloatCtrlDesc() {return &exprFloatCD;}

static Class_ID exprScaleControlClassID(EXPR_SCALE_CONTROL_CLASS_ID,0); 
class ExprScaleClassDesc:public ClassDesc 
{
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new ExprScaleControl(loading); }
	const TCHAR *	ClassName() { return EXPR_SCALE_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_SCALE_CLASS_ID; }
	Class_ID		ClassID() { return exprScaleControlClassID; }
	const TCHAR* 	Category() { return _T("");  }
};
static ExprScaleClassDesc exprScaleCD;
ClassDesc* GetExprScaleCtrlDesc() {return &exprScaleCD;}

static Class_ID exprRotControlClassID(EXPR_ROT_CONTROL_CLASS_ID,0); 
class ExprRotClassDesc:public ClassDesc 
{
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new ExprRotControl(loading); }
	const TCHAR *	ClassName() { return EXPR_ROT_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_ROTATION_CLASS_ID; }
	Class_ID		ClassID() { return exprRotControlClassID; }
	const TCHAR* 	Category() { return _T("");  }
};
static ExprFloatClassDesc exprRotCD;
ClassDesc* GetExprRotCtrlDesc() {return &exprRotCD;}

ExprControl::ExprControl(int t, ExprControl &ctrl)
{
	int i, j, ct, slot;
	TCHAR *cp;

	blockGetNodeName = FALSE;

	type = t;
	hParams = NULL;
	hFixedFont = CreateFont(14,0,0,0,0,0,0,0,0,0,0,0, FIXED_PITCH | FF_MODERN, _T(""));
	edbg = NULL;

	range = ctrl.range;
	curPosVal = ctrl.curPosVal;
	curFloatVal = ctrl.curFloatVal;
	ivalid = ctrl.ivalid;
	sRegCt = vRegCt = 0;
	desc = ctrl.desc;
	ct = ctrl.expr.getVarCount(SCALAR_VAR);
	for(i = 0; i < ct; i++) {
		cp = ctrl.expr.getVarName(SCALAR_VAR, i);
		updRegCt(slot = expr.defVar(SCALAR_VAR, cp), SCALAR_VAR);
		if(slot == -1)	// variable already defined
			continue;
		if(_tcscmp(cp, _T("T")) == 0)
			timeSlots[EXPR_TICKS] = slot;
		else if(_tcscmp(cp, _T("S")) == 0)
			timeSlots[EXPR_SECS] = slot;
		else if(_tcscmp(cp, _T("F")) == 0)
			timeSlots[EXPR_FRAMES] = slot;
		else if(_tcscmp(cp, _T("NT")) == 0)
			timeSlots[EXPR_NTIME] = slot;
		else {
			dfnVar(SCALAR_VAR, cp, slot, ctrl.getVarOffset(SCALAR_VAR, j=i-4));

			if(ctrl.sVars[j].refID < 0)
				assignScalarValue(slot-4, ctrl.sVars[j].val);	// -4:  001205  --prs.
			else
				assignController(SCALAR_VAR, slot-4, ctrl.refTab[ctrl.sVars[j].refID].client, ctrl.sVars[j].subNum);
		}
	}
	ct = ctrl.expr.getVarCount(VECTOR_VAR);
	for(i = 0; i < ct; i++) {
		cp = ctrl.expr.getVarName(VECTOR_VAR, i);
		updRegCt(slot = expr.defVar(VECTOR_VAR, cp), VECTOR_VAR);
		dfnVar(VECTOR_VAR, cp, slot, ctrl.getVarOffset(VECTOR_VAR, i));

		if(ctrl.vVars[i].refID < 0)
			assignVectorValue(slot, ctrl.vVars[i].val);
		else
			assignController(VECTOR_VAR, slot, ctrl.refTab[ctrl.vVars[i].refID].client, ctrl.vVars[i].subNum);
	}
	cp = ctrl.expr.getExprStr();
	expr.load(cp);
}

ExprControl::ExprControl(int t, BOOL loading) 
{
	blockGetNodeName = FALSE;
	type = t;
	range.Set(GetAnimStart(), GetAnimEnd());
	curPosVal = Point3(0,0,0);
	curFloatVal = 0.0f;
	ivalid.SetEmpty();
	sRegCt = vRegCt = 0;
	updRegCt(timeSlots[EXPR_TICKS] = expr.defVar(SCALAR_VAR, _T("T")), SCALAR_VAR);
	updRegCt(timeSlots[EXPR_SECS] = expr.defVar(SCALAR_VAR, _T("S")), SCALAR_VAR);
	updRegCt(timeSlots[EXPR_FRAMES] = expr.defVar(SCALAR_VAR, _T("F")), SCALAR_VAR);
	updRegCt(timeSlots[EXPR_NTIME] = expr.defVar(SCALAR_VAR, _T("NT")), SCALAR_VAR);
	switch(type = t) {
	case EXPR_POS_CONTROL_CLASS_ID:
	case EXPR_P3_CONTROL_CLASS_ID:
		expr.load(_T("[ 0, 0, 0 ]"));
		break;
	case EXPR_SCALE_CONTROL_CLASS_ID:
		expr.load(_T("[ 1, 1, 1 ]"));
		break;
	case EXPR_FLOAT_CONTROL_CLASS_ID:
		expr.load(_T("0"));
		break;
	case EXPR_ROT_CONTROL_CLASS_ID:
		expr.load(_T("{ [ 0, 0, 0 ], 0 }"));
		break;
	}
	hParams = NULL;
	hFixedFont = CreateFont(14,0,0,0,0,0,0,0,0,0,0,0, FIXED_PITCH | FF_MODERN, _T(""));
	edbg = NULL;
}

ExprControl::~ExprControl()
{
	deleteAllVars();
	DeleteObject(hFixedFont);
	if (hParams)
		DestroyWindow(hParams);
	DeleteAllRefsFromMe();
}

void ExprControl::updRegCt(int val, int type)
{
	if(type == SCALAR_VAR) {
		if(val+1 > sRegCt)
			sRegCt = val+1;
	}
	else {
		if(val+1 > vRegCt)
			vRegCt = val+1;
	}
}

BOOL ExprControl::dfnVar(int type, TCHAR *buf, int slot, int offset)
{
	int i;
	if(type == SCALAR_VAR) {
		SVar sv;
		sv.regNum = slot;
		sv.offset = offset;
		sv.refID = -1;
		sv.val = 0.0f;
		i = sVars.Append(1, &sv, 4);
		sVars[i].name = buf;
	}
	else {
		VVar vv;
//		vv.name = buf;
		vv.regNum = slot;
		vv.offset = offset;
		vv.refID = -1;
		vv.val.x =
		vv.val.y =
		vv.val.z = 0.0f;
		i = vVars.Append(1, &vv, 4);
		vVars[i].name = buf;
	}
	return TRUE;
}

void ExprControl::deleteAllVars()
{
	int i, ct;

	ct = sVars.Count();
	for(i = 0; i < ct; i++)
		delete sVars[i].name;
	sVars.SetCount(0);
	ct = vVars.Count();
	for(i = 0; i < ct; i++)
		delete vVars[i].name;
	vVars.SetCount(0);
}

TCHAR *ExprControl::getVarName(int type, int i)
{
	if(type == SCALAR_VAR) {
		if(i < sVars.Count())
			return sVars[i].name;
	}
	else {
		if(i < vVars.Count())
			return vVars[i].name;
	}
	return NULL;
}

int ExprControl::getVarOffset(int type, int i)
{
	if(type == SCALAR_VAR) {
		if(i < sVars.Count())
			return sVars[i].offset;
	}
	else {
		if(i < vVars.Count())
			return vVars[i].offset;
	}
	return 0;
}

int	ExprControl::getRegNum(int type, int i)
{
	if(type == SCALAR_VAR) {
		if(i < sVars.Count())
			return sVars[i].regNum;
	}
	else {
		if(i < vVars.Count())
			return vVars[i].regNum;
	}
	return -1;
}

float ExprControl::getScalarValue(int i)
{
	if((i < sVars.Count()) && (sVars[i].refID < 0))
		return sVars[i].val;
	return 0.0f;
}

Point3 ExprControl::getVectorValue(int i)
{
	if((i < vVars.Count()) && (vVars[i].refID < 0))
		return vVars[i].val;
	return Point3(0.0f, 0.0f, 0.0f);
}

BOOL ExprControl::assignScalarValue(int i, float val)
{
	if(i < sVars.Count()) {
		sVars[i].refID = -1;
		sVars[i].val = val;
		return TRUE;
	}
	return FALSE;
}

BOOL ExprControl::assignVectorValue(int i, Point3 &val)
{
	if(i < vVars.Count()) {
		vVars[i].refID = -1;
		vVars[i].val = val;
		return TRUE;
	}
	return FALSE;
}

class FloatFilter : public TrackViewFilter {
public:
	BOOL proc(Animatable *anim, Animatable *client,int subNum)
			{ return anim->SuperClassID() == CTRL_FLOAT_CLASS_ID; }
};

class VectorFilter : public TrackViewFilter {
public:
	BOOL proc(Animatable *anim, Animatable *client,int subNum)
			{ 
			if (anim->SuperClassID() == BASENODE_CLASS_ID) {
				INode *node = (INode*)anim;
				return !node->IsRootNode();
				}
			return anim->SuperClassID() == CTRL_POSITION_CLASS_ID ||
					 anim->SuperClassID() == CTRL_POINT3_CLASS_ID; 
			}
};

BOOL ExprControl::assignController(int type, int varIndex, ReferenceTarget *client, int subNum)
{
	if (!client) return FALSE;

	int i, ct;

	ct = refTab.Count();
	for(i = 0; i < ct; i++) {
		if(refTab[i].client == client) {
			refTab[i].refCt++;
			break;
		}
	}
	if(i >= ct) {
		VarRef vr(client);
		i = refTab.Append(1, &vr, 4);
		if(MakeRefByID(FOREVER, StdControl::NumRefs()+i, client) != REF_SUCCEED) {
			refTab.Delete(i, 1);
			TSTR s = GetString(IDS_DB_CIRCULAR_DEPENDENCY);
			MessageBox(hDlg, s, GetString(IDS_DB_CANT_ASSIGN), 
					MB_ICONEXCLAMATION | MB_SYSTEMMODAL | MB_OK);
			return FALSE;
		}
	}
	if(type == SCALAR_VAR) {
		if(varIndex >= sVars.Count()) {
			// should delete reference
			return FALSE;
		}
		sVars[varIndex].refID = i;
		sVars[varIndex].subNum = subNum;
		return TRUE;
	}
	else if(type == VECTOR_VAR) {
		if(varIndex >= vVars.Count()) {
			// should delete reference
			return FALSE;
		}
		vVars[varIndex].refID = i;
		vVars[varIndex].subNum = subNum;
		return TRUE;
	}
	return FALSE;
}

// This patch is here to convert number strings where the decimal point
// is a comma into numbers with a '.' for a decimal point.
// This is needed because the expression object only parses the '.' notation

TCHAR *UglyPatch(TCHAR *buf)
{
	TCHAR *cp = buf;
	while(*cp) {
		if( *cp == _T(','))
			*cp = _T('.');
		else if (*cp == _T(';'))
			*cp = _T(',');
		cp++;
	}
	return buf;
}

void ExprControl::Copy(Control *from)
{
	int i, j, ct, slot;
	TCHAR *cp;
	TCHAR buf[256];

	if (from->ClassID()==ClassID()) {
		ExprControl *ctrl = (ExprControl*)from;
		if(type != ctrl->type)
			goto dropOut;
		curPosVal = ctrl->curPosVal;
		curFloatVal = ctrl->curFloatVal;
		ivalid = ctrl->ivalid;
		range = ctrl->range;
		type = ctrl->type;
		desc = ctrl->desc;
//		sRegCt = vRegCt = 0;
		ct = ctrl->expr.getVarCount(SCALAR_VAR);
		for(i = 0; i < ct; i++) {
			cp = ctrl->expr.getVarName(SCALAR_VAR, i);
			updRegCt(slot = expr.defVar(SCALAR_VAR, cp), SCALAR_VAR);
			if(slot == -1)	// variable already defined
				continue;
			if(_tcscmp(cp, _T("T")) == 0)
				timeSlots[EXPR_TICKS] = slot;
			else if(_tcscmp(cp, _T("S")) == 0)
				timeSlots[EXPR_SECS] = slot;
			else if(_tcscmp(cp, _T("F")) == 0)
				timeSlots[EXPR_FRAMES] = slot;
			else if(_tcscmp(cp, _T("NT")) == 0)
				timeSlots[EXPR_NTIME] = slot;
			else {
				dfnVar(SCALAR_VAR, cp, slot, ctrl->getVarOffset(SCALAR_VAR, j=i-4));

				if(ctrl->sVars[j].refID < 0)
					assignScalarValue(slot, ctrl->sVars[j].val);
				else
					assignController(SCALAR_VAR, slot-4, ctrl->refTab[ctrl->sVars[j].refID].client, ctrl->sVars[j].subNum);
			}
		}
		ct = ctrl->expr.getVarCount(VECTOR_VAR);
		for(i = 0; i < ct; i++) {
			cp = ctrl->expr.getVarName(VECTOR_VAR, i);
			updRegCt(slot = expr.defVar(VECTOR_VAR, cp), VECTOR_VAR);
			dfnVar(VECTOR_VAR, cp, slot, ctrl->getVarOffset(VECTOR_VAR, i));

			if(ctrl->vVars[i].refID < 0)
				assignVectorValue(slot, ctrl->vVars[i].val);
			else
				assignController(VECTOR_VAR, slot, ctrl->refTab[ctrl->vVars[i].refID].client, ctrl->vVars[i].subNum);
		}
		expr.load(ctrl->expr.getExprStr());
	} 
	else {
dropOut:
		switch(type) {
		case EXPR_POS_CONTROL_CLASS_ID:
		case EXPR_P3_CONTROL_CLASS_ID:
			from->GetValue(0,&curPosVal,ivalid);
			_stprintf(buf, _T("[ %g; %g; %g ]"), curPosVal.x, curPosVal.y, curPosVal.z);
			expr.load(UglyPatch(buf));
			break;
		case EXPR_FLOAT_CONTROL_CLASS_ID:
			from->GetValue(0,&curFloatVal,ivalid);
			_stprintf(buf, _T("%g"), curFloatVal);
			expr.load(UglyPatch(buf));
			break;
		// should deal with SCALE and ROT types, but punt for now...
		}
	}
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
}


/*
class FindNodeDEP : public DependentEnumProc {
	public:		
		INode *node;		
		FindNodeDEP() {node=NULL;}
		int proc(ReferenceMaker *rmaker) {
			if (rmaker->SuperClassID()==BASENODE_CLASS_ID) {
				node = (INode*)rmaker;
				return 1;
			} else {
				return 0;
				}			
			}
	};
class FindPRSDEP : public DependentEnumProc {
	public:		
		Control *cont;
		FindPRSDEP() {cont=NULL;}
		int proc(ReferenceMaker *rmaker) {
			if (rmaker->SuperClassID()==CTRL_MATRIX3_CLASS_ID) {
				cont = (Control*)rmaker;
				return 1;
			} else {
				return 0;
				}			
			}
	};

void ExprControl::GetAbsoluteControlValue(
		Control *c,TimeValue t,Point3 *pt,Interval &iv)
	{
	FindPRSDEP dep1;
	FindNodeDEP dep2;
	c->EnumDependents(&dep1);
	if (dep1.cont) dep1.cont->EnumDependents(&dep2);
	if (dep2.node) {
		Matrix3 tm = dep2.node->GetNodeTM(t,&iv);
		*pt = tm.GetTrans();
	} else {
		c->GetValue(t, pt, iv);
		}
	}
*/

void ExprControl::GetAbsoluteControlValue(
		INode *node,TimeValue t,Point3 *pt,Interval &iv)
	{
	Matrix3 tm = node->GetNodeTM(t,&iv);
	*pt = tm.GetTrans();
	}


static BOOL OKPControl(SClass_ID cid) {
	if (cid==CTRL_POINT3_CLASS_ID || cid==CTRL_POS_CLASS_ID || CTRL_POSITION_CLASS_ID )
		return 1;
	return 0;
	}

void ExprControl::GetValueLocalTime(TimeValue t, void *val, Interval &valid, GetSetMethod method)
{
	int i;
	float *sregs = new float[sRegCt];
	Point3 *vregs = new Point3[vRegCt];
	ivalid.SetInstant(t);
	valid &= ivalid;

	// Limit to in range
	if (t<range.Start()) t = range.Start();
	if (t>range.End()) t = range.End();	
	
	switch(type) {
	case EXPR_POS_CONTROL_CLASS_ID:
	case EXPR_P3_CONTROL_CLASS_ID:
	case EXPR_SCALE_CONTROL_CLASS_ID:
		if(expr.getExprType() == VECTOR_EXPR) {
			sregs[timeSlots[EXPR_TICKS]] = (float)t;
			sregs[timeSlots[EXPR_SECS]] = (float)t/4800.0f;
			sregs[timeSlots[EXPR_FRAMES]] = (float)t/GetTicksPerFrame();
			sregs[timeSlots[EXPR_NTIME]] = (float)(t-range.Start()) / (float)(range.End()-range.Start());
			for(i = 0; i < sVars.Count(); i++) {
				if(sVars[i].refID < 0)
					sregs[sVars[i].regNum] = sVars[i].val;
				else {
					Control *c;
					Interval iv;
					c = (Control *)refTab[sVars[i].refID].client->SubAnim(sVars[i].subNum);
					if (c&&(c->SuperClassID()==CTRL_FLOAT_CLASS_ID))
						c->GetValue(t+sVars[i].offset, &sregs[sVars[i].regNum], iv);
				}
			}
			for(i = 0; i < vVars.Count(); i++) {
				if(vVars[i].refID < 0 || !refTab[vVars[i].refID].client)
					vregs[vVars[i].regNum] = vVars[i].val;
				else {
					Control *c;
					Interval iv;
/*
<<<<<<< SourceSafe version
					c = (Control *)refTab[vVars[i].refID].client->SubAnim(vVars[i].subNum);
					c->GetValue(t+vVars[i].offset, &vregs[vVars[i].regNum], iv);
======= 
*/
					// Rolf: check vVars[i].absPos						
					if (vVars[i].subNum<0) {
						GetAbsoluteControlValue(
							(INode*)refTab[vVars[i].refID].client,t+vVars[i].offset,&vregs[vVars[i].regNum], iv);
					} else {
						c = (Control *)refTab[vVars[i].refID].client->SubAnim(vVars[i].subNum);										 
						if (c&&OKPControl(c->SuperClassID()))
							c->GetValue(t+vVars[i].offset, &vregs[vVars[i].regNum], iv);
					}
//>>>>>>> Local version
				}
			}
			if(expr.eval((float *)&curPosVal, sRegCt, sregs, vRegCt, vregs))
				curPosVal = type == EXPR_SCALE_CONTROL_CLASS_ID ? Point3(1,1,1) : Point3(0,0,0);
		}
		else
			curPosVal = type == EXPR_SCALE_CONTROL_CLASS_ID ? Point3(1,1,1) : Point3(0,0,0);
		if (method==CTRL_RELATIVE) {
			if(type == EXPR_POS_CONTROL_CLASS_ID) {
		  		Matrix3 *mat = (Matrix3*)val;
//				mat->SetTrans(mat->GetTrans()+curPosVal);
				mat->PreTranslate(curPosVal);  // DS 9/23/96
			}
			else if(type == EXPR_SCALE_CONTROL_CLASS_ID) {
		  		Matrix3 *mat = (Matrix3*)val;
				mat->Scale(curPosVal);
			}
			else {
		  		Point3 *p = (Point3 *)val;
				*p += curPosVal;
			}
		} 
		else {
			if(type == EXPR_SCALE_CONTROL_CLASS_ID)
				*((ScaleValue *)val) = ScaleValue(curPosVal);
			else
				*((Point3*)val) = curPosVal;
		}
		break;
	case EXPR_FLOAT_CONTROL_CLASS_ID:
		if(expr.getExprType() == SCALAR_EXPR) {
			sregs[timeSlots[EXPR_TICKS]] = (float)t;
			sregs[timeSlots[EXPR_SECS]] = (float)t/4800.0f;
			sregs[timeSlots[EXPR_FRAMES]] = (float)t/GetTicksPerFrame();
			sregs[timeSlots[EXPR_NTIME]] = (float)(t-range.Start()) / (float)(range.End()-range.Start());
			for(i = 0; i < sVars.Count(); i++) {
				if(sVars[i].refID < 0)
					sregs[sVars[i].regNum] = sVars[i].val;
				else {
					Control *c;
					Interval iv;
					c = (Control *)refTab[sVars[i].refID].client->SubAnim(sVars[i].subNum);
					// DS 2/8/99 Added type checking to avoid crashes due to sub anim's moving around
					// when param block 2 code is added.
					if(c&&(c->SuperClassID()==CTRL_FLOAT_CLASS_ID)) {
						c->GetValue(t+sVars[i].offset, &sregs[sVars[i].regNum], iv);
						}
					else
						sregs[sVars[i].regNum] = sVars[i].val;
				}
			}
			for(i = 0; i < vVars.Count(); i++) {
				if(vVars[i].refID < 0 || !refTab[vVars[i].refID].client)
					vregs[vVars[i].regNum] = vVars[i].val;
				else {
					Control *c;
					Interval iv;
/*
<<<<<<< SourceSafe version
					c = (Control *)refTab[vVars[i].refID].client->SubAnim(vVars[i].subNum);
					c->GetValue(t+vVars[i].offset, &vregs[vVars[i].regNum], iv);
======= 
*/
					if (vVars[i].subNum<0) {
						GetAbsoluteControlValue(
							(INode*)refTab[vVars[i].refID].client,t+vVars[i].offset,&vregs[vVars[i].regNum], iv);
					} else {
						c = (Control *)refTab[vVars[i].refID].client->SubAnim(vVars[i].subNum);										 
						// DS 2/8/99 Added type checking to avoid crashes due to sub anim's moving around
						// when param block 2 code is added.
						if(c&&OKPControl(c->SuperClassID()))
							c->GetValue(t+vVars[i].offset, &vregs[vVars[i].regNum], iv);
					}
//>>>>>>> Local version
				}
			}
			if(expr.eval((float *)&curFloatVal, sRegCt, sregs, vRegCt, vregs))
				curFloatVal = 0.0f;
		}
		else
			curFloatVal = 0.0f;
		if (method==CTRL_RELATIVE) {
			*((float *)val) += curFloatVal;
		} 
		else {
			*((float *)val) = curFloatVal;
		}
	    break;
	}
	delete [] sregs;
	delete [] vregs;
}

void *ExprControl::CreateTempValue()
{
	switch(type) {
	case EXPR_POS_CONTROL_CLASS_ID:
	case EXPR_P3_CONTROL_CLASS_ID:
		return new Point3;
	case EXPR_SCALE_CONTROL_CLASS_ID:
		return new ScaleValue;
	case EXPR_FLOAT_CONTROL_CLASS_ID:
		return new float;
	}
	return NULL;
}

void ExprControl::DeleteTempValue(void *val)
{
	switch(type) {
	case EXPR_POS_CONTROL_CLASS_ID:
	case EXPR_P3_CONTROL_CLASS_ID:
		delete (Point3 *)val;
		break;
	case EXPR_SCALE_CONTROL_CLASS_ID:
		delete (ScaleValue *)val;
		break;
	case EXPR_FLOAT_CONTROL_CLASS_ID:
		delete (float *)val;
		break;
	}
}

void ExprControl::ApplyValue(void *val, void *delta)
{
	Matrix3 *mat;
	switch(type) {
	case EXPR_POS_CONTROL_CLASS_ID:
  		mat = (Matrix3*)val;
//		mat->SetTrans(mat->GetTrans()+*((Point3 *)delta));		
		mat->PreTranslate(*((Point3 *)delta));  //DS 9/23/96
		break;
	case EXPR_P3_CONTROL_CLASS_ID:
		*((Point3 *)val) += *((Point3 *)delta);
		break;
	case EXPR_FLOAT_CONTROL_CLASS_ID:
		*((float *)val) += *((float *)delta);
		break;
	case EXPR_SCALE_CONTROL_CLASS_ID:
		Matrix3 *mat = (Matrix3*)val;
		ScaleValue *s = (ScaleValue*)delta;
		ApplyScaling(*mat,*s);
		break;
	}
}

void ExprControl::MultiplyValue(void *val, float m)
{
	switch(type) {
	case EXPR_POS_CONTROL_CLASS_ID:
	case EXPR_P3_CONTROL_CLASS_ID:
		*((Point3 *)val) *= m;
		break;
	case EXPR_SCALE_CONTROL_CLASS_ID:
		*((ScaleValue *)val) *= m;
		break;
	case EXPR_FLOAT_CONTROL_CLASS_ID:
		*((float *)val) *= m;
		break;
	}
}

void ExprControl::Extrapolate(Interval range,TimeValue t,void *val,Interval &valid,int etype)
{
	if(type == EXPR_FLOAT_CONTROL_CLASS_ID) {
		float fval0, fval1, fval2, res;
		switch (etype) {
		case ORT_LINEAR:			
			if (t<range.Start()) {
				GetValueLocalTime(range.Start(),&fval0,valid);
				GetValueLocalTime(range.Start()+1,&fval1,valid);
				res = LinearExtrapolate(range.Start(),t,fval0,fval1,fval0);				
			} 
			else {
				GetValueLocalTime(range.End()-1,&fval0,valid);
				GetValueLocalTime(range.End(),&fval1,valid);
				res = LinearExtrapolate(range.End(),t,fval0,fval1,fval1);
			}
			break;

		case ORT_IDENTITY:
			if (t<range.Start()) {
				GetValueLocalTime(range.Start(),&fval0,valid);
				res = IdentityExtrapolate(range.Start(),t,fval0);
			} 
			else {
				GetValueLocalTime(range.End(),&fval0,valid);
				res = IdentityExtrapolate(range.End(),t,fval0);
			}
			break;

		case ORT_RELATIVE_REPEAT:
			GetValueLocalTime(range.Start(),&fval0,valid);
			GetValueLocalTime(range.End(),&fval1,valid);
			GetValueLocalTime(CycleTime(range,t),&fval2,valid);
			res = RepeatExtrapolate(range,t,fval0,fval1,fval2);			
			break;
		}
		valid.Set(t,t);
		*((float*)val) = res;
	}
	else if(type == EXPR_SCALE_CONTROL_CLASS_ID) {
		ScaleValue val0, val1, val2, res;
		switch (etype) {
		case ORT_LINEAR:			
			if (t<range.Start()) {
				GetValueLocalTime(range.Start(),&val0,valid);
				GetValueLocalTime(range.Start()+1,&val1,valid);
				res = LinearExtrapolate(range.Start(),t,val0,val1,val0);				
			} 
			else {
				GetValueLocalTime(range.End()-1,&val0,valid);
				GetValueLocalTime(range.End(),&val1,valid);
				res = LinearExtrapolate(range.End(),t,val0,val1,val1);
			}
			break;

		case ORT_IDENTITY:
			if (t<range.Start()) {
				GetValueLocalTime(range.Start(),&val0,valid);
				res = IdentityExtrapolate(range.Start(),t,val0);
			} 
			else {
				GetValueLocalTime(range.End(),&val0,valid);
				res = IdentityExtrapolate(range.End(),t,val0);
			}
			break;

		case ORT_RELATIVE_REPEAT:
			GetValueLocalTime(range.Start(),&val0,valid);
			GetValueLocalTime(range.End(),&val1,valid);
			GetValueLocalTime(CycleTime(range,t),&val2,valid);
			res = RepeatExtrapolate(range,t,val0,val1,val2);			
			break;
		}
		valid.Set(t,t);
		*((ScaleValue *)val) = res;
	}
	else {
		Point3 val0, val1, val2, res;
		switch (etype) {
		case ORT_LINEAR:			
			if (t<range.Start()) {
				GetValueLocalTime(range.Start(),&val0,valid);
				GetValueLocalTime(range.Start()+1,&val1,valid);
				res = LinearExtrapolate(range.Start(),t,val0,val1,val0);				
			} 
			else {
				GetValueLocalTime(range.End()-1,&val0,valid);
				GetValueLocalTime(range.End(),&val1,valid);
				res = LinearExtrapolate(range.End(),t,val0,val1,val1);
			}
			break;

		case ORT_IDENTITY:
			if (t<range.Start()) {
				GetValueLocalTime(range.Start(),&val0,valid);
				res = IdentityExtrapolate(range.Start(),t,val0);
			} 
			else {
				GetValueLocalTime(range.End(),&val0,valid);
				res = IdentityExtrapolate(range.End(),t,val0);
			}
			break;

		case ORT_RELATIVE_REPEAT:
			GetValueLocalTime(range.Start(),&val0,valid);
			GetValueLocalTime(range.End(),&val1,valid);
			GetValueLocalTime(CycleTime(range,t),&val2,valid);
			res = RepeatExtrapolate(range,t,val0,val1,val2);			
			break;
		}
		valid.Set(t,t);
		*((Point3 *)val) = res;
	}
}

ReferenceTarget* ExprControl::GetReference(int i) 
{
	int num;

	if(i < (num = StdControl::NumRefs()))
		return StdControl::GetReference(i);
	return (ReferenceTarget*)refTab[i-num].client;
}

void ExprControl::SetReference(int i, RefTargetHandle rtarg)
{
	int num;

	if(i < (num = StdControl::NumRefs()))
		StdControl::SetReference(i, rtarg);
	else
		refTab[i-num].client = rtarg;
}


// RB: When the last reference to an expression controller is
// deleted its parameter dialog needs to be closed.
void ExprControl::RefDeleted()
	{
	if (hParams) {
		int c=0;
		RefListItem  *ptr = GetRefList().first;
		while (ptr) {
			if (ptr->maker!=NULL) {
				if (ptr->maker->SuperClassID()) c++;
				}
			ptr = ptr->next;
			}	
		if (!c) DestroyWindow(hParams);	
		}
	}


class ExprDelTargetRestore : public RestoreObj {	
public:
	ExprControl *ec;
	SVarTab		sVarsUndo;
	VVarTab		vVarsUndo;
	VarRefTab	refTabUndo;
	SVarTab		sVarsRedo;
	VVarTab		vVarsRedo;
	VarRefTab	refTabRedo;
	ExprDelTargetRestore(ExprControl *ec) 
	{ 
		this->ec=ec;

		sVarsUndo.SetCount(ec->sVars.Count());
		for(int i = 0; i < ec->sVars.Count(); i++) {
			sVarsUndo[i].refID = ec->sVars[i].refID;
			sVarsUndo[i].subNum = ec->sVars[i].subNum;
		}

		vVarsUndo.SetCount(ec->vVars.Count());
		for(i = 0; i < ec->vVars.Count(); i++) {
			vVarsUndo[i].refID = ec->vVars[i].refID;
			vVarsUndo[i].subNum = ec->vVars[i].subNum;
		}

		refTabUndo.SetCount(ec->refTab.Count());
		for (i = 0; i < ec->refTab.Count(); i++)
			refTabUndo[i] = ec->refTab[i]; 
	}
	void Restore(int isUndo) 
	{
		// save the undo data in the redo tables
		sVarsRedo.SetCount(sVarsUndo.Count());
		for(int i = 0; i < sVarsUndo.Count(); i++) {
			sVarsRedo[i].refID = sVarsUndo[i].refID;
			sVarsRedo[i].subNum = sVarsUndo[i].subNum;
		}

		vVarsRedo.SetCount(vVarsUndo.Count());
		for(i = 0; i < vVarsUndo.Count(); i++) {
			vVarsRedo[i].refID = vVarsUndo[i].refID;
			vVarsRedo[i].subNum = vVarsUndo[i].subNum;
		}

		refTabRedo.SetCount(refTabUndo.Count());
		for (i = 0; i < refTabUndo.Count(); i++)
			refTabRedo[i] = refTabUndo[i]; 

		// put the undo data back into the controller
		for(i = 0; i < ec->sVars.Count(); i++) {
			ec->sVars[i].refID = sVarsUndo[i].refID;
			ec->sVars[i].subNum = sVarsUndo[i].subNum;
		}

		for(i = 0; i < ec->vVars.Count(); i++) {
			ec->vVars[i].refID = vVarsUndo[i].refID;
			ec->vVars[i].subNum = vVarsUndo[i].subNum;
		}

		for (i = 0; i < ec->refTab.Count(); i++)
			ec->refTab[i] = refTabUndo[i]; 

		ec->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}
	void Redo()
	{
		// put the redo data back into the controller
		for(int i = 0; i < ec->sVars.Count(); i++) {
			ec->sVars[i].refID = sVarsRedo[i].refID;
			ec->sVars[i].subNum = sVarsRedo[i].subNum;
		}

		for(i = 0; i < ec->vVars.Count(); i++) {
			ec->vVars[i].refID = vVarsRedo[i].refID;
			ec->vVars[i].subNum = vVarsRedo[i].subNum;
		}

		for (i = 0; i < ec->refTab.Count(); i++)
			ec->refTab[i] = refTabRedo[i]; 

		ec->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}
	void EndHold() { }
};

RefResult ExprControl::NotifyRefChanged(
		Interval iv, 
		RefTargetHandle hTarg, 
		PartID& partID, 
		RefMessage msg) 
{
	int i;

	switch (msg) {
	case REFMSG_GET_NODE_NAME:
		// RB 3/23/99: See comment at imp of getNodeName().
		if (blockGetNodeName) return REF_STOP;
		break;

	case REFMSG_CHANGE:
		ivalid.SetEmpty();
		if(edbg)
			InvalidateRect(edbg->hWnd, NULL, FALSE);	// this is to update the debug window, since
														// calling Update directly in a NotifyRefChanged
														// is a no-no    DB 2/23/99
			//edbg->Update();
		break;

// DS: 5-21-97 -- removed the REFMSG_REF_DELETED. This would cause problems
// when multiple Expr controls referenced the same thing.  When one of the expr
// controls got deleted this message was sent to all the others, setting their
// refs to NULL even though the target still had backptr to them. This would cause
// crashes later.
//	case REFMSG_REF_DELETED:		// this is for when visibility tracks are deleted

	case REFMSG_TARGET_DELETED: {	// this is for when referenced nodes are deleted
		if (theHold.Holding())
			theHold.Put(new ExprDelTargetRestore(this));
		// get rid of any scalar or vector variable references
		for(i = 0; i < sVars.Count(); i++)
			if(sVars[i].refID >= 0 && refTab[sVars[i].refID].client == hTarg)
				sVars[i].refID = -1;
		for(i = 0; i < vVars.Count(); i++)
			if(vVars[i].refID >= 0 && refTab[vVars[i].refID].client == hTarg)
				vVars[i].refID = -1;
		// then get rid of the reference itself
		for (i = 0; i < refTab.Count(); i++)
			if (refTab[i].client == hTarg) 
				refTab[i].client = NULL;
		break;	
		}
	// jbw 11.7.00 - sent by objects whose parameters have changed subanim order
	case REFMSG_SUBANIM_NUMBER_CHANGED:
		{
			// partID points to an Tab<ULONG> of renumberings, old num in LOWORD, new in HIWORD
			Tab<ULONG> renumberings = *reinterpret_cast<Tab<ULONG>*>(partID);  // take local copy so we can mark the ones used

			// update variables to point at new param subnum
			for(i = 0; i < sVars.Count(); i++)
				if(sVars[i].refID >= 0 && refTab[sVars[i].refID].client == hTarg)
				{
					for (int j = 0; j < renumberings.Count(); j++)
					{
						short old_subNum = LOWORD(renumberings[j]);
						short new_subNum = HIWORD(renumberings[j]);
						if (sVars[i].subNum == old_subNum)
						{
							// found old subnuym use, change to new subnum
							if (new_subNum < 0)
								sVars[i].refID = -1;
							else
								sVars[i].subNum = new_subNum;
							renumberings[i] = -1;  // mark renumbering as used
							break;
						}
					}
				}
			// same for vector vars
			for(i = 0; i < vVars.Count(); i++)
				if(vVars[i].refID >= 0 && refTab[vVars[i].refID].client == hTarg)
				{
					for (int j = 0; j < renumberings.Count(); j++)
					{
						short old_subNum = LOWORD(renumberings[j]);
						short new_subNum = HIWORD(renumberings[j]);
						if (vVars[i].subNum == old_subNum)
						{
							// found old subnuym use, change to new subnum
							if (new_subNum < 0)
								vVars[i].refID = -1;
							else
								vVars[i].subNum = new_subNum;
							renumberings[i] = -1;  // mark renumbering as used
							break;
						}
					}
				}
			break;
		}
	}
	return REF_SUCCEED;
}


class ExprControlRestore : public RestoreObj {	
public:
	ExprControl *ec;
	Interval undo, redo;
	ExprControlRestore(ExprControl *ec) { this->ec=ec; undo=ec->range; }
	void Restore(int isUndo) 
	{
		redo = ec->range;
		ec->range = undo;
		ec->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}
	void Redo() 
	{
		ec->range = redo;
		ec->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}
	void EndHold() { ec->ClearAFlag(A_HELD); }
};

void ExprControl::Hold()
{
	if (theHold.Holding() && !TestAFlag(A_HELD)) {
		theHold.Put(new ExprControlRestore(this));
		SetAFlag(A_HELD);
	}
}

void ExprControl::MapKeys( TimeMap *map, DWORD flags ) 
{
	Hold();
	range.Set(map->map(range.Start()), map->map(range.End())); 
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
}

void ExprControl::EditTimeRange(Interval range,DWORD flags)
{
	if(flags == EDITRANGE_LINKTOKEYS)
		return;
	this->range = range;
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

void ExprControl::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{	
}

void ExprControl::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
{
}

TSTR ExprControl::SvGetRelTip(IGraphObjectManager *gom, IGraphNode *gNodeTarger, int id, IGraphNode *gNodeMaker)
{
	return SvGetName(gom, gNodeMaker, false) + " -> " + gNodeTarger->GetAnim()->SvGetName(gom, gNodeTarger, false);
}

bool ExprControl::SvHandleRelDoubleClick(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker)
{
	return Control::SvHandleDoubleClick(gom, gNodeMaker);
}

SvGraphNodeReference ExprControl::SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags)
{
	SvGraphNodeReference nodeRef = Control::SvTraverseAnimGraph( gom, owner, id, flags );

	if( nodeRef.stat == SVT_PROCEED ) {
		for( int i=0; i<refTab.Count(); i++ ) {
			gom->AddRelationship( nodeRef.gNode, refTab[i].client, i, RELTYPE_CONTROLLER );
		}
	}

	return nodeRef;
}

// RB 3/23/99: To solve 75139 (the problem where a node name is found for variables that 
// are not associated with nodes such as globabl tracks) we need to block the propogation
// of this message through our reference to the client of the variable we're referencing.
// In the expression controller's imp of NotifyRefChanged() we're going to block the get
// node name message if the blockGetNodeName variable is TRUE.
void ExprControl::getNodeName(ReferenceTarget *client, TSTR &name)
	{
	blockGetNodeName = TRUE;
	if (client) client->NotifyDependents(FOREVER,(PartID)&name,REFMSG_GET_NODE_NAME);
	blockGetNodeName = FALSE;
	}

static void setupSpin(HWND hDlg, int spinID, int editID, float val)
{
	ISpinnerControl *spin;

	spin = GetISpinner(GetDlgItem(hDlg, spinID));
	spin->SetLimits( (float)-9e30, (float)9e30, FALSE );
	spin->SetScale( 1.0f );
	spin->LinkToEdit( GetDlgItem(hDlg, editID), EDITTYPE_FLOAT );
	spin->SetValue( val, FALSE );
	ReleaseISpinner( spin );
}

static float getSpinVal(HWND hDlg, int spinID)
{
	ISpinnerControl *spin = GetISpinner(GetDlgItem(hDlg, spinID));
	float res = spin->GetFVal();
	ReleaseISpinner(spin);
	return res;
}

static INT_PTR CALLBACK ScalarAsgnDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ExprControl *ec = (ExprControl *)GetWindowLongPtr(hDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		ec = (ExprControl *)lParam;
		SetWindowText(hDlg, ec->getVarName(SCALAR_VAR, ec->curIndex));
		setupSpin(hDlg, IDC_EXPR_CONST_SPIN, IDC_EXPR_CONST, ec->getScalarValue(ec->curIndex));
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
            goto done;
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			return FALSE;
		}
		break;

	case WM_CLOSE:
done:
		ec->assignScalarValue(ec->curIndex, getSpinVal(hDlg, IDC_EXPR_CONST_SPIN));
		EndDialog(hDlg, TRUE);
		return FALSE;
	}
	return FALSE;
}

static INT_PTR CALLBACK VectorAsgnDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Point3 pt;
	ExprControl *ec = (ExprControl *)GetWindowLongPtr(hDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		ec = (ExprControl *)lParam;
		SetWindowText(hDlg, ec->getVarName(VECTOR_VAR, ec->curIndex));
		pt = ec->getVectorValue(ec->curIndex);
		setupSpin(hDlg, IDC_VEC_X_SPIN, IDC_VEC_X, pt.x);
		setupSpin(hDlg, IDC_VEC_Y_SPIN, IDC_VEC_Y, pt.y);
		setupSpin(hDlg, IDC_VEC_Z_SPIN, IDC_VEC_Z, pt.z);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
            goto done;
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			return FALSE;
		}
		break;

	case WM_CLOSE:
done:
		pt.x = getSpinVal(hDlg, IDC_VEC_X_SPIN);
		pt.y = getSpinVal(hDlg, IDC_VEC_Y_SPIN);
		pt.z = getSpinVal(hDlg, IDC_VEC_Z_SPIN);
		ec->assignVectorValue(ec->curIndex, pt);
		EndDialog(hDlg, TRUE);
		return FALSE;
	}
	return FALSE;
}

static int isEmpty(TCHAR *s)
{
	int c;
	while(c = *s) {
		if(c != ' ' && c != '\t' && c != '\n' && c != '\r')
			return FALSE;
		s++;
	}
	return TRUE;
}

static TCHAR revertVal[4096];			// 001110  --prs.

static int updateExpr(HWND hDlg, ExprControl *ec, bool dismiss)
{
	TCHAR buf[4096];
	int exprType;
	GetDlgItemText(hDlg, IDC_EXPR_EDIT, buf, 4096);
	if(ec->expr.load(buf)) {
		if(isEmpty(buf)) {
			switch(ec->type) {
			case EXPR_POS_CONTROL_CLASS_ID:
			case EXPR_P3_CONTROL_CLASS_ID:
				_tcscpy(buf,_T("[ 0, 0, 0 ]"));
				break;
			case EXPR_SCALE_CONTROL_CLASS_ID:
				_tcscpy(buf, _T("[ 1, 1, 1 ]"));
				break;
			case EXPR_FLOAT_CONTROL_CLASS_ID:
				_tcscpy(buf, _T("0"));
				break;
			case EXPR_ROT_CONTROL_CLASS_ID:
				_tcscpy(buf, _T("{ [ 0, 0, 0 ], 0 }"));
				break;
			}
			SetDlgItemText(hDlg, IDC_EXPR_EDIT, buf);
			ec->expr.load(buf);
		}
		else if (dismiss) {		// this case 001110  --prs.
			TCHAR sbuf[4200];
			_stprintf(sbuf, GetString(IDS_PRS_EXPR_INVALID_REVERT), ec->expr.getProgressStr());
			int id = MessageBox(hDlg, sbuf, GetString(IDS_DB_EXPR_PARSE_ERROR),
						MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OKCANCEL);
			if (id == IDCANCEL)
				return FALSE;		// let user edit it again
			SetDlgItemText(hDlg, IDC_EXPR_EDIT, revertVal);	// else revert it
			ec->expr.load(revertVal);
			return TRUE;
		}
		else {
			MessageBox(hDlg, ec->expr.getProgressStr(), GetString(IDS_DB_EXPR_PARSE_ERROR), 
					MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
			return FALSE;
		}
	}
	exprType = ec->expr.getExprType();

	switch(ec->type) {
	case EXPR_SCALE_CONTROL_CLASS_ID:
	case EXPR_POS_CONTROL_CLASS_ID:
	case EXPR_P3_CONTROL_CLASS_ID:
		if(exprType != VECTOR_EXPR) {
			TSTR s = GetString(IDS_DB_NEED_VECTOR);
			MessageBox(hDlg, s, GetString(IDS_DB_EXPR_PARSE_ERROR), 
					MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
			return FALSE;
		}
		break;
	case EXPR_FLOAT_CONTROL_CLASS_ID:
		if(exprType != SCALAR_EXPR) {
			TSTR s = GetString(IDS_DB_NEED_SCALAR);
			MessageBox(hDlg, s, GetString(IDS_DB_EXPR_PARSE_ERROR), 
					MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
			return FALSE;
		}
		break;
	}

	ec->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	ec->ip->RedrawViews(ec->ip->GetTime());
	return TRUE;
}

static void updateVarList(HWND hDlg, ExprControl *ec)
{
	int i, ct;

	ct = ec->getVarCount(SCALAR_VAR);
	SendDlgItemMessage(hDlg, IDC_SCALAR_LIST, LB_RESETCONTENT, 0, 0);
	for(i = 0; i < ct; i++)
		SendDlgItemMessage(hDlg, IDC_SCALAR_LIST, LB_ADDSTRING, 0, (LPARAM)ec->getVarName(SCALAR_VAR, i));
	
	ct = ec->getVarCount(VECTOR_VAR);
	SendDlgItemMessage(hDlg, IDC_VECTOR_LIST, LB_RESETCONTENT, 0, 0);
	for(i = 0; i < ct; i++)
		SendDlgItemMessage(hDlg, IDC_VECTOR_LIST, LB_ADDSTRING, 0, (LPARAM)ec->getVarName(VECTOR_VAR, i));
}

#define XPR	_T(".xpr")

static void FixExprFileExt(OPENFILENAME &ofn) 
{
	int l = _tcslen(ofn.lpstrFile);
	int e = _tcslen(XPR);
	if (e>l || _tcsicmp(ofn.lpstrFile+l-e, XPR))
		_tcscat(ofn.lpstrFile,XPR);	
}

static BOOL BrowseForExprFilename(TCHAR *name, TCHAR *dir, int save) 
{
	int tried = 0;
	FilterList filterList;
	HWND hWnd = GetActiveWindow();
	static int filterIndex = 1;
    OPENFILENAME	ofn;
    TCHAR			fname[256];

	_tcscpy(fname, name);

	filterList.Append( GetString(IDS_DB_XPR_FILES) );
	filterList.Append( _T("*.xpr"));

    memset(&ofn, 0, sizeof(OPENFILENAME));

    ofn.lStructSize      = sizeof(OPENFILENAME);
    ofn.hwndOwner        = hWnd;
	ofn.hInstance        = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);

	ofn.nFilterIndex = filterIndex;
    ofn.lpstrFilter  = filterList;

    ofn.lpstrTitle   = GetString(save ? IDS_DB_SAVE_EXPR : IDS_DB_LOAD_EXPR);
    ofn.lpstrFile    = fname;
    ofn.nMaxFile     = sizeof(fname) / sizeof(TCHAR);      
	
	if(dir && dir[0])
	   	ofn.lpstrInitialDir = dir;
	else
	   	ofn.lpstrInitialDir = _T("");

	ofn.Flags = OFN_HIDEREADONLY | (save ? OFN_OVERWRITEPROMPT : (OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST));

	if(save) {
		if(!GetSaveFileName(&ofn))
			return FALSE;
	}
	else {
		if(!GetOpenFileName(&ofn))
			return FALSE;
	}
	FixExprFileExt(ofn); // add ".xpr" if absent
	if(dir) {
		_tcsncpy(dir, ofn.lpstrFile, ofn.nFileOffset-1);
		dir[ofn.nFileOffset-1] = _T('\0');
	}
	_tcscpy(name, ofn.lpstrFile);
	return TRUE;
}

static TCHAR exprDir[512];

static void SaveExpression(HWND hDlg, ExprControl *ec)
{
	TCHAR buf[4096];
	int i, ct;
	FILE *fp;

	if(!exprDir[0])
		_tcscpy(exprDir, ec->ip->GetDir(APP_EXPRESSION_DIR));
	buf[0] = _T('\0');
	if(BrowseForExprFilename(buf, exprDir, TRUE))	{
		if((fp = _tfopen(buf, _T("w"))) == NULL)
			return;
		GetDlgItemText(hDlg, IDC_EXPR_EDIT, buf, 4096);
		_ftprintf(fp, _T("%s\n"), buf);
		GetDlgItemText(hDlg, IDC_DESCRIPTION, buf, 4096);
		if(buf[0]) {
			TCHAR line[1024];
			TCHAR *cp = buf;
			int i;
			for(i = 0; *cp; i++, cp++) {
				line[i] = *cp;
				if(*cp == _T('\r')) {
					line[i] = 0;
					_ftprintf(fp, _T("d: %s\n"), line);
					line[0] = 0;
					cp++;	// skip over linefeed
					i = -1;	// reset i
				}
			}
			if(line[0]) {
				line[i] = 0;
				_ftprintf(fp, _T("d: %s\n"), line);
			}
		}
		ct = ec->getVarCount(SCALAR_VAR);
		for(i = 0; i < ct; i++)
			_ftprintf(fp, _T("s: %s:%d\n"), ec->getVarName(SCALAR_VAR, i), ec->sVars[i].offset);
		ct = ec->getVarCount(VECTOR_VAR);
		for(i = 0; i < ct; i++)
			_ftprintf(fp, _T("v: %s:%d %d\n"), ec->getVarName(VECTOR_VAR, i), ec->vVars[i].offset);
		fclose(fp);
	}
}

static void LoadExpression(HWND hDlg, ExprControl *ec)
{
	TCHAR buf[4096];
	TCHAR line[1024];
	TCHAR desc[4096];
	int slot, i, ct;
	int type;
	int offset;
	TCHAR *cp;
	FILE *fp;

	if(!exprDir[0])
		_tcscpy(exprDir, ec->ip->GetDir(APP_EXPRESSION_DIR));
	buf[0] = _T('\0');
	if(BrowseForExprFilename(buf, exprDir, FALSE))	{
		if((fp = _tfopen(buf, _T("r"))) == NULL)
			return;
		desc[0] = _T('\0');
		SetDlgItemText(hDlg, IDC_DESCRIPTION, _T(""));		// empty-out description
		ct = ec->sVars.Count();								// empty-out vars
		for(i = 0; i < ct; i++) {	// leave pre-defined scalars
			ec->expr.deleteVar(ec->sVars[0].name);
			delete ec->sVars[0].name;
			ec->sVars.Delete(0, 1);
		}
		ct = ec->vVars.Count();
		for(i = 0; i < ct; i++) {
			ec->expr.deleteVar(ec->vVars[0].name);
			delete ec->vVars[0].name;
			ec->vVars.Delete(0, 1);
		}
		buf[0] = _T('\0');
		while(_fgetts(line, 1024, fp) && (line[1] != _T(':')))
			_tcscat(buf, line);
		if(buf[_tcslen(buf) - 1] == _T('\n'))
			buf[_tcslen(buf) - 1] = _T('\0');
		SetDlgItemText(hDlg, IDC_EXPR_EDIT, buf);
		if(line[1] == _T(':')) {
			do {
				if(line[_tcslen(line) - 1] == _T('\n'))
					line[_tcslen(line) - 1] = _T('\0');
				switch(line[0]) {
				case _T('d'):
					if(desc[0])
						_tcscat(desc, _T("\r\n"));
					_tcscat(desc, line+3);
					break;
				case _T('s'):
				case _T('v'):
					type = line[0] == _T('s') ? SCALAR_VAR : VECTOR_VAR;
					offset = 0;
					cp = line+3;
					while(cp[offset] != _T('\n') && cp[offset] != ':')
						offset++;
					cp[offset++] = _T('\0');
					ec->updRegCt(slot = ec->expr.defVar(type, cp), type);
					if(slot >= 0)
						ec->dfnVar(type, cp, slot, _ttoi(cp+offset));
					break;
				}
			} while(_fgetts(line, 1024, fp));
		}
		SetDlgItemText(hDlg, IDC_DESCRIPTION, desc);
		updateVarList(hDlg, ec);
		fclose(fp);
	}
}


BOOL invalidName(TCHAR *buf)
{
	if(buf[0] >= '0' && buf[0] <= '9')
		return TRUE;
	TCHAR *cp = buf;
	while(*cp) {
		if(!_istalnum(*cp))
			return TRUE;
		cp++;
	}
	return FALSE;
}

static int fnListIDs[] = {
	IDS_DB_FN_SIN,
	IDS_DB_FN_NOISE,
	IDS_DB_FN_IF,
	IDS_DB_FN_VIF,
	IDS_DB_FN_MIN,
	IDS_DB_FN_MAX,
	IDS_DB_FN_POW,
	IDS_DB_FN_MOD,
	IDS_DB_FN_DEGTORAD,
	IDS_DB_FN_RADTODEG,
	IDS_DB_FN_COS,
	IDS_DB_FN_TAN,
	IDS_DB_FN_ASIN,
	IDS_DB_FN_ACOS,
	IDS_DB_FN_ATAN,
	IDS_DB_FN_SINH,
	IDS_DB_FN_COSH,
	IDS_DB_FN_TANH,
	IDS_DB_FN_LN,
	IDS_DB_FN_LOG,
	IDS_DB_FN_EXP,
	IDS_DB_FN_SQRT,
	IDS_DB_FN_ABS,
	IDS_DB_FN_CEIL,
	IDS_DB_FN_FLOOR,
	IDS_DB_FN_COMP,
	IDS_DB_FN_UNIT,
	IDS_DB_FN_LENGTH,
	IDS_DB_FN_PI,
	IDS_DB_FN_E,
	IDS_DB_FN_TPS
};

INT_PTR CALLBACK functionListProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i = 57;
    switch (message)  {
    case WM_INITDIALOG:
		CenterWindow(hDlg, GetWindow(hDlg, GW_OWNER));
		SendDlgItemMessage(hDlg, IDC_FUNC_LIST, LB_RESETCONTENT, 0, 0L);
		SendDlgItemMessage(hDlg, IDC_FUNC_LIST, LB_SETTABSTOPS, 1, (LPARAM)&i);
		for(i = 0; i < sizeof(fnListIDs) / sizeof(int); i++)
			SendDlgItemMessage(hDlg, IDC_FUNC_LIST, LB_ADDSTRING, 0, (LPARAM)GetString(fnListIDs[i]));
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDOK:
			goto done;
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
		default:
			return FALSE;
		}
		break;
		
	case WM_CLOSE:
done:
		EndDialog(hDlg, TRUE);
		break;
    }
    return FALSE;
}


static INT_PTR CALLBACK ExprParamsWndProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int type, slot, i, ct;
	Point3 pt;
	TCHAR buf[4096], buf2[100];
	TrackViewPick res;
	ExprControl *ec = (ExprControl *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	ISpinnerControl *spin;

	switch (msg) {
	case WM_INITDIALOG:
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		ec = (ExprControl *)lParam;
		if (ec->winX!=-1 && ec->winY!=-1)
			SetWindowPos(hDlg,NULL,ec->winX, ec->winY, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
		ec->hDlg = hDlg;
		spin = GetISpinner(GetDlgItem(hDlg, IDC_OFFSET_SPIN));
		spin->SetLimits( -1000000, 1000000, FALSE );
		spin->SetScale( 1.0f );
		spin->LinkToEdit( GetDlgItem(hDlg, IDC_OFFSET), EDITTYPE_INT );
		spin->SetValue( 0, FALSE );
		ReleaseISpinner( spin );
		SendDlgItemMessage(hDlg, IDC_EXPR_EDIT, WM_SETFONT, (WPARAM)ec->hFixedFont, TRUE);
		CheckRadioButton(hDlg, IDC_SCALAR_RB, IDC_VECTOR_RB, IDC_SCALAR_RB);
		updateVarList(hDlg, ec);
		EnableWindow(GetDlgItem(hDlg, IDC_ASGN_CONST), 0);
		EnableWindow(GetDlgItem(hDlg, IDC_ASGN_CNTRL), 0);
		SetDlgItemText(hDlg, IDC_EXPR_EDIT, ec->expr.getExprStr());
		GetDlgItemText(hDlg, IDC_EXPR_EDIT, revertVal, 4096);	// 001110  --prs.
		SetDlgItemText(hDlg, IDC_DESCRIPTION, ec->desc);
		SetFocus(GetDlgItem(hDlg, IDC_EXPR_EDIT));
		GetDlgItemText(hDlg, IDC_EXPLAIN_NOTATION, buf, sizeof(buf));
		if(buf[0]) {
			WINDOWPLACEMENT wp;
			wp.length = sizeof(WINDOWPLACEMENT);
			GetWindowPlacement(GetDlgItem(hDlg, IDC_EXPR_EDIT), &wp);
			wp.rcNormalPosition.bottom -= 12;
			SetWindowPlacement(GetDlgItem(hDlg, IDC_EXPR_EDIT), &wp);
			ShowWindow(GetDlgItem(hDlg, IDC_EXPLAIN_NOTATION), SW_SHOW);
		}
			
		return FALSE;

	case CC_SPINNER_CHANGE:
		switch ( LOWORD(wParam) ) {
		case IDC_OFFSET_SPIN:
			GetDlgItemText(hDlg, IDC_VAR_NAME, buf, 256);
			if(buf[0]) {
				ct = ec->sVars.Count();
				for(i = 0; i < ct; i++) {
					if(_tcscmp(buf, ec->sVars[i].name) == 0) {
						spin = GetISpinner(GetDlgItem(hDlg, IDC_OFFSET_SPIN));
						ec->sVars[i].offset = spin->GetIVal();
						ReleaseISpinner(spin);
						break;
					}
				}
				if(i < ct)
					break;
				ct = ec->vVars.Count();
				for(i = 0; i < ct; i++) {
					if(_tcscmp(buf, ec->vVars[i].name) == 0) {
						spin = GetISpinner(GetDlgItem(hDlg, IDC_OFFSET_SPIN));
						ec->vVars[i].offset = spin->GetIVal();
						ReleaseISpinner(spin);
						break;
					}
				}
			}
			break;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_EXPR_DEBUG:
			if(ec->edbg)
				ShowWindow(ec->edbg->hWnd, SW_SHOWNORMAL);
			else
				ec->edbg = new ExprDebug(hDlg, ec);
			updateExpr(hDlg, ec, false);
			if(ec->edbg)			// 001019  --prs.
				ec->edbg->Update();
			break;
		case IDC_FUNCTIONS:
			DialogBox(hInstance, MAKEINTRESOURCE(IDD_FUNC_LIST), hDlg, functionListProc);
			break;
		case IDC_CREATE_VAR:
			GetDlgItemText(hDlg, IDC_VAR_NAME, buf, 256);
			if(buf[0]) {
				if(invalidName(buf)) {
					_tcscpy(buf2, GetString(IDS_DB_BAD_NAME));
					MessageBox(hDlg, buf2, GetString(IDS_DB_CANT_CREATE_VAR), 
							MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
					break;
				}
				type = IsDlgButtonChecked(hDlg, IDC_SCALAR_RB) ? SCALAR_VAR : VECTOR_VAR;
				ec->updRegCt(slot = ec->expr.defVar(type, buf), type);
				spin = GetISpinner(GetDlgItem(hDlg, IDC_OFFSET_SPIN));
				i = spin->GetIVal();
				ReleaseISpinner(spin);
				if(slot >= 0)
					ec->dfnVar(type, buf, slot, i);
				else {
					_tcscpy(buf2, GetString(IDS_DB_DUPNAME));
					MessageBox(hDlg, buf2, GetString(IDS_DB_CANT_CREATE_VAR), 
							MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
				}
			}
			updateVarList(hDlg, ec);
			slot = type == SCALAR_VAR ? IDC_SCALAR_LIST : IDC_VECTOR_LIST;
			ct = SendDlgItemMessage(hDlg, slot, LB_GETCOUNT, 0, 0);
			for(i = 0; i < ct; i++) {
				SendDlgItemMessage(hDlg, slot, LB_GETTEXT, i, (LPARAM)buf2);
				if(_tcscmp(buf, buf2) == 0) {
					SendDlgItemMessage(hDlg, slot, LB_SETCURSEL, i, 0);
					break;
				}
			}
			SendMessage(hDlg, WM_COMMAND, slot | (LBN_SELCHANGE << 16), 0); 
			break;
		case IDC_DELETE_VAR:
			GetDlgItemText(hDlg, IDC_VAR_NAME, buf, 256);
			if(buf[0]) {
				ct = ec->sVars.Count();
				for(i = 0; i < ct; i++) {
					if(_tcscmp(buf, ec->sVars[i].name) == 0) {
						ec->expr.deleteVar(buf);
						delete ec->sVars[i].name;
						ec->sVars.Delete(i, 1);
						SetDlgItemText(hDlg, IDC_VAR_NAME, _T(""));
						break;
					}
				}
				if(i < ct) {
					updateVarList(hDlg, ec);
					break;
				}

				ct = ec->vVars.Count();
				for(i = 0; i < ct; i++) {
					if(_tcscmp(buf, ec->vVars[i].name) == 0) {
						ec->expr.deleteVar(buf);
						delete ec->vVars[i].name;
						ec->vVars.Delete(i, 1);
						SetDlgItemText(hDlg, IDC_VAR_NAME, _T(""));
						break;
					}
				}
				if(i < ct) {
					updateVarList(hDlg, ec);
					break;
				}
				_stprintf(buf2, GetString(IDS_DB_CANTDELETE), buf);
				MessageBox(hDlg, buf2, GetString(IDS_DB_EXPRCNTL), 
						MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
			}
			else {
				_tcscpy(buf2, GetString(IDS_DB_NOTHINGDEL));
				MessageBox(hDlg, buf2, GetString(IDS_DB_EXPRCNTL), 
						MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
			}
			break;
		case IDC_EXPR_EVAL:
			updateExpr(hDlg, ec, false);		// 3rd arg added 001110  --prs.
			if(ec->edbg)
				ec->edbg->Update();
			break;
		case IDC_VAR_NAME:
		case IDC_EXPR_EDIT:
		case IDC_DESCRIPTION:
			if(HIWORD(wParam) == EN_SETFOCUS)
				DisableAccelerators();
			else if(HIWORD(wParam) == EN_KILLFOCUS) {
				EnableAccelerators();
				if (LOWORD(wParam) == IDC_DESCRIPTION) {
					GetDlgItemText(hDlg, IDC_DESCRIPTION, buf, 4096);
					ec->desc = buf;
				}					
			}
			break;
		case IDC_ASGN_CNTRL:
			ct = SendDlgItemMessage(hDlg, IDC_SCALAR_LIST, LB_GETCOUNT, 0, 0);
			for(i = 0; i < ct; i++)
				if(SendDlgItemMessage(hDlg, IDC_SCALAR_LIST, LB_GETSEL, i, 0))
					break;
			if(i < ct) {
				SendDlgItemMessage(hDlg, IDC_SCALAR_LIST, LB_GETTEXT, i, (LPARAM)buf);
				ct = ec->getVarCount(SCALAR_VAR);
				for(i = 0; i < ct; i++) {
					if(_tcscmp(buf, ec->getVarName(SCALAR_VAR, i)) == 0)
						break;
				}
				FloatFilter ff;
				if(ec->ip->TrackViewPickDlg(hDlg, &res, &ff)) {
					assert(res.anim->SuperClassID() == CTRL_FLOAT_CLASS_ID);
					ec->assignController(SCALAR_VAR, i, res.client, res.subNum);
					SendMessage(hDlg, WM_COMMAND, IDC_SCALAR_LIST | (LBN_SELCHANGE << 16), 0); 
				}
				break;
			}
			ct = SendDlgItemMessage(hDlg, IDC_VECTOR_LIST, LB_GETCOUNT, 0, 0);
			for(i = 0; i < ct; i++)
				if(SendDlgItemMessage(hDlg, IDC_VECTOR_LIST, LB_GETSEL, i, 0))
					break;
			if(i < ct) {
				slot = IDC_VECTOR_LIST;
				SendDlgItemMessage(hDlg, IDC_VECTOR_LIST, LB_GETTEXT, i, (LPARAM)buf);
				ct = ec->getVarCount(VECTOR_VAR);
				for(i = 0; i < ct; i++) {
					if(_tcscmp(buf, ec->getVarName(VECTOR_VAR, i)) == 0)
						break;
				}
				VectorFilter vf;
				if(ec->ip->TrackViewPickDlg(hDlg, &res, &vf)) {
					assert(res.anim->SuperClassID() == CTRL_POSITION_CLASS_ID ||
						   res.anim->SuperClassID() == CTRL_POINT3_CLASS_ID ||
						   res.anim->SuperClassID() == BASENODE_CLASS_ID);
					
					if (res.anim->SuperClassID()==BASENODE_CLASS_ID) {
						ec->assignController(VECTOR_VAR, i, res.anim, -1);
					} else {
						ec->assignController(VECTOR_VAR, i, res.client, res.subNum);
						}
					SendMessage(hDlg, WM_COMMAND, IDC_VECTOR_LIST | (LBN_SELCHANGE << 16), 0); 
				}
				break;
			}
			break;
		case IDC_ASGN_CONST:
			ct = SendDlgItemMessage(hDlg, IDC_SCALAR_LIST, LB_GETCOUNT, 0, 0);
			for(i = 0; i < ct; i++)
				if(SendDlgItemMessage(hDlg, IDC_SCALAR_LIST, LB_GETSEL, i, 0))
					break;
			if(i < ct) {
				SendDlgItemMessage(hDlg, IDC_SCALAR_LIST, LB_GETTEXT, i, (LPARAM)buf);
				ct = ec->getVarCount(SCALAR_VAR);
				for(i = 0; i < ct; i++) {
					if(_tcscmp(buf, ec->getVarName(SCALAR_VAR, i)) == 0)
						break;
				}
				ec->curIndex = i;
				DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_SCALAR_ASGN),
							hDlg, ScalarAsgnDlgProc, (LPARAM)ec);
				SendMessage(hDlg, WM_COMMAND, IDC_SCALAR_LIST | (LBN_SELCHANGE << 16), 0); 
				break;
			}
			ct = SendDlgItemMessage(hDlg, IDC_VECTOR_LIST, LB_GETCOUNT, 0, 0);
			for(i = 0; i < ct; i++)
				if(SendDlgItemMessage(hDlg, IDC_VECTOR_LIST, LB_GETSEL, i, 0))
					break;
			if(i < ct) {
				slot = IDC_VECTOR_LIST;
				SendDlgItemMessage(hDlg, IDC_VECTOR_LIST, LB_GETTEXT, i, (LPARAM)buf);
				ct = ec->getVarCount(VECTOR_VAR);
				for(i = 0; i < ct; i++) {
					if(_tcscmp(buf, ec->getVarName(VECTOR_VAR, i)) == 0)
						break;
				}
				ec->curIndex = i;
				DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_VECTOR_ASGN),
							hDlg, VectorAsgnDlgProc, (LPARAM)ec);
				SendMessage(hDlg, WM_COMMAND, IDC_VECTOR_LIST | (LBN_SELCHANGE << 16), 0); 
				break;
			}
			break;
		case IDC_SCALAR_LIST:
			if(HIWORD(wParam) == LBN_SELCHANGE) {
				SendDlgItemMessage(hDlg, IDC_VECTOR_LIST, LB_SETCURSEL, (WPARAM)-1, (LPARAM)0);
				ct = SendDlgItemMessage(hDlg, IDC_SCALAR_LIST, LB_GETCOUNT, 0, 0);
				for(i = 0; i < ct; i++)
					if(SendDlgItemMessage(hDlg, IDC_SCALAR_LIST, LB_GETSEL, i, 0))
						break;
				if(i < ct) {
					SendDlgItemMessage(hDlg, IDC_SCALAR_LIST, LB_GETTEXT, i, (LPARAM)buf);
					ct = ec->getVarCount(SCALAR_VAR);
					for(i = 0; i < ct; i++) 
						if(_tcscmp(buf, ec->getVarName(SCALAR_VAR, i)) == 0)
							break;
					SetDlgItemText(hDlg, IDC_VAR_NAME, buf);
					CheckRadioButton(hDlg, IDC_SCALAR_RB, IDC_VECTOR_RB, IDC_SCALAR_RB);
					spin = GetISpinner(GetDlgItem(hDlg, IDC_OFFSET_SPIN));
					spin->SetValue(ec->sVars[i].offset, FALSE);
					ReleaseISpinner(spin);
					if(ec->sVars[i].refID < 0) {
						_stprintf(buf, "Constant: %g", ec->getScalarValue(i));
						UglyPatch(buf);
					}
					else {
						slot = ec->sVars[i].refID;
#if 0	// displays unique controller id
						_stprintf(buf, "Controller: %s {%08x}:%d", 
									ec->refTab[slot].client->SubAnimName(ec->sVars[i].subNum), 
									ec->refTab[slot].client, ec->sVars[i].subNum);
#else	// displays more user-friendly (but possibly wrong) text
						TSTR nname;
						// RB 3/23/99: See comment at imp of getNodeName().
						//ec->refTab[slot].client->NotifyDependents(FOREVER,(PartID)&nname,REFMSG_GET_NODE_NAME);
						ec->getNodeName(ec->refTab[slot].client,nname);
						TSTR pname;
						if (nname.Length())
							pname = nname + TSTR(_T("\\")) + ec->refTab[slot].client->SubAnimName(ec->sVars[i].subNum);
						else 
							pname = ec->refTab[slot].client->SubAnimName(ec->sVars[i].subNum);
						_tcscpy(buf, pname);
#endif
					}
					SetDlgItemText(hDlg, IDC_CUR_ASGN, buf);
				}
				EnableWindow(GetDlgItem(hDlg, IDC_ASGN_CONST), 1);
				EnableWindow(GetDlgItem(hDlg, IDC_ASGN_CNTRL), 1);
			}
			break;
		case IDC_VECTOR_LIST:
			if(HIWORD(wParam) == LBN_SELCHANGE) {
				SendDlgItemMessage(hDlg, IDC_SCALAR_LIST, LB_SETCURSEL, (WPARAM)-1, (LPARAM)0);
				ct = SendDlgItemMessage(hDlg, IDC_VECTOR_LIST, LB_GETCOUNT, 0, 0);
				for(i = 0; i < ct; i++)
					if(SendDlgItemMessage(hDlg, IDC_VECTOR_LIST, LB_GETSEL, i, 0))
						break;
				if(i < ct) {
					SendDlgItemMessage(hDlg, IDC_VECTOR_LIST, LB_GETTEXT, i, (LPARAM)buf);
					ct = ec->getVarCount(VECTOR_VAR);
					for(i = 0; i < ct; i++) 
						if(_tcscmp(buf, ec->getVarName(VECTOR_VAR, i)) == 0)
							break;
					SetDlgItemText(hDlg, IDC_VAR_NAME, buf);
					CheckRadioButton(hDlg, IDC_SCALAR_RB, IDC_VECTOR_RB, IDC_VECTOR_RB);
					spin = GetISpinner(GetDlgItem(hDlg, IDC_OFFSET_SPIN));
					spin->SetValue(ec->vVars[i].offset, FALSE);
					ReleaseISpinner(spin);
					if(ec->vVars[i].refID < 0 || !ec->refTab[ec->vVars[i].refID].client) {
						pt = ec->getVectorValue(i);
						_stprintf(buf, "Constant: [%g; %g; %g]", pt.x, pt.y, pt.z);
						UglyPatch(buf);
					}
					else {
						slot = ec->vVars[i].refID;
#if 0	// displays unique controller id
						_stprintf(buf, "Controller: %s {%08x}:%d", 
									ec->refTab[slot].client->SubAnimName(ec->vVars[i].subNum), 
									ec->refTab[slot].client, ec->vVars[i].subNum);
#else	// displays more user-friendly (but possibly wrong) text
						TSTR pname;
						if(ec->vVars[i].subNum == -1)	// special case: we're referencing a node
							pname = ((INode *)ec->refTab[slot].client)->GetName();
						else {
							TSTR nname;
							// RB 3/23/99: See comment at imp of getNodeName().
							//ec->refTab[slot].client->NotifyDependents(FOREVER,(PartID)&nname,REFMSG_GET_NODE_NAME);
							ec->getNodeName(ec->refTab[slot].client,nname);
							if (nname.Length())
								pname = nname + TSTR(_T("\\")) + ec->refTab[slot].client->SubAnimName(ec->vVars[i].subNum);
							else 
								pname = ec->refTab[slot].client->SubAnimName(ec->vVars[i].subNum);
						}
						_tcscpy(buf, pname);
#endif
					}
					SetDlgItemText(hDlg, IDC_CUR_ASGN, buf);
				}
				EnableWindow(GetDlgItem(hDlg, IDC_ASGN_CONST), 1);
				EnableWindow(GetDlgItem(hDlg, IDC_ASGN_CNTRL), 1);
			}
			break;
		case IDOK:
		case IDCANCEL:
			if(updateExpr(hDlg, ec,true))		// 3rd argument added 001110  --prs.
				DestroyWindow(hDlg);
			break;

		case IDC_SAVE:
			SaveExpression(hDlg, ec);
			break;

		case IDC_LOAD:
			LoadExpression(hDlg, ec);
//			updateExpr(hDlg, ec);
			break;
		}
		break;

	case WM_DESTROY: {
			Rect rect;
			GetWindowRect(hDlg,&rect);
			ec->winX = rect.left;
			ec->winY = rect.top;
		}
		ec->hParams = NULL;
		break;

	default:
		return 0;			
	}
	return 1;
}


void ExprControl::EditTrackParams(
			TimeValue t,	// The horizontal position of where the user right clicked.
			ParamDimensionBase *dim,
			TCHAR *pname,
			HWND hParent,
			IObjParam *ip,
			DWORD flags)
{
	if (!hParams) {
		this->ip = ip;
		hParams = CreateDialogParam(
			hInstance,
			MAKEINTRESOURCE(IDD_EXPRPARAMS),
			hParent,
			ExprParamsWndProc,
			(LPARAM)this);
		TSTR title = TSTR(GetString(IDS_RB_EXPRESSIONCONTROLTITLE)) + TSTR(pname);
		SetWindowText(hParams,title);
	} 
	else {
		SetActiveWindow(hParams);
	}
}

#define EXPR_STR_CHUNK		0x5000
#define EXPR_RANGE_CHUNK	0x5001
#define EXPR_REFTAB_SIZE	0x5002
#define EXPR_SVAR_TABSIZE	0x5003
#define EXPR_VVAR_TABSIZE	0x5005
#define EXPR_VAR_NAME		0x5006
#define EXPR_VAR_REFID		0x5007
#define EXPR_VAR_FLOAT		0x5008
#define EXPR_VAR_POINT3		0x5009
#define EXPR_VAR_SUBNUM		0x500a
#define EXPR_DESCRIPTION	0x500b
#define EXPR_REFTAB_REFCT	0x500c
#define EXPR_VAR_OFFSET		0x500d
#define EXPR_SVAR_ENTRY0	0x6000
#define EXPR_SVAR_ENTRYN	0x6fff
#define EXPR_VVAR_ENTRY0	0x7000
#define EXPR_VVAR_ENTRYN	0x7fff

IOResult ExprControl::Save(ISave *isave)
{
	ULONG 	nb;
	int		i, ct, intVar;

	Control::Save(isave); // RB: this will handle saving ORTs

	if(desc.Length()) {
	 	isave->BeginChunk(EXPR_DESCRIPTION);
		isave->WriteCString(desc);
	 	isave->EndChunk();
	}

 	isave->BeginChunk(EXPR_RANGE_CHUNK);
	isave->Write(&range, sizeof(range), &nb);
 	isave->EndChunk();

 	isave->BeginChunk(EXPR_REFTAB_SIZE);
	intVar = refTab.Count();
	isave->Write(&intVar, sizeof(intVar), &nb);
 	isave->EndChunk();

	ct = refTab.Count();
	for(i = 0; i < ct; i++) {
	 	isave->BeginChunk(EXPR_REFTAB_REFCT);
		isave->Write(&i, sizeof(int), &nb);
		isave->Write(&refTab[i].refCt, sizeof(int), &nb);
		isave->EndChunk();
	}


 	isave->BeginChunk(EXPR_SVAR_TABSIZE);
	intVar = sVars.Count();
	isave->Write(&intVar, sizeof(intVar), &nb);
 	isave->EndChunk();

	ct = sVars.Count();
	for(i = 0; i < ct; i++) {
	 	isave->BeginChunk(EXPR_SVAR_ENTRY0+i);
	 	 isave->BeginChunk(EXPR_VAR_NAME);
		 isave->WriteCString(sVars[i].name);
 		 isave->EndChunk();
	 	 isave->BeginChunk(EXPR_VAR_REFID);
		 isave->Write(&sVars[i].refID, sizeof(int), &nb);
 		 isave->EndChunk();
	 	 isave->BeginChunk(EXPR_VAR_FLOAT);
		 isave->Write(&sVars[i].val, sizeof(float), &nb);
 		 isave->EndChunk();
	 	 isave->BeginChunk(EXPR_VAR_SUBNUM);
		 isave->Write(&sVars[i].subNum, sizeof(int), &nb);
 		 isave->EndChunk();
	 	 isave->BeginChunk(EXPR_VAR_OFFSET);
		 isave->Write(&sVars[i].offset, sizeof(int), &nb);
 		 isave->EndChunk();
	 	isave->EndChunk();
	}

 	isave->BeginChunk(EXPR_VVAR_TABSIZE);
	intVar = vVars.Count();
	isave->Write(&intVar, sizeof(intVar), &nb);
 	isave->EndChunk();

	ct = vVars.Count();
	for(i = 0; i < ct; i++) {
	 	isave->BeginChunk(EXPR_VVAR_ENTRY0+i);
	 	 isave->BeginChunk(EXPR_VAR_NAME);
		 isave->WriteCString(vVars[i].name);
 		 isave->EndChunk();
	 	 isave->BeginChunk(EXPR_VAR_REFID);
		 isave->Write(&vVars[i].refID, sizeof(int), &nb);
 		 isave->EndChunk();
	 	 isave->BeginChunk(EXPR_VAR_POINT3);
		 isave->Write(&vVars[i].val, sizeof(Point3), &nb);
 		 isave->EndChunk();
	 	 isave->BeginChunk(EXPR_VAR_SUBNUM);
		 isave->Write(&vVars[i].subNum, sizeof(int), &nb);
 		 isave->EndChunk();
	 	 isave->BeginChunk(EXPR_VAR_OFFSET);
		 isave->Write(&vVars[i].offset, sizeof(int), &nb);
 		 isave->EndChunk();
	 	isave->EndChunk();
	}

 	isave->BeginChunk(EXPR_STR_CHUNK);
	isave->WriteCString(expr.getExprStr());
 	isave->EndChunk();
	
	return IO_OK;
}

// RB 3/19/99: Check for refs to path controller. These must be refs to the percent track made from
// MAX2.5 or earlier. Convert to paramblock 2 refs.
class CheckForPathContRefsPLCB : public PostLoadCallback {
	public:
		ExprControl *cont;
		CheckForPathContRefsPLCB(ExprControl *c) {cont = c;}
		void proc(ILoad *iload) {
			for (int i=0; i<cont->refTab.Count(); i++) {
				if (!cont->refTab[i].client) continue;
				if (cont->refTab[i].client->SuperClassID()==CTRL_POSITION_CLASS_ID &&
					cont->refTab[i].client->ClassID()==Class_ID(PATH_CONTROL_CLASS_ID,0)) {
										
					cont->ReplaceReference(i+cont->StdControl::NumRefs(),
						cont->refTab[i].client->GetReference(PATHPOS_PBLOCK_REF));					
					}
				}
			delete this;
			}
	};

IOResult ExprControl::Load(ILoad *iload)
{
	ULONG 	nb;
	TCHAR	*cp;
	int		id, i, varIndex, intVar;
	IOResult res;
	VarRef	dummyVarRef;

	Control::Load(iload); // RB: this will handle loading ORTs

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (id = iload->CurChunkID()) {
		case EXPR_STR_CHUNK:
			iload->ReadCStringChunk(&cp);
			expr.load(cp);
			break;
		case EXPR_DESCRIPTION:
			iload->ReadCStringChunk(&cp);
			desc = cp;
			break;
		case EXPR_RANGE_CHUNK:
			iload->Read(&range, sizeof(range), &nb);
			break;
		case EXPR_REFTAB_SIZE:
			iload->Read(&intVar, sizeof(intVar), &nb);
			refTab.SetCount(intVar);
			for(i = 0; i < intVar; i++) {
				refTab[i].refCt = 0;
				refTab[i].client = NULL;
			}
			break;
		case EXPR_REFTAB_REFCT:
			iload->Read(&intVar, sizeof(int), &nb);
			if(intVar < refTab.Count())		// this should always be true!!!!
				iload->Read(&refTab[intVar].refCt, sizeof(int), &nb);
			break;
		case EXPR_SVAR_TABSIZE:
			iload->Read(&intVar, sizeof(intVar), &nb);
			sVars.SetCount(intVar);
			for(i = 0; i < intVar; i++)
				memset(&sVars[i], 0, sizeof(SVar));
			break;
		case EXPR_VVAR_TABSIZE:
			iload->Read(&intVar, sizeof(intVar), &nb);
			vVars.SetCount(intVar);
			for(i = 0; i < intVar; i++)
				memset(&vVars[i], 0, sizeof(VVar));
			break;
		}	
		if(id >= EXPR_SVAR_ENTRY0 && id <= EXPR_SVAR_ENTRYN) {
			varIndex = id - EXPR_SVAR_ENTRY0;
			assert(varIndex < sVars.Count());
			while (IO_OK == iload->OpenChunk()) {
				switch (iload->CurChunkID()) {
				case EXPR_VAR_NAME:
					iload->ReadCStringChunk(&cp);
					sVars[varIndex].name = cp;
					break;
				case EXPR_VAR_REFID:
					iload->Read(&sVars[varIndex].refID, sizeof(int), &nb);
					break;
				case EXPR_VAR_SUBNUM:
					iload->Read(&sVars[varIndex].subNum, sizeof(int), &nb);
					break;
				case EXPR_VAR_OFFSET:
					iload->Read(&sVars[varIndex].offset, sizeof(int), &nb);
					break;
				case EXPR_VAR_FLOAT:
					iload->Read(&sVars[varIndex].val, sizeof(float), &nb);
					break;
				}	
				iload->CloseChunk();
			}
			updRegCt(intVar = expr.defVar(SCALAR_VAR, sVars[varIndex].name), SCALAR_VAR);
			sVars[varIndex].regNum = intVar;
		}
		else if(id >= EXPR_VVAR_ENTRY0 && id <= EXPR_VVAR_ENTRYN) {
			varIndex = id - EXPR_VVAR_ENTRY0;
			assert(varIndex < vVars.Count());
			while (IO_OK == iload->OpenChunk()) {
				switch (iload->CurChunkID()) {
				case EXPR_VAR_NAME:
					iload->ReadCStringChunk(&cp);
					vVars[varIndex].name = cp;
					break;
				case EXPR_VAR_REFID:
					iload->Read(&vVars[varIndex].refID, sizeof(int), &nb);
					break;
				case EXPR_VAR_SUBNUM:
					iload->Read(&vVars[varIndex].subNum, sizeof(int), &nb);
					break;
				case EXPR_VAR_OFFSET:
					iload->Read(&vVars[varIndex].offset, sizeof(int), &nb);
					break;
				case EXPR_VAR_POINT3:
					iload->Read(&vVars[varIndex].val, sizeof(Point3), &nb);
					break;
				}	
				iload->CloseChunk();
			}
			updRegCt(intVar = expr.defVar(VECTOR_VAR, vVars[varIndex].name), VECTOR_VAR);
			vVars[varIndex].regNum = intVar;
		}
		iload->CloseChunk();
	}
	
	// RB 3/19/99: Refs to pathController\percent need to be changed to refer to the parameter block.
	iload->RegisterPostLoadCallback(new CheckForPathContRefsPLCB(this));

	return IO_OK;
}


static INT_PTR CALLBACK ExprDbgWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int ExprDebug::winX = -1;
int ExprDebug::winY = -1;


ExprDebug::ExprDebug(HWND hParent, ExprControl *exprControl)
{
	ec = exprControl;
	CreateDialogParam(
		hInstance,
		MAKEINTRESOURCE(IDD_EXPR_DEBUG),
		hParent,
		ExprDbgWndProc,
		(LPARAM)this);	
}

ExprDebug::~ExprDebug()
{
	Rect rect;
	GetWindowRect(hWnd,&rect);
	winX = rect.left;
	winY = rect.top;
	GetCOREInterface()->UnRegisterTimeChangeCallback(this);
	if(ec)
		ec->edbg = NULL;
}

void ExprDebug::Invalidate()
{
	Rect rect;
	rect.left = 0;
	rect.right = 10;
	rect.top = 0;
	rect.bottom = 10;
	InvalidateRect(hWnd,&rect,FALSE);
}

void ExprDebug::Update()
{
	int i, ct;
	float fval;
	Point3 vval;
	TCHAR buf[1024];
	HWND hList = GetDlgItem(hWnd, IDC_DEBUG_LIST);

	SendMessage(hList, LB_RESETCONTENT, 0, 0);

	_stprintf(buf, _T("%s\t%g"), _T("T"), (float)t);
	SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)buf);
	_stprintf(buf, _T("%s\t%g"), _T("S"), (float)t/4800.0f);
	SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)buf);
	_stprintf(buf, _T("%s\t%g"), _T("F"), (float)t/GetTicksPerFrame());
	SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)buf);
	_stprintf(buf, _T("%s\t%g"), _T("NT"), (float)(t-ec->range.Start()) / 
						(float)(ec->range.End()-ec->range.Start()));
	SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)buf);
 
	ct = ec->getVarCount(SCALAR_VAR);
	for(i = 0; i < ct; i++) {
		if(ec->sVars[i].refID < 0)
			fval = ec->sVars[i].val;
		else {
			Control *c;
			Interval iv;
			c = (Control *)ec->refTab[ec->sVars[i].refID].client->SubAnim(ec->sVars[i].subNum);
			c->GetValue(t+ec->sVars[i].offset, &fval, iv);
		}
		_stprintf(buf, _T("%s\t%.4f"), ec->getVarName(SCALAR_VAR, i), fval);
		SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)buf);
	}
	ct = ec->getVarCount(VECTOR_VAR);
	for(i = 0; i < ct; i++) {
		if(ec->vVars[i].refID < 0 || !ec->refTab[ec->vVars[i].refID].client)
			vval = ec->vVars[i].val;
		else {
			Control *c;
			Interval iv;
			if (ec->vVars[i].subNum < 0) {	// this is referencing a node, not a controller! DB 1/98
				ec->GetAbsoluteControlValue(
					(INode*)ec->refTab[ec->vVars[i].refID].client,t+ec->vVars[i].offset,&vval, iv);
			} 
			else {	// controller reference...
				c = (Control *)ec->refTab[ec->vVars[i].refID].client->SubAnim(ec->vVars[i].subNum);
				c->GetValue(t+ec->vVars[i].offset, &vval, iv);
			}
		}
		_stprintf(buf, _T("%s\t[%.4f, %.4f, %.4f]"), ec->getVarName(VECTOR_VAR, i), vval.x, vval.y, vval.z);
		SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)buf);
	}
	if(ec->expr.getExprType() == SCALAR_EXPR) {
		Interval iv;
		ec->GetValue(t, &ec->curFloatVal, iv);
		_stprintf(buf, _T("%g"), ec->curFloatVal);
	}
	else if(ec->type == EXPR_SCALE_CONTROL_CLASS_ID) {
		Interval iv;
		Quat q;
		ec->GetValue(t, &q, iv);
		_stprintf(buf, _T("[%g, %g, %g]"), q.x, q.y, q.z);
	}
	else {
		Interval iv;
		ec->GetValue(t, &ec->curPosVal, iv);
		_stprintf(buf, _T("[%g, %g, %g]"), ec->curPosVal.x, ec->curPosVal.y, ec->curPosVal.z);
	}
	SetDlgItemText(hWnd, IDC_DEBUG_VALUE, buf);
}

static int tabs[] = { 75 };

void ExprDebug::Init(HWND hWnd)
{
	this->hWnd = hWnd;
	
	if (winX==-1 || winY==-1)
		CenterWindow(hWnd,GetParent(hWnd));
	else 
		SetWindowPos(hWnd,NULL,winX, winY, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

	t = GetCOREInterface()->GetTime();

	SendMessage(GetDlgItem(hWnd, IDC_DEBUG_LIST), LB_SETTABSTOPS, 1, (LPARAM)tabs);
	GetCOREInterface()->RegisterTimeChangeCallback(this);
}


static INT_PTR CALLBACK ExprDbgWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ExprDebug *ed = (ExprDebug*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
//		SetWindowContextHelpId(hWnd,idh_dialog_xform_typein);
		ed = (ExprDebug*)lParam;
		SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
		ed->Init(hWnd);
		return FALSE;

	case WM_SYSCOMMAND:
//		if ((wParam & 0xfff0)==SC_CONTEXTHELP) 
//			DoHelp(HELP_CONTEXT,idh_dialog_xform_typein);				
		return 0;

	case WM_PAINT:
//		if (!ed->valid)
			ed->Update();
		return 0;
		
	case WM_COMMAND:
		if(LOWORD(wParam) == IDCANCEL)
			goto weAreDone;
		SendMessage(GetParent(hWnd), msg, wParam, lParam);						
		break;
	
	case WM_CLOSE:
weAreDone:
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		delete ed;
		break;

	default:
		return 0;
	}
	return 1;
}



