// 3dsurfer.h

#include "istdplug.h"

class SurferPatchDataReaderCallback : public ObjectDataReaderCallback {
public:
	char *DataName () { return "MinervaSoftware_Patch_3"; }
	Object *ReadData (TriObject *tri, void *data, DWORD len);
	void DeleteThis() { return; }
};

class SurferSplineDataReaderCallback : public ObjectDataReaderCallback {
public:
	char *DataName () { return "MinervaSoftware_Spline_2"; }
	Object *ReadData (TriObject *tri, void *data, DWORD len);
	void DeleteThis () { return; }
};
