#include "hdtLCP.h"

namespace hdt
{

	btScalar btMachEps()
	{
		static bool calculated = false;
		static btScalar machEps = btScalar(1.);
		if (!calculated)
		{
			do {
				machEps /= btScalar(2.0);
				// If next epsilon yields 1, then break, because current
				// epsilon is the machine epsilon.
			} while ((btScalar)(1.0 + (machEps / btScalar(2.0))) != btScalar(1.0));
			//		printf( "\nCalculated Machine epsilon: %G\n", machEps );
			calculated = true;
		}
		return machEps;
	}

	btScalar btEpsRoot() {

		static btScalar epsroot = 0.;
		static bool alreadyCalculated = false;

		if (!alreadyCalculated) {
			epsroot = btSqrt(btMachEps());
			alreadyCalculated = true;
		}
		return epsroot;
	}

	float LargeDot(float* a, float* b, int n)
	{
		register __m128 xmm0 = _mm_setzero_ps();
		register int i;
		for (i = 0; i < n - 7; i+=8, a+=8, b+=8)
		{
			register __m128 xmm1 = _mm_loadu_ps(a);
			register __m128 xmm2 = _mm_loadu_ps(b);
			register __m128 xmm3 = _mm_loadu_ps(a + 4);
			register __m128 xmm4 = _mm_loadu_ps(b + 4);
			xmm1 *= xmm2;
			xmm3 *= xmm4;
			xmm1 += xmm3;
			xmm0 += xmm1;
		}

		xmm0 = _mm_hadd_ps(xmm0, xmm0);
		xmm0 = _mm_hadd_ps(xmm0, xmm0);

		float sum = _mm_cvtss_f32(xmm0);
		for (; i < n; ++i, ++a, ++b)
			sum += *a * *b;
		return sum;
	}

#define btLCP_FAST		// use fast btLCP object

	// option 1 : matrix row pointers (less data copying)
#define BTROWPTRS
#define BTATYPE btScalar **
#define BTAROW(i) (m_A[i])

	// option 2 : no matrix row pointers (slightly faster inner loops)
	//#define NOROWPTRS
	//#define BTATYPE btScalar *
	//#define BTAROW(i) (m_A+(i)*m_nskip)

#define BTNUB_OPTIMIZATIONS



	/* solve L*X=B, with B containing 1 right hand sides.
	* L is an n*n lower triangular matrix with ones on the diagonal.
	* L is stored by rows and its leading dimension is lskip.
	* B is an n*1 matrix that contains the right hand sides.
	* B is stored by columns and its leading dimension is also lskip.
	* B is overwritten with X.
	* this processes blocks of 2*2.
	* if this is in the factorizer source file, n must be a multiple of 2.
	*/

	static void btSolveL1_1(const btScalar *L, btScalar *B, int n, int lskip1)
	{
		/* declare variables - Z matrix, p and q vectors, etc */
		btScalar Z11, m11, Z21, m21, p1, q1, p2, *ex;
		const btScalar *ell;
		int i, j;
		/* compute all 2 x 1 blocks of X */
		for (i = 0; i < n; i += 2) {
			/* compute all 2 x 1 block of X, from rows i..i+2-1 */
			/* set the Z matrix to 0 */
			Z11 = 0;
			Z21 = 0;
			ell = L + i*lskip1;
			ex = B;
			/* the inner loop that computes outer products and adds them to Z */
			for (j = i - 4; j >= 0; j -= 4, ell+=4, ex+=4)
			{
				__m128 p1 = _mm_loadu_ps(ell);
				__m128 q1 = _mm_loadu_ps(ex);
				__m128 p2 = _mm_loadu_ps(ell + lskip1);
				
				Z11 += _mm_cvtss_f32(_mm_dp_ps(p1, q1, 0xf1));
				Z21 += _mm_cvtss_f32(_mm_dp_ps(p2, q1, 0xf1));
			}
			/* compute left-over iterations */
			for (j += 4; j > 0; j--) {
				/* compute outer product and add it to the Z matrix */
				p1 = ell[0];
				q1 = ex[0];
				m11 = p1 * q1;
				p2 = ell[lskip1];
				m21 = p2 * q1;
				/* advance pointers */
				ell += 1;
				ex += 1;
				Z11 += m11;
				Z21 += m21;
			}
			/* finish computing the X(i) block */
			Z11 = ex[0] - Z11;
			ex[0] = Z11;
			p1 = ell[lskip1];
			Z21 = ex[1] - Z21 - p1*Z11;
			ex[1] = Z21;
			/* end of outer loop */
		}
	}

	/* solve L*X=B, with B containing 2 right hand sides.
	* L is an n*n lower triangular matrix with ones on the diagonal.
	* L is stored by rows and its leading dimension is lskip.
	* B is an n*2 matrix that contains the right hand sides.
	* B is stored by columns and its leading dimension is also lskip.
	* B is overwritten with X.
	* this processes blocks of 2*2.
	* if this is in the factorizer source file, n must be a multiple of 2.
	*/

	static void btSolveL1_2(const btScalar *L, btScalar *B, int n, int lskip1)
	{
		/* declare variables - Z matrix, p and q vectors, etc */
		btScalar Z11, m11, Z12, m12, Z21, m21, Z22, m22, p1, q1, p2, q2, *ex;
		const btScalar *ell;
		int i, j;
		/* compute all 2 x 2 blocks of X */
		for (i = 0; i < n; i += 2)
		{
			/* compute all 2 x 2 block of X, from rows i..i+2-1 */
			/* set the Z matrix to 0 */
			Z11 = 0;
			Z12 = 0;
			Z21 = 0;
			Z22 = 0;
			ell = L + i*lskip1;
			ex = B;
			/* the inner loop that computes outer products and adds them to Z */
			for (j = i - 4; j >= 0; j -= 4, ell += 4, ex += 4)
			{
				/* compute outer product and add it to the Z matrix */
				__m128 p1 = _mm_loadu_ps(ell);
				__m128 q1 = _mm_loadu_ps(ex);
				__m128 p2 = _mm_loadu_ps(ell + lskip1);
				__m128 q2 = _mm_loadu_ps(ex + lskip1);
				
				Z11 += _mm_cvtss_f32(_mm_dp_ps(p1, q1, 0xf1));
				Z12 += _mm_cvtss_f32(_mm_dp_ps(p1, q2, 0xf1));
				Z21 += _mm_cvtss_f32(_mm_dp_ps(p2, q1, 0xf1));
				Z22 += _mm_cvtss_f32(_mm_dp_ps(p2, q2, 0xf1));
				/* end of inner loop */
			}
			/* compute left-over iterations */
			j += 4;
			for (; j > 0; j--) {
				/* compute outer product and add it to the Z matrix */
				p1 = ell[0];
				q1 = ex[0];
				m11 = p1 * q1;
				q2 = ex[lskip1];
				m12 = p1 * q2;
				p2 = ell[lskip1];
				m21 = p2 * q1;
				m22 = p2 * q2;
				/* advance pointers */
				ell += 1;
				ex += 1;
				Z11 += m11;
				Z12 += m12;
				Z21 += m21;
				Z22 += m22;
			}
			/* finish computing the X(i) block */
			Z11 = ex[0] - Z11;
			ex[0] = Z11;
			Z12 = ex[lskip1] - Z12;
			ex[lskip1] = Z12;
			p1 = ell[lskip1];
			Z21 = ex[1] - Z21 - p1*Z11;
			ex[1] = Z21;
			Z22 = ex[1 + lskip1] - Z22 - p1*Z12;
			ex[1 + lskip1] = Z22;
			/* end of outer loop */
		}
	}


