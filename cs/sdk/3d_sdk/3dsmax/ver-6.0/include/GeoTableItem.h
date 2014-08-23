#pragma once

class GeoTableItem
{
public:
	bool m_geoInfo;
	Matrix3 m_matrix;
	NameTab m_names;
	Point3 m_pts[4];
	Point3 m_scale;

	GeoTableItem() : m_geoInfo(false), m_matrix(1)
	{
		m_names.SetSize(0);
	}
};
