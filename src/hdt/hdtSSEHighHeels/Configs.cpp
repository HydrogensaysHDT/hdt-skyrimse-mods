#include "stdafx.h"
#include "Configs.h"
#include "../hdtSSEUtils/LogUtils.h"

namespace hdt
{
	namespace configs
	{
		static const char* INIPath = "Data/SKSE/Plugins/hdtSSEHighHeels.ini";

		INIWrapper<float> offset("Adapter", "fGlobalOffset", 0.f, INIPath);
		INIWrapper<int> optionals("Adapter", "bfOptionals", 0x0FB, INIPath);
		INIWrapper<int> stableMode("Adapter", "iStableMode", 1, INIPath);
		INIWrapper<float> stableModeMaxSwing("Adapter", "fStableModeMaxSwing", 0.1f, INIPath);

		INIWrapper<int> decalUpdate("Performance", "iDecalUpdate", 20, INIPath);
		INIWrapper<int> priority("Performance", "iThreadPriority", -1, INIPath);

		INIWrapper<int> logLevel("Debug", "iLogLevel", LogManager::LOG_INFO, INIPath);
		INIWrapper<bool> enableDebugInfo("Debug", "bEnableDebugInfo", false, INIPath);

		void reloadAll()
		{
			offset.reload();
			optionals.reload();
			stableMode.reload();
			stableModeMaxSwing.reload();

			decalUpdate.reload();
			priority.reload();

			logLevel.reload();
			enableDebugInfo.reload();
		}
	}
}