	void btFactorLDLT(btScalar *A, btScalar *d, int n, int nskip1)
	{
		int i, j;
		btScalar sum, *ell, *dee, dd, p1, p2, q1, q2, Z11, m11, Z21, m21, Z22, m22;
		if (n < 1) return;

		for (i = 0; i <= n - 2; i += 2)
		{
			/* solve L*(D*l)=a, l is scaled elements in 2 x i block at A(i,0) */
			btSolveL1_2(A, A + i*nskip1, i, nskip1);
			/* scale the elements in a 2 x i block at A(i,0), and also */
			/* compute Z = the outer product matrix that we'll need. */
			Z11 = 0;
			Z21 = 0;
			Z22 = 0;
			ell = A + i*nskip1;
			dee = d;
			for (j = i - 4; j >= 0; j -= 4, ell+=4, dee+=4)
			{
				__m128 p1 = _mm_loadu_ps(ell);
				__m128 p2 = _mm_loadu_ps(ell+nskip1);
				__m128 dd = _mm_loadu_ps(dee);
				__m128 q1 = p1 * dd;
				__m128 q2 = p2 * dd;
				_mm_storeu_ps(ell, q1);
				_mm_storeu_ps(ell+nskip1, q2);
				Z11 += _mm_cvtss_f32(_mm_dp_ps(p1, q1, 0xf1));
				Z21 += _mm_cvtss_f32(_mm_dp_ps(p2, q1, 0xf1));
				Z22 += _mm_cvtss_f32(_mm_dp_ps(p2, q2, 0xf1));
			}
			/* compute left-over iterations */
			j += 4;
			for (; j > 0; j--) 
			{
				p1 = ell[0];
				p2 = ell[nskip1];
				dd = dee[0];
				q1 = p1*dd;
				q2 = p2*dd;
				ell[0] = q1;
				ell[nskip1] = q2;
				m11 = p1*q1;
				m21 = p2*q1;
				m22 = p2*q2;
				Z11 += m11;
				Z21 += m21;
				Z22 += m22;
				ell++;
				dee++;
			}
			/* solve for diagonal 2 x 2 block at A(i,i) */
			Z11 = ell[0] - Z11;
			Z21 = ell[nskip1] - Z21;
			Z22 = ell[1 + nskip1] - Z22;
			dee = d + i;
			/* factorize 2 x 2 block Z,dee */
			/* factorize row 1 */
			dee[0] = btRecip(Z11);
			/* factorize row 2 */
			sum = 0;
			q1 = Z21;
			q2 = q1 * dee[0];
			Z21 = q2;
			sum += q1*q2;
			dee[1] = btRecip(Z22 - sum);
			/* done factorizing 2 x 2 block */
			ell[nskip1] = Z21;
		}
		/* compute the (less than 2) rows at the bottom */
		switch (n - i) {
		case 0:
			break;

		case 1:
			btSolveL1_1(A, A + i*nskip1, i, nskip1);
			/* scale the elements in a 1 x i block at A(i,0), and also */
			/* compute Z = the outer product matrix that we'll need. */
			Z11 = 0;
			ell = A + i*nskip1;
			dee = d;
			for (j = i - 6; j >= 0; j -= 6) {
				p1 = ell[0];
				dd = dee[0];
				q1 = p1*dd;
				ell[0] = q1;
				m11 = p1*q1;
				Z11 += m11;
				p1 = ell[1];
				dd = dee[1];
				q1 = p1*dd;
				ell[1] = q1;
				m11 = p1*q1;
				Z11 += m11;
				p1 = ell[2];
				dd = dee[2];
				q1 = p1*dd;
				ell[2] = q1;
				m11 = p1*q1;
				Z11 += m11;
				p1 = ell[3];
				dd = dee[3];
				q1 = p1*dd;
				ell[3] = q1;
				m11 = p1*q1;
				Z11 += m11;
				p1 = ell[4];
				dd = dee[4];
				q1 = p1*dd;
				ell[4] = q1;
				m11 = p1*q1;
				Z11 += m11;
				p1 = ell[5];
				dd = dee[5];
				q1 = p1*dd;
				ell[5] = q1;
				m11 = p1*q1;
				Z11 += m11;
				ell += 6;
				dee += 6;
			}
			/* compute left-over iterations */
			j += 6;
			for (; j > 0; j--) {
				p1 = ell[0];
				dd = dee[0];
				q1 = p1*dd;
				ell[0] = q1;
				m11 = p1*q1;
				Z11 += m11;
				ell++;
				dee++;
			}
			/* solve for diagonal 1 x 1 block at A(i,i) */
			Z11 = ell[0] - Z11;
			dee = d + i;
			/* factorize 1 x 1 block Z,dee */
			/* factorize row 1 */
			dee[0] = btRecip(Z11);
			/* done factorizing 1 x 1 block */
			break;

			//default: *((char*)0)=0;  /* this should never happen! */
		}
	}

	/* solve L*X=B, with B containing 1 right hand sides.
	* L is an n*n lower triangular matrix with ones on the diagonal.
	* L is stored by rows and its leading dimension is lskip.
	* B is an n*1 matrix that contains the right hand sides.
	* B is stored by columns and its leading dimension is also lskip.
	* B is overwritten with X.
	* this processes blocks of 4*4.
	* if this is in the factorizer source file, n must be a multiple of 4.
	*/

	void btSolveL1(const btScalar *L, btScalar *B, int n, int lskip1)
	{
		/* declare variables - Z matrix, p and q vectors, etc */
		btScalar Z11, Z21, Z31, Z41, p1, q1, p2, p3, p4, *ex;
		const btScalar *ell;
		int lskip2, lskip3, i, j;
		/* compute lskip values */
		lskip2 = 2 * lskip1;
		lskip3 = 3 * lskip1;
		/* compute all 4 x 1 blocks of X */
		for (i = 0; i <= n - 4; i += 4)
		{
			/* compute all 4 x 1 block of X, from rows i..i+4-1 */
			ell = L + i*lskip1;
			ex = B;
			
			/* set the Z matrix to 0 */
			__m128 Z = _mm_setzero_ps();
			/* the inner loop that computes outer products and adds them to Z */
			for (j = i - 4; j >= 0; j -= 4, ell += 4, ex += 4)
			{
				__m128 p1 = _mm_loadu_ps(ell);
				__m128 p2 = _mm_loadu_ps(ell + lskip1);
				__m128 p3 = _mm_loadu_ps(ell + lskip2);
				__m128 p4 = _mm_loadu_ps(ell + lskip3);
				__m128 q1 = _mm_loadu_ps(ex);

				p1 = _mm_dp_ps(p1, q1, 0xf1);
				p2 = _mm_dp_ps(p2, q1, 0xf2);
				p3 = _mm_dp_ps(p3, q1, 0xf4);
				p4 = _mm_dp_ps(p4, q1, 0xf8);

				p1 = _mm_or_ps(p1, p3);
				p2 = _mm_or_ps(p2, p4);
				p1 = _mm_or_ps(p1, p2);
				Z = _mm_add_ps(Z, p1);
			}
			for (j += 4; j > 0; j--)
			{
				/* load p and q values */
				p1 = ell[0];
				q1 = ex[0];
				p2 = ell[lskip1];
				p3 = ell[lskip2];
				p4 = ell[lskip3];

				/* compute outer product and add it to the Z matrix */
				Z += _mm_set_ps(p4, p3, p2, p1) * setAll(q1);

				/* advance pointers */
				ell += 1;
				ex += 1;
			}
			Z11 = _mm_cvtss_f32(Z);
			Z21 = _mm_cvtss_f32(pshufd<1>(Z));
			Z31 = _mm_cvtss_f32(pshufd<2>(Z));
			Z41 = _mm_cvtss_f32(pshufd<3>(Z));

			/* finish computing the X(i) block */
			Z11 = ex[0] - Z11;
			ex[0] = Z11;
			p1 = ell[lskip1];
			Z21 = ex[1] - Z21 - p1*Z11;
			ex[1] = Z21;
			p1 = ell[lskip2];
			p2 = ell[1 + lskip2];
			Z31 = ex[2] - Z31 - p1*Z11 - p2*Z21;
			ex[2] = Z31;
			p1 = ell[lskip3];
			p2 = ell[1 + lskip3];
			p3 = ell[2 + lskip3];
			Z41 = ex[3] - Z41 - p1*Z11 - p2*Z21 - p3*Z31;
			ex[3] = Z41;
			/* end of outer loop */
		}
		/* compute rows at end that are not a multiple of block size */
		for (; i < n; i++)
		{
			/* compute all 1 x 1 block of X, from rows i..i+1-1 */
			/* set the Z matrix to 0 */
			Z11 = 0;
			ell = L + i*lskip1;
			ex = B;
			/* the inner loop that computes outer products and adds them to Z */
			for (j = i - 12; j >= 0; j -= 12)
			{
				__m128 p1 = _mm_loadu_ps(ell);
				__m128 q1 = _mm_loadu_ps(ex);
				__m128 p2 = _mm_loadu_ps(ell+4);
				__m128 q2 = _mm_loadu_ps(ex+4);
				__m128 p3 = _mm_loadu_ps(ell+8);
				__m128 q3 = _mm_loadu_ps(ex+8);

				Z11 += _mm_cvtss_f32(_mm_dp_ps(p1, q1, 0xf1));
				Z11 += _mm_cvtss_f32(_mm_dp_ps(p2, q2, 0xf1));
				Z11 += _mm_cvtss_f32(_mm_dp_ps(p3, q3, 0xf1));
				
				ell += 12;
				ex += 12;
			}

			for (j += 12; j > 0; j--)
			{
				/* load p and q values */
				p1 = ell[0];
				q1 = ex[0];
				/* compute outer product and add it to the Z matrix */
				Z11 += p1 * q1;
				/* advance pointers */
				ell += 1;
				ex += 1;
			}
			/* finish computing the X(i) block */
			Z11 = ex[0] - Z11;
			ex[0] = Z11;
		}
	}

