#include "hdtGeneric6DofConstraint.h"

namespace hdt
{
	Generic6DofConstraint::Generic6DofConstraint(SkinnedMeshBone* a, SkinnedMeshBone* b, const btTransform& frameInA, const btTransform& frameInB)
		: BoneScaleConstraint(a, b, static_cast<btTypedConstraint*>(this))
		, btGeneric6DofSpring2Constraint(a->m_rig, b->m_rig, btTransform::getIdentity(), btTransform::getIdentity(), RO_XYZ)
	{
		auto fa = a->m_rigToLocal * frameInA;
		auto fb = b->m_rigToLocal * frameInB;
		setFrames(fa, fb);

		for (int i = 0; i < 6; ++i)
			enableSpring(i, true);

		//m_linearLimits.m_stopERP.setValue(0.1f, 0.1f, 0.1f);

		//m_oldLinearDiff.setZero();
		//m_oldAngularDiff.setZero();
		//m_linearBounce.setZero();
	}

	void Generic6DofConstraint::scaleConstraint()
	{
		float w0 = m_boneA->m_rig.getInvMass();
		float w1 = m_boneB->m_rig.getInvMass();
		w0 /= (w0 + w1);
		w1 /= (w0 + w1);

		auto newScaleA = m_boneA->m_currentTransform.getScale();
		auto newScaleB = m_boneB->m_currentTransform.getScale();
		auto factorA = newScaleA / m_scaleA;
		auto factorB = newScaleB / m_scaleB;
		auto factor = factorA * w0 + factorB * w1;
		auto factor2 = factor * factor;
		auto factor3 = factor2 * factor;

		if (btFuzzyZero(newScaleA - m_scaleA) && btFuzzyZero(newScaleB - m_scaleB))
			return;

		getFrameOffsetA().setOrigin(getFrameOffsetA().getOrigin() * factorA);
		getFrameOffsetB().setOrigin(getFrameOffsetB().getOrigin() * factorB);
		for (int i = 0; i < 3; ++i)
		{
			auto factor5 = factor3 * factor2;
			m_linearLimits.m_equilibriumPoint[i] *= factor;
			// target k = ma / x(kg/s^2)
			m_linearLimits.m_springStiffness[i] *= factor3;
			m_linearLimits.m_springDamping[i] *= factor3;
			m_angularLimits[i].m_springStiffness *= factor5;
			m_angularLimits[i].m_springDamping *= factor5;
		}

		m_scaleA = newScaleA;
		m_scaleB = newScaleB;
	}
	//
	//void Generic6DofConstraint::internalUpdateSprings(btConstraintInfo2* info)
	//{
	//	// it is assumed that calculateTransforms() have been called before this call
	//	int i;
	//	//btVector3 relVel = m_rbB.getLinearVelocity() - m_rbA.getLinearVelocity();
	//	btScalar velFactor = info->fps / btScalar(info->m_numIterations);
	//	//auto velocity = (m_calculatedLinearDiff - m_oldLinearDiff) * info->fps;
	//	auto va = m_frameInA.getBasis().transpose() * m_boneA->m_rig.getLinearVelocity();
	//	auto vb = m_frameInB.getBasis().transpose() * m_boneB->m_rig.getLinearVelocity();
	//	auto velocity = vb - va;
	//	for (i = 0; i < 3; i++)
	//	{
	//		if (m_springStiffness[i])
	//		{
	//			// get current position of constraint
	//			//auto prevPos = m_oldLinearDiff[i];
	//			auto currPos = m_calculatedLinearDiff[i];
	//			// calculate difference
	//			//auto delta = (prevPos + currPos) * 0.5f - m_equilibriumPoint[i];
	//			auto delta = currPos - m_equilibriumPoint[i];

	//			// spring force is (delta * m_stiffness) according to Hooke's Law
	//			btScalar force = delta * m_springStiffness[i];
	//			btScalar friction = m_springDamping[i] * velocity[i];
	//			force += force * friction < 0 ? btClamped(friction, -abs(force), abs(force)) : friction;
	//			m_linearLimits.m_targetVelocity[i] = velFactor * force;
	//			m_linearLimits.m_maxMotorForce[i] = btFabs(force) / info->fps;
	//		}
	//	}
	//	m_oldLinearDiff = m_calculatedLinearDiff;

	//	auto tA = m_frameInA.getBasis().transpose().scaled(m_rbA.getInvInertiaDiagLocal()) * m_frameInA.getBasis();
	//	auto tB = m_frameInB.getBasis().transpose().scaled(m_rbB.getInvInertiaDiagLocal()) * m_frameInB.getBasis();
	//	va = m_frameInA.getBasis().transpose() * m_boneA->m_rig.getAngularVelocity();
	//	vb = m_frameInB.getBasis().transpose() * m_boneA->m_rig.getAngularVelocity();
	//	//velocity = (m_calculatedAxisAngleDiff - m_oldAngularDiff) * info->fps;
	//	velocity = vb - va;
	//	for (i = 0; i < 3; i++)
	//	{
	//		if (m_springStiffness[i + 3])
	//		{
	//			// get current position of constraint
	//			//auto prevPos = m_oldLinearDiff[i];
	//			auto currPos = m_calculatedLinearDiff[i];
	//			// calculate difference
	//			//auto delta = (prevPos + currPos) * 0.5f - m_equilibriumPoint[i + 3];
	//			auto delta = currPos - m_equilibriumPoint[i + 3];
	//			// spring force is (-delta * m_stiffness) according to Hooke's Law
	//			btScalar force = -delta * m_springStiffness[i + 3];
	//			btScalar friction = -m_springDamping[i + 3] * velocity[i];
	//			force += force * friction < 0 ? btClamped(friction, -abs(force), abs(force)) : friction;
	//			//force *= tA[i][i] + tB[i][i];
	//			m_angularLimits[i].m_targetVelocity = velFactor * force;
	//			m_angularLimits[i].m_maxMotorForce = btFabs(force) / info->fps;
	//		}
	//	}
	//	m_oldAngularDiff = m_calculatedAxisAngleDiff;
	//}

