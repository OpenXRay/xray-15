/*	
 *		MAXNURBS.h - MAX NURBS access classes
 *
 *			Copyright © Autodesk, Inc. 1997
 *			Author: John Wainwright
 *
 */

#ifndef _H_MAXNURBS
#define _H_MAXNURBS

#include "Max.h"
#include "MaxObj.h"

#ifndef NO_NURBS

class NURBSSetValue;

/* ----------- base class for MAXScript NURBS wrapper classes -------- */

visible_class (NURBSObjectValue)

class NURBSObjectValue : public Value
{
public:
//	NURBSSetValue*		nset;			// NURBBSet I'm in
	static HashTable*	intern_table;
	static void			setup();
	BOOL				in_set;			// TRUE if residing in a NURBSSet (can only be in one and the NS owns deletion)

				NURBSObjectValue() { in_set = FALSE; }
				classof_methods(NURBSObjectValue, Value);
	void		collect() { delete this; }

	// operations
#include "defimpfn.h"
	// standard props
	def_property    ( name );
	def_prop_getter ( nurbsID );
	def_property    ( selected );
	def_prop_getter ( index );
//	def_prop_getter ( nurbsSet );

	void		common_nurbs_params(Value** arg_list, int count);

};

#define NRB_DIRECT		0x0001	// direct access

/* --------------- base class for NURBSPoints ----------------------- */

visible_class_s (NURBSPointValue, NURBSObjectValue)

class NURBSPointValue : public NURBSObjectValue
{
public:
				classof_methods(NURBSPointValue, NURBSObjectValue);
	void		collect() { delete this; }
	
	// operations
#include "defimpfn.h"
	// built-in property accessors
	def_prop_getter ( pos );
	def_prop_getter ( x );
	def_prop_getter ( y );
	def_prop_getter ( z );

	Point3	to_point3() { return to_nurbspoint()->GetPosition(MAXScript_time()); }
};

/* ---------------  wrapper for NURBSIndependentPoints ----------------------- */

applyable_class_s (NURBSIndependentPointValue, NURBSPointValue)

class NURBSIndependentPointValue : public NURBSPointValue
{
public:
	NURBSIndependentPoint* point;

	ScripterExport NURBSIndependentPointValue(Point3 pt);
	ScripterExport NURBSIndependentPointValue(NURBSIndependentPoint* pt);
	static ScripterExport NURBSIndependentPointValue* intern(NURBSIndependentPoint* pt);
	ScripterExport ~NURBSIndependentPointValue() { if (!in_set) delete point; }

				classof_methods(NURBSIndependentPointValue, NURBSPointValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"
	use_generic  ( eq,	"=");
	use_generic  ( ne,	"!=");

	// built-in property accessors
	def_prop_setter ( pos );
	def_prop_setter ( x );
	def_prop_setter ( y );
	def_prop_setter ( z );

	NURBSIndependentPoint*	to_nurbsindependentpoint() { return point; }
	NURBSPoint*				to_nurbspoint() { return point; }
	NURBSObject*			to_nurbsobject() { return point; }
};

/* ---------------  wrapper for NURBSPointConstPoints ----------------------- */

applyable_class_s (NURBSPointConstPointValue, NURBSPointValue)

class NURBSPointConstPointValue : public NURBSPointValue
{
public:
	NURBSPointConstPoint* point;

	ScripterExport NURBSPointConstPointValue();
	ScripterExport NURBSPointConstPointValue(NURBSPointConstPoint* pt);
	static ScripterExport NURBSPointConstPointValue* intern(NURBSPointConstPoint* pt);
	ScripterExport ~NURBSPointConstPointValue() { if (!in_set) delete point; }

				classof_methods(NURBSPointConstPointValue, NURBSPointValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	 
	// operations
#include "defimpfn.h"

	// built-in property accessors
	def_property ( parent );
	def_property ( parentID );
	def_property ( type );
	def_property ( offset );

	NURBSPointConstPoint*	to_nurbspointconstpoint() { return point; }
	NURBSPoint*				to_nurbspoint() { return point; }
	NURBSObject*			to_nurbsobject() { return point; }
};

/* ---------------  wrapper for NURBSCurveConstPoints ----------------------- */

applyable_class_s (NURBSCurveConstPointValue, NURBSPointValue)

class NURBSCurveConstPointValue : public NURBSPointValue
{
public:
	NURBSCurveConstPoint* point;

	ScripterExport NURBSCurveConstPointValue();
	ScripterExport NURBSCurveConstPointValue(NURBSCurveConstPoint* pt);
	static ScripterExport NURBSCurveConstPointValue* intern(NURBSCurveConstPoint* pt);
	ScripterExport ~NURBSCurveConstPointValue() { if (!in_set) delete point; }

				classof_methods(NURBSCurveConstPointValue, NURBSPointValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"
	// built-in property accessors
	def_property ( parent );
	def_property ( parentID );
	def_property ( type );
	def_property ( offset );
	def_property ( uParam );
	def_property ( normal );
	def_property ( uTangent );
	def_property ( trimCurve );
	def_property ( flipTrim );

	NURBSCurveConstPoint*	to_nurbscurveconstpoint() { return point; }
	NURBSPoint*				to_nurbspoint() { return point; }
	NURBSObject*			to_nurbsobject() { return point; }
};

/* --------------- wrapper for NURBSCurveIntersectPoint ----------------------- */

applyable_class_s (NURBSCurveIntersectPointValue, NURBSPointValue)

class NURBSCurveIntersectPointValue : public NURBSPointValue
{
public:
	NURBSCurveCurveIntersectionPoint* point;

	ScripterExport NURBSCurveIntersectPointValue();
	ScripterExport NURBSCurveIntersectPointValue(NURBSCurveCurveIntersectionPoint* pt);
	static ScripterExport NURBSCurveIntersectPointValue* intern(NURBSCurveCurveIntersectionPoint* pt);
	ScripterExport ~NURBSCurveIntersectPointValue() { if (!in_set) delete point; }

				classof_methods(NURBSCurveIntersectPointValue, NURBSPointValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"
	// built-in property accessors
	def_property ( parent1 );
	def_property ( parent1ID );
	def_property ( parent2 );
	def_property ( parent2ID );
	def_property ( trimCurve1 );
	def_property ( trimCurve2 );
	def_property ( flipTrim1 );
	def_property ( flipTrim2 );

	NURBSCurveCurveIntersectionPoint*	to_nurbscurvecurveintersectionpoint() { return point; }
	NURBSPoint*							to_nurbspoint() { return point; }
	NURBSObject*						to_nurbsobject() { return point; }
};

/* --------------- wrapper for NURBSCurveSurfaceIntersectPoint ----------------------- */

applyable_class_s (NURBSCurveSurfaceIntersectPointValue, NURBSPointValue)

class NURBSCurveSurfaceIntersectPointValue : public NURBSPointValue
{
public:
	NURBSCurveSurfaceIntersectionPoint* point;

	ScripterExport NURBSCurveSurfaceIntersectPointValue();
	ScripterExport NURBSCurveSurfaceIntersectPointValue(NURBSCurveSurfaceIntersectionPoint* pt);
	static ScripterExport NURBSCurveSurfaceIntersectPointValue* intern(NURBSCurveSurfaceIntersectionPoint* pt);
	ScripterExport ~NURBSCurveSurfaceIntersectPointValue() { if (!in_set) delete point; }

				classof_methods(NURBSCurveSurfaceIntersectPointValue, NURBSPointValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"
	// built-in property accessors
	def_property ( seed );
	def_property ( parent1 );
	def_property ( parent1ID );
	def_property ( parent2 );
	def_property ( parent2ID );
	def_property ( trimCurve );
	def_property ( flipTrim );

	NURBSCurveSurfaceIntersectionPoint*	to_nurbscurvesurfaceintersectionpoint() { return point; }
	NURBSPoint*							to_nurbspoint() { return point; }
	NURBSObject*						to_nurbsobject() { return point; }
};

/* --------------- wrapper for NURBSSurfConstPoint ----------------------- */

applyable_class_s (NURBSSurfConstPointValue, NURBSPointValue)

class NURBSSurfConstPointValue : public NURBSPointValue
{
public:
	NURBSSurfConstPoint* point;

