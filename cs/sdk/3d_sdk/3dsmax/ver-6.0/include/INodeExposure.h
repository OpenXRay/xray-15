/**********************************************************************
 
	FILE: INodeExposure.h

	DESCRIPTION:  Public interface for determining if a node should be
					exposed in misc. UIs (SelectObjects, TrackView etc).

	CREATED BY: Oleg Bayborodin

	HISTORY: - created March 21, 2002

 *>	Copyright (c) 1998-2002, All Rights Reserved.
 **********************************************************************/

#ifndef __INODEEXPOSURE__H
#define __INODEEXPOSURE__H

#include "iFnPub.h"
#include "maxtypes.h"

#define NODEEXPOSURE_INTERFACE Interface_ID(0x1b00412c, 0x6ebf48f2)
#define NODEEXPOSURE_INTERFACE_TOAPPEND Interface_ID(0x1b00412c, 0x6ebf48f3)


class INodeExposure : public FPMixinInterface {
public:

enum {	
		kSelectObjects,	kSchematicView,		kMaxscript,
		kMerge,			kMergeAnimation,	kReplace, 
		kKeyEditor,		kCurveEditor,		kRangeEditor, // three components for TrackView editors
		kUI_num // always last in the enum
	};

	// Utilities:
	static INodeExposure* GetINodeExposure(INode& n) {
		return static_cast<INodeExposure*>(n.GetInterface(NODEEXPOSURE_INTERFACE)); }

	static INodeExposure* AppendINodeExposure(INode& n) {
		return static_cast<INodeExposure*>(n.GetInterface(NODEEXPOSURE_INTERFACE_TOAPPEND)); }

	// exposure in a specific UI
	virtual bool IsExposed(int ui) const =0;
	// set exposure in all UIs
	virtual void SetExposed(bool state) =0;
	// set exposure in a specific UI
	virtual void SetExposed(bool state, int ui) =0;

	// Derived methods. They are all inline'd at the end of the file.
	bool IsExposedInSelectObjects() const;
	bool IsExposedInSchematicView() const;
	bool IsExposedInMaxscript() const;
	bool IsExposedInMerge() const;
	bool IsExposedInMergeAnimation() const;
	bool IsExposedInReplace() const;
	bool IsExposedInTrackView() const;
	bool IsExposedInKeyEditor() const;
	bool IsExposedInCurveEditor() const;
	bool IsExposedInRangeEditor() const;
	void SetExposedInSelectObjects(bool state);
	void SetExposedInSchematicView(bool state);
	void SetExposedInMaxscript(bool state);
	void SetExposedInMerge(bool state);
	void SetExposedInMergeAnimation(bool state);
	void SetExposedInReplace(bool state);
	void SetExposedInTrackView(bool state);
	void SetExposedInKeyEditor(bool state);
	void SetExposedInCurveEditor(bool state);
	void SetExposedInRangeEditor(bool state);
	
	virtual IOResult	Save(ISave*) const =0;
	virtual IOResult	Load(ILoad*) =0;
	virtual void	 	Copy(INodeExposure*) =0;

	// makes exposure parameters to be permanent (not mutable)
	virtual void BakeExposure() =0;
	// defines if exposure is baked
	virtual bool IsExposureBaked() =0;

	// Used by implementation class:
	virtual const void* ObjectOf(void*) const =0;
	virtual		  void*	ObjectOf(void*) =0;

	// Due to FPMixinInterface:
	FPInterfaceDesc* GetDesc() { return GetDescByID(NODEEXPOSURE_INTERFACE); }

	// Function publishings:
	//
	enum FuncID {
		kIsExposed, kSetExposedInAll, kSetExposed, kBakeExposure
	};

BEGIN_FUNCTION_MAP
	FN_1(kIsExposed,	TYPE_bool,	IsExposed,	TYPE_INT);
	VFN_1(kSetExposedInAll,			SetExposed, TYPE_bool);
	VFN_2(kSetExposed,				SetExposed, TYPE_bool, TYPE_INT);
	VFN_0(kBakeExposure,			BakeExposure );
END_FUNCTION_MAP

};

// inlines for derived methods
//
inline bool INodeExposure::IsExposedInSelectObjects() const
{
	return IsExposed(kSelectObjects);
}

inline bool INodeExposure::IsExposedInSchematicView() const
{
	return IsExposed(kSchematicView);
}

inline bool INodeExposure::IsExposedInMaxscript() const
{
	return IsExposed(kMaxscript);
}

inline bool INodeExposure::IsExposedInMerge() const
{
	return IsExposed(kMerge);
}

inline bool INodeExposure::IsExposedInMergeAnimation() const
{
	return IsExposed(kMergeAnimation);
}

inline bool INodeExposure::IsExposedInReplace() const
{
	return IsExposed(kReplace);
}

inline bool INodeExposure::IsExposedInTrackView() const
{
	return (IsExposed(kKeyEditor) || IsExposed(kCurveEditor) || IsExposed(kRangeEditor));
}

inline bool INodeExposure::IsExposedInKeyEditor() const
{
	return IsExposed(kKeyEditor);
}

inline bool INodeExposure::IsExposedInCurveEditor() const
{
	return IsExposed(kCurveEditor);
}

inline bool INodeExposure::IsExposedInRangeEditor() const
{
	return IsExposed(kRangeEditor);
}

inline void INodeExposure::SetExposedInSelectObjects(bool state)
{
	SetExposed(state, kSelectObjects);
}

inline void INodeExposure::SetExposedInSchematicView(bool state)
{
	SetExposed(state, kSchematicView);
}

inline void INodeExposure::SetExposedInMaxscript(bool state)
{
	SetExposed(state, kMaxscript);
}

inline void INodeExposure::SetExposedInMerge(bool state)
{
	SetExposed(state, kMerge);
}

inline void INodeExposure::SetExposedInMergeAnimation(bool state)
{
	SetExposed(state, kMergeAnimation);
}

inline void INodeExposure::SetExposedInReplace(bool state)
{
	SetExposed(state, kReplace);
}

inline void INodeExposure::SetExposedInTrackView(bool state)
{
	SetExposed(state, kKeyEditor);
	SetExposed(state, kCurveEditor);
	SetExposed(state, kRangeEditor);
}

inline void INodeExposure::SetExposedInKeyEditor(bool state)
{
	SetExposed(state, kKeyEditor);
}

inline void INodeExposure::SetExposedInCurveEditor(bool state)
{
	SetExposed(state, kCurveEditor);
}

inline void INodeExposure::SetExposedInRangeEditor(bool state)
{
	SetExposed(state, kRangeEditor);
}



#endif
