#include "hdtSkinnedMeshSystem.h"

#include "hdtSkinnedMeshBody.h"
#include "hdtSkinnedMeshShape.h"
#include "hdtBoneScaleConstraint.h"

namespace hdt
{
	void SkinnedMeshSystem::readTransform(float timeStep)
	{
		for (int i = 0; i < m_bones.size(); ++i)
			m_bones[i]->readTransform(timeStep);

		for (auto i : m_constraints)
			i->scaleConstraint();

		for (auto i : m_constraintGroups)
			i->scaleConstraint();
	}

	void SkinnedMeshSystem::writeTransform()
	{
		for (int i = 0; i < m_bones.size(); ++i)
		{
			if (m_bones[i]->m_rig.isKinematicObject()) continue;

			m_bones[i]->writeTransform();
		}
	}

	void SkinnedMeshSystem::internalUpdate()
	{
		for (auto& i : m_bones)
			i->internalUpdate();

		for (auto& i : m_meshes)
			i->updateBoundingSphereAabb();
	}
	
	//void SkinnedMeshSystem::internalUpdateCL()
	//{
	//	for (auto& i : m_bones)
	//		i->internalUpdate();

	//	//i->internalUpdate();
	//}

	void SkinnedMeshSystem::gather(std::vector<SkinnedMeshBody*>& bodies, std::vector<SkinnedMeshShape*>& shapes)
	{
		for (auto& i : m_meshes)
		{
			bodies.push_back(i);
			shapes.push_back(i->m_shape);
			auto triShape = dynamic_cast<PerTriangleShape*>(i->m_shape());
			if (triShape)
				shapes.push_back(triShape->m_verticesCollision);
		}
	}
}