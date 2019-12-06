#pragma once

#include "hdtBulletHelper.h"
#include <ppl.h>
#include <ppltasks.h>
#include <vector>

namespace hdt
{
	class SkinnedMeshBody;
	class CollisionDispatcher : public btCollisionDispatcher
	{
	public:

		CollisionDispatcher(btCollisionConfiguration* collisionConfiguration) :btCollisionDispatcher(collisionConfiguration){}

		virtual btPersistentManifold*	getNewManifold(const btCollisionObject* b0, const btCollisionObject* b1)
		{
			m_lock.lock();
			auto ret = btCollisionDispatcher::getNewManifold(b0, b1);
			m_lock.unlock();
			return ret;
		}

		virtual void releaseManifold(btPersistentManifold* manifold)
		{
			m_lock.lock();
			btCollisionDispatcher::releaseManifold(manifold);
			m_lock.unlock();
		}

		virtual bool needsCollision(const btCollisionObject* body0, const btCollisionObject* body1);
		virtual void dispatchAllCollisionPairs(btOverlappingPairCache* pairCache, const btDispatcherInfo& dispatchInfo, btDispatcher* dispatcher);

		virtual int getNumManifolds() const;
		virtual	btPersistentManifold**	getInternalManifoldPointer();
		virtual btPersistentManifold* getManifoldByIndexInternal(int index);

		void clearAllManifold();

		std::mutex m_lock;
		std::vector<std::pair<SkinnedMeshBody*, SkinnedMeshBody*>> m_pairs;
	};
}