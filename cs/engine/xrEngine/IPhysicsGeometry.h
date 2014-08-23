#ifndef	__IPHYSICS_GEOMETRY_H__
#define	__IPHYSICS_GEOMETRY_H__

xr_pure_interface	IPhysicsGeometry
{
	virtual		void		get_Box				(  Fmatrix& form, Fvector&	sz )const	=0;
	virtual		bool		collide_fluids		() const								=0;
};

#endif