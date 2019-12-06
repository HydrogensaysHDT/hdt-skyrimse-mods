#include "hdtGroupConstraintSolver.h"
#include <unordered_map>

#include <LinearMath/btCpuFeatureUtility.h>

#if defined (BT_ALLOW_SSE4)
#include <intrin.h>

#define USE_FMA					1
#define USE_FMA3_INSTEAD_FMA4	1
#define USE_SSE4_DOT			1

#define SSE4_DP(a, b)			_mm_dp_ps(a, b, 0x7f)
#define SSE4_DP_FP(a, b)		_mm_cvtss_f32(_mm_dp_ps(a, b, 0x7f))

#if USE_SSE4_DOT
#define DOT_PRODUCT(a, b)		SSE4_DP(a, b)
#else
#define DOT_PRODUCT(a, b)		btSimdDot3(a, b)
#endif

#if USE_FMA
#if USE_FMA3_INSTEAD_FMA4
// a*b + c
#define FMADD(a, b, c)		_mm_fmadd_ps(a, b, c)
#define FMADD256(a, b, c)		_mm256_fmadd_ps(a, b, c)
// -(a*b) + c
#define FMNADD(a, b, c)		_mm_fnmadd_ps(a, b, c)
#define FMNADD256(a, b, c)		_mm256_fnmadd_ps(a, b, c)
#else // USE_FMA3
// a*b + c
#define FMADD(a, b, c)		_mm_macc_ps(a, b, c)
// -(a*b) + c
#define FMNADD(a, b, c)		_mm_nmacc_ps(a, b, c)
#endif
#else // USE_FMA
// c + a*b
#define FMADD(a, b, c)		_mm_add_ps(c, _mm_mul_ps(a, b))
// c - a*b
#define FMNADD(a, b, c)		_mm_sub_ps(c, _mm_mul_ps(a, b))
#endif
#endif

namespace hdt
{
	inline __m256 pack256(__m128 lo, __m128 hi)
	{
		return _mm256_set_m128(hi, lo);
	}

	inline std::tuple<__m128, __m128> unpack256(__m256 ymm)
	{
		return std::tie(_mm256_castps256_ps128(ymm), _mm256_extractf128_ps(ymm, 1));
	}

