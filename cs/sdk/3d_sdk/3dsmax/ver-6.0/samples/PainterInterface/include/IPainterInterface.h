

#ifndef __IPAINTERINTERFACE__H
#define __IPAINTERINTERFACE__H

/********************************************************************************

  IPainterInterface contains all the interfaces to let a plugin interact with
  Maxs Paint system

  Basically the system consists of 2 components the IPainterInterface and IPainterCanvasInterface

  For a plugin to use the paint system it must sub class of IPainterCanvasInterface_V5 or greater 
  and implement the virtual methods and be a sub class of ReferenceTarget.  A canvas is what the 
  painterInterface paints on. Basically  IPainterCanvasInterface_V5 consists of functions to 
  identify the canvas version and functions that are called a as the system paints, basically a 
  start stroke, stroke and endstroke.

  The IPainterInterface is what does all the hit testing against the mesh using a quad tree.  
  Basically it tells the canvas what the stroke looks like.  In addition the canvas can use it
  do additional hit testing not associated with a stroke.

  In its simplest from all that needs to be done is
  
	1.  Subclass your plugin from IPainterCanvasInterface_V5 and ReferenceTarget
	2.  Fill out these virtually methods

		StartStroke() - called when a stroke is started
		PaintStroke()/PaintStroke(<..>) - called as a stroke is going on
		EndStroke() - called when the stroke ends and accepted.
		EndStroke(<...>) - called when the system is set to non interactive mode.
		CancelStroke() - called if a stroke is canceled.
		SystemEndPaintSession() - is called when the painter wants to end a paint session for some reason
		See the header for me description of the parameters

        You also need to add the painter interface to the GetInterface method from ReferenceTarget.  
		Something along the lines below.  This lets the painter set what interfaces the canvas supports.
			
			  void* PainterTester::GetInterface(ULONG id)
				{
				switch(id)
					{
					case PAINTERCANVASINTERFACE_V5 : return (IPainterCanvasInterface_V5 *) this;
						break;
					default: return ReferenceTarget::GetInterface(id);
					break;
					}
				}



    3.  In addition to implementing these methods, The plugin needs to hook up to 
	    the painterInterface.  Using these functions.  The pointer to the painter 
		can be gotten by using the CreateInstance(REF_TARGET_CLASS_ID,PAINTERINTERFACE_CLASS_ID). 
		Once that pointer is gotten you need to grab a specific version of the painterInterface
		using GetPaintInterface.   See sample below

	
			ReferenceTarget *painterRef = (ReferenceTarge *) GetCOREInterface()->CreateInstance(REF_TARGET_CLASS_ID,PAINTERINTERFACE_CLASS_ID);	
//set it to the correct verion
			if (painterRef)			
				IPainterInterface_V5 *painter = (IPainterInterface_V5 *) painterRef->GetInterface(PAINTERINTERFACE_V5);

		Once you have a valid pointer you need to hook your self up to the Painter using the
		InitializeCallback method.  This is should be done just before you want to start a paint seesion.
		All you are doing is passing a pointer of yourself to the painter system so it knows where 
		to send the messages.  Note yourself to IPainterCanvasInterface	since the Painter will 
		interogate you to find out which version of the canvas you are.

			painter->InitializeCallback((ReferenceTarget *) this);

		Once you have called InitializedCallback and are ready to start a paint session you need load up which 
		Nodes you want to paint on.  This is done with InitializeNodes(int flags, Tab<INode*> &nodeList).

				painter->InitializeNodes(0, nodes);

		Then StartPaintSession() is called.  Once this is called everytime the the users drag paints across
		the nodes the Canvas's StartStroke/PaintStroke/EndStroke will get called.
		
				painter->StartPaintSession();

		If the canvas changes the topology or geometry of the painted nodes UpdateMeshes() needs to be called
		so the quad tree gets updated.

				painter->UpdateMeshes(BOOL updatePoitnCache);

		
		Once the canvas is done with the painter it needs to call EndPaintSession() so that all the data can
		be freed and it can be unhooked.  Otherwise the StartStroke/PaintStroke/EndStroke will still be called.

				painter->EndPaintSession();

		You can also bring up the Painters Option dialog using the BringUpOptions().

				painter->BringUpOptions() ;

		In addition there will be some helper functions for gathering points within a stroke and their weigths; and 
		tools for intersecting rays against the quad tree.  See the header for more descriptions.


		See the PaintTester project for a simple implementation of a canvas and interaction with the painter.



*********************************************************************************/


