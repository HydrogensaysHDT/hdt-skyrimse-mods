#pragma once

#include "hdtDispatcher.h"
#include <BulletCollision\CollisionDispatch\btSimulationIslandManager.h>

namespace hdt
{
	class SimulationIslandManager : public btSimulationIslandManager
	{
	public:
		virtual void updateActivationState(btCollisionWorld* colWorld, btDispatcher* dispatcher);

		void findUnions(btDispatcher* dispatcher, btCollisionWorld* colWorld);
		//void buildAndProcessIslands(btDispatcher* dispatcher, btCollisionWorld* collisionWorld, IslandCallback* callback);
	};
}