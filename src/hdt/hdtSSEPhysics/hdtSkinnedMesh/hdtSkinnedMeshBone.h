#pragma once

#include "hdtBulletHelper.h"
#include"hdtAABB.h"
#include <memory>

namespace hdt
{
	class SkinnedMeshBody;
	_CRT_ALIGN(16) struct SkinnedMeshBone : public RefObject
	{
		BT_DECLARE_ALIGNED_ALLOCATOR();

		SkinnedMeshBone(const IDStr& name, btRigidBody::btRigidBodyConstructionInfo& ci);
		~SkinnedMeshBone();

		IDStr m_name;
		float m_marginMultipler;
		float m_boudingSphereMultipler = 1.0f;
		float m_gravityFactor = 1.0f;

		btRigidBody m_rig;
		btTransform m_localToRig;
		btTransform m_rigToLocal;
		btQsTransform m_currentTransform;

		std::vector<IDStr>	m_canCollideWithBone;
		std::vector<IDStr>	m_noCollideWithBone;

		virtual void readTransform(float timeStep) = 0;
		virtual void writeTransform() = 0;

		void internalUpdate();

		bool canCollideWith(SkinnedMeshBone* rhs);
	};
}