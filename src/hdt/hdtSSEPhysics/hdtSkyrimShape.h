#pragma once

#include "hdtConvertNi.h"
#include "hdtSkyrimBone.h"
#include "hdtSkinnedMesh\hdtSkinnedMeshBody.h"

namespace hdt
{
	class SkyrimMesh;
	class SkyrimShape : public SkinnedMeshBody
	{
	public:
		SkyrimShape();
		~SkyrimShape();

		enum SharedType
		{
			SHARED_PUBLIC,
			SHARED_INTERNAL,
			SHARED_PRIVATE,
		};

		SkyrimMesh*	m_mesh;
		SharedType		m_shared;
		bool			m_disabled = false;
		int				m_disablePriority = 0;
		IDStr			m_disableTag;

		virtual bool canCollideWith(const SkinnedMeshBody* body) const override;
		virtual void internalUpdate() override;
	};
}
