/**********************************************************************
 *<
	FILE: IGameControl.h

	DESCRIPTION: Controller interfaces for IGame

	CREATED BY: Neil Hazzard, Discreet

	HISTORY: created 02/02/02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/
/*!\file IGameControl.h
\brief High level access to MAX's controllers, including Biped and constraints and list controllers

High level access to MAX's controllers, including Biped and constraints and list controllers
*/
#ifndef __IGAMECONTROL__H
#define __IGAMECONTROL__H

#pragma once

#include "max.h"
#include "ISTDPLUG.H"
#include "IGameProperty.h"
#include "IConversionManager.h"

class IGameNode;


//! An enum of basic controller types used by IGame
/*! These controllers types are used to define the type of controller being queried.
*/

enum IGameControlType{
	 IGAME_POS,						/*!<Position Controller*/	
	 IGAME_ROT,						/*!<Rotation Controller*/	
	 IGAME_SCALE,					/*!<Scale Controller*/
	 IGAME_FLOAT,					/*!<Float Controller*/
	 IGAME_POINT3,					/*!<Point3 Controller*/
	 IGAME_TM,						/*!<Used for sampling the node transformation matrix.  This is the only time this control
										type can be used*/		
	 IGAME_EULER_X,					/*!<Euler X controller*/
	 IGAME_EULER_Y,					/*!<Euler Y controller*/
	 IGAME_EULER_Z,					/*!<Euler Z controller*/	
};




class IGameConstraint;
class GMatrix;

//! A generic animation key wrapper class
/*! A generic TCB key class for IGame
*/
class IGameTCBKey {
	public:	
		//! access to basic TCB data
		/*! This provides access to the Tension, continuity, bias and easein/out properties of a TCB Key
		*/
		float tens, cont, bias, easeIn, easeOut;
		//! float based value, 
		/*! This would be accessed when using the IGAME_FLOAT specifier
		*/
		float fval;
		//! Point3 based value, 
		/*! This would be accessed when using the IGAME_POS or IGAME_POINT3 specifiers
		*/
		Point3 pval;
		//! Ang Axis based value, 
		/*! This would be accessed when using the IGAME_ROT specifier
		*/
		AngAxis aval;
		//! scale based value, 
		/*! This would be accessed when using the IGAME_SCALE specifier
		*/
		ScaleValue sval;
	};

//! A generic animation key wrapper class
/*! A generic Bezier Key class for IGame
*/
class IGameBezierKey {
	public:
		//! float based In and out tangents
		/*! This would be access when using the IGAME_FLOAT specifier
		*/
		float fintan, fouttan;

		//! float based value, 
		/*! This would be accessed when using the IGAME_FLOAT specifier
		*/
		float fval;
		//! float based tangent lengths
		/*! This would be access when using the IGAME_FLOAT specifier
		*/
		float finLength, foutLength;

		//! Point3 based In and out tangents
		/*! This would be access when using the IGAME_POS or IGAME_POINT3 specifiers
		*/
		Point3 pintan, pouttan;
		
		//! Point3 based value, 
		/*! This would be accessed when using the IGAME_POS or IGAME_POINT3 specifiers
		*/
		Point3 pval;
		//! Point3 based tangent lengths
		/*! This would be access when using the IGAME_POS or IGAME_POINT3 specifiers
		*/
		Point3 pinLength, poutLength;
		//! Quaternion based value, 
		/*! This would be accessed when using the IGAME_ROT specifier
		*/
		Quat qval;
		//! scale based value, 
		/*! This would be accessed when using the IGAME_SCALE specifier
		*/
		ScaleValue sval;

	};

//! A generic animation key wrapper class
/*! A generic Linear Key class for IGame
*/
class IGameLinearKey  {
	public:
		//! float based value, 
		/*! This would be accessed when using the IGAME_FLOAT specifier
		*/
		float fval;
		//! Point3 based value, 
		/*! This would be accessed when using the IGAME_POS or IGAME_POINT3 specifiers
		*/
		Point3 pval;
		//! Quaternion based value, 
		/*! This would be accessed when using the IGAME_ROT specifier
		*/
		Quat qval;
		//! scale based value, 
		/*! This would be accessed when using the IGAME_SCALE specifier
		*/
		ScaleValue sval;
	};