	/* solve L^T * x=b, with b containing 1 right hand side.
	* L is an n*n lower triangular matrix with ones on the diagonal.
	* L is stored by rows and its leading dimension is lskip.
	* b is an n*1 matrix that contains the right hand side.
	* b is overwritten with x.
	* this processes blocks of 4.
	*/

	void btSolveL1T(const btScalar *L, btScalar *B, int n, int lskip1)
	{
		/* declare variables - Z matrix, p and q vectors, etc */
		btScalar Z11, m11, Z21, m21, Z31, m31, Z41, m41, p1, q1, p2, p3, p4, *ex;
		const btScalar *ell;
		int lskip2, i, j;
		//  int lskip3;
		/* special handling for L and B because we're solving L1 *transpose* */
		L = L + (n - 1)*(lskip1 + 1);
		B = B + n - 1;
		lskip1 = -lskip1;
		/* compute lskip values */
		lskip2 = 2 * lskip1;
		//lskip3 = 3*lskip1;
		/* compute all 4 x 1 blocks of X */
		for (i = 0; i <= n - 4; i += 4) {
			/* compute all 4 x 1 block of X, from rows i..i+4-1 */
			/* set the Z matrix to 0 */
			__m128 Z = _mm_setzero_ps();
			ell = L - i;
			ex = B;

			ell -= 3;
			/* the inner loop that computes outer products and adds them to Z */
			for (j = i - 4; j >= 0; j -= 4, ex -= 4)
			{
				__m128 q0 = _mm_loadu_ps(ex - 3);
				__m128 p0 = _mm_loadu_ps(ell);
				__m128 q = setAll3(q0);
				p0 = _mm_mul_ps(p0, q);
				Z = _mm_add_ps(Z, p0);
				ell += lskip1;

				p0 = _mm_loadu_ps(ell);
				q = setAll2(q0);
				p0 = _mm_mul_ps(p0, q);
				Z = _mm_add_ps(Z, p0);
				ell += lskip1;

				p0 = _mm_loadu_ps(ell);
				q = setAll1(q0);
				p0 = _mm_mul_ps(p0, q);
				Z = _mm_add_ps(Z, p0);
				ell += lskip1;

				p0 = _mm_loadu_ps(ell);
				q = setAll0(q0);
				p0 = _mm_mul_ps(p0, q);
				Z = _mm_add_ps(Z, p0);
				ell += lskip1;
				/* end of inner loop */
			}
			/* compute left-over iterations */
			j += 4;
			for (; j > 0; j--, ell += lskip1, --ex)
			{
				__m128 p = _mm_loadu_ps(ell);
				__m128 q = setAll(*ex);
				p = _mm_mul_ps(p, q);
				Z = _mm_add_ps(Z, p);
			}
			/* finish computing the X(i) block */
			ell += 3;
			Z11 = _mm_cvtss_f32(pshufd<3>(Z)); 
			Z21 = _mm_cvtss_f32(pshufd<2>(Z)); 
			Z31 = _mm_cvtss_f32(pshufd<1>(Z));
			Z41 = _mm_cvtss_f32(Z);

			Z11 = ex[0] - Z11;
			ex[0] = Z11;
			p1 = ell[-1];
			Z21 = ex[-1] - Z21 - p1*Z11;
			ex[-1] = Z21;
			p1 = ell[-2];
			p2 = ell[-2 + lskip1];
			Z31 = ex[-2] - Z31 - p1*Z11 - p2*Z21;
			ex[-2] = Z31;
			p1 = ell[-3];
			p2 = ell[-3 + lskip1];
			p3 = ell[-3 + lskip2];
			Z41 = ex[-3] - Z41 - p1*Z11 - p2*Z21 - p3*Z31;
			ex[-3] = Z41;
			/* end of outer loop */
		}
		/* compute rows at end that are not a multiple of block size */
		for (; i < n; i++) {
			/* compute all 1 x 1 block of X, from rows i..i+1-1 */
			/* set the Z matrix to 0 */
			Z11 = 0;
			ell = L - i;
			ex = B;
			/* the inner loop that computes outer products and adds them to Z */
			for (j = i - 4; j >= 0; j -= 4) {
				/* load p and q values */
				p1 = ell[0];
				q1 = ex[0];
				/* compute outer product and add it to the Z matrix */
				m11 = p1 * q1;
				ell += lskip1;
				Z11 += m11;
				/* load p and q values */
				p1 = ell[0];
				q1 = ex[-1];
				/* compute outer product and add it to the Z matrix */
				m11 = p1 * q1;
				ell += lskip1;
				Z11 += m11;
				/* load p and q values */
				p1 = ell[0];
				q1 = ex[-2];
				/* compute outer product and add it to the Z matrix */
				m11 = p1 * q1;
				ell += lskip1;
				Z11 += m11;
				/* load p and q values */
				p1 = ell[0];
				q1 = ex[-3];
				/* compute outer product and add it to the Z matrix */
				m11 = p1 * q1;
				ell += lskip1;
				ex -= 4;
				Z11 += m11;
				/* end of inner loop */
			}
			/* compute left-over iterations */
			j += 4;
			for (; j > 0; j--) {
				/* load p and q values */
				p1 = ell[0];
				q1 = ex[0];
				/* compute outer product and add it to the Z matrix */
				m11 = p1 * q1;
				ell += lskip1;
				ex -= 1;
				Z11 += m11;
			}
			/* finish computing the X(i) block */
			Z11 = ex[0] - Z11;
			ex[0] = Z11;
		}
	}



	void btVectorScale(btScalar *a, const btScalar *d, int n)
	{
		btAssert(a && d && n >= 0);
		for (int i = 0; i<n; i++) {
			a[i] *= d[i];
		}
	}

	void btSolveLDLT(const btScalar *L, const btScalar *d, btScalar *b, int n, int nskip)
	{
		btAssert(L && d && b && n > 0 && nskip >= n);
		btSolveL1(L, b, n, nskip);
		btVectorScale(b, d, n);
		btSolveL1T(L, b, n, nskip);
	}



	//***************************************************************************

	// swap row/column i1 with i2 in the n*n matrix A. the leading dimension of
	// A is nskip. this only references and swaps the lower triangle.
	// if `do_fast_row_swaps' is nonzero and row pointers are being used, then
	// rows will be swapped by exchanging row pointers. otherwise the data will
	// be copied.

