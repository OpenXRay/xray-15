/******************************************************************************
 * Copyright 1986-2007 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Created:	27.05.98
 * Module:	physics
 * Purpose:	Physical light source
 *
 * Exports:
 *
 * History:
 *
 * Description:
 *****************************************************************************/

struct physical_light {
	miColor		color;		/* energy */
	miScalar	cone;		/* inner solid cone (for spot light) */
	miScalar	threshold;	/* accuracy threshold for optimiz. */
	miScalar	cos_exp;	/* cosine expon. (disc and rect only)*/
};

extern "C" {
DLLEXPORT int	    physical_light_version (void);
DLLEXPORT miBoolean physical_light	   (miColor *, miState *,
					    struct physical_light *);
}
