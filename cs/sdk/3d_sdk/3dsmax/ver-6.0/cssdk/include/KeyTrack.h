#ifndef KEYTRACK_H
#define KEYTRACK_H

#include "BipExp.h"
#include "Tracks.h"

#define BIPSLAVE_CONTROL_CLASS_ID Class_ID(0x9154,0)

// this is the class for the center of mass, biped root controller ("Bip01")
#define BIPBODY_CONTROL_CLASS_ID  Class_ID(0x9156,0) 

// this is the class for the biped footstep controller ("Bip01 Footsteps")
#define FOOTPRINT_CLASS_ID Class_ID(0x3011,0)

#define SKELOBJ_CLASS_ID Class_ID(0x9125, 0)
#define BIPED_CLASS_ID Class_ID(0x9155, 0)

class vector {public: float  x,y,z,w;

    vector(){x=0.0;y=0.0;z=0.0;w=1.0;}; //constructors
    vector(float X, float Y, float Z)  { x = X; y = Y; z = Z; w= 1.0; };

	vector (float xx, float yy, float zz, float ww) : x(xx), y(yy), z(zz), w(ww) { }
	vector(Point4 p) : x(p.x), y(p.y), z(p.z), w(p.w) { }
 
	// Binary operators
	inline  vector operator-(const vector&) const;
	inline  vector operator+(const vector&) const; 
	  
};


 

inline vector vector::operator-(const vector& b) const {
	return(vector(x-b.x,y-b.y,z-b.z));
	}

inline vector vector::operator+(const vector& b) const {
	return(vector(x+b.x,y+b.y,z+b.z));
	}

inline vector operator*(float f, const vector& a) {
	return(vector(a.x*f, a.y*f, a.z*f, a.w)); //MG was a.w*f
	}

inline Point4 VtoP4(const vector& v) { return Point4(v.x, v.y, v.z, v.w); }


//typedef  struct {float  x,z;} vector2D;

//typedef Point4 vector;
typedef Point3 vector2D;

class path_property
{
public:
	float val;
	int   time;
	float distance;
	path_property(){
		val = 0.0;
		time = 0;
		distance = 0;
	}

};

class path_properties
{ 
      	public:
			float  turn_angle;
			float  line_distance;
			float  path_distance;
			float  average_speed;
			float speed_change;
			vector direction;
			path_property min_speed;
			path_property max_speed;
			path_property stop;
			path_properties(){
				turn_angle = 0.0;
				line_distance    = 0.0;
				path_distance = 0.0;
				average_speed = 0.0;
				speed_change  = 1.0;
				min_speed.val     = 1000000.0;
			}

		 path_properties& operator=(const path_properties& T);

};

class fingPos
        {
		  public:
          float jnt[MAXFINGERLINKS];
          float quat[4];
		  float Worldquat[4];
		  fingPos(){
		         quat[0] = 0.0;      
		         quat[1] = 0.0;      
		         quat[2] = 0.0;      
		         quat[3] = 1.0;
				 Worldquat[0] = 0.0;      
		         Worldquat[1] = 0.0;      
		         Worldquat[2] = 0.0;      
		         Worldquat[3] = 1.0;
				 jnt[0] = 0.0;
				 jnt[1] = 0.0;
				 jnt[2] = 0.0;
			  }
        }  ;


class handPos
         {
		   public:
           float yaw,pitch,roll;
           float  quat[4];
           //NEW FOR TBC control
           //float src_quat[4];
           //float des_quat[4];

		   float  inc_quat[4];
           float Worldquat[4];
           fingPos fingerPos[MAXFINGERS];
		   handPos()
		    {
			  yaw = 0.0;
			  pitch = 0.0;
			  roll  = 0.0;
			  quat[0] = 0.0;      
			  quat[1] = 0.0;      
			  quat[2] = 0.0;      
			  quat[3] = 1.0;
			  inc_quat[0] = 0.0;      
			  inc_quat[1] = 0.0;      
			  inc_quat[2] = 0.0;      
			  inc_quat[3] = 1.0;
			  Worldquat[0] = 0.0; 
			  Worldquat[1] = 0.0; 
			  Worldquat[2] = 0.0; 
			  Worldquat[3] = 1.0;
			}
         } ;

class limbPos
        {
		  public:

          float jnt[MAXLEGJNTS];	 // was MAXLINKS 
          vector pos;
          vector Worldpos;
          vector Pathpos;
          vector rot;
		  float knee_rotation;
          int gaitState;
          int state;
          float quat[4];
 
          float  clav_jnt[2];
          float  ikblend;
		  float  ikankleten;
          int    ikworld;
		  int    ik_link_index;
		  int    ik_toe_index;
		  int    ik_heel_index;
		  int    ik_use_toes;
		  int    ik_joined_pivot;
		  vector ik_pivot_pnt;
		  vector ik_moved_pivot_pnt;
		  handPos hand_pos;
		  limbPos(){
				   jnt[0]      = 0.0f;
				   jnt[1]      = 0.0f;
				   jnt[2]      = 0.0f;
				   jnt[3]      = 0.0f;
				   knee_rotation = 0.0f;
				   gaitState   = 0;
				   state       = 0;
				   quat[0]     = 0.0f;
				   quat[1]     = 0.0f;
				   quat[2]     = 0.0f;
				   quat[3]     = 1.0f;
				   clav_jnt[0] = -0.12f;
		           clav_jnt[1] = 0.0f;
                   ikblend     = 0.0f;
				   ikankleten  = 0.0f;
                   ikworld     = 0;
				   ik_link_index = 0;
				   ik_toe_index  = 0;
				   ik_heel_index = 5;
				   ik_use_toes = false;
				   ik_joined_pivot = false;
		          };
        };

#endif