	// Enhanced version of gResolveSingleConstraintRowGeneric_sse2 with AVX
	static btSimdScalar gResolveSingleConstraintRowGeneric_avx256(btSolverBody& body1, btSolverBody& body2, const btSolverConstraint& c)
	{
		__m128 tmp = _mm_set_ps1(c.m_jacDiagABInv);
		__m128 deltaImpulse = _mm_set_ps1(c.m_rhs - btScalar(c.m_appliedImpulse) * c.m_cfm);
		const __m128 lowerLimit = _mm_set_ps1(c.m_lowerLimit);
		const __m128 upperLimit = _mm_set_ps1(c.m_upperLimit);

		__m256 invMass = pack256(body1.internalGetInvMass().mVec128, body2.internalGetInvMass().mVec128);
		__m256 deltaLinearVelocity = pack256(body1.internalGetDeltaLinearVelocity().mVec128, body2.internalGetDeltaLinearVelocity().mVec128);
		__m256 deltaAngularVelocity = pack256(body1.internalGetDeltaAngularVelocity().mVec128, body2.internalGetDeltaAngularVelocity().mVec128);

		__m256 contactNormal = pack256(c.m_contactNormal1.mVec128, c.m_contactNormal2.mVec128);
		__m256 relposCrossNormal = pack256(c.m_relpos1CrossNormal.mVec128, c.m_relpos2CrossNormal.mVec128);
		__m256 deltaVelDotn = _mm256_add_ps(_mm256_dp_ps(contactNormal, deltaLinearVelocity, 0x7f), _mm256_dp_ps(relposCrossNormal, deltaAngularVelocity, 0x7f));
		deltaImpulse = FMNADD(_mm256_castps256_ps128(deltaVelDotn), tmp, deltaImpulse);
		deltaImpulse = FMNADD(_mm256_extractf128_ps(deltaVelDotn, 1), tmp, deltaImpulse);

		//const __m128 deltaVel1Dotn = _mm_add_ps(DOT_PRODUCT(c.m_contactNormal1.mVec128, body1.internalGetDeltaLinearVelocity().mVec128), DOT_PRODUCT(c.m_relpos1CrossNormal.mVec128, body1.internalGetDeltaAngularVelocity().mVec128));
		//const __m128 deltaVel2Dotn = _mm_add_ps(DOT_PRODUCT(c.m_contactNormal2.mVec128, body2.internalGetDeltaLinearVelocity().mVec128), DOT_PRODUCT(c.m_relpos2CrossNormal.mVec128, body2.internalGetDeltaAngularVelocity().mVec128));
		//deltaImpulse = FMNADD(deltaVel1Dotn, tmp, deltaImpulse);
		//deltaImpulse = FMNADD(deltaVel2Dotn, tmp, deltaImpulse);
		tmp = _mm_add_ps(c.m_appliedImpulse, deltaImpulse); // sum
		auto appliedImpulse = _mm_max_ps(_mm_min_ps(tmp, upperLimit), lowerLimit);
		deltaImpulse = _mm_sub_ps(appliedImpulse, c.m_appliedImpulse);
		c.m_appliedImpulse = appliedImpulse;
		//const __m128 maskLower = _mm_cmpgt_ps(tmp, lowerLimit);
		//const __m128 maskUpper = _mm_cmpgt_ps(upperLimit, tmp);
		//deltaImpulse = _mm_blendv_ps(_mm_sub_ps(lowerLimit, c.m_appliedImpulse), _mm_blendv_ps(_mm_sub_ps(upperLimit, c.m_appliedImpulse), deltaImpulse, maskUpper), maskLower);
		//c.m_appliedImpulse = _mm_blendv_ps(lowerLimit, _mm_blendv_ps(upperLimit, tmp, maskUpper), maskLower);

		auto deltaImpulse2 = pack256(deltaImpulse, deltaImpulse);
		auto angularComponent = pack256(c.m_angularComponentA.mVec128, c.m_angularComponentB.mVec128);
		deltaLinearVelocity = FMADD256(_mm256_mul_ps(contactNormal, invMass), deltaImpulse2, deltaLinearVelocity);
		deltaAngularVelocity = FMADD256(angularComponent, deltaImpulse2, deltaAngularVelocity);
		std::tie(body1.internalGetDeltaLinearVelocity().mVec128, body2.internalGetDeltaLinearVelocity().mVec128) = unpack256(deltaLinearVelocity);
		std::tie(body1.internalGetDeltaAngularVelocity().mVec128, body2.internalGetDeltaAngularVelocity().mVec128) = unpack256(deltaAngularVelocity);
		
		//body1.internalGetDeltaLinearVelocity().mVec128 = FMADD(_mm_mul_ps(c.m_contactNormal1.mVec128, body1.internalGetInvMass().mVec128), deltaImpulse, body1.internalGetDeltaLinearVelocity().mVec128);
		//body2.internalGetDeltaLinearVelocity().mVec128 = FMADD(_mm_mul_ps(c.m_contactNormal2.mVec128, body2.internalGetInvMass().mVec128), deltaImpulse, body2.internalGetDeltaLinearVelocity().mVec128);
		//body1.internalGetDeltaAngularVelocity().mVec128 = FMADD(c.m_angularComponentA.mVec128, deltaImpulse, body1.internalGetDeltaAngularVelocity().mVec128);
		//body2.internalGetDeltaAngularVelocity().mVec128 = FMADD(c.m_angularComponentB.mVec128, deltaImpulse, body2.internalGetDeltaAngularVelocity().mVec128);
		return deltaImpulse;
	}