	static void btSwapRowsAndCols(BTATYPE A, int n, int i1, int i2, int nskip,
		int do_fast_row_swaps)
	{
		btAssert(A && n > 0 && i1 >= 0 && i2 >= 0 && i1 < n && i2 < n &&
			nskip >= n && i1 < i2);

# ifdef BTROWPTRS
		btScalar *A_i1 = A[i1];
		btScalar *A_i2 = A[i2];
		for (int i = i1 + 1; i<i2; ++i) {
			btScalar *A_i_i1 = A[i] + i1;
			A_i1[i] = *A_i_i1;
			*A_i_i1 = A_i2[i];
		}
		A_i1[i2] = A_i1[i1];
		A_i1[i1] = A_i2[i1];
		A_i2[i1] = A_i2[i2];
		// swap rows, by swapping row pointers
		if (do_fast_row_swaps) {
			A[i1] = A_i2;
			A[i2] = A_i1;
		}
		else {
			// Only swap till i2 column to match A plain storage variant.
			for (int k = 0; k <= i2; ++k) {
				btScalar tmp = A_i1[k];
				A_i1[k] = A_i2[k];
				A_i2[k] = tmp;
			}
		}
		// swap columns the hard way
		for (int j = i2 + 1; j<n; ++j) {
			btScalar *A_j = A[j];
			btScalar tmp = A_j[i1];
			A_j[i1] = A_j[i2];
			A_j[i2] = tmp;
		}
# else
		btScalar *A_i1 = A + i1*nskip;
		btScalar *A_i2 = A + i2*nskip;
		for (int k = 0; k < i1; ++k) {
			btScalar tmp = A_i1[k];
			A_i1[k] = A_i2[k];
			A_i2[k] = tmp;
		}
		btScalar *A_i = A_i1 + nskip;
		for (int i = i1 + 1; i<i2; A_i += nskip, ++i) {
			btScalar tmp = A_i2[i];
			A_i2[i] = A_i[i1];
			A_i[i1] = tmp;
		}
		{
			btScalar tmp = A_i1[i1];
			A_i1[i1] = A_i2[i2];
			A_i2[i2] = tmp;
		}
		btScalar *A_j = A_i2 + nskip;
		for (int j = i2 + 1; j<n; A_j += nskip, ++j) {
			btScalar tmp = A_j[i1];
			A_j[i1] = A_j[i2];
			A_j[i2] = tmp;
		}
# endif
	}


	// swap two indexes in the n*n LCP problem. i1 must be <= i2.

	static void btSwapProblem(BTATYPE A, btScalar *x, btScalar *b, btScalar *w, btScalar *lo,
		btScalar *hi, int *p, bool *state, int *findex,
		int n, int i1, int i2, int nskip,
		int do_fast_row_swaps)
	{
		btScalar tmpr;
		int tmpi;
		bool tmpb;
		btAssert(n>0 && i1 >= 0 && i2 >= 0 && i1 < n && i2 < n && nskip >= n && i1 <= i2);
		if (i1 == i2) return;

		btSwapRowsAndCols(A, n, i1, i2, nskip, do_fast_row_swaps);

		tmpr = x[i1];
		x[i1] = x[i2];
		x[i2] = tmpr;

		tmpr = b[i1];
		b[i1] = b[i2];
		b[i2] = tmpr;

		tmpr = w[i1];
		w[i1] = w[i2];
		w[i2] = tmpr;

		tmpr = lo[i1];
		lo[i1] = lo[i2];
		lo[i2] = tmpr;

		tmpr = hi[i1];
		hi[i1] = hi[i2];
		hi[i2] = tmpr;

		tmpi = p[i1];
		p[i1] = p[i2];
		p[i2] = tmpi;

		tmpb = state[i1];
		state[i1] = state[i2];
		state[i2] = tmpb;

		if (findex) {
			tmpi = findex[i1];
			findex[i1] = findex[i2];
			findex[i2] = tmpi;
		}
	}




	//***************************************************************************
	// btLCP manipulator object. this represents an n*n LCP problem.
	//
	// two index sets C and N are kept. each set holds a subset of
	// the variable indexes 0..n-1. an index can only be in one set.
	// initially both sets are empty.
	//
	// the index set C is special: solutions to A(C,C)\A(C,i) can be generated.

	//***************************************************************************
	// fast implementation of btLCP. see the above definition of btLCP for
	// interface comments.
	//
	// `p' records the permutation of A,x,b,w,etc. p is initially 1:n and is
	// permuted as the other vectors/matrices are permuted.
	//
	// A,x,b,w,lo,hi,state,findex,p,c are permuted such that sets C,N have
	// contiguous indexes. the don't-care indexes follow N.
	//
	// an L*D*L' factorization is maintained of A(C,C), and whenever indexes are
	// added or removed from the set C the factorization is updated.
	// thus L*D*L'=A[C,C], i.e. a permuted top left nC*nC submatrix of A.
	// the leading dimension of the matrix L is always `nskip'.
	//
	// at the start there may be other indexes that are unbounded but are not
	// included in `nub'. btLCP will permute the matrix so that absolutely all
	// unbounded vectors are at the start. thus there may be some initial
	// permutation.
	//
	// the algorithms here assume certain patterns, particularly with respect to
	// index transfer.

#ifdef btLCP_FAST

	struct btLCP
	{
		const int m_n;
		const int m_nskip;
		int m_nub;
		int m_nC, m_nN;				// size of each index set
		BTATYPE const m_A;				// A rows
		btScalar *const m_x, *const m_b, *const m_w, *const m_lo, *const m_hi;	// permuted LCP problem data
		btScalar *const m_L, *const m_d;				// L*D*L' factorization of set C
		btScalar *const m_Dell, *const m_ell, *const m_tmp;
		bool *const m_state;
		int *const m_findex, *const m_p, *const m_C;

		btLCP(int _n, int _nskip, int _nub, btScalar *_Adata, btScalar *_x, btScalar *_b, btScalar *_w,
			btScalar *_lo, btScalar *_hi, btScalar *l, btScalar *_d,
			btScalar *_Dell, btScalar *_ell, btScalar *_tmp,
			bool *_state, int *_findex, int *p, int *c, btScalar **Arows);
		int getNub() const { return m_nub; }
		void transfer_i_to_C(int i);
		void transfer_i_to_N(int i) { m_nN++; }			// because we can assume C and N span 1:i-1
		void transfer_i_from_N_to_C(int i);
		void transfer_i_from_C_to_N(int i, btAlignedObjectArray<btScalar>& scratch);
		int numC() const { return m_nC; }
		int numN() const { return m_nN; }
		int indexC(int i) const { return i; }
		int indexN(int i) const { return i + m_nC; }
		btScalar Aii(int i) const  { return BTAROW(i)[i]; }
		btScalar AiC_times_qC(int i, btScalar *q) const { return LargeDot(BTAROW(i), q, m_nC); }
		btScalar AiN_times_qN(int i, btScalar *q) const { return LargeDot(BTAROW(i) + m_nC, q + m_nC, m_nN); }
		void pN_equals_ANC_times_qC(btScalar *p, btScalar *q);
		void pN_plusequals_ANi(btScalar *p, int i, int sign = 1);
		void pC_plusequals_s_times_qC(btScalar *p, btScalar s, btScalar *q);
		void pN_plusequals_s_times_qN(btScalar *p, btScalar s, btScalar *q);
		void solve1(btScalar *a, int i, int dir = 1, int only_transfer = 0);
		void unpermute();
	};