	//void Generic6DofConstraint::getInfo2(btConstraintInfo2* info)
	//{
	//	internalUpdateSprings(info);

	//	btAssert(!m_useSolveConstraintObsolete);

	//	const btTransform& transA = m_rbA.getCenterOfMassTransform();
	//	const btTransform& transB = m_rbB.getCenterOfMassTransform();
	//	const btVector3& linVelA = m_rbA.getLinearVelocity();
	//	const btVector3& linVelB = m_rbB.getLinearVelocity();
	//	const btVector3& angVelA = m_rbA.getAngularVelocity();
	//	const btVector3& angVelB = m_rbB.getAngularVelocity();

	//	if (m_useOffsetForConstraintFrame)
	//	{ // for stability better to solve angular limits first
	//		int row = setAngularLimits(info, 0, transA, transB, linVelA, linVelB, angVelA, angVelB);
	//		setLinearLimits(info, row, transA, transB, linVelA, linVelB, angVelA, angVelB);
	//	}
	//	else
	//	{ // leave old version for compatibility
	//		int row = setLinearLimits(info, 0, transA, transB, linVelA, linVelB, angVelA, angVelB);
	//		setAngularLimits(info, row, transA, transB, linVelA, linVelB, angVelA, angVelB);
	//	}
	//}

	//int Generic6DofConstraint::setLinearLimits(btConstraintInfo2* info, int row, const btTransform& transA, const btTransform& transB, const btVector3& linVelA, const btVector3& linVelB, const btVector3& angVelA, const btVector3& angVelB)
	//{
	//	//	int row = 0;
	//	//solve linear limits
	//	btRotationalLimitMotor limot;
	//	for (int i = 0; i<3; i++)
	//	{
	//		if (m_linearLimits.needApplyForce(i))
	//		{ // re-use rotational motor code
	//			limot.m_bounce = m_linearBounce[i];
	//			limot.m_currentLimit = m_linearLimits.m_currentLimit[i];
	//			limot.m_currentPosition = m_linearLimits.m_currentLinearDiff[i];
	//			limot.m_currentLimitError = m_linearLimits.m_currentLimitError[i];
	//			limot.m_damping = m_linearLimits.m_damping;
	//			limot.m_enableMotor = m_linearLimits.m_enableMotor[i];
	//			limot.m_hiLimit = m_linearLimits.m_upperLimit[i];
	//			limot.m_limitSoftness = m_linearLimits.m_limitSoftness;
	//			limot.m_loLimit = m_linearLimits.m_lowerLimit[i];
	//			limot.m_maxLimitForce = btScalar(0.f);
	//			limot.m_maxMotorForce = m_linearLimits.m_maxMotorForce[i];
	//			limot.m_targetVelocity = m_linearLimits.m_targetVelocity[i];
	//			btVector3 axis = m_calculatedTransformA.getBasis().getColumn(i);
	//			int flags = m_flags >> (i * BT_6DOF_FLAGS_AXIS_SHIFT);
	//			limot.m_normalCFM = (flags & BT_6DOF_FLAGS_CFM_NORM) ? m_linearLimits.m_normalCFM[i] : info->cfm[0];
	//			limot.m_stopCFM = (flags & BT_6DOF_FLAGS_CFM_STOP) ? m_linearLimits.m_stopCFM[i] : info->cfm[0];
	//			limot.m_stopERP = (flags & BT_6DOF_FLAGS_ERP_STOP) ? m_linearLimits.m_stopERP[i] : info->erp;
	//			if (m_useOffsetForConstraintFrame)
	//			{
	//				int indx1 = (i + 1) % 3;
	//				int indx2 = (i + 2) % 3;
	//				int rotAllowed = 1; // rotations around orthos to current axis
	//				if (m_angularLimits[indx1].m_currentLimit && m_angularLimits[indx2].m_currentLimit)
	//				{
	//					rotAllowed = 0;
	//				}
	//				row += get_limit_motor_info2(&limot, transA, transB, linVelA, linVelB, angVelA, angVelB, info, row, axis, 0, rotAllowed);
	//			}
	//			else
	//			{
	//				row += get_limit_motor_info2(&limot, transA, transB, linVelA, linVelB, angVelA, angVelB, info, row, axis, 0);
	//			}
	//		}
	//	}
	//	return row;
	//}

}