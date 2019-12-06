#include "hdtCollisionAlgorithm.h"

namespace hdt
{
	CheckTriangle::CheckTriangle(const btVector3& p0, const btVector3& p1, const btVector3& p2, float margin, float prenetration)
		: p0(p0), p1(p1), p2(p2), margin(margin), prenetration(prenetration)
	{
		normal = (p1 - p0).cross(p2 - p0);
		__m128 len2 = _mm_dp_ps(normal.get128(), normal.get128(), 0x71);
		if (_mm_cvtss_f32(len2) < FLT_EPSILON * FLT_EPSILON)
		{
			valid = false;
		}
		else
		{
			valid = true;
			normal.set128(_mm_div_ps(normal.get128(), setAll0(_mm_sqrt_ss(len2))));

			if (prenetration > -FLT_EPSILON && prenetration < FLT_EPSILON)
				prenetration = 0;

			if (prenetration < 0)
			{
				//triangle facing the other way
				normal = -normal;
				prenetration = -prenetration;
			}
			this->prenetration = prenetration;
		}
	}

	inline static float signedDistanceFromPointToPlane(const btVector3& point, const btVector3& n, float c, btVector3& closestPointOnFace)
	{
		float dist = n.dot(point) + c;
		closestPointOnFace = point - n * dist;
		return dist;
	}

	inline static float segmentSqrDistance(const btVector3& from, const btVector3& to, const btVector3 &p, btVector3 &nearest)
	{
		btVector3 diff = p - from;
		btVector3 v = to - from;
		float t = v.dot(diff);

		if (t > 0)
		{
			float dotVV = v.dot(v);
			if (t < dotVV)
			{
				t /= dotVV;
				diff -= v*t;
			}
			else
			{
				t = 1;
				diff -= v;
			}
		}
		else t = 0;

		nearest = from + v*t;
		return diff.length2();
	}

	inline static bool pointInTriangle(const btVector3& p1, const btVector3& p2, const btVector3& p3, const btVector3& normal, const btVector3& p)
	{
		auto ab = p2 - p1;
		auto ac = p3 - p1;

		auto ap = p - p1;
		auto d1 = ab.dot(ap);
		auto d2 = ac.dot(ap);

		if (d1 <= 0 && d2 <= 0) return false;				//1,0,0

		auto bp = p - p2;
		auto d3 = ab.dot(bp);
		auto d4 = ac.dot(bp);
		if (d3 >= 0 && d4 <= d3) return false;				//0,1,0

		auto cp = p - p3;
		auto d5 = ab.dot(cp);
		auto d6 = ac.dot(cp);
		if (d6 >= 0 && d5 <= d6) return false;				//0,0,1

		auto vc = d1*d4 - d3*d2;
		if (vc <= 0 && d1 >= 0 && d3 <= 0) return false;	//u,v,0

		auto vb = d5*d2 - d1*d6;
		if (vb <= 0 && d2 >= 0 && d6 <= 0) return false;	//u,0,w

		auto va = d3*d6 - d5*d4;
		if (va <= 0 && d4 >= d3 && d5 >= d6) return false;	//0,v,w

		return true;
	}


	bool checkSphereSphere(const btVector3& a, const btVector3& b, float ra, float rb, CollisionResult& res)
	{
		btVector3 diff = a - b;
		float dist2 = diff.length2();
		float bound2 = (ra + rb) * (ra + rb);
		if (dist2 > bound2)
			return false;

		float len = sqrt(dist2);
		float dist = len - (ra + rb);

		res.normOnB = btVector3(1, 0, 0);
		if (len > FLT_EPSILON)
			res.normOnB = diff.normalized();

		res.posA = a - res.normOnB*ra;
		res.posB = b + res.normOnB*rb;

		res.depth = dist;

		return true;
	}