	ScripterExport NURBSSurfConstPointValue();
	ScripterExport NURBSSurfConstPointValue(NURBSSurfConstPoint* pt);
	static ScripterExport NURBSSurfConstPointValue* intern(NURBSSurfConstPoint* pt);
	ScripterExport ~NURBSSurfConstPointValue() { if (!in_set) delete point; }

				classof_methods(NURBSSurfConstPointValue, NURBSPointValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"
	// built-in property accessors
	def_property ( parent );
	def_property ( parentID );
	def_property ( type );
	def_property ( offset );
	def_property ( uParam );
	def_property ( vParam );
	def_property ( normal );
	def_property ( uTangent );
	def_property ( vTangent );

	NURBSSurfConstPoint*	to_nurbssurfconstpoint() { return point; }
	NURBSPoint*				to_nurbspoint() { return point; }
	NURBSObject*			to_nurbsobject() { return point; }
};

#ifdef WAITING_ON_APT
	/* --------------- 	wrapper for NURBSTrimPoint ----------------------- */

	applyable_class_s (NURBSTrimPointValue, NURBSPointValue)

	class NURBSTrimPointValue : public Value
	{
	public:
		NURBSTrimPoint point;

		ScripterExport NURBSTrimPointValue(double param, NURBSTrimDirection dir);
		ScripterExport NURBSTrimPointValue(NURBSTrimPoint* pt);

					classof_methods(NURBSTrimPointValue, Value);
		void		collect() { delete this; }
		ScripterExport void		sprin1(CharStream* s);
		
		// operations
	#include "defimpfn.h"
		// built-in property accessors
		def_prop_getter ( parameter );
		def_prop_setter ( dir );

		NURBSTrimPoint*			to_nurbstrimpoint() { return point; }
	};
#endif

/* --------------- wrapper for NURBSControlVertex ----------------------- */

applyable_class_s (NURBSControlVertexValue, NURBSObjectValue)

class NURBSControlVertexValue : public NURBSObjectValue
{
public:
	NURBSControlVertex*	cv;

	ScripterExport NURBSControlVertexValue(NURBSControlVertex* icv);
	ScripterExport NURBSControlVertexValue(Point3 pt, float weight);
	static ScripterExport NURBSControlVertexValue* intern(NURBSControlVertex* icv);
	ScripterExport ~NURBSControlVertexValue() { if (!in_set) delete cv; }

				classof_methods(NURBSControlVertexValue, NURBSObjectValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
#	define		is_nurbscontrolvertex(p) ((p)->tag == class_tag(NURBSControlVertexValue))
	
	// operations
#include "defimpfn.h"
	use_generic  ( eq,	"=");
	use_generic  ( ne,	"!=");

	// built-in property accessors
	def_property ( weight );
	def_property ( pos );
	def_property ( x );
	def_property ( y );
	def_property ( z );

	Point3				to_point3() { return cv->GetPosition(MAXScript_time()); }
	NURBSControlVertex* to_nurbscontrolvertex() { return cv; }
	NURBSObject*		to_nurbsobject() { return cv; }
};

/* --------------- base class for NURBSCurve ----------------------- */

class NURBSCurveValueClass : public ValueMetaClass  // visible_class_s (NURBSCurveValue)
{
	public:	
				NURBSCurveValueClass(TCHAR* name) : ValueMetaClass (name) { }
	Value*		classOf_vf(Value** arg_list, int count) { return class_tag(NURBSObjectValue); }
	Value*		superClassOf_vf(Value** arg_list, int count) { return NURBSObjectValue_class.classOf_vf(NULL, 0);}
	void		collect() { delete this; }
	Class_ID	get_max_class_id() { return EDITABLE_CVCURVE_CLASS_ID; }
};
extern ScripterExport NURBSCurveValueClass NURBSCurveValue_class;

class NURBSCurveValue : public NURBSObjectValue
{
public:
				classof_methods(NURBSCurveValue, NURBSObjectValue);
	void		collect() { delete this; }
	
	// operations
#include "defimpfn.h"
	def_generic ( evalPos,	   "evalPos" );
	def_generic ( evalTangent, "evalTangent" );

	// built-in property accessors
	def_prop_getter ( isClosed );
	def_prop_getter ( numTrimPoints );
	def_prop_getter ( parameterRangeMin );
	def_prop_getter ( parameterRangeMax );
	def_property	( matID );

	void		common_curve_params(Value** arg_list, int count);
};

/* --------------- wrapper for NURBSCVCurve ----------------------- */

applyable_class_s (NURBSCVCurveValue, NURBSCurveValue)

class NURBSCVCurveValue : public NURBSCurveValue
{
public:
	NURBSCVCurve* curve;

	ScripterExport NURBSCVCurveValue();
	ScripterExport NURBSCVCurveValue(NURBSCVCurve* pt);
	static ScripterExport NURBSCVCurveValue* intern(NURBSCVCurve* pt);
	ScripterExport ~NURBSCVCurveValue() { if (!in_set) delete curve; }

				classof_methods(NURBSCVCurveValue, NURBSCurveValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"
	use_generic ( close,	"close" );
	def_generic ( getKnot,	"getKnot" );
	def_generic ( setKnot,	"setKnot" );
	def_generic ( getCV,	"getCV" );
	def_generic ( setCV,	"setCV" );
	def_generic ( refine,	"refine" );
	def_generic ( reparameterize, "reparameterize" );

	// built-in property accessors
	def_property ( order );
	def_property ( numKnots );
	def_property ( numCVs );
	def_property ( transform );
	def_property ( autoParam );
	def_prop_getter ( endsOverlap );

	NURBSCurve*		to_nurbscurve() { return curve; }
	NURBSCVCurve*	to_nurbscvcurve() { return curve;  }
	NURBSObject*	to_nurbsobject() { return curve; }

};

/* --------------- wrapper for NURBSPointCurve ----------------------- */

applyable_class_s (NURBSPointCurveValue, NURBSCurveValue)

class NURBSPointCurveValue : public NURBSCurveValue
{
public:
	NURBSPointCurve* curve;

	ScripterExport NURBSPointCurveValue();
	ScripterExport NURBSPointCurveValue(NURBSPointCurve* pt);
	static ScripterExport NURBSPointCurveValue* intern(NURBSPointCurve* pt);
	ScripterExport ~NURBSPointCurveValue() { if (!in_set) delete curve; }

				classof_methods(NURBSPointCurveValue, NURBSCurveValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"
	use_generic ( close,	"close" );
	def_generic ( getPoint,	"getPoint" );
	def_generic ( setPoint,	"setPoint" );
	def_generic ( refine,	"refine" );

	// built-in property accessors
	def_property ( numPoints );
	def_property ( transform );

	NURBSCurve*		to_nurbscurve() { return curve; }
	NURBSObject*	to_nurbsobject() { return curve; }
};

/* --------------- wrapper for NURBSBlendCurve ----------------------- */

applyable_class_s (NURBSBlendCurveValue, NURBSCurveValue)

class NURBSBlendCurveValue : public NURBSCurveValue
{
public:
	NURBSBlendCurve* curve;

	ScripterExport NURBSBlendCurveValue();
	ScripterExport NURBSBlendCurveValue(NURBSBlendCurve* pt);
	static ScripterExport NURBSBlendCurveValue* intern(NURBSBlendCurve* pt);
	ScripterExport ~NURBSBlendCurveValue() { if (!in_set) delete curve; }

