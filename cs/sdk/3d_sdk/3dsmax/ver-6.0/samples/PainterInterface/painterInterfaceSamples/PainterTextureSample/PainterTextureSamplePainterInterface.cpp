#include "PainterTextureSample.h"


// This is called when the user tart a pen stroke
BOOL  PainterTextureSample::StartStroke()
	{


	pblock->GetValue( pb_color, 0, ac, FOREVER);


	bit.r = (int)(ac.r*65535.0f);
	bit.g = (int)(ac.g*65535.0f);
	bit.b = (int)(ac.b*65535.0f);
	bit.a = 0xFFFF;

	lagRate = pPainter->GetLagRate();
	lagCount = 0;

	return TRUE;
	}

//This is called as the user strokes across the mesh or screen with the mouse down
BOOL  PainterTextureSample::PaintStroke(		  BOOL hit,
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
						  ) 
{
	static IPoint2 lastPoint;
	if (hit)
		{
//loop through 500 random points along the stroke
		BOOL airBrushMode = FALSE;
		if (airBrushMode)
			{
			int ct = radius; 
			if (lagRate > 0)
				{
				Point2 p1;
				p1.x = mousePos.x;
				p1.y = mousePos.y;
				Point2 p2;
				p2.x = lastPoint.x;
				p2.y = lastPoint.y;
				float dist = Length(p2-p1);
				ct =  dist * radius;
				}
			else ct =  radius * 100.0f;

			for (int i =0; i < ct; i++)
				{
				Point3 dummyPointWorldPos;
				Point3 dummyPointWorldNormal;
				Point3 dummyPointLocalPos;
				Point3 dummyPointLocalNormal;

				Point3 dummyPointWorldPosM;
				Point3 dummyPointWorldNormalM;
				Point3 dummyPointLocalPosM;
				Point3 dummyPointLocalNormalM;

				Point3 bary;
				int index;
				float str;
				INode *node = NULL;
				BOOL mirror;
				if (pPainter->RandomHitAlongStroke(dummyPointWorldPos, dummyPointWorldNormal,
						  dummyPointLocalPos, dummyPointLocalNormal,
						  bary,  index,
						  str, node,
						  mirror,
						  dummyPointWorldPosM, dummyPointWorldNormalM,
						  dummyPointLocalPosM, dummyPointLocalNormalM,lagCount))
					{
	//get the hit point bary
					if ((index >=0) && (index < uvwFaces.Count()))
						{
	//convert to UVW
						Point3 a = uvwPoints[uvwFaces[index].t[0]];
						Point3 b = uvwPoints[uvwFaces[index].t[1]];
						Point3 c = uvwPoints[uvwFaces[index].t[2]];
						Point3 uvw = a * bary.x +
							 	 b * bary.y +
								 c * bary.z;
	//draw in the bitmap
						if (bm)
							{
							BMM_Color_64 basebit;
							int x = uvw.x*width;
							int y = (1.0f-uvw.y)*width;
							float baseAlpha, incAlpha;
							incAlpha = str;
							baseAlpha = 1.0f - str;
							bm->GetPixels(x,y,1,&basebit);
							basebit.r = (basebit.r * baseAlpha) + (bit.r * incAlpha);
							basebit.g = (basebit.g * baseAlpha) + (bit.g * incAlpha);
							basebit.b = (basebit.b * baseAlpha) + (bit.b * incAlpha);
							bm->PutPixels(x,y,1,&basebit);
	
							}
						}
					}
				}
			}
		else
			{
			int count = 0;
			float *str = pPainter->RetrievePointGatherStr(node, count);
			
			for (int i =0; i < count; i++)
				{
				if (*str > 0.0f)
					{
					Point3 uvw = uvwList[i];
	//draw in the bitmap
					if (bm)
						{
						BMM_Color_64 basebit;
						int x = uvw.x*width;
						int y = (1.0f-uvw.y)*width;
						float baseAlpha, incAlpha;
						incAlpha = *str;
						baseAlpha = 1.0f - *str;
						bm->GetPixels(x,y,1,&basebit);
						basebit.r = (basebit.r * baseAlpha) + (bit.r * incAlpha);
						basebit.g = (basebit.g * baseAlpha) + (bit.g * incAlpha);
						basebit.b = (basebit.b * baseAlpha) + (bit.b * incAlpha);
						bm->PutPixels(x,y,1,&basebit);
	
						}
					}
				str++;
				}

			}
		lagCount++;
		if ((lagRate == 0) || ((lagCount%lagRate) == 0))
			{
			texHandleValid.Empty();
			DiscardTexHandle();
			}
		lastPoint = mousePos;
//		NotifyDependents(FOREVER, PART_TEXMAP, REFMSG_CHANGE);
		}

return TRUE;
}