//! A generic animation key wrapper class
/*! A generic Sample Key class for IGame.  This is used for unkown controllers or controllers that
simply need to be sampled, this can includes Biped
*/
class IGameSampleKey{
	public:
		//! Point3 value, used with IGAME_POINT3 and IGAME_POS
		Point3 pval;
		//! float value, used with IGAME_FLOAR
		float fval;
		//! Quaternion value, used with IGAME_ROT
		Quat qval;
		//! Scale value, used with IGAME_SCALE
		ScaleValue sval;
		//! GMatrix, used with IGAME_TM
		GMatrix gval;

};


//!Main animation key container
/*! A simple container class for direct Key access of all the available Key types
	
*/
class IGameKey
{
	public:
		//! The time the key was set
		TimeValue t;
		//! Flags various selection states for the key.
		DWORD flags;
		//!The TCB Keys
		/*! This key access would be used if you used one of the GetTCBKeys methods
		*/
		IGameTCBKey tcbKey;
		//!The Bezier Keys
		/*! This key access would be used if you used one of the GetBezierKeys methods
		*/
		IGameBezierKey bezierKey;
		//!The Linear Keys
		/*! This key access would be used if you used one of the GetLinearKeys methods
		*/		
		IGameLinearKey linearKey;

		//!The Sampled Keys
		/*! This key access would be used if you used one of the GetSampledKeys methods
		*/		
		IGameSampleKey sampleKey;
};

/*!\var typedef  Tab<IGameKey> IGameKeyTab
\brief A Tab of IGameKey. 
Uses <A HREF="sdk.chm::/html/idx_R_template_class_tab.htm">Max's Template class Tab</A> 
*/

typedef  Tab<IGameKey> IGameKeyTab;



//!A simple access class for controllers.
/*! IGameControl provides a simplified access to the various key frame controllers used throughout max.  In max a controller
needs to be queried for the key interface and then its class ID checked before casting to the approriate Key class.  This class
provide the developer with all the keys based on the key type being asked for.  As the game engine may only support certain type 
of max controllers it is far more efficient for a developer to ask IGame for all the Bezier Postion keys then to check wtith max for 
the controller type.  This class also provides direct support for Euler Rotation controllers.  The developer can use GetControlType
to see if the rotation is Euler and can then can use IGAME_EULER_X in the approprate control access type to retrieve the keys.
\br
In 3ds max some controllers such as TCB, Linear and Bezier support direct access to their keys.  Other controllers are more private
and usually base them selves on a float or Point3 controller.  If there is no direct access then sampling is the easiest choice.  IGame
supports two types - Full and Quick.  Full samples across the full animation range, whilst Quick only samples where keys are found.  The 
limitation of Quick, is that it does not support IGAME_TM or controllers that do not set keys.



\sa <A HREF="sdk.chm::/html/idx_R_template_class_tab.htm">Max's Template class Tab</a>
\sa GMatrix
\sa IGameProperty
*/
class IGameControl
{
	public:

		enum MaxControlType{
			IGAME_UNKNOWN,					/*!<An umknown controller type*/
			IGAME_MAXSTD,					/*!<A Standard max key frame controller*/
			IGAME_BIPED,					/*!<A Biped Controller*/
			IGAME_EULER,					/*!<An Euler Controller*/
			IGAME_ROT_CONSTRAINT,			/*!<A Rotation constraint*/
			IGAME_POS_CONSTRAINT,			/*!<A Position constraint*/
			IGAME_LINK_CONSTRAINT,			/*!<A Link Constraint*/
			IGAME_LIST,						/*!<A List Controller*/

		};
		
	//! An enum of Euler Orderings
	/*! These are the rotation orders for an Euler Controller
	*/
		enum EulerOrder{
			XYZ,	/*!<XYZ Ordering*/
			XZY,	/*!<XZY Ordering*/
			YZX,	/*!<YZX Ordering*/
			YXZ,	/*!<YXZ Ordering*/
			ZXY,	/*!<ZXY Ordering*/
			ZYX,	/*!<ZYX Ordering*/
			XYX,	/*!<XYX Ordering*/
			YZY,	/*!<YZY Ordering*/
			ZXZ,	/*!<ZXZ Ordering*/
			BAD		/*!<If this is not a Euler Controller*/
		};
		//! Return the Bezier Keys
		/*! IGameControl will check the appropriate control and fill the IGameKeyTab with the Key data.  To access the 
		keys you would look in the bezierKey Tab maintained by IGameKey.
		\param &gameKeyTab The Tab to receieve the data
		\param Type The controller type (based on Transform style) to query.  This can be one of the following\n
		IGAME_POS\n
		IGAME_ROT\n
		IGAME_SCALE\n
		IGAME_FLOAT\n
		IGAME_POINT3\n
		IGAME_EULER_X\n
		IGAME_EULER_Y\n
		IGAME_EULER_Z\n
		\return TRUE is the controller was accessed successfully.
		*/
		virtual bool GetBezierKeys(IGameKeyTab &gameKeyTab,IGameControlType Type)=0;