#include "iFnPub.h"
#include "icurvctl.h"


#define PAINTERINTERFACE_V5 0x78ffe181
#define PAINTERCANVASINTERFACE_V5 0x29ff7ac9
#define PAINTERCANVASINTERFACE_V5_1 0x29ff7ad0

#define PAINTERINTERFACE_CLASS_ID	Class_ID(0x2f2ef7e9, 0x78ffe181)

#define PAINTERINTERFACEV5_INTERFACE Interface_ID(0x53b4520c, 0x29ff7ac9)

//These are defines used to when mirroring is enabled with point gathering
//a point can be hit by the original brush, the mirror brushed, or both
#define NO_MIRRROR		0
#define MIRRRORED		1
#define MIRRROR_SHARED	2


//These are IDs for the colors of the brush for use with the color manager
#define			RINGCOLOR			0x445408e0
#define			NORMALCOLOR			0x445408e1
#define			RINGPRESSEDCOLOR	0x445408e2
#define			NORMALPRESSEDCOLOR	0x445408e3
#define			TRACECOLOR			0x445408e4
#define			GIZMOCOLOR			0x445408e5

//These are IDs to define what the pressure effects on a stroke
#define PRESSURE_AFFECTS_NONE	0
#define PRESSURE_AFFECTS_STR	1
#define PRESSURE_AFFECTS_SIZE	2
#define PRESSURE_AFFECTS_BOTH	3





		