	// Enhanced version of gResolveSingleConstraintRowGeneric_sse2 with AVX
	static btSimdScalar gResolveSingleConstraintRowLowerLimit_avx256(btSolverBody& body1, btSolverBody& body2, const btSolverConstraint& c)
	{
		__m128 tmp = _mm_set_ps1(c.m_jacDiagABInv);
		__m128 deltaImpulse = _mm_set_ps1(c.m_rhs - btScalar(c.m_appliedImpulse) * c.m_cfm);
		const __m128 lowerLimit = _mm_set_ps1(c.m_lowerLimit);

		__m256 invMass = pack256(body1.internalGetInvMass().mVec128, body2.internalGetInvMass().mVec128);
		__m256 deltaLinearVelocity = pack256(body1.internalGetDeltaLinearVelocity().mVec128, body2.internalGetDeltaLinearVelocity().mVec128);
		__m256 deltaAngularVelocity = pack256(body1.internalGetDeltaAngularVelocity().mVec128, body2.internalGetDeltaAngularVelocity().mVec128);

		__m256 contactNormal = pack256(c.m_contactNormal1.mVec128, c.m_contactNormal2.mVec128);
		__m256 relposCrossNormal = pack256(c.m_relpos1CrossNormal.mVec128, c.m_relpos2CrossNormal.mVec128);
		__m256 deltaVelDotn = _mm256_add_ps(_mm256_dp_ps(contactNormal, deltaLinearVelocity, 0x7f), _mm256_dp_ps(relposCrossNormal, deltaAngularVelocity, 0x7f));
		deltaImpulse = FMNADD(_mm256_castps256_ps128(deltaVelDotn), tmp, deltaImpulse);
		deltaImpulse = FMNADD(_mm256_extractf128_ps(deltaVelDotn, 1), tmp, deltaImpulse);
		//const __m128 deltaVel1Dotn = _mm_add_ps(DOT_PRODUCT(c.m_contactNormal1.mVec128, body1.internalGetDeltaLinearVelocity().mVec128), DOT_PRODUCT(c.m_relpos1CrossNormal.mVec128, body1.internalGetDeltaAngularVelocity().mVec128));
		//const __m128 deltaVel2Dotn = _mm_add_ps(DOT_PRODUCT(c.m_contactNormal2.mVec128, body2.internalGetDeltaLinearVelocity().mVec128), DOT_PRODUCT(c.m_relpos2CrossNormal.mVec128, body2.internalGetDeltaAngularVelocity().mVec128));
		//deltaImpulse = FMNADD(deltaVel1Dotn, tmp, deltaImpulse);
		//deltaImpulse = FMNADD(deltaVel2Dotn, tmp, deltaImpulse);

		tmp = _mm_add_ps(c.m_appliedImpulse, deltaImpulse);
		auto appliedImpulse = _mm_max_ps(tmp, lowerLimit);
		deltaImpulse = _mm_sub_ps(appliedImpulse, c.m_appliedImpulse);
		c.m_appliedImpulse = appliedImpulse;
		//const __m128 mask = _mm_cmpgt_ps(tmp, lowerLimit);
		//deltaImpulse = _mm_blendv_ps(_mm_sub_ps(lowerLimit, c.m_appliedImpulse), deltaImpulse, mask);
		//c.m_appliedImpulse = _mm_blendv_ps(lowerLimit, tmp, mask);

		auto deltaImpulse2 = pack256(deltaImpulse, deltaImpulse);
		auto angularComponent = pack256(c.m_angularComponentA.mVec128, c.m_angularComponentB.mVec128);
		deltaLinearVelocity = FMADD256(_mm256_mul_ps(contactNormal, invMass), deltaImpulse2, deltaLinearVelocity);
		deltaAngularVelocity = FMADD256(angularComponent, deltaImpulse2, deltaAngularVelocity);
		std::tie(body1.internalGetDeltaLinearVelocity().mVec128, body2.internalGetDeltaLinearVelocity().mVec128) = unpack256(deltaLinearVelocity);
		std::tie(body1.internalGetDeltaAngularVelocity().mVec128, body2.internalGetDeltaAngularVelocity().mVec128) = unpack256(deltaAngularVelocity);
		//body1.internalGetDeltaLinearVelocity().mVec128 = FMADD(_mm_mul_ps(c.m_contactNormal1.mVec128, body1.internalGetInvMass().mVec128), deltaImpulse, body1.internalGetDeltaLinearVelocity().mVec128);
		//body1.internalGetDeltaAngularVelocity().mVec128 = FMADD(c.m_angularComponentA.mVec128, deltaImpulse, body1.internalGetDeltaAngularVelocity().mVec128);
		//body2.internalGetDeltaLinearVelocity().mVec128 = FMADD(_mm_mul_ps(c.m_contactNormal2.mVec128, body2.internalGetInvMass().mVec128), deltaImpulse, body2.internalGetDeltaLinearVelocity().mVec128);
		//body2.internalGetDeltaAngularVelocity().mVec128 = FMADD(c.m_angularComponentB.mVec128, deltaImpulse, body2.internalGetDeltaAngularVelocity().mVec128);
		return deltaImpulse;
	}

