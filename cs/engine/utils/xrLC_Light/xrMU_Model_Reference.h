#ifndef XRMUMODEL_REFERENCE_H
#define XRMUMODEL_REFERENCE_H

#include "base_color.h"

class xrMU_Model;
namespace CDB { class CollectorPacked; }
class XRLC_LIGHT_API xrMU_Reference
{
public:
	xrMU_Model*				model;
    Fmatrix					xform;
    Flags32					flags;
	u16						sector;

	xr_vector<base_color>	color;

	base_color_c			c_scale;
	base_color_c			c_bias;
public:
	void					Load				( IReader& fs, xr_vector<xrMU_Model*>& mu_models );
	void					calc_lighting		();

	void					export_cform_game	(CDB::CollectorPacked& CL);
	void					export_cform_rcast	(CDB::CollectorPacked& CL);
//	void					export_ogf			();
};

#endif