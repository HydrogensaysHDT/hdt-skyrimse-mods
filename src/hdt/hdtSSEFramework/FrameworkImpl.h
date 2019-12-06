#pragma once

#include "IFramework.h"
#include "EventDispatcherImpl.h"
#include <unordered_map>

namespace hdt
{
	class FrameworkImpl : public IFramework
	{
	public:
		FrameworkImpl();
		~FrameworkImpl();

		static FrameworkImpl* instance();
		virtual APIVersion getApiVersion() override;

		virtual bool isSupportedSkyrimVersion(uint32_t version) override;

		virtual IString* getString(const char* strBegin, const char* strEnd = nullptr) override;

		virtual IEventDispatcher<void*>* getCustomEventDispatcher(IString*) override;
		virtual IEventDispatcher<FrameEvent>* getFrameEventDispatcher() override;
		virtual IEventDispatcher<ShutdownEvent>* getShutdownEventDispatcher() override;
		virtual IEventDispatcher<ArmorAttachEvent>* getArmorAttachEventDispatcher() override;

		virtual float getFrameInterval(bool raceMenu) override;
		
		void hook();
		void unhook();

	protected:

		bool m_isHooked = false;

		EventDispatcherImpl<FrameEvent>			m_frameEventDispatcher;
		EventDispatcherImpl<ShutdownEvent>		m_shutdownEventDispatcher;
		EventDispatcherImpl<ArmorAttachEvent>	m_armorAttachEventDispatcher;

		std::mutex m_customEventLock;
		std::unordered_map<Ref<IString>, std::unique_ptr<EventDispatcherImpl<void*>>> m_customEventDispatchers;
	};
}
