#ifndef __kfio__
#define __kfio__


#define DOGIO
#define KFVERSION 5

/* kfio.h: defs for saving and loading .KFB files */

#define UNREC_OBJECT_NAME 0xff00
#define BAD_PARENT_REF 0xff01
#define BAD_OBJ_NAME 0xff02

#define NO_PARENT 0xFFFF

#define M3DMAGIC 0x4D4D	
#define M3D_VERSION 0x0002

#define KFDATA  0xB000
#define KFHDR 0xB00A
#define AMBIENT_NODE_TAG  0xB001
#define OBJECT_NODE_TAG  0xB002
#define CAMERA_NODE_TAG  0xB003
#define TARGET_NODE_TAG  0xB004	 /* for cameras only */
#define LIGHT_NODE_TAG  0xB005
#define L_TARGET_NODE_TAG  0xB006	 /* for lights only */
#define SPOTLIGHT_NODE_TAG  0xB007
#define KFSEG  0xB008
#define KFCURTIME  0xB009
#define NODE_HDR  0xB010
#define INSTANCE_NAME 0xB011
#define PRESCALE 0xB012
#define PIVOT 0xB013
#define BOUNDBOX  0xB014
#define MORPH_SMOOTH  0xB015
#define POS_TRACK_TAG 0xB020
#define ROT_TRACK_TAG 0xB021
#define SCL_TRACK_TAG 0xB022
#define FOV_TRACK_TAG 0xB023
#define ROLL_TRACK_TAG 0xB024
#define COL_TRACK_TAG 0xB025
#define MORPH_TRACK_TAG 0xB026
#define HOT_TRACK_TAG 0xB027
#define FALL_TRACK_TAG 0xB028
#define HIDE_TRACK_TAG 0xB029
#define NODE_ID 0xB030

#define W_TENS  1
#define W_CONT  (1<<1)
#define W_BIAS  (1<<2)
#define W_EASETO  (1<<3)
#define W_EASEFROM  (1<<4)


#endif
