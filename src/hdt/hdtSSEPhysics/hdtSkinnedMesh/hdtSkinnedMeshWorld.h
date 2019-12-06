#pragma once

#include "hdtGroupConstraintSolver.h"

namespace hdt
{
	class SkinnedMeshWorld : public btDiscreteDynamicsWorld
	{
	public:

		SkinnedMeshWorld();
		~SkinnedMeshWorld();
		
		virtual void addSkinnedMeshSystem(SkinnedMeshSystem* system);
		virtual void removeSkinnedMeshSystem(SkinnedMeshSystem* system);
		
		virtual int stepSimulation(btScalar timeStep, int maxSubSteps = 1, btScalar fixedTimeStep = btScalar(1.) / btScalar(60.));

		btVector3& getWind(){ return m_windSpeed; }
		const btVector3& getWind() const { return m_windSpeed; }
		
	protected:

		inline void readTransform(float timeStep){ for (int i = 0; i < m_systems.size(); ++i) m_systems[i]->readTransform(timeStep); }
		inline void writeTransform(){ for (int i = 0; i < m_systems.size(); ++i) m_systems[i]->writeTransform(); }
		
		virtual void applyGravity() override;

		virtual void predictUnconstraintMotion(btScalar timeStep);
		virtual void integrateTransforms(btScalar timeStep);
		virtual void performDiscreteCollisionDetection();
		virtual void solveConstraints(btContactSolverInfo& solverInfo);

		std::vector<Ref<SkinnedMeshSystem>> m_systems;

		btVector3 m_windSpeed;

		std::vector<SkinnedMeshBody*> _bodies;
		std::vector<SkinnedMeshShape*> _shapes;

		GroupConstraintSolver m_constraintSolver;
	};

}