/*	
 *		nurbspro.h - MAX NURBS protocols
 *
 *			Copyright © Autodesk, Inc. 1997
 *			Author: John Wainwright
 *
 */

	def_visible_generic ( evalPos,			"evalPos" );
	def_visible_generic ( evalTangent,		"evalTangent" );
//	def_visible_generic ( getTrimPoint,		"getTrimPoint" );
	def_visible_generic ( getKnot,			"getKnot" );
	def_visible_generic ( setKnot,			"setKnot" );
	def_visible_generic ( getCV,			"getCV" );
	def_visible_generic ( setCV,			"setCV" );
	def_visible_generic ( refine,			"refine" );
	def_visible_generic ( getPoint,			"getPoint" );
	def_visible_generic ( setPoint,			"setPoint" );
	def_visible_generic ( evalUTangent,		"evalUTangent" );
	def_visible_generic ( evalVTangent,		"evalVTangent" );
	def_visible_generic ( setTiling,		"setTiling" );
	def_visible_generic ( getTiling,		"getTiling" );
	def_visible_generic ( setTilingOffset,	"setTilingOffset" );
	def_visible_generic ( getTilingOffset,	"getTilingOffset" );
	def_visible_generic ( setTextureUVs,	"setTextureUVs" );
	def_visible_generic ( getTextureUVs,	"getTextureUVs" );
	def_visible_generic ( setGenerateUVs,	"setGenerateUVs" );
	def_visible_generic ( getGenerateUVs,	"getGenerateUVs" );
	def_visible_generic ( closeU,			"closeU" );
	def_visible_generic ( closeV,			"closeV" );
	def_visible_generic ( getUKnot,			"getUKnot" );
	def_visible_generic ( getVKnot,			"getVKnot" );
	def_visible_generic ( setUKnot,			"setUKnot" );
	def_visible_generic ( setVKnot,			"setVKnot" );
	def_visible_generic ( refineU,			"refineU" );
	def_visible_generic ( refineV,			"refineV" );
	def_visible_generic ( appendCurve,		"appendCurve" );
	def_visible_generic ( appendCurveByID,  "appendCurveByID" );
	def_visible_generic ( getCurve,			"getCurve" );
	def_visible_generic ( setCurve,			"setCurve" );
	def_visible_generic ( getCurveID,		"getCurveID" );
	def_visible_generic ( setCurveByID,		"setCurveByID" );
	def_visible_generic ( getFlip,			"getFlip" );
	def_visible_generic ( setFlip,			"setFlip" );
	def_visible_generic ( getObject,		"getObject" );
	def_visible_generic ( setObject,		"setObject" );
	def_visible_generic ( appendObject,		"appendObject" );
	def_visible_generic ( removeObject,		"removeObject" );
	def_visible_generic ( deleteObjects,	"deleteObjects" );
	def_visible_generic ( reparameterize, "reparameterize" );

	def_visible_generic ( getParent,		"getParent" );
	def_visible_generic ( getParentID,		"getParentID" );
	def_visible_generic ( setParent,		"setParent" );
	def_visible_generic ( setParentID,		"setParentID" );
	def_visible_generic ( getEdge,			"getEdge" );
	def_visible_generic ( setEdge,			"setEdge" );
	def_visible_generic ( appendUCurve,		"appendUCurve" );
	def_visible_generic ( appendUCurveByID,	"appendUCurveByID" );
	def_visible_generic ( getUCurve,		"getUCurve" );
	def_visible_generic ( getUCurveID,		"getUCurveID" );
	def_visible_generic ( setUCurve,		"setUCurve" );
	def_visible_generic ( setUCurveByID,	"setUCurveByID" );
	def_visible_generic ( appendVCurve,		"appendVCurve" );
	def_visible_generic ( appendVCurveByID,	"appendVCurveByID" );
	def_visible_generic ( getVCurve,		"getVCurve" );
	def_visible_generic ( getVCurveID,		"getVCurveID" );
	def_visible_generic ( setVCurve,		"setVCurve" );
	def_visible_generic ( setVCurveByID,	"setVCurveByID" );
	def_visible_generic ( disconnect,		"disconnect" );
	def_visible_generic ( setSeed, 	"setSeed" );
	def_visible_generic ( getSeed, 	"getSeed" );
	def_visible_generic ( getRadius, "getRadius" );
	def_visible_generic ( setRadius, "setRadius" );
	def_visible_generic ( getTrimSurface,	"getTrimSurface" );
	def_visible_generic ( setTrimSurface,	"setTrimSurface" );
	def_visible_generic ( getFlipTrim, "getFlipTrim" );
	def_visible_generic ( setFlipTrim, "setFlipTrim" );

	def_visible_generic ( setTextureSurface, "setTextureSurface" );
	def_visible_generic ( getTextureSurface, "getTextureSurface" );

	def_visible_generic ( getProdTess,		"getProdTess" );
	def_visible_generic ( setProdTess,		"setProdTess" );
	def_visible_generic ( getViewTess,		"getViewTess" );
	def_visible_generic ( setViewTess,		"setViewTess" );
	def_visible_generic ( clearViewTess,	"clearViewTess" );
	def_visible_generic ( clearProdTess,	"clearProdTess" );

	def_visible_generic ( setCurveStartPoint,	"setCurveStartPoint" );
	def_visible_generic ( getCurveStartPoint,	"getCurveStartPoint" );

	def_visible_primitive ( NURBSLatheSurface,		"MakeNURBSLatheSurface" );
	def_visible_primitive ( NURBSSphereSurface,		"MakeNURBSSphereSurface" );
	def_visible_primitive ( NURBSCylinderSurface,	"MakeNURBSCylinderSurface" );
	def_visible_primitive ( NURBSConeSurface,		"MakeNURBSConeSurface" );
	def_visible_primitive ( NURBSTorusSurface,		"MakeNURBSTorusSurface" );

	def_visible_primitive ( NURBSNode,				"NURBSNode" );
	def_visible_primitive ( NURBSLatheNode,			"NURBSLatheNode" );
	def_visible_primitive ( NURBSExtrudeNode,		"NURBSExtrudeNode" );

	def_visible_primitive ( updateSurfaceMapper,	"updateSurfaceMapper" );
