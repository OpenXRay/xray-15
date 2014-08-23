#ifndef __BONESMIRRORDEF__H
#define __BONESMIRRORDEF__H

class BonesDefMod;
class BoneModData;







#define NOT_MIRRORED		0
#define POS_MIRRORED		1
#define NEG_MIRRORED		2


//Just our parm block for the mirror rollup
class MirrorMapDlgProc : public ParamMap2UserDlgProc {
	public:
		BonesDefMod *mod;		
		MirrorMapDlgProc(BonesDefMod *m) {mod = m;}		
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {
				delete this;
				}
	};

class VMirrorData
{
public:
	int index;
	int flags;
};

//this is where we store our vertex mirror data
class MirrorVertexData
{
public:

	MirrorVertexData();
	//local data which holds local pos and selection
	BoneModData *bmd;


	Tab<VMirrorData> vertexMirrorList;  //list of our mirror connections
};


//this is where we store our bone mirror data
class MirrorBoneData
{
public:

	MirrorBoneData()
		{
		flags = 0;
		index = -1;
		node = NULL;

		pa = Point3(0.0f,0.0f,0.0f);
		pb = Point3(0.0f,0.0f,0.0f);

		initialBounds.Init();
		currentBounds.Init();

		selected = FALSE;

		}

	INode *node;		//the node of the bone
	int flags;			//some flags to store attribs of the bone
	int index;			//the index of the matching mirror bone
	Point3 pa,pb;		// the envelope end points in bone space
	Box3 initialBounds;  //initial bounds of the bones in world space
	Box3 currentBounds;  //current bounds of the bones in world space
	BOOL selected;		//whether the bone is selected
	
};

class MirrorData
{
	public:

		//Constructor
		MirrorData();
		//Destructor
		~MirrorData();

		//Frees all our data
		void Free();

		void DrawBounds(GraphicsWindow *gw, Box3 bounds);
		//Draw display overrid
		int DisplayMirrorData(GraphicsWindow *gw);
		//Hittest override



		//Builds our mirror bone connections
		//should ve called when ever a mirror 
		void BuildBonesMirrorData();
		//Builds our vertex mirror data
		void BuildVertexMirrorData();

		//Initialize all our connections to skin
		void InitializeData(BonesDefMod *mod);

		//the actual pasting of the mirror data
		//using the current selection
		void Paste();

		void PasteAllBones(BOOL posDir);
		void PasteAllVertices(BOOL posDir);

		//given 2 bones paste across the mirror plane
		void PasteBones(TimeValue t, Matrix3 mtm, int mirrorPlane, int sourceBone, int destBone);

		//Whether the mirror mode is enabled
		BOOL Enabled();

		HWND hWnd; // the handle to the mirror rollup


		void EnableMirrorButton(BOOL enable);	//this enables the mirror button
		void EnableUIButton(BOOL enable);		//this enables the UI buttons 


		Box3 GetMirrorBounds() {return mirrorBounds;}	//returns the bounds of the mirror plane
		Matrix3 GetInitialTM() {return initialTM;}		//returns the tm that defines the mirror plane
		void SetInitialTM(Matrix3 tm) {initialTM = tm;}		//this sets the initial tm that define the mirror plane


		void GetMirrorTM(Matrix3 &tm, int &dir);  //this returns the mirror tm and direction of the mirror plane



		Box3 worldBounds;	//just a debug var

		//clears the bone slection
		void ClearBoneSelection();
		
		//selects an individual bone
		//bone the bone to select
		//sel whether to unselect or select it
		void SelectBone(int bone, BOOL  sel);
		//selects a bunch bones
		//sel the bitarray that defines the selection
		void SelectBones(BitArray sel);

		//takes the current bone selection and emit it as a script
		void EmitBoneSelectionScript();

		UniformGrid ngrid;	//this is a grid data struct used for fast look up of closest vertices
		UniformGrid pgrid;	//this is a grid data struct used for fast look up of closest vertices

	private:

		
		Tab<MirrorBoneData> bonesMirrorList;		//list of bone mirror data
		Tab<MirrorVertexData*> localMirrorData;		//list of vertex mirror data

		BonesDefMod *mod;	//Pointer the skin modifier

		Matrix3 initialTM;  //This is the matrix of the first object that we hit that has skin
							//this is used as the reference matrix for mirroring for direction and the
							// origin

		Box3 mirrorBounds;	//just the bounds of teh object


		BOOL fastEngine;


};

#endif
