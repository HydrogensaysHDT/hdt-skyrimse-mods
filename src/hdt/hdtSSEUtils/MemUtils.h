#pragma once

#include "stdafx.h"
#include "LogUtils.h"

namespace hdt
{
	struct HookPointPattern
	{
		size_t		offset;
		size_t		count;
		const char*	bytes;
	};

	std::vector<intptr_t> findHookPoint(std::vector<HookPointPattern> patterns);
	void printHookPoint(std::vector<HookPointPattern> patterns, const char* file, int line);
}
