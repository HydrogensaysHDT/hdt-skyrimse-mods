#include "hdtSimulationIslandManager.h"

namespace hdt
{
	void SimulationIslandManager::findUnions(btDispatcher* dispatcher, btCollisionWorld* colWorld)
	{

		const int numOverlappingPairs = dispatcher->getNumManifolds();
		if (numOverlappingPairs)
		{
			for (int i = 0; i<numOverlappingPairs; i++)
			{
				auto p = dispatcher->getInternalManifoldPointer()[i];
				auto colObj0 = p->getBody0();
				auto colObj1 = p->getBody1();

				if ((colObj0 && (colObj0->mergesSimulationIslands())) &&
					(colObj1 && (colObj1->mergesSimulationIslands())))
				{

					m_unionFind.unite(colObj0->getIslandTag(), colObj1->getIslandTag());
				}
			}
		}
	}

	void SimulationIslandManager::updateActivationState(btCollisionWorld* colWorld, btDispatcher* dispatcher)
	{
		// put the index into m_controllers into m_tag   
		int index = 0;
		{

			int i;
			for (i = 0; i<colWorld->getCollisionObjectArray().size(); i++)
			{
				btCollisionObject*   collisionObject = colWorld->getCollisionObjectArray()[i];
				//Adding filtering here
				if (!collisionObject->isStaticOrKinematicObject())
				{
					collisionObject->setIslandTag(index++);
				}
				collisionObject->setCompanionId(-1);
				collisionObject->setHitFraction(btScalar(1.));
			}
		}
		// do the union find

		initUnionFind(index);

		findUnions(dispatcher, colWorld);
	}
}