	btLCP::btLCP(int _n, int _nskip, int _nub, btScalar *_Adata, btScalar *_x, btScalar *_b, btScalar *_w,
		btScalar *_lo, btScalar *_hi, btScalar *l, btScalar *_d,
		btScalar *_Dell, btScalar *_ell, btScalar *_tmp,
		bool *_state, int *_findex, int *p, int *c, btScalar **Arows) :
		m_n(_n), m_nskip(_nskip), m_nub(_nub), m_nC(0), m_nN(0),
# ifdef BTROWPTRS
		m_A(Arows),
#else
		m_A(_Adata),
#endif
		m_x(_x), m_b(_b), m_w(_w), m_lo(_lo), m_hi(_hi),
		m_L(l), m_d(_d), m_Dell(_Dell), m_ell(_ell), m_tmp(_tmp),
		m_state(_state), m_findex(_findex), m_p(p), m_C(c)
	{
		{
			btSetZero(m_x, m_n);
		}

		{
# ifdef BTROWPTRS
			// make matrix row pointers
			btScalar *aptr = _Adata;
			BTATYPE A = m_A;
			const int n = m_n, nskip = m_nskip;
			for (int k = 0; k<n; aptr += nskip, ++k) A[k] = aptr;
# endif
		}

		{
			int *p = m_p;
			const int n = m_n;
			for (int k = 0; k<n; ++k) p[k] = k;		// initially unpermuted
		}

		/*
		// for testing, we can do some random swaps in the area i > nub
		{
		const int n = m_n;
		const int nub = m_nub;
		if (nub < n) {
		for (int k=0; k<100; k++) {
		int i1,i2;
		do {
		i1 = dRandInt(n-nub)+nub;
		i2 = dRandInt(n-nub)+nub;
		}
		while (i1 > i2);
		//printf ("--> %d %d\n",i1,i2);
		btSwapProblem (m_A,m_x,m_b,m_w,m_lo,m_hi,m_p,m_state,m_findex,n,i1,i2,m_nskip,0);
		}
		}
		*/

		// permute the problem so that *all* the unbounded variables are at the
		// start, i.e. look for unbounded variables not included in `nub'. we can
		// potentially push up `nub' this way and get a bigger initial factorization.
		// note that when we swap rows/cols here we must not just swap row pointers,
		// as the initial factorization relies on the data being all in one chunk.
		// variables that have findex >= 0 are *not* considered to be unbounded even
		// if lo=-inf and hi=inf - this is because these limits may change during the
		// solution process.

		{
			int *findex = m_findex;
			btScalar *lo = m_lo, *hi = m_hi;
			const int n = m_n;
			for (int k = m_nub; k<n; ++k) {
				if (findex && findex[k] >= 0) continue;
				if (lo[k] == -BT_INFINITY && hi[k] == BT_INFINITY) {
					btSwapProblem(m_A, m_x, m_b, m_w, lo, hi, m_p, m_state, findex, n, m_nub, k, m_nskip, 1);
					m_nub++;
				}
			}
		}

		// if there are unbounded variables at the start, factorize A up to that
		// point and solve for x. this puts all indexes 0..nub-1 into C.
		if (m_nub > 0) {
			const int nub = m_nub;
			{
				btScalar *Lrow = m_L;
				const int nskip = m_nskip;
				for (int j = 0; j<nub; Lrow += nskip, ++j) memcpy(Lrow, BTAROW(j), (j + 1)*sizeof(btScalar));
			}
			btFactorLDLT(m_L, m_d, nub, m_nskip);
			memcpy(m_x, m_b, nub*sizeof(btScalar));
			btSolveLDLT(m_L, m_d, m_x, nub, m_nskip);
			btSetZero(m_w, nub);
			{
				int *C = m_C;
				for (int k = 0; k<nub; ++k) C[k] = k;
			}
			m_nC = nub;
		}

		// permute the indexes > nub such that all findex variables are at the end
		if (m_findex) {
			const int nub = m_nub;
			int *findex = m_findex;
			int num_at_end = 0;
			for (int k = m_n - 1; k >= nub; k--) {
				if (findex[k] >= 0) {
					btSwapProblem(m_A, m_x, m_b, m_w, m_lo, m_hi, m_p, m_state, findex, m_n, k, m_n - 1 - num_at_end, m_nskip, 1);
					num_at_end++;
				}
			}
		}

		// print info about indexes
		/*
		{
		const int n = m_n;
		const int nub = m_nub;
		for (int k=0; k<n; k++) {
		if (k<nub) printf ("C");
		else if (m_lo[k]==-BT_INFINITY && m_hi[k]==BT_INFINITY) printf ("c");
		else printf (".");
		}
		printf ("\n");
		}
		*/
	}


	void btLCP::transfer_i_to_C(int i)
	{
		{
			if (m_nC > 0) {
				// ell,Dell were computed by solve1(). note, ell = D \ L1solve (L,A(i,C))
				{
					const int nC = m_nC;
					btScalar *const Ltgt = m_L + nC*m_nskip, *ell = m_ell;
					for (int j = 0; j<nC; ++j) Ltgt[j] = ell[j];
				}
				const int nC = m_nC;
				m_d[nC] = btRecip(BTAROW(i)[i] - LargeDot(m_ell, m_Dell, nC));
			}
			else {
				m_d[0] = btRecip(BTAROW(i)[i]);
			}

			btSwapProblem(m_A, m_x, m_b, m_w, m_lo, m_hi, m_p, m_state, m_findex, m_n, m_nC, i, m_nskip, 1);

			const int nC = m_nC;
			m_C[nC] = nC;
			m_nC = nC + 1; // nC value is outdated after this line
		}

	}


	void btLCP::transfer_i_from_N_to_C(int i)
	{
		{
			if (m_nC > 0) {
				{
					btScalar *const aptr = BTAROW(i);
					btScalar *Dell = m_Dell;
					const int *C = m_C;
#   ifdef BTNUB_OPTIMIZATIONS
					// if nub>0, initial part of aptr unpermuted
					const int nub = m_nub;
					int j = 0;
					for (; j<nub; ++j) Dell[j] = aptr[j];
					const int nC = m_nC;
					for (; j<nC; ++j) Dell[j] = aptr[C[j]];
#   else
					const int nC = m_nC;
					for (int j = 0; j<nC; ++j) Dell[j] = aptr[C[j]];
#   endif
				}
				btSolveL1(m_L, m_Dell, m_nC, m_nskip);
				{
					const int nC = m_nC;
					btScalar *const Ltgt = m_L + nC*m_nskip;
					btScalar *ell = m_ell, *Dell = m_Dell, *d = m_d;
					for (int j = 0; j<nC; ++j) Ltgt[j] = ell[j] = Dell[j] * d[j];
				}
				const int nC = m_nC;
				m_d[nC] = btRecip(BTAROW(i)[i] - LargeDot(m_ell, m_Dell, nC));
			}
			else {
				m_d[0] = btRecip(BTAROW(i)[i]);
			}

			btSwapProblem(m_A, m_x, m_b, m_w, m_lo, m_hi, m_p, m_state, m_findex, m_n, m_nC, i, m_nskip, 1);

			const int nC = m_nC;
			m_C[nC] = nC;
			m_nN--;
			m_nC = nC + 1; // nC value is outdated after this line
		}

		// @@@ TO DO LATER
		// if we just finish here then we'll go back and re-solve for
		// delta_x. but actually we can be more efficient and incrementally
		// update delta_x here. but if we do this, we wont have ell and Dell
		// to use in updating the factorization later.

	}

	void btRemoveRowCol(btScalar *A, int n, int nskip, int r)
	{
		btAssert(A && n > 0 && nskip >= n && r >= 0 && r < n);
		if (r >= n - 1) return;
		if (r > 0) {
			{
				const size_t move_size = (n - r - 1)*sizeof(btScalar);
				btScalar *Adst = A + r;
				for (int i = 0; i<r; Adst += nskip, ++i) {
					btScalar *Asrc = Adst + 1;
					memmove(Adst, Asrc, move_size);
				}
			}
			{
				const size_t cpy_size = r*sizeof(btScalar);
				btScalar *Adst = A + r * nskip;
				for (int i = r; i<(n - 1); ++i) {
					btScalar *Asrc = Adst + nskip;
					memcpy(Adst, Asrc, cpy_size);
					Adst = Asrc;
				}
			}
		}
		{
			const size_t cpy_size = (n - r - 1)*sizeof(btScalar);
			btScalar *Adst = A + r * (nskip + 1);
			for (int i = r; i<(n - 1); ++i) {
				btScalar *Asrc = Adst + (nskip + 1);
				memcpy(Adst, Asrc, cpy_size);
				Adst = Asrc - 1;
			}
		}
	}




