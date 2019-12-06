#include "stdafx.h"
#include "HookUtils.h"

uintptr_t hdt::hookGetBaseAddr()
{
	static auto baseAddr = reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr));
	return baseAddr;
}