	SolverBodyMt::SolverBodyMt()
	{
	}

	SolverBodyMt::~SolverBodyMt()
	{
	}

	SolverTask::SolverTask(SolverBodyMt * A, SolverBodyMt * B)
		: m_bodyA(A), m_bodyB(B)
	{
		if (A > B) std::swap(A, B);
		m_lockOrderA = A;
		m_lockOrderB = B;
	}

	NonContactSolverTask::NonContactSolverTask(SolverBodyMt * A, SolverBodyMt * B, btSolverConstraint ** begin, btSolverConstraint ** end, btSingleConstraintRowSolver s)
		: SolverTask(A, B), m_begin(begin), m_end(end), m_solver(s)
	{
		std::random_shuffle(m_begin, m_end);
	}

	void NonContactSolverTask::solve()
	{
		HDT_LOCK_GUARD(la, *m_lockOrderA);
		HDT_LOCK_GUARD(lb, *m_lockOrderB);

		for (auto i = m_begin; i < m_end; ++i)
			m_solver(*m_bodyA->m_body, *m_bodyB->m_body, **i);
	}

	ContactSolverTask::ContactSolverTask(SolverBodyMt * A, SolverBodyMt * B, btSolverConstraint * c, btSolverConstraint * f0, btSolverConstraint * f1, btSingleConstraintRowSolver sl, btSingleConstraintRowSolver s)
		: SolverTask(A, B), m_contact(c), m_friction0(f0), m_friction1(f1), m_solverLowerLimit(sl), m_solver(s)
	{
	}

	void ContactSolverTask::solve()
	{
		HDT_LOCK_GUARD(la, *m_lockOrderA);
		HDT_LOCK_GUARD(lb, *m_lockOrderB);

		m_solverLowerLimit(*m_bodyA->m_body, *m_bodyB->m_body, *m_contact);
		float totalImpulse = m_contact->m_appliedImpulse;

		if (totalImpulse > 0)
		{
			if (m_friction0)
			{
				m_friction0->m_lowerLimit = -(m_friction0->m_friction * totalImpulse);
				m_friction0->m_upperLimit = m_friction0->m_friction * totalImpulse;
				m_solver(*m_bodyA->m_body, *m_bodyB->m_body, *m_friction0);
			}

			if (m_friction1)
			{
				m_friction1->m_lowerLimit = -(m_friction1->m_friction * totalImpulse);
				m_friction1->m_upperLimit = m_friction1->m_friction * totalImpulse;
				m_solver(*m_bodyA->m_body, *m_bodyB->m_body, *m_friction1);
			}
		}
	}

	ObsoleteSolverTask::ObsoleteSolverTask(SolverBodyMt * A, SolverBodyMt * B, btTypedConstraint * c, float t)
		: SolverTask(A, B), m_timeStep(t), m_constraint(c)
	{
	}

	void ObsoleteSolverTask::solve()
	{
		HDT_LOCK_GUARD(la, *m_lockOrderA);
		HDT_LOCK_GUARD(lb, *m_lockOrderB);
		m_constraint->solveConstraintObsolete(*m_bodyA->m_body, *m_bodyB->m_body, m_timeStep);
	}