		//! Return the Linear Keys
		/*! IGameControl will check the appropriate control and fill the IGameKeyTab with data
		\param &gameKeyTab The tab to receieve the data
		\param Type The controller type to query.  See IGameControl::GetBezierKeys for more info
		\return TRUE is the controller was accessed successfully.
		*/
		virtual bool GetLinearKeys(IGameKeyTab &gameKeyTab, IGameControlType Type)=0;

		//! Return the TCB Keys
		/*! IGameControl will check the appropriate control and fill the IGameKeyTab with data
		\param &gameKeyTab The tab to receieve the data
		\param Type The controller type to query.  See IGameControl::GetBezierKeys for more info
		\return TRUE is the controller was accessed successfully.
		*/
		virtual bool GetTCBKeys(IGameKeyTab &gameKeyTab, IGameControlType Type)=0;

		//! Return the Sampled Keys
		/*! IGameControl will sample the control based on the type supplied.  It will sample the node TM , float or point3 
		contorllers.  The TM sample will be in the Coord System that you defined when initialising IGame.  This method
		will sample the controller across the complete animation range.
		\param &sample The tab to receieve the data
		\param frameRate This is the number frames that the controller will be sampled at.  It will be converted to Ticks internally
		\param Type The controller type to query.  This can be any of the standard type but also include  IGAME_TM
		\return TRUE is the controller was accessed successfully.
		*/
		virtual bool GetFullSampledKeys(IGameKeyTab &sample, int frameRate, IGameControlType Type) =0;

		//! Return the Sampled Keys 
		/*! IGameControl will sample the control based on the type supplied.  It will sample float or point3 
		contorllers.  The TM sample will be in the Coord System that you defined when initialising IGame.  This 
		method only samples the controller where a key is Set, so it will not support the IGAME_TM type.  If the 
		controller does not support setting of keys, it will return false.  This method will only sample the controller
		at times where keys exist.  This is useful to limit the data where controller can not be accessed directly
		\param &sample The tab to receieve the data
		\param Type The controller type to query.  This can be any of the standard type but also include  IGAME_TM
		\return TRUE is the controller was accessed successfully.
		*/
		virtual bool GetQuickSampledKeys(IGameKeyTab &sample, IGameControlType Type) =0;


		//! Return an individual IGameKey
		/*! fills out the supplied IGameKEy with the bezier data for the key index supplied
		\param Type The controller type to query.  See IGameControl::GetBezierKeys for more info
		\param &bezKey
		\param index The key to retrieve
		\return TRUE if successful
		*/
		virtual bool GetBezierIGameKey(IGameControlType Type, IGameKey &bezKey, int index) =0 ;

		//! Return an individual IGameKey
		/*! fills out the supplied IGameKey with the TCB data for the key index supplied
		\param Type The controller type to query.  See IGameControl::GetBezierKeys for more info
		\param &tcbKey
		\param index The key to retrieve
		\return TRUE if successful
		*/
		virtual bool GetTCBIGameKey(IGameControlType Type, IGameKey &tcbKey, int index)=0 ;

		//! Return an individual IGameKey
		/*! fills out the supplied IGameKey with the Linear data for the key index supplied
		\param Type The controller type to query.  See IGameControl::GetBezierKeys for more info
		\param &linearKey
		\param index The key to retrieve
		\return TRUE if successful
		*/
		virtual bool GetLinearIGameKey(IGameControlType Type, IGameKey &linearKey, int index)=0;
		//! The total number of keys for this controller
		/*! This return the total number of keys for the controller supplied.
		\param Type The controller type to query.  See IGameControl::GetBezierKeys for more info
		\return The total number of keys
		*/
		virtual int GetIGameKeyCount(IGameControlType Type)=0;

