#pragma once

#include "../hdtSSEUtils/INIUtils.h"

namespace hdt
{
	namespace configs
	{
		extern INIWrapper<float> offset;
		extern INIWrapper<int> stableMode;
		extern INIWrapper<float> stableModeMaxSwing;

		extern INIWrapper<int> optionals;
		enum
		{
			FLAG_ALCHEMY,
			FLAG_SMITHING,
			FLAG_SHARPENINGWHEEL,
			FLAG_ARMORTABLE,
			FLAG_ENCHANTING,
			FLAG_TANNINGRACK,
			FLAG_COOKPOT,
			FLAG_SMELTER,
			FLAG_MALE,
			FLAG_FIRSTPERSON,
		};

		extern INIWrapper<int> decalUpdate;
		extern INIWrapper<int> priority;

		extern INIWrapper<int> logLevel;
		extern INIWrapper<bool> enableDebugInfo;

		void reloadAll();
	}
}