	void btLDLTAddTL(btScalar *L, btScalar *d, const btScalar *a, int n, int nskip, btAlignedObjectArray<btScalar>& scratch)
	{
		btAssert(L && d && a && n > 0 && nskip >= n);

		if (n < 2) return;
		scratch.resize(2 * nskip);
		btScalar *W1 = &scratch[0];

		btScalar *W2 = W1 + nskip;

		W1[0] = btScalar(0.0);
		W2[0] = btScalar(0.0);
		for (int j = 1; j<n; ++j) {
			W1[j] = W2[j] = (btScalar)(a[j] * SIMDSQRT12);
		}
		btScalar W11 = (btScalar)((btScalar(0.5)*a[0] + 1)*SIMDSQRT12);
		btScalar W21 = (btScalar)((btScalar(0.5)*a[0] - 1)*SIMDSQRT12);

		btScalar alpha1 = btScalar(1.0);
		btScalar alpha2 = btScalar(1.0);

		{
			btScalar dee = d[0];
			btScalar alphanew = alpha1 + (W11*W11)*dee;
			btAssert(alphanew != btScalar(0.0));
			dee /= alphanew;
			btScalar gamma1 = W11 * dee;
			dee *= alpha1;
			alpha1 = alphanew;
			alphanew = alpha2 - (W21*W21)*dee;
			dee /= alphanew;
			//btScalar gamma2 = W21 * dee;
			alpha2 = alphanew;
			btScalar k1 = btScalar(1.0) - W21*gamma1;
			btScalar k2 = W21*gamma1*W11 - W21;
			btScalar *ll = L + nskip;
			for (int p = 1; p<n; ll += nskip, ++p) {
				btScalar Wp = W1[p];
				btScalar ell = *ll;
				W1[p] = Wp - W11*ell;
				W2[p] = k1*Wp + k2*ell;
			}
		}

		btScalar *ll = L + (nskip + 1);
		for (int j = 1; j<n; ll += nskip + 1, ++j) {
			btScalar k1 = W1[j];
			btScalar k2 = W2[j];

			btScalar dee = d[j];
			btScalar alphanew = alpha1 + (k1*k1)*dee;
			btAssert(alphanew != btScalar(0.0));
			dee /= alphanew;
			btScalar gamma1 = k1 * dee;
			dee *= alpha1;
			alpha1 = alphanew;
			alphanew = alpha2 - (k2*k2)*dee;
			dee /= alphanew;
			btScalar gamma2 = k2 * dee;
			dee *= alpha2;
			d[j] = dee;
			alpha2 = alphanew;

			btScalar *l = ll + nskip;
			for (int p = j + 1; p<n; l += nskip, ++p) {
				btScalar ell = *l;
				btScalar Wp = W1[p] - k1 * ell;
				ell += gamma1 * Wp;
				W1[p] = Wp;
				Wp = W2[p] - k2 * ell;
				ell -= gamma2 * Wp;
				W2[p] = Wp;
				*l = ell;
			}
		}
	}


#define _BTGETA(i,j) (A[i][j])
	//#define _GETA(i,j) (A[(i)*nskip+(j)])
#define BTGETA(i,j) ((i > j) ? _BTGETA(i,j) : _BTGETA(j,i))

	inline size_t btEstimateLDLTAddTLTmpbufSize(int nskip)
	{
		return nskip * 2 * sizeof(btScalar);
	}


	void btLDLTRemove(btScalar **A, const int *p, btScalar *L, btScalar *d,
		int n1, int n2, int r, int nskip, btAlignedObjectArray<btScalar>& scratch)
	{
		btAssert(A && p && L && d && n1 > 0 && n2 > 0 && r >= 0 && r < n2 &&
			n1 >= n2 && nskip >= n1);
#ifdef BT_DEBUG
		for (int i = 0; i<n2; ++i)
			btAssert(p[i] >= 0 && p[i] < n1);
#endif

		if (r == n2 - 1) {
			return;		// deleting last row/col is easy
		}
		else {
			size_t LDLTAddTL_size = btEstimateLDLTAddTLTmpbufSize(nskip);
			btAssert(LDLTAddTL_size % sizeof(btScalar) == 0);
			scratch.resize(nskip * 2 + n2);
			btScalar *tmp = &scratch[0];
			if (r == 0) {
				btScalar *a = (btScalar *)((char *)tmp + LDLTAddTL_size);
				const int p_0 = p[0];
				for (int i = 0; i<n2; ++i) {
					a[i] = -BTGETA(p[i], p_0);
				}
				a[0] += btScalar(1.0);
				btLDLTAddTL(L, d, a, n2, nskip, scratch);
			}
			else {
				btScalar *t = (btScalar *)((char *)tmp + LDLTAddTL_size);
				{
					btScalar *Lcurr = L + r*nskip;
					for (int i = 0; i<r; ++Lcurr, ++i) {
						btAssert(d[i] != btScalar(0.0));
						t[i] = *Lcurr / d[i];
					}
				}
				btScalar *a = t + r;
				{
					btScalar *Lcurr = L + r*nskip;
					const int *pp_r = p + r, p_r = *pp_r;
					const int n2_minus_r = n2 - r;
					for (int i = 0; i<n2_minus_r; Lcurr += nskip, ++i) {
						a[i] = LargeDot(Lcurr, t, r) - BTGETA(pp_r[i], p_r);
					}
				}
				a[0] += btScalar(1.0);
				btLDLTAddTL(L + r*nskip + r, d + r, a, n2 - r, nskip, scratch);
			}
		}

		// snip out row/column r from L and d
		btRemoveRowCol(L, n2, nskip, r);
		if (r < (n2 - 1)) memmove(d + r, d + r + 1, (n2 - r - 1)*sizeof(btScalar));
	}


	void btLCP::transfer_i_from_C_to_N(int i, btAlignedObjectArray<btScalar>& scratch)
	{
		{
			int *C = m_C;
			// remove a row/column from the factorization, and adjust the
			// indexes (black magic!)
			int last_idx = -1;
			const int nC = m_nC;
			int j = 0;
			for (; j<nC; ++j) {
				if (C[j] == nC - 1) {
					last_idx = j;
				}
				if (C[j] == i) {
					btLDLTRemove(m_A, C, m_L, m_d, m_n, nC, j, m_nskip, scratch);
					int k;
					if (last_idx == -1) {
						for (k = j + 1; k<nC; ++k) {
							if (C[k] == nC - 1) {
								break;
							}
						}
						btAssert(k < nC);
					}
					else {
						k = last_idx;
					}
					C[k] = C[j];
					if (j < (nC - 1)) memmove(C + j, C + j + 1, (nC - j - 1)*sizeof(int));
					break;
				}
			}
			btAssert(j < nC);

			btSwapProblem(m_A, m_x, m_b, m_w, m_lo, m_hi, m_p, m_state, m_findex, m_n, i, nC - 1, m_nskip, 1);

			m_nN++;
			m_nC = nC - 1; // nC value is outdated after this line
		}

	}


	void btLCP::pN_equals_ANC_times_qC(btScalar *p, btScalar *q)
	{
		// we could try to make this matrix-vector multiplication faster using
		// outer product matrix tricks, e.g. with the dMultidotX() functions.
		// but i tried it and it actually made things slower on random 100x100
		// problems because of the overhead involved. so we'll stick with the
		// simple method for now.
		const int nC = m_nC;
		btScalar *ptgt = p + nC;
		const int nN = m_nN;
		for (int i = 0; i<nN; ++i) {
			ptgt[i] = LargeDot(BTAROW(i + nC), q, nC);
		}
	}


	void btLCP::pN_plusequals_ANi(btScalar *p, int i, int sign)
	{
		const int nC = m_nC;
		btScalar *aptr = BTAROW(i) + nC;
		btScalar *ptgt = p + nC;
		if (sign > 0) {
			const int nN = m_nN;
			for (int j = 0; j<nN; ++j) ptgt[j] += aptr[j];
		}
		else {
			const int nN = m_nN;
			for (int j = 0; j<nN; ++j) ptgt[j] -= aptr[j];
		}
	}