	btScalar GroupConstraintSolver::solveGroupCacheFriendlySetup(btCollisionObject** bodies, int numBodies, btPersistentManifold** manifoldPtr, int numManifolds, btTypedConstraint** constraints, int numConstraints, const btContactSolverInfo& infoGlobal, btIDebugDraw* debugDrawer)
	{
		auto ret = Base::solveGroupCacheFriendlySetup(bodies, numBodies, manifoldPtr, numManifolds, constraints, numConstraints, infoGlobal, debugDrawer);

		concurrency::parallel_for_each(m_groups.begin(), m_groups.end(), [&](ConstraintGroup* i)
		{
			i->setup(&m_tmpSolverBodyPool, infoGlobal);
			i->iteration(bodies, numBodies, infoGlobal);
		});

		// init solver body
		for (int j = 0; j < numConstraints; j++)
		{
			if (constraints[j]->isEnabled())
			{
				getOrInitSolverBody(constraints[j]->getRigidBodyA(), infoGlobal.m_timeStep);
				getOrInitSolverBody(constraints[j]->getRigidBodyB(), infoGlobal.m_timeStep);
			}
		}

		// init mt bodies
		m_bodiesMt.resize(m_tmpSolverBodyPool.size());
		for (int i = 0; i < m_bodiesMt.size(); ++i)
			m_bodiesMt[i].m_body = &m_tmpSolverBodyPool[i];

		// add tasks;
		if (m_tmpSolverNonContactConstraintPool.size())
		{
			{
				auto begin = &m_tmpSolverNonContactConstraintPool[0];
				auto end = begin + m_tmpSolverNonContactConstraintPool.size();
				m_nonContactConstraintRowPtrs.reserve(m_tmpSolverNonContactConstraintPool.size());
				for (auto i = begin; i < end; ++i)
					m_nonContactConstraintRowPtrs.push_back(i);

				std::sort(m_nonContactConstraintRowPtrs.begin(), m_nonContactConstraintRowPtrs.end(), [](btSolverConstraint* a, btSolverConstraint* b) {
					return (((uint64_t)a->m_solverBodyIdA << 32) | a->m_solverBodyIdB) < (((uint64_t)b->m_solverBodyIdA << 32) | b->m_solverBodyIdB);
				});
			}

			SolverBodyMt* lastA = nullptr;
			SolverBodyMt* lastB = nullptr;
			btSolverConstraint** lastBegin = m_nonContactConstraintRowPtrs.data();
			for (int i = 0; i < m_nonContactConstraintRowPtrs.size(); ++i)
			{
				auto curr = &m_nonContactConstraintRowPtrs[i];
				auto c = m_nonContactConstraintRowPtrs[i];
				auto a = &m_bodiesMt[c->m_solverBodyIdA];
				auto b = &m_bodiesMt[c->m_solverBodyIdB];
				if (lastA != a || lastB != b)
				{
					if (lastA && lastB)
					{
						auto task = SolverTaskPtr(new NonContactSolverTask(lastA, lastB, lastBegin, curr, getActiveConstraintRowSolverGeneric()));
						m_tasks.push_back(task);
						m_nonContactTasks.push_back(task);
					}
					lastA = a;
					lastB = b;
					lastBegin = curr;
				}
			}

			if (lastA && lastB)
			{
				auto task = SolverTaskPtr(new NonContactSolverTask(lastA, lastB, lastBegin, m_nonContactConstraintRowPtrs.data() + m_nonContactConstraintRowPtrs.size(), getActiveConstraintRowSolverGeneric()));
				m_tasks.push_back(task);
				m_nonContactTasks.push_back(task);
			}
		}

		for (int j = 0; j<numConstraints; j++)
		{
			if (constraints[j]->isEnabled())
			{
				btTypedConstraint::btConstraintInfo1 info1;
				constraints[j]->getInfo1(&info1);
				if (!info1.m_numConstraintRows && !info1.nub)
				{
					int bodyAid = getOrInitSolverBody(constraints[j]->getRigidBodyA(), infoGlobal.m_timeStep);
					int bodyBid = getOrInitSolverBody(constraints[j]->getRigidBodyB(), infoGlobal.m_timeStep);
					auto bodyA = &m_bodiesMt[bodyAid];
					auto bodyB = &m_bodiesMt[bodyBid];
					auto task = SolverTaskPtr(new ObsoleteSolverTask(bodyA, bodyB, constraints[j], infoGlobal.m_timeStep));
					m_tasks.push_back(task);
					m_nonContactTasks.push_back(task);
				}
			}
		}

		for (int i = 0; i < m_tmpSolverContactConstraintPool.size(); ++i)
		{
			int multiplier = (infoGlobal.m_solverMode & SOLVER_USE_2_FRICTION_DIRECTIONS) ? 2 : 1;
			auto c = &m_tmpSolverContactConstraintPool[i];
			auto a = &m_bodiesMt[c->m_solverBodyIdA];
			auto b = &m_bodiesMt[c->m_solverBodyIdB];
			auto f0 = &m_tmpSolverContactFrictionConstraintPool[i * multiplier];
			auto f1 = infoGlobal.m_solverMode & SOLVER_USE_2_FRICTION_DIRECTIONS ? &m_tmpSolverContactFrictionConstraintPool[i*multiplier + 1] : nullptr;
			auto task = SolverTaskPtr(new ContactSolverTask(a, b, c, f0, f1, getActiveConstraintRowSolverLowerLimit(), getActiveConstraintRowSolverGeneric()));
			m_tasks.push_back(task);
			m_contactTasks.push_back(task);
		}

		std::random_shuffle(m_tasks.begin(), m_tasks.end());
		return ret;
	}

