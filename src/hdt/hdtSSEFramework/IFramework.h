#pragma once

#include "IEventDispatcher.h"
#include "HookArmor.h"
#include "HookEngine.h"
#include "HookScene.h"
#include "IString.h"

namespace hdt
{
	class IFramework
	{
	public:

		union APIVersion
		{
		public:
			APIVersion(uint32_t major, uint32_t minor) : majorVersion(major), minorVersion(minor) {}
			struct
			{
				uint32_t minorVersion;
				uint32_t majorVersion;
			};
			uint64_t version;
		};

		static IFramework* instance();

		virtual APIVersion getApiVersion() = 0;
		virtual bool isSupportedSkyrimVersion(uint32_t version) = 0;

		virtual IString* getString(const char* strBegin, const char* strEnd = nullptr) = 0;
		virtual IEventDispatcher<void*>* getCustomEventDispatcher(IString* name) = 0;
		virtual IEventDispatcher<FrameEvent>* getFrameEventDispatcher() = 0;
		virtual IEventDispatcher<ShutdownEvent>* getShutdownEventDispatcher() = 0;
		virtual IEventDispatcher<ArmorAttachEvent>* getArmorAttachEventDispatcher() = 0;

		virtual float getFrameInterval(bool raceMenu) = 0;

		inline IString* getString(const std::string s) { return getString(s.c_str(), s.c_str() + s.length()); }
	};
}

extern "C"
{
	hdt::IFramework* hdtGetFramework();
}