	void btLCP::pC_plusequals_s_times_qC(btScalar *p, btScalar s, btScalar *q)
	{
		const int nC = m_nC;
		for (int i = 0; i<nC; ++i) {
			p[i] += s*q[i];
		}
	}

	void btLCP::pN_plusequals_s_times_qN(btScalar *p, btScalar s, btScalar *q)
	{
		const int nC = m_nC;
		btScalar *ptgt = p + nC, *qsrc = q + nC;
		const int nN = m_nN;
		for (int i = 0; i<nN; ++i) {
			ptgt[i] += s*qsrc[i];
		}
	}

	void btLCP::solve1(btScalar *a, int i, int dir, int only_transfer)
	{
		// the `Dell' and `ell' that are computed here are saved. if index i is
		// later added to the factorization then they can be reused.
		//
		// @@@ question: do we need to solve for entire delta_x??? yes, but
		//     only if an x goes below 0 during the step.

		if (m_nC > 0) {
			{
				btScalar *Dell = m_Dell;
				int *C = m_C;
				btScalar *aptr = BTAROW(i);
#   ifdef BTNUB_OPTIMIZATIONS
				// if nub>0, initial part of aptr[] is guaranteed unpermuted
				const int nub = m_nub;
				int j = 0;
				for (; j<nub; ++j) Dell[j] = aptr[j];
				const int nC = m_nC;
				for (; j<nC; ++j) Dell[j] = aptr[C[j]];
#   else
				const int nC = m_nC;
				for (int j = 0; j<nC; ++j) Dell[j] = aptr[C[j]];
#   endif
			}
			btSolveL1(m_L, m_Dell, m_nC, m_nskip);
			{
				btScalar *ell = m_ell, *Dell = m_Dell, *d = m_d;
				const int nC = m_nC;
				for (int j = 0; j<nC; ++j) ell[j] = Dell[j] * d[j];
			}

			if (!only_transfer) {
				btScalar *tmp = m_tmp, *ell = m_ell;
				{
					const int nC = m_nC;
					for (int j = 0; j<nC; ++j) tmp[j] = ell[j];
				}
				btSolveL1T(m_L, tmp, m_nC, m_nskip);
				if (dir > 0) {
					int *C = m_C;
					btScalar *tmp = m_tmp;
					const int nC = m_nC;
					for (int j = 0; j<nC; ++j) a[C[j]] = -tmp[j];
				}
				else {
					int *C = m_C;
					btScalar *tmp = m_tmp;
					const int nC = m_nC;
					for (int j = 0; j<nC; ++j) a[C[j]] = tmp[j];
				}
			}
		}
	}


	void btLCP::unpermute()
	{
		// now we have to un-permute x and w
		{
			memcpy(m_tmp, m_x, m_n*sizeof(btScalar));
			btScalar *x = m_x, *tmp = m_tmp;
			const int *p = m_p;
			const int n = m_n;
			for (int j = 0; j<n; ++j) x[p[j]] = tmp[j];
		}
		{
			memcpy(m_tmp, m_w, m_n*sizeof(btScalar));
			btScalar *w = m_w, *tmp = m_tmp;
			const int *p = m_p;
			const int n = m_n;
			for (int j = 0; j<n; ++j) w[p[j]] = tmp[j];
		}
	}

#endif // btLCP_FAST

