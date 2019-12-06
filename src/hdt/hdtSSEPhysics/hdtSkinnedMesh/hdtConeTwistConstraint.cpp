#include "hdtConeTwistConstraint.h"

namespace hdt
{

	ConeTwistConstraint::ConeTwistConstraint(SkinnedMeshBone* a, SkinnedMeshBone* b, const btTransform& frameInA, const btTransform& frameInB)
		: BoneScaleConstraint(a, b, static_cast<btConeTwistConstraint*>(this))
		, btConeTwistConstraint(a->m_rig, b->m_rig, btTransform::getIdentity(), btTransform::getIdentity())
	{
		auto fa = a->m_rigToLocal * frameInA;
		auto fb = b->m_rigToLocal * frameInB;
		setFrames(fa, fb);
		
		enableMotor(false); // motor temporary not support
	}

	void ConeTwistConstraint::scaleConstraint()
	{
		float w0 = m_boneA->m_rig.getInvMass();
		float w1 = m_boneB->m_rig.getInvMass();
		w0 /= (w0 + w1);
		w1 /= (w0 + w1);

		auto newScaleA = m_boneA->m_currentTransform.getScale();
		auto newScaleB = m_boneB->m_currentTransform.getScale();
		auto factorA = newScaleA / m_scaleA;
		auto factorB = newScaleB / m_scaleB;
		auto factorA2 = factorA * factorA;
		auto factorB2 = factorB * factorB;
		auto factor = factorA * w0 + factorB * w1;
		auto factor2 = factor * factor;
		auto factor3 = factor2 * factor;

		if (btFuzzyZero(newScaleA - m_scaleA) && btFuzzyZero(newScaleB - m_scaleB))
			return;

		auto frameA = getFrameOffsetA();
		auto frameB = getFrameOffsetB();
		frameA.setOrigin(frameA.getOrigin() * factorA);
		frameB.setOrigin(frameB.getOrigin() * factorB);
		setFrames(frameA, frameB);
		m_scaleA = newScaleA;
		m_scaleB = newScaleB;
	}
}