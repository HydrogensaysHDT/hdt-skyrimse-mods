#pragma once

#include "hdtBulletHelper.h"
#include "hdtBoneScaleConstraint.h"

namespace hdt
{
	class Generic6DofConstraint
		: public BoneScaleConstraint
		, public btGeneric6DofSpring2Constraint
	{
	public:

		virtual void scaleConstraint();

		Generic6DofConstraint(SkinnedMeshBone* a, SkinnedMeshBone* b, const btTransform& frameInA, const btTransform& frameInB);

		//void internalUpdateSprings(btConstraintInfo2* info);
		//virtual void getInfo2(btConstraintInfo2* info);

		//// rewrite this since we need to apply bouncing
		//int setLinearLimits(btConstraintInfo2* info, int row, const btTransform& transA, const btTransform& transB, const btVector3& linVelA, const btVector3& linVelB, const btVector3& angVelA, const btVector3& angVelB);

		//btVector3 m_oldLinearDiff;
		//btVector3 m_oldAngularDiff;
		//btVector3 m_linearBounce;
	};

}