	//***************************************************************************
	// an optimized Dantzig LCP driver routine for the lo-hi LCP problem.
	bool SolveDantzigLCP(int n, btScalar *A, btScalar *x, btScalar *b,
		btScalar* outer_w, int nub, btScalar *lo, btScalar *hi, int *findex, btDantzigScratchMemory& scratchMem)
	{
		bool s_error = false;
		//	printf("btSolveDantzigLCP n=%d\n",n);
		btAssert(n>0 && A && x && b && lo && hi && nub >= 0 && nub <= n);
		btAssert(outer_w);
		
		// if all the variables are unbounded then we can just factor, solve,
		// and return
		if (nub >= n)
		{
			int nskip = (n);
			btFactorLDLT(A, outer_w, n, nskip);
			btSolveLDLT(A, outer_w, b, n, nskip);
			memcpy(x, b, n*sizeof(btScalar));

			return !s_error;
		}

		const int nskip = (n);
		scratchMem.L.resize(n*nskip);

		scratchMem.d.resize(n);

		btScalar *w = outer_w;
		scratchMem.delta_w.resize(n);
		scratchMem.delta_x.resize(n);
		scratchMem.Dell.resize(n);
		scratchMem.ell.resize(n);
		scratchMem.Arows.resize(n);
		scratchMem.p.resize(n);
		scratchMem.C.resize(n);

		// for i in N, state[i] is 0 if x(i)==lo(i) or 1 if x(i)==hi(i)
		scratchMem.state.resize(n);


		// create LCP object. note that tmp is set to delta_w to save space, this
		// optimization relies on knowledge of how tmp is used, so be careful!
		btLCP lcp(n, nskip, nub, A, x, b, w, lo, hi, &scratchMem.L[0], &scratchMem.d[0], &scratchMem.Dell[0], &scratchMem.ell[0], &scratchMem.delta_w[0], &scratchMem.state[0], findex, &scratchMem.p[0], &scratchMem.C[0], &scratchMem.Arows[0]);
		int adj_nub = lcp.getNub();

		// loop over all indexes adj_nub..n-1. for index i, if x(i),w(i) satisfy the
		// LCP conditions then i is added to the appropriate index set. otherwise
		// x(i),w(i) is driven either +ve or -ve to force it to the valid region.
		// as we drive x(i), x(C) is also adjusted to keep w(C) at zero.
		// while driving x(i) we maintain the LCP conditions on the other variables
		// 0..i-1. we do this by watching out for other x(i),w(i) values going
		// outside the valid region, and then switching them between index sets
		// when that happens.

		bool hit_first_friction_index = false;
		for (int i = adj_nub; i<n; ++i)
		{
			s_error = false;
			// the index i is the driving index and indexes i+1..n-1 are "dont care",
			// i.e. when we make changes to the system those x's will be zero and we
			// don't care what happens to those w's. in other words, we only consider
			// an (i+1)*(i+1) sub-problem of A*x=b+w.

			// if we've hit the first friction index, we have to compute the lo and
			// hi values based on the values of x already computed. we have been
			// permuting the indexes, so the values stored in the findex vector are
			// no longer valid. thus we have to temporarily unpermute the x vector. 
			// for the purposes of this computation, 0*infinity = 0 ... so if the
			// contact constraint's normal force is 0, there should be no tangential
			// force applied.

			if (!hit_first_friction_index && findex && findex[i] >= 0) {
				// un-permute x into delta_w, which is not being used at the moment
				for (int j = 0; j<n; ++j) scratchMem.delta_w[scratchMem.p[j]] = x[j];

				// set lo and hi values
				for (int k = i; k<n; ++k) {
					btScalar wfk = scratchMem.delta_w[findex[k]];
					if (wfk == 0) {
						hi[k] = 0;
						lo[k] = 0;
					}
					else {
						hi[k] = btFabs(hi[k] * wfk);
						lo[k] = -hi[k];
					}
				}
				hit_first_friction_index = true;
			}

			// thus far we have not even been computing the w values for indexes
			// greater than i, so compute w[i] now.
			w[i] = lcp.AiC_times_qC(i, x) + lcp.AiN_times_qN(i, x) - b[i];

			// if lo=hi=0 (which can happen for tangential friction when normals are
			// 0) then the index will be assigned to set N with some state. however,
			// set C's line has zero size, so the index will always remain in set N.
			// with the "normal" switching logic, if w changed sign then the index
			// would have to switch to set C and then back to set N with an inverted
			// state. this is pointless, and also computationally expensive. to
			// prevent this from happening, we use the rule that indexes with lo=hi=0
			// will never be checked for set changes. this means that the state for
			// these indexes may be incorrect, but that doesn't matter.

			// see if x(i),w(i) is in a valid region
			if (lo[i] == 0 && w[i] >= 0)
			{
				lcp.transfer_i_to_N(i);
				scratchMem.state[i] = false;
			}
			else if (hi[i] == 0 && w[i] <= 0)
			{
				lcp.transfer_i_to_N(i);
				scratchMem.state[i] = true;
			}
			else if (w[i] == 0)
			{
				// this is a degenerate case. by the time we get to this test we know
				// that lo != 0, which means that lo < 0 as lo is not allowed to be +ve,
				// and similarly that hi > 0. this means that the line segment
				// corresponding to set C is at least finite in extent, and we are on it.
				// NOTE: we must call lcp.solve1() before lcp.transfer_i_to_C()
				lcp.solve1(&scratchMem.delta_x[0], i, 0, 1);

				lcp.transfer_i_to_C(i);
			}
			else
			{
				// we must push x(i) and w(i)
				for (;;)
				{
					int dir;
					btScalar dirf;
					// find direction to push on x(i)
					if (w[i] <= 0)
					{
						dir = 1;
						dirf = btScalar(1.0);
					}
					else
					{
						dir = -1;
						dirf = btScalar(-1.0);
					}

					// compute: delta_x(C) = -dir*A(C,C)\A(C,i)
					lcp.solve1(&scratchMem.delta_x[0], i, dir);

					// note that delta_x[i] = dirf, but we wont bother to set it

					// compute: delta_w = A*delta_x ... note we only care about
					// delta_w(N) and delta_w(i), the rest is ignored
					lcp.pN_equals_ANC_times_qC(&scratchMem.delta_w[0], &scratchMem.delta_x[0]);
					lcp.pN_plusequals_ANi(&scratchMem.delta_w[0], i, dir);
					scratchMem.delta_w[i] = lcp.AiC_times_qC(i, &scratchMem.delta_x[0]) + lcp.Aii(i)*dirf;

					// find largest step we can take (size=s), either to drive x(i),w(i)
					// to the valid LCP region or to drive an already-valid variable
					// outside the valid region.

					int cmd = 1;		// index switching command
					int si = 0;		// si = index to switch if cmd>3
					btScalar s = -w[i] / scratchMem.delta_w[i];
					if (dir > 0)
					{
						if (hi[i] < BT_INFINITY)
						{
							btScalar s2 = (hi[i] - x[i])*dirf;	// was (hi[i]-x[i])/dirf	// step to x(i)=hi(i)
							if (s2 < s)
							{
								s = s2;
								cmd = 3;
							}
						}
					}
					else
					{
						if (lo[i] > -BT_INFINITY)
						{
							btScalar s2 = (lo[i] - x[i])*dirf;	// was (lo[i]-x[i])/dirf	// step to x(i)=lo(i)
							if (s2 < s)
							{
								s = s2;
								cmd = 2;
							}
						}
					}

					const int numN = lcp.numN();
					for (int k = 0; k < numN; ++k)
					{
						const int indexN_k = lcp.indexN(k);
						if (!scratchMem.state[indexN_k] ? scratchMem.delta_w[indexN_k] < 0 : scratchMem.delta_w[indexN_k] > 0)
						{
							// don't bother checking if lo=hi=0
							if (lo[indexN_k] == 0 && hi[indexN_k] == 0) continue;
							btScalar s2 = -w[indexN_k] / scratchMem.delta_w[indexN_k];
							if (s2 < s) {
								s = s2;
								cmd = 4;
								si = indexN_k;
							}
						}
					}

					const int numC = lcp.numC();
					for (int k = adj_nub; k < numC; ++k)
					{
						const int indexC_k = lcp.indexC(k);
						if (scratchMem.delta_x[indexC_k] < 0 && lo[indexC_k] > -BT_INFINITY)
						{
							btScalar s2 = (lo[indexC_k] - x[indexC_k]) / scratchMem.delta_x[indexC_k];
							if (s2 < s)
							{
								s = s2;
								cmd = 5;
								si = indexC_k;
							}
						}
						if (scratchMem.delta_x[indexC_k] > 0 && hi[indexC_k] < BT_INFINITY)
						{
							btScalar s2 = (hi[indexC_k] - x[indexC_k]) / scratchMem.delta_x[indexC_k];
							if (s2 < s)
							{
								s = s2;
								cmd = 6;
								si = indexC_k;
							}
						}
					}

					//static char* cmdstring[8] = {0,"->C","->NL","->NH","N->C",
					//			     "C->NL","C->NH"};
					//printf ("cmd=%d (%s), si=%d\n",cmd,cmdstring[cmd],(cmd>3) ? si : i);

					// if s <= 0 then we've got a problem. if we just keep going then
					// we're going to get stuck in an infinite loop. instead, just cross
					// our fingers and exit with the current solution.
					if (s <= btScalar(0.0))
					{
						//          printf("LCP internal error, s <= 0 (s=%.4e)",(double)s);
						if (i < n)
						{
							btSetZero(x + i, n - i);
							btSetZero(w + i, n - i);
						}
						s_error = true;
						break;
					}

					// apply x = x + s * delta_x
					lcp.pC_plusequals_s_times_qC(x, s, &scratchMem.delta_x[0]);
					x[i] += s * dirf;

					// apply w = w + s * delta_w
					lcp.pN_plusequals_s_times_qN(w, s, &scratchMem.delta_w[0]);
					w[i] += s * scratchMem.delta_w[i];

					//        void *tmpbuf;
					// switch indexes between sets if necessary
					switch (cmd) {
					case 1:		// done
						w[i] = 0;
						lcp.transfer_i_to_C(i);
						break;
					case 2:		// done
						x[i] = lo[i];
						scratchMem.state[i] = false;
						lcp.transfer_i_to_N(i);
						break;
					case 3:		// done
						x[i] = hi[i];
						scratchMem.state[i] = true;
						lcp.transfer_i_to_N(i);
						break;
					case 4:		// keep going
						w[si] = 0;
						lcp.transfer_i_from_N_to_C(si);
						break;
					case 5:		// keep going
						x[si] = lo[si];
						scratchMem.state[si] = false;
						lcp.transfer_i_from_C_to_N(si, scratchMem.m_scratch);
						break;
					case 6:		// keep going
						x[si] = hi[si];
						scratchMem.state[si] = true;
						lcp.transfer_i_from_C_to_N(si, scratchMem.m_scratch);
						break;
					}

					if (cmd <= 3) break;
				} // for (;;)
			} // else

			if (s_error)
			{
				break;
			}
		} // for (int i=adj_nub; i<n; ++i)

		lcp.unpermute();

		return !s_error;
	}

	bool DantzigSolver::solveMLCP(const btMatrixXu & A, const btVectorXu & b, btVectorXu& x, const btVectorXu & lo, const btVectorXu & hi, const btAlignedObjectArray<int>& limitDependency, int numIterations, bool useSparsity)
	{
		bool result = true;
		int n = b.rows();
		if (n)
		{
			int nub = 0;
			btAlignedObjectArray<btScalar> ww;
			ww.resize(n);

			const btScalar* Aptr = A.getBufferPointer();
			m_A.resize(n*n);
			for (int i = 0; i<n*n; i++)
				m_A[i] = Aptr[i];

			m_b.resize(n);
			m_x.resize(n);
			m_lo.resize(n);
			m_hi.resize(n);
			m_dependencies.resize(n);
			for (int i = 0; i<n; i++)
			{
				m_lo[i] = lo[i];
				m_hi[i] = hi[i];
				m_b[i] = b[i];
				m_x[i] = x[i];
				m_dependencies[i] = limitDependency[i];
			}


			result = SolveDantzigLCP(n, &m_A[0], &m_x[0], &m_b[0], &ww[0], nub, &m_lo[0], &m_hi[0], &m_dependencies[0], m_scratchMemory);
			if (!result)
				return result;

			//			printf("numAllocas = %d\n",numAllocas);
			for (int i = 0; i<n; i++)
			{
				volatile btScalar xx = m_x[i];
				if (xx != m_x[i])
					return false;
				if (x[i] >= m_acceptableUpperLimitSolution)
					return false;

				if (x[i] <= -m_acceptableUpperLimitSolution)
					return false;
			}

			for (int i = 0; i<n; i++)
			{
				x[i] = m_x[i];
			}

		}

		return result;
	}
}