// This is called as the user ends a strokes when the users has it set to always update
BOOL  PainterTextureSample::EndStroke()
	{

	texHandleValid.Empty();
	DiscardTexHandle();
	NotifyDependents(FOREVER, PART_TEXMAP, REFMSG_CHANGE);

	return TRUE;
	}

// This is called as the user ends a strokes when the users has it set to update on mouse up only
// the canvas gets a list of all points, normals etc instead of one at a time
//		int ct - the number of elements in the following arrays
//  <...> see paintstroke() these are identical except they are arrays of values
BOOL  PainterTextureSample::EndStroke(int ct, BOOL *hit, IPoint2 *mousePos, 
						  Point3 *worldPoint, Point3 *worldNormal,
						  Point3 *localPoint, Point3 *localNormal,
						  Point3 *bary,  int *index,
						  BOOL *shift, BOOL *ctrl, BOOL *alt, 
						  float *radius, float *str,
						  float *pressure, INode **node,
						  BOOL mirrorOn,
						  Point3 *worldMirrorPoint, Point3 *worldMirrorNormal,
						  Point3 *localMirrorPoint, Point3 *localMirrorNormal)
	{

//loop through 500 random points along the stroke
	BOOL airBrushMode = FALSE;
	if (airBrushMode)
		{
		int totalCount = ct * 100;
		for (int i =0; i < totalCount; i++)
			{
			Point3 dummyPointWorldPos;
			Point3 dummyPointWorldNormal;
			Point3 dummyPointLocalPos;
			Point3 dummyPointLocalNormal;

			Point3 dummyPointWorldPosM;
			Point3 dummyPointWorldNormalM;
			Point3 dummyPointLocalPosM;
			Point3 dummyPointLocalNormalM;

			Point3 bary;
			int index;
			float str;
			INode *node = NULL;
			BOOL mirror;
			if (pPainter->RandomHitAlongStroke(dummyPointWorldPos, dummyPointWorldNormal,
						  dummyPointLocalPos, dummyPointLocalNormal,
						  bary,  index,
						  str, node,
						  mirror,
						  dummyPointWorldPosM, dummyPointWorldNormalM,
						  dummyPointLocalPosM, dummyPointLocalNormalM,-1))
				{
	//get the hit point bary
				if ((index >=0) && (index < uvwFaces.Count()))
					{
	//convert to UVW
					Point3 a = uvwPoints[uvwFaces[index].t[0]];
					Point3 b = uvwPoints[uvwFaces[index].t[1]];
					Point3 c = uvwPoints[uvwFaces[index].t[2]];
					Point3 uvw = a * bary.x +
						 	 b * bary.y +
							 c * bary.z;
	//draw in the bitmap
					if (bm)
						{
						BMM_Color_64 basebit;
						int x = uvw.x*width;
						int y = (1.0f-uvw.y)*width;
						float baseAlpha, incAlpha;
						incAlpha = str;
						baseAlpha = 1.0f - str;
						bm->GetPixels(x,y,1,&basebit);
						basebit.r = (basebit.r * baseAlpha) + (bit.r * incAlpha);
						basebit.g = (basebit.g * baseAlpha) + (bit.g * incAlpha);
						basebit.b = (basebit.b * baseAlpha) + (bit.b * incAlpha);
						bm->PutPixels(x,y,1,&basebit);

						}
					}
				}
			}
		}
	else
		{
		int count = 0;
		float *str = pPainter->RetrievePointGatherStr(this->node, count);
			
		for (int i =0; i < count; i++)
			{
			if (*str > 0.0f)
				{
				Point3 uvw = uvwList[i];
	//draw in the bitmap
				if (bm)
					{
					BMM_Color_64 basebit;
					int x = uvw.x*width;
					int y = (1.0f-uvw.y)*width;
					float baseAlpha, incAlpha;
					incAlpha = *str;
					baseAlpha = 1.0f - *str;
					bm->GetPixels(x,y,1,&basebit);
					basebit.r = (basebit.r * baseAlpha) + (bit.r * incAlpha);
					basebit.g = (basebit.g * baseAlpha) + (bit.g * incAlpha);
					basebit.b = (basebit.b * baseAlpha) + (bit.b * incAlpha);
					bm->PutPixels(x,y,1,&basebit);
	
					}
				}
			str++;
			}
		}

	texHandleValid.Empty();
	DiscardTexHandle();

	NotifyDependents(FOREVER, PART_TEXMAP, REFMSG_CHANGE);
	return TRUE;
	}

// This is called as the user cancels a stroke by right clicking
BOOL  PainterTextureSample::CancelStroke()
	{
	return TRUE;
	}

//This is called when the painter want to end a paint session for some reason.
BOOL  PainterTextureSample::SystemEndPaintSession()
	{
	if (iPaintButton) iPaintButton->SetCheck(FALSE);
	return TRUE;
	}

