/**********************************************************************
 *<
	FILE:  marketDefaults.h

	DESCRIPTION:  Market Specific Defaults API

	CREATED BY: Cleve Ard

	HISTORY: created 5/19/2003

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#ifndef __MARKETDEFAULTS_H__
#define __MARKETDEFAULTS_H__

class MarketDefaults;

// Get the market default settings. We can handle multiple
// files, so third parties can add there own market defaults.
// Name should be the name, without and extension, for the
// default to be retrieved. NULL will retrieve the defaults
// for Autodesk/Discreet settings.
CoreExport MarketDefaults* GetMarketDefaults(const TCHAR* name = NULL);

// Get a filename that is in the current MarketDefaults set.
// If the file doesn't exist in the MarketDefaults set and
// does exits in the factoryDefaults for the MarketDefaults
// set it is copied to the MarketDefault set.
CoreExport TSTR GetMarketDefaultsFileName(const TCHAR* name = NULL);

// This class is exported from the marketDefaults.dll
class MarketDefaults {
public:
	typedef bool (*FloatValidator)(float&);
	typedef bool (*IntValidator)(int&);
	typedef bool (*Point3Validator)(Point3&);
	typedef bool (*Point4Validator)(Point4&);
	typedef bool (*StringValidator)(TSTR&);
	typedef bool (*AnimatableValidator)(Animatable*&);
	typedef bool (*ClassIDValidator)(Class_ID&);

	// This class generates range checkers for values. 
	template<class T, int low, int high>
	class Range;

	// This function rejects NULL animatable objects
	CoreExport static bool CheckNULL(Animatable*& obj);

	// This function rejects Null strings
	CoreExport static bool CheckNULL(TSTR& str);

	// This function rejects Class_ID(0, 0) and
	// Class_ID(STANDIN_CLASS_ID, 0)
	CoreExport static bool CheckNULL(Class_ID& id);

	// High level access to defaults. These return the default
	// value, based on the class ID of the object and the name
	// of the setting. A default value is supplied, to be used
	// when the setting name is not present for the class ID.
	// The validator and dimension are only used for setting
	// names that are found. The dimension is used to convert
	// the UI values in the settings file to internal values
	// before the validator is called. The validator may change
	// the value. It returns true if the retrieved value is to
	// be used, or false if the default value is to be used.
	virtual int GetInt(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const TCHAR*		name,
		int					defaultValue,
		IntValidator		validator = NULL,
		int					dimension = DIM_NONE
	) = 0;

	int GetTime(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const TCHAR*		name,
		int					defaultValue,
		IntValidator		validator = NULL
	)
	{
		return GetInt(objectSuperClass, objectClass, name, defaultValue, validator, DIM_TIME);
	}

	virtual float GetFloat(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const TCHAR*		name,
		float				defaultValue,
		FloatValidator		validator = NULL,
		int					dimension = DIM_NONE
	) = 0;

	float GetAngle(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const TCHAR*		name,
		float				defaultValue,
		FloatValidator		validator = NULL
	)
	{
		return GetFloat(objectSuperClass, objectClass, name, defaultValue, validator, DIM_ANGLE);
	}

	float GetPercentage(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const TCHAR*		name,
		float				defaultValue,
		FloatValidator		validator = NULL
	)
	{
		return GetFloat(objectSuperClass, objectClass, name, defaultValue, validator, DIM_PERCENT);
	}

	float GetWorld(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const TCHAR*		name,
		float				defaultValue,
		FloatValidator		validator = NULL
	)
	{
		return GetFloat(objectSuperClass, objectClass, name, defaultValue, validator, DIM_WORLD);
	}

	float GetColorChannel(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const TCHAR*		name,
		float				defaultValue,
		FloatValidator		validator = NULL
	)
	{
		return GetFloat(objectSuperClass, objectClass, name, defaultValue, validator, DIM_COLOR255);
	}

	virtual Point3 GetPoint3(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const TCHAR*		name,
		const Point3&		defaultValue,
		Point3Validator		validator = NULL,
		int					dimension = DIM_NONE
	) = 0;

	Point3 GetRGBA(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const TCHAR*		name,
		const Point3&		defaultValue,
		Point3Validator		validator = NULL
	)
	{
		return GetPoint3(objectSuperClass, objectClass, name, defaultValue, validator, DIM_COLOR255);
	}

	virtual Point4 GetPoint4(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const TCHAR*		name,
		const Point4&		defaultValue,
		Point4Validator		validator = NULL,
		int					dimension = DIM_NONE
		) = 0;

	Point4 GetFRGBA(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const TCHAR*		name,
		const Point4&		defaultValue,
		Point4Validator		validator = NULL
		)
	{
		return GetPoint4(objectSuperClass, objectClass, name, defaultValue, validator, DIM_COLOR);
	}

	virtual TSTR GetString(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const TCHAR*		name,
		const TCHAR*		defaultValue,
		StringValidator		validator = NULL
	) = 0;

	virtual Class_ID GetClassID(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const TCHAR*		name,
		const Class_ID&		defaultID,
		ClassIDValidator	validator = NULL
	) = 0;

	virtual Animatable* CreateInstance(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const TCHAR*		name,
		SClass_ID			superClass,
		const Class_ID&		defaultInstance,
		AnimatableValidator	validator = NULL
	) = 0;
	
	ReferenceTarget* CreateRefTarget(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const TCHAR*		name,
		SClass_ID			superClass,
		const Class_ID&		defaultInstance,
		AnimatableValidator	validator = NULL
	)
	{
		return static_cast<ReferenceTarget*>(CreateInstance(objectSuperClass,
			objectClass, name, superClass, defaultInstance, validator));
	}

	Mtl* CreateMtl(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const TCHAR*		name,
		const Class_ID&		defaultInstance,
		AnimatableValidator	validator = NULL
	)
	{
		return static_cast<Mtl*>(CreateInstance(objectSuperClass, objectClass,
			name, MATERIAL_CLASS_ID, defaultInstance, validator));
	}

	Texmap* CreateTexmap(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const TCHAR*		name,
		const Class_ID&		defaultInstance,
		AnimatableValidator	validator = NULL
	)
	{
		return static_cast<Texmap*>(CreateInstance(objectSuperClass, objectClass,
			name, TEXMAP_CLASS_ID, defaultInstance, validator));
	}

	// This is the low level interface into the defaults.
	// This interface lets you retrive raw values directly from the
	// defaults.
	virtual bool SectionExists(const TCHAR* section) = 0;
	virtual bool SectionExists(SClass_ID sid, const Class_ID& id) = 0;
	virtual void SetSectionName(const TCHAR* section) = 0;
	virtual void SetSectionName(SClass_ID sid, const Class_ID& id) = 0;
	virtual const TCHAR* GetSectionName() = 0;
	virtual void DeleteSection(const TCHAR* section) = 0;
	virtual void DeleteSection(SClass_ID sid, const Class_ID& id) = 0;

	virtual bool KeyExists(const TCHAR* key) = 0;
	virtual bool DeleteKey(const TCHAR* key) = 0;

	virtual bool PutInt(
		const TCHAR*	key,
		int				val
	) = 0;
	virtual bool PutIntArray(
		const TCHAR*	key,
		int				ct,
		const int*		array
	) = 0;
	virtual bool PutFloat(
		const TCHAR*	key,
		float val
	) = 0;
	virtual bool PutFloatArray(
		const TCHAR*	key,
		int				ct,
		const float*	array
	) = 0; 
	virtual bool PutString(
		const TCHAR*	key,
		const TCHAR*	str
	) = 0;

	virtual bool GetInt(
		const TCHAR*	key,
		int&			val
	) = 0;
	virtual bool GetIntArray(
		const TCHAR*	key,
		int&			ct,
		int*			array,
		int				arrayCt
	) = 0;
	virtual bool GetFloat(
		const TCHAR*	key,
		float&			val
	) = 0;
	virtual bool GetFloatArray(
		const TCHAR*	key,
		int&			ct,
		float*			array,
		int				arrayCt
	) = 0;
	virtual bool GetString(
		const TCHAR*	key,
		TCHAR*			buf,
		int				bufSize
	) = 0;
};



// This class generates range checkers for values. 
template<class T, int low, int high>
class MarketDefaults::Range {
public:
	// Return whether val is in the range including the endpoints
	static bool CheckII(T& val);

	// Return whether val is in the range excluding the low endpoint
	static bool CheckEI(T& val);

	// Return whether val is in the range excluding the high endpoint
	static bool CheckIE(T& val);

	// Return whether val is in the range excluding the endpoints
	static bool CheckEE(T& val);

	// Correct val to fall within the givin range including the endpoints
	static bool Correct(T& val);

	// Correct val if it is above the given range. Fail if it is below.
	static bool CorrectHigh(T& val);

	// Correct val if it is below the given range. Fail if it is above.
	static bool CorrectLow(T& val);
};

template<class T, int low, int high>
bool MarketDefaults::Range<T, low, high>::CheckII(T& val)
{
	return val >= T(low) && val <= T(high);
}

template<class T, int low, int high>
bool MarketDefaults::Range<T, low, high>::CheckEI(T& val)
{
	return val > T(low) && val <= T(high);
}

template<class T, int low, int high>
bool MarketDefaults::Range<T, low, high>::CheckIE(T& val)
{
	return val >= T(low) && val < T(high);
}

template<class T, int low, int high>
bool MarketDefaults::Range<T, low, high>::CheckEE(T& val)
{
	return val > T(low) && val < T(high);
}

template<class T, int low, int high>
bool MarketDefaults::Range<T, low, high>::Correct(T& val)
{
	if (val < T(low))
		val = T(low);
	else if (val > T(high))
		val = T(high);
	return true;
}

template<class T, int low, int high>
bool MarketDefaults::Range<T, low, high>::CorrectHigh(T& val)
{
	if (val < T(low))
		return false;
	if (val > T(high))
		val = T(high);
	return true;
}

template<class T, int low, int high>
bool MarketDefaults::Range<T, low, high>::CorrectLow(T& val)
{
	if (val > T(high))
		return false;
	if (val < T(low))
		val = T(low);
	return true;
}

#endif
