#pragma once
#include "hdtSkinnedMesh/hdtBulletHelper.h"
#include "../hdtSSEUtils/NetImmerseUtils.h"

namespace hdt
{
	btQuaternion convertNi(const NiMatrix33& rhs);

	inline btVector3 convertNi(const NiPoint3& rhs)
	{
		return btVector3(rhs.x, rhs.y, rhs.z);
	}

	inline btQsTransform convertNi(const NiTransform& rhs)
	{
		btQsTransform ret;
		ret.setBasis(convertNi(rhs.rot));
		ret.setOrigin(convertNi(rhs.pos));
		ret.setScale(rhs.scale);
		return ret;
	}

	NiPoint3 convertBt(const btVector3& rhs);
	NiMatrix33 convertBt(const btMatrix3x3& rhs);
	NiMatrix33 convertBt(const btQuaternion& rhs);
	NiTransform convertBt(const btQsTransform& rhs);

	static const float scaleRealWorld = 0.01425;
	static const float scaleSkyrim = 1 / scaleRealWorld;
}
