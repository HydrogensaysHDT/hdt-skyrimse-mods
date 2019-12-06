#pragma once

#include "hdtBulletHelper.h"
#include <ppl.h>
#include <BulletDynamics\MLCPSolvers\btDantzigSolver.h>

namespace hdt
{
	class DantzigSolver : public btDantzigSolver
	{
	public:
		virtual bool solveMLCP(const btMatrixXu & A, const btVectorXu & b, btVectorXu& x, const btVectorXu & lo, const btVectorXu & hi, const btAlignedObjectArray<int>& limitDependency, int numIterations, bool useSparsity = true);
	};
}