		//!The controller type
		/*!Retrieves what type of IGame Controller it is based on transformation style..
		\param Type The controller to query. See IGameControl::GetBezierKeys for more info
		\return The type of controller,  It can be one of the following\n
		IGAME_UNKNOWN\n
		IGAME_MAXSTD\n
		IGAME_BIPED\n
		IGAME_ROT_CONSTRAINT\n
		IGAME_POS_CONSTRAINT\n
		*/
		virtual MaxControlType GetControlType(IGameControlType Type)=0;

		//!Access to the Constraints
		/*! If a controller has a constraint system, then this will provide access to it
		\param Type The controller to check. THis can be either of\
		IGAME_POS\n
		IGAME_ROT\n
		\return A Pointer to IGameConstraint, or NULL if not available
		*/
		virtual IGameConstraint * GetConstraint(IGameControlType Type)=0;
		
		//! The order of Rotation
		/*! This provides a way of determining the order of rotation for Euler controllers.  THis is important
		so that the rotation can be rebuilt correctly on import.\nThis data is also important when accessing the 
		controller keys.  You still access the Euler data bsed on X,Y and Z - but you would use the ordering to 
		work out the meaning of each controller.  So if EulerOrder was ZXZ, then controller access would mean 
		x=z, y=x, z=z.
		\return The order of Rotation.  This can be a value from the EulerOrder
		*/
		virtual EulerOrder GetEulerOrder()=0;

		/*! Get access to the actual max controller
		\param Type This can be either\n
		IGAME_POS\n
		IGAME_ROT\n
		IGAME_SCALE\n
		\return The max controller
		*/
		virtual Control * GetMaxControl(IGameControlType Type)=0;

		//! Access the list controller
		/*! Access the n'th controller from the List controller.
		\param index The index into the list controller
		\param Type The Control type to access
		\return An IGameControl interface
		*/
		virtual IGameControl * GetListSubControl(int index, IGameControlType Type)=0;

		//! The number of controllers maintained by the list controller
		/*! The number of controllers maintained by the list controller for the Controller type being queried
		\param Type The controller to type
		\return The number of controllers in the list controller
		*/
		virtual int GetNumOfListSubControls(IGameControlType Type)=0;


				
};

//! simple wrapper class for constraints
/*! A unified wrapper around the various constraints that are available in Max.  There is access to the type of constraint
in use, plus easier access to the constrainers.  If further access it needed, the IPropertyContainer interface can be used 
and additions made to property file used to access other data, as the source fro these constraints is available in the SDK.
*/
class IGameConstraint : public IExportEntity
{
		

public:
	//! An enum of Max Constraint
	/*! These are the constraints supported by IGame
	*/
	enum ConstraintType{
		IGAME_PATH,			/*!<Path Constraint*/
		IGAME_ORIENTATION,	/*!<Orientation Constraint*/
		IGAME_LOOKAT,		/*!<look At Constraint*/
		IGAME_POSITION,		/*!<Position Constraint*/
		IGAME_LINK,			/*!<A TM link contraint*/
		IGAME_UNKNOWN,		/*!<Unknown Constraint*/
	};

	//!Number of constraining Node
	/*!The number of nodes in use by the Constraint system
	\return The number fo nodes
	*/
	virtual int NumberOfConstraintNodes()=0;

	//!The constraint Node
	/*! The actual node of the index passed in that is working in the system
	\param index The index of the node to rerieve
	\return A pointer to IGameNode
	*/
	virtual IGameNode * GetConstraintNodes(int index)=0;

	//!The influence of the bone
	/*! This is the weight, or influence the specified node has in the constraint system.  The index used here is the same
	as the index used in GetConstraintNodes, otherwise the weights will not match.  This has no effect for a Link Constraint
	\param nodeIndex The node index to query
	\return The weight value
	*/
	virtual float GetConstraintWeight(int nodeIndex)=0;

	//!Get the start frame for the Link constraint
	/*!This specifies when the link for the n'th node will start.
	\param index The node index
	\return The start frame for the node queried.
	*/
	virtual int GetLinkConstBeginFrame(int index) = 0;

	//! The type of Constraint
	/*! This defines the actual constraint being used on the controller
	\return The type of max constraint.  It can be one ConstraintType enum
	*/
	virtual ConstraintType GetConstraintType()=0;
	

};

#endif 