class IPainterInterface_V5
	{
	public:
	
//these are the esswential tools to get you started		
//they cover the basics to get you started


		//the canvas passes a pointer of itself to the painter, so the painter
		//knows where to send the stroke messages
		//		ReferenceTarget *canvas - is the pointer to the canvas
		virtual BOOL  InitializeCallback(ReferenceTarget *canvas) = 0;//tested

		//this loads of the nodes that you want to paint on, anytime you want
		//to add delete a node this must be called
		//		int flags				- not yet implemented does nothing but there to allow 
		//								  flags per node for special conditions
		//		Tab<INode*> &nodeList   - a table of nodes that you want to paint on
		virtual BOOL  InitializeNodes(int flags, Tab<INode*> &nodeList) = 0;//tested

		//This forces the quadtree to rebuilt.  Any time you change a geometry or togopology
		//of a node you passed to the InitializedNodes methode this must be called.
		//Ideally I could listen to the notifyremessage changed and rebuild on that 
		//but the since rebuilding the quad tree is inetensive, I leave it up to the 
		//canvas as to when to rebuild so we can better control the amount of stack reevals
		//   BOOL updatePointGather - determines whether the pointGather data gets updated also
		//							  normally if your mesh does not change shape or topo you dont
		//							  need to update the pointgather.  For instance ifyou rotate a view
		//							  your quad tree needs to get updated but not the point list
		virtual BOOL  UpdateMeshes(BOOL updatePointGather) = 0;

		//This is called when the a canvas wants to start a paint session.
		virtual BOOL  StartPaintSession() = 0;//tested

		//This is called when the a canvas wants to end a paint session.
		virtual BOOL  EndPaintSession() = 0;//tested


		//this return whether the user is in the paint mode, so a plugin can 
		//determine how to paint the UI paint button
		virtual BOOL  InPaintMode()=0;//tested

		//this brings up the Painter Options dialog that lets the users sets various
		//setting of the painter system
		virtual BOOL  BringUpOptions() = 0;//tested

		//This lets you access the time stamp of each sample of a stroke.  This lets you look at 
		//the acceleration of the mouse as it moves if say you wanted to paint a stroke and use
		//it for an animation path.  
		virtual int *RetrieveTimeList(int &ct) = 0;

//These functions are additional tools to further hit testing and point gathering
//to help with finding points on a mesh that are within the stroke and theoir weights

		//this function lets you hit test against the quad tree given a mouse coord
		//it returns TRUE if that point insterects the quad tree
		// 		  IPoint2 mousePos  - the position in screen space that you want to hit test agains
		//		  Point3 &worldPoint, Point3 &worldNormal - the world hit point normal and position
		//		  Point3 &localPoint, Point3 &localNormal - the local hit point normal and position
		//		  Point3 &bary - the barycentry coord of the face that was hit
		//		  int &index - the index of the face that was hit
		//		  INode *node - the node that was hit
		//		  BOOL &mirrorOn - whether mirroring was on or off
		//		  Point3 &worldMirrorPoint, Point3 &worldMirrorNormal - the world hit point normal and position after it was mirrored
		//		  Point3 &localMirrorPoint, Point3 &localMirrorNormal - the local hit point normal and position after it was mirrored
		virtual BOOL TestHit(
						  IPoint2 mousePos,
						  Point3 &worldPoint, Point3 &worldNormal,
						  Point3 &localPoint, Point3 &localNormal,
						  Point3 &bary,  int &index,
						  INode *node,
						  BOOL &mirrorOn,
						  Point3 &worldMirrorPoint, Point3 &worldMirrorNormal,
						  Point3 &localMirrorPoint, Point3 &localMirrorNormal
						  ) = 0;

		//Retrieves a random hit point around the last hit point or a specified hit point within the brush 
		//Useful if you want to do airbrush type effects or to just sample around the hit point
		//it returns FALSE if a hit point was not found
		//		  Point3 &worldPoint, Point3 &worldNormal - the world hit point normal and position
		//		  Point3 &localPoint, Point3 &localNormal - the local hit point normal and position
		//		  Point3 &bary - the barycentry coord of the face that was hit
		//		  int &index - the index of the face that was hit
		//		  float &strFromFalloff - the strength of the point based on the fall off of the brush
		//		  INode *node - the node that was hit
		//		  BOOL &mirrorOn - whether mirroring was on or off
		//		  Point3 &worldMirrorPoint, Point3 &worldMirrorNormal - the world hit point normal and position after it was mirrored
		//		  Point3 &localMirrorPoint, Point3 &localMirrorNormal - the local hit point normal and position after it was mirrored
		//		  tabIndex is what hit you want to sample around if 0 or less it will hit around the last hit test
		virtual BOOL RandomHit(Point3 &worldPoint, Point3 &worldNormal,
						  Point3 &localPoint, Point3 &localNormal,
						  Point3 &bary,  int &index,
						  float &strFromFalloff, INode *node,
						  BOOL &mirrorOn,
						  Point3 &worldMirrorPoint, Point3 &worldMirrorNormal,
						  Point3 &localMirrorPoint, Point3 &localMirrorNormal,
						  int tabIndex) = 0;

		//This will do random hit point along the stroke segment
		//It will return FALSE if it does not find a hit
		//		  Point3 &worldPoint, Point3 &worldNormal - the world hit point normal and position
		//		  Point3 &localPoint, Point3 &localNormal - the local hit point normal and position
		//		  Point3 &bary - the barycentry coord of the face that was hit
		//		  int &index - the index of the face that was hit
		//		  float &strFromFalloff - the strength of the point based on the fall off of the brush
		//		  INode *node - the node that was hit
		//		  BOOL &mirrorOn - whether mirroring was on or off
		//		  Point3 &worldMirrorPoint, Point3 &worldMirrorNormal - the world hit point normal and position after it was mirrored
		//		  Point3 &localMirrorPoint, Point3 &localMirrorNormal - the local hit point normal and position after it was mirrored
		//		  tabIndex is what segment you want to sample around if 0 or less it will hit around the last segment

		virtual BOOL RandomHitAlongStroke(Point3 &worldPoint, Point3 &worldNormal,
						  Point3 &localPoint, Point3 &localNormal,
						  Point3 &bary,  int &index,
						  float &strFromFalloff, INode *node,
						  BOOL &mirrorOn,
						  Point3 &worldMirrorPoint, Point3 &worldMirrorNormal,
						  Point3 &localMirrorPoint, Point3 &localMirrorNormal,
						  int tabIndex) = 0;


	//These next 2 methods are used if you want to do a custom stroke.   Say for instance
	//you wanted to just stroke a straight line, by default the painter uses a path stroke.
	//so what you do on the PaintStroke method record the first and last mousePos, clear all the 
	//stroke data and then Add your custom stroke to the system.
		//this clears out all the stroke data for the current stroke
		virtual BOOL ClearStroke()=0;

		//this adds a hit test to the current stroke
		//			IPoint2 mousePos the point that want to test to add
		//			BOOL rebuildPointGatherData this determines whether the poing gather data get rebuilt
		//				this allows you to delay the building of the data if you are addding mulitple
		//				points at once
		//			BOOL updateViewport determines if the viewports get updated after this call
		virtual BOOL AddToStroke(IPoint2 mousePos, BOOL rebuildPointGatherData, BOOL updateViewport)=0;

	//These are direct to stroke data lists if you are doing your own point gather
	//Each stroke contains an arrays of data that are used to describe it such as
	//position, str, radius etc.

		//returns the number of sample points in the current stroke
		virtual int GetStrokeCount() = 0;
		//this returns a pointer to an array of floats where each entry is the str of a point sample
		virtual float *GetStrokeStr() = 0;
		//this returns a pointer to an array of floats where each entry is the radius of a point sample
		virtual float *GetStrokeRadius() = 0;
		//this returns a pointer to an array of point3s where each entry is the world space hit point of the sample
		virtual Point3 *GetStrokePointWorld() = 0;
		//this returns a pointer to an array of point3s where each entry is the world space normal of the sample
		virtual Point3 *GetStrokeNormalWorld() = 0;
		//this returns a pointer to an array of point3s where each entry is the world space hit point of the sample after it has been mirrored
		virtual Point3 *GetStrokePointWorldMirror() = 0;
		//this returns a pointer to an array of point3s where each entry is the world space normal of the sample after it has been mirrored
		virtual Point3 *GetStrokeNormalWorldMirror() = 0;
		//this returns a pointer to an array of floats where each entry is the pressure of a point sample either from a pressure sensitive tablet
		//or from a predefined presssure graph
		virtual float *GetStrokePressure() = 0;

		//this returns a pointer to an array of point3s where each entry is the local space hit point of the sample
		virtual Point3 *GetStrokePointLocal() = 0;
		//this returns a pointer to an array of point3s where each entry is the local space normal of the sample
		virtual Point3 *GetStrokeNormalLocal() = 0;
		//this returns a pointer to an array of point3s where each entry is the local space hit point of the sample after it has been mirrored
		virtual Point3 *GetStrokePointLocalMirror() = 0;
		//this returns a pointer to an array of point3s where each entry is the world space normal of the sample after it has been mirrored
		virtual Point3 *GetStrokeNormalLocalMirror() = 0;

		//this returns a pointer to an array of Ipoint2s where each entry is the mouse pos in screen space for the sample
		virtual IPoint2 *GetStrokeMousePos() = 0;
		//this returns a pointer to an array of s where each entry is whether the sample hit the mesh or not
		//the system allows the user to paint off the mesh, where all hitpoint are projected onto a plane
		//based on the last hit point and norml,
		virtual BOOL *GetStrokeHitList() = 0;

		//this returns a pointer to an array of point3s where each entry is the barycentri coords of the sample
		virtual Point3 *GetStrokeBary() = 0;
		//this returns a pointer to an array of ints where each entry is the index of the face of the sample 
		virtual int *GetStrokeIndex() = 0;

		//this returns a pointer to an array of bools where each entry is the state of the shift of the sample 
		virtual BOOL *GetStrokeShift() = 0;
		//this returns a pointer to an array of bools where each entry is the state of the ctrl of the sample 
		virtual BOOL *GetStrokeCtrl() = 0;
		//this returns a pointer to an array of bools where each entry is the state of the alt of the sample 
		virtual BOOL *GetStrokeAlt() = 0;

		//this returns a pointer to an array of INode where each entry is the INode of the sample 
		virtual INode **GetStrokeNode() = 0;
		//this returns a pointer to an array of ints where each entry is the time stamp of the sample 
		virtual int *GetStrokeTime() = 0;


		//given a point in world space it returns the str of that point based on the current stroke
		virtual float GetStrFromPoint(Point3 point) = 0;

//These functions let you interogate and set the state of the options dialog

		//this is used to ask the system if a stroke str or size changes as it is painted
		//the user can set predetermined shapes graphs and attach them to the str or size
		//of brush based on the position of the in the curve it is.  If you are in an interactive
		//mode this data will always be changing so you can use this to get the current str/sizes
		//of th hit points.  If you are in a non interactive mode you do not need to call this
		//since all the correct sizes/str are sent to the end stroke arrays.  If the user has not
		//specified any predetermined graphs the arrays will be NULL
		//		int &ct         - count of the arrays
		//		float *radius   - array of radii size
		//these return array of floats one entry for each point on the stroke
		 virtual float *GetPredefineStrStrokeData(int &ct)=0;
		 virtual float *GetPredefineSizeStrokeData(int &ct)=0;


		 //put in access to all the dialog properties

		//This will build a vertex normal list that you can access through RetrievePointGatherNormals
		//This is by default is off to save memory.  Also if you use a custom point list through
		//LoadCustomPointGather no normals will be built since there is no topo data to build them from
		virtual BOOL  GetBuildNormalData() = 0;
		virtual void  SetBuildNormalData(BOOL enable) = 0;

//These functions deal with the point gather.  The painter can automatically determines the weights
//of points by using the  PointGather
		//These let turns on/off the point gather and get its stae.  If this is enabled, 
		//the points of the mesh will be  used as your points
		virtual BOOL  GetEnablePointGather() = 0;
		virtual void  SetEnablePointGather(BOOL enable) = 0;//tested

		//This lets you set up a custom list of points to weight to override the currentlist
		//for instance if you have a non mesh you will need to do this since by default the
		//point gather uses the mesh points to weight
		//		int ct			the number of points that you want to add
		//		Point3 *points	the points you want to add in world space?
		//		INode *node		which node you want to assign them to
		virtual	BOOL LoadCustomPointGather(int ct, Point3 *points, INode *node) = 0;//tested

		//Once a stroke has started you can retrieve data about you point list such
		//as weigths, str etc.
		//This retrieves the weight of the points based on the current stroke
		//		INode *node		the node that you want to inspect
		//		int &ct			the number of points in the array
		virtual float *RetrievePointGatherWeights(INode *node, int &ct) = 0;
		//This retrieves the strength of the points based on the current stroke
		//		INode *node		the node that you want to inspect
		//		int &ct			the number of points in the array
		virtual float *RetrievePointGatherStr(INode *node, int &ct) = 0;//tested
		//This retrieves the whether the point was affected by a mirror stroke
		//		INode *node		the node that you want to inspect
		//		int &ct			the number of points in the array
		virtual BOOL *RetrievePointGatherIsMirror(INode *node, int &ct) = 0;//tested
		//This retrieves the the array of the points 
		//		INode *node		the node that you want to inspect
		//		int &ct			the number of points in the array
		virtual Point3 *RetrievePointGatherPoints(INode *node, int &ct) = 0;//tested
		
		//Normals are in local space and only valid if you do not use a CustomPointGather
		//This retrieves the the array of the normals 
		//		INode *node		the node that you want to inspect
		//		int &ct			the number of points in the array
		virtual Point3 *RetrievePointGatherNormals(INode *node, int &ct) = 0;

		//This retrieves the the array of the U vals, this is how far along the stroke that point is 
		//		INode *node		the node that you want to inspect
		//		int &ct			the number of points in the array
		virtual float *RetrievePointGatherU(INode *node, int &ct) = 0;

		

//functions to get the mirror plane data
//NOTE all mirror function work in world space
		//returns if the mirror plane is on or off
		virtual BOOL  GetMirrorEnable() = 0 ;//tested
		//lets you set whether the mirror plane is on/off
		virtual void  SetMirrorEnable(BOOL enable) = 0;
		//returns the center of the mirror plane in world space coords
		//the mirror plane is always aligned to the world axis
		virtual Point3 GetMirrorPlaneCenter() = 0;//tested
		//returns which mirror axis is active
		//	0 = x axis
		//	1 = y axis
		//	2 = z axis
		virtual int GetMirrorAxis() = 0;//tested
		//lets you set the mirror axis where
		//	0 = x axis
		//	1 = y axis
		//	2 = z axis
		virtual void SetMirrorAxis(int dir) = 0;
		virtual float GetMirrorOffset() = 0;//tested
		virtual void  SetMirrorOffset(float offset) = 0;//tested


	//These 2 function let you get and set the quad tree depth
	//The deeper the quad tree the more memory you consume, but the 
	//more memory you consume (the memory consumption is exponential so becarefule)
		virtual int GetTreeDepth() = 0;//tested
		virtual void SetTreeDepth(int depth) = 0;//tested

	//These 2 function let you get and set the Update on Mouse Up option
	//When this is enabled you will not get PaintStroke calls.
	//Instead you will get all the points at the end through the endStoke function
		virtual BOOL GetUpdateOnMouseUp() = 0;
		virtual void SetUpdateOnMouseUp(BOOL update) = 0;

	//These 2 function let you get and set the lag rate
	//When this is enabled you get PaintStroke delayed by the lag rate calls.
	//every x(lagrate) stroke points you willget the strokes.
		virtual int GetLagRate() = 0;
		virtual void SetLagRate(int lagRate) = 0;

//These functions control how the brush behaves

	//These 4 functions let you set the min/max strength for a brush.
	//If there is no pressure sensitive device attached only the max str is used
		virtual float GetMinStr() = 0;//tested
		virtual void  SetMinStr(float str) = 0;//tested
		virtual float GetMaxStr() = 0;//tested
		virtual void  SetMaxStr(float str) = 0;//tested

	//These 4 functions let you set the min/max radius for a brush.
	//If there is no pressure sensitive device attached only the max radius is used
		virtual float GetMinSize() = 0;//tested
		virtual void  SetMinSize(float str) = 0;//tested
		virtual float GetMaxSize() = 0;//tested
		virtual void  SetMaxSize(float str) = 0;//tested

	//These 2 functions get/set the aditive mode
	//When additive mode is off the weight is absolutely set based on the current stroke hit.
	//Previous stroke data is over written.  In Additive mode the strength is added to current
	//strength and is not capped.
		virtual BOOL  GetAdditiveMode()=0;//tested
		virtual void  SetAdditiveMode(BOOL enable)=0;//tested
	//This returns the brush falloff curve if you want to handle the doing the brush 
	//falloff yourself
		virtual ICurve *GetFalloffGraph()=0;


//These functions allow you to control the display of a stroke
//Colors are stored in the color manager. See the color ID defines at the top

	//This lets you get/set whether the ring is drawn around the hit point
		virtual BOOL  GetDrawRing()=0;//tested
		virtual void  SetDrawRing(BOOL draw)=0;//tested

	//This lets you get/set whether the normal vector is drawn at the hit point
		virtual BOOL  GetDrawNormal()=0;//tested
		virtual void  SetDrawNormal(BOOL draw)=0;//tested

	//This lets you get/set whether the a line is left behind a stroke as it is drawn
		virtual BOOL  GetDrawTrace()=0;//tested
		virtual void  SetDrawTrace(BOOL draw)=0;//tested


//These functions deal with the pressure sensitive devices and mimicing pressure sensitivity

	//These 2 functions let you get/set whether pressure sensistivity is turned on
	//when Pressure is enabled it can affect Str, Radius, Both Str and Radius or Nothing
	//You would nothing fr instance if you wanted to do a custom affect for pressure.
		virtual BOOL  GetPressureEnable()=0;//tested
		virtual void  SetPressureEnable(BOOL enable)=0;//tested

	//These 2 functions Get/Set what the pressure of a brush affects
	//You can affact Str. Radius, Str and Radius or None
	//See the defines at top 
		virtual BOOL  GetPressureAffects()=0;//tested
		virtual void  SetPressureAffects(int affect)=0;//tested

	//These function let you get/set whether a predefined str is enabled for a stroke.
	//A predefined str stroke lets the user graph the str of stroke over the length of stroke
		virtual BOOL  GetPredefinedStrEnable()=0;//tested
		virtual void  SetPredefinedStrEnable(BOOL enable)=0;//tested

	//These function let you get/set whether a predefined radius is enabled for a stroke.
	//A predefined radius stroke lets the user graph the radius of stroke over the length of stroke
		virtual BOOL  GetPredefinedSizeEnable()=0;//tested
		virtual void  SetPredefinedSizeEnable(BOOL enable)=0;//tested
		virtual ICurve *GetPredefineSizeStrokeGraph()=0;
		virtual ICurve *GetPredefineStrStrokeGraph()=0;

		virtual float GetNormalScale() = 0;
		virtual void SetNormalScale(float scale) = 0;

		virtual BOOL GetMarkerEnable() = 0;
		virtual void SetMarkerEnable(BOOL on) = 0;
		virtual float GetMarker() = 0;
		virtual void SetMarker(float pos) = 0;

		// 0 = creates a plance based on your last hit point and normal
		// 1 = a zdepth into the screen
		// 2 = a point in world space aligned to current view
		virtual int GetOffMeshHitType() = 0;
		virtual void SetOffMeshHitType(int type) = 0;

		virtual float GetOffMeshHitZDepth() = 0;
		virtual void SetOffMeshHitZDepth(float depth) = 0;

		virtual Point3 GetOffMeshHitPos() = 0;
		virtual void SetOffMeshHitPos(Point3 pos) = 0;


	};