				classof_methods(NURBSBlendCurveValue, NURBSCurveValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"

	// built-in property accessors
	def_property ( parent1 );
	def_property ( parent1ID );
	def_property ( parent2 );
	def_property ( parent2ID );
	def_property ( flip1 );
	def_property ( flip2 );
	def_property ( tension1 );
	def_property ( tension2 );

	NURBSCurve*		to_nurbscurve() { return curve; }
	NURBSObject*	to_nurbsobject() { return curve; }
};

/* --------------- wrapper for NURBSOffsetCurve ----------------------- */

applyable_class_s (NURBSOffsetCurveValue, NURBSCurveValue)

class NURBSOffsetCurveValue : public NURBSCurveValue
{
public:
	NURBSOffsetCurve* curve;

	ScripterExport NURBSOffsetCurveValue();
	ScripterExport NURBSOffsetCurveValue(NURBSOffsetCurve* pt);
	static ScripterExport NURBSOffsetCurveValue* intern(NURBSOffsetCurve* pt);
	ScripterExport ~NURBSOffsetCurveValue() { if (!in_set) delete curve; }

				classof_methods(NURBSOffsetCurveValue, NURBSCurveValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"

	// built-in property accessors
	def_property ( parent );
	def_property ( parentID );
	def_property ( distance );

	NURBSCurve*		to_nurbscurve() { return curve; }
	NURBSObject*	to_nurbsobject() { return curve; }
};

/* --------------- wrapper for NURBSXFormCurve ----------------------- */

applyable_class_s (NURBSXFormCurveValue, NURBSCurveValue)

class NURBSXFormCurveValue : public NURBSCurveValue
{
public:
	NURBSXFormCurve* curve;

	ScripterExport NURBSXFormCurveValue();
	ScripterExport NURBSXFormCurveValue(NURBSXFormCurve* pt);
	static ScripterExport NURBSXFormCurveValue* intern(NURBSXFormCurve* pt);
	ScripterExport ~NURBSXFormCurveValue() { if (!in_set) delete curve; }

				classof_methods(NURBSXFormCurveValue, NURBSCurveValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"

	// built-in property accessors
	def_property ( parent );
	def_property ( parentID );
	def_property ( transform );

	NURBSCurve*		to_nurbscurve() { return curve; }
	NURBSObject*	to_nurbsobject() { return curve; }
};

/* --------------- wrapper for NURBSMirrorCurve ----------------------- */

applyable_class_s (NURBSMirrorCurveValue, NURBSCurveValue)

class NURBSMirrorCurveValue : public NURBSCurveValue
{
public:
	NURBSMirrorCurve* curve;

	ScripterExport NURBSMirrorCurveValue();
	ScripterExport NURBSMirrorCurveValue(NURBSMirrorCurve* pt);
	static ScripterExport NURBSMirrorCurveValue* intern(NURBSMirrorCurve* pt);
	ScripterExport ~NURBSMirrorCurveValue() { if (!in_set) delete curve; }

				classof_methods(NURBSMirrorCurveValue, NURBSCurveValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"

	// built-in property accessors
	def_property ( parent );
	def_property ( parentID );
	def_property ( axis );
	def_property ( distance );
	def_property ( transform );

	NURBSCurve*		to_nurbscurve() { return curve; }
	NURBSObject*	to_nurbsobject() { return curve; }
};

/* --------------- wrapper for NURBSFilletCurve ----------------------- */

applyable_class_s (NURBSFilletCurveValue, NURBSCurveValue)

class NURBSFilletCurveValue : public NURBSCurveValue
{
public:
	NURBSFilletCurve* curve;

	ScripterExport NURBSFilletCurveValue();
	ScripterExport NURBSFilletCurveValue(NURBSFilletCurve* pt);
	static ScripterExport NURBSFilletCurveValue* intern(NURBSFilletCurve* pt);
	ScripterExport ~NURBSFilletCurveValue() { if (!in_set) delete curve; }

				classof_methods(NURBSFilletCurveValue, NURBSCurveValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"

	// built-in property accessors
	def_property ( parent1 );
	def_property ( parent1ID );
	def_property ( parent2 );
	def_property ( parent2ID );
	def_property ( flip1 );
	def_property ( flip2 );
	def_property ( radius );
	def_property ( trim1 );
	def_property ( trim2 );
	def_property ( flipTrim1 );
	def_property ( flipTrim2 );

	NURBSCurve*		to_nurbscurve() { return curve; }
	NURBSObject*	to_nurbsobject() { return curve; }
};

/* --------------- wrapper for NURBSChamferCurve ----------------------- */

applyable_class_s (NURBSChamferCurveValue, NURBSCurveValue)

class NURBSChamferCurveValue : public NURBSCurveValue
{
public:
	NURBSChamferCurve* curve;

	ScripterExport NURBSChamferCurveValue();
	ScripterExport NURBSChamferCurveValue(NURBSChamferCurve* pt);
	static ScripterExport NURBSChamferCurveValue* intern(NURBSChamferCurve* pt);
	ScripterExport ~NURBSChamferCurveValue() { if (!in_set) delete curve; }

				classof_methods(NURBSChamferCurveValue, NURBSCurveValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"

	// built-in property accessors
	def_property ( parent1 );
	def_property ( parent1ID );
	def_property ( parent2 );
	def_property ( parent2ID );
	def_property ( flip1 );
	def_property ( flip2 );
	def_property ( length1 );
	def_property ( length2 );
	def_property ( trim1 );
	def_property ( trim2 );
	def_property ( flipTrim1 );
	def_property ( flipTrim2 );

	NURBSCurve*		to_nurbscurve() { return curve; }
	NURBSObject*	to_nurbsobject() { return curve; }
};

/* --------------- wrapper for NURBSIsoCurve ----------------------- */

applyable_class_s (NURBSIsoCurveValue, NURBSCurveValue)

class NURBSIsoCurveValue : public NURBSCurveValue
{
public:
	NURBSIsoCurve* curve;

	ScripterExport NURBSIsoCurveValue();
	ScripterExport NURBSIsoCurveValue(NURBSIsoCurve* pt);
	static ScripterExport NURBSIsoCurveValue* intern(NURBSIsoCurve* pt);
	ScripterExport ~NURBSIsoCurveValue() { if (!in_set) delete curve; }

				classof_methods(NURBSIsoCurveValue, NURBSCurveValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"

	// built-in property accessors
	def_property ( parent );
	def_property ( parentID );
	def_property ( dir );
	def_property ( parameter );
	def_property ( trim );
	def_property ( flipTrim );
	def_property ( seed );

	NURBSCurve*		to_nurbscurve() { return curve; }
	NURBSObject*	to_nurbsobject() { return curve; }
};

/* --------------- wrapper for NURBSProjectVectorCurve ----------------------- */

applyable_class_s (NURBSProjectVectorCurveValue, NURBSCurveValue)

class NURBSProjectVectorCurveValue : public NURBSCurveValue
{
public:
	NURBSProjectVectorCurve* curve;

	ScripterExport NURBSProjectVectorCurveValue();
	ScripterExport NURBSProjectVectorCurveValue(NURBSProjectVectorCurve* pt);
	static ScripterExport NURBSProjectVectorCurveValue* intern(NURBSProjectVectorCurve* pt);
	ScripterExport ~NURBSProjectVectorCurveValue() { if (!in_set) delete curve; }

				classof_methods(NURBSProjectVectorCurveValue, NURBSCurveValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"

	// built-in property accessors
	def_property ( parent1 );
	def_property ( parent1ID );
	def_property ( parent2 );
	def_property ( parent2ID );
	def_property ( trim );
	def_property ( flipTrim );
	def_property ( seed );
	def_property ( pVec );

	NURBSCurve*		to_nurbscurve() { return curve; }
	NURBSObject*	to_nurbsobject() { return curve; }
};

/* --------------- wrapper for NURBSProjectNormalCurve ----------------------- */

applyable_class_s (NURBSProjectNormalCurveValue, NURBSCurveValue)

class NURBSProjectNormalCurveValue : public NURBSCurveValue
{
public:
	NURBSProjectNormalCurve* curve;

