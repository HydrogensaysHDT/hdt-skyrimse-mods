#include "hdtConvertNi.h"

namespace hdt
{
	btQuaternion convertNi(const NiMatrix33& rhs)
	{
		//using namespace DirectX;
		//auto mat = XMLoadFloat3x3((const XMFLOAT3X3*)&rhs);
		//auto q = XMQuaternionRotationMatrix(XMMatrixTranspose(mat));
		//if (XMQuaternionIsInfinite(q) || XMQuaternionIsNaN(q))
		//	return XMQuaternionIdentity();
		//return q;
		btMatrix3x3 mat;
		mat[0][0] = rhs.data[0][0];
		mat[0][1] = rhs.data[0][1];
		mat[0][2] = rhs.data[0][2];
		mat[1][0] = rhs.data[1][0];
		mat[1][1] = rhs.data[1][1];
		mat[1][2] = rhs.data[1][2];
		mat[2][0] = rhs.data[2][0];
		mat[2][1] = rhs.data[2][1];
		mat[2][2] = rhs.data[2][2];
		btQuaternion q;
		mat.getRotation(q);
		return q;
	}

	NiTransform convertBt(const btQsTransform& rhs)
	{
		NiTransform ret;

		ret.rot = convertBt(btMatrix3x3(rhs.getBasis()));
		ret.pos = convertBt(rhs.getOrigin());
		ret.scale = rhs.getScale();

		return ret;
	}

	NiMatrix33 convertBt(const btQuaternion& rhs)
	{
		btMatrix3x3 mat(rhs.normalized());
		return convertBt(mat);
	}

	NiPoint3 convertBt(const btVector3 & rhs)
	{
		NiPoint3 ret;
		ret.x = rhs[0];
		ret.y = rhs[1];
		ret.z = rhs[2];
		return ret;
	}

	NiMatrix33 convertBt(const btMatrix3x3& rhs)
	{
		NiMatrix33 ret;
		ret.data[0][0] = rhs[0][0];
		ret.data[0][1] = rhs[0][1];
		ret.data[0][2] = rhs[0][2];
		ret.data[1][0] = rhs[1][0];
		ret.data[1][1] = rhs[1][1];
		ret.data[1][2] = rhs[1][2];
		ret.data[2][0] = rhs[2][0];
		ret.data[2][1] = rhs[2][1];
		ret.data[2][2] = rhs[2][2];
		return ret;
	}
}
