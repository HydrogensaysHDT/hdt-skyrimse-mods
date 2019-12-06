#pragma once

#include "hdtConvertNi.h"
#include "hdtSkinnedMesh\hdtSkinnedMeshBone.h"

namespace hdt
{
	class SkyrimBone : public hdt::SkinnedMeshBone
	{
	public:

		SkyrimBone(IDStr name, NiNode* node, btRigidBody::btRigidBodyConstructionInfo& ci);

		virtual void readTransform(float timeStep);
		virtual void writeTransform();

		int			m_depth;
		NiNode* m_node;
	};
}