	ScripterExport NURBSProjectNormalCurveValue();
	ScripterExport NURBSProjectNormalCurveValue(NURBSProjectNormalCurve* pt);
	static ScripterExport NURBSProjectNormalCurveValue* intern(NURBSProjectNormalCurve* pt);
	ScripterExport ~NURBSProjectNormalCurveValue() { if (!in_set) delete curve; }

				classof_methods(NURBSProjectNormalCurveValue, NURBSCurveValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"

	// built-in property accessors
	def_property ( parent1 );
	def_property ( parent1ID );
	def_property ( parent2 );
	def_property ( parent2ID );
	def_property ( trim );
	def_property ( flipTrim );
	def_property ( seed );

	NURBSCurve*		to_nurbscurve() { return curve; }
	NURBSObject*	to_nurbsobject() { return curve; }
};

/* --------------- wrapper for NURBSSurfSurfIntersectionCurve ----------------------- */

applyable_class_s (NURBSSurfSurfIntersectionCurveValue, NURBSCurveValue)

class NURBSSurfSurfIntersectionCurveValue : public NURBSCurveValue
{
public:
	NURBSSurfSurfIntersectionCurve* curve;

	ScripterExport NURBSSurfSurfIntersectionCurveValue();
	ScripterExport NURBSSurfSurfIntersectionCurveValue(NURBSSurfSurfIntersectionCurve* pt);
	static ScripterExport NURBSSurfSurfIntersectionCurveValue* intern(NURBSSurfSurfIntersectionCurve* pt);
	ScripterExport ~NURBSSurfSurfIntersectionCurveValue() { if (!in_set) delete curve; }

				classof_methods(NURBSSurfSurfIntersectionCurveValue, NURBSCurveValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"

	// built-in property accessors
	def_property ( parent1 );
	def_property ( parent1ID );
	def_property ( parent2 );
	def_property ( parent2ID );
	def_property ( trim1 );
	def_property ( flipTrim1 );
	def_property ( trim2 );
	def_property ( flipTrim2 );
	def_property ( seed );

	NURBSCurve*		to_nurbscurve() { return curve; }
	NURBSObject*	to_nurbsobject() { return curve; }
};

/* --------------- wrapper for NURBSCurveOnSurface ----------------------- */

applyable_class_s (NURBSCurveOnSurfaceValue, NURBSCVCurveValue)

class NURBSCurveOnSurfaceValue : public NURBSCVCurveValue
{
public:
	NURBSCurveOnSurface* curve;

	ScripterExport NURBSCurveOnSurfaceValue();
	ScripterExport NURBSCurveOnSurfaceValue(NURBSCurveOnSurface* pt);
	static ScripterExport NURBSCurveOnSurfaceValue* intern(NURBSCurveOnSurface* pt);
	ScripterExport ~NURBSCurveOnSurfaceValue() { if (!in_set) delete curve; }

				classof_methods(NURBSCurveOnSurfaceValue, NURBSCVCurveValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"
	// operations
#include "defimpfn.h"
	// from NURBSCVCurve
	use_generic ( close,	"close" );
	def_generic ( getKnot,	"getKnot" );
	def_generic ( setKnot,	"setKnot" );
	def_generic ( getCV,	"getCV" );
	def_generic ( setCV,	"setCV" );
	def_generic ( refine,	"refine" );

	// built-in property accessors
	// from NURBSCVCurve
	def_property ( order ); 
	def_property ( numKnots );
	def_property ( numCVs );
	def_property ( transform );
	def_prop_getter ( endsOverlap );

	// from NURBSCurveOnSurface
	def_property ( parent );
	def_property ( parentID );
	def_property ( trim );
	def_property ( flipTrim );

	NURBSCurve*		to_nurbscurve() { return curve; }
	NURBSObject*	to_nurbsobject() { return curve; }
};

/* --------------- wrapper for NURBSPointCurveOnSurface ----------------------- */

applyable_class_s (NURBSPointCurveOnSurfaceValue, NURBSPointCurveValue)

class NURBSPointCurveOnSurfaceValue : public NURBSPointCurveValue
{
public:
	NURBSPointCurveOnSurface* curve;

	ScripterExport NURBSPointCurveOnSurfaceValue();
	ScripterExport NURBSPointCurveOnSurfaceValue(NURBSPointCurveOnSurface* pt);
	static ScripterExport NURBSPointCurveOnSurfaceValue* intern(NURBSPointCurveOnSurface* pt);
	ScripterExport ~NURBSPointCurveOnSurfaceValue() { if (!in_set) delete curve; }

				classof_methods(NURBSPointCurveOnSurfaceValue, NURBSPointCurveValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"
	// operations
#include "defimpfn.h"
	// from NURBSPointCurve
	use_generic ( close,	"close" );
	def_generic ( getPoint,	"getPoint" );
	def_generic ( setPoint,	"setPoint" );
	def_generic ( refine,	"refine" );

	// built-in property accessors
	// from NURBSPointCurve
	def_property ( numPoints );
	def_property ( transform );

	// from NURBSPointCurveOnSurface
	def_property ( parent );
	def_property ( parentID );
	def_property ( trim );
	def_property ( flipTrim );

	NURBSCurve*		to_nurbscurve() { return curve; }
	NURBSObject*	to_nurbsobject() { return curve; }
};

/* --------------- wrapper for NURBSSurfaceNormalCurve ----------------------- */

applyable_class_s (NURBSSurfaceNormalCurveValue, NURBSCurveValue)

class NURBSSurfaceNormalCurveValue : public NURBSCurveValue
{
public:
	NURBSSurfaceNormalCurve* curve;

	ScripterExport NURBSSurfaceNormalCurveValue();
	ScripterExport NURBSSurfaceNormalCurveValue(NURBSSurfaceNormalCurve* pt);
	static ScripterExport NURBSSurfaceNormalCurveValue* intern(NURBSSurfaceNormalCurve* pt);
	ScripterExport ~NURBSSurfaceNormalCurveValue() { if (!in_set) delete curve; }

				classof_methods(NURBSSurfaceNormalCurveValue, NURBSCurveValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"

	// built-in property accessors
	def_property ( parent );
	def_property ( parentID );
	def_property ( distance );

	NURBSCurve*		to_nurbscurve() { return curve; }
	NURBSObject*	to_nurbsobject() { return curve; }
};

/* --------------- wrapper for NURBSIsoCurve ----------------------- */

applyable_class_s (NURBSSurfaceEdgeCurveValue, NURBSCurveValue)

class NURBSSurfaceEdgeCurveValue : public NURBSCurveValue
{
public:
	NURBSSurfaceEdgeCurve* curve;

	ScripterExport NURBSSurfaceEdgeCurveValue();
	ScripterExport NURBSSurfaceEdgeCurveValue(NURBSSurfaceEdgeCurve* c);
	static ScripterExport NURBSSurfaceEdgeCurveValue* intern(NURBSSurfaceEdgeCurve* c);
	ScripterExport ~NURBSSurfaceEdgeCurveValue() { if (!in_set) delete curve; }

				classof_methods(NURBSSurfaceEdgeCurveValue, NURBSCurveValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"

	// built-in property accessors
	def_property ( parent );
	def_property ( parentID );
	def_property ( seed );

	NURBSCurve*		to_nurbscurve() { return curve; }
	NURBSObject*	to_nurbsobject() { return curve; }
};

/* --------------- wrapper for NURBSTextureSurfaceValue ----------------------- */

applyable_class (NURBSTextureSurfaceValue)

class NURBSTextureSurfaceValue : public Value
{
public:
	NURBSTextureSurface	surface;		// surface texture