//This is the Max5 version of a PainterCanvas.   Any plugin that wants to be able to be painted 
//on must sub class from this class or a later version of it.
//Basically this calls contains all the methods that deal with a paint stroke.
class IPainterCanvasInterface_V5 
	{
	public:
	
		// This is called when the user tart a pen stroke
		virtual BOOL  StartStroke() = 0;


		//This is called as the user strokes across the mesh or screen with the mouse down
		//This only gets called if the interactive mode is off (the user has turned off Update on Mouse Up)
		//		BOOL hit - if this is a hit or not, since the user can paint off a mesh
		//		IPoint2 mousePos - this is mouse coords of the current hit in view space
		//		Point3 worldPoint - this is the hit point on the painted mesh in world space
		//		Point3 worldNormal - this is the normal of the hit point on the painted mesh in world space
		//		Point3 localPoint - this is the hit point on the painted mesh in local space of the mesh that was hit
		//		Point3 localNormal - this is the normal of the hit point on the painted mesh in local space of the mesh that was hit
		//		Point3 bary - this the barcentric coords of the hit point based on the face that was hit
		//				      this may or may not be valid depending on the state of the stack.  For instance
		//					  if a paint modifier was below a MeshSmooth, the barycentric coords would be based on
		//					  the MeshSmooth'ed mesh and not the mesh at the point of the paint modifier
		//		int index - the index of the face that was hit.  See the warning in Point3 bary above.
		//      BOOL shift, ctrl, alt - the state of the shift, alt, and ctrl keys when the point was hit
		//		float radius - of the radius of the brush when the point was hit, this is after all modifiers
		//						have been applied like pressure or predefined radius etc
		//		float str - of the strength of the brush when the point was hit, this is after all modifiers
		//						have been applied like pressure or predefined radius etc
		//		float pressure - if a pressure sensitive tablet is used this value will be the pressure of the stroke
		//					   ranging from 0 to 1
		//		INode *node  - this node that got hit
		//		BOOL mirrorOn - whther the user had mirror on for the stroke, you can ignore the next for params if false
		//		Point3 worldMirrorPoint, Point3 worldMirrorNormal - the mirrored stroke pos
		//      Point3 localMirrorPoint, Point3 localMirrorNormal - the mirros stroke normals
		virtual BOOL  PaintStroke(
						  BOOL hit,
						  IPoint2 mousePos, 
						  Point3 worldPoint, Point3 worldNormal,
						  Point3 localPoint, Point3 localNormal,
						  Point3 bary,  int index,
						  BOOL shift, BOOL ctrl, BOOL alt, 
						  float radius, float str,
						  float pressure, INode *node,
						  BOOL mirrorOn,
						  Point3 worldMirrorPoint, Point3 worldMirrorNormal,
						  Point3 localMirrorPoint, Point3 localMirrorNormal
						  ) = 0;

		// This is called as the user ends a strokes when the users has it set to always update
		virtual BOOL  EndStroke() = 0;

		// This is called as the user ends a strokes when the users has it set to update on mouse up only
		// the canvas gets a list of all points, normals etc instead of one at a time
		//		int ct - the number of elements in the following arrays
		//  <...> see paintstroke() these are identical except they are arrays of values
		virtual BOOL  EndStroke(int ct, BOOL *hit, IPoint2 *mousePos, 
						  Point3 *worldPoint, Point3 *worldNormal,
						  Point3 *localPoint, Point3 *localNormal,
						  Point3 *bary,  int *index,
						  BOOL *shift, BOOL *ctrl, BOOL *alt, 
						  float *radius, float *str,
						  float *pressure, INode **node,
						  BOOL mirrorOn,
						  Point3 *worldMirrorPoint, Point3 *worldMirrorNormal,
						  Point3 *localMirrorPoint, Point3 *localMirrorNormal	) = 0;

		// This is called as the user cancels a stroke by right clicking
		virtual BOOL  CancelStroke() = 0;	

		//This is called when the painter want to end a paint session for some reason.
		virtual BOOL  SystemEndPaintSession() = 0;

		//this is called when the painter updates the view ports
		//this will let you do custom drawing if you need to
		virtual void PainterDisplay(TimeValue t, ViewExp *vpt, int flags) = 0;


	};


//This is the Max5.1 version of a PainterCanvas.   
//This interface changes how the command mode behaves
//It adds the ability to send back to the canvas whether to start or end the paint mode
//This bacially replaces the SystemEndPaintSession and also

class IPainterCanvasInterface_V5_1
	{
	public:

		//the painter interface will call these when it wants to start the painter or end it
		//an example of this is when the user scrubs the time slider whne in a paint session
		//the system will turn off the paint mode while scrubbing so you will get a CanvasEndPaint
		//then when the user stops srcubing it will try to turn it back on using CanvasStartPaint
		//so if you use this interface after you call StartPaintSession() you will imediately get 
		//a CanvasStartPaint callback where all your setup code and UI code shoudl be handled
		virtual void CanvasStartPaint()=0;
		virtual void CanvasEndPaint()=0;
	

	};


#endif