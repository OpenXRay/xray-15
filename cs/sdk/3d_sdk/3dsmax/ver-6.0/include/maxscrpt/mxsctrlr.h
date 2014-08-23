/*	
 *		MSController.h - MAXScript scriptable controllers for MAX
 *
 *			Copyright © John Wainwright 1996
 *
 */

#ifndef _H_MSCONTROLLER
#define _H_MSCONTROLLER

#define SCRIPT_POS_CONTROL_CLASS_ID		Class_ID(0x236c6aa5, 0x27590853)
#define SCRIPT_P3_CONTROL_CLASS_ID		Class_ID(0x3d7b231d, 0x2b986df3)
#define SCRIPT_FLOAT_CONTROL_CLASS_ID	Class_ID(0x151d5ead, 0x55626f88)
#define SCRIPT_SCALE_CONTROL_CLASS_ID	Class_ID(0x5f346d25, 0x2c67ff7)
#define SCRIPT_ROT_CONTROL_CLASS_ID		Class_ID(0xc6625, 0xb003c2a)
#define SCRIPT_P4_CONTROL_CLASS_ID		Class_ID(0x3d7b234d, 0x2b986df4)

class ScriptControl : public StdControl 
{
public:
	int			type;
	Interval	ivalid;
	Interval	range;
	HWND		hParams;
	IObjParam *	ip;
	TSTR		desc;
	HWND		hDlg;

	ScriptControl(int type, ScriptControl &ctrl);
	ScriptControl(int type, BOOL loading);
	~ScriptControl();

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
	RefTargetHandle Clone(RemapDir& remap=NoRemap()) {  ScriptControl *newob = new ScriptControl(this->type, *this); BaseClone(this, newob, remap); return(newob); }		
	BOOL IsLeaf() { return TRUE; }
	void GetValueLocalTime(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);	
	void SetValueLocalTime(TimeValue t, void *val, int commit, GetSetMethod method) {}
	void Extrapolate(Interval range,TimeValue t,void *val,Interval &valid,int type);
	void *CreateTempValue();
	void DeleteTempValue(void *val);
	void ApplyValue(void *val, void *delta);
	void MultiplyValue(void *val, float m);
};

class ScriptPosControl : public ScriptControl 
{
public:
	ScriptPosControl(ScriptPosControl &ctrl) : ScriptControl(CTRL_POSITION_CLASS_ID, ctrl) {}
	ScriptPosControl(BOOL loading=FALSE) : ScriptControl(CTRL_POSITION_CLASS_ID, loading) {}
	~ScriptPosControl() {}

	RefTargetHandle Clone(RemapDir& remap=NoRemap()) {  ScriptPosControl *newob = new ScriptPosControl(*this); BaseClone(this, newob, remap); return(newob); }		
	void GetClassName(TSTR& s) { s = _T("Position_script"); }
	Class_ID ClassID() { return Class_ID(SCRIPT_POS_CONTROL_CLASS_ID,0); }  
	SClass_ID SuperClassID() { return CTRL_POSITION_CLASS_ID; }  		
};

class ScriptP3Control : public ScriptControl 
{
public:
	ScriptP3Control(ScriptP3Control &ctrl) : ScriptControl(CTRL_POINT3_CLASS_ID, ctrl) {}
	ScriptP3Control(BOOL loading=FALSE) : ScriptControl(CTRL_POINT3_CLASS_ID, loading) {}
	~ScriptP3Control() {}

	RefTargetHandle Clone(RemapDir& remap=NoRemap()) {  ScriptP3Control *newob = new ScriptP3Control(*this); BaseClone(this, newob, remap); return(newob); }		
	void GetClassName(TSTR& s) { s = _T("Point3_script"); }
	Class_ID ClassID() { return Class_ID(SCRIPT_P3_CONTROL_CLASS_ID,0); }  
	SClass_ID SuperClassID() { return CTRL_POINT3_CLASS_ID; }  		
};

class ScriptP4Control : public ScriptControl 
{
public:
	ScriptP4Control(ScriptP4Control &ctrl) : ScriptControl(CTRL_POINT4_CLASS_ID, ctrl) {}
	ScriptP4Control(BOOL loading=FALSE) : ScriptControl(CTRL_POINT4_CLASS_ID, loading) {}
	~ScriptP4Control() {}

	RefTargetHandle Clone(RemapDir& remap=NoRemap()) {  ScriptP4Control *newob = new ScriptP4Control(*this); BaseClone(this, newob, remap); return(newob); }		
	void GetClassName(TSTR& s) { s = _T("Point4_script"); }
	Class_ID ClassID() { return Class_ID(SCRIPT_P4_CONTROL_CLASS_ID,0); }  
	SClass_ID SuperClassID() { return CTRL_POINT4_CLASS_ID; }  		
};

class ScriptFloatControl : public ScriptControl 
{
public:
	ScriptFloatControl(ScriptFloatControl &ctrl) : ScriptControl(CTRL_FLOAT_CLASS_ID, ctrl) {}
	ScriptFloatControl(BOOL loading=FALSE) : ScriptControl(CTRL_FLOAT_CLASS_ID, loading) {}
	~ScriptFloatControl() {}

	RefTargetHandle Clone(RemapDir& remap=NoRemap()) {  ScriptFloatControl *newob = new ScriptFloatControl(*this); BaseClone(this, newob, remap); return(newob); }		
	void GetClassName(TSTR& s) { s = _T("Float_script"); }
	Class_ID ClassID() { return Class_ID(SCRIPT_FLOAT_CONTROL_CLASS_ID,0); }  
	SClass_ID SuperClassID() { return CTRL_FLOAT_CLASS_ID; }  		
};

class ScriptScaleControl : public ScriptControl 
{
public:
	ScriptScaleControl(ScriptScaleControl &ctrl) : ScriptControl(CTRL_SCALE_CLASS_ID, ctrl) {}
	ScriptScaleControl(BOOL loading=FALSE) : ScriptControl(CTRL_SCALE_CLASS_ID, loading) {}
	~ScriptScaleControl() {}

	RefTargetHandle Clone(RemapDir& remap=NoRemap()) {  ScriptScaleControl *newob = new ScriptScaleControl(*this); BaseClone(this, newob, remap); return(newob); }		
	void GetClassName(TSTR& s) { s = _T("Scale_script"); }
	Class_ID ClassID() { return Class_ID(SCRIPT_SCALE_CONTROL_CLASS_ID,0); }  
	SClass_ID SuperClassID() { return CTRL_SCALE_CLASS_ID; }  		
};

class ScriptRotControl : public ScriptControl 
{
public:
	ScriptRotControl(ScriptRotControl &ctrl) : ScriptControl(CTRL_ROTATION_CLASS_ID, ctrl) {}
	ScriptRotControl(BOOL loading=FALSE) : ScriptControl(CTRL_ROTATION_CLASS_ID, loading) {}
	~ScriptRotControl() {}

	RefTargetHandle Clone(RemapDir& remap=NoRemap()) {  ScriptRotControl *newob = new ScriptRotControl(*this); BaseClone(this, newob, remap); return(newob); }		
	void GetClassName(TSTR& s) { s = _T("Rotation_script"); }
	Class_ID ClassID() { return Class_ID(SCRIPT_ROT_CONTROL_CLASS_ID,0); }  
	SClass_ID SuperClassID() { return CTRL_ROTATION_CLASS_ID; }  		
};

#endif
