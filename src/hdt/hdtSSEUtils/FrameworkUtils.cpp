#include "stdafx.h"
#include "FrameworkUtils.h"
#include "LogUtils.h"

#define NOMINMAX
#include <Windows.h>

namespace hdt
{
	IFramework * getFramework()
	{
		static IFramework* s = nullptr;
		if (s) return s;

		auto module = LoadLibraryW(L"Data/SKSE/Plugins/hdtSSEFramework.dll");
		if (!module) return nullptr;

		auto getter = (IFramework* (*)())GetProcAddress(module, "hdtGetFramework");
		if (!getter) return nullptr;

		s = getter();
		return s;
	}

	bool checkFrameworkVersion(uint32_t major, uint32_t minor)
	{
		auto framework = getFramework();
		if (!framework) return false;

		auto version = framework->getApiVersion();
		if (version.majorVersion != major || version.minorVersion < minor)
		{
			LogError("hdtSSEFramework api version mismatch");
			LogError("current version %u.%u", version.majorVersion, version.minorVersion);
			LogError("require version %u.X with X >= %u", major, minor);
			return false;
		}

		return true;
	}
}
