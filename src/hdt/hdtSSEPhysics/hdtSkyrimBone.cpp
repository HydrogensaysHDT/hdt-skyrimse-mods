#include "hdtSkyrimBone.h"

namespace hdt
{
	SkyrimBone::SkyrimBone(IDStr name, NiNode* node, btRigidBody::btRigidBodyConstructionInfo& ci)
		: m_node(node), SkinnedMeshBone(name, ci)
	{
		if (ci.m_mass)
			m_rig.setCollisionFlags(0);
		else m_rig.setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT);

		m_depth = 0;
		for (auto i = node; i; i = i->m_parent)
			++m_depth;
	}

	void SkyrimBone::readTransform(float timeStep)
	{
		auto oldScale = m_currentTransform.getScale();
		m_currentTransform = convertNi(m_node->m_worldTransform);
		auto newScale = m_currentTransform.getScale();

		auto current = m_rig.getWorldTransform();
		auto dest = m_currentTransform.asTransform() * m_localToRig;

		auto factor = oldScale / newScale;
		if (!m_rig.isStaticOrKinematicObject() && !btFuzzyZero(factor - 1))
		{
			auto factor2 = factor * factor;
			auto factor3 = factor2 * factor;
			auto factor5 = factor3 * factor2;
			auto inertia = m_rig.getInvInertiaDiagLocal();
			m_rig.setMassProps(1.0f / (m_rig.getInvMass() * factor3), btVector3(1, 1, 1));
			m_rig.setInvInertiaDiagLocal(inertia * factor5);
			m_rig.updateInertiaTensor();
		}

		factor = newScale / oldScale;
		if (!btFuzzyZero(factor - 1))
		{
			m_localToRig.getOrigin() *= factor;
			m_rigToLocal.getOrigin() *= factor;
		}
		m_rig.getCollisionShape()->setLocalScaling(setAll(newScale));

		if (timeStep <= 1e-4f)
		{
			m_rig.setWorldTransform(dest);
			m_rig.setInterpolationWorldTransform(dest);
			m_rig.setLinearVelocity(btVector3(0, 0, 0));
			m_rig.setAngularVelocity(btVector3(0, 0, 0));
			m_rig.setInterpolationLinearVelocity(btVector3(0, 0, 0));
			m_rig.setInterpolationAngularVelocity(btVector3(0, 0, 0));
			m_rig.updateInertiaTensor();

			//auto det = dest.getBasis().determinant();
			//if (det < FLT_EPSILON || isnan(det) || isinf(det))
			//	LogWarning("Invalid rotation matrix!!");

			//det = m_rig.getInvInertiaTensorWorld().determinant();
			//if (isnan(det) || isinf(det))
			//	LogWarning("Invalid inertia tensor matrix!!");
		}
		else if (m_rig.isStaticOrKinematicObject())
		{
			btVector3 linVel, angVel;
			btTransformUtil::calculateVelocity(current, dest, timeStep, linVel, angVel);
			m_rig.setLinearVelocity(linVel);
			m_rig.setAngularVelocity(angVel);
			m_rig.setInterpolationLinearVelocity(linVel);
			m_rig.setInterpolationAngularVelocity(angVel);
		}
		//else
		//{
		//	auto det = m_rig.getWorldTransform().getBasis().determinant();
		//	if (isnan(det))
		//	{
		//		LogWarning("Invalid world transform");
		//		m_rig.setWorldTransform(dest);
		//		m_rig.setInterpolationWorldTransform(dest);
		//		m_rig.setLinearVelocity(btVector3(0, 0, 0));
		//		m_rig.setAngularVelocity(btVector3(0, 0, 0));
		//		m_rig.setInterpolationLinearVelocity(btVector3(0, 0, 0));
		//		m_rig.setInterpolationAngularVelocity(btVector3(0, 0, 0));
		//		m_rig.updateInertiaTensor();
		//	}
		//}
	}

	void SkyrimBone::writeTransform()
	{
		//if (m_rig.isStaticOrKinematicObject()) return;
		auto transform = m_rig.getWorldTransform() * m_rigToLocal;

		m_currentTransform.setBasis(transform.getBasis());
		m_currentTransform.setOrigin(transform.getOrigin());

		m_node->m_worldTransform.rot = convertBt(transform.getBasis());
		m_node->m_worldTransform.pos = convertBt(transform.getOrigin());
		m_node->m_worldTransform = m_node->m_worldTransform;

		//auto parentTransform = m_node->m_parent ? m_node->m_parent->unkTransform : NiTransform();
		//NiTransform invParentTransform;
		//parentTransform.Invert(invParentTransform);
		//m_node->m_localTransform = invParentTransform * m_node->unkTransform;

		//updateTransformUpDown(m_node->GetAsNiNode());
	}
}
