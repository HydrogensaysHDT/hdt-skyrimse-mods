#include "hdtBoneScaleConstraint.h"

namespace hdt
{
	BoneScaleConstraint::BoneScaleConstraint(SkinnedMeshBone* a, SkinnedMeshBone* b, btTypedConstraint* constraint)
	{
		m_boneA = a;
		m_boneB = b;
		m_scaleA = m_scaleB = 1;
		m_constraint = constraint;
	}

	BoneScaleConstraint::~BoneScaleConstraint()
	{
	}
	/*
	void BoneScaleConstraint::scaleConstraint()
	{
		float w0 = m_boneA->m_rig.getInvMass();
		float w1 = m_boneB->m_rig.getInvMass();
		w0 /= (w0 + w1);
		w1 /= (w0 + w1);

		auto newScaleA = m_boneA->m_currentTransform.m_scale;
		auto newScaleB = m_boneB->m_currentTransform.m_scale;
		auto factorA = newScaleA / m_scaleA;
		auto factorB = newScaleB / m_scaleB;
		auto factorA2 = factorA * factorA;
		auto factorB2 = factorB * factorB;
		auto factor = factorA * w0 + factorB * w1;
		auto factor2 = factor * factor;

		if ((newScaleA - m_scaleB).fuzzyZero() && (newScaleB - m_scaleB).fuzzyZero())
			return;

		switch (m_constraint->getConstraintType())
		{
		case POINT2POINT_CONSTRAINT_TYPE:
		{
			auto ptr = (btPoint2PointConstraint*)m_constraint;
			ptr->setPivotA(ptr->getPivotInA() * factorA);
			ptr->setPivotA(ptr->getPivotInA() * factorB);
			break;
		}
		case HINGE_CONSTRAINT_TYPE:
		{
			auto ptr = (btHingeConstraint*)m_constraint;
			ptr->getFrameOffsetA().setOrigin(ptr->getFrameOffsetA().getOrigin() * factorA);
			ptr->getFrameOffsetB().setOrigin(ptr->getFrameOffsetB().getOrigin() * factorB);
			ptr->setMaxMotorImpulse(ptr->getMaxMotorImpulse() * factorA.x());
			break;
		}
		case CONETWIST_CONSTRAINT_TYPE:
		{
			auto ptr = (btConeTwistConstraint*)m_constraint;
			btTransform tA = ptr->getAFrame(), tB = ptr->getBFrame();
			tA.setOrigin(tA.getOrigin() * factorA);
			tB.setOrigin(tB.getOrigin() * factorB);
			ptr->setFrames(tA, tB);
			//ptr->setMaxMotorImpulse(ptr->getMaxMotorImpulse() * factorA.x());
			break;
		}
		case D6_CONSTRAINT_TYPE:
		{
			auto ptr = (btGeneric6DofConstraint*)m_constraint;
			ptr->getFrameOffsetA().setOrigin(ptr->getFrameOffsetA().getOrigin() * factorA);
			ptr->getFrameOffsetB().setOrigin(ptr->getFrameOffsetB().getOrigin() * factorB);
			break;
		}
		case SLIDER_CONSTRAINT_TYPE:
		{
			auto ptr = (btSliderConstraint*)m_constraint;
			ptr->getFrameOffsetA().setOrigin(ptr->getFrameOffsetA().getOrigin() * factorA);
			ptr->getFrameOffsetB().setOrigin(ptr->getFrameOffsetB().getOrigin() * factorB);
			ptr->setTargetLinMotorVelocity(ptr->getTargetLinMotorVelocity() * (ptr->getUseLinearReferenceFrameA() ? factorA.x() : factorB.x()));
			break;
		}
		case CONTACT_CONSTRAINT_TYPE:
			break;
		case D6_SPRING_CONSTRAINT_TYPE:
		{
			// use overflow to access protected value
			struct Data : public btGeneric6DofConstraint
			{
				bool		m_springEnabled[6];
				btScalar	m_equilibriumPoint[6];
				btScalar	m_springStiffness[6];
				btScalar	m_springDamping[6];
			};
			auto ptr = (btGeneric6DofSpringConstraint*)m_constraint;
			auto data = (Data*)ptr;
			ptr->getFrameOffsetA().setOrigin(ptr->getFrameOffsetA().getOrigin() * factorA);
			ptr->getFrameOffsetB().setOrigin(ptr->getFrameOffsetB().getOrigin() * factorB);
			for (int i = 0; i < 3; ++i)
			{
				ptr->setEquilibriumPoint(i, data->m_equilibriumPoint[i] * factor.m_floats[i]);
				// target v = x * sqrt(k/m)
			}
			break;
		}
		case GEAR_CONSTRAINT_TYPE:
			break;
		case FIXED_CONSTRAINT_TYPE:
		{
			// use overflow to access protected value
			struct Data : public btTypedConstraint
			{
				btTransform m_frameInA;
				btTransform m_frameInB;
			};
			auto ptr = (btFixedConstraint*)m_constraint;
			auto data = (Data*)ptr;
			data->m_frameInA.setOrigin(data->m_frameInA.getOrigin() * factorA);
			data->m_frameInB.setOrigin(data->m_frameInB.getOrigin() * factorB);
		}
		case D6_SPRING_2_CONSTRAINT_TYPE:
		{
			auto ptr = (btGeneric6DofSpring2Constraint*)m_constraint;
			ptr->getFrameOffsetA().setOrigin(ptr->getFrameOffsetA().getOrigin() * factorA);
			ptr->getFrameOffsetB().setOrigin(ptr->getFrameOffsetB().getOrigin() * factorB);
			break;
		}
		}

		m_scaleA = newScaleA;
		m_scaleB = newScaleB;
	}*/
}