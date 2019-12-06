#pragma once

#include "../stdafx.h"
#include "../../hdtSSEUtils/FrameworkUtils.h"

#define FLT_DIG         6                       /* # of decimal digits of precision */
#define FLT_EPSILON     1.192092896e-07F        /* smallest such that 1.0+FLT_EPSILON != 1.0 */
#define FLT_GUARD       0
#define FLT_MANT_DIG    24                      /* # of bits in mantissa */
#define FLT_MAX         3.402823466e+38F        /* max value */
#define FLT_MAX_10_EXP  38                      /* max decimal exponent */
#define FLT_MAX_EXP     128                     /* max binary exponent */
#define FLT_MIN         1.175494351e-38F        /* min positive value */
#define FLT_MIN_10_EXP  (-37)                   /* min decimal exponent */
#define FLT_MIN_EXP     (-125)                  /* min binary exponent */
#define FLT_NORMALIZE   0
#define FLT_RADIX       2                       /* exponent radix */

#undef min
#undef max

namespace hdt
{
	typedef char I8;
	typedef short I16;
	typedef int I32;
	typedef long long I64;

	typedef unsigned char U8;
	typedef unsigned short U16;
	typedef unsigned int U32;
	typedef unsigned long long U64;

	template <int imm> __m128 pshufd(__m128 m) { return _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(m), imm)); }
	inline __m128 setAll(float f) { return pshufd<0>(_mm_load_ss(&f)); }
	inline __m128 setAll0(__m128 m) { return pshufd<0>(m); }
	inline __m128 setAll1(__m128 m) { return pshufd<0x55>(m); }
	inline __m128 setAll2(__m128 m) { return pshufd<0xAA>(m); }
	inline __m128 setAll3(__m128 m) { return pshufd<0xFF>(m); }

	inline __m128& operator +=(__m128& l, __m128 r) { l = _mm_add_ps(l, r); return l; }
	inline __m128& operator -=(__m128& l, __m128 r) { l = _mm_sub_ps(l, r); return l; }
	inline __m128& operator *=(__m128& l, __m128 r) { l = _mm_mul_ps(l, r); return l; }
	inline __m128& operator +=(__m128& l, float r) { l = _mm_add_ps(l, setAll(r)); return l; }
	inline __m128& operator -=(__m128& l, float r) { l = _mm_sub_ps(l, setAll(r)); return l; }
	inline __m128& operator *=(__m128& l, float r) { l = _mm_mul_ps(l, setAll(r)); return l; }

	inline __m128 cross(__m128 a, __m128 b)
	{
		__m128	T, V;

		T = pshufd<_MM_SHUFFLE(3, 0, 2, 1)>(a);
		V = pshufd<_MM_SHUFFLE(3, 0, 2, 1)>(b);

		V = _mm_mul_ps(V, a);
		T = _mm_mul_ps(T, b);
		V = _mm_sub_ps(V, T);

		V = pshufd<_MM_SHUFFLE(3, 0, 2, 1)>(V);
		return V;
	}

	inline float rsqrt(float number)
	{
		const float threehalfs = 1.5F;
		const float x2 = number * 0.5F;

		float res = number;
		U32& i = *reinterpret_cast<U32*>(&res);    // evil floating point bit level hacking
		i = 0x5f375a86 - (i >> 1);                             // what the fuck?
		res = res * (threehalfs - (x2 * res * res));   // 1st iteration
		res = res * (threehalfs - (x2 * res * res));   // 2nd iteration, this can be removed
		res = res * (threehalfs - (x2 * res * res));
		res = res * (threehalfs - (x2 * res * res));
		return res;
	}

	template<class T> inline T abs(T rhs) { return rhs < 0 ? -rhs : rhs; }
	template<> inline float abs(float rhs) { return _mm_cvtss_f32(_mm_andnot_ps(_mm_set_ss(-0.f), _mm_set_ss(rhs))); }

	template <class T> T min(const T& a, const T& b) restrict(cpu, amp) { return a < b ? a : b; }
	template <class T> T max(const T& a, const T& b) restrict(cpu, amp) { return a < b ? b : a; }

	inline int aligned(int x, int a) { return (x + a - 1) & -a; }
	template <int a> int aligned(int x) { return (x + a - 1) & -a; }

	inline U32 aligned2Pow(U32 lim)
	{
		unsigned long size;
		_BitScanReverse(&size, lim);
		size = 1 << size;
		return size;
	}


	ATTRIBUTE_ALIGNED16(class) btQsTransform
	{
		btQuaternion m_basis;
		btVector4 m_originScale;

	public:
		BT_DECLARE_ALIGNED_ALLOCATOR();

		btQsTransform() : m_originScale(0, 0, 0, 1), m_basis(btQuaternion::getIdentity()) {}
		btQsTransform(const btQuaternion& r, const btVector3& t, float s = 1.0f)
			: m_originScale(t.get128()), m_basis(r)
		{
			setScale(s);
		}

		btQsTransform(const btTransform& t, float s = 1.0f)
		{
			m_basis = t.getRotation();
			m_originScale = t.getOrigin();
			setScale(s);
		}

		btQsTransform(const btQsTransform& rhs)
			: m_basis(rhs.m_basis), m_originScale(rhs.m_originScale.get128())
		{
			assert(getScale() > 0);
		}

		inline btQuaternion getBasis() const { return m_basis; }
		inline btQuaternion& getBasis() { return m_basis; }
		inline void setBasis(const btQuaternion& q) { m_basis = q; }
		inline void setBasis(const btMatrix3x3& m) { m.getRotation(m_basis); }

		inline float getScale() const { return m_originScale[3]; }
		inline float& getScale() { return m_originScale[3]; }
		inline float getScaleReg() const { return _mm_cvtss_f32(setAll3(m_originScale.get128())); }
		inline void setScale(float s) { assert(s > 0); m_originScale[3] = s; }

		inline btVector3 getOrigin() const { return m_originScale; }
		inline void setOrigin(const btVector3& vec) { float s = getScale(); m_originScale = vec.mVec128; setScale(s); }
		inline void setOrigin(float x, float y, float z) { m_originScale[0] = x; m_originScale[1] = y; m_originScale[2] = z; }

		inline btQsTransform& operator =(const btQsTransform& rhs)
		{
			m_basis = rhs.m_basis;
			m_originScale = rhs.m_originScale;
			assert(getScale() > 0);
			return *this;
		}

		inline btQsTransform operator *(const btQsTransform& rhs) const
		{
			return btQsTransform(m_basis * rhs.m_basis, *this * rhs.getOrigin(), getScale() * rhs.getScale());
		}

		inline btVector3 operator *(const btVector3& rhs) const
		{
			return getOrigin() + quatRotate(m_basis, rhs * getScale());
		}

		inline void operator *=(const btQsTransform& rhs)
		{
			float s = getScale();
			setOrigin(getOrigin() + quatRotate(m_basis, rhs.getOrigin() * s));
			m_basis *= rhs.m_basis;
			setScale(s * rhs.getScale());
		}

		inline btQsTransform inverse() const
		{
			auto r = m_basis.inverse();
			auto s = 1 / getScale();
			return btQsTransform(r, quatRotate(r, -getOrigin() * s), s);
		}

		inline btTransform asTransform() const
		{
			return btTransform(m_basis, m_originScale);
		}
	};

	ATTRIBUTE_ALIGNED16(class) btMatrix4x3
	{
	public:
		btMatrix4x3() {}
		btMatrix4x3(const btQsTransform& t)
		{
			((btMatrix3x3*)this)->setRotation(t.getBasis());
			__m128 scale = pshufd<0xFF>(t.getOrigin().get128());
			m_row[0] = _mm_mul_ps(m_row[0], scale);
			m_row[1] = _mm_mul_ps(m_row[1], scale);
			m_row[2] = _mm_mul_ps(m_row[2], scale);
			m_row[0].m128_f32[3] = t.getOrigin()[0];
			m_row[1].m128_f32[3] = t.getOrigin()[1];
			m_row[2].m128_f32[3] = t.getOrigin()[2];
		}
		/*btMatrix4x3(const btMatrix4x3& rhs)
		{
		_mm_store_ps(m_row[0].m128_f32, rhs.m_row[0]);
		_mm_store_ps(m_row[1].m128_f32, rhs.m_row[1]);
		_mm_store_ps(m_row[2].m128_f32, rhs.m_row[2]);
		}*/

		inline btVector3 operator *(const btVector3& rhs) const
		{
#ifdef BT_ALLOW_SSE4
			auto v = _mm_blend_ps(rhs.get128(), _mm_set_ps1(1), 0x8);
			__m128 xmm0 = _mm_dp_ps(m_row[0], v, 0xF1);
			__m128 xmm1 = _mm_dp_ps(m_row[1], v, 0xF2);
			__m128 xmm2 = _mm_dp_ps(m_row[2], v, 0xF4);
			xmm0 = _mm_or_ps(xmm0, xmm1);
			xmm0 = _mm_or_ps(xmm0, xmm2);
#else
			auto v = rhs.get128();
			v.m128_f32[3] = 1;

			__m128 xmm0 = _mm_mul_ps(m_row[0], v);
			__m128 xmm1 = _mm_mul_ps(m_row[1], v);
			__m128 xmm2 = _mm_mul_ps(m_row[2], v);

			xmm0 = _mm_hadd_ps(xmm0, xmm1);
			xmm2 = _mm_hadd_ps(xmm2, xmm2);
			xmm0 = _mm_hadd_ps(xmm0, xmm2);
#endif
			return xmm0;
		}

		inline __m128 mulPack(const btVector3& rhs, float packW) const
		{
#ifdef BT_ALLOW_SSE4
			auto v = _mm_blend_ps(rhs.get128(), _mm_set_ps1(1), 0x8);
			__m128 xmm0 = _mm_dp_ps(m_row[0], v, 0xF1);		// x, 0, 0, 0
			__m128 xmm1 = _mm_dp_ps(m_row[1], v, 0xF2);		// 0, y, 0, 0
			xmm0 = _mm_or_ps(xmm0, xmm1);					// x, y, 0, 0
			xmm1 = _mm_dp_ps(m_row[2], v, 0xF1);			// z, 0, 0, 0
			xmm1 = _mm_unpacklo_ps(xmm1, _mm_set_ss(packW));// z, w, 0, 0
			xmm0 = _mm_movelh_ps(xmm0, xmm1);				// x, y, z, w
#else
			auto v = rhs.get128();
			v.m128_f32[3] = 1;
			auto w = _mm_load_ss(&packW);

			__m128 xmm0 = _mm_mul_ps(m_row[0], v);
			__m128 xmm1 = _mm_mul_ps(m_row[1], v);
			__m128 xmm2 = _mm_mul_ps(m_row[2], v);

			xmm0 = _mm_hadd_ps(xmm0, xmm1);
			xmm2 = _mm_hadd_ps(xmm2, w);
			xmm0 = _mm_hadd_ps(xmm0, xmm2);
#endif
			return xmm0;
		}

		/*inline btMatrix4x3& operator =(const btMatrix4x3& rhs)
		{
		_mm_store_ps(m_row[0].m128_f32, rhs.m_row[0]);
		_mm_store_ps(m_row[1].m128_f32, rhs.m_row[1]);
		_mm_store_ps(m_row[2].m128_f32, rhs.m_row[2]);
		return *this;
		}*/

		__m128 m_row[3];
	};

	ATTRIBUTE_ALIGNED16(class) btMatrix4x3T
	{
	public:
		btMatrix4x3T() {}
		btMatrix4x3T(const btQsTransform& t)
		{
			btMatrix3x3 rot;
			rot.setRotation(t.getBasis());
			rot = rot.transpose();
			__m128 scale = pshufd<0xFF>(t.getOrigin().get128());
			m_col[0] = rot[0].get128() * scale;
			m_col[1] = rot[1].get128() * scale;
			m_col[2] = rot[2].get128() * scale;
			m_col[3] = t.getOrigin().get128();
		}

		inline btVector3 operator *(const btVector3& rhs) const
		{
			return m_col[0] * rhs[0] + m_col[1] * rhs[1] + m_col[2] * rhs[2] + m_col[3];
		}

		inline btMatrix4x3T operator*(const btMatrix4x3T& r)
		{
			btMatrix4x3T ret;
			ret.m_col[0] = m_col[0] * r.m_col[0][0] + m_col[1] * r.m_col[0][1] + m_col[2] * r.m_col[0][2];
			ret.m_col[1] = m_col[0] * r.m_col[1][0] + m_col[1] * r.m_col[1][1] + m_col[2] * r.m_col[1][2];
			ret.m_col[2] = m_col[0] * r.m_col[2][0] + m_col[1] * r.m_col[2][1] + m_col[2] * r.m_col[2][2];
			ret.m_col[3] = *this * r.m_col[3];
			return ret;
		}

		inline btMatrix3x3 basis() const { return ((btMatrix3x3*)this)->transpose(); }
		inline btTransform toTransform() const { return btTransform(((btMatrix3x3*)this)->transpose(), m_col[3]); }
		/*inline btMatrix4x3& operator =(const btMatrix4x3& rhs)
		{
		_mm_store_ps(m_row[0].m128_f32, rhs.m_row[0]);
		_mm_store_ps(m_row[1].m128_f32, rhs.m_row[1]);
		_mm_store_ps(m_row[2].m128_f32, rhs.m_row[2]);
		return *this;
		}*/

		btVector3 m_col[4];
	};

	class RefObject
	{
	public:
		RefObject(){ m_refCount = 0; }
		virtual ~RefObject(){}

		void retain(){ m_refCount.fetch_add(1); }
		void release(){ assert(m_refCount > 0); if (m_refCount.fetch_sub(1) == 1) delete this; }
		long getRefCount(){ return m_refCount; }

	private:
		std::atomic_long m_refCount;
	};

	namespace ref
	{
		inline void retain(RefObject* o) { o->retain(); }
		inline void release(RefObject* o) { o->release(); }
	}

	template<> inline btVector3 abs(btVector3 rhs) { return _mm_andnot_ps(_mm_set_ps1(-0.f), rhs.get128()); }

	template<class T> using vectorA16 = std::vector<T>;

	struct SpinLock
	{
	public:
		inline void lock()
		{
			long count = 0;
			while (m_flag.test_and_set(std::memory_order_acquire))
			{
				if (++count > 10000)
					SwitchToThread();
			}
		}
		inline void unlock() { m_flag.clear(std::memory_order_release); }

	protected:
		std::atomic_flag m_flag = ATOMIC_FLAG_INIT;
	};
}