	ScripterExport NURBSTextureSurfaceValue();
	ScripterExport NURBSTextureSurfaceValue(NURBSTextureSurface& surf);

				classof_methods(NURBSTextureSurfaceValue, Value);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
#	define		is_nurbstexturesurface(p) ((p)->tag == class_tag(NURBSTextureSurfaceValue))
	
	// operations
#include "defimpfn.h"
	def_generic ( getPoint,		"getPoint" );
	def_generic ( setPoint,		"setPoint" );

	// built-in property accessors
	def_property ( type );
	def_property ( numPoints );
	def_prop_getter ( numUPoints );
	def_prop_getter ( numVPoints );
	def_property ( parent );
	def_property ( parentID );

	NURBSTextureSurface*	to_nurbstexturesurface() { return &surface; }
};

/* --------------- base class for NURBSSurface ----------------------- */

class NURBSSurfaceValueClass : public ValueMetaClass  // visible_class_s (NURBSSurfaceValue)
{
	public:	
				NURBSSurfaceValueClass(TCHAR* name) : ValueMetaClass (name) { }
	Value*		classOf_vf(Value** arg_list, int count) { return class_tag(NURBSObjectValue); }
	Value*		superClassOf_vf(Value** arg_list, int count) { return NURBSObjectValue_class.classOf_vf(NULL, 0);}
	void		collect() { delete this; }
	Class_ID	get_max_class_id() { return EDITABLE_SURF_CLASS_ID; }
};
extern ScripterExport NURBSSurfaceValueClass NURBSSurfaceValue_class;

class NURBSSurfaceValue : public NURBSObjectValue
{
public:
	void		common_surface_params(Value** arg_list, int count);
				classof_methods(NURBSSurfaceValue, NURBSObjectValue);
	void		collect() { delete this; }

	// operations
#include "defimpfn.h"
	def_generic ( evalPos,			"evalPos" );
	def_generic ( evalUTangent,		"evalUTangent" );
	def_generic ( evalVTangent,		"evalVTangent" );
	def_generic ( setTiling,		"setTiling" );
	def_generic ( getTiling,		"getTiling" );
	def_generic ( setTilingOffset,	"setTilingOffset" );
	def_generic ( getTilingOffset,	"getTilingOffset" );
	def_generic ( setTextureUVs,	"setTextureUVs" );
	def_generic ( getTextureUVs,	"getTextureUVs" );
	def_generic ( setGenerateUVs,	"setGenerateUVs" );
	def_generic ( getGenerateUVs,	"getGenerateUVs" );
	def_generic ( setTextureSurface, "setTextureSurface" );
	def_generic ( getTextureSurface, "getTextureSurface" );
	def_generic ( getProdTess,		"getProdTess" );
	def_generic ( setProdTess,		"setProdTess" );
	def_generic ( getViewTess,		"getViewTess" );
	def_generic ( setViewTess,		"setViewTess" );
	def_generic ( clearViewTess,	"clearViewTess" );
	def_generic ( clearProdTess,	"clearProdTess" );

	// built-in property accessors
	def_property ( renderable );
	def_property ( flipNormals );
	def_property ( generateUVs1 );
	def_property ( generateUVs2 );
	def_property ( textureSurface1 );
	def_property ( textureSurface2 );
	def_property ( matID );
	def_prop_getter ( closedInU );
	def_prop_getter ( closedInV );
	def_prop_getter ( uParameterRangeMin );
	def_prop_getter ( vParameterRangeMin );
	def_prop_getter ( uParameterRangeMax );
	def_prop_getter ( vParameterRangeMax );
	def_prop_getter ( numChannels );
};

/* --------------- wrapper for NURBSCVSurface ----------------------- */

applyable_class_s (NURBSCVSurfaceValue, NURBSSurfaceValue)

class NURBSCVSurfaceValue : public NURBSSurfaceValue
{
public:
	NURBSCVSurface* surface;

	ScripterExport NURBSCVSurfaceValue();
	ScripterExport NURBSCVSurfaceValue(NURBSCVSurface* pt);
	static ScripterExport NURBSCVSurfaceValue* intern(NURBSCVSurface* pt);
	ScripterExport ~NURBSCVSurfaceValue() { if (!in_set) delete surface; }

				classof_methods(NURBSCVSurfaceValue, NURBSSurfaceValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"
	def_generic ( closeU,	"closeU" );
	def_generic ( closeV,	"closeV" );
	def_generic ( getUKnot,	"getUKnot" );
	def_generic ( getVKnot,	"getVKnot" );
	def_generic ( setUKnot,	"setUKnot" );
	def_generic ( setVKnot,	"setVKnot" );
	def_generic ( getCV,	"getCV" );
	def_generic ( setCV,	"setCV" );
	def_generic ( refineU,	"refineU" );
	def_generic ( refineV,	"refineV" );
	def_generic ( refine,	"refine" );
	def_generic ( reparameterize, "reparameterize" );

	// built-in property accessors
	def_property ( uOrder );
	def_property ( vOrder );
	def_property ( numUKnots );
	def_property ( numVKnots );
	def_property ( numCVs );
	def_property ( transform );
	def_property ( rigid );
	def_property ( autoParam );
	def_prop_getter ( uEdgesOverlap );
	def_prop_getter ( vEdgesOverlap );

	NURBSSurface*	to_nurbssurface() { return surface; }
	NURBSObject*	to_nurbsobject() { return surface; }
};

/* --------------- wrapper for NURBSPointSurface ----------------------- */

applyable_class_s (NURBSPointSurfaceValue, NURBSSurfaceValue)

class NURBSPointSurfaceValue : public NURBSSurfaceValue
{
public:
	NURBSPointSurface* surface;

	ScripterExport NURBSPointSurfaceValue();
	ScripterExport NURBSPointSurfaceValue(NURBSPointSurface* pt);
	static ScripterExport NURBSPointSurfaceValue* intern(NURBSPointSurface* pt);
	ScripterExport ~NURBSPointSurfaceValue() { if (!in_set) delete surface; }

				classof_methods(NURBSPointSurfaceValue, NURBSSurfaceValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"
	def_generic ( closeU,	"closeU" );
	def_generic ( closeV,	"closeV" );
	def_generic ( getPoint,	"getPoint" );
	def_generic ( setPoint,	"setPoint" );
	def_generic ( refineU,	"refineU" );
	def_generic ( refineV,	"refineV" );
	def_generic ( refine,	"refine" );

	// built-in property accessors
	def_property ( numPoints );
	def_property ( transform );

	NURBSSurface*	to_nurbssurface() { return surface; }
	NURBSObject*	to_nurbsobject() { return surface; }
};

/* --------------- wrapper for NURBSBlendSurface ----------------------- */

applyable_class_s (NURBSBlendSurfaceValue, NURBSSurfaceValue)

class NURBSBlendSurfaceValue : public NURBSSurfaceValue
{
public:
	NURBSBlendSurface* surface;

	ScripterExport NURBSBlendSurfaceValue();
	ScripterExport NURBSBlendSurfaceValue(NURBSBlendSurface* pt);
	static ScripterExport NURBSBlendSurfaceValue* intern(NURBSBlendSurface* pt);
	ScripterExport ~NURBSBlendSurfaceValue() { if (!in_set) delete surface; }

				classof_methods(NURBSBlendSurfaceValue, NURBSSurfaceValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"

	// built-in property accessors
	def_property ( parent1 );
	def_property ( parent1ID );
	def_property ( parent2ID );
	def_property ( parent2 );
	def_property ( edge1 );
	def_property ( edge2 );
	def_property ( flip1 );
	def_property ( flip2 );
	def_property ( tension1 );
	def_property ( tension2 );
	def_property ( curveStartPoint1 );
	def_property ( curveStartPoint2 );

	NURBSSurface*	to_nurbssurface() { return surface; }
	NURBSObject*	to_nurbsobject() { return surface; }
};

/* --------------- wrapper for NURBSNBlendSurface ----------------------- */

applyable_class_s (NURBSNBlendSurfaceValue, NURBSSurfaceValue)

class NURBSNBlendSurfaceValue : public NURBSSurfaceValue
{
public:
	NURBSNBlendSurface* surface;

