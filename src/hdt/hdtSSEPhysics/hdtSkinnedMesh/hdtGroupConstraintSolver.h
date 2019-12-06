#pragma once

#include <BulletDynamics\ConstraintSolver\btNNCGConstraintSolver.h>
#include <BulletDynamics\MLCPSolvers\btMLCPSolver.h>
#include "hdtSkinnedMeshSystem.h"

#include <mutex>

namespace hdt
{
	struct SolverBodyMt
	{
	public:
		SolverBodyMt();
		~SolverBodyMt();

		btSolverBody* m_body;

		inline void lock() { m_lock.lock(); }
		inline void unlock() { m_lock.unlock(); }

		SolverBodyMt(const SolverBodyMt& rhs) : m_body(rhs.m_body) {}
		SolverBodyMt& operator=(const SolverBodyMt& rhs) { m_body = rhs.m_body; }

	private:
		SpinLock m_lock;
	};

	class SolverTask
	{
	public:
		SolverTask(SolverBodyMt* A, SolverBodyMt* B);
		virtual ~SolverTask() {}
		virtual void solve() = 0;

	protected:
		SolverBodyMt * m_bodyA;
		SolverBodyMt * m_bodyB;
		SolverBodyMt * m_lockOrderA;
		SolverBodyMt * m_lockOrderB;
	};
	typedef std::shared_ptr<SolverTask> SolverTaskPtr;

	class NonContactSolverTask : public SolverTask
	{
	public:

		NonContactSolverTask(SolverBodyMt* A, SolverBodyMt* B, btSolverConstraint** begin, btSolverConstraint** end, btSingleConstraintRowSolver s);
		virtual void solve() override;

	protected:
		btSolverConstraint ** m_begin;
		btSolverConstraint ** m_end;
		btSingleConstraintRowSolver m_solver;
	};

	class ObsoleteSolverTask : public SolverTask
	{
	public:

		ObsoleteSolverTask(SolverBodyMt* A, SolverBodyMt* B, btTypedConstraint* c, float t);
		virtual void solve() override;

	protected:
		float				m_timeStep;
		btTypedConstraint * m_constraint;
	};

	class ContactSolverTask : public SolverTask
	{
	public:
		ContactSolverTask(SolverBodyMt* A, SolverBodyMt* B, btSolverConstraint* c, btSolverConstraint* f0, btSolverConstraint* f1, btSingleConstraintRowSolver sl, btSingleConstraintRowSolver s);
		virtual void solve() override;
	protected:
		btSolverConstraint * m_contact;
		btSolverConstraint * m_friction0;
		btSolverConstraint * m_friction1;

		btSingleConstraintRowSolver m_solver;
		btSingleConstraintRowSolver m_solverLowerLimit;
	};

	class GroupConstraintSolver : public btSequentialImpulseConstraintSolver
	{
		typedef btSequentialImpulseConstraintSolver Base;
	public:
		GroupConstraintSolver::GroupConstraintSolver();

		virtual btScalar solveSingleIteration(int iteration, btCollisionObject** bodies, int numBodies, btPersistentManifold** manifoldPtr, int numManifolds, btTypedConstraint** constraints, int numConstraints, const btContactSolverInfo& infoGlobal, btIDebugDraw* debugDrawer);
		virtual btScalar solveGroupCacheFriendlySetup(btCollisionObject** bodies, int numBodies, btPersistentManifold** manifoldPtr, int numManifolds, btTypedConstraint** constraints, int numConstraints, const btContactSolverInfo& infoGlobal, btIDebugDraw* debugDrawer);
		virtual btScalar solveGroupCacheFriendlyFinish(btCollisionObject** bodies, int numBodies, const btContactSolverInfo& infoGlobal) override;

		static btSingleConstraintRowSolver getResolveSingleConstraintRowGenericAVX();
		static btSingleConstraintRowSolver getResolveSingleConstraintRowLowerLimitAVX();

		std::vector<ConstraintGroup*>		m_groups;
		std::vector<SolverBodyMt>			m_bodiesMt;
		std::vector<btSolverConstraint*>	m_nonContactConstraintRowPtrs;
		std::vector<SolverTaskPtr>			m_tasks;
		std::vector<SolverTaskPtr>			m_contactTasks;
		std::vector<SolverTaskPtr>			m_nonContactTasks;
	};
}
