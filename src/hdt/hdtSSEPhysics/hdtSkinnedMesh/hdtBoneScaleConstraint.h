#pragma once

#include "hdtSkinnedMeshBody.h"

namespace hdt
{
	class BoneScaleConstraint : public RefObject
	{
	public:
		BoneScaleConstraint(SkinnedMeshBone* a, SkinnedMeshBone* b, btTypedConstraint* constraint);
		~BoneScaleConstraint();

		virtual void scaleConstraint() = 0;

		inline btTypedConstraint* getConstraint() const { return m_constraint; }

		float m_scaleA, m_scaleB;

		SkinnedMeshBone* m_boneA;
		SkinnedMeshBone* m_boneB;
		btTypedConstraint* m_constraint;
	};
}