	ScripterExport NURBSNBlendSurfaceValue();
	ScripterExport NURBSNBlendSurfaceValue(NURBSNBlendSurface* pt);
	static ScripterExport NURBSNBlendSurfaceValue* intern(NURBSNBlendSurface* pt);
	ScripterExport ~NURBSNBlendSurfaceValue() { if (!in_set) delete surface; }

				classof_methods(NURBSNBlendSurfaceValue, NURBSSurfaceValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"

	def_generic ( setParent,	"setParent" );
	def_generic ( getParent,	"getParent" );
	def_generic ( setParentID,	"setParentID" );
	def_generic ( getParentID,	"getParentID" );
	def_generic ( setEdge,		"setEdge" );
	def_generic ( getEdge,		"getEdge" );

	// built-in property accessors

	NURBSSurface*	to_nurbssurface() { return surface; }
	NURBSObject*	to_nurbsobject() { return surface; }
};

/* --------------- wrapper for NURBSMultiCurveTrimSurfaceValue ----------------------- */

applyable_class_s (NURBSMultiCurveTrimSurfaceValue, NURBSSurfaceValue)

class NURBSMultiCurveTrimSurfaceValue : public NURBSSurfaceValue
{
public:
	NURBSMultiCurveTrimSurface* surface;

	ScripterExport NURBSMultiCurveTrimSurfaceValue();
	ScripterExport NURBSMultiCurveTrimSurfaceValue(NURBSMultiCurveTrimSurface* pt);
	static ScripterExport NURBSMultiCurveTrimSurfaceValue* intern(NURBSMultiCurveTrimSurface* pt);
	ScripterExport ~NURBSMultiCurveTrimSurfaceValue() { if (!in_set) delete surface; }

				classof_methods(NURBSMultiCurveTrimSurfaceValue, NURBSSurfaceValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"

	def_generic ( setParent,		"setParent" );
	def_generic ( getParent,		"getParent" );
	def_generic ( setParentID,		"setParentID" );
	def_generic ( getParentID,		"getParentID" );
	def_generic ( appendCurve,		"appendCurve" );
	def_generic ( appendCurveByID,	"appendCurveByID" );

	// built-in property accessors
	def_property ( numCurves );
	def_property ( flipTrim );
	def_property ( surfaceParent );
	def_property ( surfaceParentID );

	NURBSSurface*	to_nurbssurface() { return surface; }
	NURBSObject*	to_nurbsobject() { return surface; }
};

/* --------------- wrapper for NURBSFilletSurfaceValue ----------------------- */

applyable_class_s (NURBSFilletSurfaceValue, NURBSSurfaceValue)

class NURBSFilletSurfaceValue : public NURBSSurfaceValue
{
public:
	NURBSFilletSurface* surface;

	ScripterExport NURBSFilletSurfaceValue();
	ScripterExport NURBSFilletSurfaceValue(NURBSFilletSurface* pt);
	static ScripterExport NURBSFilletSurfaceValue* intern(NURBSFilletSurface* pt);
	ScripterExport ~NURBSFilletSurfaceValue() { if (!in_set) delete surface; }

				classof_methods(NURBSFilletSurfaceValue, NURBSSurfaceValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"

	def_generic ( setParent,		"setParent" );
	def_generic ( getParent,		"getParent" );
	def_generic ( setParentID,		"setParentID" );
	def_generic ( getParentID,		"getParentID" );
	def_generic ( setSeed,			"setSeed" );
	def_generic ( getSeed,			"getSeed" );
	def_generic ( getRadius,		"getRadius" );
	def_generic ( setRadius,		"setRadius" );
	def_generic ( getTrimSurface,	"getTrimSurface" );
	def_generic ( setTrimSurface,	"setTrimSurface" );
	def_generic ( getFlipTrim,		"getFlipTrim" );
	def_generic ( setFlipTrim,		"setFlipTrim" );

	// built-in property accessors
	def_property ( cubic );

	NURBSSurface*	to_nurbssurface() { return surface; }
	NURBSObject*	to_nurbsobject() { return surface; }
};

/* --------------- wrapper for NURBSOffsetSurface ----------------------- */

applyable_class_s (NURBSOffsetSurfaceValue, NURBSSurfaceValue)

class NURBSOffsetSurfaceValue : public NURBSSurfaceValue
{
public:
	NURBSOffsetSurface* surface;

	ScripterExport NURBSOffsetSurfaceValue();
	ScripterExport NURBSOffsetSurfaceValue(NURBSOffsetSurface* pt);
	static ScripterExport NURBSOffsetSurfaceValue* intern(NURBSOffsetSurface* pt);
	ScripterExport ~NURBSOffsetSurfaceValue() { if (!in_set) delete surface; }

				classof_methods(NURBSOffsetSurfaceValue, NURBSSurfaceValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"

	// built-in property accessors
	def_property ( parent );
	def_property ( parentID );
	def_property ( distance );

	NURBSSurface*	to_nurbssurface() { return surface; }
	NURBSObject*	to_nurbsobject() { return surface; }
};

/* --------------- wrapper for NURBSXFormSurface ----------------------- */

applyable_class_s (NURBSXFormSurfaceValue, NURBSSurfaceValue)

class NURBSXFormSurfaceValue : public NURBSSurfaceValue
{
public:
	NURBSXFormSurface* surface;

	ScripterExport NURBSXFormSurfaceValue();
	ScripterExport NURBSXFormSurfaceValue(NURBSXFormSurface* pt);
	static ScripterExport NURBSXFormSurfaceValue* intern(NURBSXFormSurface* pt);
	ScripterExport ~NURBSXFormSurfaceValue() { if (!in_set) delete surface; }

				classof_methods(NURBSXFormSurfaceValue, NURBSSurfaceValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"

	// built-in property accessors
	def_property ( parent );
	def_property ( parentID );
	def_property ( transform );

	NURBSSurface*	to_nurbssurface() { return surface; }
	NURBSObject*	to_nurbsobject() { return surface; }
};

/* --------------- wrapper for NURBSMirrorSurface ----------------------- */

applyable_class_s (NURBSMirrorSurfaceValue, NURBSSurfaceValue)

class NURBSMirrorSurfaceValue : public NURBSSurfaceValue
{
public:
	NURBSMirrorSurface* surface;

	ScripterExport NURBSMirrorSurfaceValue();
	ScripterExport NURBSMirrorSurfaceValue(NURBSMirrorSurface* pt);
	static ScripterExport NURBSMirrorSurfaceValue* intern(NURBSMirrorSurface* pt);
	ScripterExport ~NURBSMirrorSurfaceValue() { if (!in_set) delete surface; }

				classof_methods(NURBSMirrorSurfaceValue, NURBSSurfaceValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"

	// built-in property accessors
	def_property ( parent );
	def_property ( parentID );
	def_property ( axis );
	def_property ( distance );
	def_property ( transform );

	NURBSSurface*	to_nurbssurface() { return surface; }
	NURBSObject*	to_nurbsobject() { return surface; }
};

/* --------------- wrapper for NURBSRuledSurface ----------------------- */

applyable_class_s (NURBSRuledSurfaceValue, NURBSSurfaceValue)

class NURBSRuledSurfaceValue : public NURBSSurfaceValue
{
public:
	NURBSRuledSurface* surface;

	ScripterExport NURBSRuledSurfaceValue();
	ScripterExport NURBSRuledSurfaceValue(NURBSRuledSurface* pt);
	static ScripterExport NURBSRuledSurfaceValue* intern(NURBSRuledSurface* pt);
	ScripterExport ~NURBSRuledSurfaceValue() { if (!in_set) delete surface; }

