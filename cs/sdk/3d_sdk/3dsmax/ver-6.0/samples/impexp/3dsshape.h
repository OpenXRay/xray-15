
/* Shape work stuff & defines */

#ifndef _SHAPE_H_DEFINED

/* Shape info flag bit meanings */

#define SHPTICKS 1	/* Shape displayed with ticks on control pts */
#define SHPMODI 2	/* Shape uses MODICOUNT for steps */

/* Define edge flag bit meanings */

#define BRIDGE 4
#define SPLICE 2
#define VISEDGE 1

/* Template flag bit meanings */

#define TMPEND 0x0001		/* Last vertex of template polygon */
#define TMPCLOSED 0x0002	/* Template polygon closed */
#define TMPCTL 0x0004		/* Vertex is spline control pt */
#define TMPSMOOTH 0x0008	/* Vertex is smoothed */
#define TMPLINEAR 0x0010	/* Linear vertex on span */
#define TMPCSPAN 0x0020		/* Closing span */

#define TMPENDOFF 0xFFFE	/* Last vertex of template polygon */
#define TMPCLSOFF 0xFFFD	/* Template polygon closed */
#define TMPCTLOFF 0xFFFB	/* Vertex is spline control pt */
#define TMPSMOFF 0xFFF7		/* Vertex is smoothed */
#define TMPLINOFF 0xFFEF	/* Linear vertex on span */
#define TMPCSPOFF 0xFFDF	/* Closing span */

#undef POLYBOOL
#undef POLYINSIDE
#undef POLYOUTSIDE

/* Shape flags bit meanings */

#define POLYEND	0x0001		/* Set for last point in shape */
#define POLYCLOSED 0x0002	/* Set in last point to indicate closed poly */
#define POLYSPAN 0x0004		/* Span draw required */
#define POLYFLAG 0x0008		/* Misc-use flag */
#define POLYA	0x0010		/* Group A select bit */
#define POLYB	0x0020		/* Group B select bit */
#define POLYC	0x0040		/* Group C select bit */
#define POLYALL	0x0080		/* Entire poly selected bit */
#define POLYSHP	0x0100		/* Poly is part of shape bit */
#define POLYSTART 0x0200	/* Set for 1st control pt in poly */
#define POLYBOOL 0x0400		/* Boolean-created vertex */
#define POLYINSIDE 0x0800	/* Boolean: Inside other poly */
#define POLYOUTSIDE 0x1000	/* Boolean: Outside other poly */
#define POLYFROZEN 0x2000	/* Polygon is frozen */

#define POLYENDOFF 0xFFFE	/* Bitmask to turn off end flag */
#define POLYCLSOFF 0xFFFD	/* Bitmask to turn off closed flag */
#define POLYSPANOFF 0xFFFB	/* Bitmask to turn off span flag */
#define POLYFLAGOFF 0xFFF7	/* Bitmask to turn off selected flag */
#define POLYAOFF 0xFFEF		/* Group A select bit off */
#define POLYBOFF 0xFFDF		/* Group B select bit off */
#define POLYCOFF 0xFFBF		/* Group C select bit off */
#define POLYALLOFF 0xFF7F	/* Entire poly selected bit off */
#define POLYSHPOFF 0xFEFF	/* Poly is part of shape bit off */
#define POLYSTARTOFF 0xFDFF	/* 1st control pt in poly off */
#define POLYBOOLOFF 0xFBFF	/* Boolean-created vertex off */
#define POLYINSIDEOFF 0xF7FF	/* Boolean: Inside other poly off */
#define POLYOUTSIDEOFF 0x7FFF	/* Boolean: Outside other poly off */
#define POLYFROZENOFF 0xDFFF	/* Polygon is frozen off */

/* Spline open/closed flags */

#define NULLSPLINE -1	/* Partial segment */
#define OPEN 0		/* Open spline */
#define CLOSED 1	/* Closed spline */

#endif

#define _SHAPE_H_DEFINED

