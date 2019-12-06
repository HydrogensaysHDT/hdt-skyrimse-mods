#include "stdafx.h"
#include "FrameworkImpl.h"
#include "HookUtils.h"
#include "HookEngine.h"
#include "HookArmor.h"
#include "StringImpl.h"
#include "../hdtSSEUtils/MemUtils.h"
#include <debugapi.h>

#include <skse64/skse64_common/skse_version.h>

namespace hdt
{
	FrameworkImpl::FrameworkImpl()
	{
	}

	FrameworkImpl::~FrameworkImpl()
	{
		unhook();
	}

	IFramework::APIVersion FrameworkImpl::getApiVersion()
	{
		return APIVersion(1, 2);
	}

	bool FrameworkImpl::isSupportedSkyrimVersion(uint32_t version)
	{
		return version == CURRENT_RELEASE_RUNTIME;
	}

	IString * FrameworkImpl::getString(const char * strBegin, const char * strEnd)
	{
		if (!strBegin) return nullptr;
		if (!strEnd) strEnd = strBegin + strlen(strBegin);
		return StringManager::instance()->get(strBegin, strEnd);
	}

	IEventDispatcher<void*>* FrameworkImpl::getCustomEventDispatcher(IString * name)
	{
		std::lock_guard<decltype(m_customEventLock)> l(m_customEventLock);
		auto iter = m_customEventDispatchers.find(name);
		if (iter == m_customEventDispatchers.end())
		{
			auto dispatcher = std::unique_ptr<EventDispatcherImpl<void*>>(new EventDispatcherImpl<void*>);
			iter = m_customEventDispatchers.insert(std::make_pair(name, std::move(dispatcher))).first;
		}
		return iter->second.get();
	}

	IEventDispatcher<FrameEvent>* FrameworkImpl::getFrameEventDispatcher()
	{
		return &m_frameEventDispatcher;
	}

	IEventDispatcher<ShutdownEvent>* FrameworkImpl::getShutdownEventDispatcher()
	{
		return &m_shutdownEventDispatcher;
	}

	IEventDispatcher<ArmorAttachEvent>* FrameworkImpl::getArmorAttachEventDispatcher()
	{
		return &m_armorAttachEventDispatcher;
	}
	
	float FrameworkImpl::getFrameInterval(bool raceMenu)
	{
		if (raceMenu)
			return *(float*)(hookGetBaseAddr() + 0x02f6b94c); // updateTimer instance + 0x1C
		else return *(float*)(hookGetBaseAddr() + 0x02f6b948); // updateTimer instance + 0x18
	}
	
	struct Timer
	{
		MEMBER_FN_PREFIX(Timer);
		DEFINE_MEMBER_FN_HOOK(updateTimer, void, 0x00c076a0, intptr_t);

		void updateTimer(intptr_t ptr)
		{
			static bool flag = false;
			if (!flag)
			{
				auto offset = (intptr_t)this - hookGetBaseAddr();
				hdt::LogInfo("updateTimer instance address : 0x%08llx", offset);
				flag = true;
			}
			CALL_MEMBER_FN(this, updateTimer)(ptr);
		}
	};
	
	void FrameworkImpl::hook()
	{
		if (!m_isHooked)
		{
			DetourRestoreAfterWith();
			DetourTransactionBegin();

			hookEngine();
			hookArmor();
			//hookScene();
			// NiNode::ctor 0x00C57AC0
			printHookPoint({
				HookPointPattern{0, 34, "\x48\x89\x4c\x24\x08\x57\x48\x83\xec\x30\x48\xc7\x44\x24\x20\xfe\xff\xff\xff\x48\x89\x5c\x24\x48\x48\x89\x74\x24\x58\x8b\xf2\x48\x8b\xf9"},
				HookPointPattern{0x2F, 16, "\x48\x89\x07\x48\x8d\x9f\x10\x01\x00\x00\x48\x89\x5c\x24\x50\x48"},
				}, __FILE__, __LINE__);
			// updateTimer
			printHookPoint({
				HookPointPattern{0, 48, "\x44\x8b\xd2\x4c\x8b\xc1\x44\x2b\x51\x28\x83\x79\x34\x00\x0f\x85\xeb\x01\x00\x00\xf3\x0f\x10\x51\x10\x0f\x57\xe4\x0f\x2e\xd4\x0f\x57\xc0\x74\x35\x45\x8b\x50\x24\x0f\x28\xca\xf3\x0f\x58\x49\x14"},
				}, __FILE__, __LINE__);
			DetourAttach((void**)Timer::_updateTimer_GetPtrAddr(), (void*)GetFnAddr(&Timer::updateTimer));
			
			DetourTransactionCommit();
			m_isHooked = true;
		}
	}

	void FrameworkImpl::unhook()
	{
		if (m_isHooked)
		{
			DetourRestoreAfterWith();
			DetourTransactionBegin();
			unhookEngine();
			unhookArmor();
			//unhookScene();
			DetourTransactionCommit();
			m_isHooked = false;
		}
	}

	FrameworkImpl * FrameworkImpl::instance()
	{
		static FrameworkImpl s;
		return &s;
	}

}