	btScalar GroupConstraintSolver::solveGroupCacheFriendlyFinish(btCollisionObject ** bodies, int numBodies, const btContactSolverInfo & infoGlobal)
	{
		auto ret = Base::solveGroupCacheFriendlyFinish(bodies, numBodies, infoGlobal);
		m_tasks.clear();
		m_contactTasks.clear();
		m_nonContactTasks.clear();
		m_bodiesMt.clear();
		m_nonContactConstraintRowPtrs.clear();
		return ret;
	}

	btSingleConstraintRowSolver GroupConstraintSolver::getResolveSingleConstraintRowGenericAVX()
	{
		return gResolveSingleConstraintRowGeneric_avx256;
	}

	btSingleConstraintRowSolver GroupConstraintSolver::getResolveSingleConstraintRowLowerLimitAVX()
	{
		return gResolveSingleConstraintRowLowerLimit_avx256;
	}

	GroupConstraintSolver::GroupConstraintSolver()
	{
		int cpuFeatures = btCpuFeatureUtility::getCpuFeatures();
		if ((cpuFeatures & btCpuFeatureUtility::CPU_FEATURE_FMA3) && (cpuFeatures & btCpuFeatureUtility::CPU_FEATURE_SSE4_1))
		{
			m_resolveSingleConstraintRowGeneric = gResolveSingleConstraintRowGeneric_avx256;
			m_resolveSingleConstraintRowLowerLimit = gResolveSingleConstraintRowLowerLimit_avx256;
		}
	}

	btScalar GroupConstraintSolver::solveSingleIteration(int iteration, btCollisionObject** bodies, int numBodies, btPersistentManifold** manifoldPtr, int numManifolds, btTypedConstraint** constraints, int numConstraints, const btContactSolverInfo& infoGlobal, btIDebugDraw* debugDrawer)
	{
		int maxIterations = m_maxOverrideNumSolverIterations > infoGlobal.m_numIterations ? m_maxOverrideNumSolverIterations : infoGlobal.m_numIterations;
		if (iteration <= (maxIterations * 3 + 3) / 4)
		{
			concurrency::parallel_for_each(m_tasks.begin(), m_tasks.end(), [](const SolverTaskPtr& task) { task->solve(); });
		}
		else
		{
			std::random_shuffle(m_nonContactTasks.begin(), m_nonContactTasks.end());
			std::random_shuffle(m_contactTasks.begin(), m_contactTasks.end());
			concurrency::parallel_for_each(m_nonContactTasks.begin(), m_nonContactTasks.end(), [](const SolverTaskPtr& task) { task->solve(); });
			concurrency::parallel_for_each(m_contactTasks.begin(), m_contactTasks.end(), [](const SolverTaskPtr& task) { task->solve(); });
		}
		return FLT_MAX;
	}

}
