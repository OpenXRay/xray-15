/*******************************************************************
 *
 *    DESCRIPTION: euler.h
 *
 *    AUTHOR: Converted from Ken Shoemake's Graphics Gems IV code by Dan Silva
 *
 *    HISTORY:  converted 11/21/96
 *
 *              RB: This file provides only a subset of those
 *                  found in the original Graphics Gems paper.
 *                  All orderings are 'static axis'.
 *
 *******************************************************************/

#ifndef __EULER__
#define __EULER__

#include "matrix3.h"
#include "quat.h"

#define EULERTYPE_XYZ	0
#define EULERTYPE_XZY	1
#define EULERTYPE_YZX	2
#define EULERTYPE_YXZ	3
#define EULERTYPE_ZXY	4
#define EULERTYPE_ZYX	5
#define EULERTYPE_XYX	6
#define EULERTYPE_YZY	7
#define EULERTYPE_ZXZ	8

#define EULERTYPE_RF    16  // rotating frame (axes)  --prs.

void DllExport QuatToEuler(const Quat &q, float *ang, int type, bool flag = false);	// flag added 001101  --prs.
void DllExport EulerToQuat(float *ang, Quat &q,int type);
void DllExport MatrixToEuler(const Matrix3 &mat, float *ang, int type, bool flag = FALSE);
void DllExport EulerToMatrix(float *ang, Matrix3 &mat, int type);
float DllExport GetEulerQuatAngleRatio(Quat &quat1,Quat &quat2, float *euler1, float *euler2, int type = EULERTYPE_XYZ);
float DllExport GetEulerMatAngleRatio(Matrix3 &mat1,Matrix3 &mat2, float *euler1, float *euler2, int type = EULERTYPE_XYZ);


class RotationValue {
//
// There are two classes of Euler angle types, that of which rotation
// axes are not repeated, from kXYZ to kZYX, and one of which an axis is
// repeated, from kXYX to kZXZ. Repeated types starts from kReptd.
// For non-repetitive Euler angles, there are two well-defined methods
// to associate three ordered angles, to axes. First, we can associate
// them with x-, y-, and z-, axes in order. The first angle, for example,
// is always associated with the x-axis, no matter where it appears
// in the Euler type (x-axis appears at second place in kZXY, for example).
// Second, we can associate them by position: the first angle is always
// associated with the first axis in the Euler type. For examples,
// the first angle is applied to the x-axis for kXYZ and kXZY, to the
// y-axis for kYXZ and kYZX, etc.
// Let's call the first method by (axis) name, and the second method
// by order. We associate angles by name for non-repetitive Euler types.
// For repetitive types, by-name is not well-defined, because there is
// a missing axis. For repetitive types, we associate angles by-order.
// Example:
//   Point3 a = Euler(RotationValue::kZYX);
// then, a.x, a.y, a.z, are the Euler angles for the x-axis, y-axis,
// and z-axis.
//   Point3 a = Euler(RotationValue::kZXZ);
// then, a.x is angle applied to the first z-axis (from left), a.y is
// applied to the x-axis, and a.z is applied to the second z-axis.
//
public:
	enum EulerType {
		kXYZ = EULERTYPE_XYZ,
		kXZY,
		kYZX,
		kYXZ,
		kZXY,
		kZYX,
		kXYX,
		kYZY,
		kZXZ
	};
	enum {
		kReptd = kXYX,
		kQuat = 100
	};
	static bool IsEuler(int rep) { return (kXYZ <= rep && rep <= kZXZ); }
	static bool IsRepetitive(int rep) {
		// Pre-cond: IsEuler(rep)
		return rep >= kReptd; }
	static bool IsQuat(int rep) { return rep == kQuat; }

	void Set(const Point3& a, EulerType et) {
		mQ.x = a.x;
		mQ.y = a.y;
		mQ.z = a.z;
		mRep = et; }
	void Set(const Quat& q) {
		mQ = q;
		mRep = kQuat; }
	RotationValue() : mQ(), mRep(kXYZ) {}
	RotationValue(const Point3& a, EulerType et) { Set(a, et); }
	RotationValue(const Quat& q) { Set(q); }
	RotationValue(const RotationValue& src) : mQ(src.mQ), mRep(src.mRep) {}

	Point3 Euler(EulerType et =kXYZ) const {
		if (et == mRep) return Point3(mQ.x, mQ.y, mQ.z);
		else return ToEulerAngles(et); }
	operator Quat() const {
		if (mRep == kQuat) return mQ;
		else return ToQuat(); }
	DllExport operator Matrix3() const;
	DllExport void PreApplyTo(Matrix3& m) const; // m = *this * m
	DllExport void PostApplyTo(Matrix3& m) const; // m = m * *this
	// *this = *this * aa
	// Post-condition: NativeRep() will be changed after RotateBy()
	DllExport void PostRotate(const AngAxis& aa);

	int		NativeRep() const { return mRep; }
	Quat	GetNative() const { return mQ; }

	// Suppose a and o are Point3's, which holds Euler angles by axis name
	// and by order, respectively. That is, a[0] is applied to the x-axis and
	// o[0] is applied to the first axis (from left in the Euler type).
	// Let et be non-repetitive Euler type. Then,
	//		o[kAxisToOrdinal[et][i]] = a[i]
	// and	a[kOrdinalToAxis[et][i]] = o[i]
	//
	static const int kAxisToOrdinal[kReptd][3];
	static const int kOrdinalToAxis[kZXZ+1][3];

protected:
	Point3 DllExport	ToEulerAngles(EulerType et) const;
	Quat   DllExport	ToQuat() const;

private:
	Quat	mQ;
	short	mRep;
};

#endif // __EULER__