				classof_methods(NURBSRuledSurfaceValue, NURBSSurfaceValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"

	// built-in property accessors
	def_property ( parent1 );
	def_property ( parent1ID );
	def_property ( parent2 );
	def_property ( parent2ID );
	def_property ( flip1 );
	def_property ( flip2 );
	def_property ( curveStartPoint1 );
	def_property ( curveStartPoint2 );

	NURBSSurface*	to_nurbssurface() { return surface; }
	NURBSObject*	to_nurbsobject() { return surface; }
};

/* --------------- wrapper for NURBSULoftSurface ----------------------- */

applyable_class_s (NURBSULoftSurfaceValue, NURBSSurfaceValue)

class NURBSULoftSurfaceValue : public NURBSSurfaceValue
{
public:
	NURBSULoftSurface* surface;

	ScripterExport NURBSULoftSurfaceValue();
	ScripterExport NURBSULoftSurfaceValue(NURBSULoftSurface* pt);
	static ScripterExport NURBSULoftSurfaceValue* intern(NURBSULoftSurface* pt);
	ScripterExport ~NURBSULoftSurfaceValue() { if (!in_set) delete surface; }

				classof_methods(NURBSULoftSurfaceValue, NURBSSurfaceValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"
	def_generic ( appendCurve,		"appendCurve" );
	def_generic ( appendCurveByID,	"appendCurveByID" );
	def_generic ( getCurve,			"getCurve" );
	def_generic ( getCurveID,		"getCurveID" );
	def_generic ( setCurve,			"setCurve" );
	def_generic ( setCurveByID,		"setCurveByID" );
	def_generic ( getFlip,			"getFlip" );
	def_generic ( setFlip,			"setFlip" );

	// built-in property accessors
	def_property ( numCurves );

	NURBSSurface*	to_nurbssurface() { return surface; }
	NURBSObject*	to_nurbsobject() { return surface; }
};

/* --------------- wrapper for NURBSUVLoftSurface ----------------------- */

applyable_class_s (NURBSUVLoftSurfaceValue, NURBSSurfaceValue)

class NURBSUVLoftSurfaceValue : public NURBSSurfaceValue
{
public:
	NURBSUVLoftSurface* surface;

	ScripterExport NURBSUVLoftSurfaceValue();
	ScripterExport NURBSUVLoftSurfaceValue(NURBSUVLoftSurface* pt);
	static ScripterExport NURBSUVLoftSurfaceValue* intern(NURBSUVLoftSurface* pt);
	ScripterExport ~NURBSUVLoftSurfaceValue() { if (!in_set) delete surface; }

				classof_methods(NURBSUVLoftSurfaceValue, NURBSSurfaceValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"
	def_generic ( appendUCurve,		"appendCurve" );
	def_generic ( appendUCurveByID,	"appendCurveByID" );
	def_generic ( getUCurve,		"getCurve" );
	def_generic ( getUCurveID,		"getCurveID" );
	def_generic ( setUCurve,		"setCurve" );
	def_generic ( setUCurveByID,	"setCurveByID" );

	def_generic ( appendVCurve,		"appendCurve" );
	def_generic ( appendVCurveByID,	"appendCurveByID" );
	def_generic ( getVCurve,		"getCurve" );
	def_generic ( getVCurveID,		"getCurveID" );
	def_generic ( setVCurve,		"setCurve" );
	def_generic ( setVCurveByID,	"setCurveByID" );

	// built-in property accessors
	def_property ( numUCurves );
	def_property ( numVCurves );

	NURBSSurface*	to_nurbssurface() { return surface; }
	NURBSObject*	to_nurbsobject() { return surface; }
};

/* --------------- wrapper for NURBSExtrudeSurface ----------------------- */

applyable_class_s (NURBSExtrudeSurfaceValue, NURBSSurfaceValue)

class NURBSExtrudeSurfaceValue : public NURBSSurfaceValue
{
public:
	NURBSExtrudeSurface* surface;

	ScripterExport NURBSExtrudeSurfaceValue();
	ScripterExport NURBSExtrudeSurfaceValue(NURBSExtrudeSurface* pt);
	static ScripterExport NURBSExtrudeSurfaceValue* intern(NURBSExtrudeSurface* pt);
	ScripterExport ~NURBSExtrudeSurfaceValue() { if (!in_set) delete surface; }

				classof_methods(NURBSExtrudeSurfaceValue, NURBSSurfaceValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"

	// built-in property accessors
	def_property ( parent );
	def_property ( parentID );
	def_property ( axisTM );
	def_property ( distance );
	def_property ( curveStartPoint );

	NURBSSurface*	to_nurbssurface() { return surface; }
	NURBSObject*	to_nurbsobject() { return surface; }
};

/* --------------- wrapper for NURBSLatheSurface ----------------------- */

applyable_class_s (NURBSLatheSurfaceValue, NURBSSurfaceValue)

class NURBSLatheSurfaceValue : public NURBSSurfaceValue
{
public:
	NURBSLatheSurface* surface;

	ScripterExport NURBSLatheSurfaceValue();
	ScripterExport NURBSLatheSurfaceValue(NURBSLatheSurface* pt);
	static ScripterExport NURBSLatheSurfaceValue* intern(NURBSLatheSurface* pt);
	ScripterExport ~NURBSLatheSurfaceValue() { if (!in_set) delete surface; }

				classof_methods(NURBSLatheSurfaceValue, NURBSSurfaceValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"

	// built-in property accessors
	def_property ( parent );
	def_property ( parentID );
	def_property ( axisTM );
	def_property ( sweep );
	def_property ( curveStartPoint );

	NURBSSurface*	to_nurbssurface() { return surface; }
	NURBSObject*	to_nurbsobject() { return surface; }
};

/* --------------- wrapper for NURBSCapSurface ----------------------- */

applyable_class_s (NURBSCapSurfaceValue, NURBSSurfaceValue)

class NURBSCapSurfaceValue : public NURBSSurfaceValue
{
public:
	NURBSCapSurface* surface;

	ScripterExport NURBSCapSurfaceValue();
	ScripterExport NURBSCapSurfaceValue(NURBSCapSurface* pt);
	static ScripterExport NURBSCapSurfaceValue* intern(NURBSCapSurface* pt);
	ScripterExport ~NURBSCapSurfaceValue() { if (!in_set) delete surface; }

				classof_methods(NURBSCapSurfaceValue, NURBSSurfaceValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"

	// built-in property accessors
	def_property ( parent );
	def_property ( parentID );
	def_property ( edge );
	def_property ( curveStartPoint );

	NURBSSurface*	to_nurbssurface() { return surface; }
	NURBSObject*	to_nurbsobject() { return surface; }
};

/* --------------- wrapper for NURBS1RailSweepSurface ----------------------- */

applyable_class_s (NURBS1RailSweepSurfaceValue, NURBSSurfaceValue)

class NURBS1RailSweepSurfaceValue : public NURBSSurfaceValue
{
public:
	NURBS1RailSweepSurface* surface;

	ScripterExport NURBS1RailSweepSurfaceValue();
	ScripterExport NURBS1RailSweepSurfaceValue(NURBS1RailSweepSurface* pt);
	static ScripterExport NURBS1RailSweepSurfaceValue* intern(NURBS1RailSweepSurface* pt);
	ScripterExport ~NURBS1RailSweepSurfaceValue() { if (!in_set) delete surface; }