	bool checkSphereTriangle(const btVector3& s, float r, const CheckTriangle& tri, CollisionResult& res)
	{
		//if (normal.fuzzyZero()) return false;
		//normal.normalize();
		btVector3 p1ToCentre = s - tri.p0;
		float distanceFromPlane = p1ToCentre.dot(tri.normal);
		float radiusWithMargin = r + tri.margin;
		
		auto normal = tri.normal;
		auto prenetration = tri.prenetration;
		
		bool isInsideContactPlane;
		if (prenetration >= FLT_EPSILON)
			isInsideContactPlane = distanceFromPlane < radiusWithMargin && distanceFromPlane >= -prenetration;
		else
		{
			if (distanceFromPlane < 0)
			{
				distanceFromPlane = -distanceFromPlane;
				normal = -normal;
			}
			isInsideContactPlane = distanceFromPlane > -radiusWithMargin;
		}

		// Check for contact / intersection
		bool hasContact = false;
		btVector3 contactPoint;
		if (isInsideContactPlane)
		{
			if (pointInTriangle(tri.p0, tri.p1, tri.p2, normal, s))
			{
				// Inside the contact wedge - touches a point on the shell plane
				hasContact = true;
				contactPoint = s - normal*distanceFromPlane;
			}
			//else if (prenetration < FLT_EPSILON)
			//{
			//	// Could be inside one of the contact capsules
			//	float contactCapsuleRadiusSqr = radiusWithMargin*radiusWithMargin;
			//	btVector3 p[] = { tri.p0, tri.p1, tri.p2, tri.p0 };
			//	for (int i = 0; i < 3; i++)
			//	{
			//		if (segmentSqrDistance(p[i], p[i + 1], s, contactPoint) < contactCapsuleRadiusSqr)
			//		{
			//			// Yep, we're inside a capsule
			//			auto diff = s - contactPoint;
			//			if (diff.length2() > FLT_EPSILON)
			//			{
			//				hasContact = true;
			//				distanceFromPlane = diff.length();
			//				normal = diff.normalized();
			//				break;
			//			}
			//		}
			//	}
			//}
		}

		if (hasContact)
		{
			res.posA = s - normal * r;
			res.posB = contactPoint;
			res.normOnB = normal;
			res.depth = distanceFromPlane - radiusWithMargin;
			return res.depth < -FLT_EPSILON;
		}
		return false;
	}

	bool checkTriangleSphere(const btVector3& s, float r, const CheckTriangle& tri, CollisionResult& res)
	{
		auto ret = checkSphereTriangle(s, r, tri, res);
		if (ret)
		{
			res.normOnB = -res.normOnB;
			std::swap(res.posA, res.posB);
		}
		return ret;
	}

	static bool linePlaneIntersection(btVector3& contact, const btVector3& p0, const btVector3& p1, const btVector3& normal, const btVector3& coord, float radius)
	{
		// get d value
		float d = normal.dot(coord);
		auto dir = p1 - p0;
		if (normal.dot(dir) < FLT_EPSILON) {
			return false; // No intersection, the line is parallel to the plane
		}

		// Compute the X value for the directed line ray intersecting the plane
		float e = radius / dir.length();
		float x = (d - normal.dot(p0)) / normal.dot(dir);
		if (x <= 0.f || x >= 1.f + e)
			return false;

		x = btClamped(x - e, 0.f, 1.f);
		// output contact point
		contact = p0 + dir * x; //Make sure your ray vector is normalized
		return true;
	}
	
	bool checkSphereTriangle(const btVector3& so, const btVector3& sn, float r, const CheckTriangle& tri, CollisionResult& res)
	{
		bool collide = checkSphereTriangle(sn, r, tri, res);
		if (collide) return true;

		auto radiusWithMargin = r + tri.margin;
		auto diff = sn - so;
		if (diff.length2() < radiusWithMargin * radiusWithMargin)
			return false;

		if (tri.prenetration > FLT_EPSILON) // single face
		{
			if (diff.normalized().dot(tri.normal) > 0.f) // ignore by direction
				return false;
		}
		btVector3 realC = so;
		collide = checkSphereTriangle(realC, r, tri, res);
		if (!collide)
		{
			if (!linePlaneIntersection(realC, so, sn, tri.normal, tri.p0, radiusWithMargin))
				return false;
			collide = checkSphereTriangle(realC, r, tri, res);
		}

		if (collide)
			res.depth += (sn - realC).dot(res.normOnB);
		return collide && res.depth < FLT_EPSILON;
	}

	bool checkTriangleSphere(const btVector3& so, const btVector3& sn, float r, const CheckTriangle& tri, CollisionResult& res)
	{
		auto ret = checkSphereTriangle(so, sn, r, tri, res);
		if (ret)
		{
			res.normOnB = -res.normOnB;
			std::swap(res.posA, res.posB);
		}
		return ret;
	}
}
