#include "hdtSkyrimShape.h"
#include "hdtSkyrimMesh.h"

namespace hdt
{
	SkyrimShape::SkyrimShape()
	{
		m_mesh = 0;
		m_shared = SHARED_PUBLIC;
	}


	SkyrimShape::~SkyrimShape()
	{
	}

	bool SkyrimShape::canCollideWith(const SkinnedMeshBody* rhs) const
	{
		auto body = (SkyrimShape*)rhs;
		if (m_disabled || body->m_disabled)
			return false;

		switch (m_shared)
		{
		case SHARED_PUBLIC:
			break;
		case SHARED_INTERNAL:
			if (m_mesh->m_skeleton != body->m_mesh->m_skeleton)
				return false;
			break;
		case SHARED_PRIVATE:
			if (m_mesh != body->m_mesh)
				return false;
			break;
		default:
			return false;
		}

		return SkinnedMeshBody::canCollideWith(rhs);
	}

	void SkyrimShape::internalUpdate()
	{
		if (m_disabled) return;
		SkinnedMeshBody::internalUpdate();
	}

}
