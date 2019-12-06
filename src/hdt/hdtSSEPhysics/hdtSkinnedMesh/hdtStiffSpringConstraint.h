#pragma once

#include "hdtBulletHelper.h"
#include "hdtBoneScaleConstraint.h"

namespace hdt
{
	class StiffSpringConstraint
		: public BoneScaleConstraint
		, public btTypedConstraint
	{
	public:
		StiffSpringConstraint(SkinnedMeshBone* a, SkinnedMeshBone* b);

		virtual void scaleConstraint();

		float m_minDistance;
		float m_maxDistance;
		float m_stiffness;
		float m_damping;
		float m_equilibriumPoint;

	protected:

		float m_oldDiff;

		///internal method used by the constraint solver, don't use them directly
		virtual void getInfo1(btConstraintInfo1* info);
		///internal method used by the constraint solver, don't use them directly
		virtual void getInfo2(btConstraintInfo2* info);

		///override the default global value of a parameter (such as ERP or CFM), optionally provide the axis (0..5). 
		///If no axis is provided, it uses the default axis for this constraint.
		virtual	void	setParam(int num, btScalar value, int axis = -1){};

		///return the local value of parameter
		virtual	btScalar getParam(int num, int axis = -1) const{ return 0; };
	};

}