				classof_methods(NURBS1RailSweepSurfaceValue, NURBSSurfaceValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"
	def_generic ( appendCurve,		"appendCurve" );
	def_generic ( appendCurveByID,	"appendCurveByID" );
	def_generic ( getCurve,			"getCurve" );
	def_generic ( getCurveID,		"getCurveID" );
	def_generic ( setCurve,			"setCurve" );
	def_generic ( setCurveByID,		"setCurveByID" );
	def_generic ( getFlip,			"getFlip" );
	def_generic ( setFlip,			"setFlip" );
	def_generic ( getCurveStartPoint, "getCurveStartPoint" );
	def_generic ( setCurveStartPoint, "setCurveStartPoint" );

	// built-in property accessors
	def_property ( rail );
	def_property ( railID );
	def_property ( numCurves );
	def_property ( parallel );
	def_property ( axisTM );

	NURBSSurface*	to_nurbssurface() { return surface; }
	NURBSObject*	to_nurbsobject() { return surface; }
};
/* --------------- wrapper for NURBS2RailSweepSurface ----------------------- */

applyable_class_s (NURBS2RailSweepSurfaceValue, NURBSSurfaceValue)

class NURBS2RailSweepSurfaceValue : public NURBSSurfaceValue
{
public:
	NURBS2RailSweepSurface* surface;

	ScripterExport NURBS2RailSweepSurfaceValue();
	ScripterExport NURBS2RailSweepSurfaceValue(NURBS2RailSweepSurface* pt);
	static ScripterExport NURBS2RailSweepSurfaceValue* intern(NURBS2RailSweepSurface* pt);
	ScripterExport ~NURBS2RailSweepSurfaceValue() { if (!in_set) delete surface; }

				classof_methods(NURBS2RailSweepSurfaceValue, NURBSSurfaceValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"
	def_generic ( appendCurve,		"appendCurve" );
	def_generic ( appendCurveByID,	"appendCurveByID" );
	def_generic ( getCurve,			"getCurve" );
	def_generic ( getCurveID,		"getCurveID" );
	def_generic ( setCurve,			"setCurve" );
	def_generic ( setCurveByID,		"setCurveByID" );
	def_generic ( getFlip,			"getFlip" );
	def_generic ( setFlip,			"setFlip" );
	def_generic ( getCurveStartPoint, "getCurveStartPoint" );
	def_generic ( setCurveStartPoint, "setCurveStartPoint" );

	// built-in property accessors
	def_property ( rail1 );
	def_property ( rail1ID );
	def_property ( rail2 );
	def_property ( rail2ID );
	def_property ( numCurves );
	def_property ( parallel );

	NURBSSurface*	to_nurbssurface() { return surface; }
	NURBSObject*	to_nurbsobject() { return surface; }
};

/* --------------- wrapper for NURBSDisplay ----------------------- */

applyable_class (NURBSDisplayValue)

class NURBSDisplayValue : public Value
{
public:
	NURBSDisplay	display;		// surface texture

	ScripterExport NURBSDisplayValue();
	ScripterExport NURBSDisplayValue(NURBSDisplay& surf);

				classof_methods(NURBSDisplayValue, Value);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
#	define		is_nurbsdisplay(p) ((p)->tag == class_tag(NURBSDisplayValue))
	
	// operations
#include "defimpfn.h"

	// built-in property accessors
	def_property ( displayCurves );
	def_property ( displaySurfaces );
	def_property ( displayLattices );
	def_property ( displaySurfCVLattices );
	def_property ( displayCurveCVLattices );
	def_property ( displayDependents );
	def_property ( displayTrimming );
	def_property ( degradeOnMove );
	def_property ( displayShadedLattice );

	NURBSDisplay*	to_nurbsdisplay() { return &display; }
};

/* --------------- wrapper for NURBSSurfaceApproximation ----------------------- */

applyable_class (NURBSSurfaceApproximationValue)

class NURBSSurfaceApproximationValue : public Value
{
public:
	TessApprox	tess;		// surface approximation

	ScripterExport NURBSSurfaceApproximationValue();
	ScripterExport NURBSSurfaceApproximationValue(TessApprox& surf);

				classof_methods(NURBSSurfaceApproximationValue, Value);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
#	define		is_NURBSSurfaceApproximation(p) ((p)->tag == class_tag(NURBSSurfaceApproximationValue))
	
	// operations
#include "defimpfn.h"

	// built-in property accessors
	def_property ( config );
	def_property ( isoULines );
	def_property ( isoVLines );
	def_property ( meshUSteps );
	def_property ( meshVSteps );
	def_property ( meshApproxType );
	def_property ( spacialEdge );
	def_property ( curvatureAngle );
	def_property ( curvatureDistance );
	def_property ( viewDependent );

	TessApprox*	to_tessapprox() { return &tess; }
};

/* --------------- wrapper for NURBSTexturePoint ----------------------- */

applyable_class_s (NURBSTexturePointValue, NURBSObjectValue)

class NURBSTexturePointValue : public NURBSObjectValue
{
public:
	NURBSTexturePoint* point;

	ScripterExport NURBSTexturePointValue();
	ScripterExport NURBSTexturePointValue(Point2 pt);
	ScripterExport NURBSTexturePointValue(NURBSTexturePoint* pt);
	static ScripterExport NURBSTexturePointValue* intern(NURBSTexturePoint* pt);
	ScripterExport ~NURBSTexturePointValue() { if (!in_set) delete point; }

				classof_methods(NURBSTexturePointValue, NURBSObjectValue);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	
	// operations
#include "defimpfn.h"
	// built-in property accessors
	def_property ( pos );

	def_generic ( setIndices,		"setIndices" );

	NURBSTexturePoint*	to_nurbstexturepoint() { return point; }
	NURBSObject*		to_nurbsobject() { return point; }
};

/* --------------- wrapper for NURBSSet ----------------------- */

applyable_class (NURBSSetValue)

class NURBSSetValue : public Value
{
public:
	NURBSSet set;							// wrapped NURBSet

	ScripterExport NURBSSetValue();
	ScripterExport NURBSSetValue(NURBSSet& set);

				classof_methods(NURBSSetValue, Value);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
#	define		is_nurbsset(p) ((p)->tag == class_tag(NURBSSetValue))
	NURBSObject* sel_index_to_obj(int i);

	// operations
#include "defimpfn.h"
	def_generic ( getObject,		"getObject" );
	def_generic ( setObject,		"setObject" );
	def_generic ( appendObject,		"appendObject" );
	def_generic ( removeObject,		"removeObject" );
	def_generic ( deleteObjects,	"deleteObjects" );
	def_generic ( disconnect,		"disconnect" );
	def_generic ( getProdTess,		"getProdTess" );
	def_generic ( setProdTess,		"setProdTess" );
	def_generic ( getViewTess,		"getViewTess" );
	def_generic ( setViewTess,		"setViewTess" );
	def_generic ( clearViewTess,	"clearViewTess" );
	def_generic ( clearProdTess,	"clearProdTess" );

	// built-in property accessors
	def_prop_getter ( numObjects );

	// tesselation properties
	def_property ( viewConfig );
	def_property ( viewIsoULines );
	def_property ( viewIsoVLines );
	def_property ( viewMeshUSteps );
	def_property ( viewMeshVSteps );
	def_property ( viewMeshApproxType );
	def_property ( viewSpacialEdge );
	def_property ( viewCurvatureAngle );
	def_property ( viewCurvatureDistance );
	def_property ( viewViewDependent );
	def_property ( renderConfig );
	def_property ( renderIsoULines );
	def_property ( renderIsoVLines );
	def_property ( renderMeshUSteps );
	def_property ( renderMeshVSteps );
	def_property ( renderMeshApproxType );
	def_property ( renderSpacialEdge );
	def_property ( renderCurvatureAngle );
	def_property ( renderCurvatureDistance );
	def_property ( renderViewDependent );
	def_property ( merge );
	def_property ( display );
	def_property ( viewApproximation );
	def_property ( renderApproximation );

	NURBSSet*	to_nurbsset() { return &set; }

	// add array protocol
	def_generic ( get,		"get" );
	def_generic ( put,		"put" );
	ScripterExport Value* map(node_map& m);

	def_prop_getter(count);
};

extern void check_nurbs_result(NURBSResult r);

#endif
#endif
