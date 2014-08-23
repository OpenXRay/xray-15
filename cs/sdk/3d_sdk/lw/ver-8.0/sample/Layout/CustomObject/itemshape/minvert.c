// Matrix Inversion
// Copyright 2001, NewTek, Inc.

#include "minvert.h"

static int LUDecompose (
	Matrix			 a,
	SVector			 indx)
{
	Vector			 scaling;
	double			 amax, d;
	int			 i, j, k, imax = 0;

	for (i = 0; i < ND; i++) {
		amax = 0.0;
		for (j = 0; j < ND; j++) {
			d = ABS (a[i][j]);
			amax = MAX (amax, d);
		}
		if (!amax)
			return 0;

		scaling[i] = 1.0 / amax;
	}

	/*
	 * Loop over columns using Crout's Method.
	 */
	for (j = 0; j < ND; j++) {

		/*
		 * lower left corner.
		 */
		for (i = 0; i < j; i++) {
			d = a[i][j];
			for (k = 0; k < i; k++)
				d -= a[i][k] * a[k][j];

			a[i][j] = d;
		}

		/*
		 * Initialize search for largest element.
		 */
		amax = 0.0;
      
		/*
		 * upper right corner.
		 */
		for (i = j; i < ND; i++) {
			d = a[i][j];
			for (k = 0; k < j; k++)
				d -= a[i][k] * a[k][j];

			a[i][j] = d;
			d = scaling[i] * ABS (d);
			if (d > amax) {
				amax = d;
				imax = i;
			}
		}

		/*
		 * Change rows if it is necessary.
		 */
		if (j != imax) {
			for (k = 0; k < ND; k++) {
				d = a[imax][k];
				a[imax][k] = a[j][k];
				a[j][k] = d;
			}
			scaling[imax] = scaling[j];
		}

		/*
		 * Mark the column with the pivot row.
		 */
		indx[j] = imax;

		/*
		 * Replace zeroes on the diagonal with a small number.
		 */
		if (a[j][j] == 0.0)
			a[j][j] = 1.0e-20;

		/*
		 * Divide by the pivot element.
		 */
		if (j != ND - 1) {
			d = 1.0 / a[j][j];
			for (i = j + 1; i < ND; i++)
				a[i][j] *= d;
		}
	}
	return 1;
}

//Do the backsubstitution on LU decomposition matrix.

static void LUBackSubstitute (
	Matrix				 a,
	Vector				 b,
	SVector				 indx)
{
	double				 sum;
	int				 i, ip, j, ii = -1;

	for (i = 0; i < ND; i++) {
		ip  = indx[i];
		sum = b[ip];

		b[ip] = b[i];
		if (ii != -1) {
			for (j = ii; j < i; j++)
				sum -= a[i][j] * b[j];

		} else if (sum)
			ii = i;

		b[i] = sum;
	}

	for (i = ND - 1; i >= 0; i--) {
		sum = b[i];
		for (j = i + 1; j < ND; j++)
			sum -= a[i][j] * b[j];

		b[i] = sum / a[i][i];
	}
}

static void MatrixCopy (Matrix dst, const Matrix src)
{
	int			 i;

	for (i = 0; i < ND; i++)
		VCPY (dst[i], src[i]);
}

void MatrixInit( Matrix m )
{
	int	i;
	for(i=0; i<3; i++)
	{
		VCLR((m[i]));
		m[i][i] = 1.0;
	}
}

/*Matrix inversion computes the LU decompostion and then does
backsubstitution with the b matrix being all zeros except for
a 1 in the row that matches the column we're in.
*/
int MatrixInvert (
	Matrix			 mInv,
	const Matrix		 m)
{
	Matrix			 lud;
	Vector			 col;
	SVector			 indx;
	int			 i, j;

	MatrixCopy (lud, m);
	if (!LUDecompose (lud, indx))
		return 0;

	for (j = 0; j < ND; j++) {
		VCLR (col);
		col[j] = 1.0;
		LUBackSubstitute (lud, col, indx);

		for (i = 0; i < ND; i++)
			mInv[i][j] = col[i];
	}
	return 1;
}

void MatrixApply(LWDVector v, Matrix mat, LWDVector src)
{
	v[0] = mat[0][0]*src[0] + mat[1][0]*src[1] + mat[2][0]*src[2];
	v[1] = mat[0][1]*src[0] + mat[1][1]*src[1] + mat[2][1]*src[2];
	v[2] = mat[0][2]*src[0] + mat[1][2]*src[1] + mat[2][2]*src[2];
}
