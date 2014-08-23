#ifndef __TRACKS_H
#define __TRACKS_H

// The Biped Tracks
#define KEY_NOTHING    -1
#define KEY_LARM		0
#define KEY_RARM		1
#define KEY_LHAND		2
#define KEY_RHAND		3
#define KEY_LLEG		4
#define KEY_RLEG		5
#define KEY_LFOOT		6
#define KEY_RFOOT		7
#define KEY_SPINE		8
#define KEY_TAIL		9 
#define KEY_HEAD		10					
#define KEY_PELVIS		11		
#define KEY_VERTICAL	12
#define KEY_HORIZONTAL	13
#define KEY_TURN		14
#define KEY_FOOTPRINTS	15
#define KEY_NECK	    16
#define KEY_PONY1	    17
#define KEY_PONY2	    18

#define KEY_PROP1	    19
#define KEY_PROP2	    20
#define KEY_PROP3	    21

#define KEY_LFARM		22
#define KEY_RFARM		23

#define NKEYTRACKS      24

// RK: 10/12/99 defines for adapt locks
#define ADAPT_LOCKHOR	0	
#define ADAPT_LOCKVER	1
#define ADAPT_LOCKTURN	2
#define ADAPT_LOCKLLEG	3
#define ADAPT_LOCKRLEG	4
#define ADAPT_LOCKFFRM	5
#define ADAPT_LOCKTIME	6


#define NUB_START    NKEYTRACKS
#define NUB_RHAND   (NKEYTRACKS+0)
#define NUB_LHAND   (NKEYTRACKS+1) 
#define NUB_LFOOT   (NKEYTRACKS+2) 
#define NUB_RFOOT   (NKEYTRACKS+3)
#define NUB_TAIL    (NKEYTRACKS+4) 
#define NUB_HEAD    (NKEYTRACKS+5) 
#define NUB_PONY1   (NKEYTRACKS+6) 
#define NUB_PONY2   (NKEYTRACKS+7) 
#define NNUBTRACKS      8


// Track Selection used in IBipMaster::GetTrackSelection.. SetTrackSelection.
//bodytracks
#define NO_TRACK        0
#define HOR_TRACK       1
#define VER_TRACK       2
#define ROT_TRACK       3
#define HORVER_TRACK    4

#define MAXFINGERS      5
#define MAXFINGERLINKS  3
#define MAXLEGJNTS		4

#endif
