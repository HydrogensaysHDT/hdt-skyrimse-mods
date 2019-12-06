#pragma once

#include <skse64/skse64/GameReferences.h>
#include <skse64/skse64/GameRTTI.h>

namespace hdt
{
	int getSex(Actor* akActor)
	{
		auto base = DYNAMIC_CAST(akActor->baseForm, TESForm, TESNPC);
		if (!base) return false;
		return CALL_MEMBER_FN(base, GetSex)();
	}
}